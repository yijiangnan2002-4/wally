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

/******************************************************************************
 *
 * Include
 *
 ******************************************************************************/
#include "preloader_pisplit.h"
#include "source_inter.h"
#include "stream_mixer.h"
//#include "sw_gain_interface.h"
#ifdef AIR_SOFTWARE_MIXER_ENABLE
#include "sw_mixer_interface.h"
#endif
#ifdef AIR_SOFTWARE_GAIN_ENABLE
#include "sw_gain_interface.h"
#endif
#include "stream_audio_driver.h"
#include "dsp_callback.h"
#include "transform_inter.h"
#include "sink_inter.h"
#include "hal_dwt.h"
#include "dsp_dump.h"
#include "hal_audio_register.h"
#include "bt_types.h"
#include "hal_audio_driver.h"
#include "bt_interface.h"
#include "hal_gpt.h"
#include "ch_select_interface.h"
#include "audio_transmitter_mcu_dsp_common.h"
#include "dsp_audio_msg.h"
#include "clk_skew.h"
#ifdef AIR_DCHS_MODE_ENABLE
#include "stream_dchs.h"
#include "dsp_mux_uart.h"
#endif

log_create_module(MIXER_STREAM,       PRINT_LEVEL_INFO);
#if 1
log_create_module(MIXER_STREAM_DEBUG, PRINT_LEVEL_WARNING);
#else
log_create_module(MIXER_STREAM_DEBUG, PRINT_LEVEL_INFO);
#endif

#define MIXER_STREAM_LOG_E(fmt, arg...) LOG_MSGID_E(MIXER_STREAM,       "[Mixer Stream] "fmt,##arg)
#define MIXER_STREAM_LOG_W(fmt, arg...) LOG_MSGID_W(MIXER_STREAM,       "[Mixer Stream] "fmt,##arg)
#define MIXER_STREAM_LOG_I(fmt, arg...) LOG_MSGID_I(MIXER_STREAM,       "[Mixer Stream] "fmt,##arg)
#define MIXER_STREAM_LOG_D(fmt, arg...) LOG_MSGID_I(MIXER_STREAM_DEBUG, "[Mixer Debug Stream] "fmt,##arg)

/******************************************************************************
 *
 * Extern Function
 *
 ******************************************************************************/
EXTERN VOID DSP_C2D_BufferCopy(VOID *DestBuf,
                               VOID *SrcBuf,
                               U32 CopySize,
                               VOID *SrcCBufStart,
                               U32 SrcCBufSize);

EXTERN VOID DSP_I2D_BufferCopy_16bit_mute(U16 *SrcBuf,
                                     U16 *DestBuf1,
                                     U16 *DestBuf2,
                                          U16  SampleSize,
                                          BOOL muteflag);

EXTERN VOID DSP_I2D_BufferCopy_32bit_mute(U32 *SrcBuf,
                                     U32 *DestBuf1,
                                     U32 *DestBuf2,
                                   U16  SampleSize,
                                   BOOL muteflag);


EXTERN VOID dsp_converter_16bit_to_32bit(S32 *des, S16 *src, U32  sample);
EXTERN VOID LC_Add_us_FromA(U32 n, BTTIME_STRU_PTR pa, BTTIME_STRU_PTR pb);
EXTERN SOURCE new_source(SOURCE_TYPE SourceType);
EXTERN void dsp_start_stream_out(mcu2dsp_start_param_p start_param, SINK sink);
EXTERN void dsp_start_stream_in(mcu2dsp_start_param_p start_param, SOURCE source);
EXTERN SOURCE dsp_open_stream_in(mcu2dsp_open_param_p open_param);
EXTERN SINK dsp_open_stream_out(mcu2dsp_open_param_p open_param);
EXTERN ATTR_TEXT_IN_IRAM_LEVEL_2 U32 SinkSizeAudioAfe(SINK sink);
EXTERN stream_feature_list_t stream_feature_list_mixer_stream[];
EXTERN bool hal_audio_src_set_start(afe_src_configuration_t *configuration, hal_audio_memory_sync_selection_t sync_select, hal_audio_control_status_t control);
EXTERN bool hal_audio_src_configuration(afe_src_configuration_t *configuration, hal_audio_control_status_t control);

/******************************************************************************
 *
 * Private Macro and Variable Declaration
 *
 ******************************************************************************/
#ifdef AIR_CPU_IN_SECURITY_MODE
#define CONN_BT_TIMCON_BASE 0xA0000000
#else
#define CONN_BT_TIMCON_BASE 0xB0000000
#endif
#define HWSRC_BUF_LENGTH     (4096)

#ifdef AIR_SOFTWARE_MIXER_ENABLE
static sw_mixer_member_t *mixer_member = NULL;
#endif
static sw_gain_port_t * mixer_stream_gain_port = NULL;

U32 g_mixer_stream_mix_count     = 0;
U32 g_mixer_stream_process_count = 0;
U64 g_mixer_stream_start_time    = 0;
U32 g_mixer_stream_timer_handle  = 0;
static U32 play_en_debug_timer   = 0;

mixer_scenario_msg_t  mix_scenarios_msg[MIX_SCENARIO_MAX] = {0};
U8 hwsrc_buffer[MEM_ASRC_NUM][HWSRC_BUF_LENGTH] = {0};

CONNECTION_IF mixer_stream_if;

#define MAGIC_NUMBER     (0xFFFFFF00)

typedef struct
{
    hal_audio_memory_t memory_agent;
    int32_t  vol_gain;
}gian_wating_set_msg_t;

static gian_wating_set_msg_t waiting_set_gain_msg[MIX_SCENARIO_MAX] = {0};

/******************************************************************************
 *
 * Private Function Define
 *
 ******************************************************************************/
source_ch_type_t check_idle_source_ch(void)
{
    source_ch_type_t ch_type;
    for(ch_type = MIX_SCENARIO_1; ch_type < MIX_SCENARIO_MAX; ch_type ++){
        if(mix_scenarios_msg[ch_type].mix_type == NO_MIX){
            return ch_type;
        }
    }
    return MIX_SCENARIO_MAX;
}

source_ch_type_t mixer_stream_get_source_ch_by_scenario(audio_scenario_type_t scenario_type)
{
    source_ch_type_t ch_type;
    for(ch_type = MIX_SCENARIO_1; ch_type < MIX_SCENARIO_MAX; ch_type ++){
        if(mix_scenarios_msg[ch_type].scenario_type == scenario_type){
            return ch_type;
        }
    }
    return MIX_SCENARIO_MAX;
}

audio_scenario_type_t mixer_stream_get_scenario_type_by_channel(source_ch_type_t ch_type)
{
    return mix_scenarios_msg[ch_type].scenario_type;
}

source_ch_type_t mixer_stream_get_source_ch_by_agent(hal_audio_memory_t memory_agent)
{
    source_ch_type_t ch_type;
    for(ch_type = MIX_SCENARIO_1; ch_type < MIX_SCENARIO_MAX; ch_type ++){
        if(mix_scenarios_msg[ch_type].memory_agent == memory_agent){
            return ch_type;
        }
    }
    return MIX_SCENARIO_MAX;
}

hal_audio_memory_t mixer_stream_get_agent_by_scenario(audio_scenario_type_t scenario_type)
{
    source_ch_type_t ch_type;
    for(ch_type = MIX_SCENARIO_1; ch_type < MIX_SCENARIO_MAX; ch_type ++){
        if(mix_scenarios_msg[ch_type].scenario_type == scenario_type){
            return mix_scenarios_msg[ch_type].memory_agent;
        }
    }
    return 0;
}

bool mixer_stream_check_waiting_mix(void)
{
    source_ch_type_t ch_type;
    for(ch_type = MIX_SCENARIO_1; ch_type < MIX_SCENARIO_MAX; ch_type ++){
        if(mix_scenarios_msg[ch_type].mix_type  == WAITING_MIX){
            return true;
        }
    }
    return false;
}

SINK_TYPE get_sink_type_by_agent(hal_audio_memory_t memory_agent)
{
    SINK_TYPE sink_type = SINK_TYPE_MAX;
    if(memory_agent == HAL_AUDIO_MEM1){
        sink_type = SINK_TYPE_AUDIO;
    }else if(memory_agent == HAL_AUDIO_MEM2){
        sink_type = SINK_TYPE_VP_AUDIO;
    }else if(memory_agent == HAL_AUDIO_MEM3){
        sink_type = SINK_TYPE_AUDIO_DL3;
    }
    AUDIO_ASSERT(sink_type != SINK_TYPE_MAX && "[Mixer Stream] no found sink type");
    return sink_type;
}

void mixer_stream_sw_gain_init(SOURCE source)
{
#ifdef AIR_SOFTWARE_GAIN_ENABLE
    /* sw gain config */
    int32_t default_gain;
    sw_gain_config_t default_config;
    default_config.resolution = RESOLUTION_32BIT;
    default_config.target_gain = -14400;
    default_config.up_step = 25;
    default_config.up_samples_per_step = 1;
    default_config.down_step = -25;
    default_config.down_samples_per_step = 1;
    mixer_stream_gain_port = stream_function_sw_gain_get_port(source);
    default_gain =  0xFFFFF15A;
    U32 ch_num;
    U32 total_ch_num = 2 * MAX_MIXER_NUM;
    stream_function_sw_gain_init(mixer_stream_gain_port, total_ch_num, &default_config);
    MIXER_STREAM_LOG_I("[SW Gain][Init]total channel %d, default gain = %ddB", 2, total_ch_num, default_gain);
    for(ch_num = 1; ch_num <= total_ch_num; ch_num ++){
        stream_function_sw_gain_configure_gain_target(mixer_stream_gain_port, ch_num, default_gain); //-120 = mute
    }
#endif /* AIR_SOFTWARE_GAIN_ENABLE */
}

static void sw_mixer_postcallback(sw_mixer_member_t *member, void *para, uint32_t *out_frame_size)
{
    UNUSED(member);
    UNUSED(out_frame_size);
    ((DSP_ENTRY_PARA_PTR)para)->out_channel_num = MIXER_STREAM_OUT_CHANNEL;
    MIXER_STREAM_LOG_D("[SW mix][postcallback]total channel %d", 1, ((DSP_ENTRY_PARA_PTR)para)->out_channel_num);
}

void mixer_stream_sw_mixer_init(SOURCE source)
{
    #ifdef AIR_SOFTWARE_MIXER_ENABLE
    /* sw mixer config */
    sw_mixer_callback_config_t       callback_config;
    sw_mixer_input_channel_config_t  in_ch_config;
    sw_mixer_output_channel_config_t out_ch_config;
    stream_function_sw_mixer_init(SW_MIXER_PORT_0);
    callback_config.preprocess_callback  = NULL;
    callback_config.postprocess_callback = sw_mixer_postcallback;
    in_ch_config.resolution = RESOLUTION_32BIT;
    in_ch_config.input_mode = SW_MIXER_CHANNEL_MODE_NORMAL;
    in_ch_config.total_channel_number = 2 * MAX_MIXER_NUM;
    in_ch_config.buffer_size = source->param.audio.frame_size * source->param.audio.format_bytes;
    out_ch_config.total_channel_number = MIXER_STREAM_OUT_CHANNEL;
    out_ch_config.resolution = RESOLUTION_32BIT;
    mixer_member = stream_function_sw_mixer_member_create((void *)source,
                                                                         SW_MIXER_MEMBER_MODE_NO_BYPASS,
                                                                         &callback_config,
                                                                         &in_ch_config,
                                                                         &out_ch_config);

    stream_function_sw_mixer_member_register(SW_MIXER_PORT_0, mixer_member, false);
    /* do connections */
    U32 i;
    for(i = 1; i <= (MAX_MIXER_NUM * 2); i++){
        sw_mixer_channel_attribute_t ch_attr = ((i == 1 || i == 2) ? SW_MIXER_CHANNEL_ATTRIBUTE_MAIN : SW_MIXER_CHANNEL_ATTRIBUTE_NORMAL);
        U32 out_ch = (i % 2 ? 1 : 2);//L ch:1, R ch:2
        stream_function_sw_mixer_channel_connect(mixer_member, i, ch_attr, mixer_member, out_ch);
        MIXER_STREAM_LOG_D("[SW mix] in ch:%d, ch_attr:%d, out ch:%d", 3, i, ch_attr, out_ch);
    }
    #endif//AIR_SOFTWARE_MIXER_ENABLE
}

U32 mixer_get_ch_data_size(source_ch_type_t ch_type)
{
    if(mix_scenarios_msg[ch_type].hwsrc_enable){
        afe_mem_asrc_id_t hwsrc_id = mix_scenarios_msg[ch_type].hwsrc_id;
        U32 addr_offset = hwsrc_id * 0x100;
        U32 oro  = AFE_READ(ASM_CH01_OBUF_RDPNT + addr_offset) - AFE_READ(ASM_OBUF_SADR + addr_offset);
        U32 owo = AFE_READ(ASM_CH01_OBUF_WRPNT + addr_offset) - AFE_READ(ASM_OBUF_SADR + addr_offset);
        U32 length   = AFE_READ(ASM_OBUF_SIZE + addr_offset);
        U32 data_size = (owo > oro) ? (owo - oro) : (length - oro + owo);
        return data_size;
    }else{
        BUFFER_INFO_PTR buf_info = mix_scenarios_msg[ch_type].sink_buf_info;
        AUDIO_ASSERT(buf_info != NULL && "[get ch size]sink buf == NULL");
        U32 owo     = buf_info->WriteOffset;
        U32 oro     = buf_info->ReadOffset;
        U32 length  = buf_info->length;
        U32 data_size = (owo > oro) ? (owo - oro) : (length - oro + owo);
        return data_size;
    }
    return 0;
}

void source_ch_i2d_buffer_copy(source_ch_type_t ch_type, U8 *dst_addr_L, U8 *dst_addr_R, U32 length, U8 channel_sel)
{
    bool hwsrc_enable = mix_scenarios_msg[ch_type].hwsrc_enable;
    U8 format_bytes   = mix_scenarios_msg[ch_type].format_bytes;
    afe_mem_asrc_id_t hwsrc_id = mix_scenarios_msg[ch_type].hwsrc_id;
    U32 addr_offset = hwsrc_id * 0x100;

    U32 oro          = hwsrc_enable ? 
                                    AFE_READ(ASM_CH01_OBUF_RDPNT + addr_offset) - AFE_READ(ASM_OBUF_SADR + addr_offset)
                                    : mix_scenarios_msg[ch_type].sink_buf_info->ReadOffset;
    U32 owo         = hwsrc_enable ?
                                    AFE_READ(ASM_CH01_OBUF_WRPNT + addr_offset) - AFE_READ(ASM_OBUF_SADR + addr_offset)
                                    : mix_scenarios_msg[ch_type].sink_buf_info->WriteOffset;
    U8 *src_addr    = hwsrc_enable ?
                                    (U8*)AFE_READ(ASM_CH01_OBUF_RDPNT + addr_offset)
                                    : mix_scenarios_msg[ch_type].sink_buf_info->startaddr[channel_sel] + mix_scenarios_msg[ch_type].sink_buf_info->ReadOffset;
    U8 *src_start   = hwsrc_enable ?
                                    (U8*)AFE_READ(ASM_OBUF_SADR + addr_offset)
                                    : mix_scenarios_msg[ch_type].sink_buf_info->startaddr[channel_sel];
    U32 unwrap_size = hwsrc_enable ?
                                    AFE_READ(ASM_OBUF_SIZE + addr_offset) - (AFE_READ(ASM_CH01_OBUF_RDPNT + addr_offset) - AFE_READ(ASM_OBUF_SADR + addr_offset))
                                    : mix_scenarios_msg[ch_type].sink_buf_info->length - mix_scenarios_msg[ch_type].sink_buf_info->ReadOffset;
    U32 buf_length  = hwsrc_enable ?
                                    AFE_READ(ASM_OBUF_SIZE + addr_offset)
                                    : mix_scenarios_msg[ch_type].sink_buf_info->length;
    U32 wrap_size   = unwrap_size >= length ? 0 : (length - unwrap_size);

    U8 ch_num = mix_scenarios_msg[ch_type].channel_num;
    MIXER_STREAM_LOG_D("[Source I2D]ch type=%d,length=%d,format_bytes=%d,owo=%d,oro=%d,unwrap_size=%d,wrap_size=%d, channel_num=%d,src_addr:0x%x", 9, ch_type,length,format_bytes,owo,oro,unwrap_size,wrap_size,ch_num,src_addr);
    if(mix_scenarios_msg[ch_type].channel_num == 1){
        DSP_C2D_BufferCopy((VOID *)dst_addr_L, (VOID *)src_addr, length, (VOID *)src_start, buf_length);
        memcpy((void *)dst_addr_R, (void *)dst_addr_L, length);
    }else{ //stere source data 
        if(owo > oro){
            if(format_bytes == 4){
                DSP_I2D_BufferCopy_32bit_mute((U32 *)src_addr,
                                                (U32 *)dst_addr_L,
                                                (U32 *)dst_addr_R,
                                                (length / sizeof(U32) / ch_num),
                                                false);
            }else{
                DSP_I2D_BufferCopy_16bit_mute((U16 *)src_addr,
                                                (U16 *)dst_addr_L,
                                                (U16 *)dst_addr_R,
                                                (length / sizeof(U16) / ch_num),
                                                false);
                //LOG_AUDIO_DUMP(src_addr, length, AUDIO_DUMP_TEST_ID_1);
            }
        }else{
            if(format_bytes == 4){
                DSP_I2D_BufferCopy_32bit_mute((U32 *)src_addr,
                                                (U32 *)dst_addr_L,
                                                (U32 *)dst_addr_R,
                                                (MIN(unwrap_size, length) / sizeof(U32) / ch_num),
                                                false);
                if(wrap_size){
                    DSP_I2D_BufferCopy_32bit_mute((U32 *)src_start,
                                                (U32 *)(dst_addr_L + unwrap_size/2),
                                                (U32 *)(dst_addr_R + unwrap_size/2),
                                                (wrap_size / sizeof(U32) / ch_num),
                                                 false);
                }
            }else{
                DSP_I2D_BufferCopy_16bit_mute((U16 *)src_addr,
                                                (U16 *)dst_addr_L,
                                                (U16 *)dst_addr_R,
                                                (MIN(unwrap_size, length) / sizeof(U16) / ch_num),
                                                false);
                //LOG_AUDIO_DUMP(src_addr, MIN(unwrap_size, length), AUDIO_DUMP_TEST_ID_1);
                if(wrap_size){
                    DSP_I2D_BufferCopy_16bit_mute((U16 *)src_start,
                                                (U16 *)(dst_addr_L + unwrap_size/2),
                                                (U16 *)(dst_addr_R + unwrap_size/2),
                                                (wrap_size / sizeof(U16) / ch_num),
                                                false);
                }
                //LOG_AUDIO_DUMP(src_start, wrap_size, AUDIO_DUMP_TEST_ID_1);
            }
        }
    }
    //LOG_AUDIO_DUMP(dst_addr_L, length / 2, AUDIO_DUMP_TEST_ID_1);
    if(format_bytes == 2){
        dsp_converter_16bit_to_32bit((int32_t *)dst_addr_L, (int16_t *)dst_addr_L, length / ch_num / sizeof(int16_t));
        dsp_converter_16bit_to_32bit((int32_t *)dst_addr_R, (int16_t *)dst_addr_R, length / ch_num / sizeof(int16_t));
        //LOG_AUDIO_DUMP(dst_addr_L, length, AUDIO_DUMP_TEST_ID_2);
    }
}

void reset_mixer_stream_status(void)
{
    g_mixer_stream_mix_count     = 0;
    g_mixer_stream_process_count = PRE_PROCESS_COUNT; //pre process
    g_mixer_stream_start_time    = 0;
    MIXER_STREAM_LOG_I("g_mixer_stream_process_count:%d", 1 , g_mixer_stream_process_count);
}

void resume_task(void)
{
    MIXER_STREAM_LOG_D("enter mixer_stream_resume_task", 0);
    volatile SOURCE mixer_source = Source_blks[SOURCE_TYPE_MIXER];
    if(mixer_source && mixer_source->transform && mixer_source->transform->source){
        AudioCheckTransformHandle(mixer_source->transform);
    }
}

bool check_ch_enough_data_size(source_ch_type_t ch_type)
{
    U32 ch_data_size = mixer_get_ch_data_size(ch_type);
    SOURCE source = Source_blks[SOURCE_TYPE_MIXER];
    U32 need_size    = source->param.audio.frame_size * mix_scenarios_msg[ch_type].format_bytes * mix_scenarios_msg[ch_type].channel_num;
    MIXER_STREAM_LOG_D("[source size]ch_type:%d,scenario type:%d, ch_data_size:%d, need_size:%d", 4, ch_type,
                        mix_scenarios_msg[ch_type].scenario_type,
                        ch_data_size,
                        need_size);
    return (ch_data_size >= need_size);
}

void source_ch_buf_drop(source_ch_type_t ch_type, U32 amount)
{
    if(!amount){
        return;
    }
    U32 addr_offset = mix_scenarios_msg[ch_type].hwsrc_id * 0x100;
    MIXER_STREAM_LOG_D("ch_type:%d, scenario:%d, hwsrc id:%d, addr_offset:0x%x, obuf size:%d", 5,ch_type, mix_scenarios_msg[ch_type].scenario_type, mix_scenarios_msg[ch_type].hwsrc_id,addr_offset,AFE_READ(ASM_OBUF_SIZE + addr_offset));
    if(mix_scenarios_msg[ch_type].hwsrc_enable){
        U32 update_ro = (amount + AFE_READ(ASM_CH01_OBUF_RDPNT + addr_offset) -  AFE_READ(ASM_OBUF_SADR + addr_offset))
                        % AFE_READ(ASM_OBUF_SIZE + addr_offset);
        AFE_WRITE(ASM_CH01_OBUF_RDPNT + addr_offset, update_ro + AFE_READ(ASM_OBUF_SADR + addr_offset));
    }else{
        if (mix_scenarios_msg[ch_type].sink_buf_info->bBufferIsFull == TRUE) {
            mix_scenarios_msg[ch_type].sink_buf_info->bBufferIsFull = FALSE;
        }
        mix_scenarios_msg[ch_type].sink_buf_info->ReadOffset = 
                    (mix_scenarios_msg[ch_type].sink_buf_info->ReadOffset + amount)
                      % (mix_scenarios_msg[ch_type].sink_buf_info->length);
    }
    MIXER_STREAM_LOG_D("[Source Drop] drop size:%d, ch type:%d, hwsrc id:%d, hwsrc ro:%d, hwsrc enable:%d, normal ro=%d", 6, 
                        amount,
                        ch_type,
                        mix_scenarios_msg[ch_type].hwsrc_id,
                        AFE_READ(ASM_CH01_OBUF_RDPNT + addr_offset) - AFE_READ(ASM_OBUF_SADR + addr_offset),
                        mix_scenarios_msg[ch_type].hwsrc_enable,
                        (mix_scenarios_msg[ch_type].sink_buf_info ? mix_scenarios_msg[ch_type].sink_buf_info->ReadOffset : 0));
}

void copy_source_ch_data_2_stream_buf(source_ch_type_t ch_type, U8 *dst_addr_L, U8 *dst_addr_R, U32 length)
{
    U32 ch_data_size = mixer_get_ch_data_size(ch_type);
    U32 drop_bytes = (mix_scenarios_msg[ch_type].scenario_type ==  AUDIO_SCENARIO_TYPE_VP) ?  //vp trigger stop is special,need SRAM Empty trigger
                        MIN(ch_data_size, mix_scenarios_msg[ch_type].prefill_silence_count)
                        : MIN((ch_data_size >= length ? ch_data_size - length : 0), mix_scenarios_msg[ch_type].prefill_silence_count);
    if(drop_bytes){
        source_ch_buf_drop(ch_type, drop_bytes);
        mix_scenarios_msg[ch_type].prefill_silence_count -= drop_bytes;
        MIXER_STREAM_LOG_W("[Source Drop] source ch: %d,scenario:%d,ch size:%d, length:%d, drop_size: %d for silence before. remain prefill:%d", 6, ch_type, mix_scenarios_msg[ch_type].scenario_type,ch_data_size, length, drop_bytes, mix_scenarios_msg[ch_type].prefill_silence_count);
    }
    ch_data_size = mixer_get_ch_data_size(ch_type);
    MIXER_STREAM_LOG_D("[Source Read] ch type:%d, ch_data_size:%d, length:%d, format_bytes=%d", 4, ch_type, ch_data_size, length, mix_scenarios_msg[ch_type].format_bytes);
    if(ch_data_size >= length){
        U32 i;
        U32 ch_num = mix_scenarios_msg[ch_type].channel_num;
        SOURCE source = Source_blks[SOURCE_TYPE_MIXER];
        TRANSFORM transform =  source->transform;
        volatile DSP_CALLBACK_PTR callback_ptr = DSP_Callback_Get(source, transform->sink);
        U8  channel_sel = 0;
        for(i = 0; i < ch_num; i += 2){
            if(ch_num > 2){
                dst_addr_L = callback_ptr->EntryPara.in_ptr[i];
                dst_addr_R = callback_ptr->EntryPara.in_ptr[i+1];
            }
            source_ch_i2d_buffer_copy(ch_type, dst_addr_L, dst_addr_R, length, channel_sel);
            channel_sel++;
        }
    }else{
        //length =/ ch_num, but stream is 32bit, if 16bit,so need << 1
        U32 silence_length = (length / mix_scenarios_msg[ch_type].channel_num) << (mix_scenarios_msg[ch_type].format_bytes == 2);
        mix_scenarios_msg[ch_type].prefill_silence_count += length;
        MIXER_STREAM_LOG_W("[Source Read] source ch:%d,scenario:%d, read %d silence, ch_data_size:%d", 4, ch_type, mix_scenarios_msg[ch_type].scenario_type,length, ch_data_size);
        memset(dst_addr_L, 0, silence_length);
        memset(dst_addr_R, 0, silence_length);
    }
}

U32 get_scenario_sample_rate(hal_audio_memory_t memory_agent)
{
    volatile SINK scenario_sink = Sink_blks[get_sink_type_by_agent(memory_agent)];
    return scenario_sink->param.audio.src_rate ? scenario_sink->param.audio.src_rate : scenario_sink->param.audio.rate;
}

void mixer_stream_set_sink_prefill(bool hw_clk_skew_enable, bool mixer_stream_running, hal_audio_memory_t sync_agent)
{
    U32 prefill_bytes = 0;
    if(!mixer_stream_running) {
        volatile SINK mixer_sink = Sink_blks[SINK_TYPE_AUDIO_DL12];
        prefill_bytes = (MIXER_SINK_PREFILL_MS) * (mixer_sink->param.audio.rate / 1000) * mixer_sink->param.audio.format_bytes * mixer_sink->param.audio.channel_num; //prefill for uart relay delay;
        mixer_sink->streamBuffer.BufferInfo.WriteOffset += prefill_bytes % (mixer_sink->streamBuffer.BufferInfo.length);
        MIXER_STREAM_LOG_I("dl12 sink prefill:%d bytes, sync_agent:%d, hw_clk_skew_enable:%d,rate:%d,format byte:%d,ch num:%d", 6, prefill_bytes, sync_agent, hw_clk_skew_enable,mixer_sink->param.audio.rate,mixer_sink->param.audio.format_bytes,mixer_sink->param.audio.channel_num);
    }
    #ifdef AIR_DCHS_MODE_ENABLE
        return;//dchs no need scenario prefill
    #endif
    volatile SINK scenario_sink = Sink_blks[get_sink_type_by_agent(sync_agent)];
    U32 sink_reamain = SinkSizeAudioAfe(scenario_sink);
    U32 pre_wo       = scenario_sink->streamBuffer.BufferInfo.WriteOffset;
    prefill_bytes = (hw_clk_skew_enable ? STREAM_PERIOD_MS : 0) * (get_scenario_sample_rate(sync_agent) / 1000) * scenario_sink->param.audio.format_bytes * scenario_sink->param.audio.channel_num;
    if(sink_reamain >= prefill_bytes){
        U32 mask;
        hal_nvic_save_and_set_interrupt_mask(&mask);
        scenario_sink->streamBuffer.BufferInfo.WriteOffset
            = (scenario_sink->streamBuffer.BufferInfo.WriteOffset + prefill_bytes)
            % (scenario_sink->streamBuffer.BufferInfo.length);
        if (scenario_sink->streamBuffer.BufferInfo.WriteOffset == scenario_sink->streamBuffer.BufferInfo.ReadOffset) {
            scenario_sink->streamBuffer.BufferInfo.bBufferIsFull = TRUE;
        }
        hal_nvic_restore_interrupt_mask(mask);
    } else {
        MIXER_STREAM_LOG_E("sync_agent:%d, sink remain size:%d < prefill_bytes:%d", sync_agent, sink_reamain, prefill_bytes);
        AUDIO_ASSERT(0);
    }
    
    MIXER_STREAM_LOG_I("[agent:%d prefill] scenario_sink:0x%x, pre_wo:%d, cur_wo:%d, prefill_bytes:%d, sink remain:%d,hw_clk_skew_enable:%d", 7,
                    sync_agent,
                    scenario_sink,
                    pre_wo,
                    scenario_sink->streamBuffer.BufferInfo.WriteOffset,
                    prefill_bytes,
                    sink_reamain,
                    hw_clk_skew_enable);
}

void source_ch_gain_init_config(void)
{
    U32 mask;
    source_ch_type_t i;
    for(i = MIX_SCENARIO_1; i < MAX_MIXER_NUM; i++){
        //check if have gain need set  
        if(waiting_set_gain_msg[i].memory_agent){
            source_ch_type_t ch_type = mixer_stream_get_source_ch_by_agent(waiting_set_gain_msg[i].memory_agent);

            hal_nvic_save_and_set_interrupt_mask(&mask);
            waiting_set_gain_msg[i].memory_agent = 0;
            int32_t vol_gain = waiting_set_gain_msg[i].vol_gain;
            hal_nvic_restore_interrupt_mask(mask);

            stream_function_sw_gain_configure_gain_target(mixer_stream_gain_port, ch_type * 2 + 1, vol_gain);
            stream_function_sw_gain_configure_gain_target(mixer_stream_gain_port, ch_type * 2 + 2, vol_gain);
            MIXER_STREAM_LOG_I("[SW Gain][Init]set ch_type:%d, gain:%d, sceanrio_type:%d", 3, ch_type, vol_gain, mix_scenarios_msg[ch_type].scenario_type);
        }
    }
}

U32 get_cur_native_clk(void)
{
    return rBb->rClkCtl.rNativeClock & 0x0FFFFFFC;
}

U16 get_cur_native_phase(void)
{
    return rBb->rClkCtl.rNativePhase;
}

BTTIME_STRU get_delay_native_clk(U32 delay_us)
{
    U32 cur_bt_clk;
    U16 cur_bt_phase;
    BTTIME_STRU cur_time, delay_time;
    U8 clk_offset_type = BT_CLK_Offset;
    #ifdef AIR_DCHS_MODE_ENABLE
    clk_offset_type    = DCHS_CLK_Offset;
    #endif
    MCE_GetBtClk((BTCLK *)&cur_bt_clk, (BTPHASE *)&cur_bt_phase, clk_offset_type);
    cur_time.period = cur_bt_clk;
    cur_time.phase  = cur_bt_phase;   
    LC_Add_us_FromA(delay_us, &cur_time, &delay_time);
    MCE_TransBT2NativeClk(delay_time.period, delay_time.phase, &delay_time.period, &delay_time.phase, clk_offset_type);
    return delay_time;
}

void update_hwsrc_input_wrpnt(U8 hwsrc_id, U32 iwo)
{
    U32 addr_offset = hwsrc_id * 0x100;
    AFE_WRITE(ASM_CH01_IBUF_WRPNT + addr_offset, iwo + AFE_READ(ASM_IBUF_SADR + addr_offset));
    MIXER_STREAM_LOG_D("update hwsrc input, get wo=%d, hwsrc_id=%d, hwsrc iwo=%d, iro=%d",4, iwo, hwsrc_id, AFE_READ(ASM_CH01_IBUF_WRPNT + addr_offset) - AFE_READ(ASM_IBUF_SADR + addr_offset), AFE_READ(ASM_CH01_IBUF_RDPNT + addr_offset) - AFE_READ(ASM_IBUF_SADR + addr_offset));
}

void mixer_stream_source_ch_hwsrc_cfg(source_ch_type_t ch_type, afe_mem_asrc_id_t hwsrc_id)
{
    mix_scenarios_msg[ch_type].hwsrc_id          = hwsrc_id;
    mix_scenarios_msg[ch_type].hwsrc_enable_type = MIXER_ADD_HWSRC;
}

void mixer_stream_hwsrc_set_parameters(source_ch_type_t ch_type, afe_src_configuration_t *hwsrc_cfg)
{
    afe_mem_asrc_id_t hwsrc_id = mix_scenarios_msg[ch_type].hwsrc_id;
    SOURCE source = Source_blks[SOURCE_TYPE_MIXER];
    hwsrc_cfg->hw_update_obuf_rdpnt = false;
    hwsrc_cfg->is_mono = mix_scenarios_msg[ch_type].channel_num == 1 ? true : false;
    hwsrc_cfg->id = hwsrc_id;
    hwsrc_cfg->sample_count_threshold = AUDIO_DURATION_TIME * source->param.audio.rate / MUX_UART_BUF_SLICE / 1000;
    hwsrc_cfg->mode = AFE_SRC_NO_TRACKING;
    hwsrc_cfg->ul_mode = false;
    memset(hwsrc_buffer[hwsrc_id], 0, HWSRC_BUF_LENGTH);
    //intput buffer
    hwsrc_cfg->input_buffer.addr   = (U32)mix_scenarios_msg[ch_type].sink_buf_info->startaddr[0];
    hwsrc_cfg->input_buffer.size   = mix_scenarios_msg[ch_type].sink_buf_info->length;
    hwsrc_cfg->input_buffer.rate   = mix_scenarios_msg[ch_type].sample_rate;
    hwsrc_cfg->input_buffer.offset = 0;
    hwsrc_cfg->input_buffer.format = mix_scenarios_msg[ch_type].format_bytes == 4 ? HAL_AUDIO_PCM_FORMAT_S32_LE : HAL_AUDIO_PCM_FORMAT_S16_LE;
    //output buffer
    hwsrc_cfg->output_buffer.addr   = (U32)hwsrc_buffer[hwsrc_id];
    hwsrc_cfg->output_buffer.size   = HWSRC_BUF_LENGTH;
    hwsrc_cfg->output_buffer.rate   = source->param.audio.rate;
    hwsrc_cfg->output_buffer.offset = 32;
    hwsrc_cfg->output_buffer.format = hwsrc_cfg->input_buffer.format;

    MIXER_STREAM_LOG_I("[HWSRC Driver Cfg] hwsrc id:%d, in rate:%d, in addr=0x%x,in size=%d,in format=%d,out addr=0x%d,out size=%d,out format=%d,out rate:%d, is mono=%d", 10,
                 hwsrc_cfg->id+1, hwsrc_cfg->input_buffer.rate, hwsrc_cfg->input_buffer.addr,hwsrc_cfg->input_buffer.size,hwsrc_cfg->input_buffer.format,
                 hwsrc_cfg->output_buffer.addr,hwsrc_cfg->output_buffer.size,hwsrc_cfg->output_buffer.format,hwsrc_cfg->output_buffer.rate,hwsrc_cfg->is_mono);
}

void mixer_stream_source_ch_hwsrc_driver_control(source_ch_type_t ch_type, bool control)
{
    if(mix_scenarios_msg[ch_type].hwsrc_enable != control){
        if(control){
            afe_src_configuration_t hwsrc_cfg;
            memset(&hwsrc_cfg, 0, sizeof(afe_src_configuration_t));
            mixer_stream_hwsrc_set_parameters(ch_type, &hwsrc_cfg);
            hal_audio_src_configuration(&hwsrc_cfg, control);
            hal_audio_src_set_start(&hwsrc_cfg, HAL_AUDIO_MEMORY_SYNC_NONE, control);
            #if 1
            U32 hwsrc_id    = mix_scenarios_msg[ch_type].hwsrc_id;
            U32 addr_offset = hwsrc_id * 0x100;
            MIXER_STREAM_LOG_D("[HWSRC%d Driver]ASM_FREQUENCY_0=0x%x",2, hwsrc_id+1, AFE_READ(ASM_FREQUENCY_0 + addr_offset));
            MIXER_STREAM_LOG_D("[HWSRC%d Driver]ASM_FREQUENCY_1=0x%x",2, hwsrc_id+1, AFE_READ(ASM_FREQUENCY_1 + addr_offset));
            MIXER_STREAM_LOG_D("[HWSRC%d Driver]ASM_FREQUENCY_2=0x%x",2, hwsrc_id+1, AFE_READ(ASM_FREQUENCY_2 + addr_offset));
            MIXER_STREAM_LOG_D("[HWSRC%d Driver]ASM_FREQUENCY_3=0x%x",2, hwsrc_id+1, AFE_READ(ASM_FREQUENCY_3 + addr_offset));
            MIXER_STREAM_LOG_D("[HWSRC%d Driver]ASM_CALI_DENOMINATOR=0x%x",2, hwsrc_id+1, AFE_READ(ASM_CALI_DENOMINATOR + addr_offset));
            MIXER_STREAM_LOG_D("[HWSRC%d Driver]ASM_MAX_OUT_PER_IN0=0x%x",2, hwsrc_id+1, AFE_READ(ASM_MAX_OUT_PER_IN0 + addr_offset));
            MIXER_STREAM_LOG_D("[HWSRC%d Driver]ASM_FREQ_CALI_CYC=0x%x",2, hwsrc_id+1, AFE_READ(ASM_FREQ_CALI_CYC + addr_offset));
            MIXER_STREAM_LOG_D("[HWSRC%d Driver]ASM_FREQ_CALI_CTRL=0x%x",2, hwsrc_id+1, AFE_READ(ASM_FREQ_CALI_CTRL + addr_offset));
            MIXER_STREAM_LOG_D("[HWSRC%d Driver]ASM_GEN_CONF=0x%x",2, hwsrc_id+1, AFE_READ(ASM_GEN_CONF + addr_offset));
            MIXER_STREAM_LOG_D("[HWSRC%d Driver]ASM_CH01_CNFG=0x%x",2, hwsrc_id+1, AFE_READ(ASM_CH01_CNFG + addr_offset));
            MIXER_STREAM_LOG_D("[HWSRC%d Driver]ASM_IER=0x%x",2, hwsrc_id+1, AFE_READ(ASM_IER + addr_offset));
            MIXER_STREAM_LOG_D("[HWSRC%d Driver]ASM_IFR=0x%x",2, hwsrc_id+1, AFE_READ(ASM_IFR + addr_offset));
            #endif
            if(mix_scenarios_msg[ch_type].sink_buf_info->bBufferIsFull){
                update_hwsrc_input_wrpnt(hwsrc_id, mix_scenarios_msg[ch_type].sink_buf_info->length);
            }else{
                update_hwsrc_input_wrpnt(hwsrc_id, mix_scenarios_msg[ch_type].sink_buf_info->WriteOffset);
            }
        }else{
            afe_src_configuration_t hwsrc_cfg;
            memset(&hwsrc_cfg, 0, sizeof(afe_src_configuration_t));
            mixer_stream_hwsrc_set_parameters(ch_type, &hwsrc_cfg);
            hal_audio_src_set_start(&hwsrc_cfg, HAL_AUDIO_MEMORY_SYNC_NONE, control);
            hal_audio_src_configuration(&hwsrc_cfg, control);
        }
        mix_scenarios_msg[ch_type].hwsrc_enable = control;
        MIXER_STREAM_LOG_I("[HWSRC Driver] ch_type=%d, hwsrc id : %d, control : %d, sink buf addr=0x%x, wo=%d", 5, ch_type, mix_scenarios_msg[ch_type].hwsrc_id, control, mix_scenarios_msg[ch_type].sink_buf_info->startaddr[0],mix_scenarios_msg[ch_type].sink_buf_info->WriteOffset);
    }
}

void mixer_stream_mcu2dsp_msg_sync_callback(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    uint32_t mask;
    sceanrio_mcu2dsp_sync_msg_t * sync_msg = (sceanrio_mcu2dsp_sync_msg_t *)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    MIXER_STREAM_LOG_I("[DSP msg sync] sync_type:%d", 1, sync_msg->sync_type);
    if(sync_msg->sync_type == SYNC_SCENARIO_OPEN){
        source_ch_type_t ch_type = check_idle_source_ch();
        AUDIO_ASSERT(ch_type != MIX_SCENARIO_MAX && "[Mixer Stream] have no idle source ch");
        mix_scenarios_msg[ch_type].mix_type      = WAITING_COUNT_MIX;
        mix_scenarios_msg[ch_type].scenario_type = sync_msg->scenario_type;
        mix_scenarios_msg[ch_type].memory_agent  = sync_msg->memory_agent;
        mix_scenarios_msg[ch_type].format_bytes  = sync_msg->format_bytes;
        mix_scenarios_msg[ch_type].sample_rate   = sync_msg->sample_rate;
        mix_scenarios_msg[ch_type].channel_num   = sync_msg->channel_num;
        if(sync_msg->memory_agent == HAL_AUDIO_MEM_SUB){ //other chip scenario or virtual memory
            #ifdef AIR_DCHS_MODE_ENABLE
            if(dchs_get_device_mode() == DCHS_MODE_LEFT){
                SINK sink = Sink_blks[SINK_TYPE_AUDIO];
                if(sink){
                    U32 prefill = sink->streamBuffer.BufferInfo.WriteOffset * sink->param.audio.channel_num;
                    audio_dchs_dsp2dsp_cmd_t dchs_dsp2dsp_cmd;
                    dchs_dsp2dsp_cmd.cmd_param.dchs_dl_param.prefill_size       = prefill;
                    dchs_dsp2dsp_cmd.cmd_param.dchs_dl_param.channel_num        = sink->param.audio.channel_num;
                    dchs_dsp2dsp_cmd.header.cmd_type = AUDIO_DCHS_DL_UART_SCENARIO_PREFILL_SIZE;
                    dchs_dsp2dsp_cmd.header.param_size = sizeof(audio_dchs_dsp2dsp_cmd_t) - sizeof(uart_cmd_header_t);
                    dsp_uart_tx(UART_CMD, (U8 *)&dchs_dsp2dsp_cmd, sizeof(audio_dchs_dsp2dsp_cmd_t));
                    MIXER_STREAM_LOG_I("[DSP msg sync]send,scenario type:%d, prefill size:%d,ch_num:%d", 3, sync_msg->scenario_type, prefill, sink->param.audio.channel_num);
                }
            }
            dchs_dl_uart_buf_init(ch_type);
            #endif
        } else {
            volatile SINK scenario_sink = Sink_blks[get_sink_type_by_agent(sync_msg->memory_agent)];
            AUDIO_ASSERT(scenario_sink && "[Mixer Stream]get sink NULL");
            mix_scenarios_msg[ch_type].channel_num = scenario_sink->param.audio.channel_num;
            if(scenario_sink->param.audio.mem_handle.pure_agent_with_src || scenario_sink->param.audio.AfeBlkControl.u4asrcflag){
                mix_scenarios_msg[ch_type].hwsrc_enable  = true;
                mix_scenarios_msg[ch_type].hwsrc_id      = sync_msg->memory_agent - 1;
            }else{
                mix_scenarios_msg[ch_type].sink_buf_info = &(scenario_sink->streamBuffer.BufferInfo); 
            }
        }
        mixer_stream_set_sink_prefill(sync_msg->clkskew_mode, g_mixer_stream_mix_count, sync_msg->memory_agent);
        source_ch_gain_init_config();
        volatile SOURCE mixer_source = Source_blks[SOURCE_TYPE_MIXER];
        if(mix_scenarios_msg[ch_type].sample_rate != mixer_source->param.audio.rate){
            mixer_stream_source_ch_hwsrc_cfg(ch_type, AFE_MEM_ASRC_1);
            mixer_stream_source_ch_hwsrc_driver_control(ch_type, true);
        }
        if(sync_msg->need_play_en){
            U32 bt_play_clk;
            U16 bt_play_phase;
            if(sync_msg->bt_clk){
                bt_play_clk   = sync_msg->bt_clk;
                bt_play_phase = sync_msg->bt_phase;
            } else {
                BTTIME_STRU play_time = get_delay_native_clk(PLAY_DELAY_MS * 1000);
                bt_play_clk   = play_time.period;
                bt_play_phase = play_time.phase;
            }
            //hal_audio_afe_set_play_en(bt_play_clk, (uint32_t)bt_play_phase);//set play en
            dchs_dl_uart_relay_play_en_info(bt_play_clk, bt_play_phase);
            mixer_stream_setup_play_en(bt_play_clk, bt_play_phase, NULL, sync_msg->scenario_type);
            MIXER_STREAM_LOG_I("[DSP msg sync]set play en:0x%x,phase:0x%x, cur clk:0x%x,pahse:0x%x, scenario_type:%d", 5, bt_play_clk, bt_play_phase, get_cur_native_clk(), get_cur_native_phase(), sync_msg->scenario_type);
        }

        MIXER_STREAM_LOG_I("[DSP msg sync] Open syn,scenario_type:%d,memory_agent:%d,format_bytes:%d,channel_num:%d,hwsrc_enable:%d,hwsrc_id:%d,mix_type:%d,sink_buf_info:0x%x,start_time:0x%x%x", 10,
                    mix_scenarios_msg[ch_type].scenario_type,
                    mix_scenarios_msg[ch_type].memory_agent,
                    mix_scenarios_msg[ch_type].format_bytes,
                    mix_scenarios_msg[ch_type].channel_num,
                    mix_scenarios_msg[ch_type].hwsrc_enable,
                    mix_scenarios_msg[ch_type].hwsrc_id,
                    mix_scenarios_msg[ch_type].mix_type,
                    (U32)(g_mixer_stream_start_time >> 32),
                    (U32)(g_mixer_stream_start_time & 0xFFFFFFFF));
        //resume_task();
    }else if(sync_msg->sync_type == SYNC_SCENARIO_CLOSE){
        source_ch_type_t ch_type = mixer_stream_get_source_ch_by_scenario(sync_msg->scenario_type);
        if(ch_type == MIX_SCENARIO_MAX){
            MIXER_STREAM_LOG_I("this scenario:%d not start", 1, sync_msg->scenario_type);
        }
        #ifdef AIR_DCHS_MODE_ENABLE
        if(sync_msg->memory_agent == HAL_AUDIO_MEM_SUB){ //other chip scenario or virtual memory
            if(dl_uart_buf_info.startaddr[0]){
                preloader_pisplit_free_memory(dl_uart_buf_info.startaddr[0]);
                MIXER_STREAM_LOG_I("[DSP msg sync] free uart buffer", 0);
            }
            memset(&dl_uart_buf_info, 0, sizeof(BUFFER_INFO));
        }
        #endif
        if(mix_scenarios_msg[ch_type].hwsrc_enable && mix_scenarios_msg[ch_type].hwsrc_enable_type == MIXER_ADD_HWSRC){
            mixer_stream_source_ch_hwsrc_driver_control(ch_type, false);
        }
        hal_nvic_save_and_set_interrupt_mask(&mask);
        memset(&mix_scenarios_msg[ch_type], 0 , sizeof(mixer_scenario_msg_t));//reset msg
        hal_nvic_restore_interrupt_mask(mask);
        MIXER_STREAM_LOG_I("[DSP msg sync] Close sync, scenario_type:%d", 1, sync_msg->scenario_type);

    }else if(sync_msg->sync_type == SYNC_SET_GAIN_VALUE){
        int32_t  vol_gain = sync_msg->vol_gain;
        source_ch_type_t ch_type = mixer_stream_get_source_ch_by_agent(sync_msg->memory_agent);
        if(ch_type == MIX_SCENARIO_MAX){//no open
            MIXER_STREAM_LOG_I("this agent:%d not open, gain:%d set later", 2, sync_msg->memory_agent, vol_gain);
            source_ch_type_t i;
            for(i = MIX_SCENARIO_1; i < MAX_MIXER_NUM; i++){
                if(!waiting_set_gain_msg[i].memory_agent){
                    waiting_set_gain_msg[i].memory_agent = sync_msg->memory_agent;
                    waiting_set_gain_msg[i].vol_gain     = vol_gain;
                    return;
                }
            }
            AUDIO_ASSERT(0 && "[Mixer Stream]waiting set not avaible");
        }
        sw_gain_config_t old_config;
        stream_function_sw_gain_get_config(mixer_stream_gain_port, ch_type * 2 + 1, &old_config);
        MIXER_STREAM_LOG_I("[Sw Gain][config] ch_type: %d, set ch:%d and %d gain from %d to %d\r\n", 5,
                    ch_type,
                    ch_type * 2 + 1,
                    ch_type * 2 + 2,
                    old_config.target_gain,
                    vol_gain);
        stream_function_sw_gain_configure_gain_target(mixer_stream_gain_port, ch_type * 2 + 1, vol_gain);
        stream_function_sw_gain_configure_gain_target(mixer_stream_gain_port, ch_type * 2 + 2, vol_gain);
    }
    //extend here
    else{
        MIXER_STREAM_LOG_W("[DSP msg sync] invalid sync type : %d", 1, sync_msg->sync_type);
    }
}

bool check_source_ch_enough_data(SOURCE source, source_ch_type_t ch_type)
{
    U32 need_size = source->param.audio.frame_size * mix_scenarios_msg[ch_type].format_bytes * mix_scenarios_msg[ch_type].channel_num;
    if(mixer_get_ch_data_size(ch_type) >= need_size){
        return true;
    }
    return false;
}

bool check_waiting_mix_enough_data(SOURCE source)
{
    for(source_ch_type_t ch_type = MIX_SCENARIO_1; ch_type < MIX_SCENARIO_MAX; ch_type ++){
        if(mix_scenarios_msg[ch_type].mix_type == WAITING_MIX){
            if(check_source_ch_enough_data(source, ch_type)){
                return true;
            }
        }
    }
    return false;
}

U32 SourceSize_Mixer(SOURCE source)
{
    U32 mix_stream_frame_size = source->param.audio.frame_size * source->param.audio.format_bytes;
    for(source_ch_type_t ch_type = MIX_SCENARIO_1; ch_type < MIX_SCENARIO_MAX; ch_type ++){
        if(mix_scenarios_msg[ch_type].mix_type == WAITING_MIX){
            if(g_mixer_stream_mix_count > mix_scenarios_msg[ch_type].mix_point){
                mix_scenarios_msg[ch_type].mix_type = IS_RUNNING;
            }
            MIXER_STREAM_LOG_I("[Source Size] ch_type:%d, mixer_mix_count:%d, ch_mix_point:%d, mix_type:%d", 4, ch_type, g_mixer_stream_mix_count, mix_scenarios_msg[ch_type].mix_point, mix_scenarios_msg[ch_type].mix_type);
        }
    }
    volatile DSP_CALLBACK_PTR callback_ptr = DSP_Callback_Get(source, source->transform->sink);
    callback_ptr->EntryPara.out_channel_num = 2 * MAX_MIXER_NUM; // max mix ch /mono data

    if(!g_mixer_stream_process_count){//no need read
        MIXER_STREAM_LOG_D("[Source Size] already process, this time no need process", 0);
        return 0;
    }
    if(!g_mixer_stream_mix_count && !check_waiting_mix_enough_data(source)){
        return 0;
    }
    return mix_stream_frame_size;
}

BOOL SourceReadBuf_Mixer(SOURCE source, U8 *dst_addr, U32 length)
{
    UNUSED(dst_addr);
    U32 mix_stream_frame_size = source->param.audio.frame_size * source->param.audio.format_bytes;
    volatile DSP_CALLBACK_PTR callback_ptr = DSP_Callback_Get(source, source->transform->sink);
    for(source_ch_type_t ch_type = MIX_SCENARIO_1; ch_type < MIX_SCENARIO_MAX; ch_type ++){
        if(mix_scenarios_msg[ch_type].mix_type == IS_RUNNING
                || (mix_scenarios_msg[ch_type].mix_type == WAITING_MIX && mix_scenarios_msg[ch_type].pre_process_count < PRE_PROCESS_COUNT && check_source_ch_enough_data(source, ch_type))){

            length = source->param.audio.frame_size * mix_scenarios_msg[ch_type].format_bytes * mix_scenarios_msg[ch_type].channel_num;
            copy_source_ch_data_2_stream_buf(ch_type, 
                                            callback_ptr->EntryPara.in_ptr[ch_type * 2],
                                            callback_ptr->EntryPara.in_ptr[ch_type * 2 + 1],
                                            length);
            if(mix_scenarios_msg[ch_type].mix_type == WAITING_MIX){
                mix_scenarios_msg[ch_type].pre_process_count ++;
            }
        } else {
            memset(callback_ptr->EntryPara.in_ptr[ch_type * 2],     0, mix_stream_frame_size);
            memset(callback_ptr->EntryPara.in_ptr[ch_type * 2 + 1], 0, mix_stream_frame_size);
        }
        
        LOG_AUDIO_DUMP(callback_ptr->EntryPara.in_ptr[ch_type * 2],     mix_stream_frame_size, AUDIO_MIXER_STREAM_SOURCE_IN_CH1 + ch_type * 2);
        LOG_AUDIO_DUMP(callback_ptr->EntryPara.in_ptr[ch_type * 2 + 1], mix_stream_frame_size, AUDIO_MIXER_STREAM_SOURCE_IN_CH1 + ch_type * 2 + 1); 
    }
    return true;
}

void source_drop_precess_count(void)
{
    uint32_t mask = 0;
    hal_nvic_save_and_set_interrupt_mask(&mask);
    g_mixer_stream_process_count --;
    hal_nvic_restore_interrupt_mask(mask);
    if(g_mixer_stream_process_count >= 10){ //avoid too much log
        MIXER_STREAM_LOG_W("[Source Drop] resume task again, process count=%d", 1, g_mixer_stream_process_count);
    }
}

VOID SourceDrop_Mixer(SOURCE source, U32 amount)
{
    UNUSED(source);
    source_drop_precess_count();
    for(source_ch_type_t ch_type = MIX_SCENARIO_1; ch_type < MIX_SCENARIO_MAX; ch_type ++){
        if(mix_scenarios_msg[ch_type].prefill_silence_count == 0){
            amount = source->param.audio.frame_size * mix_scenarios_msg[ch_type].format_bytes * mix_scenarios_msg[ch_type].channel_num;
            source_ch_buf_drop(ch_type, amount);
        }
    }
}


BOOL SourceClose_Mixer(SOURCE source)
{
    UNUSED(source);
    #ifdef AIR_SOFTWARE_MIXER_ENABLE
    if(mixer_member){
        stream_function_sw_mixer_member_unregister(SW_MIXER_PORT_0, mixer_member);
        stream_function_sw_mixer_member_delete(mixer_member);
        stream_function_sw_mixer_deinit(SW_MIXER_PORT_0);
        mixer_member = NULL;
    }
    #endif /* AIR_SOFTWARE_MIXER_ENABLE */
    if(mixer_stream_gain_port){
        stream_function_sw_gain_deinit(mixer_stream_gain_port);
        mixer_stream_gain_port = NULL;
    }
    //reset
    reset_mixer_stream_status();
    if(g_mixer_stream_timer_handle){
        hal_gpt_sw_free_timer(g_mixer_stream_timer_handle);
        g_mixer_stream_timer_handle = 0;
    }
    MIXER_STREAM_LOG_I("[Source Close] Mixer close done", 0);
    return TRUE;
}

void play_clk_align_period(U32 *bt_clk, U16 *bt_phase, U32 align_us, U64* cur_clk_us, U64* align_clk_us)
{
    U64 get_play_clk_us = ((U64)(*bt_clk) * 625 + (*bt_phase)) / 2;
    U64 get_align_clk_us = ((((U64)(*bt_clk) * 625 + (*bt_phase)) / 2 + align_us - 1) / align_us) * align_us;
    U32 clk_gap = get_align_clk_us - get_play_clk_us;
    BTTIME_STRU bt_time, align_time;
    bt_time.period = *bt_clk;
    bt_time.phase  = *bt_phase;
    LC_Add_us_FromA(clk_gap, &bt_time, &align_time);
    *bt_clk   = align_time.period;
    *bt_phase = align_time.phase;
    if(cur_clk_us){
        *cur_clk_us = get_play_clk_us;
    }
    if(align_clk_us){
        *align_clk_us = get_align_clk_us;
        #ifdef AIR_DCHS_MODE_ENABLE
        *align_clk_us = get_play_clk_us;//DCHS no need align
        #endif
    }
    MIXER_STREAM_LOG_I("[clk align] get clk:0x%x,get phase:0x%d,align clk:0x%x,align phase:0x%x", 4, bt_time.period,bt_time.phase,align_time.period,align_time.phase);
}

void mixer_stream_save_and_align_play_clk(U32 *bt_clk, U16 *bt_phase, audio_scenario_type_t scenario_type)
{
    U32 mix_point = 0;
    source_ch_type_t ch_type = scenario_type ? mixer_stream_get_source_ch_by_scenario(scenario_type) : mixer_stream_get_source_ch_by_agent(HAL_AUDIO_MEM_SUB);
    AUDIO_ASSERT(ch_type != MIX_SCENARIO_MAX && "[Mixer Stream] not get running source ch");
    U32 stream_period_us     = STREAM_PERIOD_MS * 1000;
    if(bt_clk){
        U64 align_clk_us, cur_clk_us;
        play_clk_align_period(bt_clk, bt_phase, stream_period_us, &cur_clk_us, &align_clk_us);
        if(g_mixer_stream_start_time == 0){ //first mix
            g_mixer_stream_start_time = align_clk_us;
        } else {
            //count mix point
            mix_point = (align_clk_us - g_mixer_stream_start_time) / stream_period_us;
        }
        mix_scenarios_msg[ch_type].mix_point = mix_point;
        mix_scenarios_msg[ch_type].mix_type  = WAITING_MIX;
        #ifdef AIR_DCHS_MODE_ENABLE
            return; // DCHS no need gap bytes prefill
        #endif
        //add align gap samples
        hal_audio_memory_t memory_agent = mixer_stream_get_agent_by_scenario(scenario_type);
        volatile SINK scenario_sink = Sink_blks[get_sink_type_by_agent(memory_agent)];
        AUDIO_ASSERT(scenario_sink && "[Mixer Stream][count mix point]get NULL sink");
        U32 sample_bytes = mix_scenarios_msg[ch_type].format_bytes * mix_scenarios_msg[ch_type].channel_num;
        //U32 gap_us       = (cur_clk_us - g_mixer_stream_start_time) % stream_period_us;
        U32 gap_us      = stream_period_us - (align_clk_us - cur_clk_us);
        U32 gap_bytes    = (U64)gap_us * get_scenario_sample_rate(mix_scenarios_msg[ch_type].memory_agent) * sample_bytes / 1000000;
        U32 sink_reamain = SinkSizeAudioAfe(scenario_sink);
        U32 pre_wo       = scenario_sink->streamBuffer.BufferInfo.WriteOffset;
        gap_bytes        = ((gap_bytes + (sample_bytes - 1)) / sample_bytes * sample_bytes);//sample align
        if(sink_reamain >= gap_bytes){
            U32 mask;
            hal_nvic_save_and_set_interrupt_mask(&mask);
            scenario_sink->streamBuffer.BufferInfo.WriteOffset
                = (scenario_sink->streamBuffer.BufferInfo.WriteOffset + gap_bytes)
                % (scenario_sink->streamBuffer.BufferInfo.length);
            if (scenario_sink->streamBuffer.BufferInfo.WriteOffset == scenario_sink->streamBuffer.BufferInfo.ReadOffset) {
                scenario_sink->streamBuffer.BufferInfo.bBufferIsFull = TRUE;
            }
            hal_nvic_restore_interrupt_mask(mask);
        } else {
            MIXER_STREAM_LOG_E("sceanrio type:%d, sink remain size:%d < gap_bytes:%d,mix count:%d", 4, scenario_type, sink_reamain,gap_bytes, g_mixer_stream_mix_count);
            //AUDIO_ASSERT(0);
        }
        MIXER_STREAM_LOG_I("[count mix]scenario_sink:0x%x,align(us):0x%x%x, cur_clk(us):0x%x%x, pre_wo:%d, cur_wo:%d, gap_bytes(%dus):%d, cur count:%d, mix point:%d, scenario_type:%d,start_time:0x%x%x", 14, 
                                scenario_sink,
                                (U32)(align_clk_us >> 32),
                                (U32)(align_clk_us & 0xFFFFFFFF),
                                (U32)(cur_clk_us >> 32),
                                (U32)(cur_clk_us & 0xFFFFFFFF),
                                pre_wo,
                                scenario_sink->streamBuffer.BufferInfo.WriteOffset,
                                gap_us,
                                gap_bytes,
                                g_mixer_stream_mix_count,
                                mix_point,
                                scenario_type,
                                (U32)(g_mixer_stream_start_time >> 32),
                                (U32)(g_mixer_stream_start_time & 0xFFFFFFFF));
    }
}

SOURCE SourceInit_Mixer(SOURCE source)
{
    audio_scenario_type_t data_scenario_type       = (audio_scenario_type_t)source->param.audio.scenario_sub_id;
    MIXER_STREAM_LOG_I("[Source] frame size = %d, data_scenario_type=%d,sample_rate=%d,format_bytes=%d", 4, source->param.audio.frame_size, data_scenario_type,source->param.audio.rate, source->param.audio.format_bytes);
    //init sw gain
    mixer_stream_sw_gain_init(source);
    //init sw mixer
    mixer_stream_sw_mixer_init(source);
    //reset
    reset_mixer_stream_status();
    #ifdef AIR_DCHS_MODE_ENABLE
    dsp_uart_open(UART_DL);
    dsp_uart_open(UART_CMD);
    #endif
    if(!g_mixer_stream_timer_handle){
        hal_gpt_sw_get_timer(&g_mixer_stream_timer_handle);
    }
    /* interface init */
    source->sif.SourceSize        = SourceSize_Mixer;
    source->sif.SourceDrop        = SourceDrop_Mixer;
    source->sif.SourceClose       = SourceClose_Mixer;
    source->sif.SourceReadBuf     = SourceReadBuf_Mixer;
    return source;
}



void dsp_mixer_stream_open(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    MIXER_STREAM_LOG_I("stream opening\r\n", 0);
    mcu2dsp_open_param_p open_param = (mcu2dsp_open_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    MIXER_STREAM_LOG_I("in rate:%d,audio_device:%d,memory:%d,stream out:0x%x",4,open_param->stream_in_param.afe.sampling_rate,open_param->stream_out_param.afe.audio_device,open_param->stream_out_param.afe.memory,&open_param->stream_out_param);
    mixer_stream_if.source   = dsp_open_stream_in(open_param);
    mixer_stream_if.sink     = dsp_open_stream_out(open_param);
    mixer_stream_if.transform = NULL;
    mixer_stream_if.pfeature_table = stream_feature_list_mixer_stream;
    DSP_Callback_PreloaderConfig(mixer_stream_if.pfeature_table, open_param->audio_scenario_type);
    MIXER_STREAM_LOG_I("stream open done\r\n", 0);
}

void dsp_mixer_stream_start(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    MIXER_STREAM_LOG_I("stream starting\r\n", 0);

    mcu2dsp_start_param_p start_param;
    start_param = (mcu2dsp_start_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    MIXER_STREAM_LOG_I("stream starting,sub sceario:%d", 1, mixer_stream_if.source->param.audio.scenario_sub_id);
    dsp_start_stream_in(start_param, mixer_stream_if.source);
    dsp_start_stream_out(start_param, mixer_stream_if.sink);

    mixer_stream_if.transform = TrasformAudio2Audio(mixer_stream_if.source, mixer_stream_if.sink, mixer_stream_if.pfeature_table);

    if (mixer_stream_if.transform == NULL) {
        MIXER_STREAM_LOG_I("stream transform failed", 0);
    } else {
        MIXER_STREAM_LOG_I("stream start done", 0);
    }
}

void dsp_mixer_stream_stop(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    MIXER_STREAM_LOG_I("stream stopping\r\n", 0);
    if (mixer_stream_if.transform != NULL) {
        StreamDSPClose(mixer_stream_if.transform->source, mixer_stream_if.transform->sink, msg.ccni_message[0] >> 16 | 0x8000);
    }
    mixer_stream_if.transform = NULL;
    MIXER_STREAM_LOG_I("stream stop done\r\n", 0);
}

void dsp_mixer_stream_close(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    MIXER_STREAM_LOG_I("stream closing\r\n", 0);
    DSP_Callback_UnloaderConfig(mixer_stream_if.pfeature_table, mixer_stream_if.source->scenario_type);
    SourceClose(mixer_stream_if.source);
    SinkClose(mixer_stream_if.sink);
    memset(&mixer_stream_if, 0, sizeof(CONNECTION_IF));
    MIXER_STREAM_LOG_I("stream close done\r\n", 0);
}

SOURCE StreamMixerSource(void *param)
{
    SOURCE source = NULL;
    if (Source_blks[SOURCE_TYPE_MIXER]) {
        return Source_blks[SOURCE_TYPE_MIXER];
    }
    source = new_source(SOURCE_TYPE_MIXER);
    //cfg param
    mcu2dsp_open_param_p open_param     = param;
    source->param.audio.frame_size      = open_param->stream_in_param.afe.frame_size;
    source->scenario_type               = open_param->audio_scenario_type;
    source->param.audio.format_bytes    = (open_param->stream_in_param.afe.format > HAL_AUDIO_PCM_FORMAT_U16_BE) ? 4 : 2;
    source->param.audio.rate            = open_param->stream_in_param.afe.sampling_rate;
    source->param.audio.format          = open_param->stream_in_param.afe.format;
    source->param.audio.channel_num     = open_param->stream_in_param.afe.sw_channels;
    source->param.audio.scenario_sub_id = open_param->stream_in_param.data_dl.scenario_type;

    MIXER_STREAM_LOG_I("[stream_mixer_source] scenario_type = %d,format_bytes=%d, rate=%d, format=%d, frame_number=%d, scenario_sub_id=%d",6,
                  source->scenario_type,
                  source->param.audio.format_bytes,
                  source->param.audio.rate,
                  source->param.audio.format,
                  open_param->stream_in_param.afe.frame_number,
                  source->param.audio.scenario_sub_id);
    if(source && source->scenario_type == AUDIO_SCENARIO_TYPE_MIXER_STREAM){
        SourceInit_Mixer(source);
    }
    return source;
}

SOURCE dsp_open_stream_in_mixer(mcu2dsp_open_param_p open_param)
{
    SOURCE source;
    MIXER_STREAM_LOG_I("Stream in mixer", 0);
    source = StreamMixerSource(open_param);
    return source;
}
void mixer_stream_resume_task_callback(void *user_data)
{
    UNUSED(user_data);
    resume_task();
}


void mixer_stream_resume_task(SINK sink)
{
    audio_scenario_type_t scenario_type = sink->scenario_type;
    if(scenario_type == AUDIO_SCENARIO_TYPE_BLE_MUSIC_DL){
        scenario_type = AUDIO_SCENARIO_TYPE_BLE_DL;
    }
    source_ch_type_t ch_type = mixer_stream_get_source_ch_by_scenario(scenario_type);
    SOURCE source = Source_blks[SOURCE_TYPE_MIXER];
    if(ch_type == MIX_SCENARIO_MAX || !source){
        MIXER_STREAM_LOG_I("[resume task]not support scenario type:%d", 1, sink->scenario_type);
        return;
    }

    if((sink->param.audio.mem_handle.pure_agent_with_src || sink->param.audio.AfeBlkControl.u4asrcflag)
        && !check_ch_enough_data_size(ch_type)) {
        //this delay timer for hwsrc trans time: 1.5ms
        hal_gpt_sw_start_timer_us(g_mixer_stream_timer_handle, 1500, mixer_stream_resume_task_callback, NULL);
    } else {
        resume_task();
    }
}

void debug_timer_callback(void *user_data)
{
    static U32 debug_count = 0;
    debug_count++;
    SINK_TYPE sink_type = (SINK_TYPE)user_data;
    volatile SINK sink = Sink_blks[sink_type];
    volatile SINK dl1_sink = Sink_blks[SINK_TYPE_AUDIO];
    volatile SINK dl2_sink = Sink_blks[SINK_TYPE_VP_AUDIO];
    volatile SINK dl3_sink = Sink_blks[SINK_TYPE_AUDIO_DL3];
    //AUDIO_ASSERT(sink && "[Mixer Stream][timer callback] get NULL sink");
    if(sink && sink->param.audio.irq_exist == false){
        if(g_mixer_stream_process_count){
            resume_task();
        }
        MIXER_STREAM_LOG_I("Play en check, target clk:0x%x phase:0x%x, curent clk:0x%x, phase:0x%x, enable:0x%x,dl12 wo:%d,ro:%d,len:%d,dl1 wo:%d,ro:%d,dl2 wo:%d,ro:%d,dl3 wo:%d,ro:%d,target wake up 0x%x 0x%x", 16,
        *((volatile uint32_t *)(CONN_BT_TIMCON_BASE + 0x0204)), // audio target playen nclk: 0.3125ms base
        *((volatile uint32_t *)(CONN_BT_TIMCON_BASE + 0x0208)), // audio target playen nphase: 0.5us base
        get_cur_native_clk(),                                   // playen hardware compare curent nclk
        get_cur_native_phase(),                                 // playen hardware compare curent nphase
        *((volatile uint8_t *)(CONN_BT_TIMCON_BASE + 0x0200)),  // playen enable: 1->0 means enable
        sink->streamBuffer.BufferInfo.WriteOffset,
        sink->streamBuffer.BufferInfo.ReadOffset,
        sink->streamBuffer.BufferInfo.length,
        dl1_sink ? dl1_sink->streamBuffer.BufferInfo.WriteOffset : 0,
        dl1_sink ? dl1_sink->streamBuffer.BufferInfo.ReadOffset : 0,
        dl2_sink ? dl2_sink->streamBuffer.BufferInfo.WriteOffset : 0,
        dl2_sink ? dl2_sink->streamBuffer.BufferInfo.ReadOffset : 0,
        dl3_sink ? dl3_sink->streamBuffer.BufferInfo.WriteOffset : 0,
        dl3_sink ? dl3_sink->streamBuffer.BufferInfo.ReadOffset : 0,
        *((volatile uint32_t *)(CONN_BT_TIMCON_BASE + 0x0104)), // bt hw next wake-up nclk
        *((volatile uint32_t *)(CONN_BT_TIMCON_BASE + 0x0108))  // bt hw next wake-up nphase
        );
        if(debug_count >= 1000){
            //goto FREE_TIMER;
            AUDIO_ASSERT(0 && "[Mixer Stream][timer callback] play en time out");
        }
        hal_gpt_sw_start_timer_ms(play_en_debug_timer, 2, debug_timer_callback, (void *)sink_type);
    } else {
        goto FREE_TIMER;
    }
    return;
FREE_TIMER:
    hal_gpt_sw_free_timer(play_en_debug_timer);
    debug_count = 0;
    play_en_debug_timer = 0;
}

void mixer_stream_trigger_debug_timer(SINK_TYPE sink_type)
{
    if(!play_en_debug_timer){
        hal_gpt_sw_get_timer(&play_en_debug_timer);
        hal_gpt_sw_start_timer_ms(play_en_debug_timer, 1, debug_timer_callback, (void *)sink_type);
    }  
}

void dsp_start_stream_in_mixer(mcu2dsp_start_param_p start_param, SOURCE source)
{
    UNUSED(start_param);
    UNUSED(source);
}

void* play_en_postpone_callback(void *usingPtr)
{
    source_ch_type_t ch_type = mixer_stream_get_source_ch_by_scenario((audio_scenario_type_t)usingPtr);
    MIXER_STREAM_LOG_W("enter play_en_postpone_callback, scenario_type:%d", 1, (audio_scenario_type_t)usingPtr);
    if(NO_MIX != mix_scenarios_msg[ch_type].mix_type){
        AUDIO_ASSERT(0 && "[Mixer Stream] sceanrio is running, play en exception");
    }
    return NULL;
}

void setup_play_en(U32 play_clk, U16 play_phase, audio_scenario_type_t scenario_type, hal_audio_memory_selection_t trigger_agent)
{
    play_en_setup_info_t setup_info = {0};
    setup_info.bt_time.period       = play_clk;
    setup_info.bt_time.phase        = play_phase;
    setup_info.usingPtr             = (void *)scenario_type;
    setup_info.play_memory          = trigger_agent;
    setup_info.postpone_handler     = play_en_postpone_callback;

    DSP_setup_play_en(&setup_info);
    MIXER_STREAM_LOG_I("Play en setup_info:0x%x,0x%x,0x%x,0x%x", 4,
                        setup_info.bt_time.period,
                        setup_info.bt_time.phase,
                        setup_info.usingPtr,
                        setup_info.play_memory);
}

void mixer_stream_setup_play_en(U32 bt_clk, U16 bt_phase, SOURCE source, audio_scenario_type_t scenario_type)
{
    if(source){
        scenario_type = source->scenario_type;
        #ifdef AIR_BT_CODEC_BLE_ENABLED
        if(scenario_type == AUDIO_SCENARIO_TYPE_BLE_DL){
            if(source->param.n9ble.context_type != BLE_CONTEXT_CONVERSATIONAL){
                scenario_type = AUDIO_SCENARIO_TYPE_BLE_ULL_DL;//LE call : AUDIO_SCENARIO_TYPE_BLE_DL,
            }                                                  //LE music: AUDIO_SCENARIO_TYPE_BLE_MUSIC_DL
        }                                                      //ULL     : AUDIO_SCENARIO_TYPE_BLE_ULL_DL
        #endif
    }
    U32 pre_bt_clk   = bt_clk;
    U16 pre_bt_phase = bt_phase;
    mixer_stream_save_and_align_play_clk(&bt_clk, &bt_phase, scenario_type);//play clk align strem period
    hal_audio_memory_selection_t memory_agent = scenario_type ? (hal_audio_memory_selection_t)mixer_stream_get_agent_by_scenario(scenario_type) : 0;
    //check if trigger dl12
    volatile SINK sink = Sink_blks[SINK_TYPE_AUDIO_DL12];
    AUDIO_ASSERT(sink && "[Mixer Stream][timer callback] get NULL sink");
    if(sink && sink->param.audio.irq_exist == false){
        memory_agent |= HAL_AUDIO_MEMORY_DL_DL12;
    }
    //--
    if(memory_agent) {
        setup_play_en(bt_clk, bt_phase, scenario_type, memory_agent);
        mixer_stream_trigger_debug_timer(memory_agent & HAL_AUDIO_MEMORY_DL_DL12 ? SINK_TYPE_AUDIO_DL12 : get_sink_type_by_agent((hal_audio_memory_t)memory_agent));
        MIXER_STREAM_LOG_I("setup_play_en, align bt_clk:0x%x, align bt_phase:0x%x, pre bt clk:0x%x, pre bt phase:0x%x, scenario_type:%d,memory_agent:%d", 6,
                            bt_clk,
                            bt_phase,
                            pre_bt_clk,
                            pre_bt_phase,
                            scenario_type,
                            memory_agent);
    } else {
        MIXER_STREAM_LOG_I("stream is running, no need set play en", 0);
    }
}