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
#include "stream_mixer.h"
#include "scenario_wired_audio.h"

static sw_gain_port_t *dchs_ul_gain_port = NULL;

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


/******************************************************************************
 *
 * Private Macro and Variable Declaration
 *
 ******************************************************************************/
BUFFER_INFO dl_uart_buf_info = {0};

/******************************************************************************
 *
 * Private Function Define
 *
 ******************************************************************************/
void dchs_dl_copy_uart_data_2_source_buf()
{
    U32 uart_buf_size  = dsp_query_uart_rx_buf_remain_size(UART_DL);
    source_ch_type_t ch_type = mixer_stream_get_source_ch_by_agent(HAL_AUDIO_MEM_SUB);
    U32 uart_scenario_reamain = mixer_get_ch_data_size(ch_type) - 32; //avoid upadte hwsrc wo = ro case
    U32 copy_uart_data_size = MIN(uart_buf_size, uart_scenario_reamain);
    if(!uart_scenario_reamain){
        DCHS_DL_LOG_W("[Source Read] source uart buf full", 0);
        return;
    }
    DCHS_DL_LOG_D("[Source Read] mux uart_buf_size=%d,uart_scenario_reamain=%d,copy_uart_data_size=%d", 3,uart_buf_size,uart_scenario_reamain,copy_uart_data_size);
    if(copy_uart_data_size){
        U32 wo        = mix_scenarios_msg[ch_type].sink_buf_info->WriteOffset;
        U32 length    = mix_scenarios_msg[ch_type].sink_buf_info->length;
        U8 *src_start = mix_scenarios_msg[ch_type].sink_buf_info->startaddr[0];
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
        mix_scenarios_msg[ch_type].sink_buf_info->WriteOffset = (wo + copy_uart_data_size) % (length);
        if(mix_scenarios_msg[ch_type].hwsrc_enable){
            update_hwsrc_input_wrpnt(mix_scenarios_msg[ch_type].hwsrc_id, mix_scenarios_msg[ch_type].sink_buf_info->WriteOffset);
        }
    }else{
        //DCHS_DL_AUDIO_LOG_I("[Source Read] copy_uart_data_size=0",0);
    }
}

void dchs_dl_uart_relay_play_en_info(U32 play_en_clk, U16 play_en_phase)
{
    audio_dchs_dsp2dsp_cmd_t dchs_dsp2dsp_cmd;
    dchs_dsp2dsp_cmd.cmd_param.dchs_dl_param.play_en_clk    = play_en_clk;
    dchs_dsp2dsp_cmd.cmd_param.dchs_dl_param.play_en_phase  = play_en_phase;
    dchs_dsp2dsp_cmd.header.cmd_type = AUDIO_DCHS_DL_PLAY_EN_INFO;
    dchs_dsp2dsp_cmd.header.param_size = sizeof(audio_dchs_dsp2dsp_cmd_t) - sizeof(uart_cmd_header_t);
    DCHS_DL_LOG_I("[tx cmd] relay Play en clk:0x%x(%d),phase:0x%x(%d)", 4, play_en_clk, play_en_clk, play_en_phase, play_en_phase);
    //uart relay play en info to other chip
    dsp_uart_tx(UART_CMD, (U8 *)&dchs_dsp2dsp_cmd, sizeof(audio_dchs_dsp2dsp_cmd_t));
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
    *pNativeBTCLK &= 0x0FFFFFFC;
}

void dchs_dl_set_hfp_play_en(void)
{
    U32 play_en_nclk;
    U16 play_en_intra_clk;
    MCE_TransBT2NativeClk(Forwarder_Rx_AncClk() + 0x8, 1250, &play_en_nclk, &play_en_intra_clk, BT_CLK_Offset);// - phone offset
    mixer_stream_setup_play_en(play_en_nclk, play_en_intra_clk, NULL, AUDIO_SCENARIO_TYPE_HFP_DL);
    DCHS_DL_LOG_I("Play en 4 anchor clk, play_en_nclk:0x%08x(%u), play_en_nphase:0x%08x(%u),cur native clk=%u,dchs clk offset=0x%08x(%d)", 7, play_en_nclk, play_en_nclk,play_en_intra_clk,play_en_intra_clk,rBb->rClkCtl.rNativeClock & 0x0FFFFFFC,*((volatile uint32_t *)(0xA0010974)),*((volatile int32_t *)(0xA0010974)));
    DCHS_TransBT2NativeClk(play_en_nclk, play_en_intra_clk, &play_en_nclk, &play_en_intra_clk, DCHS_CLK_Offset);// + dchs offset
    dchs_dl_uart_relay_play_en_info(play_en_nclk, play_en_intra_clk);
}

void dchs_dl_set_play_en(U32 play_en_clk, U16 play_en_phase)
{
    U32 native_play_clk = 0;
    U16 native_play_phase = 0;
    U32 cur_native_clk = rBb->rClkCtl.rNativeClock & 0x0FFFFFFC;
    MCE_TransBT2NativeClk((BTCLK)play_en_clk, (BTPHASE)play_en_phase, (BTCLK *)&native_play_clk, (BTPHASE *)&native_play_phase, DCHS_CLK_Offset);
    DCHS_DL_LOG_I("Play en native play clk:0x%08x(%u), play phase:0x%08x(%u), cur native clk=0x%08x(%u), clk offset:0x%08x(%d),phase offset:0x%08x(%d), period ms:%d", 11, native_play_clk, native_play_clk,native_play_phase, native_play_phase,
                cur_native_clk,cur_native_clk, *((volatile uint32_t *)(0xA0010974)),*((volatile int32_t *)(0xA0010974)), *((volatile uint32_t *)(0xA0010978)),*((volatile int32_t *)(0xA0010978)), (native_play_clk-cur_native_clk)*0.3125);
    if (native_play_clk > cur_native_clk) {
        mixer_stream_setup_play_en(native_play_clk, native_play_phase, NULL, AUDIO_SCENARIO_TYPE_COMMON);//uart sceanrio/sub agent,common scenario.
    }else{
        AUDIO_ASSERT(0 && "[DCHS DL]Play en not legal, is too short");
    }
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

void dchs_dl_uart_buf_clear()
{
    U32 uart_buf_data_size = dsp_query_uart_rx_buf_remain_size(UART_DL);
    DCHS_DL_LOG_I("enter clear uart bug, size:%d", 1, uart_buf_data_size);
    if(uart_buf_data_size){
        mux_handle_t uart_handle = dsp_get_uart_handle(UART_DL);
        mux_clear_ll_user_buffer(uart_handle, true);
    }
}

void dchs_dl_uart_buf_init(source_ch_type_t ch_type)
{
    if(!dl_uart_buf_info.startaddr[0])
    {
        dl_uart_buf_info.startaddr[0] = preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, DCHS_DL_UART_BUF_SIZE);
        dl_uart_buf_info.length       = DCHS_DL_UART_BUF_SIZE;
        mix_scenarios_msg[ch_type].sink_buf_info = &dl_uart_buf_info;
        DCHS_DL_LOG_I("malloc uart buffer", 0);
    }
    dchs_dl_uart_buf_clear();
}

bool dchs_dl_check_fill_silence(void)
{
    volatile SINK dchs_dl_sink = Sink_blks[SINK_TYPE_AUDIO_DL12];
    U32 dchs_sink_threshold = (dchs_get_device_mode() == DCHS_MODE_RIGHT ? DCHS_DL_SINK_THRESHOLD_MASTER : DCHS_DL_SINK_THRESHOLD_SLAVE) * (dchs_dl_sink->param.audio.rate / 1000) * dchs_dl_sink->param.audio.format_bytes * dchs_dl_sink->param.audio.channel_num;
    U32 dchs_sink_size = 0;//SinkSizeAudioAfe(dchs_dl_sink);
    if(dchs_sink_size <= dchs_sink_threshold)
    {
        DCHS_DL_LOG_W("[Source Size]sink under threshold, sink size:%d,threshold:%d", 2, dchs_sink_size, dchs_sink_threshold);
        return true;
    }
    return false;
}

/******************************************************************************
 *
 * Public Function Define
 *
 ******************************************************************************/
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

void dchs_ul_sw_gain_init(SOURCE source)
{
    uint32_t out_channel_num;
#ifdef AIR_ECHO_MEMIF_IN_ORDER_ENABLE
    out_channel_num = source->param.audio.channel_num;
#else
    if (source->param.audio.echo_reference) {
        out_channel_num = source->param.audio.channel_num + 1;
    } else {
        out_channel_num = source->param.audio.channel_num;
    }
#endif
    /* SW GAIN init */
    sw_gain_config_t default_config;
    default_config.resolution = RESOLUTION_16BIT;
    default_config.target_gain = 0;
    default_config.up_step = 1;
    default_config.up_samples_per_step = 2;
    default_config.down_step = -1;
    default_config.down_samples_per_step = 2;
    dchs_ul_gain_port = stream_function_sw_gain_get_port(source);
    DSP_MW_LOG_I("[DCHS UL USB_OUT][start] sw gain init: 0x%08x, %d, %d, target_gain:%d, %d, %d, %d, %d", 8,
                                dchs_ul_gain_port,
                                out_channel_num,
                                default_config.resolution,
                                default_config.target_gain,
                                default_config.up_step,
                                default_config.up_samples_per_step,
                                default_config.down_step,
                                default_config.down_samples_per_step);
    stream_function_sw_gain_init(dchs_ul_gain_port, out_channel_num, &default_config);
}

void dchs_ul_sw_gain_config(void *config_param, SOURCE source, SINK sink)
{
    int32_t new_gain,i;
    sw_gain_config_t old_config;
    ul_sw_gain_config_param_p p_config_param;
    DSP_STREAMING_PARA_PTR ul_stream;
    uint8_t input_channel_number;
    ul_stream = DSP_Streaming_Get(source, sink);
    input_channel_number = ul_stream->callback.EntryPara.in_channel_num;
    p_config_param = (ul_sw_gain_config_param_p)config_param;

    for (i = 0; i < input_channel_number; i++) {
        new_gain = p_config_param->gain[0];
        stream_function_sw_gain_get_config(dchs_ul_gain_port, i + 1, &old_config);
        DSP_MW_LOG_I("[config] change channel%d gain from %d*0.01dB to %d*0.01dB\r\n", 3,
                        2,
                        old_config.target_gain,
                        new_gain);
        stream_function_sw_gain_configure_gain_target(dchs_ul_gain_port, i + 1, new_gain);
    }
}

void dchs_ul_sw_gain_deinit(void)
{
    if(dchs_ul_gain_port){
        stream_function_sw_gain_deinit(dchs_ul_gain_port);
        dchs_ul_gain_port = NULL;
    }
}
