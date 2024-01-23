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
 * Include
 ******************************************************************************/
#include "dsp_mux_uart.h"
#include "dsp_temp.h"
#include "dsp_dump.h"
#include "source_inter.h"
#include "stream_dchs.h"
#include "stream_mixer.h"

/******************************************************************************
 * Private Macro
 ******************************************************************************/
#define DSP_TEST_SOURCE_DEBUG         (0)
#define DSP_UART_SOURCE_ONLY_DEBUG    (0)

/******************************************************************************
 * Private Global Variables
 ******************************************************************************/
static mux_handle_t        g_uart_handle[UART_TYPE_MAX] = {0};
static BUFFER_INFO_PTR     dl_rx_buffer = NULL;
static BUFFER_INFO_PTR     ul_rx_buffer = NULL;
typedef enum
{
    READY_READ,
    WAITING_READ,
}cmd_status_t;

typedef struct
{
    uart_cmd_type_t cmd_type;
    uint32_t param_size;
    cmd_status_t cmd_status;
}uart_cmd_status_t;
static uart_cmd_status_t cur_uart_cmd_status = {AUDIO_DCHS_CMD_NONE, 0 , READY_READ};

extern ATTR_TEXT_IN_IRAM_LEVEL_1 VOID AudioCheckTransformHandle(TRANSFORM transform);

/******************************************************************************
 * Public Global Variables
 ******************************************************************************/

/******************************************************************************
 * Private Function Define
 ******************************************************************************/
void dsp_uart_rx(uart_type_t type, U8 * user_rx_buf, U32 buf_size)
{
    if(user_rx_buf == NULL){
        AUDIO_ASSERT(0 && "dsp uart rx buf == NULL");
    }
    mux_handle_t uart_handle = g_uart_handle[type];
    if(!uart_handle){
        dsp_uart_open(type);
    }
    mux_status_t mux_status;
    mux_buffer_t uart_rx_buffer;
    U32 rx_size = 0;
    uart_rx_buffer.p_buf    = user_rx_buf;
    uart_rx_buffer.buf_size = buf_size;
    mux_status = mux_rx(uart_handle, &uart_rx_buffer, &rx_size);
    if (mux_status != MUX_STATUS_OK) {
        DSP_MW_LOG_E("[DCHS][uart callback] dsp uart rx fail,status=%d,type:%d", 2, mux_status,type);
    }
    if(rx_size != buf_size){
        DSP_MW_LOG_E("[DCHS][uart callback] dsp uart rx fail,already_rx_size=%d,need_rx_size=%d,uart type:%d", 3, rx_size, buf_size, type);
    }
    //LOG_AUDIO_DUMP(user_rx_buf,rx_size,AUDIO_DCHS_UART_DL_SOURCE);
    //DSP_MW_LOG_I("[DCHS][dsp uart] rx data size: %d", 1, buf_size);
}

uint32_t dsp_query_uart_rx_buf_remain_size(uart_type_t type)
{
    mux_handle_t uart_handle = g_uart_handle[type];
    // ul uart extend here
    if(!uart_handle){
        dsp_uart_open(type);
    }
    mux_ctrl_para_t rx_param;
    mux_user_control(uart_handle, MUX_CMD_GET_LL_USER_RX_BUFFER_DATA_SIZE, &rx_param);
    return rx_param.mux_ll_user_rx_data_len;
}

void reset_cur_cmd_status()
{
    //reset cur cmd status
    cur_uart_cmd_status.cmd_status = READY_READ;
    cur_uart_cmd_status.cmd_type  = AUDIO_DCHS_CMD_NONE;
    cur_uart_cmd_status.param_size = 0;
}

ATTR_TEXT_IN_IRAM void dsp_mux_uart_callback(mux_handle_t handle, mux_event_t event, uint32_t data_len, void *user_data)
{
    UNUSED(user_data);
    UNUSED(data_len);
    //DSP_MW_LOG_I("[DSP][MUX UART][uart callback] event %d,handle 0x%x, cmd_uart_handle 0x%x",3,event, handle, g_uart_handle[UART_CMD]);
    switch (event) {
        case MUX_EVENT_READY_TO_READ:
            if(handle == g_uart_handle[UART_DL]){
                #if DSP_UART_SOURCE_ONLY_DEBUG
                static U8 test_buff[1024];
                dsp_uart_rx(UART_DL,test_buff,data_len);
                return;
                #endif
                dchs_dl_copy_uart_data_2_source_buf();//copy uart data to source buf
                return;
            }
            if(handle == g_uart_handle[UART_CMD]){
                while (dsp_query_uart_rx_buf_remain_size(UART_CMD))
                {
                    if (cur_uart_cmd_status.cmd_status == READY_READ){
                        uint32_t uart_remain_size =  dsp_query_uart_rx_buf_remain_size(UART_CMD);
                        if(uart_remain_size >= sizeof(uart_cmd_header_t)){
                            uart_cmd_header_t cmd_header;
                            dsp_uart_rx(UART_CMD, (uint8_t*)&cmd_header, sizeof(uart_cmd_header_t));
                            cur_uart_cmd_status.cmd_type  = cmd_header.cmd_type;
                            cur_uart_cmd_status.param_size = cmd_header.param_size;
                            if(cur_uart_cmd_status.cmd_type <= AUDIO_DCHS_CMD_NONE || cur_uart_cmd_status.cmd_type >= AUDIO_DCHS_CMD_MAX){
                                DSP_MW_LOG_E("[DCHS][uart callback]  rx invalid dsp cmd, param_size = %d", 1, cur_uart_cmd_status.param_size);
                                //AUDIO_ASSERT(0);
                                return;
                            }
                            cur_uart_cmd_status.cmd_status = WAITING_READ;
                        } else {
                            break;//end while
                        }
                    }
                    if (cur_uart_cmd_status.cmd_status == WAITING_READ){
                        uint32_t uart_remain_size =  dsp_query_uart_rx_buf_remain_size(UART_CMD);
                        if(uart_remain_size >= cur_uart_cmd_status.param_size){
                            audio_dchs_dsp2dsp_cmd_t dchs_dsp2dsp_cmd;
                            uint8_t header_size  = sizeof(uart_cmd_header_t);
                            dsp_uart_rx(UART_CMD, (uint8_t*)&dchs_dsp2dsp_cmd + header_size, sizeof(audio_dchs_dsp2dsp_cmd_t) - header_size);
                            if(cur_uart_cmd_status.cmd_type == AUDIO_DCHS_DL_PLAY_EN_INFO)
                            {
                                U32 play_en_clk   = dchs_dsp2dsp_cmd.cmd_param.dchs_dl_param.play_en_clk;
                                U16 play_en_phase = dchs_dsp2dsp_cmd.cmd_param.dchs_dl_param.play_en_phase;
                                DSP_MW_LOG_I("[DCHS DL][dsp2dsp rx cmd] get play_en_clk = %u, play_en_phase = %u, cmd type = AUDIO_DCHS_DL_PLAY_EN_INFO", 2 , play_en_clk, play_en_phase);
                                dchs_dl_set_play_en(play_en_clk, play_en_phase);
                            }else if (cur_uart_cmd_status.cmd_type == AUDIO_DCHS_UL_MEM_SYNC_INFO){
                                S32 play_bt_clk   = dchs_dsp2dsp_cmd.cmd_param.dchs_ul_param.play_bt_clk;
                                S32 play_bt_phase = dchs_dsp2dsp_cmd.cmd_param.dchs_ul_param.play_bt_phase;
                                DSP_MW_LOG_I("[DCHS UL][dsp2dsp rx cmd] ul get play_bt_clk = %u, play_bt_phase = %u, cmd type = AUDIO_DCHS_DL_PLAY_EN_INFO", 2 , play_bt_clk, play_bt_phase);
                                dchs_ul_set_bt_clk(play_bt_clk, play_bt_phase);
                            }else if(cur_uart_cmd_status.cmd_type == AUDIO_DCHS_DL_UART_SCENARIO_PREFILL_SIZE){
                                U32 prefill_size = dchs_dsp2dsp_cmd.cmd_param.dchs_dl_param.prefill_size;
                                source_ch_type_t ch_type = mixer_stream_get_source_ch_by_agent(HAL_AUDIO_MEM_SUB);
                                U32 uart_remain = mixer_get_ch_data_size(ch_type);
                                if(uart_remain >= prefill_size){
                                    mix_scenarios_msg[ch_type].sink_buf_info->WriteOffset = (mix_scenarios_msg[ch_type].sink_buf_info->WriteOffset + prefill_size) 
                                                                                            % (mix_scenarios_msg[ch_type].sink_buf_info->length);
                                    if(mix_scenarios_msg[ch_type].hwsrc_enable){
                                        update_hwsrc_input_wrpnt(mix_scenarios_msg[ch_type].hwsrc_id, mix_scenarios_msg[ch_type].sink_buf_info->WriteOffset);
                                    }
                                } else {
                                    AUDIO_ASSERT(0 && "prefill meet uart buffer space not enough");
                                }
                                DSP_MW_LOG_I("[DCHS DL][dsp2dsp rx cmd] rx uart prefill:%d, pre uart remain:%d, now uart wo:%d", 4, prefill_size, uart_remain, mix_scenarios_msg[ch_type].sink_buf_info->WriteOffset);
                            }
                            /***************
                             *   ul extend here
                             * ************/
                            //reset cur cmd status
                            reset_cur_cmd_status();
                        } else {
                            break;//end while
                        }
                    }
                }//while
            }
            break;
        case MUX_EVENT_READY_TO_WRITE:
            DSP_MW_LOG_I("[DCHS][uart callback] MUX_EVENT_READY_TO_WRITE", 0);
            break;
        default:
            DSP_MW_LOG_I("[DCHS][uart callback] no event: %d", 1, event);
    }
}

#if DSP_TEST_SOURCE_DEBUG
#include "hal_gpt.h"
U8 test_data_buf[512];
static uint32_t timer_handle;
static void dsp_uart_source_timer_callback_test(void *user_data)
{
    uint32_t need_rx_size = 0;
    if(!dl_rx_buffer){
        DSP_MW_LOG_E("[MUX UART][uart callback] no register dl_rx_buffer",0);
        AUDIO_ASSERT(0);
        return;
    }
    U32 writeOffset = dl_rx_buffer->WriteOffset;
    U32 readOffset  = dl_rx_buffer->ReadOffset;
    U32 length      = dl_rx_buffer->length;
    U32 available_length = (writeOffset >= readOffset) ? (length + readOffset - writeOffset) : (readOffset - writeOffset);
    if(dl_rx_buffer->bBufferIsFull){
        DSP_MW_LOG_W("[DCHS][uart callback] bBufferIsFull ", 0);
        hal_gpt_sw_start_timer_ms(timer_handle, 3, dsp_uart_source_timer_callback_test, NULL);
        return;
    }
    need_rx_size = MIN(available_length, 512);//[hard code]
    if (length - writeOffset >= need_rx_size){
        memcpy(dl_rx_buffer->startaddr[0] + writeOffset,test_data_buf, need_rx_size);
    }else{
        //read part1
        memcpy(dl_rx_buffer->startaddr[0] + writeOffset, test_data_buf,length - writeOffset);
        //read part2
        memcpy( dl_rx_buffer->startaddr[0],(test_data_buf+(length - writeOffset)) ,need_rx_size - (length - writeOffset));
    }
    dl_rx_buffer->WriteOffset = (dl_rx_buffer->WriteOffset + need_rx_size) % (dl_rx_buffer->length);
    if (dl_rx_buffer->WriteOffset == dl_rx_buffer->ReadOffset) {
        dl_rx_buffer->bBufferIsFull = TRUE;
    }
    //DSP_MW_LOG_W("[DCHS][uart callback] dl_rx_buffer->WriteOffset=%d", 1,dl_rx_buffer->WriteOffset);
    hal_gpt_sw_start_timer_ms(timer_handle, 3, dsp_uart_source_timer_callback_test, NULL);
}
#endif

/******************************************************************************
 * Public Function Define
 ******************************************************************************/

void dsp_uart_rx_buffer_register(uart_type_t type, BUFFER_INFO_PTR rx_buffer)
{
    if(type == UART_DL){
        dl_rx_buffer = rx_buffer;
    }else if(type == UART_UL){
        ul_rx_buffer = rx_buffer;
    }else {
        DSP_MW_LOG_E("[MUX UART][rx buffer register] don't support uart type = %d", 1, type);
    }
    DSP_MW_LOG_I("[MUX UART][rx buffer register] success, buffer len= %d,addr=0x%x", 2, dl_rx_buffer->length,dl_rx_buffer->startaddr[0]);
}



void dsp_uart_open(uart_type_t type)
{
    if(g_uart_handle[type]){
        DSP_MW_LOG_I("[DCHS][UART]type:%d already opened!", 1, type);
        return;
    }
    char * uart_user_name = "";
    if(type == UART_DL){
        uart_user_name = "R2L_DL";
    }else if(type == UART_UL){
        uart_user_name = "UL";
    }else if(type == UART_CMD){
        uart_user_name = "AUDIO_CMD_DSP";
    }
    mux_status_t status = mux_open(MUX_LL_UART_1, uart_user_name, &g_uart_handle[type], dsp_mux_uart_callback, NULL);
    if(status != MUX_STATUS_OK){
        g_uart_handle[type] = 0;
        DSP_MW_LOG_E("[DCHS][UART]type:%d open failed, status = %d", 2, type, status);
        AUDIO_ASSERT(0);
        return;
    }
    DSP_MW_LOG_I("[DCHS][UART]type:%d open success, uart_handle = 0x%x",2, type, g_uart_handle[type]);
}

mux_handle_t dsp_get_uart_handle(uart_type_t type)
{
    mux_handle_t uart_handle = g_uart_handle[type];
    // ul extend here
    if(!uart_handle){
        DSP_MW_LOG_E("[DCHS][UART]dsp uart don't open, type = %d",1,type);
        assert(0);
    }
    return uart_handle;
}

void dsp_uart_tx(uart_type_t type, uint8_t *user_tx_buffer, uint32_t buf_size)
{
    if(user_tx_buffer == NULL){
        AUDIO_ASSERT(0 && "dsp uart tx buf == NULL");
    }
    mux_handle_t uart_handle = g_uart_handle[type]; 
    if(!uart_handle){
        dsp_uart_open(type);
    }
    if (type == UART_CMD) {
        DSP_MW_LOG_I("[DCHS][UART]dsp cmd tx, uart_handle = 0x%x, data size = %d",2, uart_handle, buf_size);
    }
    mux_buffer_t uart_tx_buffer;
    uint32_t tx_size = 0;
    uart_tx_buffer.buf_size = buf_size;
    uart_tx_buffer.p_buf    = user_tx_buffer;
    mux_status_t status = mux_tx(uart_handle, &uart_tx_buffer, 1, &tx_size);
    if (status != MUX_STATUS_OK) {
        DSP_MW_LOG_E("[DCHS][UART]dsp uart tx fail, status=%d, uart_handle = 0x%x", 2, status, g_uart_handle[UART_CMD]);
    }
    if (tx_size != buf_size) {
        DSP_MW_LOG_E("[DCHS][UART]dsp uart tx fail, uart_handle = 0x%x, buf_size = %d, already send = %d", 3, uart_handle, buf_size, tx_size);
    }
    DSP_MW_LOG_D("[DCHS][UART]dsp uart tx success, uart_handle = 0x%x, data size = %d",2, uart_handle, buf_size);
}
void dsp_uart_ul_open(void)
{
    dsp_uart_open(UART_UL);
    dsp_uart_open(UART_CMD);
}
void dsp_uart_ul_clear_tx_buffer(void)
{
    mux_handle_t uart_handle = dsp_get_uart_handle(UART_UL);
    mux_clear_ll_user_buffer(uart_handle, false);
}
void dsp_uart_ul_clear_rx_buffer(void)
{
    mux_handle_t uart_handle = dsp_get_uart_handle(UART_UL);
    mux_clear_ll_user_buffer(uart_handle, true);
}
