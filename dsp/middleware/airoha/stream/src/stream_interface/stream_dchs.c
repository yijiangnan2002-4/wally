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
#include "dsp_mux_uart.h"
#include "preloader_pisplit.h"
#include "source_inter.h"
#include "stream_dchs.h"
//#include "sw_gain_interface.h"
#include "mux_ll_uart.h"
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
#include "mux_ll_uart.h"
#include "bt_interface.h"
#include "hal_gpt.h"
#include "ch_select_interface.h"
#include "audio_transmitter_mcu_dsp_common.h"
#include "dsp_audio_msg.h"

log_create_module(DCHS_DL,       PRINT_LEVEL_INFO);
log_create_module(DCHS_DL_DEBUG, PRINT_LEVEL_WARNING);

log_create_module(DCHS_UL,       PRINT_LEVEL_INFO);
log_create_module(DCHS_UL_DEBUG, PRINT_LEVEL_WARNING);

#define DCHS_DL_LOG_E(fmt, arg...) LOG_MSGID_E(DCHS_DL,       "[DCHS DL] "fmt,##arg)
#define DCHS_DL_LOG_W(fmt, arg...) LOG_MSGID_W(DCHS_DL,       "[DCHS DL] "fmt,##arg)
#define DCHS_DL_LOG_I(fmt, arg...) LOG_MSGID_I(DCHS_DL,       "[DCHS DL] "fmt,##arg)
#define DCHS_DL_LOG_D(fmt, arg...) LOG_MSGID_I(DCHS_DL_DEBUG, "[DCHS DL][Debug] "fmt,##arg)

#define DCHS_UL_LOG_E(fmt, arg...) LOG_MSGID_E(DCHS_UL,       "[DCHS UL] "fmt,##arg)
#define DCHS_UL_LOG_W(fmt, arg...) LOG_MSGID_W(DCHS_UL,       "[DCHS UL] "fmt,##arg)
#define DCHS_UL_LOG_I(fmt, arg...) LOG_MSGID_I(DCHS_UL,       "[DCHS UL] "fmt,##arg)
#define DCHS_UL_LOG_D(fmt, arg...) LOG_MSGID_I(DCHS_UL_DEBUG, "[DCHS UL][Debug] "fmt,##arg)
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

EXTERN bool hal_audio_src_set_start(afe_src_configuration_t *configuration, hal_audio_memory_sync_selection_t sync_select, hal_audio_control_status_t control);
EXTERN bool hal_audio_src_configuration(afe_src_configuration_t *configuration, hal_audio_control_status_t control);
EXTERN VOID dsp_converter_16bit_to_32bit(S32 *des, S16 *src, U32  sample);
extern hal_nvic_status_t hal_nvic_save_and_set_interrupt_mask_special(uint32_t *mask);
extern hal_nvic_status_t hal_nvic_restore_interrupt_mask_special(uint32_t mask);

/******************************************************************************
 *
 * Private Macro and Variable Declaration
 *
 ******************************************************************************/
#define BIT_SET(var, bit)        (var  |=  (1U << bit))
#define BIT_RESET(var, bit)      (var  &= ~(1U << bit))
#define BIT_GET(var, bit)        ((var & (1U << bit)) >> bit)

#define BT_NCLK_MASK                (0x0FFFFFFC)

#ifdef AIR_CPU_IN_SECURITY_MODE
#define CONN_BT_TIMCON_BASE 0xA0000000
#else
#define CONN_BT_TIMCON_BASE 0xB0000000
#endif

#ifdef AIR_SOFTWARE_MIXER_ENABLE
static sw_mixer_member_t *mixer_member = NULL;
#endif
static sw_gain_port_t *dchs_gain_port = NULL;

static U32 dchs_dl_timer_handle   = 0;
bool   dchs_play_en_timeout_flag = false;
static U8 dchs_dl_scenario_play_en_exist_mask = 0;

typedef enum
{
    DCHS_DL_OTHER_CHIP_DL_EXIST,
    DCHS_DL_LOCAL_CHIP_DL_EXIST,
    DCHS_DL_SET_GAIN_VALUE,
    DCHS_DL_SET_PLAY_EN,
    //extend here
    SYNC_TYPE_MAX = 0xFFFFFFFF,
}dchs_mcu2dsp_sync_type_t;

typedef struct
{
    bool other_chip_dl_exist;
    bool local_chip_dl_exist;
    U8 format_bytes;
    U32 sample_rate;
    audio_scenario_type_t data_scenario_type;
    uint32_t operation;
    int32_t  vol_gain;
    U32     frame_size;
    U8      channel_num;
    dchs_dl_chip_role_t chip_role;
    //dl sync extend here
}dual_chip_dl_sync_msg_t;

typedef union{
    dual_chip_dl_sync_msg_t  dual_chip_dl_sync;
    // ul sync add here
}audio_dchs_mcu2dsp_sync_msg_t;

typedef struct
{
    dchs_mcu2dsp_sync_type_t sync_type;
    audio_dchs_mcu2dsp_sync_msg_t sync_msg;
}audio_dchs_mcu2dsp_cosys_sync_t;

static U8 * hwsrc1_buf_addr = NULL;
static U8 * hwsrc2_buf_addr = NULL;
//static U8 * hwsrc3_buf_addr = NULL;

#define HWSRC_BUF_LENGTH     (4096 * 2)

uint32_t g_dchs_dl_data_mix_count     = 0;
uint32_t g_dchs_dl_process_count      = 0;
static uint8_t  scenario_exist_mask   = 0;
dl_data_scenario_msg_t  dchs_dl_ch_scenario_msg[CH_MAX] = {0};
bool    g_dchs_dl_open_done_flag      = false;
dchs_dl_play_en_info_t g_dchs_dl_play_en_info = {false, AUDIO_SCENARIO_TYPE_COMMON, 0, 0};
static U32 dchs_dl_silence_count[CH_MAX] = {0};

/******************************************************************************
 *
 * Private Function Define
 *
 ******************************************************************************/
void dchs_dl_set_scenario_exist(buf_ch_type_t ch_type, bool is_running)
{
    is_running ? BIT_SET(scenario_exist_mask, ch_type) : BIT_RESET(scenario_exist_mask, ch_type);
}

bool dchs_dl_check_scenario_exist(buf_ch_type_t ch_type)
{
    return BIT_GET(scenario_exist_mask, ch_type);
}

void dchs_dl_set_scenario_play_en_exist(hal_audio_agent_t agent, bool is_enable)
{
    is_enable ? BIT_SET(dchs_dl_scenario_play_en_exist_mask, agent) : BIT_RESET(dchs_dl_scenario_play_en_exist_mask, agent);
}

bool dchs_dl_check_scenario_play_en_exist(hal_audio_agent_t agent)
{
    return BIT_GET(dchs_dl_scenario_play_en_exist_mask, agent);
}

void dchs_dl_source_ch_hwsrc_cfg(buf_ch_type_t ch_type, hwsrc_id_t hwsrc_id)
{
    dchs_dl_ch_scenario_msg[ch_type].hwsrc_info.hwsrc_id = hwsrc_id;
}

afe_mem_asrc_id_t dchs_dl_get_hwsrc_id(buf_ch_type_t ch_type)
{
    return dchs_dl_ch_scenario_msg[ch_type].hwsrc_info.hwsrc_id - 1;
}

void dchs_dl_hwsrc_set_parameters(buf_ch_type_t ch_type, afe_src_configuration_t *hwsrc_cfg)
{
    afe_mem_asrc_id_t hwsrc_id = dchs_dl_get_hwsrc_id(ch_type);
    SOURCE source = Source_blks[SOURCE_TYPE_UART];
    hwsrc_cfg->hw_update_obuf_rdpnt = false;
    hwsrc_cfg->is_mono = dchs_dl_ch_scenario_msg[ch_type].channel_num == 1 ? true : false;
    hwsrc_cfg->id = hwsrc_id;
    hwsrc_cfg->sample_count_threshold = AUDIO_DURATION_TIME * source->param.audio.rate / MUX_UART_BUF_SLICE / 1000;
    hwsrc_cfg->mode = AFE_SRC_NO_TRACKING;
    hwsrc_cfg->ul_mode = false;
    if(hwsrc_id == AFE_MEM_ASRC_1 && !hwsrc1_buf_addr){
        hwsrc1_buf_addr = preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, HWSRC_BUF_LENGTH);
        if(!hwsrc1_buf_addr){
            AUDIO_ASSERT(0 && "[DCHS DL][HWSRC driver Cfg] hwsrc1 buf malloc fail");
        }
        memset(hwsrc1_buf_addr, 0, HWSRC_BUF_LENGTH);
    }else if(hwsrc_id == AFE_MEM_ASRC_2 && !hwsrc2_buf_addr){
        hwsrc2_buf_addr = preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, HWSRC_BUF_LENGTH);
        if(!hwsrc2_buf_addr){
            AUDIO_ASSERT(0 && "[DCHS DL][HWSRC driver Cfg] hwsrc2 buf malloc fail!");
        }
        memset(hwsrc2_buf_addr, 0, HWSRC_BUF_LENGTH);
    }
    //intput buffer
    hwsrc_cfg->input_buffer.addr   = (U32)dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->startaddr[0];
    hwsrc_cfg->input_buffer.size   = dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->length;
    hwsrc_cfg->input_buffer.rate   = dchs_dl_ch_scenario_msg[ch_type].sample_rate;
    hwsrc_cfg->input_buffer.offset = 0;
    hwsrc_cfg->input_buffer.format = dchs_dl_ch_scenario_msg[ch_type].format_bytes == 4 ? HAL_AUDIO_PCM_FORMAT_S32_LE : HAL_AUDIO_PCM_FORMAT_S16_LE;
    //output buffer
    hwsrc_cfg->output_buffer.addr   = (hwsrc_id == AFE_MEM_ASRC_1) ? (U32)hwsrc1_buf_addr : (U32)hwsrc2_buf_addr;
    hwsrc_cfg->output_buffer.size   = HWSRC_BUF_LENGTH;
    hwsrc_cfg->output_buffer.rate   = source->param.audio.rate;
    hwsrc_cfg->output_buffer.offset = 32;
    hwsrc_cfg->output_buffer.format = hwsrc_cfg->input_buffer.format;

    DCHS_DL_LOG_I("[HWSRC Driver Cfg] hwsrc id:%d, in rate:%d, in addr=0x%x,in size=%d,in format=%d,out addr=0x%d,out size=%d,out format=%d,out rate:%d, is mono=%d", 10,
                 hwsrc_cfg->id+1, hwsrc_cfg->input_buffer.rate, hwsrc_cfg->input_buffer.addr,hwsrc_cfg->input_buffer.size,hwsrc_cfg->input_buffer.format,
                 hwsrc_cfg->output_buffer.addr,hwsrc_cfg->output_buffer.size,hwsrc_cfg->output_buffer.format,hwsrc_cfg->output_buffer.rate,hwsrc_cfg->is_mono);
}

void dchs_dl_source_ch_hwsrc_driver_control(buf_ch_type_t ch_type, bool control)
{
    if(dchs_dl_ch_scenario_msg[ch_type].hwsrc_info.hwsrc_id && dchs_dl_ch_scenario_msg[ch_type].hwsrc_info.is_enable != control){
        if(control){
            afe_src_configuration_t hwsrc_cfg;
            dchs_dl_hwsrc_set_parameters(ch_type, &hwsrc_cfg);
            hal_audio_src_configuration(&hwsrc_cfg, control);
            hal_audio_src_set_start(&hwsrc_cfg, HAL_AUDIO_MEMORY_SYNC_NONE, control);
            #if 1
            U32 hwsrc_id    = (dchs_dl_ch_scenario_msg[ch_type].hwsrc_info.hwsrc_id - 1);
            U32 addr_offset = hwsrc_id * 0x100;
            DCHS_DL_LOG_D("[HWSRC%d Driver]ASM_FREQUENCY_0=0x%x",2, hwsrc_id+1, AFE_READ(ASM_FREQUENCY_0 + addr_offset));
            DCHS_DL_LOG_D("[HWSRC%d Driver]ASM_FREQUENCY_1=0x%x",2, hwsrc_id+1, AFE_READ(ASM_FREQUENCY_1 + addr_offset));
            DCHS_DL_LOG_D("[HWSRC%d Driver]ASM_FREQUENCY_2=0x%x",2, hwsrc_id+1, AFE_READ(ASM_FREQUENCY_2 + addr_offset));
            DCHS_DL_LOG_D("[HWSRC%d Driver]ASM_FREQUENCY_3=0x%x",2, hwsrc_id+1, AFE_READ(ASM_FREQUENCY_3 + addr_offset));
            DCHS_DL_LOG_D("[HWSRC%d Driver]ASM_CALI_DENOMINATOR=0x%x",2, hwsrc_id+1, AFE_READ(ASM_CALI_DENOMINATOR + addr_offset));
            DCHS_DL_LOG_D("[HWSRC%d Driver]ASM_MAX_OUT_PER_IN0=0x%x",2, hwsrc_id+1, AFE_READ(ASM_MAX_OUT_PER_IN0 + addr_offset));
            DCHS_DL_LOG_D("[HWSRC%d Driver]ASM_FREQ_CALI_CYC=0x%x",2, hwsrc_id+1, AFE_READ(ASM_FREQ_CALI_CYC + addr_offset));
            DCHS_DL_LOG_D("[HWSRC%d Driver]ASM_FREQ_CALI_CTRL=0x%x",2, hwsrc_id+1, AFE_READ(ASM_FREQ_CALI_CTRL + addr_offset));
            DCHS_DL_LOG_D("[HWSRC%d Driver]ASM_GEN_CONF=0x%x",2, hwsrc_id+1, AFE_READ(ASM_GEN_CONF + addr_offset));
            DCHS_DL_LOG_D("[HWSRC%d Driver]ASM_CH01_CNFG=0x%x",2, hwsrc_id+1, AFE_READ(ASM_CH01_CNFG + addr_offset));
            DCHS_DL_LOG_D("[HWSRC%d Driver]ASM_IER=0x%x",2, hwsrc_id+1, AFE_READ(ASM_IER + addr_offset));
            DCHS_DL_LOG_D("[HWSRC%d Driver]ASM_IFR=0x%x",2, hwsrc_id+1, AFE_READ(ASM_IFR + addr_offset));
            #endif
            if(dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->bBufferIsFull){
                dchs_dl_update_hwsrc_input_wrpnt(ch_type, dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->length);
            }else{
                dchs_dl_update_hwsrc_input_wrpnt(ch_type, dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->WriteOffset);
            }
        }else{
            afe_src_configuration_t hwsrc_cfg;
            dchs_dl_hwsrc_set_parameters(ch_type, &hwsrc_cfg);
            hal_audio_src_set_start(&hwsrc_cfg, HAL_AUDIO_MEMORY_SYNC_NONE, control);
            hal_audio_src_configuration(&hwsrc_cfg, control);
        }
        dchs_dl_ch_scenario_msg[ch_type].hwsrc_info.is_enable = control;
        DCHS_DL_LOG_I("[HWSRC Driver] ch_type=%d, hwsrc id : %d, control : %d, sink buf addr=0x%x, wo=%d", 5, ch_type, dchs_dl_ch_scenario_msg[ch_type].hwsrc_info.hwsrc_id, control, dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->startaddr[0],dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->WriteOffset);
    }
}

bool dchs_dl_check_hwsrc_enable(buf_ch_type_t ch_type)
{
    if(dchs_dl_ch_scenario_msg[ch_type].hwsrc_info.hwsrc_id && dchs_dl_ch_scenario_msg[ch_type].hwsrc_info.is_enable){
        return true;
    }
    return false;
}

void dchs_dl_update_hwsrc_input_wrpnt(buf_ch_type_t ch_type, U32 iwo)
{
    afe_mem_asrc_id_t hwsrc_id = dchs_dl_get_hwsrc_id(ch_type);
    U32 addr_offset = hwsrc_id * 0x100;
    AFE_WRITE(ASM_CH01_IBUF_WRPNT + addr_offset, iwo + AFE_READ(ASM_IBUF_SADR + addr_offset));
    DCHS_DL_LOG_D("update hwsrc input, get wo=%d, ch type=%d, hwsrc iwo=%d, iro=%d",4, iwo, ch_type, AFE_READ(ASM_CH01_IBUF_WRPNT + addr_offset) - AFE_READ(ASM_IBUF_SADR + addr_offset), AFE_READ(ASM_CH01_IBUF_RDPNT + addr_offset) - AFE_READ(ASM_IBUF_SADR + addr_offset));
}

void SourceDCHSDLSwGainInit(SOURCE source)
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
    dchs_gain_port = stream_function_sw_gain_get_port(source);
    default_gain =  0xFFFFF15A;
    U32 ch_num = 0, total_ch_num= 0;
    if(source->param.audio.channel_num == USB_IN_8_CHANNEL){
        total_ch_num = 10;
    }else{
        total_ch_num = 6;
    }
    stream_function_sw_gain_init(dchs_gain_port, total_ch_num, &default_config);
    DCHS_DL_LOG_I("[SW Gain][Init]total channel %d, default gain = %ddB", 2, total_ch_num, default_gain);
    for(ch_num = 1; ch_num <= total_ch_num; ch_num ++){
        stream_function_sw_gain_configure_gain_target(dchs_gain_port, ch_num, default_gain); //-120 = mute
    }
    //check if have gain need set
    buf_ch_type_t ch_type;
    for(ch_type = LOCAL_SCENARIO_1; ch_type <= UART_SCENARIO; ch_type++){
        if(dchs_dl_ch_scenario_msg[ch_type].waiting_set_volume){
            dchs_dl_runtime_config_operation_t operation = ((ch_type == UART_SCENARIO) ? DCHS_DL_CONFIG_OP_SET_UART_SCENARIO_VOL_INFO : (ch_type + 1));
            sw_gain_config_t old_config;
            stream_function_sw_gain_get_config(dchs_gain_port, operation * 2 + 1, &old_config);
            DCHS_DL_LOG_I("[Sw Gain][config in init] operation %d, set ch:%d and %d gain from 0x%x to 0x%x\r\n", 5,
                        operation,
                        operation * 2 + 1,
                        operation * 2 + 2,
                        old_config.target_gain,
                        dchs_dl_ch_scenario_msg[ch_type].vol_gain);
            stream_function_sw_gain_configure_gain_target(dchs_gain_port, operation * 2 + 1, dchs_dl_ch_scenario_msg[ch_type].vol_gain);
            stream_function_sw_gain_configure_gain_target(dchs_gain_port, operation * 2 + 2, dchs_dl_ch_scenario_msg[ch_type].vol_gain);
            dchs_dl_ch_scenario_msg[ch_type].waiting_set_volume = false;
        }
    }
#endif /* AIR_SOFTWARE_GAIN_ENABLE */
}

static void dchs_dl_sw_mixer_postcallback(sw_mixer_member_t *member, void *para, uint32_t *out_frame_size)
{
    UNUSED(member);
    UNUSED(out_frame_size);
    ((DSP_ENTRY_PARA_PTR)para)->out_channel_num = 2;
    DCHS_DL_LOG_D("[SW mix][postcallback]total channel %d", 1, ((DSP_ENTRY_PARA_PTR)para)->out_channel_num);
}

void SourceDCHSDLSwMixerInit(SOURCE source)
{
    #ifdef AIR_SOFTWARE_MIXER_ENABLE
    /* sw mixer config */
    sw_mixer_callback_config_t       callback_config;
    sw_mixer_input_channel_config_t  in_ch_config;
    sw_mixer_output_channel_config_t out_ch_config;
    stream_function_sw_mixer_init(SW_MIXER_PORT_0);
    callback_config.preprocess_callback  = NULL;
    callback_config.postprocess_callback = dchs_dl_sw_mixer_postcallback;
    in_ch_config.resolution = RESOLUTION_32BIT;
    in_ch_config.input_mode = SW_MIXER_CHANNEL_MODE_NORMAL;
    //in_ch_config.buffer_size = AUDIO_DURATION_TIME * DCHS_DL_FIX_SAMPLE_RATE / MUX_UART_BUF_SLICE / 1000 * 4;//4:32bit

    if(source->param.audio.channel_num == USB_IN_8_CHANNEL){
        in_ch_config.total_channel_number = 10;
    }else{
        in_ch_config.total_channel_number = 6;
    }
    in_ch_config.buffer_size = source->param.audio.frame_size * source->param.audio.format_bytes;
    out_ch_config.total_channel_number = 2;
    out_ch_config.resolution = RESOLUTION_32BIT;
    mixer_member = stream_function_sw_mixer_member_create((void *)source,
                                                                         SW_MIXER_MEMBER_MODE_NO_BYPASS,
                                                                         &callback_config,
                                                                         &in_ch_config,
                                                                         &out_ch_config);

    stream_function_sw_mixer_member_register(SW_MIXER_PORT_0, mixer_member, false);
    if(source->param.audio.channel_num == USB_IN_8_CHANNEL){
        /* do connections */
        //out L ch
        stream_function_sw_mixer_channel_connect(mixer_member, 1, SW_MIXER_CHANNEL_ATTRIBUTE_MAIN,   mixer_member, 1);
        stream_function_sw_mixer_channel_connect(mixer_member, 3, SW_MIXER_CHANNEL_ATTRIBUTE_NORMAL, mixer_member, 1);
        stream_function_sw_mixer_channel_connect(mixer_member, 5, SW_MIXER_CHANNEL_ATTRIBUTE_NORMAL, mixer_member, 1);
        stream_function_sw_mixer_channel_connect(mixer_member, 7, SW_MIXER_CHANNEL_ATTRIBUTE_NORMAL, mixer_member, 1);
        stream_function_sw_mixer_channel_connect(mixer_member, 9, SW_MIXER_CHANNEL_ATTRIBUTE_NORMAL, mixer_member, 1);
        //out R ch
        stream_function_sw_mixer_channel_connect(mixer_member, 2, SW_MIXER_CHANNEL_ATTRIBUTE_MAIN,   mixer_member, 2);
        stream_function_sw_mixer_channel_connect(mixer_member, 4, SW_MIXER_CHANNEL_ATTRIBUTE_NORMAL, mixer_member, 2);
        stream_function_sw_mixer_channel_connect(mixer_member, 6, SW_MIXER_CHANNEL_ATTRIBUTE_NORMAL, mixer_member, 2);
        stream_function_sw_mixer_channel_connect(mixer_member, 8, SW_MIXER_CHANNEL_ATTRIBUTE_NORMAL, mixer_member, 2);
        stream_function_sw_mixer_channel_connect(mixer_member, 10,SW_MIXER_CHANNEL_ATTRIBUTE_NORMAL, mixer_member, 2);
    }else{
        /* do connections */
        //out L ch
        stream_function_sw_mixer_channel_connect(mixer_member, 1, SW_MIXER_CHANNEL_ATTRIBUTE_MAIN,   mixer_member, 1);
        stream_function_sw_mixer_channel_connect(mixer_member, 3, SW_MIXER_CHANNEL_ATTRIBUTE_NORMAL, mixer_member, 1);
        stream_function_sw_mixer_channel_connect(mixer_member, 5, SW_MIXER_CHANNEL_ATTRIBUTE_NORMAL, mixer_member, 1);
        //out R ch
        stream_function_sw_mixer_channel_connect(mixer_member, 2, SW_MIXER_CHANNEL_ATTRIBUTE_MAIN,   mixer_member, 2);
        stream_function_sw_mixer_channel_connect(mixer_member, 4, SW_MIXER_CHANNEL_ATTRIBUTE_NORMAL, mixer_member, 2);
        stream_function_sw_mixer_channel_connect(mixer_member, 6, SW_MIXER_CHANNEL_ATTRIBUTE_NORMAL, mixer_member, 2);
    }
    #endif//AIR_SOFTWARE_MIXER_ENABLE
}

U32 dchs_dl_get_ch_data_size(buf_ch_type_t ch_type, bool hwsrc_enable)
{
    if(hwsrc_enable){
        afe_mem_asrc_id_t hwsrc_id = dchs_dl_get_hwsrc_id(ch_type);
        U32 addr_offset = hwsrc_id * 0x100;
        U32 oro  = AFE_READ(ASM_CH01_OBUF_RDPNT + addr_offset) - AFE_READ(ASM_OBUF_SADR + addr_offset);
        U32 owo = AFE_READ(ASM_CH01_OBUF_WRPNT + addr_offset) - AFE_READ(ASM_OBUF_SADR + addr_offset);
        U32 length   = AFE_READ(ASM_OBUF_SIZE + addr_offset);
        U32 data_size = (owo > oro) ? (owo - oro) : (length - oro + owo);
        return (data_size - 32);//-32 to avoid write = read point case
    }else{
        BUFFER_INFO_PTR buf_info = NULL;
        if(ch_type == DCHS_DL_SINK){
            volatile SINK dchs_dl_sink = Sink_blks[SINK_TYPE_AUDIO_DL12];
            buf_info = &(dchs_dl_sink->streamBuffer.BufferInfo);
        }else{
            buf_info = dchs_dl_ch_scenario_msg[ch_type].sink_buf_info;
        }
        AUDIO_ASSERT(buf_info != NULL && "[get ch size]sink buf == NULL");
        if(buf_info->bBufferIsFull){
            return buf_info->length;
        }
        U32 owo     = buf_info->WriteOffset;
        U32 oro     = buf_info->ReadOffset;
        U32 length  = buf_info->length;
        U32 data_size = (owo >= oro) ? (owo - oro) : (length - oro + owo);
        //DCHS_DL_LOG_D("[get size]ch_type=%d,owo=%d, oro=%d,data_size=%d,length=%d", 5, ch_type,owo, oro,data_size,length);
        return data_size;
    }
    return 0;
}

U32 dchs_dl_get_source_ch_remain_size(buf_ch_type_t ch_type)
{
    if(ch_type == UART_SCENARIO){
        if(dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->bBufferIsFull){
            return 0;
        }
        U32 wo     = dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->WriteOffset;
        U32 ro     = dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->ReadOffset;
        U32 length = dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->length;
        return (wo >= ro) ? (length + ro - wo) : (ro - wo);
    }else{
        return 0;
    }
}


void dchs_dl_copy_uart_data_2_source_buf()
{
    U32 uart_buf_size  = dsp_query_uart_rx_buf_remain_size(UART_DL);
    U32 uart_scenario_reamain = dchs_dl_get_source_ch_remain_size(UART_SCENARIO) - 32; //avoid upadte hwsrc wo = ro case
    U32 copy_uart_data_size = MIN(uart_buf_size, uart_scenario_reamain);
    if(dchs_dl_ch_scenario_msg[UART_SCENARIO].sink_buf_info->bBufferIsFull == TRUE){
        DCHS_DL_LOG_W("[Source Read] source uart buf full", 0);
        return;
    }
    DCHS_DL_LOG_D("[Source Read] mux uart_buf_size=%d,uart_scenario_reamain=%d,copy_uart_data_size=%d", 3,uart_buf_size,uart_scenario_reamain,copy_uart_data_size);
    if(copy_uart_data_size){
        U32 wo        = dchs_dl_ch_scenario_msg[UART_SCENARIO].sink_buf_info->WriteOffset;
        U32 length    = dchs_dl_ch_scenario_msg[UART_SCENARIO].sink_buf_info->length;
        U8 *src_start = dchs_dl_ch_scenario_msg[UART_SCENARIO].sink_buf_info->startaddr[0];
        if (length - wo >= copy_uart_data_size){
            dsp_uart_rx(UART_DL, src_start + wo, copy_uart_data_size);
            LOG_AUDIO_DUMP(src_start + wo, copy_uart_data_size, AUDIO_DCHS_DL_UART_RX);
        }else{
            //read part1 unrape
            dsp_uart_rx(UART_DL, src_start + wo, length - wo);
            LOG_AUDIO_DUMP(src_start + wo, length - wo, AUDIO_DCHS_DL_UART_RX);
            //read part2 rape
            dsp_uart_rx(UART_DL, src_start, copy_uart_data_size - (length - wo));
            LOG_AUDIO_DUMP(src_start, copy_uart_data_size - (length - wo), AUDIO_DCHS_DL_UART_RX);
        }
        dchs_dl_ch_scenario_msg[UART_SCENARIO].sink_buf_info->WriteOffset = (wo + copy_uart_data_size) % (length);
        if(dchs_dl_check_hwsrc_enable(UART_SCENARIO)){
            dchs_dl_update_hwsrc_input_wrpnt(UART_SCENARIO, dchs_dl_ch_scenario_msg[UART_SCENARIO].sink_buf_info->WriteOffset);
        }
    }else{
        //DCHS_DL_AUDIO_LOG_I("[Source Read] copy_uart_data_size=0",0);
    }
}

void dchs_dl_i2d_buffer_copy(buf_ch_type_t ch_type, U8 *dst_addr_L, U8 *dst_addr_R, U32 length, bool hwsrc_enable, U32 format_bytes, U8 channel_sel)
{

    afe_mem_asrc_id_t hwsrc_id = dchs_dl_get_hwsrc_id(ch_type);
    U32 addr_offset = hwsrc_id * 0x100;
    U32 oro   = hwsrc_enable ? AFE_READ(ASM_CH01_OBUF_RDPNT + addr_offset) - AFE_READ(ASM_OBUF_SADR + addr_offset) : dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->ReadOffset;
    U32 owo   = hwsrc_enable ? AFE_READ(ASM_CH01_OBUF_WRPNT + addr_offset) - AFE_READ(ASM_OBUF_SADR + addr_offset) : dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->WriteOffset;
    U8 *src_addr    = hwsrc_enable ? (U8*)AFE_READ(ASM_CH01_OBUF_RDPNT + addr_offset) : dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->startaddr[channel_sel] + dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->ReadOffset;
    U8 *src_start   = hwsrc_enable ? (U8*)AFE_READ(ASM_OBUF_SADR + addr_offset) : dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->startaddr[channel_sel];
    U32 unwrap_size = hwsrc_enable ? AFE_READ(ASM_OBUF_SIZE + addr_offset) - (AFE_READ(ASM_CH01_OBUF_RDPNT + addr_offset) - AFE_READ(ASM_OBUF_SADR + addr_offset))
                                    : dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->length - dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->ReadOffset;
    U32 buf_length  = hwsrc_enable ? AFE_READ(ASM_OBUF_SIZE + addr_offset) : dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->length;
    U32 wrap_size   = unwrap_size >= length ? 0 : (length - unwrap_size);
    U32 gpt_count = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count);
    DCHS_DL_LOG_D("[Source I2D]ch type=%d,length=%d,format_bytes=%d,owo=%d,oro=%d,unwrap_size=%d,wrap_size=%d, src buf addr=0x%x, channel_num=%d,gpt_count=%d,src_addr:0x%x", 11 ,ch_type,length,format_bytes,owo,oro,unwrap_size,wrap_size,dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->startaddr[channel_sel],dchs_dl_ch_scenario_msg[ch_type].channel_num, gpt_count,src_addr);
    if(dchs_dl_ch_scenario_msg[ch_type].channel_num == 1){
        DSP_C2D_BufferCopy((VOID *)dst_addr_L,
                        (VOID *)src_addr,
                        length,
                        (VOID *)src_start,
                        buf_length);
        memcpy((void *)dst_addr_R, (void *)dst_addr_L, length);
    }else{ //stere source data
        U8 channel_shift = (dchs_dl_ch_scenario_msg[ch_type].channel_num >= 2);
        if(owo > oro){
            if(format_bytes == 4){
                DSP_I2D_BufferCopy_32bit_mute((U32 *)src_addr,
                                            (U32 *)dst_addr_L,
                                            (U32 *)dst_addr_R,
                                            (length) >> 2 >> channel_shift,
                                            false);
            }else{
                DSP_I2D_BufferCopy_16bit_mute((U16 *)src_addr,
                                            (U16 *)dst_addr_L,
                                            (U16 *)dst_addr_R,
                                            (length) >> 1 >> channel_shift,
                                            false);

            }
            if(ch_type == UART_SCENARIO){
                LOG_AUDIO_DUMP(src_addr, length ,AUDIO_DCHS_DL_UART_SCENARIO_SOURCE);
            }else if(ch_type == LOCAL_SCENARIO_1){
                LOG_AUDIO_DUMP(src_addr, length ,AUDIO_DCHS_DL_LOCAL_SCENARIO_1_SOURCE);
            }else if(ch_type == LOCAL_SCENARIO_2){
                LOG_AUDIO_DUMP(src_addr, length ,AUDIO_DCHS_DL_LOCAL_SCENARIO_2_SOURCE);
            }
        }else{
            if(format_bytes == 4){
                DSP_I2D_BufferCopy_32bit_mute((U32 *)src_addr,
                                            (U32 *)dst_addr_L,
                                            (U32 *)dst_addr_R,
                                            (MIN(unwrap_size, length)) >> 2 >> channel_shift,
                                            false);
                if(ch_type == UART_SCENARIO){
                    LOG_AUDIO_DUMP(src_addr, MIN(unwrap_size, length) ,AUDIO_DCHS_DL_UART_SCENARIO_SOURCE);
                }else if(ch_type == LOCAL_SCENARIO_1){
                    LOG_AUDIO_DUMP(src_addr, MIN(unwrap_size, length) ,AUDIO_DCHS_DL_LOCAL_SCENARIO_1_SOURCE);
                }else if(ch_type == LOCAL_SCENARIO_2){
                    LOG_AUDIO_DUMP(src_addr, MIN(unwrap_size, length) ,AUDIO_DCHS_DL_LOCAL_SCENARIO_2_SOURCE);
                }
                if(wrap_size){
                    DSP_I2D_BufferCopy_32bit_mute((U32 *)src_start,
                                            (U32 *)(dst_addr_L + unwrap_size/2),
                                            (U32 *)(dst_addr_R + unwrap_size/2),
                                            (wrap_size) >> 2 >> channel_shift,
                                            false);
                    if(ch_type == UART_SCENARIO){
                        LOG_AUDIO_DUMP(src_start, wrap_size ,AUDIO_DCHS_DL_UART_SCENARIO_SOURCE);
                    }else if(ch_type == LOCAL_SCENARIO_1){
                        LOG_AUDIO_DUMP(src_start, wrap_size ,AUDIO_DCHS_DL_LOCAL_SCENARIO_1_SOURCE);
                    }else if(ch_type == LOCAL_SCENARIO_2){
                        LOG_AUDIO_DUMP(src_start, wrap_size ,AUDIO_DCHS_DL_LOCAL_SCENARIO_2_SOURCE);
                    }
                }
            }else{
                DSP_I2D_BufferCopy_16bit_mute((U16 *)src_addr,
                                            (U16 *)dst_addr_L,
                                            (U16 *)dst_addr_R,
                                            (MIN(unwrap_size, length)) >> 1 >> channel_shift,
                                            false);
                if(ch_type == UART_SCENARIO){
                    LOG_AUDIO_DUMP(src_addr, MIN(unwrap_size, length) ,AUDIO_DCHS_DL_UART_SCENARIO_SOURCE);
                }else if(ch_type == LOCAL_SCENARIO_1){
                    LOG_AUDIO_DUMP(src_addr, MIN(unwrap_size, length) ,AUDIO_DCHS_DL_LOCAL_SCENARIO_1_SOURCE);
                }else if(ch_type == LOCAL_SCENARIO_2){
                    LOG_AUDIO_DUMP(src_addr, MIN(unwrap_size, length) ,AUDIO_DCHS_DL_LOCAL_SCENARIO_2_SOURCE);
                }
                if(wrap_size){
                    DSP_I2D_BufferCopy_16bit_mute((U16 *)src_start,
                                            (U16 *)(dst_addr_L + unwrap_size/2),
                                            (U16 *)(dst_addr_R + unwrap_size/2),
                                            (wrap_size) >> 1 >> channel_shift,
                                            false);
                    if(ch_type == UART_SCENARIO){
                        LOG_AUDIO_DUMP(src_start, wrap_size ,AUDIO_DCHS_DL_UART_SCENARIO_SOURCE);
                    }else if(ch_type == LOCAL_SCENARIO_1){
                        LOG_AUDIO_DUMP(src_start, wrap_size ,AUDIO_DCHS_DL_LOCAL_SCENARIO_1_SOURCE);
                    }else if(ch_type == LOCAL_SCENARIO_2){
                        LOG_AUDIO_DUMP(src_start, wrap_size ,AUDIO_DCHS_DL_LOCAL_SCENARIO_2_SOURCE);
                    }
                }
            }
        }
    }
    if(format_bytes == 2){
        dsp_converter_16bit_to_32bit((int32_t *)dst_addr_L, (int16_t *)dst_addr_L, length / 2 / sizeof(int16_t));
        dsp_converter_16bit_to_32bit((int32_t *)dst_addr_R, (int16_t *)dst_addr_R, length / 2 / sizeof(int16_t));
    }
}

void dchs_dl_reset_dchs_dl_stream_status(void)
{
    dchs_dl_silence_count[LOCAL_SCENARIO_1] = 0;
    dchs_dl_silence_count[LOCAL_SCENARIO_2] = 0;
    dchs_dl_silence_count[UART_SCENARIO] = 0;
    dchs_play_en_timeout_flag = false;
    g_dchs_dl_data_mix_count = 0;
    g_dchs_dl_process_count = MUX_UART_BUF_SLICE; // first
}

void dchs_dl_resume_dchs_task(void)
{
    volatile SOURCE dchs_dl_source = Source_blks[SOURCE_TYPE_UART];
    if(dchs_dl_source && dchs_dl_source->transform && dchs_dl_source->transform->source){
        //DCHS_DL_LOG_W("shengbing,resume task",0);
        uint32_t mask = 0;
        hal_nvic_save_and_set_interrupt_mask_special(&mask);
        AudioCheckTransformHandle(dchs_dl_source->transform);
        hal_nvic_restore_interrupt_mask_special(mask);
    }
}

void dchs_dl_source_ch_buf_drop(buf_ch_type_t ch_type, U32 amount, bool hwsrc_enable)
{
    if(!amount){
        return;
    }
    afe_mem_asrc_id_t hwsrc_id = dchs_dl_get_hwsrc_id(ch_type);
    U32 addr_offset = hwsrc_id * 0x100;
    if(hwsrc_enable){
        U32 update_ro = (amount + AFE_READ(ASM_CH01_OBUF_RDPNT + addr_offset) -  AFE_READ(ASM_OBUF_SADR + addr_offset)) % AFE_READ(ASM_OBUF_SIZE + addr_offset);
        AFE_WRITE(ASM_CH01_OBUF_RDPNT + addr_offset, update_ro + AFE_READ(ASM_OBUF_SADR + addr_offset));
        if(ch_type == UART_SCENARIO){ //local scenario update ro in irq handler
            if (dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->bBufferIsFull == TRUE) {
                dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->bBufferIsFull = FALSE;
            }
            //if hwsrc enable, update uart buf ro with hwsrc input ro
            uint32_t osize,cur_read_offset;
            static uint32_t pre_read_offset = 0;
            pre_read_offset = dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->ReadOffset;
            //++ update uart ro
            dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->ReadOffset = (AFE_READ(ASM_CH01_IBUF_RDPNT + addr_offset) - AFE_READ(ASM_IBUF_SADR + addr_offset)) % AFE_READ(ASM_IBUF_SIZE + addr_offset);
            //--
            osize = dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->length;
            cur_read_offset = dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->ReadOffset;
            U32 cur_data_size = cur_read_offset >= pre_read_offset
                                    ? cur_read_offset - pre_read_offset
                                    : cur_read_offset + osize - pre_read_offset;
            U16 data_size1 = 0,data_size2=0,data_size=0;
            if (cur_read_offset >= pre_read_offset) {
                data_size = cur_read_offset - pre_read_offset;
                LOG_AUDIO_DUMP(dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->startaddr[0]+pre_read_offset,cur_data_size,AUDIO_DCHS_DL_UART_SCENARIO_HWSRC_IN);
            } else {
                data_size1 = osize - pre_read_offset;
                LOG_AUDIO_DUMP(dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->startaddr[0]+pre_read_offset,data_size1,AUDIO_DCHS_DL_UART_SCENARIO_HWSRC_IN);
                data_size2 = cur_read_offset;
                LOG_AUDIO_DUMP(dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->startaddr[0],data_size2,AUDIO_DCHS_DL_UART_SCENARIO_HWSRC_IN);
            }
        }
    }else{
        if (dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->bBufferIsFull == TRUE) {
            dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->bBufferIsFull = FALSE;
        }
        dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->ReadOffset = (dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->ReadOffset + amount) % (dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->length);
    }
    DCHS_DL_LOG_D("[Source Drop] drop size:%d, ch type:%d, hwsrc id:%d, hwsrc ro:%d, hwsrc enable:%d, normal ro=%d", 6, amount, ch_type, hwsrc_id + 1, AFE_READ(ASM_CH01_OBUF_RDPNT + addr_offset) - AFE_READ(ASM_OBUF_SADR + addr_offset), hwsrc_enable,dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->ReadOffset);
}

void dchs_dl_copy_source_ch_data_2_stream_buf(buf_ch_type_t ch_type, U8 *dst_addr_L, U8 *dst_addr_R, U32 length, bool hwsrc_enable)
{
    U32 ch_data_size = 0;
    if(ch_type != LOCAL_SCENARIO_2){
        ch_data_size = dchs_dl_get_ch_data_size(ch_type, hwsrc_enable);
        U32 drop_bytes = MIN((ch_data_size >= length ? ch_data_size - length : 0), dchs_dl_silence_count[ch_type]);
        if(drop_bytes){
            dchs_dl_source_ch_buf_drop(ch_type, drop_bytes, hwsrc_enable);
            dchs_dl_silence_count[ch_type] -= drop_bytes;
            DCHS_DL_LOG_W("[Source Drop] source ch: %d, ch size:%d, length:%d, drop_size: %d for silence before.", 4, ch_type, ch_data_size, length, drop_bytes);
        }
    }
    ch_data_size = dchs_dl_get_ch_data_size(ch_type, hwsrc_enable);
    DCHS_DL_LOG_D("[Source Read] ch type:%d, ch_data_size:%d, length:%d, hwsrc_enable=%d, format_bytes=%d", 5, ch_type, ch_data_size, length, hwsrc_enable, dchs_dl_ch_scenario_msg[ch_type].format_bytes);
    if(ch_data_size >= length){
        //DCHS_DL_AUDIO_LOG_I("[Source Read] source ch:%d, ch_data_siz:%d, length=%d", 3, ch_type,ch_data_size,length);
        //dchs_dl_i2d_buffer_copy(ch_type, dst_addr_L, dst_addr_R, length, hwsrc_enable, dchs_dl_ch_scenario_msg[ch_type].format_bytes, 0);
        U32 i;//copy 8ch usb in data to stream
        U32 ch_num = dchs_dl_ch_scenario_msg[ch_type].channel_num;
        SOURCE source = Source_blks[SOURCE_TYPE_UART];
        TRANSFORM transform =  source->transform;
        volatile DSP_CALLBACK_PTR callback_ptr = DSP_Callback_Get(source, transform->sink);
        U8  channel_sel = 0;
        for(i = 0; i < ch_num; i += 2){
            DCHS_DL_LOG_D("[copy and drop] source ch: %d, length: %d, ro:%d,wo:%d", 4, ch_type, length, dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->ReadOffset, dchs_dl_ch_scenario_msg[ch_type].sink_buf_info->WriteOffset);
            if(ch_num > 2){
                dst_addr_L = callback_ptr->EntryPara.in_ptr[i];
                dst_addr_R = callback_ptr->EntryPara.in_ptr[i+1];
            }
            dchs_dl_i2d_buffer_copy(ch_type, dst_addr_L, dst_addr_R, length, hwsrc_enable, dchs_dl_ch_scenario_msg[ch_type].format_bytes, channel_sel);
            if (i == 0) {
                LOG_AUDIO_DUMP((U8 *)(callback_ptr->EntryPara.in_ptr[i]), length/2, AUDIO_DCHS_DL_SOURCE_IN_CH_1);
                LOG_AUDIO_DUMP((U8 *)(callback_ptr->EntryPara.in_ptr[i + 1]), length/2, AUDIO_DCHS_DL_SOURCE_IN_CH_2);
            } else if (i == 2) {
                LOG_AUDIO_DUMP((U8 *)(callback_ptr->EntryPara.in_ptr[i]), length/2, AUDIO_DCHS_DL_SOURCE_IN_CH_3);
                LOG_AUDIO_DUMP((U8 *)(callback_ptr->EntryPara.in_ptr[i + 1]), length/2, AUDIO_DCHS_DL_SOURCE_IN_CH_4);
            } else if (i == 4) {
                LOG_AUDIO_DUMP((U8 *)(callback_ptr->EntryPara.in_ptr[i]), length/2, AUDIO_DCHS_DL_SOURCE_IN_CH_5);
                LOG_AUDIO_DUMP((U8 *)(callback_ptr->EntryPara.in_ptr[i + 1]), length/2, AUDIO_DCHS_DL_SOURCE_IN_CH_6);
            } else if (i == 6) {
                LOG_AUDIO_DUMP((U8 *)(callback_ptr->EntryPara.in_ptr[i]), length/2, AUDIO_DCHS_DL_SOURCE_IN_CH_7);
                LOG_AUDIO_DUMP((U8 *)(callback_ptr->EntryPara.in_ptr[i + 1]), length/2, AUDIO_DCHS_DL_SOURCE_IN_CH_8);
            }
            channel_sel++;
        }
        if(ch_type == LOCAL_SCENARIO_2){
            dchs_dl_source_ch_buf_drop(LOCAL_SCENARIO_2, length, hwsrc_enable);
        }
    }else{
        //length =/ ch_num, but stream is 32bit, if 16bit,so need << 1
        U32 silence_length = (length / dchs_dl_ch_scenario_msg[ch_type].channel_num) << (dchs_dl_ch_scenario_msg[ch_type].format_bytes == 2);
        if(!dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_2].is_running){
            dchs_dl_silence_count[ch_type] += length;
            DCHS_DL_LOG_W("[Source Read] source ch:%d, read %d silence, ch_data_size:%d", 3, ch_type, silence_length, ch_data_size);
        }
        memset(dst_addr_L, 0, silence_length);
        memset(dst_addr_R, 0, silence_length);
    }
}

void dchs_dl_stream_uart_timer_callback(void *user_data)
{
    DCHS_DL_LOG_I("[timer callback]Play en check, clk:0x%x, phase:0x%x, enable :0x%x, AFE_AUDIO_BT_SYNC_CON0:0x%x, AFE_AUDIO_BT_SYNC_MON1:0x%x,", 5,*((volatile uint32_t *)(CONN_BT_TIMCON_BASE + 0x0204)),*((volatile uint32_t*)(CONN_BT_TIMCON_BASE+0x0208)),*((volatile uint8_t*)(CONN_BT_TIMCON_BASE+0x0200)), AFE_READ(AFE_AUDIO_BT_SYNC_CON0), AFE_READ(AFE_AUDIO_BT_SYNC_MON1));// enable:1->0,play en work
    SOURCE source = (SOURCE)user_data;
    if(source && source->transform && source->transform->sink){
        if((dchs_dl_check_scenario_play_en_exist(HAL_AUDIO_AGENT_MEMORY_DL12) && !dchs_play_en_timeout_flag)){
            dchs_dl_resume_dchs_task();
            if(dchs_dl_timer_handle){
                hal_gpt_status_t gpt_status = hal_gpt_sw_start_timer_ms(dchs_dl_timer_handle, 1, dchs_dl_stream_uart_timer_callback, (void *)source);
                if(gpt_status != HAL_GPT_STATUS_OK){
                    DCHS_DL_LOG_E("timer start fail:%d",1,gpt_status);
                }else{
                    if(dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].sink_buf_info)
                        DCHS_DL_LOG_I("timer start, dchs dl sink wo=%d,ro=%d,scenario1 sink wo=%d,ro=%d",4,source->transform->sink->streamBuffer.BufferInfo.WriteOffset,source->transform->sink->streamBuffer.BufferInfo.ReadOffset,dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].sink_buf_info->WriteOffset,dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].sink_buf_info->ReadOffset);
                    else if(dchs_dl_ch_scenario_msg[UART_SCENARIO].sink_buf_info)
                        DCHS_DL_LOG_I("timer start, dchs dl sink wo=%d,ro=%d,uart sink wo=%d,ro=%d",4,source->transform->sink->streamBuffer.BufferInfo.WriteOffset,source->transform->sink->streamBuffer.BufferInfo.ReadOffset,dchs_dl_ch_scenario_msg[UART_SCENARIO].sink_buf_info->WriteOffset,dchs_dl_ch_scenario_msg[UART_SCENARIO].sink_buf_info->ReadOffset);
                    else if(dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_2].sink_buf_info)
                        DCHS_DL_LOG_I("timer start, dchs dl sink wo=%d,ro=%d,scenario2 sink wo=%d,ro=%d",4,source->transform->sink->streamBuffer.BufferInfo.WriteOffset,source->transform->sink->streamBuffer.BufferInfo.ReadOffset,dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_2].sink_buf_info->WriteOffset,dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_2].sink_buf_info->ReadOffset);
                    else
                        DCHS_DL_LOG_I("timer start, dchs dl sink wo=%d,ro=%d",2,source->transform->sink->streamBuffer.BufferInfo.WriteOffset,source->transform->sink->streamBuffer.BufferInfo.ReadOffset);
                }
            }
        }else{
            if(dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].sink_buf_info)
                DCHS_DL_LOG_I("timer stop, dchs dl sink wo=%d,ro=%d,scenario1 sink wo=%d,ro=%d",4,source->transform->sink->streamBuffer.BufferInfo.WriteOffset,source->transform->sink->streamBuffer.BufferInfo.ReadOffset,dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].sink_buf_info->WriteOffset,dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].sink_buf_info->ReadOffset);
            else if(dchs_dl_ch_scenario_msg[UART_SCENARIO].sink_buf_info)
                DCHS_DL_LOG_I("timer stop, dchs dl sink wo=%d,ro=%d,uart sink wo=%d,ro=%d",4,source->transform->sink->streamBuffer.BufferInfo.WriteOffset,source->transform->sink->streamBuffer.BufferInfo.ReadOffset,dchs_dl_ch_scenario_msg[UART_SCENARIO].sink_buf_info->WriteOffset,dchs_dl_ch_scenario_msg[UART_SCENARIO].sink_buf_info->ReadOffset);
            else if(dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_2].sink_buf_info)
                DCHS_DL_LOG_I("timer stop, dchs dl sink wo=%d,ro=%d,scenario2 sink wo=%d,ro=%d",4,source->transform->sink->streamBuffer.BufferInfo.WriteOffset,source->transform->sink->streamBuffer.BufferInfo.ReadOffset,dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_2].sink_buf_info->WriteOffset,dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_2].sink_buf_info->ReadOffset);
            else
                DCHS_DL_LOG_I("timer stop, dchs dl sink wo=%d,ro=%d",2,source->transform->sink->streamBuffer.BufferInfo.WriteOffset,source->transform->sink->streamBuffer.BufferInfo.ReadOffset);
            if(dchs_dl_timer_handle){
                hal_gpt_status_t gpt_status = hal_gpt_sw_free_timer(dchs_dl_timer_handle);
                if (HAL_GPT_STATUS_OK != gpt_status) {
                    DCHS_DL_LOG_E("dchs_dl_timer_handle fail, fail id = %d", 1, gpt_status);
                } else {
                    DCHS_DL_LOG_I("timer handle free finished", 0);
                    dchs_dl_timer_handle = 0;
                }
            }
        }
    } else {
        DCHS_DL_LOG_I("start no done, start timer still", 0);
        if(dchs_dl_timer_handle){
            hal_gpt_sw_start_timer_ms(dchs_dl_timer_handle, 1, dchs_dl_stream_uart_timer_callback, (void *)source);
        }
    }
}

void dchs_dl_set_timer_do_sw_mix_early(SOURCE source)
{
    hal_gpt_status_t gpt_status = hal_gpt_sw_get_timer(&dchs_dl_timer_handle);
    if(gpt_status != HAL_GPT_STATUS_OK){
        DCHS_DL_LOG_E("[Source] timer get fail:%d",1,gpt_status);
        AUDIO_ASSERT(0);
    }
    gpt_status = hal_gpt_sw_start_timer_ms(dchs_dl_timer_handle, 1, dchs_dl_stream_uart_timer_callback, (void *)source);
    if(gpt_status != HAL_GPT_STATUS_OK){
        DCHS_DL_LOG_E("[Source] timer start fail:%d",1,gpt_status);
    }else{
        dchs_dl_resume_dchs_task();//first resume task
        if(dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].sink_buf_info && source->transform && source->transform->sink)
            DCHS_DL_LOG_I("[Source] first timer start, dchs dl sink wo=%d,ro=%d,scenario1 sink wo=%d,ro=%d", 4, source->transform->sink->streamBuffer.BufferInfo.WriteOffset,source->transform->sink->streamBuffer.BufferInfo.ReadOffset,dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].sink_buf_info->WriteOffset,dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].sink_buf_info->ReadOffset);
    }
}

U32 dchs_get_cur_native_clk(void)
{
    return rBb->rClkCtl.rNativeClock & 0x0FFFFFFC;
}

U16 dchs_get_cur_native_phase(void)
{
    return rBb->rClkCtl.rNativePhase;
}

U32 dchs_get_duration_clk(BTTIME_STRU_PTR pa, BTTIME_STRU_PTR pb)
{
    BTCLK a_t0 = pa->period & 0xFFFFFFC;
    BTCLK b_t0 = pb->period & 0xFFFFFFC;
    BTCLK CLKOffset;
    U32 Phase;
    if (pa->period <= pb->period) {
        CLKOffset = (b_t0 - a_t0);
    } else {
        CLKOffset = (0xFFFFFFF - a_t0 + b_t0 + 1);
    }
    Phase = (CLKOffset * 625) - pa->phase + pb->phase;
    return (Phase >> 1);
}

void dchs_dl_count_mix_point(U32 play_en_clk, U16 play_en_phase, audio_scenario_type_t data_scenario_type, buf_ch_type_t ch_type)
{
    UNUSED(data_scenario_type);
    //count mix point
    BTTIME_STRU cur_time,play_en_time;
    cur_time.period = dchs_get_cur_native_clk();
    cur_time.phase  = dchs_get_cur_native_phase();
    play_en_time.period = play_en_clk;
    play_en_time.phase  = play_en_phase;
    U32 set_time = dchs_get_duration_clk(&cur_time, &play_en_time);
    U32 count_dur_point = set_time / 1000 / AUDIO_DURATION_TIME + 1;
    dchs_dl_ch_scenario_msg[ch_type].mix_point = count_dur_point + g_dchs_dl_data_mix_count;
    DCHS_DL_LOG_I("count dur_point = %d, mix_count= %d, mix_point=%d, scenario_type=%d,ch_type=%d", 5, count_dur_point, g_dchs_dl_data_mix_count, dchs_dl_ch_scenario_msg[ch_type].mix_point, data_scenario_type,ch_type);
}

void dchs_dl_uart_relay_play_en_info(U32 play_en_clk, U16 play_en_phase, audio_scenario_type_t data_scenario_type)
{
    #if 1 //test code for play en
    hal_gpio_init(HAL_GPIO_1);
    hal_pinmux_set_function(HAL_GPIO_1, HAL_GPIO_1_AUDIO_EXT_SYNC_EN);
    AFE_SET_REG(0xA0000200, 1 << 16, 0x00010000);
    DCHS_DL_LOG_I("0xA0000200 = 0x%x",1,*((volatile uint32_t *)(0xA0000200)));
    #endif
    audio_dchs_dsp2dsp_cmd_t dchs_dsp2dsp_cmd;
    dchs_dsp2dsp_cmd.cmd_param.dchs_dl_param.data_scenario_type = data_scenario_type;
    dchs_dsp2dsp_cmd.cmd_param.dchs_dl_param.play_en_clk    = play_en_clk;
    dchs_dsp2dsp_cmd.cmd_param.dchs_dl_param.play_en_phase  = play_en_phase;
    dchs_dsp2dsp_cmd.header.cmd_type = AUDIO_DCHS_DL_PLAY_EN_INFO;
    dchs_dsp2dsp_cmd.header.param_size = sizeof(audio_dchs_dsp2dsp_cmd_t) - sizeof(uart_cmd_header_t);
    DCHS_DL_LOG_I("[tx cmd] relay Play en clk:0x%x(%d),phase:0x%x(%d)", 4, play_en_clk, play_en_clk, play_en_phase, play_en_phase);
    //uart relay play en info to other chip
    dsp_uart_tx(UART_CMD, (U8 *)&dchs_dsp2dsp_cmd, sizeof(audio_dchs_dsp2dsp_cmd_t));
    //set timer
    DSP_MW_LOG_I("[audio_transmitter][DCHS] set play en check timer",0);
    SOURCE source = Source_blks[SOURCE_TYPE_UART];
    dchs_dl_set_timer_do_sw_mix_early(source); //set timer do thing early before agent trigger up
}

void dchs_dl_set_hfp_play_en(void)
{
    U32 play_en_bt_clk, play_en_nclk;
    U16 play_en_intra_clk;
    play_en_bt_clk = Forwarder_Rx_AncClk() + (DCHS_DL_HFP_DELAY_TIME / 0.3125); //4 Rx anchor time later
    MCE_TransBT2NativeClk(play_en_bt_clk, 1250, &play_en_nclk, &play_en_intra_clk, BT_CLK_Offset);// - phone offset
    hal_audio_afe_set_play_en(play_en_nclk, (uint32_t)play_en_intra_clk);//set play en
    DCHS_DL_LOG_I("Play en 4 anchor clk:0x%08x(%u), play_en_nclk:0x%08x(%u), play_en_nphase:0x%08x(%u),cur native clk=%u,dchs clk offset=0x%08x(%d)", 9, play_en_bt_clk,play_en_bt_clk, play_en_nclk, play_en_nclk,play_en_intra_clk,play_en_intra_clk,dchs_get_cur_native_clk(),*((volatile uint32_t *)(0xA0010974)),*((volatile int32_t *)(0xA0010974)));
    DCHS_TransBT2NativeClk(play_en_nclk, play_en_intra_clk, &play_en_nclk, &play_en_intra_clk, DCHS_CLK_Offset);// + dchs offset
    dchs_dl_uart_relay_play_en_info(play_en_nclk, play_en_intra_clk, AUDIO_SCENARIO_TYPE_HFP_DL);
    DCHS_DL_LOG_I("Play en check, :0x%x, enable :0x%x, BT_SYNC_CON0:0x%x, BT_SYNC_MON1:0x%x,", 4,*((volatile uint32_t*)(CONN_BT_TIMCON_BASE+0x0208)),*((volatile uint8_t*)(CONN_BT_TIMCON_BASE+0x0200)), AFE_READ(AFE_AUDIO_BT_SYNC_CON0), AFE_READ(AFE_AUDIO_BT_SYNC_MON1));// enable:1->0,play en work
    dchs_dl_set_scenario_play_en_exist(HAL_AUDIO_AGENT_MEMORY_DL1, true);
}


void DCHS_TransBT2NativeClk(BTCLK CurrCLK, BTPHASE CurrPhase, BTCLK *pNativeBTCLK, BTPHASE *pNativePhase, BT_CLOCK_OFFSET_SCENARIO type)
{
    BTPHASE    PhaseOffset;
    BTCLK      ClockOffset;

    MCE_Get_BtClkOffset(&ClockOffset, &PhaseOffset, type);

    *pNativeBTCLK = (CurrCLK + ClockOffset);
    *pNativePhase = (CurrPhase + PhaseOffset);

    *pNativeBTCLK -= 4;
    *pNativePhase += 2500;
    if(*pNativePhase >= 2500)
    {
        *pNativeBTCLK += 4 * (*pNativePhase / 2500);
        *pNativePhase %= 2500;
    }
    *pNativeBTCLK &= BT_NCLK_MASK;
}

void dchs_dl_set_play_en(U32 play_en_clk, U16 play_en_phase, audio_scenario_type_t data_scenario_type)
{
    #if 1 //test code for play en
    hal_gpio_init(HAL_GPIO_1);
    hal_pinmux_set_function(HAL_GPIO_1, HAL_GPIO_1_AUDIO_EXT_SYNC_EN);
    AFE_SET_REG(0xA0000200, 1 << 16, 0x00010000);
    DCHS_DL_LOG_I("0xA0000200 = 0x%x",1,*((volatile uint32_t *)(0xA0000200)));
    #endif
    U32 native_play_clk = 0;
    U16 native_play_phase = 0;
    U32 cur_native_clk = dchs_get_cur_native_clk();
    MCE_TransBT2NativeClk((BTCLK)play_en_clk, (BTPHASE)play_en_phase, (BTCLK *)&native_play_clk, (BTPHASE *)&native_play_phase, DCHS_CLK_Offset);
    DCHS_DL_LOG_I("Play en native play clk:0x%08x(%u), play phase:0x%08x(%u), cur native clk=0x%08x(%u), clk offset:0x%08x(%d),phase offset:0x%08x(%d), period ms:%d", 11, native_play_clk, native_play_clk,native_play_phase, native_play_phase,
                cur_native_clk,cur_native_clk, *((volatile uint32_t *)(0xA0010974)),*((volatile int32_t *)(0xA0010974)), *((volatile uint32_t *)(0xA0010978)),*((volatile int32_t *)(0xA0010978)), (native_play_clk-cur_native_clk)*0.3125);
    if (native_play_clk > cur_native_clk) { // check play time in time,0x10 = dchs dl max irq period 5s
        #if 0 //ull some time set play en long
        if(native_play_clk >= cur_native_clk + 0x880 ) //700ms as too long
        {
            AUDIO_ASSERT(0 && "[DCHS DL]Play en not legal, is too long");
        }
        #endif
        volatile SINK sink = Sink_blks[SINK_TYPE_AUDIO_DL12];
        if(!sink || sink->param.audio.irq_exist == false){ //dchs dl is not running,set play en
            #ifdef AIR_AUDIO_HARDWARE_ENABLE
            hal_audio_afe_set_play_en(native_play_clk, native_play_phase);
            DCHS_DL_LOG_I("set Play en success, scenario type=%d", 1, data_scenario_type);
            #endif
            if(!dchs_dl_timer_handle){
                SOURCE source = Source_blks[SOURCE_TYPE_UART];
                dchs_dl_set_timer_do_sw_mix_early(source); //set timer do thing early before agent trigger up   
            }
        }
        //mix count
        if(data_scenario_type != AUDIO_SCENARIO_TYPE_VP){
            buf_ch_type_t ch_type = UART_SCENARIO;
            if(dchs_get_device_mode() == DCHS_MODE_RIGHT && (data_scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_0 || data_scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_IN)){
                ch_type = LOCAL_SCENARIO_1;
            }
            dchs_dl_count_mix_point(native_play_clk, native_play_phase, data_scenario_type, ch_type);
        }
    }else{
        AUDIO_ASSERT(0 && "[DCHS DL]Play en not legal, is too short");
    }
}

void dchs_dl_play_en_disable(hal_audio_agent_t agent)
{
    hal_memory_set_enable(agent, true);
    AFE_SET_REG(AFE_AUDIO_BT_SYNC_CON0, 0x0003, 0x0003);
    hal_gpt_delay_ms(1);//Set this bit as high at least 1ms and then set to zero to simulate the pulse width.
    hal_memory_set_palyen(agent, false);
    AFE_SET_REG(AFE_AUDIO_BT_SYNC_CON0, 0x0000, 0x0003);
    DCHS_DL_LOG_I("[DCHS]][DL] disable Play en!,agent=%d", 1, agent);
}

void dchs_send_unlock_sleep_msg(bool is_dchs_dl)
{
    //send msg to mcu for unlock bt sleep
    hal_ccni_message_t msg;
    memset((void *)&msg, 0, sizeof(hal_ccni_message_t));
    if(is_dchs_dl){
        msg.ccni_message[0] = (MSG_DSP2MCU_DCHS_COSYS_SYNC_DL << 16);
    }else{
        msg.ccni_message[0] = (MSG_DSP2MCU_DCHS_COSYS_SYNC_UL << 16);
    }
    U32 try_times = 0;
    while (aud_msg_tx_handler(msg, 0, TRUE) != AUDIO_MSG_STATUS_OK && try_times <= 100) {
        try_times ++;
    }
    if(try_times < 100){
        DCHS_DL_LOG_I("send msg [0x%x] for unlock bt sleep,is dl:%d", 2, msg.ccni_message[0],is_dchs_dl);
    }else{
        DCHS_DL_LOG_E("send msg [0x%x] for unlock bt sleep fail, is dl:%d", 2, msg.ccni_message[0],is_dchs_dl);
    }
}

void dchs_dl_set_play_en_ms(U32 later_time_ms, audio_scenario_type_t data_scenario_type)
{
    U32 cur_native_clk   = dchs_get_cur_native_clk();
    U16 cur_native_phase = dchs_get_cur_native_phase();
    U32 native_play_clk  = cur_native_clk + later_time_ms/0.3125;
    DCHS_DL_LOG_I("count Play en, native_play_clk=%u, cur_native_phase=%u, cur native clk=%u,scenario_type=%d", 4, native_play_clk,cur_native_phase,cur_native_clk,data_scenario_type);
    dchs_dl_uart_relay_play_en_info(native_play_clk, cur_native_phase, data_scenario_type);
    dchs_dl_set_play_en(native_play_clk, cur_native_phase, data_scenario_type);
}

void dchs_dl_uart_buf_clear()
{
    U32 uart_buf_data_size = dsp_query_uart_rx_buf_remain_size(UART_DL);
    DCHS_DL_LOG_I("enter clear uart bug, size:%d", 1, uart_buf_data_size);
    if(uart_buf_data_size){
        mux_handle_t uart_handle = dsp_get_uart_handle(UART_DL);
        mux_clear_ll_user_buffer(uart_handle, true);
    }
}

void dchs_mcu2dsp_msg_sync_callback(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    uint32_t mask;
    dsp_uart_open(UART_DL);
    dsp_uart_open(UART_CMD);
    dchs_mode_t chip_role = dchs_get_device_mode();
    SOURCE source = Source_blks[SOURCE_TYPE_UART];
    audio_dchs_mcu2dsp_cosys_sync_t * sync_msg = (audio_dchs_mcu2dsp_cosys_sync_t *)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    audio_scenario_type_t data_scenario_type = sync_msg->sync_msg.dual_chip_dl_sync.data_scenario_type;
    DCHS_DL_LOG_I("[DSP msg sync] rx ccni msg, sync_type:%d", 1, sync_msg->sync_type);
    if(sync_msg->sync_type == DCHS_DL_OTHER_CHIP_DL_EXIST){
        bool other_chip_dl_exist = sync_msg->sync_msg.dual_chip_dl_sync.other_chip_dl_exist;
        if(!other_chip_dl_exist){
            hal_nvic_save_and_set_interrupt_mask(&mask);
            dchs_dl_set_scenario_exist(UART_SCENARIO, false);
            dchs_dl_ch_scenario_msg[UART_SCENARIO].mix_type = STOP_MIX;
            hal_nvic_restore_interrupt_mask(mask);
        }
        dchs_dl_ch_scenario_msg[UART_SCENARIO].data_scenario_type = data_scenario_type;
        if(chip_role == DCHS_MODE_RIGHT){
            dchs_dl_ch_scenario_msg[UART_SCENARIO].channel_num        = 2;
            dchs_dl_ch_scenario_msg[UART_SCENARIO].format_bytes       = sync_msg->sync_msg.dual_chip_dl_sync.format_bytes;
            dchs_dl_ch_scenario_msg[UART_SCENARIO].sample_rate        = sync_msg->sync_msg.dual_chip_dl_sync.sample_rate;
            dchs_dl_ch_scenario_msg[UART_SCENARIO].frame_size         = sync_msg->sync_msg.dual_chip_dl_sync.frame_size;
            if(data_scenario_type == AUDIO_SCENARIO_TYPE_HFP_DL || data_scenario_type == AUDIO_SCENARIO_TYPE_BLE_DL){
                dchs_dl_ch_scenario_msg[UART_SCENARIO].format_bytes = 4;//uart send side 16bit->32bit
            }
            if(other_chip_dl_exist){//reset right uart buf
                dchs_dl_ch_scenario_msg[UART_SCENARIO].sink_buf_info->WriteOffset += dchs_dl_ch_scenario_msg[UART_SCENARIO].prefill_size;
                dchs_dl_uart_buf_clear();
            }
            if(sync_msg->sync_msg.dual_chip_dl_sync.sample_rate != source->param.audio.rate){
                dchs_dl_source_ch_hwsrc_cfg(UART_SCENARIO, HWSRC_1);
                if(source){
                    dchs_dl_source_ch_hwsrc_driver_control(UART_SCENARIO, other_chip_dl_exist);
                }
            }
        }else if(chip_role == DCHS_MODE_LEFT){
            dchs_dl_ch_scenario_msg[UART_SCENARIO].channel_num  = 1;
            dchs_dl_ch_scenario_msg[UART_SCENARIO].format_bytes = 4;
            dchs_dl_ch_scenario_msg[UART_SCENARIO].sample_rate  = DCHS_DL_FIX_SAMPLE_RATE;
            dchs_dl_ch_scenario_msg[UART_SCENARIO].frame_size   = AUDIO_DURATION_TIME * DCHS_DL_FIX_SAMPLE_RATE / MUX_UART_BUF_SLICE / 1000;
            //uart tx sink buf prefill
            if(sync_msg->sync_msg.dual_chip_dl_sync.chip_role == LOCAL_CHIP && (data_scenario_type == AUDIO_SCENARIO_TYPE_HFP_DL || data_scenario_type == AUDIO_SCENARIO_TYPE_BLE_DL)){
                SINK sink = Sink_blks[SINK_TYPE_AUDIO];
                if(sink){
                    U32 prefill = sink->streamBuffer.BufferInfo.WriteOffset * (sync_msg->sync_msg.dual_chip_dl_sync.format_bytes == 2 ? 2 : 1);
                        audio_dchs_dsp2dsp_cmd_t dchs_dsp2dsp_cmd;
                        dchs_dsp2dsp_cmd.cmd_param.dchs_dl_param.data_scenario_type = data_scenario_type;
                        dchs_dsp2dsp_cmd.cmd_param.dchs_dl_param.prefill_size       = prefill;
                        dchs_dsp2dsp_cmd.header.cmd_type = AUDIO_DCHS_DL_UART_SCENARIO_PREFILL_SIZE;
                        dchs_dsp2dsp_cmd.header.param_size = sizeof(audio_dchs_dsp2dsp_cmd_t) - sizeof(uart_cmd_header_t);
                        dsp_uart_tx(UART_CMD, (U8 *)&dchs_dsp2dsp_cmd, sizeof(audio_dchs_dsp2dsp_cmd_t));
                    DCHS_DL_LOG_I("[DSP msg sync] get scenario type:%d, send prefill size:%d,format:%d", 3, data_scenario_type, prefill, sync_msg->sync_msg.dual_chip_dl_sync.format_bytes);
                }
            }
            if(other_chip_dl_exist){//reset left uart buf
                dchs_dl_uart_buf_clear();
            }
        }
        if(other_chip_dl_exist){
            dchs_dl_silence_count[UART_SCENARIO] = 0;
            if(g_dchs_dl_data_mix_count && chip_role == DCHS_MODE_RIGHT){
                dchs_dl_ch_scenario_msg[UART_SCENARIO].mix_type = WAITING_MIX;
            }else{
                dchs_dl_set_scenario_exist(UART_SCENARIO, true);
            }
            if(!dchs_dl_check_scenario_exist(LOCAL_SCENARIO_1)){
                dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].channel_num = 2;
            }
        }else{
            dchs_dl_ch_scenario_msg[UART_SCENARIO].prefill_size = 0;
            dchs_dl_ch_scenario_msg[UART_SCENARIO].mix_point = 0;
            dchs_dl_ch_scenario_msg[UART_SCENARIO].mix_type = STOP_MIX;
            dchs_dl_ch_scenario_msg[UART_SCENARIO].sink_buf_info->WriteOffset = 0;
            dchs_dl_ch_scenario_msg[UART_SCENARIO].sink_buf_info->ReadOffset  = 0;
        }
        DCHS_DL_LOG_I("[DSP msg sync] other_chip_dl_exist = %d,mix_type=%d,format bytes=%d,rate=%d,channel_num=%d,chip_role=%d", 6, dchs_dl_check_scenario_exist(UART_SCENARIO), dchs_dl_ch_scenario_msg[UART_SCENARIO].mix_type, dchs_dl_ch_scenario_msg[UART_SCENARIO].format_bytes,sync_msg->sync_msg.dual_chip_dl_sync.sample_rate,dchs_dl_ch_scenario_msg[UART_SCENARIO].channel_num
                    ,sync_msg->sync_msg.dual_chip_dl_sync.chip_role);
        if(other_chip_dl_exist){
            dchs_dl_resume_dchs_task();
        }
    }else if(sync_msg->sync_type == DCHS_DL_LOCAL_CHIP_DL_EXIST){
        bool local_chip_dl_exist = sync_msg->sync_msg.dual_chip_dl_sync.local_chip_dl_exist;
        U32 sample_rate = sync_msg->sync_msg.dual_chip_dl_sync.sample_rate;
        U8 format_bytes = sync_msg->sync_msg.dual_chip_dl_sync.format_bytes;
        U32 frame_size  = sync_msg->sync_msg.dual_chip_dl_sync.frame_size;
        if(local_chip_dl_exist && !dchs_dl_check_scenario_exist(LOCAL_SCENARIO_1)){
            dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].channel_num = 2;
        }
        if(data_scenario_type == AUDIO_SCENARIO_TYPE_VP){
            if(!local_chip_dl_exist){
                hal_nvic_save_and_set_interrupt_mask(&mask);
                dchs_dl_set_scenario_exist(LOCAL_SCENARIO_2, false);
                dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_2].mix_type = STOP_MIX;
                hal_nvic_restore_interrupt_mask(mask);
            }
            dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_2].data_scenario_type = data_scenario_type;
            dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_2].format_bytes       = format_bytes;
            dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_2].sample_rate        = sample_rate;
            if(local_chip_dl_exist){
                SINK sink = Sink_blks[SINK_TYPE_VP_AUDIO];
                if(sink){
                    dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_2].sink_buf_info = &(sink->streamBuffer.BufferInfo);
                    dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_2].channel_num   = sink->param.audio.channel_num;
                    DCHS_DL_LOG_I("[DSP msg sync] get local scenario 2 sink buf info=0x%x, buf addr=0x%x",2,dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_2].sink_buf_info,dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_2].sink_buf_info->startaddr[0]);
                }else{
                    dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_2].sink_buf_info = NULL;
                    DCHS_DL_LOG_W("[DCHS DL][DSP msg sync] get scenario 2 sink buf info fail first", 0);
                }
            }else{
                //memset(&dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_2], 0, sizeof(dl_data_scenario_msg_t));
            }
            dchs_dl_source_ch_hwsrc_cfg(LOCAL_SCENARIO_2, HWSRC_2);
            if(source){
                dchs_dl_source_ch_hwsrc_driver_control(LOCAL_SCENARIO_2, local_chip_dl_exist);
            }
            if(local_chip_dl_exist){
                dchs_dl_set_scenario_exist(LOCAL_SCENARIO_2, true);
                if(!dchs_dl_check_scenario_exist(LOCAL_SCENARIO_1) && dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].mix_type == STOP_MIX){
                    dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].channel_num = 0;
                }
            }
            dchs_dl_silence_count[LOCAL_SCENARIO_2] = 0;
        }else{
            if(!local_chip_dl_exist){
                hal_nvic_save_and_set_interrupt_mask(&mask);
                dchs_dl_set_scenario_exist(LOCAL_SCENARIO_1, false);
                dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].mix_type  = STOP_MIX;
                hal_nvic_restore_interrupt_mask(mask);
            }
            dchs_dl_silence_count[LOCAL_SCENARIO_1] = 0;
            dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].data_scenario_type = data_scenario_type;
            dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].format_bytes       = format_bytes;
            dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].sample_rate        = sample_rate;
            dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].frame_size         = frame_size;
            if(local_chip_dl_exist){
                SINK sink = NULL;
                if(data_scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_IN){
                    sink = Sink_blks[SINK_TYPE_AUDIO_DL3];
                }else if(data_scenario_type == AUDIO_SCENARIO_TYPE_BLE_DL){
                    sink = Sink_blks[SINK_TYPE_AUDIO];
                }
                if(sink){
                    dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].sink_buf_info = &(sink->streamBuffer.BufferInfo);
                    dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].channel_num   = sink->param.audio.channel_num;
                    DCHS_DL_LOG_I("[DSP msg sync] get local scenario 1 sink buf info=0x%x, buf addr=0x%x, prefill=%d,buf length=%d,channel_num=%d", 5, dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].sink_buf_info,dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].sink_buf_info->startaddr[0],dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].sink_buf_info->WriteOffset,dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].sink_buf_info->length, dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].channel_num);
                }else{
                    dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].sink_buf_info = NULL;
                }
                if(data_scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_0){
                    dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].channel_num = sync_msg->sync_msg.dual_chip_dl_sync.channel_num;
                }
                if(g_dchs_dl_data_mix_count && data_scenario_type == AUDIO_SCENARIO_TYPE_BLE_DL){
                    dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].mix_type = WAITING_MIX;
                }else{
                    dchs_dl_set_scenario_exist(LOCAL_SCENARIO_1, true);
                    dchs_dl_resume_dchs_task();
                }
                if(data_scenario_type != AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_0 && sample_rate != source->param.audio.rate && !dchs_dl_check_hwsrc_enable(UART_SCENARIO)){
                    dchs_dl_source_ch_hwsrc_cfg(LOCAL_SCENARIO_1, HWSRC_1);
                    dchs_dl_source_ch_hwsrc_driver_control(LOCAL_SCENARIO_1, true);
                }
            }else{
                dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].data_scenario_type = AUDIO_SCENARIO_TYPE_COMMON;
                dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].mix_point = 0;
                dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].mix_type  = STOP_MIX;
                if(dchs_dl_check_hwsrc_enable(LOCAL_SCENARIO_1)){
                    dchs_dl_source_ch_hwsrc_driver_control(LOCAL_SCENARIO_1, local_chip_dl_exist);
                }
            }
        }
        DCHS_DL_LOG_I("[DSP msg sync] local_chip_dl_exist=%d, data scenario=%d, sample_rate=%d, format bytes=%d, scenario 1 ch num=%d,scenario 2 ch num=%d", 6,
                    local_chip_dl_exist,
                    data_scenario_type,
                    sample_rate,
                    format_bytes,
                    dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].channel_num,
                    dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_2].channel_num);
        dchs_dl_resume_dchs_task();
    }else if(sync_msg->sync_type == DCHS_DL_SET_GAIN_VALUE){
        int32_t  vol_gain = sync_msg->sync_msg.dual_chip_dl_sync.vol_gain;
        dchs_dl_runtime_config_operation_t operation = (dchs_dl_runtime_config_operation_t)sync_msg->sync_msg.dual_chip_dl_sync.operation;
        if(operation >= DCHS_DL_CONFIG_OP_MAX || operation < DCHS_DL_CONFIG_OP_SET_UART_SCENARIO_VOL_INFO){
            return;
        }
        if(dchs_gain_port){
            sw_gain_config_t old_config;
            int32_t  vol_gain = sync_msg->sync_msg.dual_chip_dl_sync.vol_gain;
            dchs_dl_runtime_config_operation_t operation = (dchs_dl_runtime_config_operation_t)sync_msg->sync_msg.dual_chip_dl_sync.operation;
            stream_function_sw_gain_get_config(dchs_gain_port, operation * 2 + 1, &old_config);
            DCHS_DL_LOG_I("[Sw Gain][config] operation %d, set ch:%d and %d gain from 0x%x to 0x%x\r\n", 5,
                        operation,
                        operation * 2 + 1,
                        operation * 2 + 2,
                        old_config.target_gain,
                        vol_gain);
            stream_function_sw_gain_configure_gain_target(dchs_gain_port, operation * 2 + 1, vol_gain);
            stream_function_sw_gain_configure_gain_target(dchs_gain_port, operation * 2 + 2, vol_gain);
        }else{
            buf_ch_type_t ch_type = ((operation == DCHS_DL_CONFIG_OP_SET_UART_SCENARIO_VOL_INFO) ? UART_SCENARIO : (operation - 1));
            dchs_dl_ch_scenario_msg[ch_type].waiting_set_volume = true;
            dchs_dl_ch_scenario_msg[ch_type].vol_gain = vol_gain;
            DCHS_DL_LOG_I("sw gain don't init done, will set gain later, operation:%d,gain:0x%x", 2, operation, vol_gain);
        }
    }else if(sync_msg->sync_type == DCHS_DL_SET_PLAY_EN){
        volatile SINK sink = Sink_blks[SINK_TYPE_AUDIO_DL12];
        if(!sink || sink->param.audio.irq_exist == true){ //dchs dl is running,no need set play en
            DCHS_DL_LOG_I("dchs irq is running, no need set play en", 0);
            return;
        }
        audio_scenario_type_t data_scenario = sync_msg->sync_msg.dual_chip_dl_sync.data_scenario_type;
        dchs_dl_set_play_en_ms(DCHS_DL_PLAY_EN_DELAY_MS, data_scenario);
        DSP_MW_LOG_I("[DCHS DL] set Play en by scenario:%d", 1, data_scenario);
    }
    //extend here
    else{
        DCHS_DL_LOG_W("[DSP msg sync] invalid sync type : %d", 1, sync_msg->sync_type);
    }
}

bool dchs_dl_source_size_pre_process(SOURCE source)
{
    //check fix point
    if(!dchs_dl_check_scenario_exist(UART_SCENARIO) && dchs_dl_ch_scenario_msg[UART_SCENARIO].mix_type == WAITING_MIX){
        if(dchs_dl_ch_scenario_msg[UART_SCENARIO].mix_point){
            if(g_dchs_dl_data_mix_count >= dchs_dl_ch_scenario_msg[UART_SCENARIO].mix_point){
                dchs_dl_set_scenario_exist(UART_SCENARIO, true);
                dchs_dl_ch_scenario_msg[UART_SCENARIO].mix_type = STOP_MIX;
                if(!dchs_dl_ch_scenario_msg[UART_SCENARIO].sink_buf_info->WriteOffset && dchs_dl_ch_scenario_msg[UART_SCENARIO].prefill_size){
                    dchs_dl_ch_scenario_msg[UART_SCENARIO].sink_buf_info->WriteOffset += dchs_dl_ch_scenario_msg[UART_SCENARIO].prefill_size;
                }
            }
        }else{
            DCHS_DL_LOG_W("[Source Size]UART_SCENARIO waiting count mix point", 0);
        }
        DCHS_DL_LOG_I("[Source Size] UART_SCENARIO exist:%d, mix_count:%d, mix_point:%d,mix_type:%d", 4, dchs_dl_check_scenario_exist(UART_SCENARIO), g_dchs_dl_data_mix_count,dchs_dl_ch_scenario_msg[UART_SCENARIO].mix_point,dchs_dl_ch_scenario_msg[UART_SCENARIO].mix_type);
    }
    if(!dchs_dl_check_scenario_exist(LOCAL_SCENARIO_1) && dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].mix_type == WAITING_MIX){
        if(dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].mix_point){
            if(g_dchs_dl_data_mix_count >= dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].mix_point){
                dchs_dl_set_scenario_exist(LOCAL_SCENARIO_1, true);
                dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].mix_type = STOP_MIX;
            }
        }else{
            DCHS_DL_LOG_W("[Source Size]SCENARIO_1 waiting count mix point", 0);
        }
        DCHS_DL_LOG_I("[Source Size] SCENARIO_1 exist:%d, mix_count:%d, mix_point:%d,mix_type:%d", 4, dchs_dl_check_scenario_exist(LOCAL_SCENARIO_1), g_dchs_dl_data_mix_count,dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].mix_point,dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].mix_type);
    }
    if(dchs_get_device_mode() == DCHS_MODE_RIGHT){
        TRANSFORM transform =  source->transform;
        volatile DSP_CALLBACK_PTR callback_ptr = DSP_Callback_Get(source, transform->sink);
        callback_ptr->EntryPara.out_channel_num = 6; // 6 ch mono data
        if(dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].channel_num == USB_IN_8_CHANNEL){
            callback_ptr->EntryPara.out_channel_num = 10; // 10 ch mono data: 8ch for usb,2ch for vp
        }
        //DCHS_DL_LOG_D("[source size]out_channel_num=%d",1,callback_ptr->EntryPara.out_channel_num);
    }
    if(dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_2].sink_buf_info == NULL && dchs_dl_check_scenario_exist(LOCAL_SCENARIO_2)){
        SINK sink = Sink_blks[SINK_TYPE_VP_AUDIO];
        if(sink){
            dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_2].sink_buf_info = &(sink->streamBuffer.BufferInfo);
            dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_2].channel_num   = sink->param.audio.channel_num;
            DCHS_DL_LOG_I("[source size] get local scenario 2 sink buf info=0x%x, buf addr=0x%x",2,dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_2].sink_buf_info,dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_2].sink_buf_info->startaddr[0]);
            if(dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_2].sample_rate != DCHS_DL_FIX_SAMPLE_RATE && !dchs_dl_check_hwsrc_enable(LOCAL_SCENARIO_2)){
                dchs_dl_source_ch_hwsrc_cfg(LOCAL_SCENARIO_2, HWSRC_1);
                dchs_dl_source_ch_hwsrc_driver_control(LOCAL_SCENARIO_2, true);
            }
        }else{
            DCHS_DL_LOG_W("[source size] waiting get local scenario 2 sink buf info", 0);
            return false;
        }  
    }
    if(dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].sink_buf_info == NULL && dchs_dl_check_scenario_exist(LOCAL_SCENARIO_1)){
        SINK sink = NULL;
        if(dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].data_scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_IN){
            sink = Sink_blks[SINK_TYPE_AUDIO_DL3];
        }else if(dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].data_scenario_type == AUDIO_SCENARIO_TYPE_HFP_DL){
            sink = Sink_blks[SINK_TYPE_AUDIO];
        }
        if(sink){
            dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].sink_buf_info = &(sink->streamBuffer.BufferInfo);
            dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].channel_num   = sink->param.audio.channel_num;
            DCHS_DL_LOG_I("[Source Size] get local scenario 1 sink buf info=0x%x, buf addr=0x%x, prefill=%d,buf length=%d,channel_num=%d", 5, dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].sink_buf_info,dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].sink_buf_info->startaddr[0],dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].sink_buf_info->WriteOffset,dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].sink_buf_info->length, dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].channel_num);
        }else{
            DCHS_DL_LOG_W("[source size] waiting get local scenario 1 sink buf info", 0);
            return false;
        } 
    }
    if(!g_dchs_dl_process_count){//no need read
        DCHS_DL_LOG_D("[Source Size] already process, this time no need process", 0);
        return false;
    }
    return true;
}

bool dchs_dl_check_fill_silence(void)
{
    volatile SINK dchs_dl_sink = Sink_blks[SINK_TYPE_AUDIO_DL12];
    U32 dchs_sink_threshold = (dchs_get_device_mode() == DCHS_MODE_RIGHT ? DCHS_DL_SINK_THRESHOLD_MASTER : DCHS_DL_SINK_THRESHOLD_SLAVE) * (dchs_dl_sink->param.audio.rate / 1000) * dchs_dl_sink->param.audio.format_bytes * dchs_dl_sink->param.audio.channel_num;
    U32 dchs_sink_size = dchs_dl_get_ch_data_size(DCHS_DL_SINK, false);
    if(dchs_sink_size <= dchs_sink_threshold)
    {
        DCHS_DL_LOG_W("[Source Size]sink under threshold, sink size:%d,threshold:%d", 2, dchs_sink_size, dchs_sink_threshold);
        return true;
    }
    return false;
}

U32 SourceSize_DCHS_DL(SOURCE source)
{ 
    U32 audio_frame_size = 0;
    if(!dchs_dl_source_size_pre_process(source)){
        return 0;
    }
    if(dchs_dl_check_scenario_exist(LOCAL_SCENARIO_1) && dchs_dl_check_scenario_exist(UART_SCENARIO)){
        U32 uart_frame_size  = source->param.audio.frame_size * dchs_dl_ch_scenario_msg[UART_SCENARIO].format_bytes * dchs_dl_ch_scenario_msg[UART_SCENARIO].channel_num;
        U32 local_frame_size = source->param.audio.frame_size * dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].format_bytes * dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].channel_num;
        audio_frame_size = MIN(uart_frame_size, local_frame_size);
        U32 local_scenario_size = dchs_dl_get_ch_data_size(LOCAL_SCENARIO_1, dchs_dl_check_hwsrc_enable(LOCAL_SCENARIO_1));
        U32 uart_scenario_size  = dchs_dl_get_ch_data_size(UART_SCENARIO, dchs_dl_check_hwsrc_enable(UART_SCENARIO));
        DCHS_DL_LOG_D("[Source Size][mix] local_scenario_size=%d, uart_scenario_size=%d, uart buf full=%d, frame data=%d", 4, local_scenario_size, uart_scenario_size, source->streamBuffer.BufferInfo.bBufferIsFull, audio_frame_size);
        if(local_scenario_size >= audio_frame_size && uart_scenario_size >= audio_frame_size){//two size meet the stream frame size,then read the buf
            return audio_frame_size;
        }else if(dchs_dl_check_fill_silence()){
            return audio_frame_size;
        }
    }else if(dchs_dl_check_scenario_exist(LOCAL_SCENARIO_1)){
        U32 local_scenario_1_size = dchs_dl_get_ch_data_size(LOCAL_SCENARIO_1, dchs_dl_check_hwsrc_enable(LOCAL_SCENARIO_1));
        audio_frame_size = source->param.audio.frame_size * dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].format_bytes * (dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].channel_num >= 2 ? 2 : 1);
        DCHS_DL_LOG_D("[Source Size][local] local_scenario_1_size=%d, local buf full=%d, frame data=%d,frame size:%d,channel_num=%d", 5, local_scenario_1_size, dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].sink_buf_info->bBufferIsFull, audio_frame_size, source->param.audio.frame_size,dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].channel_num);
        if(local_scenario_1_size >= audio_frame_size){
            return audio_frame_size;
        }else if(dchs_dl_check_fill_silence()){
            return audio_frame_size;
        }
    }else if(dchs_dl_check_scenario_exist(UART_SCENARIO)){
        audio_frame_size = source->param.audio.frame_size * dchs_dl_ch_scenario_msg[UART_SCENARIO].format_bytes * dchs_dl_ch_scenario_msg[UART_SCENARIO].channel_num;
        U32 uart_scenario_size  = dchs_dl_get_ch_data_size(UART_SCENARIO, dchs_dl_check_hwsrc_enable(UART_SCENARIO));
        DCHS_DL_LOG_D("[Source Size][uart] uart_scenario_size=%d, uart buf full=%d, frame data=%d", 3, uart_scenario_size,  dchs_dl_ch_scenario_msg[UART_SCENARIO].sink_buf_info->bBufferIsFull, audio_frame_size);
        if(uart_scenario_size >= audio_frame_size){
            return audio_frame_size;
        }else if(dchs_dl_check_fill_silence()){
            return audio_frame_size;
        }
    }
    if(dchs_dl_check_scenario_exist(LOCAL_SCENARIO_2)){
        U32 local_scenario_2_size = dchs_dl_get_ch_data_size(LOCAL_SCENARIO_2, dchs_dl_check_hwsrc_enable(LOCAL_SCENARIO_2));
        audio_frame_size = source->param.audio.frame_size * dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_2].format_bytes * dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_2].channel_num;
        DCHS_DL_LOG_D("[Source Size][local] local_scenario_2_size=%d, frame data=%d, buf full=%d, source frame size=%d, channel_num=%d", 5, local_scenario_2_size, audio_frame_size, dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_2].sink_buf_info->bBufferIsFull, source->param.audio.frame_size * source->param.audio.format_bytes, dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_2].channel_num);
        if(local_scenario_2_size >= audio_frame_size){
            dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_2].is_running = true;
            return audio_frame_size;
        }
    }
    if(g_dchs_dl_data_mix_count && !dchs_dl_check_scenario_exist(LOCAL_SCENARIO_1) && !dchs_dl_check_scenario_exist(LOCAL_SCENARIO_2) && !dchs_dl_check_scenario_exist(UART_SCENARIO)){
        if(source && source->transform && source->transform->sink){
            U32 audio_frame_size = source->param.audio.frame_size * source->param.audio.format_bytes * source->transform->sink->param.audio.channel_num;
            DCHS_DL_LOG_D("[Source Size] enter no scenario runing case, frame size=%d", 1, audio_frame_size);
            return audio_frame_size;
        }
    }
    DCHS_DL_LOG_D("[Source Size] return 0", 0);
    return 0;
}

BOOL SourceReadBuf_DCHS_DL(SOURCE source, U8 *dst_addr, U32 length)
{
    UNUSED(dst_addr);
    U32 stream_frame_size = source->param.audio.frame_size * source->param.audio.format_bytes;
    TRANSFORM transform =  source->transform;
    volatile DSP_CALLBACK_PTR callback_ptr = DSP_Callback_Get(source, transform->sink);
    U8 * uart_scenario_L = NULL; U8 * uart_scenario_R = NULL; U8 * local_scenario_1_L = NULL; U8 * local_scenario_1_R = NULL; U8 * local_scenario_2_L = NULL; U8 * local_scenario_2_R = NULL;
    dchs_mode_t mode = dchs_get_device_mode();
    //DCHS_DL_LOG_I("TEST source device mode %d", 1, mode);
    if(mode == DCHS_MODE_RIGHT){
        uart_scenario_L  = callback_ptr->EntryPara.in_ptr[0];
        uart_scenario_R  = callback_ptr->EntryPara.in_ptr[1];
        local_scenario_1_L = callback_ptr->EntryPara.in_ptr[2];
        local_scenario_1_R = callback_ptr->EntryPara.in_ptr[3];
        local_scenario_2_L = callback_ptr->EntryPara.in_ptr[4];
        local_scenario_2_R = callback_ptr->EntryPara.in_ptr[5];
        callback_ptr->EntryPara.out_channel_num = 6; // 6 ch mono data
        if(dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].channel_num == USB_IN_8_CHANNEL){
            callback_ptr->EntryPara.out_channel_num = 10; // 10 ch mono data: 8ch for usb,2ch for vp
            local_scenario_2_L = callback_ptr->EntryPara.in_ptr[8];
            local_scenario_2_R = callback_ptr->EntryPara.in_ptr[9];
        }
        DCHS_DL_LOG_D("[source read]out_channel_num=%d",1,callback_ptr->EntryPara.out_channel_num);
        //vp ch
        if(dchs_dl_check_scenario_exist(LOCAL_SCENARIO_2)){
            dchs_dl_copy_source_ch_data_2_stream_buf(LOCAL_SCENARIO_2, local_scenario_2_L, local_scenario_2_R, length, dchs_dl_check_hwsrc_enable(LOCAL_SCENARIO_2));
        }else{
            memset(local_scenario_2_L, 0, stream_frame_size);
            memset(local_scenario_2_R, 0, stream_frame_size);
        }
    }else{
        uart_scenario_L  = callback_ptr->EntryPara.in_ptr[0];
        uart_scenario_R  = callback_ptr->EntryPara.in_ptr[1];
    }
    if(dchs_dl_check_scenario_exist(LOCAL_SCENARIO_1) && dchs_dl_check_scenario_exist(UART_SCENARIO)){
        // local ch buf
        dchs_dl_copy_source_ch_data_2_stream_buf(LOCAL_SCENARIO_1, local_scenario_1_L, local_scenario_1_R, length * dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].channel_num, dchs_dl_check_hwsrc_enable(LOCAL_SCENARIO_1));
        // uart ch buf
        dchs_dl_copy_source_ch_data_2_stream_buf(UART_SCENARIO, uart_scenario_L, uart_scenario_R ,length * dchs_dl_ch_scenario_msg[UART_SCENARIO].channel_num, dchs_dl_check_hwsrc_enable(UART_SCENARIO));
    }else if(dchs_dl_check_scenario_exist(LOCAL_SCENARIO_1)){
        // local ch buf
        dchs_dl_copy_source_ch_data_2_stream_buf(LOCAL_SCENARIO_1, local_scenario_1_L, local_scenario_1_R, length * ((dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].channel_num >= 2 && dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].format_bytes == 4) ? 2 : 1), dchs_dl_check_hwsrc_enable(LOCAL_SCENARIO_1));
        // uart ch copy silence
        if(dchs_get_device_mode() == DCHS_MODE_RIGHT && dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].channel_num <= 2){
            memset(uart_scenario_L, 0, stream_frame_size);
            memset(uart_scenario_R, 0, stream_frame_size);
        }
    }else if(dchs_dl_check_scenario_exist(UART_SCENARIO)){
        // uart ch buf
        dchs_dl_copy_source_ch_data_2_stream_buf(UART_SCENARIO, uart_scenario_L, uart_scenario_R, length * dchs_dl_ch_scenario_msg[UART_SCENARIO].channel_num, dchs_dl_check_hwsrc_enable(UART_SCENARIO));
        if(dchs_get_device_mode() == DCHS_MODE_RIGHT){
            memset(local_scenario_1_L, 0, stream_frame_size);
            memset(local_scenario_1_R, 0, stream_frame_size);
        }
    }else if(dchs_get_device_mode() == DCHS_MODE_RIGHT){
        memset(uart_scenario_L,  0, stream_frame_size);
        memset(uart_scenario_R,  0, stream_frame_size);
        memset(local_scenario_1_L, 0, stream_frame_size);
        memset(local_scenario_1_R, 0, stream_frame_size);
        //DSP_MW_LOG_W("[DCHS DL][Source Read]read silence,size=%d",1,stream_frame_size);
    }else if(dchs_get_device_mode() == DCHS_MODE_LEFT){
        memset(uart_scenario_L,  0, stream_frame_size);
        memset(uart_scenario_R,  0, stream_frame_size);
    }
    LOG_AUDIO_DUMP((U8 *)uart_scenario_L, stream_frame_size, AUDIO_DCHS_DL_UART_SCENARIO_L);
    LOG_AUDIO_DUMP((U8 *)local_scenario_1_L, stream_frame_size, AUDIO_DCHS_DL_LOCAL_SCENARIO_1_L);
    LOG_AUDIO_DUMP((U8 *)local_scenario_2_L, stream_frame_size, AUDIO_DCHS_DL_LOCAL_SCENARIO_2_L);
    LOG_AUDIO_DUMP((U8 *)uart_scenario_R, stream_frame_size, AUDIO_DCHS_DL_UART_SCENARIO_R);
    LOG_AUDIO_DUMP((U8 *)local_scenario_1_R, stream_frame_size, AUDIO_DCHS_DL_LOCAL_SCENARIO_1_R);
    LOG_AUDIO_DUMP((U8 *)local_scenario_2_R, stream_frame_size, AUDIO_DCHS_DL_LOCAL_SCENARIO_2_R);
    //DCHS_DL_LOG_I("[source read] TEST out_channel_num=%d",1,callback_ptr->EntryPara.out_channel_num);
    return true;
}

bool dchs_dl_source_drop_pre_process(U32 amount)
{
    uint32_t mask = 0;
    hal_nvic_save_and_set_interrupt_mask(&mask);
    g_dchs_dl_process_count --;
    hal_nvic_restore_interrupt_mask(mask);
    if(g_dchs_dl_process_count >= 10){ //avoid too much log
        DCHS_DL_LOG_W("[Source Drop] resume task again, process count=%d", 1, g_dchs_dl_process_count);
    }
    if (amount == 0 || dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_2].is_running) {
        dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_2].is_running = false;
        return false;
    }
    return true;
}

VOID SourceDrop_DCHS_DL(SOURCE source, U32 amount)
{
    UNUSED(source);
    if(!dchs_dl_source_drop_pre_process(amount)){
        return;
    }
    if(dchs_dl_check_scenario_exist(LOCAL_SCENARIO_1) && dchs_dl_check_scenario_exist(UART_SCENARIO)){
        if(!dchs_dl_silence_count[LOCAL_SCENARIO_1]){
            dchs_dl_source_ch_buf_drop(LOCAL_SCENARIO_1, amount * dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].channel_num, dchs_dl_check_hwsrc_enable(LOCAL_SCENARIO_1));
        }
        if(!dchs_dl_silence_count[UART_SCENARIO]){
            dchs_dl_source_ch_buf_drop(UART_SCENARIO, amount * dchs_dl_ch_scenario_msg[UART_SCENARIO].channel_num, dchs_dl_check_hwsrc_enable(UART_SCENARIO));
        }
    }else if(dchs_dl_check_scenario_exist(LOCAL_SCENARIO_1)){
        if(!dchs_dl_silence_count[LOCAL_SCENARIO_1]){
            dchs_dl_source_ch_buf_drop(LOCAL_SCENARIO_1, amount * ((dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].channel_num >= 2 && dchs_dl_ch_scenario_msg[LOCAL_SCENARIO_1].format_bytes == 4)? 2 : 1), dchs_dl_check_hwsrc_enable(LOCAL_SCENARIO_1));
        }
    }else if(dchs_dl_check_scenario_exist(UART_SCENARIO)){
        if(!dchs_dl_silence_count[UART_SCENARIO]){
            dchs_dl_source_ch_buf_drop(UART_SCENARIO, amount * dchs_dl_ch_scenario_msg[UART_SCENARIO].channel_num, dchs_dl_check_hwsrc_enable(UART_SCENARIO));
        }
    }else{
        return;
    }
}

void dchs_dl_disable_play_en(void)
{
    *((volatile uint32_t *)(CONN_BT_TIMCON_BASE + 0x0204)) = 0;
    *((volatile uint32_t *)(CONN_BT_TIMCON_BASE + 0x0208)) = 0;
    *((volatile uint8_t *)(CONN_BT_TIMCON_BASE + 0x0200)) = 0;
}

BOOL SourceClose_DCHS_DL(SOURCE source)
{
    preloader_pisplit_free_memory(source->streamBuffer.BufferInfo.startaddr[0]);
    #ifdef AIR_SOFTWARE_MIXER_ENABLE
    if(mixer_member){
        stream_function_sw_mixer_member_unregister(SW_MIXER_PORT_0, mixer_member);
        stream_function_sw_mixer_member_delete(mixer_member);
        stream_function_sw_mixer_deinit(SW_MIXER_PORT_0);
        mixer_member = NULL;
    }
    #endif /* AIR_SOFTWARE_MIXER_ENABLE */
    if(hwsrc1_buf_addr){
        preloader_pisplit_free_memory(hwsrc1_buf_addr);
        hwsrc1_buf_addr = NULL;
    }
    if(hwsrc2_buf_addr){
        preloader_pisplit_free_memory(hwsrc2_buf_addr);
        hwsrc2_buf_addr = NULL;
    }
    if(dchs_gain_port){
        stream_function_sw_gain_deinit(dchs_gain_port);
        dchs_gain_port = NULL;
    }
    dchs_dl_silence_count[DCHS_DL_SINK] = 0;
    dchs_dl_set_scenario_play_en_exist(HAL_AUDIO_AGENT_MEMORY_DL12, false);
    dchs_dl_uart_buf_clear();
    if(dchs_dl_timer_handle){
        hal_gpt_sw_free_timer(dchs_dl_timer_handle);
        dchs_dl_timer_handle = 0;
    }
    dchs_dl_disable_play_en();
    DCHS_DL_LOG_I("[Source Close] source close done", 0);
    return TRUE;
}

/******************************************************************************
 *
 * Public Function Define
 *
 ******************************************************************************/
SOURCE SourceInit_DCHS_DL(SOURCE source)
{
    U8 *mem_ptr = NULL;
    U32 relay_buf_size = 20 * 1024; //align a2dp
    source->streamBuffer.BufferInfo.length   = relay_buf_size; //uart ch buf
    audio_scenario_type_t data_scenario_type       = (audio_scenario_type_t)source->param.audio.scenario_sub_id;
    mem_ptr = preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, relay_buf_size);
    if(!mem_ptr){
        AUDIO_ASSERT(0 && "[DCHS DL][Source] malloc source buffer fail!");
    }
    memset(mem_ptr, 0, relay_buf_size);
    DCHS_DL_LOG_I("[Source] malloc source buffer :0x%x, buf size = %d, frame size = %d, data_scenario_type=%d,sample_rate=%d,format_bytes=%d, uart channel_num=%d", 7, mem_ptr, relay_buf_size, source->param.audio.frame_size, data_scenario_type,source->param.audio.rate, source->param.audio.format_bytes, dchs_dl_ch_scenario_msg[UART_SCENARIO].channel_num);
    source->streamBuffer.BufferInfo.startaddr[0]  = mem_ptr;
    source->streamBuffer.BufferInfo.bBufferIsFull = FALSE;

    dchs_dl_ch_scenario_msg[UART_SCENARIO].sink_buf_info = &(source->streamBuffer.BufferInfo);
    if(dchs_get_device_mode() == DCHS_MODE_LEFT){
        dchs_dl_ch_scenario_msg[UART_SCENARIO].channel_num  = 1;
        dchs_dl_ch_scenario_msg[UART_SCENARIO].format_bytes = source->param.audio.format_bytes;
        dchs_dl_ch_scenario_msg[UART_SCENARIO].sample_rate  = source->param.audio.rate;
    }
    dchs_dl_ch_scenario_msg[UART_SCENARIO].sink_buf_info->WriteOffset = 0;
    dchs_dl_ch_scenario_msg[UART_SCENARIO].sink_buf_info->ReadOffset  = 0;
    //open uart
    dsp_uart_open(UART_DL);
    dsp_uart_open(UART_CMD);

    if(dchs_get_device_mode() == DCHS_MODE_RIGHT){
        //init sw gain
        SourceDCHSDLSwGainInit(source);
        //init sw mixer
        SourceDCHSDLSwMixerInit(source);
    }
    //if uart user buffer no empty ,clear it
    //dchs_dl_uart_buf_clear();
    //init uart frame size
    mux_ctrl_para_t para;
    if(dchs_get_device_mode() == DCHS_MODE_LEFT){
        para.mux_ll_user_tx_pkt_len = 4096;//temp set, later will set again
    }else{
        para.mux_ll_user_tx_pkt_len = AUDIO_DURATION_TIME * MUX_UART_BUF_SLICE * DCHS_DL_FIX_SAMPLE_RATE / 1000;//temp set, later will set again
    }
    DCHS_DL_LOG_I("[DCHS DL]set uart frame size=%d", 1, para.mux_ll_user_tx_pkt_len);
    mux_handle_t uart_handle = dsp_get_uart_handle(UART_DL);
    mux_control_ll(uart_handle, MUX_CMD_SET_LL_USER_TX_PKT_LEN, &para);

    //reset
    dchs_dl_reset_dchs_dl_stream_status();
    /* interface init */
    source->sif.SourceSize        = SourceSize_DCHS_DL;
    source->sif.SourceDrop        = SourceDrop_DCHS_DL;
    source->sif.SourceClose       = SourceClose_DCHS_DL;
    source->sif.SourceReadBuf     = SourceReadBuf_DCHS_DL;
    return source;
}

//ul func
bool stream_function_dchs_uplink_tx_initialize(void *para)
{
    UNUSED(para);
    return false;
}
bool stream_function_dchs_uplink_tx_process(void *para)
{
    ((DSP_ENTRY_PARA_PTR)para)->out_channel_num = 3;
    S16 *Buf = (S16 *)stream_function_get_1st_inout_buffer(para);
    U16 FrameSize = stream_function_get_output_size(para);
#ifdef AIR_AUDIO_DUMP_ENABLE
    LOG_AUDIO_DUMP((U8 *)Buf, (U32)FrameSize, AUDIO_DCHS_UL_LOCAL_TX);
#endif
    DCHS_UL_LOG_D("[UL UART_TX] mux uart buffer FrameSize = %d",1, FrameSize);
    dsp_uart_tx(UART_UL,(U8 *)Buf, FrameSize);
    return false;
}
bool stream_function_dchs_uplink_sw_buffer_slave_initialize(void *para)
{
    UNUSED(para);
    return false;
}
bool stream_function_dchs_uplink_sw_buffer_slave_process(void *para)
{
    if(dchs_get_device_mode() == DCHS_MODE_SINGLE){
        return false;
    }
    S16 *Buf_local = (S16 *)stream_function_get_2nd_inout_buffer(para);
    U16 FrameSize = stream_function_get_output_size(para);
    U32 uart_buf_size  = dsp_query_uart_rx_buf_remain_size(UART_UL);
    U8 *Buf_uart = pvPortMalloc(sizeof(U8)*FrameSize);
    memset(Buf_uart,0,FrameSize);
    if(uart_buf_size >= FrameSize){
        dsp_uart_rx(UART_UL, (U8 *)Buf_uart, FrameSize);
    }else{
        DCHS_UL_LOG_W("[ULL UART_RX]slave mux uart buffer size = %d, FrameSize = %d",2,uart_buf_size,FrameSize);
    }
    DCHS_UL_LOG_D("[ULL UART_RX] mux uart buffer size = %d, FrameSize = %d",2,uart_buf_size,FrameSize);
#ifdef AIR_AUDIO_DUMP_ENABLE
    LOG_AUDIO_DUMP((U8 *)Buf_uart, (U32)FrameSize, AUDIO_DCHS_UL_UART_RX_L);
#endif
    if(Ch_Select_Get_Param(CH_SEL_HFP)== CH_SEL_NOT_USED) {
        //Use Right Cup Mic
        Buf_local = (S16 *)stream_function_get_1st_inout_buffer(para);
    }
    memcpy(Buf_local,Buf_uart,FrameSize);

    vPortFree(Buf_uart);
    return false;
}
U8 sw_buffer_master_process_frist;
bool stream_function_dchs_uplink_sw_buffer_master_initialize(void *para)
{
    UNUSED(para);
    sw_buffer_master_process_frist = 0;
    return false;
}
bool stream_function_dchs_uplink_sw_buffer_master_process(void *para)
{
    ((DSP_ENTRY_PARA_PTR)para)->out_channel_num = 1;
    S16 *Buf_local = (S16 *)stream_function_get_1st_inout_buffer(para);
    U16 FrameSize = stream_function_get_output_size(para);
    U32 uart_buf_size;
    U8 *Buf_uart = pvPortMalloc(sizeof(U8)*960);
    memset(Buf_uart,0,960);
#ifdef AIR_AUDIO_DUMP_ENABLE
    LOG_AUDIO_DUMP((U8 *)Buf_local, (U32)FrameSize, AUDIO_DCHS_UL_LOCAL_TX);
#endif

    DCHS_UL_LOG_D("[UL UART_TX] mux uart buffer FrameSize = %d",1, FrameSize);
    dsp_uart_tx(UART_UL,(U8 *)Buf_local, FrameSize);
    uart_buf_size  = dsp_query_uart_rx_buf_remain_size(UART_UL);
    DCHS_UL_LOG_D("[BT UART_RX] mux uart buffer size = %d, FrameSize = %d",2,uart_buf_size,FrameSize);
    if(sw_buffer_master_process_frist == 0){
        memset(Buf_local,0,960);
        sw_buffer_master_process_frist = 1;
    }else{
        if(uart_buf_size >= FrameSize){
            dsp_uart_rx(UART_UL, (U8 *)Buf_uart, FrameSize);
            memcpy(Buf_local,Buf_uart,FrameSize);
        }else{
            DCHS_UL_LOG_W("[ULL UART_RX]master mux uart buffer size = %d, FrameSize = %d",2,uart_buf_size,FrameSize);
        }
    }

#ifdef AIR_AUDIO_DUMP_ENABLE
    LOG_AUDIO_DUMP((U8 *)Buf_uart, (U32)FrameSize, AUDIO_DCHS_UL_UART_RX_R);
    LOG_AUDIO_DUMP((U8 *)Buf_local, (U32)FrameSize, AUDIO_DCHS_UL_UART_RX_R_TX);
#endif
    vPortFree(Buf_uart);
    return false;
}
void dps_uart_relay_ul_mem_sync_info(uint32_t delay_time,S32 cur_native_bt_clk, S32 cur_native_bt_phase)
{
    DSP_MW_LOG_I("[DCHS UL][UART_TX]ul mux uart cur_native_bt_clk = %d,cur_native_bt_phase:%d",2, cur_native_bt_clk,cur_native_bt_phase);
    cur_native_bt_clk += delay_time/0.3125;
    audio_dchs_dsp2dsp_cmd_t dchs_dsp2dsp_cmd;
    dchs_dsp2dsp_cmd.cmd_param.dchs_ul_param.play_bt_clk    = cur_native_bt_clk;
    dchs_dsp2dsp_cmd.cmd_param.dchs_ul_param.play_bt_phase  = cur_native_bt_phase;
    dchs_dsp2dsp_cmd.header.cmd_type = AUDIO_DCHS_UL_MEM_SYNC_INFO;
    dchs_dsp2dsp_cmd.header.param_size = sizeof(audio_dchs_dsp2dsp_cmd_t) - sizeof(uart_cmd_header_t);
    DSP_MW_LOG_I("[DCHS UL][UART_TX]ul after add mux uart cur_native_bt_clk = %d,cur_native_bt_phase:%d",2, cur_native_bt_clk,cur_native_bt_phase);
    dsp_uart_tx(UART_CMD, (U8 *)&dchs_dsp2dsp_cmd, sizeof(audio_dchs_dsp2dsp_cmd_t));
}
uint32_t dchs_hfp_handle_vul;
uint32_t gpt_count_sub_end;
extern hal_audio_memory_selection_t dchs_sub_ul_mem;
ATTR_TEXT_IN_RAM_FOR_MASK_IRQ static void hal_audio_gpt_trigger_mem_vul(void)
{
    uint32_t savedmask = 0;
    uint32_t curr_cnt  = 0;
    S32 cur_native_bt_clk = 0, cur_native_bt_phase = 0;

    hal_nvic_save_and_set_interrupt_mask_special(&savedmask); // enter cirtical code region

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &curr_cnt);
    if (gpt_count_sub_end > curr_cnt) { // gpt register does not overflow
        // DSP_MW_LOG_I("[DCHS UL][hfp set value] trigger %d curr_cnt %u  tar %d", 3, __LINE__, curr_cnt, gpt_count_sub_end);
        while (1) {
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &curr_cnt);
            if (curr_cnt >= gpt_count_sub_end) { // expire at time
                break;
            }
        }
    } else if (curr_cnt - gpt_count_sub_end > 0x7fffffff) { // gpt register overflow
        // DSP_MW_LOG_I("[DCHS UL][hfp set value] trigger %d curr_cnt %u  tar %d", 3, __LINE__, curr_cnt, gpt_count_sub_end);
        while (1) {
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &curr_cnt);
            if (curr_cnt >= gpt_count_sub_end) { // expire at time
                if ((curr_cnt & 0x80000000) == 0x0) {
                    break;
                }
            }
        }
    }

    hal_audio_trigger_start_parameter_t start_parameter;
    start_parameter.memory_select = dchs_sub_ul_mem;
    start_parameter.enable = true;
    hal_audio_set_value((hal_audio_set_value_parameter_t *)&start_parameter, HAL_AUDIO_SET_TRIGGER_MEMORY_START);
    MCE_GetBtClk((BTCLK *)&cur_native_bt_clk, (BTPHASE *)&cur_native_bt_phase, DCHS_CLK_Offset);
    hal_gpt_sw_free_timer(dchs_hfp_handle_vul);
    hal_nvic_restore_interrupt_mask_special(savedmask);
    DSP_MW_LOG_I("[DCHS UL]sub Mem trigger vul1 memory_select:%x,cur_native_bt_clk:%u,cur_native_bt_phase:%u",3,start_parameter.memory_select,cur_native_bt_clk,cur_native_bt_phase);
}
ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void ul_afe_set_mem_enable(S32 play_bt_clk, S32 play_bt_phase)
{
    S32 cur_native_bt_clk = 0, cur_native_bt_phase = 0, bt_clk_diff;
    uint32_t count_1;
    hal_sw_gpt_absolute_parameter_t  dchs_hfp_absolute_parameter;
                
    hal_gpt_sw_get_timer(&dchs_hfp_handle_vul);
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &count_1);
    MCE_GetBtClk((BTCLK *)&cur_native_bt_clk, (BTPHASE *)&cur_native_bt_phase, DCHS_CLK_Offset);
    bt_clk_diff = (play_bt_clk - cur_native_bt_clk)*312.5 + (play_bt_phase - cur_native_bt_phase)*0.5;
    gpt_count_sub_end = count_1 + (uint32_t)bt_clk_diff;
    dchs_hfp_absolute_parameter.absolute_time_count = count_1 + (uint32_t)bt_clk_diff - 600;
    dchs_hfp_absolute_parameter.callback = (void*)hal_audio_gpt_trigger_mem_vul;
    dchs_hfp_absolute_parameter.maxdelay_time_count = bt_clk_diff;
    hal_gpt_sw_start_timer_for_absolute_tick_1M(dchs_hfp_handle_vul,&dchs_hfp_absolute_parameter);
}
void dchs_ul_set_bt_clk(S32 play_bt_clk, S32 play_bt_phase)
{
    S32 cur_native_bt_clk = 0, cur_native_bt_phase = 0;
    MCE_GetBtClk((BTCLK *)&cur_native_bt_clk, (BTPHASE *)&cur_native_bt_phase, DCHS_CLK_Offset);
    if ((play_bt_clk - cur_native_bt_clk) > 0x01) { // check play time in time
        DSP_MW_LOG_I("[DCHS UL]UL bt clk:%u,bt clk phase:%u,cur_native_bt_clk:%u,cur_native_bt_clk_phase:%u", 4, play_bt_clk,play_bt_phase, cur_native_bt_clk,cur_native_bt_phase);
        ul_afe_set_mem_enable(play_bt_clk,play_bt_phase);
    }else{
        DSP_MW_LOG_I("[DCHS UL]UL bt clk not legal bt clk:%u,bt clk phase:%u,cur_native_bt_clk:%u,cur_native_bt_clk_phase:%u", 4, play_bt_clk,play_bt_phase, cur_native_bt_clk,cur_native_bt_phase);
    }
}