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

#include <stdio.h>
#include <string.h>
#include "sink.h"
#include "source.h"
#include "common.h"
#include "dsp_buffer.h"
#include "stream_audio_driver.h"
#include "hal_audio_afe_control.h"
#include "hal_audio_afe_define.h"
#include "audio_afe_common.h"
#include "hal_resource_assignment.h"
#ifdef MTK_BT_A2DP_ENABLE
#include "stream_n9_a2dp.h"
#endif
#ifdef AIR_BT_CLK_SKEW_ENABLE
#include "clk_skew.h"
#endif
#include "stream_audio_setting.h"
#include "stream_n9sco.h"
#include "dsp_callback.h"
#include "FreeRTOS.h"
#include "dsp_drv_afe.h"
#include "dsp_audio_msg.h"
#include "dtm.h"
#ifdef MTK_ANC_ENABLE
#include "anc_api.h"
#ifdef AIR_ANC_ADAPTIVE_CLOCK_CONTROL_ENABLE
#include "full_adapt_anc_api.h"
#endif
#endif
#ifdef AIR_HEARTHROUGH_MAIN_ENABLE
#include "stream_llf.h"
#endif

#ifdef AIR_BT_CODEC_BLE_ENABLED
#include "stream_n9ble.h"
#endif

#include "sfr_bt.h"
#include "hal_pdma_internal.h"
#include "hal_audio_driver.h"

#ifdef ENABLE_HWSRC_ON_MAIN_STREAM
#include "audio_hwsrc_monitor.h"
#endif

#include "bt_interface.h"

#ifdef AIR_DCHS_MODE_ENABLE
#include "stream_dchs.h"
#include "hal_audio_driver.h"
#endif

#if defined(AIR_WIRED_AUDIO_ENABLE)
#include "scenario_wired_audio.h"
#endif

#ifdef AIR_MIXER_STREAM_ENABLE
#include "stream_mixer.h"
#endif

#ifdef ENABLE_HWSRC_CLKSKEW
//#define HWSRC_UNDERRUN_DETECT
#endif

#ifdef AIR_FIXED_UL_STARTUP_MUTE_CUSTOM_DELAY_TIME_ENABLE
#define GENERIC_VUL1_STARTUP_DELAY (300)
#else
#define GENERIC_VUL1_STARTUP_DELAY (240)
#endif

#ifdef AIR_FIXED_LINEIN_STARTUP_MUTE_CUSTOM_DELAY_TIME_ENABLE
#define LINE_IN_VUL1_STARTUP_DELAY (4000)
#else
#define LINE_IN_VUL1_STARTUP_DELAY GENERIC_VUL1_STARTUP_DELAY
#endif
#ifdef HWSRC_UNDERRUN_DETECT
U32 hwsrc_out_remain = 0;
#endif

#ifdef AIR_BT_HFP_ENABLE
extern bool g_ignore_next_drop_flag;
#endif
#define HW_SYSRAM_PRIVATE_MEMORY_CCNI_START_ADDR *(U8*)0x8423FC00
#define AFE_OFFSET_PROTECT (16+256)

const afe_stream_channel_t connect_type[2][2] = { // [Stream][AFE]
    {STREAM_M_AFE_M, STREAM_M_AFE_S},
    {STREAM_S_AFE_M, STREAM_S_AFE_S}
};

#define WriteREG(_addr, _value) (*(volatile uint32_t *)(_addr) = (_value))
#define ReadREG(_addr)          (*(volatile uint32_t *)(_addr))

#define WORD_ALIGN(VALUE)       ((VALUE) & ~0x3) //4 bytes align

#define ASRC_CLCOK_SKEW_DEBUG (false)

extern bool hal_src_set_start(afe_mem_asrc_id_t src_id, bool enable);

#ifdef AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE
#include "scenario_ull_audio.h"
extern gaming_mode_dongle_ul_handle_t *gaming_mode_dongle_first_ul_handle;
#endif

#if defined(AIR_WIRELESS_MIC_RX_ENABLE)
#include "scenario_wireless_mic_rx.h"
#endif /* AIR_WIRELESS_MIC_RX_ENABLE */

#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
#include "scenario_ull_audio_v2.h"
#endif /* defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE) */

#ifdef AIR_ICE_DEBUG_ENABLE
#include "hal_ice_debug.h"
#endif

#ifdef MTK_BT_A2DP_ENABLE
U32 g_hwsrc_halt_check, g_pre_iro;
extern volatile bool g_a2dp_hwsrc_ng_flag;
#endif

void afe_dl_playen_release(SINK sink)
{
    hal_audio_memory_parameter_t *mem_handle = &(sink->param.audio.mem_handle);
    if (mem_handle->sync_status == HAL_AUDIO_MEMORY_SYNC_PLAY_EN) {
        /* set DAC_CON0 to release playen for other user */
        hal_audio_agent_t agent = hal_memory_convert_agent(mem_handle->memory_select);
        hal_memory_set_enable(agent, true);
        //AFE_SET_REG(AFE_AUDIO_BT_SYNC_CON0, 0x0003, 0x0003);
        //hal_gpt_delay_ms(1);//Set this bit as high at least 1ms and then set to zero to simulate the pulse width.
        hal_memory_set_palyen(agent, false);
        //AFE_SET_REG(AFE_AUDIO_BT_SYNC_CON0, 0x0000, 0x0003);
        DSP_MW_LOG_I("[DSP][AFE] release play en usage, type %d agent=%d", 2, sink->scenario_type, agent);
    }
}

ATTR_TEXT_IN_IRAM static bool afe_offset_overflow_check(uint32_t pre_offset, BUFFER_INFO *buffer_info, bool is_dl)
{
    uint32_t now_offset     = (is_dl) ? buffer_info->ReadOffset  : buffer_info->WriteOffset;
    uint32_t compare_offset = (is_dl) ? buffer_info->WriteOffset : buffer_info->ReadOffset;
    if (is_dl && buffer_info->bBufferIsFull) { // uplink ignore this
        return false;
    }
    if ((pre_offset == now_offset) && (pre_offset == compare_offset)) { // not move
        return true;
    } else if ((now_offset >= pre_offset) && (compare_offset <= now_offset) && (compare_offset > pre_offset)) { // already read/write overflow
        return true;
    } else if ((now_offset < pre_offset) && ((compare_offset <= now_offset) || (compare_offset > pre_offset))) { // already read/write overflow
        return true;
    }
    return false;
}

BOOL chk_dl1_cur(uint32_t cur_addr)
{
    UNUSED(cur_addr);
#if 0
    if ((AFE_GET_REG(AFE_AUDIO_BT_SYNC_CON0) & afe_get_bt_sync_enable_bit(AUDIO_DIGITAL_BLOCK_MEM_DL1)) //use sync mode
        && (afe_get_bt_sync_monitor_state(AUDIO_DIGITAL_BLOCK_MEM_DL1) == 0) //play en not start yet
        && ((cur_addr < AFE_GET_REG(AFE_DL1_BASE)) || (cur_addr > AFE_GET_REG(AFE_DL1_END)))) //cur addr isn't correct
#else
    //Keep flase before play en start
    if ((AFE_GET_REG(AFE_AUDIO_BT_SYNC_CON0) & afe_get_bt_sync_enable_bit((hal_audio_agent_t)AUDIO_DIGITAL_BLOCK_MEM_DL1)) //use sync mode
        && (afe_get_bt_sync_monitor_state(AUDIO_DIGITAL_BLOCK_MEM_DL1) == 0)) //play en not start yet
#endif
    {
        DSP_MW_LOG_W("chk_dl1_cur addr=0x%08x, base=0x%08x, end=0x%08x\n", 3, cur_addr, AFE_GET_REG(AFE_DL1_BASE), AFE_GET_REG(AFE_DL1_END));
        return false;
    }

    return true;
}

void vRegSetBit(uint32_t addr, uint32_t bit)
{
    uint32_t u4CurrValue, u4Mask;
    u4Mask = 1 << bit;
    u4CurrValue = ReadREG(addr);
    WriteREG(addr, (u4CurrValue | u4Mask));
    return;
}

void vRegResetBit(uint32_t addr, uint32_t bit)
{
    uint32_t u4CurrValue, u4Mask;
    u4Mask = 1 << bit;
    u4CurrValue = ReadREG(addr);
    WriteREG(addr, (u4CurrValue & (~u4Mask)));
    return;
}

/**/
/////////////////////////////////////////////////////////////////////////////////////////////
bool audio_ops_distinguish_audio_sink(void *param)
{
    bool is_au_sink = FALSE;
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
    if ((param == Sink_blks[SINK_TYPE_VP_AUDIO]) || (param == Sink_blks[SINK_TYPE_AUDIO]) || (param == Sink_blks[SINK_TYPE_AUDIO_DL3]) || (param == Sink_blks[SINK_TYPE_AUDIO_DL12]) || (param == Sink_blks[SINK_TYPE_DSP_VIRTUAL]) || (param == Sink_blks[SINK_TYPE_TDMAUDIO])) {
#else
    if ((param == Sink_blks[SINK_TYPE_VP_AUDIO]) || (param == Sink_blks[SINK_TYPE_AUDIO]) || (param == Sink_blks[SINK_TYPE_AUDIO_DL3]) || (param == Sink_blks[SINK_TYPE_AUDIO_DL12]) || (param == Sink_blks[SINK_TYPE_DSP_VIRTUAL])) {
#endif
        is_au_sink = TRUE;
    }
    return is_au_sink;
}
bool audio_ops_distinguish_audio_source(void *param)
{
    bool is_au_source = FALSE;

    if (param == Source_blks[SOURCE_TYPE_AUDIO]) {
        is_au_source = TRUE;
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
    } else if (param == Source_blks[SOURCE_TYPE_TDMAUDIO]) {
        is_au_source = TRUE;
#endif
#ifdef AIR_I2S_SLAVE_ENABLE
    } else if (param == Source_blks[SOURCE_TYPE_AUDIO2]) {
        is_au_source = TRUE;
#endif
#ifdef AIR_FULL_ADAPTIVE_ANC_ENABLE
    } else if (param == Source_blks[SOURCE_TYPE_ADAPT_ANC]) {
        is_au_source = TRUE;
#endif
#ifdef AIR_HW_VIVID_PT_ENABLE
    } else if (param == Source_blks[SOURCE_TYPE_HW_VIVID_PT]) {
        is_au_source = TRUE;
#endif
#if defined(AIR_MULTI_MIC_STREAM_ENABLE) || defined(MTK_ANC_SURROUND_MONITOR_ENABLE) || defined(AIR_WIRED_AUDIO_ENABLE) || defined(AIR_ADVANCED_PASSTHROUGH_ENABLE) || defined(AIR_ADAPTIVE_EQ_ENABLE) || defined (AIR_BT_AUDIO_DONGLE_ENABLE)
    } else {
        uint32_t search_source_type;
        for (search_source_type = SOURCE_TYPE_SUBAUDIO_MIN ; search_source_type <= SOURCE_TYPE_SUBAUDIO_MAX ; search_source_type++) {
            if (param == Source_blks[search_source_type]) {
                is_au_source = TRUE;
                break;
            }
        }
#endif
    }
    return is_au_source;
}


int32_t audio_ops_probe(void *param)
{
    int ret = -1;
    if (param == NULL) {
        DSP_MW_LOG_E("DSP audio ops parametser invalid\r\n", 0);
        return ret;
    }
    if (audio_ops_distinguish_audio_sink(param)) {
        SINK sink = param;
        if (sink->param.audio.ops->probe != NULL) {
            sink->param.audio.ops->probe(param);
            ret = 0;
        }
    } else if (audio_ops_distinguish_audio_source(param)) {
        SOURCE source = param;
        if (source->param.audio.ops->probe != NULL) {
            source->param.audio.ops->probe(param);
            ret = 0;
        }
    }
    return ret;
}

int32_t audio_ops_hw_params(void *param)
{
    int ret = -1;
    if (param == NULL) {
        DSP_MW_LOG_E("DSP audio ops parametser invalid\r\n", 0);
        return ret;
    }
    if (audio_ops_distinguish_audio_sink(param)) {
        SINK sink = param;
        if (sink->param.audio.ops->hw_params != NULL) {
            sink->param.audio.ops->hw_params(param);
            ret = 0;
        }
    } else if (audio_ops_distinguish_audio_source(param)) {
        SOURCE source = param;
        if (source->param.audio.ops->hw_params != NULL) {
            source->param.audio.ops->hw_params(param);
            ret = 0;
        }
    }
    return ret;
}

int32_t audio_ops_open(void *param)
{
    int ret = -1;
    if (param == NULL) {
        DSP_MW_LOG_E("DSP audio ops parametser invalid\r\n", 0);
        return ret;
    }
    if (audio_ops_distinguish_audio_sink(param)) {
        SINK sink = param;
        if (sink->param.audio.ops->open != NULL) {
            sink->param.audio.ops->open(param);
            ret = 0;
        }
    } else if (audio_ops_distinguish_audio_source(param)) {
        SOURCE source = param;
        if (source->param.audio.ops->open != NULL) {
            source->param.audio.ops->open(param);
            ret = 0;
        }
    }
    return ret;
}

bool audio_ops_close(void *param)
{
    int ret = false;
    if (param == NULL) {
        DSP_MW_LOG_E("DSP audio ops parametser invalid\r\n", 0);
        return ret;
    }
    if (audio_ops_distinguish_audio_sink(param)) {
        SINK sink = param;
        if (sink->param.audio.ops->close != NULL) {
            sink->param.audio.ops->close(param);
            ret = true;
        }
    } else if (audio_ops_distinguish_audio_source(param)) {
        SOURCE source = param;
        if (source->param.audio.ops->close != NULL) {
            source->param.audio.ops->close(param);
            ret = true;
        }
    }
    return ret;
}

int32_t audio_ops_trigger(void *param, int cmd)
{
    int ret = -1;
    if (param == NULL) {
        DSP_MW_LOG_E("DSP audio ops parametser invalid\r\n", 0);
        return ret;
    }
    if (audio_ops_distinguish_audio_sink(param) == TRUE) {
        SINK sink = param;
        if (sink->param.audio.ops != NULL) {
            if (sink->param.audio.ops->trigger != NULL) {
                sink->param.audio.ops->trigger(param, cmd);
                ret = 0;
            }
        }
    } else if (audio_ops_distinguish_audio_source(param) == TRUE) {
        SOURCE source = param;
        if (source->param.audio.ops != NULL) {
            if (source->param.audio.ops->trigger != NULL) {
                source->param.audio.ops->trigger(param, cmd);
                ret = 0;
            }
        }
    }
    return ret;
}

ATTR_TEXT_IN_IRAM_LEVEL_2 bool audio_ops_copy(void *param, void *src, uint32_t count)
{
    int ret = false;
    if (param == NULL) {
        DSP_MW_LOG_E("DSP audio ops parametser invalid\r\n", 0);
        return ret;
    }
    if (audio_ops_distinguish_audio_sink(param)) {
        SINK sink = param;
        if (sink->param.audio.ops->copy != NULL) {
            sink->param.audio.ops->copy(param, src, count);
            ret = true;
        }
    } else if (audio_ops_distinguish_audio_source(param)) {
        SOURCE source = param;
        if (source->param.audio.ops->copy != NULL) {
            source->param.audio.ops->copy(param, src, count);
            ret = true;
        }
    }
    return ret;
}

extern audio_sink_pcm_ops_t afe_platform_dl1_ops;
#ifdef MTK_PROMPT_SOUND_ENABLE
extern audio_sink_pcm_ops_t afe_platform_dl2_ops;
#endif
extern audio_source_pcm_ops_t afe_platform_ul1_ops;
#if defined(AIR_I2S_SLAVE_ENABLE)
extern audio_sink_pcm_ops_t i2s_slave_dl_ops;
extern audio_source_pcm_ops_t i2s_slave_ul_ops;
#endif

void audio_afe_set_ops(void *param)
{
    if (param == NULL) {
        DSP_MW_LOG_E("DSP audio ops parametser invalid\r\n", 0);
        return;
    }
    if (audio_ops_distinguish_audio_sink(param)) {
        SINK sink = param;
#if defined(AIR_I2S_SLAVE_ENABLE)
        if (sink->param.audio.audio_device == HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) {
            sink->param.audio.ops = (audio_pcm_ops_p)&i2s_slave_dl_ops;
            return;
        }
#endif
        sink->param.audio.ops = (audio_pcm_ops_p)&afe_platform_dl1_ops;
    } else if (audio_ops_distinguish_audio_source(param)) {
        SOURCE source = param;
#if defined(AIR_I2S_SLAVE_ENABLE)
        if (source->param.audio.audio_device == HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) {
            source->param.audio.ops = (audio_pcm_ops_p)&i2s_slave_ul_ops;
            return;
        }
#endif
        source->param.audio.ops = (audio_pcm_ops_p)&afe_platform_ul1_ops;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
void afe_sink_prefill_silence_data(SINK sink)
{
    afe_block_t *afe_block = &sink->param.audio.AfeBlkControl;
    BUFFER_INFO *buffer_info = &sink->streamBuffer.BufferInfo;

    if (afe_block->u4BufferSize <= buffer_info->length) {
        afe_block->u4BufferSize =  buffer_info->length;
    }

    afe_block->u4DataRemained = ((buffer_info->WriteOffset >= buffer_info->ReadOffset)
                                 ? (buffer_info->WriteOffset - buffer_info->ReadOffset)
                                 : (buffer_info->length - buffer_info->ReadOffset + buffer_info->WriteOffset));

    if (sink->param.audio.channel_num == 2) {
        afe_block->u4DataRemained <<= 1;
    }

    afe_block->u4WriteIdx += afe_block->u4DataRemained;
    afe_block->u4WriteIdx %= afe_block->u4BufferSize;
}

void afe_source_prefill_silence_data(SOURCE source)
{
    afe_block_t *afe_block = &source->param.audio.AfeBlkControl;
    BUFFER_INFO *buffer_info = &source->streamBuffer.BufferInfo;

    if (afe_block->u4BufferSize <= buffer_info->length) {
        afe_block->u4BufferSize =  buffer_info->length;
    }

    if (source->param.audio.channel_num == 2) {
        afe_block->u4ReadIdx = (buffer_info->ReadOffset << 1) % afe_block->u4BufferSize;
    } else {
        afe_block->u4ReadIdx = buffer_info->ReadOffset % afe_block->u4BufferSize;
    }
}

/*
 * Get dl1 afe and sink buffer
 * Units: sample
*/
ATTR_TEXT_IN_IRAM_LEVEL_2 uint32_t afe_get_dl1_query_data_amount(void)
{
    volatile SINK sink = Sink_blks[SINK_TYPE_AUDIO];
    uint32_t afe_sram_data_count, sink_data_count;
    uint32_t output_rate = (sink->param.audio.rate)/100;
    uint32_t input_rate = (sink->param.audio.src_rate)/100;
    uint32_t owo, oro, osize, src_out_data;
    if (sink == NULL) {
        return 0;
    }
    //AFE DL1 SRAM data amount
#if 0
    afe_block_t *afe_block = &sink->param.audio.AfeBlkControl;
    int32_t hw_current_read_idx = AFE_GET_REG(AFE_DL1_CUR);
    afe_block->u4ReadIdx = hw_current_read_idx - AFE_GET_REG(AFE_DL1_BASE);
    if (afe_block->u4WriteIdx > afe_block->u4ReadIdx) {
        *afe_sram_data_count = afe_block->u4WriteIdx - afe_block->u4ReadIdx;
    } else {
        *afe_sram_data_count = afe_block->u4BufferSize + afe_block->u4WriteIdx - afe_block->u4ReadIdx;
    }
#else
    afe_sram_data_count = 0;
#endif
    //Sink audio data amount
    U32 buffer_per_channel_shift = ((sink->param.audio.channel_num >= 2) && (sink->buftype == BUFFER_TYPE_INTERLEAVED_BUFFER))
                                   ? 1
                                   : 0;
    sink_data_count = (sink->streamBuffer.BufferInfo.length >> buffer_per_channel_shift) - SinkSlack(sink);

    if (sink->param.audio.AfeBlkControl.u4asrcflag == true) {
        owo = AFE_GET_REG(ASM_CH01_OBUF_WRPNT) - AFE_GET_REG(ASM_OBUF_SADR);
        oro = AFE_GET_REG(ASM_CH01_OBUF_RDPNT) - AFE_GET_REG(ASM_OBUF_SADR);
        osize = AFE_GET_REG(ASM_OBUF_SIZE);
        src_out_data = (owo > oro) ? (owo - oro) : (owo + osize - oro);
        sink_data_count += ((src_out_data >> buffer_per_channel_shift) * input_rate) / output_rate;
    }

    return ((afe_sram_data_count + sink_data_count) / sink->param.audio.format_bytes);
}

uint32_t i2s_slave_port_translate(hal_audio_interface_t audio_interface)
{
    uint32_t port;

    if (audio_interface == HAL_AUDIO_INTERFACE_1) {
        port = 0;
    } else if (audio_interface == HAL_AUDIO_INTERFACE_2) {
        port = 1;
    } else if (audio_interface == HAL_AUDIO_INTERFACE_3) {
        port = 2;
    } else {
        port = 2;
    }

    return port;
}

const vdma_channel_t g_i2s_slave_vdma_channel_infra[] = {
    VDMA_I2S3TX, VDMA_I2S3RX,//I2S0 DMA TX(VDMA7),  I2S0 DMA RX(VDMA8)
    VDMA_I2S0TX, VDMA_I2S0RX,//I2S1 DMA TX(VDMA1),  I2S1 DMA RX(VDMA2)
    VDMA_I2S4TX, VDMA_I2S4RX,//I2S2 DMA TX(VDMA9),  I2S2 DMA RX(VDMA10)
};

#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
const vdma_channel_t g_i2s_slave_vdma_channel_tdm[] = {
    VDMA_I2S0TX, VDMA_I2S0RX,//I2S1 2CH TX(VDMA1),  I2S1 2CH RX(VDMA2)
    VDMA_I2S1TX, VDMA_I2S1RX,//I2S1 4CH TX(VDMA3),  I2S1 4CH RX(VDMA4)
    VDMA_I2S2TX, VDMA_I2S2RX,//I2S1 6CH TX(VDMA5),  I2S1 6CH RX(VDMA6)
    VDMA_I2S3TX, VDMA_I2S3RX,//I2S1 8CH TX(VDMA7),  I2S1 8CH RX(VDMA8)
    VDMA_I2S4TX, VDMA_I2S4RX,//I2S2 2CH TX(VDMA9),  I2S2 2CH RX(VDMA10)
    VDMA_I2S5TX, VDMA_I2S5RX,//I2S2 4CH TX(VDMA11), I2S2 4CH RX(VDMA12)
    VDMA_I2S6TX, VDMA_I2S6RX,//I2S2 6CH TX(VDMA13), I2S2 6CH RX(VDMA14)
    VDMA_I2S7TX, VDMA_I2S7RX,//I2S2 8CH TX(VDMA15), I2S2 8CH RX(VDMA16)
};
#endif

void i2s_slave_ul_update_rptr(vdma_channel_t rx_dma_channel, U32 amount)
{
    vdma_set_sw_move_byte(rx_dma_channel, amount);
}

void i2s_slave_dl_update_wptr(vdma_channel_t tx_dma_channel, U32 amount)
{
    vdma_set_sw_move_byte(tx_dma_channel, amount);
}

ATTR_TEXT_IN_IRAM void i2s_slave_ul_interrupt_handler(void)
{
    uint32_t mask, port, hw_current_write_idx;
    SOURCE source = Source_blks[SOURCE_TYPE_AUDIO];
    hal_audio_interface_t audio_interface = source->param.audio.audio_interface;
    vdma_channel_t rx_dma_channel;
    uint32_t update_frame_size = source->param.audio.count * source->param.audio.channel_num * source->param.audio.format_bytes;//unit:bytes
    BUFFER_INFO *buffer_info = &source->streamBuffer.BufferInfo;
    afe_block_t *afe_block = &Source_blks[SOURCE_TYPE_AUDIO]->param.audio.AfeBlkControl;

    hal_nvic_save_and_set_interrupt_mask(&mask);

    port = i2s_slave_port_translate(audio_interface);
    rx_dma_channel = g_i2s_slave_vdma_channel_infra[port * 2 + 1];

    // Get last WPTR and record current WPTR
    vdma_get_hw_write_point(rx_dma_channel, &hw_current_write_idx);
    if (afe_block->u4asrcflag) {
    } else {
        buffer_info->WriteOffset = hw_current_write_idx - afe_block->phys_buffer_addr;
    }

    vdma_disable_interrupt(rx_dma_channel);
    i2s_slave_ul_update_rptr(rx_dma_channel, update_frame_size * 4);
    vdma_enable_interrupt(rx_dma_channel);

    AudioCheckTransformHandle(source->transform);
    hal_nvic_restore_interrupt_mask(mask);
}
#ifdef AIR_I2S_SLAVE_ENABLE
ATTR_TEXT_IN_IRAM void i2s_slave_ul_port_interrupt_handler(vdma_event_t event, void  *user_data)
{
    // DSP_MW_LOG_I("i2s_slave_0_ul_interrupt_handler",0);
    UNUSED(event);
    UNUSED(user_data);
    i2s_slave_irq_user_data_t *data_ptr = (i2s_slave_irq_user_data_t *)user_data;
    // uint32_t mask;
    uint32_t hw_current_write_idx = 0;
    uint32_t hw_current_read_idx  = 0;
    SOURCE                source                = NULL;
    afe_block_t           *afe_block            = NULL;
    U32                   addr_offset           = 0x100 * data_ptr->asrc_id;
                          source                = data_ptr->source;
    vdma_channel_t        rx_dma_channel        = data_ptr->vdma_channel;
                          afe_block             = &(source->param.audio.AfeBlkControl);
    uint32_t              update_frame_size     = source->param.audio.count * source->param.audio.channel_num * source->param.audio.format_bytes;  //unit:bytes
    BUFFER_INFO           *buffer_info          = &source->streamBuffer.BufferInfo;
    bool                  is_sub_source_flag    = false;
    bool                  is_source_buffer_full = false;
    hal_audio_interface_t audio_interface       = source->param.audio.audio_interface;
    hal_audio_device_t    device                = source->param.audio.audio_device;
    uint32_t cur_iro = 0;
    uint32_t cur_iwo = 0;
    uint32_t cur_owo = 0;
    if ((source->type >= SOURCE_TYPE_SUBAUDIO_MIN) && (source->type <= SOURCE_TYPE_SUBAUDIO_MAX)) {
        is_sub_source_flag = true;
    }
    // hal_nvic_save_and_set_interrupt_mask(&mask);
    vdma_get_hw_write_point(rx_dma_channel, &hw_current_write_idx);
    if (afe_block->u4asrcflag) {
        // DSP_MW_LOG_I("[afe irq] i2s type %d slave_%d dma channel %d index %d iwo %d iro %d owo %d oro %d", 8,
        //     source->scenario_type,
        //     data_ptr->asrc_id,
        //     rx_dma_channel,
        //     hw_current_write_idx - afe_block->phys_buffer_addr,
        //     AFE_GET_REG(ASM_CH01_IBUF_WRPNT + addr_offset) - AFE_READ(ASM_IBUF_SADR + addr_offset),
        //     AFE_GET_REG(ASM_CH01_IBUF_RDPNT + addr_offset) - AFE_READ(ASM_IBUF_SADR + addr_offset),
        //     AFE_GET_REG(ASM_CH01_OBUF_WRPNT + addr_offset) - AFE_READ(ASM_OBUF_SADR + addr_offset),
        //     AFE_GET_REG(ASM_CH01_OBUF_RDPNT + addr_offset) - AFE_READ(ASM_OBUF_SADR + addr_offset)
        //     );
        if (AFE_GET_REG(ASM_CH01_OBUF_RDPNT + addr_offset) == AFE_GET_REG(ASM_CH01_OBUF_WRPNT + addr_offset)) {
            DSP_MW_LOG_W("slave_%d asrc out buffer RPTR=WPTR, R=0x%x, W=0x%x, asrc in buffer, R=0x%x, W=0x%x", 5,
                    data_ptr->asrc_id,
                    AFE_GET_REG(ASM_CH01_OBUF_RDPNT + addr_offset),
                    AFE_GET_REG(ASM_CH01_OBUF_WRPNT + addr_offset),
                    AFE_GET_REG(ASM_CH01_IBUF_RDPNT + addr_offset),
                    AFE_GET_REG(ASM_CH01_IBUF_WRPNT + addr_offset)
                    );
            source->streamBuffer.BufferInfo.bBufferIsFull = TRUE;
            is_source_buffer_full = true;
        }
        cur_iro = AFE_GET_REG(ASM_CH01_IBUF_RDPNT + addr_offset) - AFE_GET_REG(ASM_IBUF_SADR + addr_offset);
        vdma_get_hw_read_point(rx_dma_channel, &hw_current_read_idx);
        cur_iwo = hw_current_write_idx - afe_block->phys_buffer_addr;
        AFE_WRITE(ASM_CH01_IBUF_WRPNT + addr_offset, hw_current_write_idx);
        update_frame_size = data_ptr->vdma_threshold_samples; // Actually, we should read the RG value. AFE_GET_REG(VDMA_RG_I2S0_RX_BASE) * (1<<((AFE_GET_REG(VDMA_RG_I2S0_RX_BASE+0x4)>>8) & 0x3))
        cur_owo = AFE_GET_REG(ASM_CH01_OBUF_WRPNT + addr_offset) - AFE_GET_REG(ASM_OBUF_SADR + addr_offset);
    } else {
        cur_owo = hw_current_write_idx - afe_block->phys_buffer_addr;
    }
    vdma_disable_interrupt(rx_dma_channel);
    i2s_slave_ul_update_rptr(rx_dma_channel, update_frame_size * 4);
    vdma_enable_interrupt(rx_dma_channel);
    if (is_sub_source_flag) {
        SOURCE_TYPE source_type;
        SOURCE source_tmp = NULL;
        for (source_type = SOURCE_TYPE_SUBAUDIO_MIN; source_type <= SOURCE_TYPE_SUBAUDIO_MAX; source_type++) {
            source_tmp = Source_blks[source_type];
            if ((!source_tmp) || (!source_tmp->param.audio.is_memory_start)) {
                continue;
            }

            if ((device != source_tmp->param.audio.audio_device) || (audio_interface != source_tmp->param.audio.audio_interface)) {
                continue;
            }
            buffer_info = &source_tmp->streamBuffer.BufferInfo;
            afe_block   = &(source_tmp->param.audio.AfeBlkControl);
            source_tmp->streamBuffer.BufferInfo.bBufferIsFull = is_source_buffer_full;
            afe_block->u4ReadIdx  = cur_iro;
            afe_block->u4WriteIdx = cur_iwo;
            if ((source_tmp->scenario_type == AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_I2S_IN) ||
                (source_tmp->scenario_type == AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_I2S_IN) ||
                ((source_tmp->scenario_type >= AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_LINE_IN) && (source_tmp->scenario_type <= AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0)) ||
                ((source_tmp->scenario_type >= AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1) && (source_tmp->scenario_type <= AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2))) {
                /* do nothing */
            } else {
                /* Dongle will update it by self */
                // DSP_MW_LOG_I("source type %d iro %d iwo %d", 3, source_tmp->scenario_type, buffer_info->ReadOffset, buffer_info->WriteOffset);
                buffer_info->WriteOffset = cur_owo;
                /* First handle */
                AUDIO_PARAMETER *runtime = &source_tmp->param.audio;;
                if (runtime->irq_exist == false) {
                    runtime->irq_exist = true;
                    uint32_t vul_irq_time;
                    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &vul_irq_time);
                    runtime->pop_noise_pkt_num = 0xFFFF; // avoid mute by irq sub_source common handler
                    runtime->mute_flag = false;          // avoid mute by irq sub_source common handler
                    DSP_MW_LOG_I("slave_%d afe sub-source:%d interrupt exist, time: %d", 3,
                        data_ptr->asrc_id,
                        source_tmp->scenario_type,
                        vul_irq_time);
                }
                /* ULL Dongle I2S IN no need resume stream */
                // if ((source_tmp->scenario_type >= AUDIO_SCENARIO_TYPE_WIRED_AUDIO_AFE_IN_OUT_0) && (source_tmp->scenario_type <= AUDIO_SCENARIO_TYPE_WIRED_AUDIO_AFE_IN_OUT_1)) {
                //     /* wired audio i2s in won't need */
                // } else {
                    AudioCheckTransformHandle(source_tmp->transform);
                // }
            }
        }
    } else {
        SOURCE source_tmp = source;
        source_tmp->streamBuffer.BufferInfo.bBufferIsFull = is_source_buffer_full;
        afe_block->u4ReadIdx  = cur_iro;
        afe_block->u4WriteIdx = cur_iwo;
        if ((source_tmp->scenario_type == AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_I2S_IN) ||
            (source_tmp->scenario_type == AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_I2S_IN) ||
            ((source_tmp->scenario_type >= AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_LINE_IN) && (source_tmp->scenario_type <= AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0)) ||
            ((source_tmp->scenario_type >= AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1) && (source_tmp->scenario_type <= AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2))) {
            /* do nothing */
        } else {
            /* Dongle will update it by self */
            buffer_info->WriteOffset = cur_owo;
            /* First handle */
            AUDIO_PARAMETER *runtime = &source_tmp->param.audio;;
            if (runtime->irq_exist == false) {
                runtime->irq_exist = true;
                uint32_t vul_irq_time;
                hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &vul_irq_time);
                runtime->pop_noise_pkt_num = 0xFFFF; // avoid mute by irq sub_source common handler
                runtime->mute_flag = false;          // avoid mute by irq sub_source common handler
                DSP_MW_LOG_I("slave_%d afe sub-source:%d interrupt exist, time: %d", 3,
                    data_ptr->asrc_id,
                    source_tmp->scenario_type,
                    vul_irq_time);
            }
            /* ULL Dongle I2S IN no need resume stream */
            // if ((source_tmp->scenario_type >= AUDIO_SCENARIO_TYPE_WIRED_AUDIO_AFE_IN_OUT_0) && (source_tmp->scenario_type <= AUDIO_SCENARIO_TYPE_WIRED_AUDIO_AFE_IN_OUT_1)) {
            //     /* wired audio i2s in won't need */
            // } else {
                AudioCheckTransformHandle(source_tmp->transform);
            // }
        }
    }
}
#endif

ATTR_TEXT_IN_IRAM void i2s_slave_dl_interrupt_handler(vdma_event_t event, void  *user_data)
{
    UNUSED(event);
    UNUSED(user_data);
    uint32_t mask, port, hw_current_read_idx;
    SINK sink = Sink_blks[SINK_TYPE_AUDIO];
    hal_audio_interface_t audio_interface = sink->param.audio.audio_interface;
    vdma_channel_t tx_dma_channel;
    uint32_t update_frame_size = sink->param.audio.count * sink->param.audio.channel_num * sink->param.audio.format_bytes;//unit:bytes
    BUFFER_INFO *buffer_info = &sink->streamBuffer.BufferInfo;
    uint32_t dl_base_addr = (uint32_t)buffer_info->startaddr[0];
    uint32_t pre_offset, isr_interval;

    hal_nvic_save_and_set_interrupt_mask(&mask);

    port = i2s_slave_port_translate(audio_interface);
    tx_dma_channel = g_i2s_slave_vdma_channel_infra[port * 2];

    /* Get last RPTR and record current RPTR */
    pre_offset = buffer_info->ReadOffset;
    vdma_get_hw_read_point(tx_dma_channel, &hw_current_read_idx);

    buffer_info->ReadOffset = hw_current_read_idx - dl_base_addr;
    isr_interval = (pre_offset <= buffer_info->ReadOffset)
                   ? (buffer_info->ReadOffset - pre_offset)
                   : (buffer_info->length + buffer_info->ReadOffset - pre_offset);

    if (buffer_info->ReadOffset != buffer_info->WriteOffset) {
        buffer_info->bBufferIsFull = FALSE;
    }
    /* Check whether underflow happen */
    if (afe_offset_overflow_check(pre_offset, buffer_info, true)) {
        DSP_MW_LOG_I("SLAVE DL, SRAM Empty play en:%d pR:%d R:%d W:%d", 4, isr_interval, pre_offset, buffer_info->ReadOffset, buffer_info->WriteOffset);
        buffer_info->WriteOffset = (buffer_info->ReadOffset + 2 * isr_interval) % buffer_info->length;
        if (buffer_info->WriteOffset > buffer_info->ReadOffset) {
            memset((void *)(dl_base_addr + buffer_info->ReadOffset), 0, buffer_info->WriteOffset - buffer_info->ReadOffset);
        } else {
            memset((void *)(dl_base_addr + buffer_info->ReadOffset), 0, buffer_info->length - buffer_info->ReadOffset);
            memset((void *)dl_base_addr, 0, buffer_info->WriteOffset);
        }
    } else if (buffer_info->ReadOffset != buffer_info->WriteOffset) {
        buffer_info->bBufferIsFull = FALSE;
    }

    vdma_disable_interrupt(tx_dma_channel);
    i2s_slave_dl_update_wptr(tx_dma_channel, update_frame_size * 4);
    vdma_enable_interrupt(tx_dma_channel);

    AudioCheckTransformHandle(sink->transform);
    hal_nvic_restore_interrupt_mask(mask);
}

#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
ATTR_TEXT_IN_IRAM void i2s_slave_ul_tdm_interrupt_handler(void)
{
    uint32_t mask, port = 0, hw_current_write_idx;
    SOURCE source = Source_blks[SOURCE_TYPE_TDMAUDIO];
    hal_audio_interface_t audio_interface = source->param.audio.audio_interface;
    hal_audio_i2s_tdm_channel_setting_t tdm_channel = source->param.audio.device_handle.i2s_slave.tdm_channel;
    vdma_channel_t rx_dma_channel = 0, dma_set_ch0 = 0, dma_set_ch1 = 0, dma_set_ch2 = 0, dma_set_ch3 = 0;
    uint8_t channel_num     = (source->param.audio.channel_num >= 2) ? 2 : 1;
    uint32_t update_frame_size = source->param.audio.count * channel_num * source->param.audio.format_bytes;//unit:bytes
    uint32_t setting_cnt = 0, dma_setting_count = 0;

    BUFFER_INFO *buffer_info = &source->streamBuffer.BufferInfo;
    afe_block_t *afe_block = &Source_blks[SOURCE_TYPE_TDMAUDIO]->param.audio.AfeBlkControl;

    uint32_t volatile dma_int;
    dma_int = I2S_DMA_RG_GLB_STA;

    hal_nvic_save_and_set_interrupt_mask(&mask);

    if (audio_interface == HAL_AUDIO_INTERFACE_2) {
        port = 0;
    } else if (audio_interface == HAL_AUDIO_INTERFACE_3) {
        port = 1;
    }

    dma_set_ch0 = g_i2s_slave_vdma_channel_tdm[port * 8 + 1];

    if (tdm_channel >= HAL_AUDIO_I2S_TDM_4CH) {
        dma_set_ch1 = g_i2s_slave_vdma_channel_tdm[port * 8 + 1 + 2];
        dma_setting_count = 2;
    }
    if (tdm_channel >= HAL_AUDIO_I2S_TDM_6CH) {
        dma_set_ch2 = g_i2s_slave_vdma_channel_tdm[port * 8 + 1 + 4];
        dma_setting_count = 3;
    }
    if (tdm_channel >= HAL_AUDIO_I2S_TDM_8CH) {
        dma_set_ch3 = g_i2s_slave_vdma_channel_tdm[port * 8 + 1 + 6];
        dma_setting_count = 4;
    }

    rx_dma_channel = dma_set_ch0;

    // Get last WPTR and record current WPTR
    vdma_get_hw_write_point(rx_dma_channel, &hw_current_write_idx);
    if (afe_block->u4asrcflag) {
    } else {
        buffer_info->WriteOffset = hw_current_write_idx - afe_block->phys_buffer_addr;
    }

    for (setting_cnt = 0; setting_cnt < dma_setting_count; setting_cnt++) {
        if (setting_cnt == 0) {
            rx_dma_channel = dma_set_ch0;
        } else if (setting_cnt == 1) {
            rx_dma_channel = dma_set_ch1;
        } else if (setting_cnt == 2) {
            rx_dma_channel = dma_set_ch2;
        } else {
            rx_dma_channel = dma_set_ch3;
        }
        vdma_disable_interrupt(rx_dma_channel);
        i2s_slave_ul_update_rptr(rx_dma_channel, update_frame_size * 4);
        vdma_enable_interrupt(rx_dma_channel);
    }
    AudioCheckTransformHandle(source->transform);
    hal_nvic_restore_interrupt_mask(mask);
}

ATTR_TEXT_IN_IRAM void i2s_slave_dl_tdm_interrupt_handler(void)
{
    uint32_t mask, port = 0, hw_current_read_idx;
    volatile SINK sink = Sink_blks[SINK_TYPE_TDMAUDIO];
    hal_audio_interface_t audio_interface = sink->param.audio.audio_interface;
    hal_audio_i2s_tdm_channel_setting_t tdm_channel = sink->param.audio.device_handle.i2s_slave.tdm_channel;
    vdma_channel_t tx_dma_channel = 0, dma_set_ch0 = 0, dma_set_ch1 = 0, dma_set_ch2 = 0, dma_set_ch3 = 0;
    uint8_t channel_num     = (sink->param.audio.channel_num >= 2) ? 2 : 1;
    uint32_t update_frame_size = sink->param.audio.count * channel_num * sink->param.audio.format_bytes;//unit:bytes
    uint32_t setting_cnt = 0, dma_setting_count = 0;

    BUFFER_INFO *buffer_info = &sink->streamBuffer.BufferInfo;
    uint32_t dl_base_addr = (uint32_t)buffer_info->startaddr[0];
    uint32_t pre_offset, isr_interval;

    uint32_t volatile dma_int;
    dma_int = I2S_DMA_RG_GLB_STA;

    hal_nvic_save_and_set_interrupt_mask(&mask);

    if (audio_interface == HAL_AUDIO_INTERFACE_2) {
        port = 0;
    } else if (audio_interface == HAL_AUDIO_INTERFACE_3) {
        port = 1;
    }

    dma_set_ch0 = g_i2s_slave_vdma_channel_tdm[port * 8];

    if (tdm_channel >= HAL_AUDIO_I2S_TDM_4CH) {
        dma_set_ch1 = g_i2s_slave_vdma_channel_tdm[port * 8 + 2];
        dma_setting_count = 2;
    }
    if (tdm_channel >= HAL_AUDIO_I2S_TDM_6CH) {
        dma_set_ch2 = g_i2s_slave_vdma_channel_tdm[port * 8 + 4];
        dma_setting_count = 3;
    }
    if (tdm_channel >= HAL_AUDIO_I2S_TDM_8CH) {
        dma_set_ch3 = g_i2s_slave_vdma_channel_tdm[port * 8 + 6];
        dma_setting_count = 4;
    }

    tx_dma_channel = dma_set_ch0;

    /* Get last RPTR and record current RPTR */
    pre_offset = buffer_info->ReadOffset;
    vdma_get_hw_read_point(tx_dma_channel, &hw_current_read_idx);

    buffer_info->ReadOffset = hw_current_read_idx - dl_base_addr;
    isr_interval = (pre_offset <= buffer_info->ReadOffset)
                   ? (buffer_info->ReadOffset - pre_offset)
                   : (buffer_info->length + buffer_info->ReadOffset - pre_offset);

    if (buffer_info->ReadOffset != buffer_info->WriteOffset) {
        buffer_info->bBufferIsFull = FALSE;
    }

    /* Check whether underflow happen */
    if (afe_offset_overflow_check(pre_offset, buffer_info, true)) {
        DSP_MW_LOG_I("SLAVE DL TDM,SRAM Empty play en:%d pR:%d R:%d W:%d", 4, isr_interval, pre_offset, buffer_info->ReadOffset, buffer_info->WriteOffset);
        buffer_info->WriteOffset = (buffer_info->ReadOffset + 2 * isr_interval) % buffer_info->length;
        if (buffer_info->WriteOffset > buffer_info->ReadOffset) {
            // TBD
            memset((void *)(dl_base_addr + buffer_info->ReadOffset), 0, buffer_info->WriteOffset - buffer_info->ReadOffset);
        } else {
            memset((void *)(dl_base_addr + buffer_info->ReadOffset), 0, buffer_info->length - buffer_info->ReadOffset);
            // TBD
            memset((void *)dl_base_addr, 0, buffer_info->WriteOffset);
        }
    } else if (buffer_info->ReadOffset != buffer_info->WriteOffset) {
        buffer_info->bBufferIsFull = FALSE;
    }

    for (setting_cnt = 0; setting_cnt < dma_setting_count; setting_cnt++) {
        if (setting_cnt == 0) {
            tx_dma_channel = dma_set_ch0;
        } else if (setting_cnt == 1) {
            tx_dma_channel = dma_set_ch1;
        } else if (setting_cnt == 2) {
            tx_dma_channel = dma_set_ch2;
        } else {
            tx_dma_channel = dma_set_ch3;
        }
        vdma_disable_interrupt(tx_dma_channel);
        i2s_slave_dl_update_wptr(tx_dma_channel, update_frame_size * 4);
        vdma_enable_interrupt(tx_dma_channel);
    }
    AudioCheckTransformHandle(sink->transform);
    hal_nvic_restore_interrupt_mask(mask);
}
#endif

int32_t dl_irq_cnt = 0;
#ifdef AIR_BT_CODEC_BLE_ENABLED
uint32_t ble_dl_gpt_start;
extern uint16_t g_ble_abr_length;
#endif

#if defined(AIR_WIRELESS_MIC_RX_ENABLE)
ATTR_TEXT_IN_IRAM void afe_dl1_wireless_mic_rx_interrupt_handler(void)
{
    uint32_t first_dl_irq_time, hw_current_read_idx;
    uint32_t hwsrc_out_avail, afe_hwsrc_len, afe_hwsrc_rptr, afe_hwsrc_wptr;
    SINK sink = Sink_blks[SINK_TYPE_AUDIO];
    SOURCE source;
    DSP_STREAMING_PARA_PTR pStream;
    BTCLK bt_clk;
    BTPHASE bt_phase;
    wireless_mic_rx_ul_handle_t *rx_handle;
    hal_audio_set_value_parameter_t set_value_parameter;

    MCE_GetBtClk(&bt_clk,&bt_phase, BT_CLK_Offset);

    source = sink->transform->source;
    pStream = DSP_Streaming_Get(source, sink);
    rx_handle = (wireless_mic_rx_ul_handle_t *)(source->param.bt_common.scenario_param.dongle_handle);
    rx_handle->ccni_in_bt_count = bt_clk;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &(rx_handle->ccni_in_gpt_count));

#ifdef ENABLE_HWSRC_ON_MAIN_STREAM
    afe_block_t *afe_block = &sink->param.audio.AfeBlkControl;
#endif
    BUFFER_INFO *buffer_info = &sink->streamBuffer.BufferInfo;
    hw_current_read_idx = WORD_ALIGN(AFE_GET_REG(AFE_DL1_CUR));
    AUDIO_PARAMETER *runtime = &sink->param.audio;
    uint32_t dl_base_addr = (uint32_t)buffer_info->startaddr[0];

#ifdef ENABLE_HWSRC_ON_MAIN_STREAM
    if (afe_block->u4asrcflag) {
        uint32_t owo, oro;
        dl_base_addr = AFE_GET_REG(ASM_IBUF_SADR);
        hw_current_read_idx = AFE_GET_REG(ASM_CH01_IBUF_RDPNT);
        owo = AFE_GET_REG(ASM_CH01_OBUF_WRPNT) - AFE_GET_REG(ASM_OBUF_SADR);
        oro = AFE_GET_REG(ASM_CH01_OBUF_RDPNT) - AFE_GET_REG(ASM_OBUF_SADR);
        afe_block->u4ReadIdx = oro;
        afe_block->u4WriteIdx = owo;
    } else {
        dl_base_addr = AFE_GET_REG(AFE_DL1_BASE);
        dl_base_addr = AFE_GET_REG(AFE_DL1_BASE);
    }
#endif

    afe_hwsrc_len = AFE_GET_REG(rx_handle->sink_info.i2s_slv_out.afe_hwsrc_len_addr);
    afe_hwsrc_rptr = AFE_GET_REG(rx_handle->sink_info.i2s_slv_out.afe_hwsrc_rptr_addr) - AFE_GET_REG(rx_handle->sink_info.i2s_slv_out.afe_hwsrc_base_addr);
    afe_hwsrc_wptr = AFE_GET_REG(rx_handle->sink_info.i2s_slv_out.afe_hwsrc_wptr_addr) - AFE_GET_REG(rx_handle->sink_info.i2s_slv_out.afe_hwsrc_base_addr);
    hwsrc_out_avail = (afe_hwsrc_wptr + afe_hwsrc_len - afe_hwsrc_rptr) % afe_hwsrc_len;

    if (runtime->irq_exist == false) {
        rx_handle->src_reserved_size = afe_hwsrc_wptr - rx_handle->sink_info.i2s_slv_out.channel_num * rx_handle->sink_info.i2s_slv_out.frame_size;
    }
    if (rx_handle->src_lock_write_flag == true) {
        if (hwsrc_out_avail < (rx_handle->src_reserved_size - 80)) {
            rx_handle->src_lock_write_flag = false;
            /* Need add dummy data for enough prefill size */
            buffer_info->WriteOffset = (buffer_info->WriteOffset + (rx_handle->src_reserved_size - hwsrc_out_avail)) % buffer_info->length;
            set_value_parameter.set_current_offset.pure_agent_with_src = sink->param.audio.mem_handle.pure_agent_with_src;
            set_value_parameter.set_current_offset.memory_select = sink->param.audio.mem_handle.memory_select;
            set_value_parameter.set_current_offset.offset = buffer_info->WriteOffset + (uint32_t)buffer_info->startaddr[0];
            hal_audio_set_value(&set_value_parameter, HAL_AUDIO_SET_SRC_INPUT_CURRENT_OFFSET);
            DSP_MW_LOG_W("[Wireless MIC RX][UL][handle 0x%x] detect clk resume, HWSRC avail expect %d, actual %d, hwsrc in wptr %d", 4, rx_handle, rx_handle->src_reserved_size, hwsrc_out_avail, buffer_info->WriteOffset);
        }
    } else {
        if (hwsrc_out_avail > (rx_handle->src_reserved_size + 80)) {
            rx_handle->src_lock_write_flag = true;
            DSP_MW_LOG_W("[Wireless MIC RX][UL][handle 0x%x] detect clk missing, HWSRC avail expect %d, actual %d", 3, rx_handle, rx_handle->src_reserved_size, hwsrc_out_avail);
        }
    }

    if (runtime->irq_exist == false) {
        runtime->irq_exist = true;
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &first_dl_irq_time);
        if ((sink->scenario_type >= AUDIO_SCENARIO_TYPE_WIRELESS_MIC_RX_UL_LINE_OUT) &&
            (sink->scenario_type <= AUDIO_SCENARIO_TYPE_WIRELESS_MIC_RX_UL_I2S_SLV_OUT_0)) {
            //wireless_mic_rx_playen_disable(sink);
            afe_dl_playen_release(sink);
            hal_memory_set_enable(HAL_AUDIO_AGENT_MEMORY_DL12, true);
            hal_memory_set_palyen(HAL_AUDIO_AGENT_MEMORY_DL12, false);
        }
        if ((rx_handle->source_info.bt_in.frame_interval == 1000) && (rx_handle->stream_status == WIRELESS_MIC_RX_STREAM_INIT)) {
            rx_handle->first_packet_status = WIRELESS_MIC_RX_UL_FIRST_PACKET_PLAYED;
            rx_handle->stream_status = WIRELESS_MIC_RX_STREAM_RUNNING;
            rx_handle->is_play_en_trigger = false;
        }
    }

    buffer_info->ReadOffset = hw_current_read_idx - dl_base_addr;
    if (buffer_info->ReadOffset != buffer_info->WriteOffset) {
        buffer_info->bBufferIsFull = FALSE;
    }

    if (pStream && pStream->streamingStatus == STREAMING_END) {
        if (Audio_setting->Audio_sink.Zero_Padding_Cnt > 0) {
            Audio_setting->Audio_sink.Zero_Padding_Cnt--;
            // DSP_MW_LOG_I("DL zero pad %d", 1, Audio_setting->Audio_sink.Zero_Padding_Cnt);
        }
    }

    //AudioCheckTransformHandle(sink->transform);

    sink->transform->Handler(source, sink);
    xTaskResumeFromISR(sink->taskid);
    portYIELD_FROM_ISR(pdTRUE);

#if 0
    DSP_MW_LOG_I("[Wireless MIC RX][UL][handle 0x%x] DSP afe dl1 interrupt trigger, curr BT: 0x%x 0x%x, fetch BT: 0x%x 0x%x, hwsrc in: rptr 0x%x wptr 0x%x, hwsrc out: rptr 0x%x wptr 0x%x, tracking freq: %d\r\n", 10,
                    rx_handle,
                    bt_clk & 0xFFFFFFFC, bt_phase + (bt_clk & 0x3) * 625,
                    rx_handle->source_info.bt_in.fetch_anchor,
                    rx_handle->source_info.bt_in.fetch_anchor_phase,
                    buffer_info->ReadOffset, buffer_info->WriteOffset,
                    afe_hwsrc_rptr, afe_hwsrc_wptr,
                    0x94C5F000 / AFE_READ(ASM_FREQUENCY_2));
#endif
}
#endif
ATTR_TEXT_IN_IRAM void afe_dl1_interrupt_handler(void)
{
    uint32_t pre_offset, isr_interval;
    uint32_t hw_current_read_idx = 0;
    SINK sink = Sink_blks[SINK_TYPE_AUDIO];
    SOURCE source;
    DSP_STREAMING_PARA_PTR pStream;
    BTCLK bt_clk;
    BTPHASE bt_phase;
    bool empty = true, overflow_check_flag = false;
    bool empty_WriteOffset;
    bool empty_eSCO = false;
    uint32_t gpt_cnt;

    if ((sink == NULL) || (sink->transform == NULL) || (sink->transform->source == NULL)) {
        return;
    }

#if defined(AIR_WIRELESS_MIC_RX_ENABLE)
    if ((sink->scenario_type >= AUDIO_SCENARIO_TYPE_WIRELESS_MIC_RX_UL_LINE_OUT) &&
        (sink->scenario_type <= AUDIO_SCENARIO_TYPE_WIRELESS_MIC_RX_UL_I2S_SLV_OUT_0)) {
        afe_dl1_wireless_mic_rx_interrupt_handler();
        return;
    }
#endif

    source = sink->transform->source;

    pStream = DSP_Streaming_Get(source, sink);

    MCE_GetBtClk(&bt_clk,&bt_phase, BT_CLK_Offset);
    #ifdef AIR_DCHS_MODE_ENABLE
    if(dchs_get_device_mode() == DCHS_MODE_LEFT){
        MCE_GetBtClk(&bt_clk,&bt_phase, DCHS_CLK_Offset);
    }
    #endif

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_cnt);
    Clock_Skew_Check_Isr_Status_From_SrcSnk(source, sink, bt_clk, bt_phase);

    if (Clock_Skew_ECDC_Is_Enable(source, sink)){
        Clock_Skew_Isr_Time_Update(source, sink, gpt_cnt, sink->param.audio.count);
    }

#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
    if ((sink->scenario_type >= AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_UL_LINE_OUT) && (sink->scenario_type <= AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_UL_I2S_SLV_OUT_0))
    {
        ull_audio_v2_dongle_ul_handle_t *dongle_handle = (ull_audio_v2_dongle_ul_handle_t *)(source->param.bt_common.scenario_param.dongle_handle);
        if (ull_audio_v2_dongle_ul_fetch_time_is_arrived(dongle_handle, bt_clk))
        {
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &(dongle_handle->ccni_in_gpt_count));
            dongle_handle->ccni_in_bt_count = bt_clk;
        }
    }
#endif /* defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE) */

#ifdef ENABLE_HWSRC_ON_MAIN_STREAM
    afe_block_t *afe_block = &sink->param.audio.AfeBlkControl;
    afe_src_configuration_t src_configuration;
    memset(&src_configuration, 0, sizeof(afe_src_configuration_t));
    src_configuration.id = AFE_MEM_ASRC_1;
#endif
    BUFFER_INFO *buffer_info = &sink->streamBuffer.BufferInfo;
    hw_current_read_idx = WORD_ALIGN(AFE_GET_REG(AFE_DL1_CUR));
    AUDIO_PARAMETER *runtime = &sink->param.audio;
    int16_t cp_samples = 0;
    uint32_t dl_base_addr = (uint32_t)buffer_info->startaddr[0];

#ifdef ENABLE_HWSRC_ON_MAIN_STREAM
    uint32_t src_out_data = 0;
    if (afe_block->u4asrcflag) {
        dl_base_addr = AFE_GET_REG(ASM_IBUF_SADR);
        hw_current_read_idx = AFE_GET_REG(ASM_CH01_IBUF_RDPNT);
        uint32_t ibuf_addr, iwo, iro, obuf_addr, owo, oro, osize, isize;
        ibuf_addr = AFE_GET_REG(ASM_IBUF_SADR);
        obuf_addr = AFE_GET_REG(ASM_OBUF_SADR);
        iro = AFE_GET_REG(ASM_CH01_IBUF_RDPNT) - AFE_GET_REG(ASM_IBUF_SADR);
        iwo = AFE_GET_REG(ASM_CH01_IBUF_WRPNT) - AFE_GET_REG(ASM_IBUF_SADR);
        owo = AFE_GET_REG(ASM_CH01_OBUF_WRPNT) - AFE_GET_REG(ASM_OBUF_SADR);
        oro = AFE_GET_REG(ASM_CH01_OBUF_RDPNT) - AFE_GET_REG(ASM_OBUF_SADR);
        osize = AFE_GET_REG(ASM_OBUF_SIZE);
        isize = AFE_GET_REG(ASM_IBUF_SIZE);
        afe_block->u4ReadIdx = oro;
        afe_block->u4WriteIdx = owo;
        src_out_data = (owo >= oro) ? (owo - oro) : (owo + osize - oro);
#ifdef MTK_BT_A2DP_ENABLE
        if ((source->type == SOURCE_TYPE_A2DP) && (g_a2dp_hwsrc_ng_flag == false)){
            if ((iro != iwo) && (iro == g_pre_iro)){
                g_hwsrc_halt_check++;
                if (g_hwsrc_halt_check == 8){
                    g_a2dp_hwsrc_ng_flag = true;
                    g_hwsrc_halt_check = 0;
                    DSP_MW_LOG_E("HWSRC halt trigger re-sync", 0);
                    Au_DL_send_reinit_request(MSG2_DSP2CN4_REINIT_AFE_ABNORMAL, TRUE);
                }
            }else{
                g_pre_iro = iro;
                g_hwsrc_halt_check = 0;
            }
        }
#endif
#if 0
        if ((source->type == SOURCE_TYPE_A2DP) && (g_a2dp_hwsrc_ng_flag == false)){
            if ((iro != iwo) && (iro == pre_iro)){
                hwsrc_hal_check++;
                if (hwsrc_hal_check == 8){
                    g_a2dp_hwsrc_ng_flag = true;
                    hwsrc_hal_check = 0;
                    DSP_MW_LOG_W("TEST1 hwsrc1 iro %d iwo %d oro %d owo %d top con0:0x%x 1:0x%x 2:0x%x 3:0x%x mon0:0x%x mon1:0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x", 16,
                    AFE_READ(ASM_CH01_IBUF_RDPNT) - AFE_READ(ASM_IBUF_SADR),
                    AFE_READ(ASM_CH01_IBUF_WRPNT) - AFE_READ(ASM_IBUF_SADR),
                    AFE_READ(ASM_CH01_OBUF_RDPNT) - AFE_READ(ASM_OBUF_SADR),
                    AFE_READ(ASM_CH01_OBUF_WRPNT) - AFE_READ(ASM_OBUF_SADR),
                    AFE_READ(MEM_ASRC_TOP_CON0),
                    AFE_READ(MEM_ASRC_TOP_CON1),
                    AFE_READ(MEM_ASRC_TOP_CON2),
                    AFE_READ(MEM_ASRC_TOP_CON3),
                    AFE_READ(MEM_ASRC_TOP_MON0),
                    AFE_READ(MEM_ASRC_TOP_MON1),
                    AFE_READ(ASM_GEN_CONF),
                    AFE_READ(ASM_IER),
                    AFE_READ(ASM_IFR),
                    AFE_READ(ASM_CH01_CNFG),
                    AFE_READ(ASM_FREQUENCY_0),
                    AFE_READ(ASM_FREQUENCY_1)
                );
                DSP_MW_LOG_W("TEST2 hwsrc1 top_con1:0x%x 0x%x 0x%x ICNT0:0x%x OCNT0:0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x CLKRG:0x%x,PDN_TOP:0x%x,DVFS_RG:0x%x", 15,
                    AFE_READ(AUDIO_TOP_CON1),
                    AFE_READ(ASM_FREQUENCY_2),
                    AFE_READ(ASM_FREQUENCY_3),
                    AFE_READ(ASM_IBUF_INTR_CNT0),
                    AFE_READ(ASM_OBUF_INTR_CNT0),
                    AFE_READ(ASM_BAK_REG),
                    AFE_READ(ASM_FREQ_CALI_CTRL),
                    AFE_READ(ASM_FREQ_CALI_CYC),
                    AFE_READ(ASM_PRD_CALI_RESULT),
                    AFE_READ(ASM_FREQ_CALI_RESULT),
                    AFE_READ(ASM_IBUF_SAMPLE_CNT),
                    AFE_READ(ASM_OBUF_SAMPLE_CNT),
                    AFE_READ(0xA2020238),
                    AFE_READ(0xA2030B60),
                    AFE_READ(0xA2020230)
                );
                DSP_MW_LOG_W("TEST3 hwsrc1 SMCONF:0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x ibufmon0:0x%x ibufmon1:0x%x 0x%x 0x%x obufmon0:0x%x,mon1:0x%x,isadr:0x%x,size:0x%x,0x%x,0x%x", 17,
                    AFE_READ(ASM_SMPCNT_CONF),
                    AFE_READ(ASM_SMPCNT_WRAP_VAL),
                    AFE_READ(ASM_SMPCNT_IRQ_VAL),
                    AFE_READ(ASM_SMPCNT1_LATCH),
                    AFE_READ(ASM_SMPCNT2_LATCH),
                    AFE_READ(ASM_CALI_DENOMINATOR),
                    AFE_READ(ASM_MAX_OUT_PER_IN0),
                    AFE_READ(ASM_IN_BUF_MON0),
                    AFE_READ(ASM_IN_BUF_MON1),
                    AFE_READ(ASM_IIR_CRAM_ADDR),
                    AFE_READ(ASM_IIR_CRAM_DATA),
                    AFE_READ(ASM_OUT_BUF_MON0),
                    AFE_READ(ASM_OUT_BUF_MON1),
                    AFE_READ(ASM_IBUF_SADR),
                    AFE_READ(ASM_IBUF_SIZE),
                    AFE_READ(ASM_OBUF_SADR),
                    AFE_READ(ASM_OBUF_SIZE)
                );
                    DSP_MW_LOG_E("HWSRC halt trigger re-sync", 0);
                    assert(0);
                }
            }else{
                pre_iro = iro;
                hwsrc_hal_check = 0;
            }
        }
#endif
        //DSP_MW_LOG_I("[afe_irq] asrc in,wo=%d,ro=%d,len=%d,out,wo=%d,ro=%d,len=%d\r\n", 6,iwo ,iro,isize,owo,oro,osize);
    } else {
        dl_base_addr = AFE_GET_REG(AFE_DL1_BASE);
        hw_current_read_idx = AFE_GET_REG(AFE_DL1_CUR);
        uint32_t owo, oro, osize;
        dl_base_addr = AFE_GET_REG(AFE_DL1_BASE);
        hw_current_read_idx = AFE_GET_REG(AFE_DL1_CUR);
        oro = AFE_GET_REG(AFE_DL1_CUR) - AFE_GET_REG(AFE_DL1_BASE);
        owo = buffer_info->WriteOffset;
        osize = AFE_GET_REG(AFE_DL1_END) - AFE_GET_REG(AFE_DL1_BASE);
        //DSP_MW_LOG_I("dl out,wo=%d,ro=%d,len=%d\r\n", 3,owo,oro,osize);
    }
#endif
#ifdef AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE
    // trigger the streaming
    gaming_mode_dongle_ul_handle_t *dongle_handle = gaming_mode_dongle_query_ul_handle_by_scenario_type(sink->scenario_type);
    if (dongle_handle != NULL) {
        if (dongle_handle->stream_status != GAMING_MODE_UL_STREAM_DEINIT) {
            uint32_t saved_mask;
            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            /* set fetch flag to trigger stream flow */
            dongle_handle->fetch_flag = 1;
            hal_nvic_restore_interrupt_mask(saved_mask);
            // check stream number
            extern gaming_mode_dongle_ul_handle_t *gaming_mode_dongle_first_ul_handle;
            extern gaming_mode_dongle_ul_handle_t *gaming_mode_dongle_query_ul_handle_by_scenario_type(audio_scenario_type_t type);
            gaming_mode_dongle_ul_handle_t *usb_dongle = gaming_mode_dongle_query_ul_handle_by_scenario_type(AUDIO_SCENARIO_TYPE_GAMING_MODE_VOICE_DONGLE_USB_OUT);
            uint32_t cnt = gaming_mode_dongle_first_ul_handle->total_number;
            if (usb_dongle != NULL) {
                if ((cnt > 1) && (usb_dongle->stream_status >= GAMING_MODE_UL_STREAM_INIT)/*&& (usb_dongle->play_en_status)*/) {
                    // usb out is on!
                    dongle_handle->bypass_source = true;
                    if (runtime->irq_exist == false) {
                        /* we should change the status to avoid that line out can't enter start status */
                        dongle_handle->stream_status = GAMING_MODE_UL_STREAM_RUNNING;
                    }
                } else {
                    if (cnt > 1) {
                        DSP_MW_LOG_I("Clear buffer st %d", 1, usb_dongle->stream_status);
                    }
                    dongle_handle->bypass_source = false;
                }
            }
            /* Handler the stream */
            AudioCheckTransformHandle(sink->transform);
        }
    }

#endif /* AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE */

#ifdef AIR_BT_CODEC_BLE_ENABLED
        if ((source->type == SOURCE_TYPE_N9BLE) && (source->param.n9ble.context_type == BLE_CONTENT_TYPE_ULL_BLE)) {
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, (uint32_t *)&ble_dl_gpt_start);
        }
#endif

    /*Mce play en check*/
    if (runtime->irq_exist == false) {
        runtime->irq_exist = true;

#ifdef MTK_BT_A2DP_ENABLE
        g_a2dp_hwsrc_ng_flag = false;
        g_hwsrc_halt_check = 0;
#endif

        #if defined(MTK_ANC_ENABLE) && defined(AIR_ANC_SCENARIO_CONTROL_GAIN_ENABLE)
        //anc call mode on
        if ((sink->scenario_type == AUDIO_SCENARIO_TYPE_BLE_DL)||(sink->scenario_type == AUDIO_SCENARIO_TYPE_HFP_DL)) {
            dsp_anc_apply_ramp_gain_by_audio_scenario(DSP_ANC_CONTROL_AUDIO_SCENARIO_TYPE_CALL, true, NULL);
        }
        #endif

        U32 first_dl_irq_time;
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &first_dl_irq_time);
#if defined(AIR_WIRELESS_MIC_RX_ENABLE)
        DSP_MW_LOG_I("DSP afe dl1 interrupt exist, bt_clk 0x%x, bt_intra:0x%x, gpt_time:%d", 3, bt_clk, bt_phase, first_dl_irq_time);
#else
        DSP_MW_LOG_I("DSP afe dl1 interrupt exist, bt_clk 0x%x, bt_intra:0x%x, gpt_time:%d", 3, rBb->rClkCtl.rNativeClock, rBb->rClkCtl.rNativePhase, first_dl_irq_time);
#endif
        dl_irq_cnt = 1;
#ifdef ENABLE_HWSRC_ON_MAIN_STREAM
        if (afe_block->u4asrcflag) {
#ifdef HWSRC_UNDERRUN_DETECT
            hwsrc_out_remain = 0;
#endif

        }
#endif
        if (source->type == SOURCE_TYPE_N9SCO) {
            if (SourceSize(source) != 0) {
                DSP_MW_LOG_I("eSCO DL audio irq first Wo:%d Ro:%d", 2, source->streamBuffer.AVMBufferInfo.WriteIndex, source->streamBuffer.AVMBufferInfo.ReadIndex);
            } else {
                DSP_MW_LOG_I("eSCO First IRQ meet size 0 Wo:%d Ro:%d", 2, source->streamBuffer.AVMBufferInfo.WriteIndex, source->streamBuffer.AVMBufferInfo.ReadIndex);
            }
        }
#ifdef AIR_BT_CODEC_BLE_ENABLED
        if (source->type == SOURCE_TYPE_N9BLE) {
            if (SourceSize(source) != 0) {
                DSP_MW_LOG_I("BLE DL audio irq first Wo:%d Ro:%d", 2,source->streamBuffer.ShareBufferInfo.write_offset,source->streamBuffer.ShareBufferInfo.read_offset);
            } else {
                DSP_MW_LOG_I("BLE First IRQ meet size 0 Wo:%d Ro:%d", 2,source->streamBuffer.ShareBufferInfo.write_offset,source->streamBuffer.ShareBufferInfo.read_offset);
            }
        }
#endif
        DSP_MW_LOG_I("DSP afe dl1 interrupt first: scenario type[%d] size%d Wo:%d Ro:%d bt_clk 0x%x bt_intra:0x%x gpt_time:%d", 7,
                sink->scenario_type,
                SourceSize(source),
                source->streamBuffer.AVMBufferInfo.WriteIndex,
                source->streamBuffer.AVMBufferInfo.ReadIndex,
                rBb->rClkCtl.rNativeClock,
                rBb->rClkCtl.rNativePhase,
                first_dl_irq_time
                );
#if defined(AIR_WIRELESS_MIC_RX_ENABLE)
        if ((sink->scenario_type >= AUDIO_SCENARIO_TYPE_WIRELESS_MIC_RX_UL_LINE_OUT) && (sink->scenario_type <= AUDIO_SCENARIO_TYPE_WIRELESS_MIC_RX_UL_I2S_SLV_OUT_0))
        {
            wireless_mic_rx_playen_disable(sink);
        }
#endif /* AIR_WIRELESS_MIC_RX_ENABLE */
    } else if (runtime->AfeBlkControl.u4awsflag == true) {
        #ifndef AIR_MIXER_STREAM_ENABLE
        uint32_t sync_reg_mon1 = afe_get_bt_sync_monitor(AUDIO_DIGITAL_BLOCK_MEM_DL1);
        uint32_t sync_reg_mon2 = afe_get_bt_sync_monitor_state(AUDIO_DIGITAL_BLOCK_MEM_DL1);
        if ((sync_reg_mon1 == 0) || (sync_reg_mon2 == 0)) {
            DSP_MW_LOG_I("DSP afe BT sync monitor by dl1 0x%x wait cnt: %d Wo: %d Ro: %d Bf: %d", 5, sync_reg_mon1, runtime->afe_wait_play_en_cnt, buffer_info->WriteOffset, buffer_info->ReadOffset, buffer_info->bBufferIsFull);
            if ((runtime->afe_wait_play_en_cnt != PLAY_EN_REINIT_DONE_MAGIC_NUM) && (runtime->afe_wait_play_en_cnt != PLAY_EN_TRIGGER_REINIT_MAGIC_NUM)) {
                runtime->afe_wait_play_en_cnt++;
                if ((runtime->afe_wait_play_en_cnt * runtime->period) > PLAY_EN_DELAY_TOLERENCE) {
                    runtime->afe_wait_play_en_cnt = PLAY_EN_TRIGGER_REINIT_MAGIC_NUM;
                    buffer_info->bBufferIsFull = FALSE;
                }
            }
        }
        #endif
    }
    dl_irq_cnt++;

#if 0//modify for ab1568
    if (afe_get_memory_path_enable(AUDIO_DIGITAL_BLOCK_MEM_DL1))
#endif
    {
        /* Update Clk Skew: clk information */
        #ifdef AIR_MIXER_STREAM_ENABLE
        if (afe_get_bt_sync_monitor_state(AUDIO_DIGITAL_BLOCK_MEM_DL1))
        #endif
        {
            Clock_Skew_Offset_Update(BT_CLK_Offset, source, sink);
            #ifdef AIR_DCHS_MODE_ENABLE
            if(dchs_get_device_mode() == DCHS_MODE_LEFT){
                Clock_Skew_Offset_Update(DCHS_CLK_Offset, source, sink);
            }
            #endif
        }

        if (sink->scenario_type != AUDIO_SCENARIO_TYPE_WIRELESS_MIC_RX_UL_I2S_SLV_OUT_0) {
            /* For Downlink Clk Skew IRQ period control */
            cp_samples = Clock_Skew_Check_Status_From_SrcSnk(source, sink);

#if 0//modify for ab1568
            afe_update_audio_irq_cnt(afe_irq_request_number(AUDIO_DIGITAL_BLOCK_MEM_DL1),
                                     runtime->count + cp_samples);
#else
            hal_audio_memory_irq_period_parameter_t irq_period;
            irq_period.memory_select = HAL_AUDIO_MEMORY_DL_DL1;
            irq_period.rate = runtime->rate;

#ifdef ENABLE_HWSRC_ON_MAIN_STREAM
            if (runtime->rate != runtime->src_rate && runtime->irq_compen_flag && !(Clock_Skew_ECDC_Is_Enable(source, sink))) {
                cp_samples += hal_audio_get_irq_compen_samples(sink);
            }
#endif

            irq_period.irq_counter = (uint32_t)((int32_t)runtime->count + (int32_t)cp_samples);

            /*if (cp_samples != 0) {
                DSP_MW_LOG_I("[ClkSkew] DL irq_cnt:%d, cp:%d, fs_in:%d, fs_out:%d, cnt:%d", 5, irq_period.irq_counter, cp_samples, runtime->src_rate, runtime->rate, runtime->count);
            }*/
            hal_audio_control_set_value((hal_audio_set_value_parameter_t *)&irq_period, HAL_AUDIO_SET_MEMORY_IRQ_PERIOD);
            Clock_Skew_Samples_Cnt_Update(source, sink, (U16)irq_period.irq_counter);
#endif
        }

        if ((hw_current_read_idx == 0) || (chk_dl1_cur(hw_current_read_idx) == false)) { //should chk setting if =0
#ifdef ENABLE_HWSRC_ON_MAIN_STREAM
            hw_current_read_idx = WORD_ALIGN(dl_base_addr);
#else
            hw_current_read_idx = dl_base_addr;
#endif
        }
        pre_offset = buffer_info->ReadOffset;

        buffer_info->ReadOffset = hw_current_read_idx - dl_base_addr;

        isr_interval = (pre_offset <= buffer_info->ReadOffset)
                       ? (buffer_info->ReadOffset - pre_offset)
                       : (buffer_info->length + buffer_info->ReadOffset - pre_offset);
#if 0
        if (dl_base_addr != NULL) { //Prevent to access null pointer when the last isr is executed after HW is turned off and pointer is cleared
            /*Clear up last time used memory */
            if ( !(source->type == SOURCE_TYPE_N9BLE && source->param.n9ble.context_type == BLE_CONTENT_TYPE_ULL_BLE)
                && !(source->type == SOURCE_TYPE_A2DP && source->param.n9_a2dp.codec_info.codec_cap.type == BT_A2DP_CODEC_AIRO_CELT))
            {
                if (buffer_info->ReadOffset >= pre_offset) {
                    memset((void *)(dl_base_addr + pre_offset), 0, buffer_info->ReadOffset - pre_offset);
                } else {
                    memset((void *)(dl_base_addr + pre_offset), 0, buffer_info->length - pre_offset);
                    memset((void *)dl_base_addr, 0, buffer_info->ReadOffset);
                }
            }
        }
#endif
        if (buffer_info->ReadOffset != buffer_info->WriteOffset) {
            buffer_info->bBufferIsFull = FALSE;
        }

#ifdef AIR_BT_CODEC_BLE_ENABLED
        if (source->type == SOURCE_TYPE_N9BLE) {
            DSP_CALLBACK_PTR callback_ptr;
            callback_ptr = DSP_Callback_Get(source, sink);

            if (callback_ptr->Status != CALLBACK_SUSPEND) {
                U32 drop_amount, flush_amount;
                drop_amount = (source->param.n9ble.dual_cis_status != DUAL_CIS_DISABLED) ? g_ble_abr_length * source->param.n9ble.process_number * 2 : g_ble_abr_length * source->param.n9ble.process_number;
                drop_amount += source->param.n9ble.plc_state_len;
                flush_amount = ((U32)sink->param.audio.src_rate * (source->param.n9ble.frame_interval * sink->param.audio.format_bytes / 1000)  / 1000);

                #ifdef AIR_CELT_DEC_V2_ENABLE
                if(source->param.n9ble.codec_type == BT_BLE_CODEC_VENDOR) {
                    drop_amount = g_ble_abr_length + 2*source->param.n9ble.plc_state_len;
                }
                #endif

                if(source->param.n9ble.predict_packet_cnt[0]) {
                    ++source->param.n9ble.predict_packet_cnt[0];
                }

                if(source->param.n9ble.predict_packet_cnt[1]) {
                    ++source->param.n9ble.predict_packet_cnt[1];
                }

                SourceDrop(source, drop_amount);
                SinkFlush(sink, flush_amount);
                DSP_MW_LOG_I("Callback Busy : %d, Source drop and move Sink WO, dl_irq_cnt:%d, flush_amount %d, drop_amount %d", 4, callback_ptr->Status, dl_irq_cnt, flush_amount, drop_amount);
            }

#ifdef ENABLE_HWSRC_ON_MAIN_STREAM
#if (AFE_REGISTER_ASRC_IRQ)
            if (Clock_Skew_HWSRC_Is_Enable(source,sink)) {
                if (afe_get_asrc_irq_is_enabled(AFE_MEM_ASRC_1, ASM_IER_IBUF_EMPTY_INTEN_MASK) == false) { //modify for clock skew
                    // afe_set_asrc_irq_enable(AFE_MEM_ASRC_1, false);
                    hal_src_set_irq_enable(&src_configuration, true);
                    //DSP_MW_LOG_I("asrc afe_dl1_interrupt_handler asrc_irq_is_enabled %d",1,afe_get_asrc_irq_is_enabled(AFE_MEM_ASRC_1, ASM_IER_IBUF_EMPTY_INTEN_MASK));
                }
            }
#endif /*AFE_REGISTER_ASRC_IRQ*/
#endif /*ENABLE_HWSRC_ON_MAIN_STREAM*/

            //#ifndef MTK_BT_HFP_FORWARDER_ENABLE
            if (SourceSize(source) == 0) {
                DSP_MW_LOG_I("BLE DL audio irq no data in Wo:%d Ro:%d", 2,source->streamBuffer.ShareBufferInfo.write_offset,source->streamBuffer.ShareBufferInfo.read_offset);
                SourceConfigure(source, SCO_SOURCE_WO_ADVANCE, 1); // force stream to process even there has no frame in avm buffer
            }
            //#endif
        }
#endif/*AIR_BT_CODEC_BLE_ENABLED*/
        if (source->type == SOURCE_TYPE_N9SCO) {
            DSP_CALLBACK_PTR callback_ptr;

            callback_ptr = DSP_Callback_Get(source, sink);
#if DL_TRIGGER_UL
            if (source->param.n9sco.dl_enable_ul == TRUE) {
                hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &(source->param.n9sco.ul_play_gpt));
                source->param.n9sco.dl_enable_ul = FALSE;
            }
#endif
#if 0
            if (callback_ptr->Status != CALLBACK_SUSPEND) {
                //DSP_MW_LOG_I("Callback Busy : %d", 1,callback_ptr->Status);
#if DL_TRIGGER_UL
                source->param.n9sco.dl_enable_ul = TRUE;
                SourceDrop(source, source->param.n9sco.process_data_length);
                source->param.n9sco.dl_enable_ul = FALSE;
#else
                SourceDrop(source, source->param.n9sco.process_data_length);
#endif
                uint32_t prewo = buffer_info->WriteOffset;
                buffer_info->WriteOffset = (buffer_info->WriteOffset + (sink->param.audio.channel_num * sink->param.audio.src_rate * sink->param.audio.period * sink->param.audio.format_bytes / 1000)) % buffer_info->length;
                DSP_MW_LOG_I("Callback Busy : %d, Source drop and move Sink WO:%d, prewo:%d, buflen:%d, dl_irq_cnt:%d", 5, callback_ptr->Status, buffer_info->WriteOffset, prewo, buffer_info->length, dl_irq_cnt);
            }
#endif

#ifdef ENABLE_HWSRC_ON_MAIN_STREAM
//#if (AFE_REGISTER_ASRC_IRQ)
            if (Clock_Skew_HWSRC_Is_Enable(source,sink)) {
                if (afe_get_asrc_irq_is_enabled(AFE_MEM_ASRC_1, ASM_IER_IBUF_EMPTY_INTEN_MASK) == false) { //modify for clock skew
                    // afe_set_asrc_irq_enable(AFE_MEM_ASRC_1, false);
                    hal_src_set_irq_enable(&src_configuration, true);
                    //DSP_MW_LOG_I("asrc afe_dl1_interrupt_handler asrc_irq_is_enabled %d",1,afe_get_asrc_irq_is_enabled(AFE_MEM_ASRC_1, ASM_IER_IBUF_EMPTY_INTEN_MASK));
                }
            }
//#endif /*AFE_REGISTER_ASRC_IRQ*/
#endif /*ENABLE_HWSRC_ON_MAIN_STREAM*/
            //#ifndef MTK_BT_HFP_FORWARDER_ENABLE
            if (SourceSize(source) == 0) {
                empty_eSCO = TRUE;
                /*DSP_MW_LOG_I("eSCO DL audio irq no data in Wo:%d Ro:%d", 2, source->streamBuffer.AVMBufferInfo.WriteIndex, source->streamBuffer.AVMBufferInfo.ReadIndex);*/
                SourceConfigure(source, SCO_SOURCE_WO_ADVANCE, 2); // add 2 frame in advance for HFP MCE
            } else {
                //DSP_MW_LOG_I("eSCO DL audio irq have data in Wo:%d Ro:%d", 2,source->streamBuffer.AVMBufferInfo.WriteIndex,source->streamBuffer.AVMBufferInfo.ReadIndex);

            }
            //#endif
        }
#ifdef ENABLE_HWSRC_ON_MAIN_STREAM
#ifdef HWSRC_UNDERRUN_DETECT
#ifdef AIR_BT_CODEC_BLE_ENABLED
        if (((source->type == SOURCE_TYPE_A2DP) || (source->type == SOURCE_TYPE_N9BLE)) && (afe_block->u4asrcflag == TRUE) && (Clock_Skew_HWSRC_Is_Enable(source,sink)))
#else
        if ((source->type == SOURCE_TYPE_A2DP) && (afe_block->u4asrcflag == TRUE) && (ClkSkewMode_g == CLK_SKEW_V2))
#endif
        {
            U16 channelnum_shift = (sink->param.audio.mem_handle.with_mono_channel == FALSE) ? 1 : 0;
            src_out_data = src_out_data >> channelnum_shift;
            hwsrc_out_remain = (hwsrc_out_remain >= src_out_data) ? (hwsrc_out_remain - src_out_data) : 0; // Calculate if there is remaining HWSRC out data been comsumed in this round
            if ((((isr_interval >> channelnum_shift) + (clock_skew_asrc_get_input_sample_size())*sink->param.audio.format_bytes) / ((runtime->src_rate / 1000)) + (hwsrc_out_remain / (afe_block->u4asrcrate / 1000))) < (sink->param.audio.period * sink->param.audio.format_bytes)) { // Check if SRC convert enough amount during this ISR interval
                DSP_MW_LOG_W("SRAM Empty with UNDERRUN_DETECT, remain_data:%d, isr_interval:%d, period_num:%d, last_remain:%d iro:%d", 5, src_out_data, (isr_interval >> channelnum_shift), (sink->param.audio.period * sink->param.audio.format_bytes), (hwsrc_out_remain / (afe_block->u4asrcrate / 1000)), buffer_info->ReadOffset);
                runtime->afe_wait_play_en_cnt = PLAY_EN_TRIGGER_REINIT_MAGIC_NUM;
            }
            hwsrc_out_remain = src_out_data;
        }
#endif
#endif

#ifdef ENABLE_HWSRC_ON_MAIN_STREAM
        if ((afe_offset_overflow_check(pre_offset, buffer_info, true))
            || (((buffer_info->WriteOffset + buffer_info->length - buffer_info->ReadOffset) % buffer_info->length < 32) && (afe_block->u4asrcflag)))
#else
        if (afe_offset_overflow_check(pre_offset, buffer_info, true)) //Sram empty
#endif
        {

            overflow_check_flag = true;
#ifdef ENABLE_HWSRC_ON_MAIN_STREAM
            if (afe_block->u4asrcflag) {
                uint32_t src_out_size, src_out_read, src_out_write, remain_data;
                src_out_write = AFE_READ(ASM_CH01_OBUF_WRPNT);
                src_out_read = AFE_READ(ASM_CH01_OBUF_RDPNT);
                src_out_size = AFE_READ(ASM_OBUF_SIZE);
                remain_data = (src_out_write > src_out_read) ? src_out_write - src_out_read : src_out_write + src_out_size - src_out_read;
                remain_data = (sink->param.audio.channel_num >= 2) ? remain_data >> 1 : remain_data;
                if (remain_data > ((afe_block->u4asrcrate * sink->param.audio.period * sink->param.audio.format_bytes / 2)) / 1000) {
                    empty = false;
                }
#ifdef AIR_BT_CODEC_BLE_ENABLED
                if (source->type == SOURCE_TYPE_N9BLE && ((source->param.n9ble.context_type == BLE_CONTENT_TYPE_ULL_BLE)||(source->param.n9ble.context_type == BLE_CONTEXT_GAME))) {
                    empty = false;
                    //DSP_MW_LOG_W("[BLE] ULL Skip SRAM Empty protect", 0);
                }
#endif
#if defined(AIR_WIRELESS_MIC_RX_ENABLE)
                /* NOTE: HWSRC EMPTY check is odd, it is a wordaround */
                if ((sink->scenario_type >= AUDIO_SCENARIO_TYPE_WIRELESS_MIC_RX_UL_LINE_OUT) && (sink->scenario_type <= AUDIO_SCENARIO_TYPE_WIRELESS_MIC_RX_UL_I2S_SLV_OUT_0))
                {
                    empty = false;
                }
#endif /* AIR_WIRELESS_MIC_RX_ENABLE */

            }
#endif
            if (empty) {
                empty_WriteOffset = buffer_info->WriteOffset;
                // DSP_MW_LOG_W("DL1 ,SRAM Empty play en:%d pR:%d R:%d W:%d", 4, isr_interval, pre_offset, buffer_info->ReadOffset, buffer_info->WriteOffset);
#ifdef ENABLE_HWSRC_ON_MAIN_STREAM
                U16 pre_write_offset = buffer_info->WriteOffset;

                if (afe_block->u4asrcflag) {
                    buffer_info->WriteOffset = (buffer_info->ReadOffset + buffer_info->length / 2) % buffer_info->length;
                } else {
                    if (pre_offset < buffer_info->ReadOffset) {
                        buffer_info->WriteOffset = (buffer_info->ReadOffset * 2 - pre_offset) % buffer_info->length;
                    } else {
                        buffer_info->WriteOffset = (buffer_info->ReadOffset * 2 + (buffer_info->length - pre_offset)) % buffer_info->length;
                    }
                }

//#if (AFE_REGISTER_ASRC_IRQ)
#ifdef ENABLE_HWSRC_CLKSKEW
                if (Clock_Skew_HWSRC_Is_Enable(source,sink)) {
                    if (afe_get_asrc_irq_is_enabled(AFE_MEM_ASRC_1, ASM_IER_IBUF_EMPTY_INTEN_MASK) == false) { //modify for clock skew
                    // afe_set_asrc_irq_enable(AFE_MEM_ASRC_1, false);
                    hal_src_set_irq_enable(&src_configuration, true);
                        //DSP_MW_LOG_W("asrc afe_dl1_interrupt_handler asrc_irq_is_enabled %d",1,afe_get_asrc_irq_is_enabled(AFE_MEM_ASRC_1, ASM_IER_IBUF_EMPTY_INTEN_MASK));
                    }
                }
#endif /*ENABLE_HWSRC_CLKSKEW*/
//#endif /*AFE_REGISTER_ASRC_IRQ*/
#endif /*ENABLE_HWSRC_ON_MAIN_STREAM*/

                if ((source->type == SOURCE_TYPE_A2DP) && (source->param.n9_a2dp.mce_flag)) {
                    uint32_t empty_fill_size = 0;
#ifdef ENABLE_HWSRC_ON_MAIN_STREAM
                    empty_fill_size = (buffer_info->WriteOffset >= pre_write_offset) ? (buffer_info->WriteOffset - pre_write_offset) : (buffer_info->length - pre_write_offset + buffer_info->WriteOffset);
#else
                    empty_fill_size = 2 * isr_interval + ((buffer_info->ReadOffset >= buffer_info->WriteOffset) ?
                                                                    (buffer_info->ReadOffset - buffer_info->WriteOffset) :
                                                                    (buffer_info->length - buffer_info->WriteOffset + buffer_info->ReadOffset));
                    buffer_info->WriteOffset = (buffer_info->ReadOffset + 2 * isr_interval) % buffer_info->length;
#endif
                    DSP_MW_LOG_W("add empty_fill_size %d %d", 2, sink->param.audio.sram_empty_fill_size, empty_fill_size);
                    sink->param.audio.sram_empty_fill_size += empty_fill_size;
                }
                if (dl_base_addr != NULL) { //Prevent to access null pointer when the last isr is executed after HW is turned off and pointer is cleared
                    if (buffer_info->WriteOffset > buffer_info->ReadOffset) {
                        memset((void *)(dl_base_addr + buffer_info->ReadOffset), 0, buffer_info->WriteOffset - buffer_info->ReadOffset);
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
                        if (sink->param.audio.channel_num == 4) {
                            memset((void *)(buffer_info->startaddr[1] + buffer_info->ReadOffset), 0, buffer_info->WriteOffset - buffer_info->ReadOffset);
                            if (sink->param.audio.channel_num == 6) {
                                memset((void *)(buffer_info->startaddr[2] + buffer_info->ReadOffset), 0, buffer_info->WriteOffset - buffer_info->ReadOffset);
                            }
                        }
#endif
                    } else {
                        memset((void *)(dl_base_addr + buffer_info->ReadOffset), 0, buffer_info->length - buffer_info->ReadOffset);
                        memset((void *)dl_base_addr, 0, buffer_info->WriteOffset);
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
                        if (sink->param.audio.channel_num >= 4) {
                            memset((void *)(buffer_info->startaddr[1] + buffer_info->ReadOffset), 0, buffer_info->length - buffer_info->ReadOffset);
                            memset((void *)buffer_info->startaddr[1], 0, buffer_info->WriteOffset);
                            if (sink->param.audio.channel_num >= 6) {
                                memset((void *)(buffer_info->startaddr[2] + buffer_info->ReadOffset), 0, buffer_info->length - buffer_info->ReadOffset);
                                memset((void *)buffer_info->startaddr[2], 0, buffer_info->WriteOffset);
                            }
                        }
#endif
                    }
                }
            }
        } else if (buffer_info->ReadOffset != buffer_info->WriteOffset) {
            buffer_info->bBufferIsFull = FALSE;
        }

#if 0
//#if defined(MTK_GAMING_MODE_HEADSET)
        if ((Source_blks[SOURCE_TYPE_AUDIO] != NULL) && (Source_blks[SOURCE_TYPE_AUDIO]->transform != NULL) && (Source_blks[SOURCE_TYPE_AUDIO]->transform->sink != NULL)) {
            if ((Source_blks[SOURCE_TYPE_AUDIO]->transform->sink->type >= SINK_TYPE_AUDIO_TRANSMITTER_MIN) && (Source_blks[SOURCE_TYPE_AUDIO]->transform->sink->type <= SINK_TYPE_AUDIO_TRANSMITTER_MAX) && (Source_blks[SOURCE_TYPE_AUDIO]->transform->sink->param.data_ul.scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_VOICE_HEADSET)) {
                SINK game_headset_voice_sink;
                n9_dsp_share_info_ptr ShareBufferInfo;
                game_headset_voice_param_t *voice_para;

                game_headset_voice_sink = Source_blks[SOURCE_TYPE_AUDIO]->transform->sink;
                memcpy(&(game_headset_voice_sink->streamBuffer.ShareBufferInfo), game_headset_voice_sink->param.data_ul.share_info_base_addr, 32);/* share info fix 32 byte */
                game_headset_voice_sink->streamBuffer.ShareBufferInfo.startaddr = hal_memview_cm4_to_dsp0(game_headset_voice_sink->streamBuffer.ShareBufferInfo.startaddr);
                voice_para = &(game_headset_voice_sink->param.data_ul.scenario_param.voice_param);
                ShareBufferInfo = &game_headset_voice_sink->streamBuffer.ShareBufferInfo;

                if ((voice_para->ul_process_done == TRUE) && ((voice_para->dl_irq_cnt == 0 || voice_para->dl_irq_cnt == 3))) {
                    // update wo per 6 dl irq
                    uint32_t total_buffer_size = ShareBufferInfo->sub_info.block_info.blk_size * ShareBufferInfo->sub_info.block_info.blk_num;
                    ShareBufferInfo->WriteOffset = (ShareBufferInfo->WriteOffset + ShareBufferInfo->sub_info.block_info.blk_size * 1) % total_buffer_size;
                    game_headset_voice_sink->param.data_ul.share_info_base_addr->WriteOffset = ShareBufferInfo->WriteOffset;
                    //DSP_MW_LOG_I("[audio transmitter][gaming_headset ul]: Update WO:%d", 1, ShareBufferInfo->WriteOffset);
                    uint32_t bt_clk;
                    uint16_t intra_clk;
                    MCE_GetBtClk((BTCLK *)&bt_clk, (BTPHASE *)&intra_clk, ULL_CLK_Offset);

                    // CCNI to notice controller
                    if (game_headset_voice_sink->param.data_ul.current_notification_index == 0) {
                        /* prepare message */
                        hal_ccni_message_t msg;
                        msg.ccni_message[0] = AUDIO_TRANSMITTER_GAMING_MODE_VOICE_HEADSET;
                        msg.ccni_message[1] = bt_clk;
                        /* send CCNI to notify controller*/
                        hal_ccni_set_event(CCNI_DSP0_TO_CM4_EVENT2, &msg);
                        game_headset_voice_sink->param.data_ul.current_notification_index = 1;
                        DSP_MW_LOG_I("[audio transmitter][gaming_headset ul]: send CCNI to notice controller, current_notification_index:%d, bt_clk:%d", 2, game_headset_voice_sink->param.data_ul.current_notification_index, bt_clk);
                    }

                    //DSP_MW_LOG_I("[audio transmitter][gaming_headset ul]: Update dl_irq_cnt:%d (after 0 / 5)", 1, voice_para->dl_irq_cnt);
                    voice_para->dl_irq_cnt++;
                    if (voice_para->dl_irq_cnt == 6) {
                        voice_para->dl_irq_cnt = 0;
                    }
                } else {
                    //DSP_MW_LOG_I("[audio transmitter][gaming_headset ul]: Update dl_irq_cnt:%d", 1, voice_para->dl_irq_cnt);
                    voice_para->dl_irq_cnt++;
                    if (voice_para->dl_irq_cnt == 6) {
                        voice_para->dl_irq_cnt = 0;
                    }
                }
            }
        }
#endif /*defined(MTK_GAMING_MODE_HEADSET)*/



#if 0
        remain_sink_data = sink->streamBuffer.BufferInfo.length - SinkSlack(sink);
        if (remain_sink_data == 0) {
            //printf("[AUD]underflow\r\n");
            SinkBufferUpdateReadPtr(sink, 0);
            hal_nvic_restore_interrupt_mask(mask);
            return;
        }

        if (afe_block->u4ReadIdx > afe_block->u4WriteIdx) {
            sram_free_space = afe_block->u4ReadIdx - afe_block->u4WriteIdx;
        } else if ((afe_block->u4bufferfullflag == TRUE) && (afe_block->u4ReadIdx == afe_block->u4WriteIdx)) { // for Play_en not active yet
            sram_free_space = 0;
        } else {
            sram_free_space = afe_block->u4BufferSize + afe_block->u4ReadIdx - afe_block->u4WriteIdx;
        }
        sram_free_space = word_size_align(sram_free_space);// 4-byte alignment
        if (channel_type == STREAM_S_AFE_S) {
            copy_size = MINIMUM(sram_free_space >> 1, remain_sink_data);
            afe_block->u4bufferfullflag = (sram_free_space == copy_size << 1) ? TRUE : FALSE;
            sram_free_space = copy_size << 1;
        } else {
            copy_size = MINIMUM(sram_free_space, remain_sink_data);
            afe_block->u4bufferfullflag = (sram_free_space == copy_size) ? TRUE : FALSE;
            sram_free_space = copy_size;
        }

#if 0
        if (afe_block->u4WriteIdx + sram_free_space < afe_block->u4BufferSize) {
            audio_split_sink_to_interleaved(sink, (afe_block->pSramBufAddr + afe_block->u4WriteIdx), 0, sram_free_space);
        } else {
            uint32_t size_1 = 0, size_2 = 0;
            size_1 = word_size_align((afe_block->u4BufferSize - afe_block->u4WriteIdx));
            size_2 = word_size_align((sram_free_space - size_1));
            audio_split_sink_to_interleaved(sink, (afe_block->pSramBufAddr + afe_block->u4WriteIdx), 0, size_1);
            audio_split_sink_to_interleaved(sink, (afe_block->pSramBufAddr), size_1, size_2);
        }
#else
        audio_afe_sink_interleaved(sink, sram_free_space);
#endif
        afe_block->u4WriteIdx = (afe_block->u4WriteIdx + sram_free_space) % afe_block->u4BufferSize;
        SinkBufferUpdateReadPtr(sink, copy_size);
#endif


        if (pStream && pStream->streamingStatus == STREAMING_END) {
            if (Audio_setting->Audio_sink.Zero_Padding_Cnt > 0) {
                Audio_setting->Audio_sink.Zero_Padding_Cnt--;
                // DSP_MW_LOG_I("DL zero pad %d", 1, Audio_setting->Audio_sink.Zero_Padding_Cnt);
            }
        }
        #ifdef AIR_BT_HFP_ENABLE
        if (g_ignore_next_drop_flag) {
            g_ignore_next_drop_flag = false;
        }
        #endif
        AudioCheckTransformHandle(sink->transform);
        if (empty  && overflow_check_flag) {
            DSP_MW_LOG_W("DL1 ,SRAM Empty play en:%d pR:%d R:%d W:%d", 4, isr_interval, pre_offset, buffer_info->ReadOffset, empty_WriteOffset);
        }
        if(empty_eSCO) {
            DSP_MW_LOG_I("eSCO DL audio irq no data in Wo:%d Ro:%d", 2, source->streamBuffer.AVMBufferInfo.WriteIndex, source->streamBuffer.AVMBufferInfo.ReadIndex);
        }
    }

    #if 0
    static uint32_t hwsrc_owo;
    if (hwsrc_owo == AFE_GET_REG(ASM_CH01_OBUF_WRPNT)) {
        DSP_MW_LOG_E("0x1160:0x%x", 1, AFE_GET_REG(ASM_CH01_OBUF_WRPNT));
    }
    hwsrc_owo = AFE_GET_REG(ASM_CH01_OBUF_WRPNT);
    #endif

#ifdef ENABLE_HWSRC_ON_MAIN_STREAM
    if (afe_block->u4asrcflag) {
//#if (AFE_REGISTER_ASRC_IRQ)

//#else
#ifdef ENABLE_HWSRC_CLKSKEW
        if (Clock_Skew_HWSRC_Is_Enable(source,sink)) {

        } else
#endif
        {
            AFE_WRITE(ASM_CH01_IBUF_WRPNT, buffer_info->WriteOffset + AFE_READ(ASM_IBUF_SADR));
        }
//#endif /*AFE_REGISTER_ASRC_IRQ*/
    }
#endif /*ENABLE_HWSRC_ON_MAIN_STREAM*/

}


#ifdef ENABLE_HWSRC_CLKSKEW
asrc_compensated_ctrl_t u4asrcSetCompensatedSample;

void clock_skew_asrc_compensated_sample_reset(void)
{
    volatile SINK sink = Sink_blks[SINK_TYPE_AUDIO];
    afe_block_t *afe_block = &sink->param.audio.AfeBlkControl;
    afe_block->u4asrcSetCompensatedSample = &u4asrcSetCompensatedSample;
    memset(afe_block->u4asrcSetCompensatedSample, 0,sizeof(asrc_compensated_ctrl_t));
}

void clock_skew_asrc_set_compensated_sample(S32 cp_point, U32 samples)
{
    volatile SINK sink = Sink_blks[SINK_TYPE_AUDIO];
    asrc_compensated_ctrl_t *u4asrcSetCompensatedSample = sink->param.audio.AfeBlkControl.u4asrcSetCompensatedSample;
    if(u4asrcSetCompensatedSample){
        u4asrcSetCompensatedSample->CtrlBuf[0][u4asrcSetCompensatedSample->WriteIdx] = cp_point;
        u4asrcSetCompensatedSample->CtrlBuf[1][u4asrcSetCompensatedSample->WriteIdx] = samples;
        u4asrcSetCompensatedSample->WriteIdx++;
        u4asrcSetCompensatedSample->WriteIdx %= 60;
    }
}

S32 clock_skew_asrc_get_compensated_sample(U32 samples)
{
    volatile SINK sink = Sink_blks[SINK_TYPE_AUDIO];
    asrc_compensated_ctrl_t *u4asrcSetCompensatedSample = sink->param.audio.AfeBlkControl.u4asrcSetCompensatedSample;
    afe_asrc_compensating_t step = 0;
    if(u4asrcSetCompensatedSample){
        step = u4asrcSetCompensatedSample->CtrlBuf[0][u4asrcSetCompensatedSample->ReadIdx];
        if(u4asrcSetCompensatedSample->ReadIdx != u4asrcSetCompensatedSample->WriteIdx){
            u4asrcSetCompensatedSample->CtrlBuf[1][u4asrcSetCompensatedSample->ReadIdx] -= samples;

            if(u4asrcSetCompensatedSample->CtrlBuf[1][u4asrcSetCompensatedSample->ReadIdx] == 0){
                u4asrcSetCompensatedSample->ReadIdx++;
                u4asrcSetCompensatedSample->ReadIdx %= 60;
            }
        }else{
            step = 0;
        }
    }
    return step;
}

uint32_t clock_skew_asrc_get_input_sample_size(void)
{
    uint32_t sample_size = 0;
    volatile SINK sink = Sink_blks[SINK_TYPE_AUDIO];
    SOURCE_TYPE source_type = sink->transform->source->type;
    bt_codec_capability_t *codec_cap_ptr;

    switch (source_type) {
        case SOURCE_TYPE_A2DP:
            codec_cap_ptr = &(sink->transform->source->param.n9_a2dp.codec_info.codec_cap);
            if ((codec_cap_ptr->type == BT_A2DP_CODEC_VENDOR) && ((codec_cap_ptr->codec.vend.codec_id == BT_A2DP_CODEC_LHDC_CODEC_ID) || (codec_cap_ptr->codec.vend.codec_id == BT_A2DP_CODEC_LC3PLUS_CODEC_ID))){
                sample_size = 240;
            }else{
                sample_size = 128;
            }
            break;

        case SOURCE_TYPE_N9SCO:
            sample_size = 240;
            break;

#ifdef AIR_BT_CODEC_BLE_ENABLED
        case SOURCE_TYPE_N9BLE:
            sample_size = ((sink->param.audio.src_rate * sink->transform->source->param.n9ble.frame_interval) / 1000) / 1000;
            break;
#endif

        default:
            DSP_MW_LOG_W("clock_skew_asrc_get_input_sample_size fail type:%d", 1, source_type);
            break;
    }

    return sample_size;
}

#endif

#if defined AIR_ULL_AUDIO_V2_DONGLE_ENABLE
ATTR_TEXT_IN_IRAM void afe_vul1_interrupt_handler(void)
{
    // uint32_t vul_irq_time = 0;
    // hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &vul_irq_time);
    /* I2S SLV tracking mode(interconn mode): hwsrc2 */
    /* This irq is only used to trigger hwsrc */
    uint32_t vul1_cur_wo = AFE_GET_REG(AFE_VUL_CUR) - AFE_GET_REG(AFE_VUL_BASE);
    /* update hwsrc input buffer wo */
    AFE_WRITE(ASM2_CH01_IBUF_WRPNT, vul1_cur_wo + AFE_READ(ASM2_IBUF_SADR));
    /* Check IRQ jitter between BT CCNI and Audio IRQ */
}
#else
#ifdef AIR_DCHS_MODE_ENABLE
static uint32_t g_vul_sw_timer_handle;
static void dchs_vul1_timer_callback(void *user_data)
{
    UNUSED(user_data);
    SOURCE source = Source_blks[SOURCE_TYPE_AUDIO];
    if (source != NULL) {
        AudioCheckTransformHandle(source->transform);
    }
    if (g_vul_sw_timer_handle != 0) {
        hal_gpt_sw_free_timer(g_vul_sw_timer_handle);
        g_vul_sw_timer_handle = 0;
    }
}
#endif
extern bool ULL_NrOffloadFlag;
ATTR_TEXT_IN_IRAM void afe_vul1_interrupt_handler(void)
{
    //uint32_t mask;
    uint32_t hw_current_write_idx_vul1;
    #ifndef AIR_ECHO_MEMIF_IN_ORDER_ENABLE
    uint32_t hw_current_write_idx_awb;
    #endif
    uint32_t wptr_vul1_offset, pre_wptr_vul1_offset, pre_offset;
    U8 rcdc_ch_num = 0;
    static bool data_first_comming = false;
    uint16_t mute_pkt_num = 0;
    BTCLK bt_clk;
    BTPHASE bt_phase;
    uint32_t afe_buffer_size = 0;
    bool callback_busy_flag = false;
    bool overflow_flag_vul1 = false;
    uint32_t overflow_ReadOffset;
    uint32_t gpt_cnt;
    SOURCE source = Source_blks[SOURCE_TYPE_AUDIO];

    if (source != NULL) {

#if defined(AIR_WIRED_AUDIO_ENABLE)
        if (source->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_OUT) {
            /* Record the TS for latency control */
            wired_audio_handle_t *handle;
            handle = (wired_audio_handle_t *)(source->transform->sink->param.data_ul.scenario_param.usb_out_local_param.handle);
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &handle->first_afe_irq_gpt);
        }
#endif

#ifdef AIR_BT_CLK_SKEW_ENABLE
        MCE_GetBtClk(&bt_clk,&bt_phase, BT_CLK_Offset);
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_cnt);
        Clock_Skew_Check_Isr_Status_From_SrcSnk(source, source->transform->sink, bt_clk, bt_phase);
        if (Clock_Skew_ECDC_Is_Enable(source, source->transform->sink)){
            Clock_Skew_Isr_Time_Update(source, source->transform->sink, gpt_cnt, source->param.audio.count);
        }
#endif

        if (Clock_Skew_ECDC_Is_Enable(source, source->transform->sink)){
            Clock_Skew_Isr_Time_Update(source, source->transform->sink, gpt_cnt, source->param.audio.count);
        }


        AUDIO_PARAMETER *runtime = &source->param.audio;
        BUFFER_INFO *buffer_info = &source->streamBuffer.BufferInfo;

        if (runtime->channel_num >= 2) {
            rcdc_ch_num = 2;
        } else {
            rcdc_ch_num = 1;
        }
        int32_t  wptr_vul1_offset_diff_defualt = (runtime->format_bytes) * (runtime->count) * (rcdc_ch_num);
        int32_t  wptr_vul1_offset_diff = 0;
        int16_t cp_samples = 0;


        /*Mce play en check*/
        if (runtime->irq_exist == false) {
            runtime->irq_exist = true;
            data_first_comming = true;
            U32 vul_irq_time;
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &vul_irq_time);
            DSP_MW_LOG_I("DSP afe ul1 interrupt exist, time: %d", 1, vul_irq_time);
            runtime->pop_noise_pkt_num = 0;
            runtime->mute_flag = TRUE;
        } else if ((Sink_blks[SINK_TYPE_AUDIO] != NULL) &&
                   (Sink_blks[SINK_TYPE_AUDIO]->param.audio.AfeBlkControl.u4awsflag == true) &&
                   (Sink_blks[SINK_TYPE_AUDIO]->param.audio.irq_exist == false)) {
            uint32_t read_reg = afe_get_bt_sync_monitor(AUDIO_DIGITAL_BLOCK_MEM_DL1);
            if (read_reg == 0) {
                DSP_MW_LOG_I("DSP afe BT sync monitor by ul1 0x%x", 1, read_reg);
            }
        }

        hw_current_write_idx_vul1 = AFE_GET_REG(AFE_VUL_CUR);
        pre_wptr_vul1_offset = buffer_info->WriteOffset;

        if (source->param.audio.echo_reference == true) {
#if 1 //modify for ab1568 workaround
            #ifndef AIR_ECHO_MEMIF_IN_ORDER_ENABLE
            hw_current_write_idx_awb = AFE_GET_REG(AFE_AWB2_CUR);
            #endif
#else
            hw_current_write_idx_awb = AFE_GET_REG(AFE_AWB_CUR);
#endif

        }

#ifdef AIR_BT_CLK_SKEW_ENABLE
        Clock_Skew_Offset_Update(BT_CLK_Offset, source, source->transform->sink);

#if 1//mdify for ab1568
        if(source->param.audio.mem_handle.pure_agent_with_src)
        {
            pre_wptr_vul1_offset = AFE_READ(ASM2_CH01_IBUF_WRPNT) - AFE_READ(ASM2_IBUF_SADR);
            afe_buffer_size = AFE_READ(ASM2_IBUF_SIZE);
        }else{
            afe_buffer_size = buffer_info->length;
        }
        if (Clock_Skew_Get_Polling_Flag(source,source->transform->sink)) {
            while (wptr_vul1_offset_diff < (wptr_vul1_offset_diff_defualt + 8 * rcdc_ch_num)) {
                wptr_vul1_offset = AFE_GET_REG(AFE_VUL_CUR) - AFE_GET_REG(AFE_VUL_BASE);
                if (wptr_vul1_offset >= pre_wptr_vul1_offset) {
                    wptr_vul1_offset_diff = wptr_vul1_offset - pre_wptr_vul1_offset;
                    //DSP_MW_LOG_I("[ClkSkew1] buffer_info->length:%d, vul now_wo:%d, pre_wo:%d, wo_diff:%d", 4, buffer_info->length, wptr_vul1_offset, pre_wptr_vul1_offset, wptr_vul1_offset_diff);
                } else {
                    wptr_vul1_offset_diff = afe_buffer_size + wptr_vul1_offset - pre_wptr_vul1_offset;
                    //DSP_MW_LOG_I("[ClkSkew2] buffer_info->length:%d, vul now_wo:%d, pre_wo:%d, wo_diff:%d", 4, buffer_info->length, wptr_vul1_offset, pre_wptr_vul1_offset, wptr_vul1_offset_diff);
                }
            }
            Clock_Skew_Set_Polling_Flag(source,source->transform->sink,FALSE);
            wptr_vul1_offset_diff = wptr_vul1_offset_diff_defualt;
        } else {
            wptr_vul1_offset = hw_current_write_idx_vul1 - AFE_GET_REG(AFE_VUL_BASE);
        }
#endif
#else
        wptr_vul1_offset = hw_current_write_idx_vul1 - AFE_GET_REG(AFE_VUL_BASE);
#endif
#if 0//modify for ab1568
        if (afe_get_memory_path_enable(AUDIO_DIGITAL_BLOCK_MEM_VUL1))
#endif

        {

#ifdef AIR_BT_CLK_SKEW_ENABLE
            /* For Uplink Clk Skew IRQ period control */
            cp_samples = Clock_Skew_Check_Status_From_SrcSnk(source, source->transform->sink);

#if 0//modify for ab1568
            afe_update_audio_irq_cnt(afe_irq_request_number(AUDIO_DIGITAL_BLOCK_MEM_VUL1),
                                     (uint32_t)((int32_t)runtime->count + (int32_t)cp_samples));
#else
            hal_audio_memory_irq_period_parameter_t irq_period;
            irq_period.memory_select = HAL_AUDIO_MEMORY_UL_VUL1;
            irq_period.rate = runtime->rate;
            irq_period.irq_counter = (uint32_t)((int32_t)runtime->count + (int32_t)cp_samples);
            /*if(cp_samples != 0) {
               DSP_MW_LOG_I("[ClkSkew] Vul cp_samples:%d, count:%d, irq_counter:%d", 3, cp_samples, runtime->count, irq_period.irq_counter);
            }*/
            hal_audio_control_set_value((hal_audio_set_value_parameter_t *)&irq_period, HAL_AUDIO_SET_MEMORY_IRQ_PERIOD);
            Clock_Skew_Samples_Cnt_Update(source, source->transform->sink, (U16)irq_period.irq_counter);
#endif
#endif
            //hal_nvic_save_and_set_interrupt_mask(&mask);
            if (hw_current_write_idx_vul1 == 0) {
                //hal_nvic_restore_interrupt_mask(mask);
                return;
            }
            #ifndef AIR_ECHO_MEMIF_IN_ORDER_ENABLE
            if ((source->param.audio.echo_reference == true) && (hw_current_write_idx_awb == 0)) {
                //hal_nvic_restore_interrupt_mask(mask);
                return;
            }
            #endif

            /* Fill zero packet to prevent UL pop noise (Units:ms) */
            if (source->param.audio.audio_device & HAL_AUDIO_CONTROL_DEVICE_LINE_IN_DUAL) {
                mute_pkt_num = LINE_IN_VUL1_STARTUP_DELAY;
            } else {
                mute_pkt_num = GENERIC_VUL1_STARTUP_DELAY;
            }

            if (runtime->pop_noise_pkt_num < mute_pkt_num) {
                runtime->mute_flag = TRUE;
                runtime->pop_noise_pkt_num += source->param.audio.period;
            } else {
                runtime->mute_flag = FALSE;
            }

            if (source && source->transform && source->transform->sink) {
                SINK sink = source->transform->sink;

                DSP_CALLBACK_PTR callback_ptr;
                callback_ptr = DSP_Callback_Get(source, sink);
#ifdef AIR_BT_CODEC_BLE_ENABLED
                if (sink->type == SINK_TYPE_N9BLE && callback_ptr->Status == CALLBACK_HANDLER) {
                    // DSP_MW_LOG_I("Callback busy! source=%d, sink=%d", 2,source->type, sink->type);
                    callback_busy_flag = true;
#if AIR_WIRELESS_MIC_TX_ENABLE
#if !DL_TRIGGER_UL
                    if ((sink->param.n9ble.IsFirstIRQ == FALSE) && (callback_ptr->IsBusy == TRUE)) {
                        SourceDrop(source, (runtime->format_bytes) * (runtime->count));
                        SinkFlush(sink, sink->param.n9ble.frame_length * sink->param.n9ble.process_number);
                    } else {
                        callback_ptr->IsBusy = TRUE;
                    }
#else
                        SourceDrop(source, (runtime->format_bytes) * (runtime->count));
                        if (ULL_NrOffloadFlag == true) {
                            SinkFlush(sink, (sink->param.n9ble.frame_length-1) * sink->param.n9ble.process_number);
                        }else{
                            SinkFlush(sink, sink->param.n9ble.frame_length * sink->param.n9ble.process_number);
                        }
                        //N9BLE_setting.N9Ble_sink.N9_Ro_abnormal_cnt = 0;
#endif
#else
                    SourceDrop(source, (runtime->format_bytes) * (runtime->count));
                    /* trigger exception handling to resync timestamp & avm index in task */
                    ++sink->param.n9ble.dummy_insert_number;
#endif
                }
#endif /*AIR_BT_CODEC_BLE_ENABLED*/
#ifdef AIR_BT_HFP_ENABLE
                if (sink->type == SINK_TYPE_N9SCO && callback_ptr->Status == CALLBACK_HANDLER) {
                    callback_busy_flag = true;
                    if (sink->param.n9sco.IsFirstIRQ == TRUE) {
                        SourceDrop(source, (runtime->format_bytes) * (runtime->count));
                    }
#if !DL_TRIGGER_UL
                    if ((sink->param.n9sco.IsFirstIRQ == FALSE) && (callback_ptr->IsBusy == TRUE)) {
                        SourceDrop(source, (runtime->format_bytes) * (runtime->count));
                        SinkFlush(sink, sink->param.n9sco.process_data_length);
                    } else {
                        callback_ptr->IsBusy = TRUE;
                    }
#else
                    else {
                        #ifdef AIR_ICE_DEBUG_ENABLE
                        if(hal_ice_debug_is_enabled() == false)
                        #endif
                        {
                            SourceDrop(source, (runtime->format_bytes) * (runtime->count));
                            SinkFlush(sink, sink->param.n9sco.process_data_length);
                            N9SCO_setting->N9Sco_sink.N9_Ro_abnormal_cnt = 0;
                        }
                    }
#endif
                }
#endif
            }
            // if (callback_busy_flag) {
            //     DSP_MW_LOG_W("VUL1 Callback busy scenario type [%d]", 1, source->scenario_type);
            // }
            pre_offset = buffer_info->WriteOffset;//check overflow
            buffer_info->WriteOffset = wptr_vul1_offset;
            if(source->param.audio.mem_handle.pure_agent_with_src)
            {
                AFE_WRITE(ASM2_CH01_IBUF_WRPNT, wptr_vul1_offset + AFE_READ(ASM2_IBUF_SADR)); //update HWSRC input buffer write_offset that AFE write hwsrc2 input buffer's
                if (data_first_comming){
                    buffer_info->WriteOffset = wptr_vul1_offset; //first update source buffer write offset to make fw move, otherwise path will mute loop
                    data_first_comming = false;
                }else{
                    buffer_info->WriteOffset = AFE_GET_REG(ASM2_CH01_OBUF_WRPNT) - AFE_READ(ASM2_OBUF_SADR);//when path work, update source buffer wirte offset with hwsrc2 out buffer write offset
                }
            }else{
                buffer_info->WriteOffset = wptr_vul1_offset;
            }
            //printf("vul pre %d wpt %d,rpt %d len %d\r\n",pre_offset,buffer_info->WriteOffset, buffer_info->ReadOffset, buffer_info->length);
            if (runtime->irq_exist) {
                if (afe_offset_overflow_check(pre_offset, buffer_info, false)) {
                    overflow_flag_vul1 = true;
                    overflow_ReadOffset = buffer_info->ReadOffset;
                    // DSP_MW_LOG_W("UL OFFSET_OVERFLOW ! pre %d,w %d,r %d", 3, pre_offset, buffer_info->WriteOffset, buffer_info->ReadOffset);
#if 0
                    if (pre_offset < buffer_info->WriteOffset) {
                        buffer_info->ReadOffset = (buffer_info->WriteOffset * 2 - pre_offset) % buffer_info->length;
                    } else {
                        buffer_info->ReadOffset = (buffer_info->WriteOffset * 2 + (buffer_info->length - pre_offset)) % buffer_info->length;
                    }
#else
                    buffer_info->ReadOffset = (buffer_info->ReadOffset + (buffer_info->length) / 2) % buffer_info->length;
#endif
                }
            }

#if defined(AIR_WIRED_AUDIO_ENABLE) && !defined(AIR_DCHS_MODE_ENABLE)
        if ((source->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_OUT) || (source->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_DUAL_CHIP_LINE_OUT_MASTER)) {
            /* For line out, use only sink afe to wakeup the stream */
            //hal_nvic_restore_interrupt_mask(mask);
            return;
        }
#endif

#ifdef AIR_DCHS_MODE_ENABLE
            if(dchs_get_device_mode() == DCHS_MODE_LEFT){
                uint32_t during_time = 5000;
                if((source->scenario_type == AUDIO_SCENARIO_TYPE_DCHS_UART_UL) || (source->scenario_type == AUDIO_SCENARIO_TYPE_HFP_UL) || (source->scenario_type == AUDIO_SCENARIO_TYPE_BLE_UL)){
                    hal_gpt_sw_get_timer(&g_vul_sw_timer_handle);
                    hal_gpt_sw_start_timer_us(g_vul_sw_timer_handle, during_time, dchs_vul1_timer_callback, NULL);
                }else{
                    AudioCheckTransformHandle(source->transform);
                }
            }else{
                AudioCheckTransformHandle(source->transform);
            }
#else
            AudioCheckTransformHandle(source->transform);
#endif
#if defined (AIR_WIRED_AUDIO_ENABLE)
            extern SOURCE g_usb_in_out_source;
            if((source->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_OUT)&&(g_usb_in_out_source != NULL)){
                if(g_usb_in_out_source->transform != NULL){
                    g_usb_in_out_source->param.virtual_para.is_processed = false;
                    AudioCheckTransformHandle(g_usb_in_out_source->transform);
                }
            }
#endif
            //hal_nvic_restore_interrupt_mask(mask);
            if (callback_busy_flag) {
                DSP_MW_LOG_W("VUL1 Callback busy scenario type [%d]", 1, source->scenario_type);
            }
            if (overflow_flag_vul1) {
                DSP_MW_LOG_W("UL OFFSET_OVERFLOW ! pre %d,w %d,r %d", 3, pre_offset, buffer_info->WriteOffset, overflow_ReadOffset);
            }
        }

    }
}
#endif /* enable dongle */
#if defined(AIR_MULTI_MIC_STREAM_ENABLE) || defined(MTK_ANC_SURROUND_MONITOR_ENABLE) || defined(AIR_WIRED_AUDIO_ENABLE) || defined(AIR_ADVANCED_PASSTHROUGH_ENABLE) || defined(AIR_ADAPTIVE_EQ_ENABLE) || defined (AIR_BT_AUDIO_DONGLE_ENABLE)
static uint32_t irq_cnt = 0;
#endif
ATTR_TEXT_IN_IRAM void afe_subsource_interrupt_handler(void)
{
#if defined(AIR_MULTI_MIC_STREAM_ENABLE) || defined(MTK_ANC_SURROUND_MONITOR_ENABLE) || defined(AIR_WIRED_AUDIO_ENABLE) || defined(AIR_ADVANCED_PASSTHROUGH_ENABLE) || defined(AIR_ADAPTIVE_EQ_ENABLE) || defined (AIR_BT_AUDIO_DONGLE_ENABLE)


    SOURCE_TYPE      source_type;
    uint32_t hw_current_write_idx, pre_offset,wptr_vul3_offset;
    static bool data_first_comming = false;
    SOURCE source = NULL;

    AUDIO_PARAMETER *runtime = &source->param.audio;
    BUFFER_INFO *buffer_info = &source->streamBuffer.BufferInfo;

    for (source_type = SOURCE_TYPE_SUBAUDIO_MIN ; source_type <= SOURCE_TYPE_SUBAUDIO_MAX ; source_type++) {
        source = Source_blks[source_type];

        if ((!source) || (!source->param.audio.is_memory_start)) {
            continue;
        }
#ifdef AIR_BT_AUDIO_DONGLE_ENABLE
        if ((source->scenario_type >= AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0) &&
            (source->scenario_type <= AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2)) {
            /* BT Dongle DL have their own irq handler of BT CCNI */
            continue;
        }
#endif
        runtime = &source->param.audio;
        buffer_info = &source->streamBuffer.BufferInfo;

        /* First handle */
        if (runtime->irq_exist == false) {
            runtime->irq_exist = true;
            data_first_comming = true;
            uint32_t vul_irq_time;
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &vul_irq_time);
            DSP_MW_LOG_I("DSP afe sub-source:%d interrupt exist, time: %d", 2, source_type, vul_irq_time);
            runtime->pop_noise_pkt_num = 0;
            runtime->mute_flag = TRUE;
        }

        /* Fill zero packet to prevent UL pop noise (Units:ms) */
        if (runtime->pop_noise_pkt_num < (240*(source->param.audio.rate/1000))) {
            runtime->mute_flag = TRUE;
            runtime->pop_noise_pkt_num += source->param.audio.count;
        } else {
            runtime->mute_flag = FALSE;
        }

        /* Get current offset */
        hal_audio_memory_selection_t memory_search;
        hal_audio_current_offset_parameter_t get_current_offset;
        for (memory_search = HAL_AUDIO_MEMORY_UL_VUL1 ; memory_search <= HAL_AUDIO_MEMORY_UL_AWB2 ; memory_search <<= 1) {
            if (source->param.audio.mem_handle.memory_select & memory_search) {
                break;
            }
        }
        pre_offset = buffer_info->WriteOffset;
        get_current_offset.memory_select = memory_search;
        get_current_offset.pure_agent_with_src = false;
        hw_current_write_idx = hal_audio_get_value((hal_audio_get_value_parameter_t *)&get_current_offset, HAL_AUDIO_GET_MEMORY_INPUT_CURRENT_OFFSET);
        if(hw_current_write_idx) {
            wptr_vul3_offset = hw_current_write_idx - get_current_offset.base_address;

            if(source->param.audio.mem_handle.pure_agent_with_src) {
#ifdef AIR_DUAL_CHIP_MASTER_HWSRC_RX_TRACKING_ENABLE
                AFE_WRITE(ASM_CH01_IBUF_WRPNT, wptr_vul3_offset + AFE_READ(ASM_IBUF_SADR)); //update HWSRC input buffer write_offset that AFE write hwsrc2 input buffer's
#else
                AFE_WRITE(ASM2_CH01_IBUF_WRPNT, wptr_vul3_offset + AFE_READ(ASM2_IBUF_SADR)); //update HWSRC input buffer write_offset that AFE write hwsrc2 input buffer's
#endif
                if (data_first_comming) {
                    buffer_info->WriteOffset = wptr_vul3_offset; //first update source buffer write offset to make fw move, otherwise path will mute loop
                    data_first_comming = false;
                } else {
#ifdef AIR_DUAL_CHIP_MASTER_HWSRC_RX_TRACKING_ENABLE
                    buffer_info->WriteOffset = AFE_GET_REG(ASM_CH01_OBUF_WRPNT) - AFE_READ(ASM_OBUF_SADR);//when path work, update source buffer wirte offset with hwsrc out buffer write offset
                    if (AFE_GET_REG(ASM_CH01_OBUF_RDPNT) == AFE_GET_REG(ASM_CH01_OBUF_WRPNT)) {
                        buffer_info->bBufferIsFull = TRUE;
                    }
#else
                    buffer_info->WriteOffset = AFE_GET_REG(ASM2_CH01_OBUF_WRPNT) - AFE_READ(ASM2_OBUF_SADR);//when path work, update source buffer wirte offset with hwsrc2 out buffer write offset
                    if (AFE_GET_REG(ASM2_CH01_OBUF_RDPNT) == AFE_GET_REG(ASM2_CH01_OBUF_WRPNT)) {
                        buffer_info->bBufferIsFull = TRUE;
                    }
#endif
                }
            } else {
                buffer_info->WriteOffset = wptr_vul3_offset;
            }
        }
#if 0
        /* Clock skew */
        hal_audio_memory_irq_period_parameter_t irq_period;
        int16_t cp_sample = 0;
        cp_sample = clk_skew_check_ul_status();

        irq_period.memory_select = HAL_AUDIO_MEMORY_UL_AWB2;//Keep at AWB2 for sub source
        irq_period.rate = Audio_setting->Audio_source.memory.audio_path_rate;
        irq_period.irq_counter = (uint32_t)(Audio_setting->Audio_source.memory.irq_counter + cp_sample);
        hal_audio_set_value((hal_audio_set_value_parameter_t *)&irq_period, HAL_AUDIO_SET_MEMORY_IRQ_PERIOD);
#endif

        /* overflow check */
        if (afe_offset_overflow_check(pre_offset, buffer_info, false)) {
            DSP_MW_LOG_W("DSP Sub-Source:%d OFFSET_OVERFLOW ! pre:0x%x, w:0x%x, r:0x%x", 4, source_type, pre_offset, buffer_info->WriteOffset, buffer_info->ReadOffset);
            buffer_info->ReadOffset = (buffer_info->ReadOffset + (buffer_info->length) / 2) % buffer_info->length;
            irq_cnt++;
        } else {
            irq_cnt = 0;
        }

        if(irq_cnt >= 10) {
            assert(false);
        }

        /* Stream handler */
        if (source->transform && source->transform->sink) {
            AudioCheckTransformHandle(source->transform);
        }
    }
#endif
}

#if defined(AIR_ADVANCED_PASSTHROUGH_ENABLE)
ATTR_TEXT_IN_IRAM void afe_subsource2_interrupt_handler(void)
{
    SOURCE_TYPE      source_type;
    uint32_t hw_current_write_idx, pre_offset,wptr_vul3_offset;
    static bool data_first_comming = false;
    SOURCE source = NULL;

    for (source_type=SOURCE_TYPE_SUBAUDIO_MIN ; source_type<=SOURCE_TYPE_SUBAUDIO_MAX ; source_type++) {
        source = Source_blks[source_type];
        if ((source) && (source->param.audio.mem_handle.memory_select&HAL_AUDIO_MEMORY_UL_AWB)){
            break;
        }
        source = NULL;
    }
    if (!source) {
        assert(0 && "subsource not found");
    }

    AUDIO_PARAMETER *runtime = &source->param.audio;
    BUFFER_INFO *buffer_info = &source->streamBuffer.BufferInfo;

    /* First handle */
    if (runtime->irq_exist == false) {
        runtime->irq_exist = true;
        data_first_comming = true;
        uint32_t vul_irq_time;
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &vul_irq_time);
        DSP_MW_LOG_I("DSP afe sub-source2:%d interrupt exist, time: %d", 2, source_type, vul_irq_time);
        runtime->pop_noise_pkt_num = 0;
        runtime->mute_flag = TRUE;
    }

    /* Fill zero packet to prevent UL pop noise (Units:ms) */
    if(runtime->pop_noise_pkt_num < (240*(source->param.audio.rate/1000))) {
        runtime->mute_flag = TRUE;
        runtime->pop_noise_pkt_num += source->param.audio.count;
    } else {
        runtime->mute_flag = FALSE;
    }

    /* Get current offset */

    pre_offset = buffer_info->WriteOffset;

    hw_current_write_idx = AFE_GET_REG(AFE_AWB_CUR);

    if(hw_current_write_idx) {
        wptr_vul3_offset = hw_current_write_idx - AFE_GET_REG(AFE_AWB_BASE);

        if(source->param.audio.mem_handle.pure_agent_with_src)
        {
#ifdef AIR_DUAL_CHIP_MASTER_HWSRC_RX_TRACKING_ENABLE
            AFE_WRITE(ASM_CH01_IBUF_WRPNT, wptr_vul3_offset + AFE_READ(ASM_IBUF_SADR)); //update HWSRC input buffer write_offset that AFE write hwsrc2 input buffer's
#else
            AFE_WRITE(ASM2_CH01_IBUF_WRPNT, wptr_vul3_offset + AFE_READ(ASM2_IBUF_SADR)); //update HWSRC input buffer write_offset that AFE write hwsrc2 input buffer's
#endif
            if (data_first_comming){
                buffer_info->WriteOffset = wptr_vul3_offset; //first update source buffer write offset to make fw move, otherwise path will mute loop
                data_first_comming = false;
            }else{
#ifdef AIR_DUAL_CHIP_MASTER_HWSRC_RX_TRACKING_ENABLE
                buffer_info->WriteOffset = AFE_GET_REG(ASM_CH01_OBUF_WRPNT) - AFE_READ(ASM_OBUF_SADR);//when path work, update source buffer wirte offset with hwsrc out buffer write offset
                if (AFE_GET_REG(ASM_CH01_OBUF_RDPNT) == AFE_GET_REG(ASM_CH01_OBUF_WRPNT)) {
                    buffer_info->bBufferIsFull = TRUE;
                }
#else
                buffer_info->WriteOffset = AFE_GET_REG(ASM2_CH01_OBUF_WRPNT) - AFE_READ(ASM2_OBUF_SADR);//when path work, update source buffer wirte offset with hwsrc2 out buffer write offset
                if (AFE_GET_REG(ASM2_CH01_OBUF_RDPNT) == AFE_GET_REG(ASM2_CH01_OBUF_WRPNT)) {
                    buffer_info->bBufferIsFull = TRUE;
                }
#endif
            }
        }else{
            buffer_info->WriteOffset = wptr_vul3_offset;
        }
    }

    /* overflow check */
    if(afe_offset_overflow_check(pre_offset, buffer_info, false)){
        DSP_MW_LOG_W("DSP Sub-Source2 OFFSET_OVERFLOW ! pre:0x%x, w:0x%x, r:0x%x", 3, pre_offset, buffer_info->WriteOffset, buffer_info->ReadOffset);
#ifdef AIR_ADVANCED_PASSTHROUGH_ENABLE
        buffer_info->ReadOffset = (buffer_info->ReadOffset + (buffer_info->length)/2)%buffer_info->length;
#endif
    }

    /* Stream handler */
    if(source->transform && source->transform->sink)
    {
        AudioCheckTransformHandle(source->transform);
    }
}
#endif

#ifdef MTK_PROMPT_SOUND_ENABLE
volatile uint32_t vp_sram_empty_flag = 0;
extern volatile uint32_t vp_config_flag;
uint32_t g_hwsrc_halt_count = 0;
uint32_t g_log_ro = 0;
#ifdef AIR_BTA_IC_PREMIUM_G2
void afe_dl2_interrupt_handler(void)
#else
ATTR_TEXT_IN_IRAM void afe_dl2_interrupt_handler(void)
#endif
{
    //printf("afe_dl2_interrupt_handler\r\n");
    // uint32_t        mask                = 0;
    uint32_t        pre_offset          = 0;
    int32_t         hw_current_read_idx = 0;
    SINK            sink                = Sink_blks[SINK_TYPE_VP_AUDIO];
    afe_block_t     *afe_block          = &sink->param.audio.AfeBlkControl;
    BUFFER_INFO     *buffer_info        = &sink->streamBuffer.BufferInfo;
    uint32_t        dl_base_addr        = 0;
    AUDIO_PARAMETER *runtime            = &sink->param.audio;
    static uint32_t pre_oro             = 0; // obuf pre owo
    static uint32_t pre_owo             = 0; // obuf pre owo
#if 0//modify for ab1568
    if (afe_block->u4asrcflag) {
#else
    if (sink->param.audio.mem_handle.pure_agent_with_src) {
#endif
        if (runtime->irq_exist == false) {
            runtime->irq_exist = true;
            g_hwsrc_halt_count = 0;
            DSP_MW_LOG_I("afe_dl2_interrupt_handler first start", 0);
            // /* clear obuf overflow */
            // AFE_SET_REG(ASM2_IFR, 1 << 12, 1 << 12);
            pre_oro = 0;
            pre_owo = 0;
        }
        dl_base_addr = AFE_GET_REG(ASM2_IBUF_SADR);
        hw_current_read_idx = AFE_GET_REG(ASM2_CH01_IBUF_RDPNT);
    } else {
        if (runtime->irq_exist == false) {
            runtime->irq_exist = true;
            DSP_MW_LOG_I("afe_dl2_interrupt_handler first start", 0);
        }
        dl_base_addr = AFE_GET_REG(AFE_DL2_BASE);
        hw_current_read_idx = AFE_GET_REG(AFE_DL2_CUR);
    }

    //printf("AFE_DL2_CUR:%x\r\n", hw_current_read_idx);
    //printf("AFE_DL2_BASE:%x\r\n", AFE_GET_REG(AFE_DL2_BASE));
    //printf("addr:%x, value is %x\r\n", hw_current_read_idx, (*(volatile uint32_t *)hw_current_read_idx));

    //if (afe_get_memory_path_enable(AUDIO_DIGITAL_BLOCK_MEM_DL2))//modify for ab1568
    if (1) { //modify for ab1568

        // hal_nvic_save_and_set_interrupt_mask(&mask);
        if (hw_current_read_idx == 0) { //should chk setting if =0
            hw_current_read_idx = word_size_align((S32) dl_base_addr);
        }
        pre_offset = buffer_info->ReadOffset;

        buffer_info->ReadOffset = hw_current_read_idx - dl_base_addr;

        if (dl_base_addr != NULL) { //Prevent to access null pointer when the last isr is executed after HW is turned off and pointer is cleared
            /*Clear up last time used memory */
            if (buffer_info->ReadOffset >= pre_offset) {
                memset((void *)(dl_base_addr + pre_offset), 0, buffer_info->ReadOffset - pre_offset);
            } else {
                memset((void *)(dl_base_addr + pre_offset), 0, buffer_info->length - pre_offset);
                memset((void *)dl_base_addr, 0, buffer_info->ReadOffset);
            }
            }
        if ((afe_offset_overflow_check(pre_offset, buffer_info, true))
            || (((buffer_info->WriteOffset + buffer_info->length - buffer_info->ReadOffset) % buffer_info->length < 32) && (afe_block->u4asrcflag))) {
            // SRAM Empty
            bool empty = true;
            if (afe_block->u4asrcflag) {
                uint32_t src_out_size, src_out_read, src_out_write, remain_data;
                src_out_write = AFE_READ(ASM2_CH01_OBUF_WRPNT);
                src_out_read = AFE_READ(ASM2_CH01_OBUF_RDPNT);
                src_out_size = AFE_READ(ASM2_OBUF_SIZE);
                remain_data = (src_out_write > src_out_read) ? src_out_write - src_out_read : src_out_write + src_out_size - src_out_read;
                remain_data = (sink->param.audio.channel_num >= 2) ? remain_data >> 1 : remain_data;
                if (remain_data > (afe_block->u4asrcrate * (U32)sink->param.audio.period * sink->param.audio.format_bytes) / 1000) {
                    empty = false;
                }
                uint32_t base = AFE_READ(ASM2_OBUF_SADR);
                uint32_t cur_obuf_oro = src_out_read - base;
                uint32_t cur_obuf_owo = src_out_write - base;
                if ((vp_config_flag == 1) && (pre_owo == cur_obuf_owo)) {
                    if ((pre_oro == cur_obuf_oro) && (pre_oro == cur_obuf_owo)) { // not move
                        empty = true;
                    } else if ((cur_obuf_oro >= pre_oro) && (cur_obuf_owo <= cur_obuf_oro) && (cur_obuf_owo > pre_oro)) { // already read/write overflow
                        empty = true;
                    } else if ((cur_obuf_oro < pre_oro) && ((cur_obuf_owo <= cur_obuf_oro) || (cur_obuf_owo > pre_oro))) { // already read/write overflow
                        empty = true;
                     }
                }
            }
            if (empty) {
                DSP_MW_LOG_W("DL2 ,SRAM Empty play pR:%d R:%d W:%d==============", 3, pre_offset, buffer_info->ReadOffset, buffer_info->WriteOffset);
                if (vp_config_flag == 1) {
                    vp_sram_empty_flag = 1;
                    g_hwsrc_halt_count = 0;
                    /* ATTENTION: We should clear the un-used buffer to avoid HW access */
                    uint32_t owo = AFE_READ(ASM2_CH01_OBUF_WRPNT);
                    memset((uint8_t *)(AFE_READ(ASM2_CH01_OBUF_WRPNT)), 0, AFE_READ(ASM2_OBUF_SIZE) - (owo - AFE_READ(ASM2_OBUF_SADR)));
                    memset((uint8_t *)(AFE_READ(ASM2_OBUF_SADR)), 0, (owo - AFE_READ(ASM2_OBUF_SADR)));
                    xTaskResumeFromISR(sink->taskid);
                    portYIELD_FROM_ISR(pdTRUE); // force to do context switch
                }
                #ifdef AIR_DCHS_MODE_ENABLE //dchs project dl2 does not go afe,no need empty compensation
                if(dchs_get_device_mode() == DCHS_MODE_SINGLE){
                #endif
                    if (vp_config_flag != 1) {
                        if (sink->param.audio.mem_handle.pure_agent_with_src) {
                            buffer_info->WriteOffset = (buffer_info->ReadOffset + buffer_info->length / 2) % buffer_info->length;
                        } else {
                            if (pre_offset < buffer_info->ReadOffset) {
                                buffer_info->WriteOffset = (buffer_info->ReadOffset * 2 - pre_offset) % buffer_info->length;
                            } else {
                                buffer_info->WriteOffset = (buffer_info->ReadOffset * 2 + (buffer_info->length - pre_offset)) % buffer_info->length;
                            }
                        }


                        if ((dl_base_addr != NULL) && (hw_current_read_idx != NULL)) { //Prevent to access null pointer when the last isr is executed after HW is turned off and pointer is cleared
                            if (buffer_info->WriteOffset > buffer_info->ReadOffset) {
                                memset((void *)hw_current_read_idx, 0, buffer_info->WriteOffset - buffer_info->ReadOffset);
                            } else {
                                memset((void *)hw_current_read_idx, 0, buffer_info->length - buffer_info->ReadOffset);
                                memset((void *)dl_base_addr, 0, buffer_info->WriteOffset);
                            }
                        }
                    }
                #ifdef AIR_DCHS_MODE_ENABLE
                }
                #endif
            }
        } else if (buffer_info->ReadOffset != buffer_info->WriteOffset) {
            buffer_info->bBufferIsFull = FALSE;
        }
        /* monitor hwsrc status */
        if (pre_offset == buffer_info->ReadOffset) {
            if (g_log_ro != pre_offset) {
                g_hwsrc_halt_count = 0;
                g_log_ro = pre_offset;
            }
            uint32_t iro = AFE_READ(ASM2_CH01_IBUF_RDPNT) - AFE_READ(ASM2_IBUF_SADR);
            uint32_t iwo = AFE_READ(ASM2_CH01_IBUF_WRPNT) - AFE_READ(ASM2_IBUF_SADR);
            if(iro != iwo){
                DSP_MW_LOG_W("TEST halt count %u", 1, g_hwsrc_halt_count);
                g_hwsrc_halt_count ++;
            }
        }
        if (g_hwsrc_halt_count > 30) { // over 150ms
            DSP_MW_LOG_W("TEST hwsrc iro %d iwo %d oro %d owo %d ifr 0x%x ier 0x%x gen:0x%x ch01:0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x", 16,
                AFE_READ(ASM2_CH01_IBUF_RDPNT) - AFE_READ(ASM2_IBUF_SADR),
                AFE_READ(ASM2_CH01_IBUF_WRPNT) - AFE_READ(ASM2_IBUF_SADR),
                AFE_READ(ASM2_CH01_OBUF_RDPNT) - AFE_READ(ASM2_OBUF_SADR),
                AFE_READ(ASM2_CH01_OBUF_WRPNT) - AFE_READ(ASM2_OBUF_SADR),
                AFE_READ(ASM2_IFR),
                AFE_READ(ASM2_IER),
                AFE_READ(ASM2_GEN_CONF),
                AFE_READ(ASM2_CH01_CNFG),
                AFE_READ(ASM2_IBUF_INTR_CNT0),
                AFE_READ(ASM2_OBUF_INTR_CNT0),
                AFE_READ(ASM2_BAK_REG),
                AFE_READ(ASM2_FREQ_CALI_CTRL),
                AFE_READ(ASM2_FREQUENCY_0),
                AFE_READ(ASM2_FREQUENCY_1),
                AFE_READ(ASM2_FREQUENCY_2),
                AFE_READ(ASM2_FREQUENCY_3)
            );
            DSP_MW_LOG_W("TEST monitor top_con1:0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x CLKRG:0x%x,PDN_TOP:0x%x,DVFS_RG:0x%x", 13,
                AFE_READ(AUDIO_TOP_CON1),
                AFE_READ(ASM2_FREQ_CALI_CYC),
                AFE_READ(ASM2_PRD_CALI_RESULT),
                AFE_READ(ASM2_FREQ_CALI_RESULT),
                AFE_READ(ASM2_IBUF_SAMPLE_CNT),
                AFE_READ(ASM2_OBUF_SAMPLE_CNT),
                AFE_READ(ASM2_SMPCNT_CONF),
                AFE_READ(ASM2_SMPCNT_WRAP_VAL),
                AFE_READ(ASM2_SMPCNT_IRQ_VAL),
                AFE_READ(ASM2_SMPCNT1_LATCH),
                AFE_READ(0xA2020238),
                AFE_READ(0xA2030B60),
                AFE_READ(0xA2020230)
            );

            DSP_MW_LOG_W("TEST monitor 0x%x 0x%x 0x%x ibufmon0:0x%x 0x%x 0x%x 0x%x ibufmon0:0x%x 0x%x ibuf_sadr:0x%x,size:0x%x,obuf:0x%x,size:0x%x", 16,
                AFE_READ(ASM2_SMPCNT2_LATCH),
                AFE_READ(ASM2_CALI_DENOMINATOR),
                AFE_READ(ASM2_MAX_OUT_PER_IN0),
                AFE_READ(ASM2_IN_BUF_MON0),
                AFE_READ(ASM2_IN_BUF_MON1),
                AFE_READ(ASM2_IIR_CRAM_ADDR),
                AFE_READ(ASM2_IIR_CRAM_DATA),
                AFE_READ(ASM2_OUT_BUF_MON0),
                AFE_READ(ASM2_IBUF_SADR),
                AFE_READ(ASM2_IBUF_SIZE),
                AFE_READ(ASM2_OBUF_SADR),
                AFE_READ(ASM2_OBUF_SIZE)
            );
            assert(0 && "hwsrc halt ----------------------------------------------");
        }
        // hal_nvic_restore_interrupt_mask(mask);
        AudioCheckTransformHandle(sink->transform);
    }
    if (afe_block->u4asrcflag) {
        AFE_WRITE(ASM2_CH01_IBUF_WRPNT, buffer_info->WriteOffset + AFE_READ(ASM2_IBUF_SADR));
        pre_oro = AFE_READ(ASM2_CH01_OBUF_RDPNT) - AFE_READ(ASM2_OBUF_SADR);
        pre_owo = AFE_READ(ASM2_CH01_OBUF_WRPNT) - AFE_READ(ASM2_OBUF_SADR);
    }

}
#endif

ATTR_TEXT_IN_IRAM void afe_dl3_interrupt_handler(void)
{
    //uint32_t mask = 0;
    uint32_t pre_offset = 0;
    int32_t  hw_current_read_idx = 0;
    uint32_t dl_base_addr = 0;
    SINK     sink = Sink_blks[SINK_TYPE_AUDIO_DL3];
    //afe_block_t *afe_block = &sink->param.audio.AfeBlkControl;
    BUFFER_INFO *buffer_info = &sink->streamBuffer.BufferInfo;
    AUDIO_PARAMETER *runtime = &sink->param.audio;
    dl_base_addr = AFE_GET_REG(AFE_DL3_BASE);
    hw_current_read_idx = AFE_GET_REG(AFE_DL3_CUR);
    if (runtime->irq_exist == false) {
        runtime->irq_exist = true;
        DSP_MW_LOG_I("DSP afe dl3 interrupt start\r\n", 0);
#if defined(AIR_WIRELESS_MIC_RX_ENABLE)
        if ((sink->scenario_type >= AUDIO_SCENARIO_TYPE_WIRELESS_MIC_RX_UL_LINE_OUT) && (sink->scenario_type <= AUDIO_SCENARIO_TYPE_WIRELESS_MIC_RX_UL_I2S_SLV_OUT_0))
        {
            wireless_mic_rx_playen_disable(sink);
        }
#endif /* AIR_WIRELESS_MIC_RX_ENABLE */
    }
    if (1) {//afe_get_memory_path_enable(AUDIO_DIGITAL_BLOCK_MEM_DL3)
        //DSP_MW_LOG_I("TEST TT\r\n", 0);
        //hal_nvic_save_and_set_interrupt_mask(&mask);
        if (hw_current_read_idx == 0) { //should chk setting if =0
            hw_current_read_idx = WORD_ALIGN(dl_base_addr);
        }
        pre_offset = buffer_info->ReadOffset;

        buffer_info->ReadOffset = hw_current_read_idx - dl_base_addr;

        #if defined(AIR_ADVANCED_PASSTHROUGH_ENABLE)
        if ((sink->param.audio.scenario_id == AUDIO_TRANSMITTER_ADVANCED_PASSTHROUGH) && (sink->param.audio.scenario_sub_id == AUDIO_TRANSMITTER_ADVANCED_PASSTHROUGH_HEARING_AID))
        {
            /* In this case, we do nothing to reduce mips. */
        }
        else
        #endif /* defined(AIR_ADVANCED_PASSTHROUGH_ENABLE) */
        /*Clear up last time used memory */
        {
            if (sink->type != SINK_TYPE_AUDIO_DL3) {
                if (buffer_info->ReadOffset >= pre_offset) {
                    memset((void *)(dl_base_addr + pre_offset), 0, buffer_info->ReadOffset - pre_offset);
                } else {
                    memset((void *)(dl_base_addr + pre_offset), 0, buffer_info->length - pre_offset);
                    memset((void *)dl_base_addr, 0, buffer_info->ReadOffset);
                }
            }
        }
        if (afe_offset_overflow_check(pre_offset, buffer_info, true)) {
                DSP_MW_LOG_W("DL3 SRAM Empty play pR:%d R:%d W:%d==============", 3, pre_offset, buffer_info->ReadOffset, buffer_info->WriteOffset);
                if (pre_offset < buffer_info->ReadOffset) {
                    buffer_info->WriteOffset = (buffer_info->ReadOffset * 2 - pre_offset) % buffer_info->length;
                } else {
                    buffer_info->WriteOffset = (buffer_info->ReadOffset * 2 + (buffer_info->length - pre_offset)) % buffer_info->length;
                }
                if (buffer_info->WriteOffset > buffer_info->ReadOffset) {
                    memset((void *)hw_current_read_idx, 0, buffer_info->WriteOffset - buffer_info->ReadOffset);
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
                    if (sink->param.audio.channel_num == 4) {
                        memset((void *)(buffer_info->startaddr[1] + buffer_info->ReadOffset), 0, buffer_info->WriteOffset - buffer_info->ReadOffset);
                        if (sink->param.audio.channel_num == 6) {
                            memset((void *)(buffer_info->startaddr[2] + buffer_info->ReadOffset), 0, buffer_info->WriteOffset - buffer_info->ReadOffset);
                        }
                    }
#endif
                } else {
                    memset((void *)hw_current_read_idx, 0, buffer_info->length - buffer_info->ReadOffset);
                    memset((void *)dl_base_addr, 0, buffer_info->WriteOffset);
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
                    if (sink->param.audio.channel_num >= 4) {
                        memset((void *)(buffer_info->startaddr[1] + buffer_info->ReadOffset), 0, buffer_info->length - buffer_info->ReadOffset);
                        memset((void *)buffer_info->startaddr[1], 0, buffer_info->WriteOffset);
                        if (sink->param.audio.channel_num >= 6) {
                            memset((void *)(buffer_info->startaddr[2] + buffer_info->ReadOffset), 0, buffer_info->length - buffer_info->ReadOffset);
                            memset((void *)buffer_info->startaddr[2], 0, buffer_info->WriteOffset);
                        }
                    }
#endif
                }
        } else if (buffer_info->ReadOffset != buffer_info->WriteOffset) {
            buffer_info->bBufferIsFull = FALSE;
        }

#if defined (AIR_WIRED_AUDIO_ENABLE)
        if ((sink->transform != NULL)
            && (sink->transform->source->type >= SOURCE_TYPE_AUDIO_TRANSMITTER_MIN) && (sink->transform->source->type <= SOURCE_TYPE_AUDIO_TRANSMITTER_MAX)
            && (sink->transform->source->param.data_dl.scenario_type == AUDIO_TRANSMITTER_WIRED_AUDIO)
            && ((sink->transform->source->param.data_dl.scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) || (sink->transform->source->param.data_dl.scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1))) {
            sink->transform->source->param.data_dl.scenario_param.usb_in_local_param.is_afe_irq_comming = true;
            //sink->transform->source->param.data_dl.scenario_param.usb_in_local_param.is_dummy_data = false;
        }
        if ((sink->transform != NULL) && (sink->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_MAINSTREAM)){
            sink->transform->source->param.virtual_para.is_processed = false;
        }
#endif
#if defined(AIR_WIRELESS_MIC_RX_ENABLE)
        BTCLK bt_clk;
        BTPHASE bt_phase;
        MCE_GetBtClk(&bt_clk,&bt_phase, BT_CLK_Offset);
        if ((sink->scenario_type >= AUDIO_SCENARIO_TYPE_WIRELESS_MIC_RX_UL_LINE_OUT) && (sink->scenario_type <= AUDIO_SCENARIO_TYPE_WIRELESS_MIC_RX_UL_I2S_SLV_OUT_0))
        {
            wireless_mic_rx_ul_handle_t *rx_handle = (wireless_mic_rx_ul_handle_t *)(sink->transform->source->param.bt_common.scenario_param.dongle_handle);
            if (rx_handle->source_info.bt_in.frame_interval == 1000) {
                hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &(rx_handle->ccni_in_gpt_count));
                rx_handle->ccni_in_bt_count = bt_clk;
            } else {
                if (wireless_mic_rx_ul_fetch_time_is_arrived(rx_handle, bt_clk, bt_phase)) {
                    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &(rx_handle->ccni_in_gpt_count));
                    rx_handle->ccni_in_bt_count = bt_clk;
                }
            }
            if ((rx_handle->source_info.bt_in.frame_interval == 1000) && (rx_handle->stream_status == WIRELESS_MIC_RX_STREAM_INIT)) {
                rx_handle->first_packet_status = WIRELESS_MIC_RX_UL_FIRST_PACKET_PLAYED;
                rx_handle->stream_status = WIRELESS_MIC_RX_STREAM_RUNNING;
                rx_handle->is_play_en_trigger = false;
            }
#if 0
            DSP_MW_LOG_I("[Wireless MIC RX][UL][handle 0x%x] DSP afe dl3 interrupt trigger, curr BT: 0x%x 0x%x, fetch BT: 0x%x 0x%x\r\n", 5,
                            rx_handle,
                            bt_clk & 0xFFFFFFFC, bt_phase + (bt_clk & 0x3) * 625,
                            rx_handle->source_info.bt_in.fetch_anchor,
                            rx_handle->source_info.bt_in.fetch_anchor_phase);
#endif
        }
#endif /* AIR_WIRELESS_MIC_RX_ENABLE */
#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
        BTCLK bt_clk;
        BTPHASE bt_phase;
        MCE_GetBtClk(&bt_clk,&bt_phase, BT_CLK_Offset);
        if ((sink->scenario_type >= AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_UL_LINE_OUT) && (sink->scenario_type <= AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_UL_I2S_SLV_OUT_0))
        {
            ull_audio_v2_dongle_ul_handle_t *dongle_handle = (ull_audio_v2_dongle_ul_handle_t *)(sink->transform->source->param.bt_common.scenario_param.dongle_handle);
            if (ull_audio_v2_dongle_ul_fetch_time_is_arrived(dongle_handle, bt_clk))
            {
                hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &(dongle_handle->ccni_in_gpt_count));
                dongle_handle->ccni_in_bt_count = bt_clk;
            }
        }
#endif /* defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE) */

#if defined(AIR_WIRED_AUDIO_ENABLE) && !defined(AIR_DCHS_MODE_ENABLE)
        if (sink->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_IN) {
            /* For line in, use only source afe to wakeup the stream */
            //hal_nvic_restore_interrupt_mask(mask);
            return;
        }
#endif

        #if defined(AIR_ADVANCED_PASSTHROUGH_ENABLE)
        if ((sink->param.audio.scenario_id == AUDIO_TRANSMITTER_ADVANCED_PASSTHROUGH) && (sink->param.audio.scenario_sub_id == AUDIO_TRANSMITTER_ADVANCED_PASSTHROUGH_HEARING_AID))
        {
            /* In this case, the stream must be trigger by AFE source irq. So we do nothing here. */
        }
        else
        #endif /* defined(AIR_ADVANCED_PASSTHROUGH_ENABLE) */
        {
            AudioCheckTransformHandle(sink->transform);
        }
#if defined (AIR_WIRED_AUDIO_ENABLE)
        for (uint8_t i = SOURCE_TYPE_AUDIO_TRANSMITTER_MIN; i < SOURCE_TYPE_AUDIO_TRANSMITTER_MAX; i++) {
            if((Source_blks[i] != NULL) &&
                ((Source_blks[i]->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_0)||(Source_blks[i]->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_1))) {
                if(Source_blks[i]->transform != NULL){
                    AudioCheckTransformHandle(Source_blks[i]->transform);
                }
            }
        }
        if(sink->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_MAINSTREAM){
            AudioCheckTransformHandle(sink->transform);
        }
#endif
        //hal_nvic_restore_interrupt_mask(mask);
    }
}

ATTR_TEXT_IN_IRAM void afe_dl12_interrupt_handler(void)
{
    // uint32_t gpt_count;
    // hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count);
    // DSP_MW_LOG_I("dl12 interrupt, %u", 1, gpt_count);
    //hal_gpio_toggle_pin(HAL_GPIO_17);
    //uint32_t mask = 0;
    uint32_t pre_offset = 0;
    int32_t  hw_current_read_idx = 0;
    uint32_t dl_base_addr = 0;
#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
    SINK     sink = Sink_blks[SINK_TYPE_AUDIO];
    dl_base_addr = AFE_GET_REG(AFE_DL1_BASE);
    hw_current_read_idx = AFE_GET_REG(AFE_DL1_CUR);
    SOURCE source = sink->transform->source;
    if (sink->param.audio.AfeBlkControl.u4asrcflag) {
        if (sink->scenario_type == AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_UL_I2S_SLV_OUT_0) {
            ull_audio_v2_dongle_ul_handle_t *dongle_handle = source->param.bt_common.scenario_param.dongle_handle;
            dl_base_addr = AFE_GET_REG(dongle_handle->sink_info.i2s_slv_out.afe_base_addr);
            //hw_current_write_idx = AFE_GET_REG(dongle_handle->sink_info.i2s_slv_out.afe_cur_addr);
            hw_current_read_idx  = AFE_GET_REG(dongle_handle->sink_info.i2s_slv_out.afe_cur_addr);
        }
    }
#else
    SINK sink = Sink_blks[SINK_TYPE_AUDIO_DL12];
    dl_base_addr = AFE_GET_REG(AFE_DL12_BASE);
    hw_current_read_idx = AFE_GET_REG(AFE_DL12_CUR);
#endif
    BUFFER_INFO *buffer_info = &sink->streamBuffer.BufferInfo;
    AUDIO_PARAMETER *runtime = &sink->param.audio;
    if (runtime->irq_exist == false) {
        runtime->irq_exist = true;
        DSP_MW_LOG_I("DSP afe dl12 interrupt start\r\n", 0);
        afe_dl_playen_release(sink);
    }
    if (1) {//afe_get_memory_path_enable(AUDIO_DIGITAL_BLOCK_MEM_DL12)
        //DSP_MW_LOG_I("TEST TT\r\n", 0);
        //hal_nvic_save_and_set_interrupt_mask(&mask);
        #ifdef AIR_MIXER_STREAM_ENABLE
        g_mixer_stream_mix_count ++;
        g_mixer_stream_process_count ++;
        //DSP_MW_LOG_W("[Mixer Stream]wo:%d,ro:%d,length:%d,g_mixer_stream_process_count:%d", 4, buffer_info->WriteOffset, buffer_info->ReadOffset, buffer_info->length,g_mixer_stream_process_count);
        #endif
        if (hw_current_read_idx == 0) { //should chk setting if =0
            hw_current_read_idx = WORD_ALIGN(dl_base_addr);
        }
        pre_offset = buffer_info->ReadOffset;
        buffer_info->ReadOffset = hw_current_read_idx - dl_base_addr;
        /*Clear up last time used memory */
        if (dl_base_addr != NULL) {
            if (buffer_info->ReadOffset >= pre_offset) {
                memset((void *)(dl_base_addr + pre_offset), 0, buffer_info->ReadOffset - pre_offset);
            } else {
                memset((void *)(dl_base_addr + pre_offset), 0, buffer_info->length - pre_offset);
                memset((void *)dl_base_addr, 0, buffer_info->ReadOffset);
            }
        }
        if (afe_offset_overflow_check(pre_offset, buffer_info, true)) {
            DSP_MW_LOG_W("DL12 SRAM Empty play pR:%d R:%d W:%d==============", 3, pre_offset, buffer_info->ReadOffset, buffer_info->WriteOffset);
            U32 pre_write_offset = buffer_info->WriteOffset;
            UNUSED(pre_write_offset);
            if (pre_offset < buffer_info->ReadOffset) {
                buffer_info->WriteOffset = (buffer_info->ReadOffset * 2 - pre_offset) % buffer_info->length;
            } else {
                buffer_info->WriteOffset = (buffer_info->ReadOffset * 2 + (buffer_info->length - pre_offset)) % buffer_info->length;
            }
            #ifdef AIR_MIXER_STREAM_ENABLE
            if(sink->scenario_type == AUDIO_SCENARIO_TYPE_MIXER_STREAM){
                sink->param.audio.sram_empty_fill_size += ((buffer_info->WriteOffset >= pre_write_offset) ? (buffer_info->WriteOffset - pre_write_offset) : (buffer_info->length - pre_write_offset + buffer_info->WriteOffset));
                DSP_MW_LOG_W("[DCHS DL] DL12 SRAM Empty, fill silence:%d", 1, ((buffer_info->WriteOffset >= pre_write_offset) ? (buffer_info->WriteOffset - pre_write_offset) : (buffer_info->length - pre_write_offset + buffer_info->WriteOffset)));
            }
            #endif
            if (buffer_info->WriteOffset > buffer_info->ReadOffset) {
                memset((void *)hw_current_read_idx, 0, buffer_info->WriteOffset - buffer_info->ReadOffset);
            } else {
                memset((void *)hw_current_read_idx, 0, buffer_info->length - buffer_info->ReadOffset);
                memset((void *)dl_base_addr, 0, buffer_info->WriteOffset);
            }
        } else if (buffer_info->ReadOffset != buffer_info->WriteOffset) {
            buffer_info->bBufferIsFull = FALSE;
        }
#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
        BTCLK bt_clk;
        BTPHASE bt_phase;
        MCE_GetBtClk(&bt_clk,&bt_phase, BT_CLK_Offset);
        if (sink->scenario_type <= AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_UL_I2S_SLV_OUT_0)
        {
            ull_audio_v2_dongle_ul_handle_t *dongle_handle = (ull_audio_v2_dongle_ul_handle_t *)(sink->transform->source->param.bt_common.scenario_param.dongle_handle);
            if (ull_audio_v2_dongle_ul_fetch_time_is_arrived(dongle_handle, bt_clk))
            {
                hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &(dongle_handle->ccni_in_gpt_count));
                dongle_handle->ccni_in_bt_count = bt_clk;
            }
        }
#endif /* defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE) */
        AudioCheckTransformHandle(sink->transform);
        //hal_nvic_restore_interrupt_mask(mask);
    }
}

void afe_dl2_query_data_amount(uint32_t *sink_data_count, uint32_t *afe_sram_data_count)
{
    volatile SINK sink = Sink_blks[SINK_TYPE_VP_AUDIO];
    afe_block_t *afe_block = &sink->param.audio.AfeBlkControl;
    int32_t hw_current_read_idx = AFE_GET_REG(AFE_DL2_CUR);
    afe_block->u4ReadIdx = hw_current_read_idx - AFE_GET_REG(AFE_DL2_BASE);

    //AFE DL2 SRAM data amount
    if (afe_block->u4WriteIdx > afe_block->u4ReadIdx) {
        *afe_sram_data_count = afe_block->u4WriteIdx - afe_block->u4ReadIdx;
    } else {
        *afe_sram_data_count = afe_block->u4BufferSize + afe_block->u4WriteIdx - afe_block->u4ReadIdx;
    }

    //Sink audio data amount
    *sink_data_count = sink->streamBuffer.BufferInfo.length - SinkSlack(sink);
}

#if defined(AIR_DAC_MODE_RUNTIME_CHANGE)
void afe_send_dac_deactive_status_ccni(void)
{
    hal_ccni_message_t msg;
    bool from_isr = HAL_NVIC_QUERY_EXCEPTION_NUMBER > HAL_NVIC_NOT_EXCEPTION ? true : false;
    memset((void *)&msg, 0, sizeof(hal_ccni_message_t));
    msg.ccni_message[0] = ((MSG_DSP2MCU_AUDIO_AMP<< 16) | 0x2);
    msg.ccni_message[1] = AFE_COMMON_PARA_DAC_DEACTIVE;
    while (aud_msg_tx_handler(msg, 0, from_isr) != AUDIO_MSG_STATUS_OK) {
        DSP_MW_LOG_E("[DSP DAC Deactive] afe_send_dac_deactive_status_ccni [0x%x], ack fail from isr %d\r\n", 2, msg.ccni_message[0], from_isr);
    }
    DSP_MW_LOG_I("[DSP DAC Deactive] afe_send_dac_deactive_status_ccni [0x%x], ack success from isr %d\r\n", 2, msg.ccni_message[0], from_isr);
}
#endif

void afe_send_amp_status_ccni(bool enable)
{
    hal_ccni_message_t msg;
    bool from_isr = HAL_NVIC_QUERY_EXCEPTION_NUMBER > HAL_NVIC_NOT_EXCEPTION ? true : false;
    memset((void *)&msg, 0, sizeof(hal_ccni_message_t));
    msg.ccni_message[0] = ((MSG_DSP2MCU_AUDIO_AMP << 16) | enable);
    while (aud_msg_tx_handler(msg, 0, from_isr) != AUDIO_MSG_STATUS_OK) {
        DSP_MW_LOG_E("[DSP AMP] afe_send_amp_status_ccni [0x%x], ack fail from isr %d\r\n", 2, msg.ccni_message[0], from_isr);
    }
    DSP_MW_LOG_I("[DSP AMP] afe_send_amp_status_ccni [0x%x], ack success from isr %d\r\n", 2, msg.ccni_message[0], from_isr);
}

#ifdef AIR_SILENCE_DETECTION_ENABLE
void afe_send_silence_status_ccni(bool SilenceFlag)
{
    hal_ccni_message_t msg;
    uint32_t status;
    memset((void *)&msg, 0, sizeof(hal_ccni_message_t));
    msg.ccni_message[0] = ((MSG_DSP2MCU_BT_AUDIO_DL_SILENCE_DETECTION_FEEDBACK << 16) | SilenceFlag);
    status = aud_msg_tx_handler(msg, 0, FALSE);

    DSP_MW_LOG_I("[SD]Silence Detection afe_send_silence_status_ccni:%d \r\n", 1, SilenceFlag);
}
#endif

#ifdef ENABLE_AMP_TIMER

bool afe_amp_open_handler(uint32_t samplerate)
{
#if 0
    return true;
#else
    bool reboot_dac;
    reboot_dac = fw_amp_timer_stop(samplerate);
    afe_send_amp_status_ccni(true);
    return reboot_dac;
#endif
}

bool afe_amp_closure_handler(void)
{
#if 0
    return true;
#else
    if (fw_amp_get_status() == FW_AMP_TIMER_END) {
        fw_amp_set_status(FW_AMP_TIMER_STOP);
        return true;
    } else {
        //Set amp timer
        fw_amp_timer_start();
        return false;
    }
#endif
}

hal_amp_function_t afe_amp_ops = {
    .open_handler       = afe_amp_open_handler,
    .closure_handler    = afe_amp_closure_handler,
};

/*Hook AFE Amp handler.*/
void afe_register_amp_handler(void)
{
    fw_amp_init_semaphore();
    fw_amp_init_timer();
    hal_audio_afe_register_amp_handle(&afe_amp_ops);
}
#endif

/*Calculate greatest common factor*/
uint32_t audio_get_gcd(uint32_t m, uint32_t n)
{
    while (n != 0) {
        uint32_t r = m % n;
        m = n;
        n = r;
    }
    return m;
}

void afe_set_asrc_ul_configuration_parameters(SOURCE source, afe_src_configuration_p asrc_config)
{
    UNUSED(source);
    UNUSED(asrc_config);
#if 1//modify for ab1568
    uint32_t device_rate;
    asrc_config->ul_mode = true;
    asrc_config->stereo = (source->param.audio.channel_num >= 2);
    asrc_config->hw_update_obuf_rdpnt = false;

    if (source->param.audio.audio_device == HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) {
        asrc_config->mode = AFE_SRC_TRACKING_MODE_RX;
        asrc_config->tracking_mode = MEM_ASRC_TRACKING_MODE_RX;
        asrc_config->tracking_clock = afe_set_asrc_tracking_clock(source->param.audio.audio_interface);
        device_rate = source->param.audio.rate;
    } else {
        asrc_config->tracking_mode = MEM_ASRC_NO_TRACKING;
        asrc_config->mode = AFE_SRC_NO_TRACKING;
        device_rate = 0;
    }
    if (device_rate >= source->param.audio.src_rate) {
        asrc_config->input_buffer.offset = device_rate / source->param.audio.src_rate * 16; // ratio = src_in / src_out
    } else {
        asrc_config->input_buffer.offset = 16 / (source->param.audio.src_rate / device_rate);
    }
#ifdef AIR_BTA_IC_STEREO_HIGH_G3
    asrc_config->input_buffer.offset += 128; // add 16 to avoid hwsrc can't convert total. // for 96k, the value should be more than 90
#else
    asrc_config->input_buffer.offset += 64; // add 16 to avoid hwsrc can't convert total.
#endif
    asrc_config->input_buffer.addr = source->param.audio.AfeBlkControl.phys_buffer_addr;
    asrc_config->input_buffer.size = source->param.audio.AfeBlkControl.u4asrc_buffer_size;
    asrc_config->input_buffer.rate = device_rate;
    // asrc_config->input_buffer.offset = 32;////((((source->param.audio.period+5)*source->param.audio.format_bytes*asrc_config->input_buffer.rate*((asrc_config->stereo==true) ? 2 : 1)/1000)+ 7) & (~7))%asrc_config->input_buffer.size;
    asrc_config->input_buffer.format = source->param.audio.format;

    asrc_config->output_buffer.addr = source->param.audio.AfeBlkControl.phys_buffer_addr + source->param.audio.AfeBlkControl.u4asrc_buffer_size;
    asrc_config->output_buffer.size = source->param.audio.buffer_size;
    asrc_config->output_buffer.rate = source->param.audio.src_rate;
    asrc_config->output_buffer.offset = source->streamBuffer.BufferInfo.ReadOffset;
    asrc_config->output_buffer.format = source->param.audio.format;

    DSP_MW_LOG_I("DSP UL asrc tracking %d, input addr 0x%x offset %d size %d rate %d, output addr 0x%x offset %d size %d rate %d", 9,
        asrc_config->tracking_mode,
        asrc_config->input_buffer.addr,
        asrc_config->input_buffer.offset,
        asrc_config->input_buffer.size,
        asrc_config->input_buffer.rate,
        asrc_config->output_buffer.addr,
        asrc_config->output_buffer.offset,
        asrc_config->output_buffer.size,
        asrc_config->output_buffer.rate
        );
#endif
}

void afe_set_asrc_dl_configuration_parameters(SINK sink, afe_src_configuration_p asrc_config)
{
    asrc_config->ul_mode = false;
    asrc_config->mode = AFE_SRC_NO_TRACKING;
    asrc_config->stereo = (sink->param.audio.channel_num >= 2);
    asrc_config->hw_update_obuf_rdpnt = true;


    asrc_config->input_buffer.addr = sink->param.audio.AfeBlkControl.phys_buffer_addr + sink->param.audio.AfeBlkControl.u4asrc_buffer_size;
    asrc_config->input_buffer.size = sink->param.audio.buffer_size;
    asrc_config->input_buffer.rate = sink->param.audio.src_rate;
    asrc_config->input_buffer.offset = sink->streamBuffer.BufferInfo.WriteOffset;
    asrc_config->input_buffer.format = sink->param.audio.format;

    asrc_config->output_buffer.addr = sink->param.audio.AfeBlkControl.phys_buffer_addr;
    asrc_config->output_buffer.size = sink->param.audio.AfeBlkControl.u4asrc_buffer_size;
    asrc_config->output_buffer.rate = 0;
    asrc_config->output_buffer.offset = ((((uint32_t)(sink->param.audio.period + 5) * sink->param.audio.format_bytes * asrc_config->output_buffer.rate * ((asrc_config->stereo == true) ? 2 : 1) / 1000) + 16 + 7) & ~7) % asrc_config->output_buffer.size ;
    asrc_config->output_buffer.format = sink->param.audio.format;


    DSP_MW_LOG_I("DSP DL asrc in rate:%d, out rate:%d\r\n", 2, asrc_config->input_buffer.rate, asrc_config->output_buffer.rate);
}

void afe_send_ccni_anc_switch_filter(uint32_t id)
{
    hal_ccni_message_t msg;
    uint32_t status;
    memset((void *)&msg, 0, sizeof(hal_ccni_message_t));
    msg.ccni_message[0] = ((MSG_DSP2MCU_COMMON_AUDIO_ANC_SWITCH << 16));
    msg.ccni_message[1] = id;
    status = aud_msg_tx_handler(msg, 0, FALSE);

    DSP_MW_LOG_I("DSP send ANC switch ccni:%d \r\n", 1, id);
}

bool afe_audio_device_ready(SOURCE_TYPE source_type, SINK_TYPE sink_type)
{
    if (source_type >= SOURCE_TYPE_MAX || sink_type >= SINK_TYPE_MAX) {
        return false;
    }
    volatile SINK sink = Sink_blks[sink_type];
    if (source_type == SOURCE_TYPE_N9SCO && sink_type == SINK_TYPE_AUDIO) { //esco DL
        //check hwsrc2 rx tracking ready
        if (sink->param.audio.audio_device == HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE && sink->param.audio.clk_skew_mode == CLK_SKEW_DISSABLE) {
#ifdef AIR_HWSRC_RX_TRACKING_ENABLE
            DSP_MW_LOG_I("[HWSRC]: check ASM2_FREQUENCY_2 = 0x%x \r\n", 1, AFE_READ(ASM2_FREQUENCY_2));
            if (AFE_READ(ASM2_FREQUENCY_2) == 0xa00000 || AFE_READ(ASM2_FREQUENCY_2) == 0x0) {
                return false;
            }
#endif
        }
    } else {
        //for other device extend
    }
    return true;
}
uint32_t *p_rx_eq = NULL;
uint32_t tmp = 0;
bool g_rx_eq_update = 0;
void dsp_sync_callback_hfp(cm4_dsp_audio_sync_action_type_t request_action_id, void *user_data)
{
    uint32_t *sync_info = (uint32_t *)((cm4_dsp_audio_sync_request_param_t *)user_data)->nvkey_addr;
    if (request_action_id == MCU2DSP_SYNC_REQUEST_SET_RX_EQ) {
        tmp = *sync_info;
        p_rx_eq = (uint32_t *)hal_memview_cm4_to_dsp0((uint32_t)tmp);
        DSP_MW_LOG_I("[DSP SYNC] HFP SET_RX_EQ %x %x %x %x %x",5,p_rx_eq,&p_rx_eq,*p_rx_eq,tmp,*sync_info);
        g_rx_eq_update = TRUE;
    }
}

void dsp_sync_callback_ble(cm4_dsp_audio_sync_action_type_t request_action_id, void *user_data)
{
    uint32_t *sync_info = (uint32_t *)((cm4_dsp_audio_sync_request_param_t *)user_data)->nvkey_addr;
    if (request_action_id == MCU2DSP_SYNC_REQUEST_SET_RX_EQ) {
        tmp = *sync_info;
        p_rx_eq = (uint32_t *)hal_memview_cm4_to_dsp0((uint32_t)tmp);
        DSP_MW_LOG_I("[DSP SYNC] BLE SET_RX_EQ %x %x %x %x %x",5,p_rx_eq,&p_rx_eq,*p_rx_eq,tmp,*sync_info);
        g_rx_eq_update = TRUE;
    }
}
#ifdef AIR_ANC_ADAPTIVE_CLOCK_CONTROL_ENABLE
void dsp_sync_callback_adapt_anc(cm4_dsp_audio_sync_action_type_t request_action_id, void *user_data)
{
    UNUSED(user_data);
    if (request_action_id == MCU2DSP_SYNC_REQUEST_START) {
        dsp_adapt_anc_change_dma_state(ANC_FULL_ADAPT_CONTROL_AWAKE);
    } else if (request_action_id == MCU2DSP_SYNC_REQUEST_STOP) {
        dsp_adapt_anc_change_dma_state(ANC_FULL_ADAPT_CONTROL_SLEEP);
    }
}
#endif

#ifdef AIR_HEARTHROUGH_MAIN_ENABLE
void dsp_sync_callback_llf(cm4_dsp_audio_sync_action_type_t request_action_id, void *user_data)
{
    cm4_dsp_audio_sync_request_param_t *sync_info;
    sync_info = (cm4_dsp_audio_sync_request_param_t*)user_data;

    if (request_action_id == MCU2DSP_SYNC_REQUEST_SET_MUTE) {
        bool mute = sync_info->vol_gain_info.gain ? false : true;
        dsp_llf_mute_dl(mute);
    }
    DSP_MW_LOG_I("[LLF SYNC] action:%u, mute:%u", 2, request_action_id, (sync_info->vol_gain_info.gain ? 0 : 1));
}
#endif

