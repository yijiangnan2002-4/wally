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
#ifdef AIR_DCHS_MODE_ENABLE
/******************************************************************************
 *
 * Include
 *
 ******************************************************************************/
#include "scenario_dchs.h"
#include "mux.h"
#include "memory_attribute.h"
#include "bt_a2dp.h"
#include "bt_sink_srv_ami.h"
#include "audio_transmitter_playback_port.h"
#include "audio_dump.h"
#include "hal_sleep_manager_platform.h"
#include "mux_ll_uart.h"
#include "hal_dvfs.h"
#include "hal_platform.h"
#include "race_xport.h"
#include "race_util.h"
#include "race_cmd_dsprealtime.h"

/******************************************************************************
 *
 * Extern Function and Variable
 *
 ******************************************************************************/
#if defined(MTK_AVM_DIRECT)
extern HAL_DSP_PARA_AU_AFE_CTRL_t audio_nvdm_HW_config;
#endif
extern void bt_sink_srv_am_set_volume(bt_sink_srv_am_stream_type_t in_out, bt_sink_srv_audio_setting_vol_info_t *vol_info);
extern void user_audio_transmitter_callback(audio_transmitter_event_t event, void *data, void *user_data);
extern void mux_ll_latch_timer_start(bool is_one_shot, uint32_t timeout);
extern void mux_ll_latch_timer_stop(void);
extern void DCHS_pka_lock_bt_sleep(void (*callback)(void *user_data), void* user_data);
extern void DCHS_pka_unlock_bt_sleep(void);

/******************************************************************************
 *
 * Private Macro and Variable Declaration
 *
 ******************************************************************************/
#define MUX_UART_TXBUF_MAX_SIZE  (4096)
#define MUX_UART_RXBUF_MAX_SIZE  (8192)
#ifdef AIR_DCHS_MODE_MASTER_ENABLE
#define MUX_UART_BUF_SLICE       (1)
#else
#define MUX_UART_BUF_SLICE       (4)
#endif
#define AUDIO_DURATION_TIME      (5)
#define HFP_DVFS_INHOUSE         HAL_DVFS_OPP_HIGH
#define MAX_SCENARIO_NUM         (10)
#define MAX_MIXER_NUM            (10)
#define PLAY_DELAY_MS            (20)

log_create_module(DCHS_DL,       PRINT_LEVEL_INFO);
log_create_module(DCHS_DL_DEBUG, PRINT_LEVEL_WARNING);

#define DCHS_DL_LOG_E(fmt, arg...) LOG_MSGID_E(DCHS_DL,       "[DCHS DL] "fmt,##arg)
#define DCHS_DL_LOG_W(fmt, arg...) LOG_MSGID_W(DCHS_DL,       "[DCHS DL] "fmt,##arg)
#define DCHS_DL_LOG_I(fmt, arg...) LOG_MSGID_I(DCHS_DL,       "[DCHS DL] "fmt,##arg)
#define DCHS_DL_LOG_D(fmt, arg...) LOG_MSGID_I(DCHS_DL_DEBUG, "[DCHS_DL_DEBUG] "fmt,##arg)

/********************************************** /
 *                                             *
 *               DCHS DL Variable              * 
 *                                             * 
************************************************/
audio_scenario_type_t dchs_dl_support_scenarios[MAX_SCENARIO_NUM] = {
    AUDIO_SCENARIO_TYPE_HFP_DL,
    AUDIO_SCENARIO_TYPE_A2DP,
    AUDIO_SCENARIO_TYPE_BLE_DL,
    AUDIO_SCENARIO_TYPE_VP,
    AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_IN,
    AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_0,
    AUDIO_SCENARIO_TYPE_COMMON,
    AUDIO_SCENARIO_TYPE_COMMON,
    AUDIO_SCENARIO_TYPE_COMMON,
    AUDIO_SCENARIO_TYPE_COMMON
};

typedef struct
{
    bool is_running;
    bool latch_timer_start_flag;
    audio_scenario_type_t scenario_type;
    U32 debug_timer_handle;
    bool need_sync_scenario_msg;
}dchs_dl_running_status_t;

typedef enum
{
    SYNC_SCENARIO_OPEN,
    SYNC_SCENARIO_CLOSE,
    SYNC_SET_GAIN_VALUE,
    //extend here
    SYNC_TYPE_MAX = 0xFFFFFFFF,
}mcu2dsp_sync_type_t;

typedef struct
{
    mcu2dsp_sync_type_t sync_type;
    audio_scenario_type_t scenario_type;
    hal_audio_memory_t memory_agent;
    int32_t  vol_gain;
    U8 format_bytes;
    U8 clkskew_mode;
    bool need_play_en;
    U32 bt_clk;
    U16 bt_phase;
    U32 sample_rate;
    U8  channel_num;
}sceanrio_mcu2dsp_sync_msg_t;

U32 g_mixer_stream_sample_rate = 96000;
audio_scenario_type_t  dl_running_scenarios[MAX_MIXER_NUM] = {AUDIO_SCENARIO_TYPE_COMMON};
static dchs_dl_running_status_t dchs_dl_running_status = {0};
sceanrio_mcu2dsp_sync_msg_t dl_scenario_msg[MAX_MIXER_NUM] = {0};
ATTR_SHARE_ZIDATA static sceanrio_mcu2dsp_sync_msg_t mcu2dsp_sync_msg;

/********************************************** /
 *                                             *
 *               DCHS UL Variable              * 
 *                                             * 
************************************************/
audio_scenario_type_t dchs_ul_support_scenarios[MAX_SCENARIO_NUM] = {
    AUDIO_SCENARIO_TYPE_HFP_UL,
    AUDIO_SCENARIO_TYPE_BLE_UL,
    AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_OUT,
    AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_OUT,
    AUDIO_SCENARIO_TYPE_COMMON,
    AUDIO_SCENARIO_TYPE_COMMON,
    AUDIO_SCENARIO_TYPE_COMMON,
    AUDIO_SCENARIO_TYPE_COMMON,
    AUDIO_SCENARIO_TYPE_COMMON,
    AUDIO_SCENARIO_TYPE_COMMON
};

static audio_transmitter_config_t transmitter_config_ul;
static audio_dchs_ul_volume_param_t g_ul_volume_ctrl_param;
static audio_transmitter_config_t transmitter_config_dl;
static U32 dchs_ul_running_codec_type  = BT_HFP_CODEC_TYPE_NONE;
static U32 dchs_ul_running_sample_rate = 0;
static audio_scenario_type_t dchs_ul_running_scenario_type = AUDIO_SCENARIO_TYPE_COMMON;
bool latch_timer_start_flag;

/********************************************** /
 *                                             *
 *             DCHS Common Variable            * 
 *                                             * 
************************************************/
#define DUMMPY_CONTENT_TYPE (0xFFFF)

#ifdef AIR_BT_CODEC_BLE_ENABLED
#define BLE_CALL  AUDIO_CONTENT_TYPE_CONVERSATIONAL
#define BLE_MUSIC AUDIO_CONTENT_TYPE_MEDIA
#define BLE_ULL   AUDIO_CONTENT_TYPE_ULL_BLE
#else
#define BLE_CALL  DUMMPY_CONTENT_TYPE
#define BLE_MUSIC DUMMPY_CONTENT_TYPE
#define BLE_ULL   DUMMPY_CONTENT_TYPE
#endif

static bool mux_uart_init_flag = false;
static mux_handle_t audio_cmd_uart_handle = 0;
bt_clock_t g_dchs_anc_target_clk = {0};
uint8_t g_anc_biquad_coef[188];
audio_uart_cmd_ack_type_t g_dchs_cmd_ack_status[MAX_CHIP][AUDIO_UART_COSYS_CMD_MAX] = {0};
bt_sink_srv_am_feature_t feature_param_dl;
bt_sink_srv_am_feature_t feature_param_ul;
bt_sink_srv_am_feature_t feature_param_anc;

ATTR_ZIDATA_IN_NONCACHED_SYSRAM_4BYTE_ALIGN static uint8_t mux_uart_txbuffer[MUX_UART_TXBUF_MAX_SIZE];
ATTR_ZIDATA_IN_NONCACHED_SYSRAM_4BYTE_ALIGN static uint8_t mux_uart_rxbuffer[MUX_UART_RXBUF_MAX_SIZE];

typedef enum
{
    READY_READ,
    WAITING_READ,
}uart_cmd_status_t;

typedef struct
{
    uart_cmd_type_t ctrl_type;
    uint32_t param_size;
    uart_cmd_status_t cmd_status;
}audio_uart_cmd_status_t;

audio_uart_cmd_status_t cur_uart_cmd_status = {AUDIO_UART_COSYS_CMD_MIN, 0 , READY_READ};

/******************************************************************************
 *
 * Private Function Define
 *
 ******************************************************************************/
bool check_support_scenario_type(uart_cmd_type_t ctrl_type, audio_scenario_type_t scenario_type)
{
    if(ctrl_type == AUDIO_UART_COSYS_DL_OPEN || ctrl_type == AUDIO_UART_COSYS_DL_START || ctrl_type == AUDIO_UART_COSYS_DL_CLOSE){
        for(U32 i = 0; i < MAX_SCENARIO_NUM; i++){
            if(dchs_dl_support_scenarios[i] == scenario_type){
                return true;
            }
        }
    } else if(ctrl_type == AUDIO_UART_COSYS_UL_OPEN || ctrl_type == AUDIO_UART_COSYS_UL_START || ctrl_type == AUDIO_UART_COSYS_UL_CLOSE){
        for(U32 i = 0; i < MAX_SCENARIO_NUM; i++){
            if(dchs_ul_support_scenarios[i] == scenario_type){
                return true;
            }
        }
    }
    return false;
}

sceanrio_mcu2dsp_sync_msg_t * get_idle_msg_handle(void)
{
    for(U32 i = 0; i < MAX_MIXER_NUM; i++){
        if(dl_scenario_msg[i].scenario_type == AUDIO_SCENARIO_TYPE_COMMON){
            return &dl_scenario_msg[i];
        }
    }
    return NULL;
}

sceanrio_mcu2dsp_sync_msg_t * get_msg_handle(audio_scenario_type_t scenario_type)
{
    for(U32 i = 0; i < MAX_MIXER_NUM; i++){
        if(dl_scenario_msg[i].scenario_type == scenario_type){
            return &dl_scenario_msg[i];
        }
    }
    return NULL;
}

void remove_msg_handle(audio_scenario_type_t scenario_type)
{
    for(U32 i = 0; i < MAX_MIXER_NUM; i++){
        if(dl_scenario_msg[i].scenario_type == scenario_type){
            memset(&dl_scenario_msg[i], 0, sizeof(sceanrio_mcu2dsp_sync_msg_t));
        }
    }
}

bool check_any_scenario_running(void)
{
    for(U32 i = 0; i < MAX_MIXER_NUM; i ++){
        if(dl_running_scenarios[i] != AUDIO_SCENARIO_TYPE_COMMON){
            DCHS_DL_LOG_I("check scenario:%d exist, i:%d", 2, dl_running_scenarios[i], i);
            return true;
        }
    }
    return false;
}

void add_running_scenario(audio_scenario_type_t scenario_type)
{
    U32 i;
    for(i = 0; i < MAX_MIXER_NUM; i ++){
        if(dl_running_scenarios[i] == AUDIO_SCENARIO_TYPE_COMMON){
            dl_running_scenarios[i] = scenario_type;
            break;
        }
    }
    if(i == MAX_MIXER_NUM){
        AUDIO_ASSERT(0 && "[Mixer Stream] exceed the Max support scenario Num");
    }
}

void remove_running_scenario(audio_scenario_type_t scenario_type)
{
    U32 i;
    for(i = 0; i < MAX_MIXER_NUM; i ++){
        if(dl_running_scenarios[i] == scenario_type){
            dl_running_scenarios[i] = AUDIO_SCENARIO_TYPE_COMMON;
            DCHS_DL_LOG_I("remove scenario:%d, i:%d", 2, scenario_type, i);
            break;
        }
    }
    if(i == MAX_MIXER_NUM){
        AUDIO_ASSERT(0 && "[Mixer Stream] remove scenario is not running");
    }
}

bool check_running_scenario(audio_scenario_type_t scenario_type)
{
    for(U32 i = 0; i < MAX_MIXER_NUM; i ++){
        if(dl_running_scenarios[i] == scenario_type){
            return true;
        }
    }
    return false;
}

audio_scenario_type_t get_ble_scenario_type(U32 context_type)
{
    if(context_type == BLE_MUSIC) {
        return AUDIO_SCENARIO_TYPE_BLE_MUSIC_DL;//LE music
    } else if(context_type == BLE_CALL){
        return AUDIO_SCENARIO_TYPE_BLE_DL;      //LE Call
    } else if(context_type == BLE_ULL) {
        return AUDIO_SCENARIO_TYPE_BLE_ULL_DL;   //ULL DL
    }
    return AUDIO_SCENARIO_TYPE_COMMON;
}

uart_cmd_type_t dchs_query_any_cmd_waiting_ack(dchs_dl_chip_role_t chip_role)
{
    uart_cmd_type_t cmd_type;
    for(cmd_type = AUDIO_UART_COSYS_CMD_MIN; cmd_type < AUDIO_UART_COSYS_CMD_MAX; cmd_type ++){
        if(g_dchs_cmd_ack_status[chip_role][cmd_type] == ACK_WAITING){
            return cmd_type;
        }
    }
    return AUDIO_UART_COSYS_CMD_MIN;
}

audio_uart_cmd_ack_type_t dchs_check_cmd_ack_status(dchs_dl_chip_role_t chip_role, uart_cmd_type_t cmd_type)
{
    return g_dchs_cmd_ack_status[chip_role][cmd_type];
}

void dchs_set_cmd_ack_status(dchs_dl_chip_role_t chip_role, uart_cmd_type_t cmd_type, audio_uart_cmd_ack_type_t ack_type)
{
    g_dchs_cmd_ack_status[chip_role][cmd_type] = ack_type;
    TRANSMITTER_LOG_I("[DCHS] set ack status, cmd:%d,chip_role:%d,ack_type:%d", 3, cmd_type, chip_role,g_dchs_cmd_ack_status[chip_role][cmd_type]);
}

void dchs_loop_waiting_cmd_ack(dchs_dl_chip_role_t chip_role, uart_cmd_type_t cmd_type)
{
    U32 i;
    TRANSMITTER_LOG_I("[DCHS] Waiting cmd:%d,chip_role:%d ack ...", 2, cmd_type, chip_role);
    for (i = 0; ; i++) {
        if(dchs_check_cmd_ack_status(chip_role, cmd_type) == ACK_RECEIVED) {
            TRANSMITTER_LOG_I("[DCHS] get sync ack, cmd:%d, chip_role:%d", 2, cmd_type, chip_role);
            dchs_set_cmd_ack_status(chip_role, cmd_type, ACK_NO);
            break;
        } else if(dchs_check_cmd_ack_status(chip_role, cmd_type) == ACK_WAIT_LATER){
            TRANSMITTER_LOG_I("[DCHS] wait ack later, cmd:%d,chip_role:%d", 2, cmd_type, chip_role);
            break;
        } else if(dchs_check_cmd_ack_status(chip_role, cmd_type) == ACK_NO) {
            TRANSMITTER_LOG_I("[DCHS] already get sync ack, cmd:%d, chip_role:%d", 2, cmd_type, chip_role);
            break;
        }
        if (i == 5000) {
            TRANSMITTER_LOG_E("[DCHS] cmd:%d NO response ack, chip_role:%d", 2, cmd_type, chip_role);
            AUDIO_ASSERT(0);
        }
        vTaskDelay(2 / portTICK_RATE_MS);
    }   
}

void dchs_response_cmd_ack(uart_cmd_type_t cmd_type, audio_uart_cmd_ack_context_t ack_context)
{
    audio_dchs_ack_param_t dchs_ack_param;
    memset(&dchs_ack_param, 0, sizeof(audio_dchs_ack_param_t));
    dchs_ack_param.ctrl_type          = cmd_type;
    dchs_ack_param.ack_context        = ack_context;
    dchs_ack_param.header.ctrl_type   = AUDIO_UART_COSYS_CMD_ACK;
    dchs_ack_param.header.param_size  = sizeof(audio_dchs_ack_param_t) - sizeof(audio_uart_cmd_header_t);
    mcu_uart_tx(UART_AUDIO_CMD, (U8 *)&dchs_ack_param, sizeof(audio_dchs_ack_param_t));
    dchs_set_cmd_ack_status(OTHER_CHIP, cmd_type, ACK_NO);
}

void debug_timer_callback(void *user_data)
{
    DCHS_pka_unlock_bt_sleep();
    TRANSMITTER_LOG_I("bt unlock sleep done", 0);
    if(dchs_dl_running_status.debug_timer_handle){
        hal_gpt_sw_free_timer(dchs_dl_running_status.debug_timer_handle);
        dchs_dl_running_status.debug_timer_handle = 0;
    }
}

void dchs_bt_lock_sleep_done_callback(void *user_data)
{
    DCHS_DL_LOG_I("[DCHS]bt lock sleep done,timeout.",0);
    if((U32)user_data && !dchs_dl_running_status.debug_timer_handle){
        hal_gpt_sw_get_timer(&dchs_dl_running_status.debug_timer_handle);
        hal_gpt_sw_start_timer_ms(dchs_dl_running_status.debug_timer_handle, (U32)user_data * 3, debug_timer_callback, NULL);
        DCHS_DL_LOG_I("trigger bt unlock sleep timer, time_out:%d ms", 1, (U32)user_data * 3);
    }
}

void dchs_dsp2mcu_msg_callback(hal_audio_event_t event, void *data)
{
    DCHS_pka_unlock_bt_sleep();
    bool is_dchs_dl = (bool)data;
    TRANSMITTER_LOG_I("[DCHS]bt unlock sleep done,is dchs dl:%d", 1, is_dchs_dl);
}

void dchs_lock_bt_sleep(bool is_dchs_dl)
{
    if(is_dchs_dl){
        hal_audio_service_hook_callback(AUDIO_MESSAGE_TYPE_DCHS_DL, dchs_dsp2mcu_msg_callback, (void *)is_dchs_dl);
    }else{
        hal_audio_service_hook_callback(AUDIO_MESSAGE_TYPE_DCHS_UL, dchs_dsp2mcu_msg_callback, (void *)is_dchs_dl);
    }
    TRANSMITTER_LOG_I("[DCHS] bt lock sleep, is dl:%d", 1, is_dchs_dl);
    DCHS_pka_lock_bt_sleep(dchs_bt_lock_sleep_done_callback, (void *)is_dchs_dl);
}

void mcu_uart_rx(uart_type_t type, U8 * user_rx_buf,U32 buf_size)
{
    if (type == UART_AUDIO_CMD) {
        if(!audio_cmd_uart_handle){
            TRANSMITTER_LOG_I("[DCHS DL][UART]mcu audio cmd uart don't opened!",0);
            mcu_uart_open(UART_AUDIO_CMD);
    }
    mux_status_t mux_status;
    mux_buffer_t uart_rx_buffer;
    U32 rx_size = 0;
    uart_rx_buffer.p_buf    = user_rx_buf;
    uart_rx_buffer.buf_size = buf_size;
    mux_status = mux_rx(audio_cmd_uart_handle, &uart_rx_buffer, &rx_size);
    if (mux_status != MUX_STATUS_OK) {
        TRANSMITTER_LOG_E("[DCHS DL][uart callback] mux uart rx fail,status=%d", 1, mux_status);
        AUDIO_ASSERT(0);
    }
    if(rx_size != buf_size){
        TRANSMITTER_LOG_E("[DCHS DL][uart callback] mux uart rx fail,already_rx_size=%d,need_rx_size=%d", 2, rx_size, buf_size);
        AUDIO_ASSERT(0);
    }
    TRANSMITTER_LOG_I("[DCHS DL][uart callback] audio cmd rx data size: %d", 1, rx_size);
    } else if (type == UART_UL) {
        /* todo */
    } else {
        TRANSMITTER_LOG_E("[DCHS DL][MUX UART] don't support uart type = %d", 1, type);
    }
}

uint32_t query_uart_buf_data_size()
{
    if(!audio_cmd_uart_handle){
        TRANSMITTER_LOG_I("[DCHS DL][UART]mcu audio cmd uart don't opened!",0);
        assert(0);
    }
    mux_ctrl_para_t rx_param;
    mux_user_control(audio_cmd_uart_handle,MUX_CMD_GET_LL_USER_RX_BUFFER_DATA_SIZE,&rx_param);
    return rx_param.mux_ll_user_rx_data_len;
}

void reset_cur_cmd_status(uart_cmd_type_t cmd_type)
{
    //reset cur cmd status
    cur_uart_cmd_status.cmd_status = READY_READ;
    cur_uart_cmd_status.ctrl_type  = AUDIO_UART_COSYS_CMD_MIN;
    cur_uart_cmd_status.param_size = 0;
}

ATTR_TEXT_IN_TCM void mcu_mux_uart_callback(mux_handle_t handle, mux_event_t event, uint32_t data_len, void *user_data)
{
    switch (event) {
        case MUX_EVENT_READY_TO_READ:
            if(handle == audio_cmd_uart_handle){
                while (query_uart_buf_data_size())
                {
                    if (cur_uart_cmd_status.cmd_status == READY_READ){
                        uint32_t uart_data_size =  query_uart_buf_data_size();
                        if(uart_data_size >= sizeof(audio_uart_cmd_header_t)){
                            audio_uart_cmd_header_t cmd_header;
                            mcu_uart_rx(UART_AUDIO_CMD, (U8 *)&cmd_header, sizeof(audio_uart_cmd_header_t));
                            cur_uart_cmd_status.ctrl_type  = cmd_header.ctrl_type;
                            cur_uart_cmd_status.param_size = cmd_header.param_size;
                            if(cur_uart_cmd_status.ctrl_type <= AUDIO_UART_COSYS_CMD_MIN || cur_uart_cmd_status.ctrl_type >= AUDIO_UART_COSYS_CMD_MAX){
                                DCHS_DL_LOG_E("[uart callback]  rx invalid header, param_size = %d", 1, cur_uart_cmd_status.param_size);
                                AUDIO_ASSERT(0);
                                return;
                            }
                            if(cmd_header.ctrl_type != AUDIO_UART_COSYS_CMD_ACK){
                                dchs_set_cmd_ack_status(OTHER_CHIP, cmd_header.ctrl_type, cmd_header.ack_type);
                            }
                            cur_uart_cmd_status.cmd_status = WAITING_READ;
                        }else{
                            break;//end while
                        }
                    }
                    if (cur_uart_cmd_status.cmd_status == WAITING_READ){
                        uint32_t uart_data_size =  query_uart_buf_data_size();
                        if(uart_data_size >= cur_uart_cmd_status.param_size){
                            bt_sink_srv_am_feature_t * feature_param = NULL;
                            uint8_t header_size  = sizeof(audio_uart_cmd_header_t);

                            if(cur_uart_cmd_status.ctrl_type == AUDIO_UART_COSYS_DL_OPEN){
                                feature_param = &feature_param_dl;
                                audio_dchs_dl_open_param_t dl_open_ctrl_param;
                                mcu_uart_rx(UART_AUDIO_CMD, (uint8_t*)&dl_open_ctrl_param + header_size, sizeof(audio_dchs_dl_open_param_t) - header_size);
                                audio_scenario_type_t scenario_type = dl_open_ctrl_param.scenario_type;
                                //save scneario msg
                                if(dchs_get_device_mode() == DCHS_MODE_RIGHT){ 
                                    sceanrio_mcu2dsp_sync_msg_t * sync_msg = get_idle_msg_handle();
                                    sync_msg->sync_type     = SYNC_SCENARIO_OPEN;
                                    sync_msg->scenario_type = scenario_type;
                                    sync_msg->memory_agent  = HAL_AUDIO_MEM_SUB;
                                    sync_msg->format_bytes  = dl_open_ctrl_param.format_bytes;
                                    sync_msg->sample_rate   = g_mixer_stream_sample_rate;
                                }
                                if(check_any_scenario_running()){
                                    add_running_scenario(scenario_type);
                                    DCHS_DL_LOG_I("dchs dl alreay running, ignore open, scenario_type:%d", 1, scenario_type);
                                    reset_cur_cmd_status(cur_uart_cmd_status.ctrl_type);
                                    //response ack if need
                                    if(dchs_check_cmd_ack_status(OTHER_CHIP, cur_uart_cmd_status.ctrl_type) == ACK_LATER){
                                        dchs_response_cmd_ack(cur_uart_cmd_status.ctrl_type, SYNC_SUCCESS);
                                    }
                                    continue;
                                }
                                dchs_dl_running_status.is_running    = true;
                                dchs_dl_running_status.scenario_type = scenario_type;
                                add_running_scenario(scenario_type);

                                feature_param->type_mask = AM_UART_COSYS_CONTROL_DL;
                                feature_param->feature_param.cosys_ctrl_dl.ctrl_param.dchs_dl_open_param = dl_open_ctrl_param;
                                feature_param->feature_param.cosys_ctrl_dl.chip_role = OTHER_CHIP;
                                feature_param->feature_param.cosys_ctrl_dl.ctrl_type = AUDIO_UART_COSYS_DL_OPEN;
                                DCHS_DL_LOG_I("[rx cmd] chip role=%d,scenario_type=%d, dchs dl sample rate=%d,ctrl type = DL_OPEN,", 3, OTHER_CHIP, dl_open_ctrl_param.scenario_type,g_mixer_stream_sample_rate);
                            }else if (cur_uart_cmd_status.ctrl_type == AUDIO_UART_COSYS_DL_START){
                                feature_param = &feature_param_dl;
                                audio_dchs_dl_start_param_t dl_start_ctrl_param;
                                mcu_uart_rx(UART_AUDIO_CMD, (uint8_t*)&dl_start_ctrl_param + header_size, sizeof(audio_dchs_dl_start_param_t) - header_size);
                                feature_param->type_mask = AM_UART_COSYS_CONTROL_DL;
                                feature_param->feature_param.cosys_ctrl_dl.ctrl_param.dchs_dl_start_param = dl_start_ctrl_param;
                                feature_param->feature_param.cosys_ctrl_dl.chip_role = OTHER_CHIP;
                                feature_param->feature_param.cosys_ctrl_dl.ctrl_type = AUDIO_UART_COSYS_DL_START;
                                DCHS_DL_LOG_I("[rx cmd] scenario_type=%d, ctrl type = DL_START", 1 , dl_start_ctrl_param.scenario_type);
                            }else if(cur_uart_cmd_status.ctrl_type == AUDIO_UART_COSYS_DL_CLOSE){
                                feature_param = &feature_param_dl;
                                audio_dchs_dl_close_param_t dl_close_ctrl_param;
                                mcu_uart_rx(UART_AUDIO_CMD, (uint8_t*)&dl_close_ctrl_param + header_size, sizeof(audio_dchs_dl_close_param_t) - header_size);
                                //dchs_dl_set_scenario_exist_flag(OTHER_CHIP, dl_close_ctrl_param.scenario_type, false);
                                DCHS_DL_LOG_I("[rx cmd] scenario_type=%d,ctrl type = DL_CLOSE", 1 , dl_close_ctrl_param.scenario_type);
                                feature_param->type_mask = AM_UART_COSYS_CONTROL_DL;
                                feature_param->feature_param.cosys_ctrl_dl.ctrl_param.dchs_dl_close_param = dl_close_ctrl_param;
                                feature_param->feature_param.cosys_ctrl_dl.chip_role = OTHER_CHIP;
                                feature_param->feature_param.cosys_ctrl_dl.ctrl_type = AUDIO_UART_COSYS_DL_CLOSE;
                            }else if(cur_uart_cmd_status.ctrl_type == AUDIO_UART_COSYS_DL_SET_GAIN){
                                feature_param = &feature_param_dl;
                                audio_dchs_dl_gain_param_t dl_gain_ctrl_param;
                                mcu_uart_rx(UART_AUDIO_CMD, (uint8_t*)&dl_gain_ctrl_param + header_size, sizeof(audio_dchs_dl_gain_param_t) - header_size);
                                feature_param->type_mask = AM_UART_COSYS_CONTROL_DL;
                                feature_param->feature_param.cosys_ctrl_dl.ctrl_param.dchs_dl_gain_param = dl_gain_ctrl_param;
                                feature_param->feature_param.cosys_ctrl_dl.ctrl_type = AUDIO_UART_COSYS_DL_SET_GAIN;
                                DCHS_DL_LOG_I("[rx cmd] get gain:0x%x,operation:%d", 2 , dl_gain_ctrl_param.vol_gain, dl_gain_ctrl_param.operation);
                            }else if(cur_uart_cmd_status.ctrl_type == AUDIO_UART_COSYS_UL_OPEN){
                                feature_param = &feature_param_ul;
                                audio_dchs_ul_open_param_t ul_open_ctrl_param;
                                mcu_uart_rx(UART_AUDIO_CMD, (uint8_t*)&ul_open_ctrl_param + header_size, sizeof(audio_dchs_ul_open_param_t) - header_size);
                                feature_param->type_mask = AM_UART_COSYS_CONTROL_UL;
                                feature_param->feature_param.cosys_ctrl_ul.ctrl_param.dchs_ul_open_param = ul_open_ctrl_param;
                                feature_param->feature_param.cosys_ctrl_ul.ctrl_type = AUDIO_UART_COSYS_UL_OPEN;
                                feature_param->feature_param.cosys_ctrl_ul.chip_role = OTHER_CHIP;
                                if((ul_open_ctrl_param.scenario_type == AUDIO_SCENARIO_TYPE_BLE_UL)|| (ul_open_ctrl_param.scenario_type == AUDIO_SCENARIO_TYPE_HFP_UL)){
                                    g_ul_volume_ctrl_param.codec = BT_HFP_CODEC_TYPE_MSBC;
                                    g_ul_volume_ctrl_param.dev_in = HAL_AUDIO_DEVICE_MAIN_MIC;
                                    g_ul_volume_ctrl_param.dev_out = HAL_AUDIO_DEVICE_HEADSET;
                                    g_ul_volume_ctrl_param.lev_in = 12;
                                    g_ul_volume_ctrl_param.lev_out = 12;
                                }
                                TRANSMITTER_LOG_I("[DCHS UL][rx cmd] scenario_type=%d, ctrl type = UL_OPEN,", 1 , ul_open_ctrl_param.scenario_type);
                            }else if (cur_uart_cmd_status.ctrl_type == AUDIO_UART_COSYS_UL_START){
                                feature_param = &feature_param_ul;
                                audio_dchs_ul_start_param_t ul_start_ctrl_param;
                                mcu_uart_rx(UART_AUDIO_CMD, (uint8_t*)&ul_start_ctrl_param + header_size, sizeof(audio_dchs_ul_start_param_t) - header_size);
                                TRANSMITTER_LOG_I("[DCHS UL][rx cmd] scenario_type=%d, ctrl type = UL_START,", 1 , ul_start_ctrl_param.scenario_type);
                                feature_param->type_mask = AM_UART_COSYS_CONTROL_UL;
                                feature_param->feature_param.cosys_ctrl_ul.ctrl_param.dchs_ul_start_param = ul_start_ctrl_param;
                                feature_param->feature_param.cosys_ctrl_ul.ctrl_type = AUDIO_UART_COSYS_UL_START;
                            }else if(cur_uart_cmd_status.ctrl_type == AUDIO_UART_COSYS_UL_CLOSE){
                                feature_param = &feature_param_ul;
                                audio_dchs_ul_close_param_t ul_close_ctrl_param;
                                mcu_uart_rx(UART_AUDIO_CMD, (uint8_t*)&ul_close_ctrl_param + header_size, sizeof(audio_dchs_ul_close_param_t) - header_size);
                                TRANSMITTER_LOG_I("[DCHS UL][rx cmd] scenario_type=%d, ctrl type = UL_CLOSE,", 1 , ul_close_ctrl_param.scenario_type);
                                feature_param->type_mask = AM_UART_COSYS_CONTROL_UL;
                                feature_param->feature_param.cosys_ctrl_ul.ctrl_param.dchs_ul_close_param = ul_close_ctrl_param;
                                feature_param->feature_param.cosys_ctrl_ul.ctrl_type = AUDIO_UART_COSYS_UL_CLOSE;
                            }else if(cur_uart_cmd_status.ctrl_type == AUDIO_UART_COSYS_UL_VOLUME){
                                memset(&g_ul_volume_ctrl_param,0,sizeof(audio_dchs_ul_volume_param_t));
                                mcu_uart_rx(UART_AUDIO_CMD, (uint8_t*)&g_ul_volume_ctrl_param + header_size, sizeof(audio_dchs_ul_volume_param_t) - header_size);
                                TRANSMITTER_LOG_I("[DCHS UL][rx cmd] scenario_type=%d, ctrl type = UL_VOLUME,", 1 , g_ul_volume_ctrl_param.scenario_type);
                                reset_cur_cmd_status(cur_uart_cmd_status.ctrl_type);
                                continue;
                            } else if (cur_uart_cmd_status.ctrl_type == AUDIO_UART_COSYS_RACE_CMD) {
                                feature_param = &feature_param_anc;
                                audio_dchs_relay_cmd_param_t calibration_param;
                                mcu_uart_rx(UART_AUDIO_CMD, (uint8_t*)&calibration_param + header_size, sizeof(audio_dchs_relay_cmd_param_t) - header_size);
                                TRANSMITTER_LOG_I("[DCHS][rx cmd] relay race process",0);

                                if (calibration_param.is_ack_cmd) {
                                    /* Receive response from Follower*/
                                    uint32_t send_length;
                                    //Send result to cmd source
                                    send_length = race_port_send_data(race_get_port_handle_by_channel_id(calibration_param.channel_id), (uint8_t *)calibration_param.payload, calibration_param.payload_length);
                                    assert(send_length);
                                    reset_cur_cmd_status(cur_uart_cmd_status.ctrl_type);
                                    continue;
                                } else {
                                    /* Receive relay CMD from Receiver */
                                    feature_param->type_mask = AM_DCHS;
                                    feature_param->feature_param.dchs_param.ctrl_param.dchs_relay_cmd_param = calibration_param;
                                    feature_param->feature_param.cosys_ctrl_ul.ctrl_type = AUDIO_UART_COSYS_RACE_CMD;
                                }

                            } else if (cur_uart_cmd_status.ctrl_type == AUDIO_UART_COSYS_ANC_CTRL) {
                                feature_param = &feature_param_anc;
                                audio_dchs_anc_param_t *anc_ctrl_param;

                                anc_ctrl_param = pvPortMalloc(sizeof(audio_dchs_anc_param_t));
                                mcu_uart_rx(UART_AUDIO_CMD, (uint8_t*)anc_ctrl_param + header_size, sizeof(audio_dchs_anc_param_t) - header_size);
                                feature_param->type_mask = AM_UART_COSYS_CONTROL_ANC;
                                feature_param->feature_param.anc_param.event = anc_ctrl_param->event_id;
                                memcpy(&feature_param->feature_param.anc_param.cap, &anc_ctrl_param->anc_param.cap, sizeof(audio_anc_control_cap_t));
                                //feature_param.feature_param.anc_param.cap = anc_ctrl_param->anc_param.cap;
                                g_dchs_anc_target_clk = anc_ctrl_param->target_clk;

                                for(uint32_t i = 0; i < 188; i++ ) {
                                    g_anc_biquad_coef[i] = anc_ctrl_param->coef[i];
                                }
                                feature_param->feature_param.anc_param.cap.filter_cap.filter_coef = (void *)&g_anc_biquad_coef;

                                TRANSMITTER_LOG_I("[DCHS ANC][rx cmd] type mask %d, event %d Fevent %d", 3, feature_param->type_mask,
                                                                                                            anc_ctrl_param->event_id,
                                                                                                            feature_param->feature_param.anc_param.event);
                                vPortFree(anc_ctrl_param);
                            } else if(cur_uart_cmd_status.ctrl_type == AUDIO_UART_COSYS_DL_SAMPLE_RATE_SYNC) {
                                audio_dchs_dl_sample_rate_param_t dchs_dl_sample_rate_sync;
                                mcu_uart_rx(UART_AUDIO_CMD, (uint8_t*)&dchs_dl_sample_rate_sync + header_size, sizeof(audio_dchs_dl_sample_rate_param_t) - header_size);
                                g_mixer_stream_sample_rate = dchs_dl_sample_rate_sync.dchs_dl_sample_rate;
                                TRANSMITTER_LOG_I("[DCHS][rx cmd] get dchs dl sample rate sync:%d", 1, g_mixer_stream_sample_rate);
                                reset_cur_cmd_status(cur_uart_cmd_status.ctrl_type);
                                continue;
                            } else if(cur_uart_cmd_status.ctrl_type == AUDIO_UART_COSYS_CMD_ACK) {
                                audio_dchs_ack_param_t dchs_cmd_ack;
                                mcu_uart_rx(UART_AUDIO_CMD, (uint8_t*)&dchs_cmd_ack + header_size, sizeof(audio_dchs_ack_param_t) - header_size);
                                if(dchs_cmd_ack.ack_context == SYNC_SUCCESS){
                                    dchs_set_cmd_ack_status(LOCAL_CHIP, dchs_cmd_ack.ctrl_type, ACK_RECEIVED);
                                }
                                TRANSMITTER_LOG_I("[DCHS][rx cmd] get dchs cmd:%d, ack:%d", 2, dchs_cmd_ack.ctrl_type, dchs_cmd_ack.ack_context);
                                reset_cur_cmd_status(cur_uart_cmd_status.ctrl_type);
                                continue;
                            } else if(cur_uart_cmd_status.ctrl_type == AUDIO_UART_COSYS_DETACHABLE_MIC) {
                                feature_param = &feature_param_dl;
                                audio_dchs_detachable_mic_param_t dchs_detachable_mic;
                                mcu_uart_rx(UART_AUDIO_CMD, (uint8_t*)&dchs_detachable_mic + header_size, sizeof(audio_dchs_detachable_mic_param_t) - header_size);
                                U32 voice_mic_type = dchs_detachable_mic.voice_mic_type;
                                feature_param->type_mask = AM_UART_COSYS_CONTROL_DL;
                                feature_param->feature_param.cosys_ctrl_dl.ctrl_param.dchs_detachable_mic_param.voice_mic_type = voice_mic_type;
                                feature_param->feature_param.cosys_ctrl_dl.ctrl_type = AUDIO_UART_COSYS_DETACHABLE_MIC;
                                TRANSMITTER_LOG_I("[DCHS][rx cmd] get dchs detachable mic sync, mic type:%d", 1, voice_mic_type);
                            } else if(cur_uart_cmd_status.ctrl_type == AUDIO_UART_COSYS_UL_MUTE) {
                                feature_param = &feature_param_ul;
                                audio_set_mute_param_t audio_set_mute_param;
                                mcu_uart_rx(UART_AUDIO_CMD, (uint8_t*)&audio_set_mute_param + header_size, sizeof(audio_set_mute_param_t) - header_size);
                                TRANSMITTER_LOG_I("[DCHS UL][rx cmd] Set Mute =%d", 1, audio_set_mute_param.mute);
                                feature_param->type_mask = AM_UART_COSYS_CONTROL_UL;
                                feature_param->feature_param.cosys_ctrl_ul.ctrl_param.audio_set_mute_param = audio_set_mute_param;
                                feature_param->feature_param.cosys_ctrl_ul.ctrl_type = AUDIO_UART_COSYS_UL_MUTE;
                            } else if(cur_uart_cmd_status.ctrl_type == AUDIO_UART_COSYS_UL_SW_GAIN) {
                                feature_param = &feature_param_ul;
                                audio_set_sw_gain_param_t audio_set_sw_gain_param;
                                mcu_uart_rx(UART_AUDIO_CMD, (uint8_t*)&audio_set_sw_gain_param + header_size, sizeof(audio_set_sw_gain_param_t) - header_size);
                                TRANSMITTER_LOG_I("[DCHS UL][rx cmd] Set sw gain ", 0);
                                feature_param->type_mask = AM_UART_COSYS_CONTROL_UL;
                                feature_param->feature_param.cosys_ctrl_ul.ctrl_param.audio_set_sw_gain_param = audio_set_sw_gain_param;
                                feature_param->feature_param.cosys_ctrl_ul.ctrl_type = AUDIO_UART_COSYS_UL_SW_GAIN;
                            }
                            /***************
                             *  extend here
                             * ************/
                            else{
                                AUDIO_ASSERT(0 && "[DCHS][rx cmd] invalid ctrl type");
                            }
                            //send to am queue
                            am_audio_set_feature_ISR(FEATURE_NO_NEED_ID, feature_param);
                            //send to am task to do flow from irq
                            uart_cmd_type_t cmd_type = dchs_query_any_cmd_waiting_ack(LOCAL_CHIP);
                            //fix deadlock
                            if(cmd_type && dchs_check_cmd_ack_status(OTHER_CHIP, cur_uart_cmd_status.ctrl_type) == ACK_LATER){ //if check any cmd waiting ack, send this waiting action msg to AM queue front
                                dchs_set_cmd_ack_status(LOCAL_CHIP, cmd_type, ACK_WAIT_LATER);
                                bt_sink_srv_am_feature_t ack_param;
                                memset(&ack_param, 0, sizeof(bt_sink_srv_am_feature_t));
                                ack_param.type_mask = AM_UART_COSYS_CONTROL_DL;
                                ack_param.feature_param.cosys_ctrl_dl.ctrl_param.dchs_wait_ack_param.cmd_type = cmd_type;
                                ack_param.feature_param.cosys_ctrl_dl.ctrl_type = AUDIO_UART_COSYS_CMD_WAITING_ACK;
                                am_audio_set_feature_ISR(FEATURE_NO_NEED_ID, &ack_param);
                                TRANSMITTER_LOG_I("[DCHS] send waiting ack and cur cmd execute to AM queue front, wait ack cmd type:%d, cur execute cmd type:%d", 2, cmd_type, cur_uart_cmd_status.ctrl_type);
                            }
                            //reset cur cmd status
                            reset_cur_cmd_status(cur_uart_cmd_status.ctrl_type);
                        } else {
                            break;//end while
                        }
                    }
                }//while()
            }
            break;
        case MUX_EVENT_READY_TO_WRITE:
            TRANSMITTER_LOG_I("[DCHS DL][uart callback] MUX_EVENT_READY_TO_WRITE", 0);
            break;
        default:
            TRANSMITTER_LOG_I("[DCHS DL][uart callback] no event: %d", 1, event);
    }
}

/******************************************************************************
 *
 * Public Function Define
 *
 ******************************************************************************/
/*------------------------------------------------PORT----AIR_DCHS_ENABLE------------------------------------------------------------------*/

void dchs_check_ul_running_scenario(audio_scenario_type_t *scenario_type, U32 *codec_type, U32 *sample_rate)
{
    *scenario_type = dchs_ul_running_scenario_type;
    *codec_type    = dchs_ul_running_codec_type;
    *sample_rate   = dchs_ul_running_sample_rate;
}

void dchs_uart_detachable_mic_sync(U32 voice_mic_type, audio_uart_sync_status_t sync_status)
{
    uart_cmd_type_t ctrl_type = AUDIO_UART_COSYS_DETACHABLE_MIC;
    TRANSMITTER_LOG_I("[DCHS] detachable mic local ack status:%d,other chip ack tatus:%d", 2, dchs_check_cmd_ack_status(LOCAL_CHIP, ctrl_type), dchs_check_cmd_ack_status(OTHER_CHIP, ctrl_type));
    if (sync_status == SYNC_START && dchs_check_cmd_ack_status(LOCAL_CHIP, ctrl_type) == ACK_NO && dchs_check_cmd_ack_status(OTHER_CHIP, ctrl_type) == ACK_NO) { //send side
        audio_dchs_detachable_mic_param_t dchs_detachable_mic_param;
        dchs_detachable_mic_param.voice_mic_type    = voice_mic_type;
        dchs_detachable_mic_param.header.ctrl_type  = ctrl_type;
        dchs_detachable_mic_param.header.ack_type   = ACK_LATER;
        dchs_detachable_mic_param.header.param_size = sizeof(audio_dchs_detachable_mic_param_t) - sizeof(audio_uart_cmd_header_t);
        dchs_set_cmd_ack_status(LOCAL_CHIP, ctrl_type, ACK_WAITING);
        mcu_uart_tx(UART_AUDIO_CMD, (U8 *)&dchs_detachable_mic_param, sizeof(audio_dchs_detachable_mic_param_t));
        TRANSMITTER_LOG_I("[MCU][DCHS] cmd tx detachable mic sync, mic type:%d, ack_type:%d", 2, voice_mic_type, dchs_check_cmd_ack_status(LOCAL_CHIP, ctrl_type));
    } else if (sync_status == SYNC_END && (dchs_check_cmd_ack_status(LOCAL_CHIP, ctrl_type) == ACK_WAITING || dchs_check_cmd_ack_status(LOCAL_CHIP, ctrl_type) == ACK_RECEIVED)) { //send side
        //change igo params
        if(dchs_ul_running_codec_type != BLE_ULL){//not ULL2.0 send
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_STREAM_DEINIT, 1, 0, false);
        }
        dchs_loop_waiting_cmd_ack(LOCAL_CHIP, ctrl_type);
    } else if (sync_status == SYNC_END && dchs_check_cmd_ack_status(OTHER_CHIP, ctrl_type) == ACK_LATER) { //recv side
        //change igo params
        if(dchs_ul_running_codec_type != BLE_ULL){//not ULL2.0 send
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_STREAM_DEINIT, 1, 0, false);
        }
        dchs_response_cmd_ack(ctrl_type, SYNC_SUCCESS);
        TRANSMITTER_LOG_I("[MCU][DCHS] detachable mic send ack, mic type:%d, ack_type:%d", 2, voice_mic_type, dchs_check_cmd_ack_status(OTHER_CHIP, ctrl_type));
    }
}

void dchs_dl_set_audio_sample_rate(uint32_t sample_rate)
{
    g_mixer_stream_sample_rate = sample_rate;
    TRANSMITTER_LOG_I("[MCU][DCHS DL] ull host set dchs dl sample rate:%d", 1, g_mixer_stream_sample_rate);
    audio_dchs_dl_sample_rate_param_t dchs_dl_sample_rate_sync_param;
    dchs_dl_sample_rate_sync_param.dchs_dl_sample_rate = sample_rate;
    dchs_dl_sample_rate_sync_param.header.ctrl_type  = AUDIO_UART_COSYS_DL_SAMPLE_RATE_SYNC;
    dchs_dl_sample_rate_sync_param.header.param_size = sizeof(audio_dchs_dl_sample_rate_param_t) - sizeof(audio_uart_cmd_header_t);
    mcu_uart_tx(UART_AUDIO_CMD, (U8 *)&dchs_dl_sample_rate_sync_param, sizeof(audio_dchs_dl_sample_rate_param_t));
}

void dchs_ul_replace_feature_nvkey(audio_scenario_type_t scenario_type, U32 codec_type, U32 sample_rate)
{
    sysram_status_t status;
    DSP_FEATURE_TYPE_LIST AudioFeatureList_DCHS[2];
    AudioFeatureList_DCHS[1] = FUNC_END;
    if(sample_rate == 16000){
        AudioFeatureList_DCHS[0] = FUNC_RX_NR;
        if(scenario_type == AUDIO_SCENARIO_TYPE_HFP_UL){
            AudioFeatureList_DCHS[0] = (codec_type == BT_HFP_CODEC_TYPE_CVSD ? FUNC_TX_NR : FUNC_RX_NR); 
        }
    } else {
        AudioFeatureList_DCHS[0] = FUNC_TX_NR_v2;
    }
    #ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
    voice_mic_type_t mic_cur_type  = hal_audio_query_voice_mic_type();
    if (mic_cur_type == VOICE_MIC_TYPE_DETACHABLE) {
        if(sample_rate == 16000){
            AudioFeatureList_DCHS[0] = FUNC_WB_BOOM_MIC;
            if(scenario_type == AUDIO_SCENARIO_TYPE_HFP_UL){
                AudioFeatureList_DCHS[0] = (codec_type == BT_HFP_CODEC_TYPE_CVSD ? FUNC_NB_BOOM_MIC : FUNC_WB_BOOM_MIC);
            }
        } else {
            #ifdef AIR_BT_ULL_SWB_ENABLE
            AudioFeatureList_DCHS[0] = FUNC_SWB_BOOM_MIC;
            #else
            AudioFeatureList_DCHS[0] = FUNC_WB_BOOM_MIC;
            #endif
        }
        TRANSMITTER_LOG_I("[MCU][DCHS UL] boom mic nvkey update done, scenario type:%d,codec_type:0x%x", 2, scenario_type, codec_type);
    } 
    #endif
    audio_nvdm_reset_sysram();
    status = audio_nvdm_set_feature(2, AudioFeatureList_DCHS);
    if (status != NVDM_STATUS_NAT_OK) {
        TRANSMITTER_LOG_E("[DCHS UL]failed to set parameters to share memory - err(%d)\r\n", 1, status);
        AUDIO_ASSERT(0);
    }
}

void dchs_ul_audio_set_mute(bool mute)
{
    TRANSMITTER_LOG_I("[MCU][DCHS UL] relay set mute [%d]", 1, mute);
    audio_set_mute_param_t audio_set_mute_param;
    audio_set_mute_param.mute = mute;
    audio_set_mute_param.header.ctrl_type  = AUDIO_UART_COSYS_UL_MUTE;
    audio_set_mute_param.header.param_size = sizeof(audio_set_mute_param_t) - sizeof(audio_uart_cmd_header_t);
    mcu_uart_tx(UART_AUDIO_CMD, (U8 *)&audio_set_mute_param, sizeof(audio_set_mute_param_t));
}

void dchs_ul_audio_set_sw_gain(mcu2dsp_audio_transmitter_runtime_config_param_t * runtime_config_param,uint32_t size)
{
    audio_set_sw_gain_param_t dchs_ul_swgain_param;
    dchs_ul_swgain_param.config_operation = runtime_config_param->config_operation;
    memcpy(dchs_ul_swgain_param.config_param, runtime_config_param->config_param, size);
    TRANSMITTER_LOG_I("[MCU][DCHS UL] relay set sw gain", 0);
    dchs_ul_swgain_param.header.ctrl_type  = AUDIO_UART_COSYS_UL_SW_GAIN;
    dchs_ul_swgain_param.header.param_size = sizeof(audio_set_sw_gain_param_t) - sizeof(audio_uart_cmd_header_t);
    mcu_uart_tx(UART_AUDIO_CMD, (U8 *)&dchs_ul_swgain_param, sizeof(audio_set_sw_gain_param_t));
}

void audio_transmitter_dchs_open_playback(audio_transmitter_config_t *config, mcu2dsp_open_param_t *open_param)
{
    if(config->scenario_sub_id == AUDIO_TRANSMITTER_DCHS_UART_DL){
        open_param->param.stream_in     = STREAM_IN_MIXER;
        open_param->param.stream_out    = STREAM_OUT_AFE;
        open_param->audio_scenario_type = AUDIO_SCENARIO_TYPE_MIXER_STREAM;
        //stream in
        open_param->stream_in_param.afe.format                = HAL_AUDIO_PCM_FORMAT_S32_LE;
        open_param->stream_in_param.afe.frame_size            = AUDIO_DURATION_TIME * g_mixer_stream_sample_rate / MUX_UART_BUF_SLICE / 1000;
        open_param->stream_in_param.afe.sampling_rate         = g_mixer_stream_sample_rate;
        open_param->stream_in_param.data_dl.scenario_type     = config->scenario_config.dchs_config.scenario_type;
        //stream out
        hal_audio_get_stream_out_setting_config(AU_DSP_VOICE, &open_param->stream_out_param);
        open_param->stream_out_param.afe.audio_device    = HAL_AUDIO_DEVICE_DAC_DUAL;
        open_param->stream_out_param.afe.stream_channel  = HAL_AUDIO_DIRECT;
        open_param->stream_out_param.afe.memory          = HAL_AUDIO_MEM4;//(config->scenario_config.dchs_config.out_memory == HAL_AUDIO_MEM1) ? HAL_AUDIO_MEM3 : HAL_AUDIO_MEM1;
        open_param->stream_out_param.afe.format          = HAL_AUDIO_PCM_FORMAT_S32_LE;
        open_param->stream_out_param.afe.sampling_rate   = g_mixer_stream_sample_rate;//48k or 96k
        open_param->stream_out_param.afe.irq_period      = 0;//config->scenario_config.dchs_config.irq_period;
        open_param->stream_out_param.afe.frame_size      = AUDIO_DURATION_TIME * g_mixer_stream_sample_rate / MUX_UART_BUF_SLICE / 1000;//open_param->stream_in_param.afe.frame_size;
        open_param->stream_out_param.afe.frame_number    = 3 * MUX_UART_BUF_SLICE;
        open_param->stream_out_param.afe.hw_gain         = false;
        DCHS_DL_LOG_I("[MCU] dchs dl data sceanrio type: %d open", 1, config->scenario_config.dchs_config.scenario_type);
    }
    if(config->scenario_sub_id == AUDIO_TRANSMITTER_DCHS_UART_UL)
    {
        dchs_ul_replace_feature_nvkey(config->scenario_config.dchs_config.scenario_type, 
                                        config->scenario_config.dchs_config.codec_type, 
                                            config->scenario_config.dchs_config.sampling_rate);
        hal_dvfs_lock_control(HFP_DVFS_INHOUSE, HAL_DVFS_LOCK);
        memset(open_param, 0, sizeof(mcu2dsp_open_param_t));
        hal_audio_get_stream_in_setting_config(AU_DSP_VOICE, &open_param->stream_in_param);
        open_param->param.stream_in = STREAM_IN_AFE;
        open_param->param.stream_out = STREAM_OUT_VIRTUAL;
        open_param->stream_in_param.afe.memory          = HAL_AUDIO_MEM1 | HAL_AUDIO_MEM3;//HAL_AUDIO_MEM3 to enable echo referencr;
        open_param->audio_scenario_type = AUDIO_SCENARIO_TYPE_DCHS_UART_UL;
        open_param->stream_in_param.afe.format          = config->scenario_config.dchs_config.format;
        open_param->stream_in_param.afe.sampling_rate   = config->scenario_config.dchs_config.sampling_rate;//16 or 32k
        open_param->stream_in_param.afe.irq_period      = 0;
        open_param->stream_in_param.afe.frame_size      = config->scenario_config.dchs_config.frame_size;
        open_param->stream_in_param.afe.frame_number    = config->scenario_config.dchs_config.frame_number;
        open_param->stream_in_param.afe.hw_gain         = false;
        if((config->scenario_config.dchs_config.scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_OUT) || (config->scenario_config.dchs_config.scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_OUT)){
            //open_param->stream_in_param.afe.dchs_ul_scenario_type   = config->scenario_config.dchs_config.scenario_type;
        }
        bt_sink_srv_audio_setting_vol_info_t vol_info = {0};
        vol_info.type = VOL_HFP;
        vol_info.vol_info.hfp_vol_info.codec = g_ul_volume_ctrl_param.codec;
        vol_info.vol_info.hfp_vol_info.dev_in = g_ul_volume_ctrl_param.dev_in;
        vol_info.vol_info.hfp_vol_info.dev_out = g_ul_volume_ctrl_param.dev_out;
        vol_info.vol_info.hfp_vol_info.lev_in = g_ul_volume_ctrl_param.lev_in;
        vol_info.vol_info.hfp_vol_info.lev_out = g_ul_volume_ctrl_param.lev_out;
        bt_sink_srv_am_set_volume(STREAM_IN, &vol_info);
        TRANSMITTER_LOG_I("[MCU][DCHS UL] dchs ul scenario_sub_id %d open, source memory=%d,scenario_type:%d", 3, config->scenario_sub_id,open_param->stream_in_param.afe.memory,config->scenario_config.dchs_config.scenario_type);
    }
}

void audio_transmitter_dchs_start_playback(audio_transmitter_config_t *config, mcu2dsp_start_param_t *start_param)
{
    if(config->scenario_sub_id == AUDIO_TRANSMITTER_DCHS_UART_DL){
        start_param->param.stream_in  = STREAM_IN_MIXER;
        start_param->param.stream_out = STREAM_OUT_AFE;
        memset((void *)&start_param->stream_in_param,  0, sizeof(mcu2dsp_start_stream_in_param_t));
        memset((void *)&start_param->stream_out_param, 0, sizeof(mcu2dsp_start_stream_out_param_t));

        start_param->stream_out_param.afe.mce_flag           = true; //enable play en
        DCHS_DL_LOG_I("[MCU]dchs dl scenario_sub_id %d start", 1, config->scenario_sub_id);
    }else if(config->scenario_sub_id == AUDIO_TRANSMITTER_DCHS_UART_UL){
        start_param->param.stream_in = STREAM_IN_AFE;
        start_param->param.stream_out = STREAM_OUT_VIRTUAL;//STREAM_OUT_AFE;//STREAM_OUT_VIRTUAL;
        memset((void *)&start_param->stream_in_param, 0, sizeof(mcu2dsp_start_stream_in_param_t));
        memset((void *)&start_param->stream_out_param, 0, sizeof(mcu2dsp_start_stream_out_param_t));
        TRANSMITTER_LOG_I("[MCU UL][DCHS] dchs ul scenario_sub_id %d start", 1, config->scenario_sub_id);
    }
}

void dchs_dl_get_stream_out_volume(hal_audio_hw_stream_out_index_t hw_gain_index, uint32_t digital_volume_index)
{
    if(dchs_get_device_mode() == DCHS_MODE_LEFT){
        audio_dchs_dl_gain_param_t dl_gain_cosys_ctrl_param = {0};
        dl_gain_cosys_ctrl_param.vol_gain = digital_volume_index;
        dl_gain_cosys_ctrl_param.header.ctrl_type  = AUDIO_UART_COSYS_DL_SET_GAIN;
        dl_gain_cosys_ctrl_param.header.param_size = sizeof(audio_dchs_dl_gain_param_t) - sizeof(audio_uart_cmd_header_t);
        //relay dchs dl open param to the other chip
        mcu_uart_tx(UART_AUDIO_CMD, (U8 *)&dl_gain_cosys_ctrl_param, sizeof(audio_dchs_dl_gain_param_t));
        DCHS_DL_LOG_I("[relay gain] memory_agent:%d, gain:%d", 2 ,hw_gain_index,digital_volume_index);
    }else if(dchs_get_device_mode() == DCHS_MODE_RIGHT){
        mcu2dsp_sync_msg.sync_type    = SYNC_SET_GAIN_VALUE;
        mcu2dsp_sync_msg.vol_gain     = digital_volume_index;
        mcu2dsp_sync_msg.memory_agent = hw_gain_index;
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_MIXER_STREAM_MSG_SYNC, 0, (U32)&mcu2dsp_sync_msg, true);
        DCHS_DL_LOG_I("[set gain]memory_agent:%d, gain:%d", 2 ,hw_gain_index,digital_volume_index);
    }
}

void mcu_uart_init(void)
{
    if(mux_uart_init_flag) { //inited
        return;
    }
    mux_status_t status;
    mux_port_t   mux_port = MUX_LL_UART_1;
    mux_port_setting_t port_setting;
    port_setting.tx_buffer_size = MUX_UART_TXBUF_MAX_SIZE;
    port_setting.rx_buffer_size = MUX_UART_RXBUF_MAX_SIZE;
    port_setting.tx_buffer = (uint32_t)mux_uart_txbuffer;
    port_setting.rx_buffer = (uint32_t)mux_uart_rxbuffer;
    port_setting.dev_setting.uart.uart_config.baudrate = HAL_UART_BAUDRATE_8666000;//HAL_UART_BAUDRATE_8666000;//HAL_UART_BAUDRATE_921600;//;
    port_setting.dev_setting.uart.uart_config.word_length = HAL_UART_WORD_LENGTH_8;
    port_setting.dev_setting.uart.uart_config.stop_bit = HAL_UART_STOP_BIT_1;
    port_setting.dev_setting.uart.uart_config.parity = HAL_UART_PARITY_NONE;
    port_setting.dev_setting.uart.flowcontrol_type = MUX_UART_SW_FLOWCONTROL;

    status = mux_init(mux_port, &port_setting, NULL);

    if (status != MUX_STATUS_OK) {
        TRANSMITTER_LOG_I("[DCHS DL][MUX UART INIT] init fail, status=%d", 1, status);
        AUDIO_ASSERT(0);
        return;
    }
    mux_uart_init_flag = true;
    TRANSMITTER_LOG_I("[DCHS DL][MUX UART INIT] init success, txbuffer=0x%x rxbuffer=0x%x", 2, port_setting.tx_buffer, port_setting.rx_buffer);
}

void mcu_uart_open(uart_type_t type)
{
    if(type == UART_AUDIO_CMD){
        if(audio_cmd_uart_handle){
            TRANSMITTER_LOG_I("[DCHS DL][MUX UART][OPEN] audio cmd uart already opened!",0);
            return;
        }
        mux_status_t status = mux_open(MUX_LL_UART_1, "AUDIO_CMD", &audio_cmd_uart_handle, mcu_mux_uart_callback, NULL);
        if(status != MUX_STATUS_OK){
            audio_cmd_uart_handle = 0;
            TRANSMITTER_LOG_E("[DCHS DL][MUX UART][OPEN] ul uart open failed, status = %d",1,status);
            AUDIO_ASSERT(0);
            return;
        }
        TRANSMITTER_LOG_I("[DCHS DL][MUX UART][OPEN] audio cmd uart open success, uart_handle = 0x%x",1, audio_cmd_uart_handle);
    }
}

void mcu_uart_tx(uart_type_t type, uint8_t *param_buf, uint32_t buf_size)
{
    if (type == UART_AUDIO_CMD) {
        if(!audio_cmd_uart_handle){
            TRANSMITTER_LOG_I("[DCHS DL][UART]mcu audio cmd uart don't opened!",0);
            mcu_uart_open(UART_AUDIO_CMD);
        }
        audio_uart_cmd_header_t * header = (audio_uart_cmd_header_t *)param_buf;
        mux_buffer_t uart_tx_buffer;
        uint32_t tx_size = 0;
        uart_tx_buffer.buf_size = buf_size;
        uart_tx_buffer.p_buf    = param_buf;
        mux_status_t status = mux_tx(audio_cmd_uart_handle, &uart_tx_buffer, 1, &tx_size);
        if (status != MUX_STATUS_OK) {
            TRANSMITTER_LOG_E("[DCHS DL][MUX UART] audio cmd uart tx fail, status=%d, uart_handle = 0x%x", 2, status, audio_cmd_uart_handle);
            AUDIO_ASSERT(0);
        }
        if(tx_size != buf_size) {
            TRANSMITTER_LOG_E("[DCHS DL][MUX UART] audio cmd uart tx fail, uart_handle = 0x%x, buf_size = %d, already send = %d", 3, audio_cmd_uart_handle, buf_size, tx_size);
            AUDIO_ASSERT(0);
        }
        TRANSMITTER_LOG_I("[DCHS DL][MUX UART][Tx] audio cmd uart tx success! data_size=%d, cmd type:%d,ack type:%d", 3, tx_size, header->ctrl_type, header->ack_type);
    } else if (type == UART_UL) {
        /* todo */
    } else {
        TRANSMITTER_LOG_E("[DCHS DL][MUX UART] don't support uart type = %d", 1, type);
    }
}

void dchs_cosys_ctrl_cmd_execute(audio_dchs_cosys_ctrl_t * cosys_ctrl)
{
    dchs_get_device_mode();
    TRANSMITTER_LOG_I("[DCHS][MCU CMD Execute]:ctrl_type:%d,scenario_type:%d", 2, cosys_ctrl->ctrl_type,cosys_ctrl->ctrl_param.dchs_dl_open_param.scenario_type);
    if(cosys_ctrl->ctrl_type == AUDIO_UART_COSYS_DL_OPEN){
        dchs_dl_chip_role_t   chip_role = cosys_ctrl->chip_role;
        audio_scenario_type_t scenario_type = cosys_ctrl->ctrl_param.dchs_dl_open_param.scenario_type;
        memset(&transmitter_config_dl, 0, sizeof(audio_transmitter_config_t));
        transmitter_config_dl.scenario_sub_id = AUDIO_TRANSMITTER_DCHS_UART_DL;
        transmitter_config_dl.scenario_config.dchs_config.scenario_type = cosys_ctrl->ctrl_param.dchs_dl_open_param.scenario_type;
        if(dchs_get_device_mode() == DCHS_MODE_LEFT){  // master latch slave clock
            if(!dchs_dl_running_status.latch_timer_start_flag){
                mux_ll_latch_timer_start(false, 101);
                dchs_dl_running_status.latch_timer_start_flag = true;
                DCHS_DL_LOG_I("[MCU CMD Execute] mcu dl latch timer started",0);
            }
            dchs_dl_running_status.need_sync_scenario_msg = true;
            sceanrio_mcu2dsp_sync_msg_t * sync_msg = get_idle_msg_handle();
            //send slave sceanrio msg to dsp
            sync_msg->sync_type     = SYNC_SCENARIO_OPEN;
            sync_msg->scenario_type = scenario_type;
            sync_msg->memory_agent  = HAL_AUDIO_MEM_SUB;
            sync_msg->sample_rate   = g_mixer_stream_sample_rate;
            sync_msg->format_bytes  = 4;
            sync_msg->need_play_en  = false;
            sync_msg->channel_num   = 1;
        }
        //lock DVFS MID
        hal_dvfs_lock_control(HAL_DVFS_OPP_MID, HAL_DVFS_LOCK);
        //open flow
        mcu2dsp_open_param_t open_param = {0};
        audio_transmitter_dchs_open_playback(&transmitter_config_dl, &open_param);
        ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_MIXER_STREAM, &open_param, true);
        DCHS_DL_LOG_I("in rate:%d,audio_device:%d,memory:%d,stream out:0x%x",4,open_param.stream_in_param.afe.sampling_rate,open_param.stream_out_param.afe.audio_device,open_param.stream_out_param.afe.memory,&open_param.stream_out_param);
        void * p_param_share = hal_audio_dsp_controller_put_paramter(&open_param, sizeof(mcu2dsp_open_param_t), AUDIO_MESSAGE_TYPE_COMMON);
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_MIXER_STREAM_OPEN, 0, (U32)p_param_share, true);
        DCHS_DL_LOG_I("[MCU CMD Execute]: dl open done,scenario_type=%d, chip_role=%d", 2, scenario_type, chip_role);
        //start flow
        mcu2dsp_start_param_t start_param = {0};
        audio_transmitter_dchs_start_playback(&transmitter_config_dl, &start_param);
        p_param_share = hal_audio_dsp_controller_put_paramter(&start_param, sizeof(mcu2dsp_start_param_t), AUDIO_MESSAGE_TYPE_COMMON);
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_MIXER_STREAM_START, 0, (U32)p_param_share, true);
        DCHS_DL_LOG_I("[MCU CMD Execute]: dl start done,scenario_type=%d,chip_role=%d", 2, scenario_type, chip_role);

        //waiting ack or response ack
        if(dchs_check_cmd_ack_status(chip_role, cosys_ctrl->ctrl_type) == ACK_WAITING){
            dchs_loop_waiting_cmd_ack(chip_role, cosys_ctrl->ctrl_type);
        }else if(dchs_check_cmd_ack_status(chip_role, cosys_ctrl->ctrl_type) == ACK_LATER){
            dchs_response_cmd_ack(cosys_ctrl->ctrl_type, SYNC_SUCCESS);
        }
    }else if(cosys_ctrl->ctrl_type == AUDIO_UART_COSYS_DL_START){
        audio_scenario_type_t scenario_type = cosys_ctrl->ctrl_param.dchs_dl_start_param.scenario_type;
        dchs_dl_chip_role_t   chip_role = cosys_ctrl->chip_role;
        if(dchs_get_device_mode() == DCHS_MODE_RIGHT){ 
            sceanrio_mcu2dsp_sync_msg_t * sync_msg = get_msg_handle(scenario_type);
            memcpy(&mcu2dsp_sync_msg, sync_msg, sizeof(sceanrio_mcu2dsp_sync_msg_t));
            DCHS_DL_LOG_I("mcu send dsp start msg,sceanrio_type:%d",1,scenario_type);
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_MIXER_STREAM_MSG_SYNC, 0, (U32)&mcu2dsp_sync_msg, true);
            remove_msg_handle(scenario_type);
        } else {
            if(dchs_dl_running_status.need_sync_scenario_msg){ //slave only open/close mixer stream, will send ctrl msg
                dchs_dl_running_status.need_sync_scenario_msg = false;
                sceanrio_mcu2dsp_sync_msg_t * sync_msg = get_msg_handle(scenario_type);
                memcpy(&mcu2dsp_sync_msg, sync_msg, sizeof(sceanrio_mcu2dsp_sync_msg_t));
                DCHS_DL_LOG_I("mcu send dsp start msg,sceanrio_type:%d",1,scenario_type);
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_MIXER_STREAM_MSG_SYNC, 0, (U32)&mcu2dsp_sync_msg, true);
                remove_msg_handle(scenario_type);
            }
        }
        if(dchs_check_cmd_ack_status(chip_role, cosys_ctrl->ctrl_type) == ACK_WAITING){
            dchs_loop_waiting_cmd_ack(chip_role, cosys_ctrl->ctrl_type);
        }else if(dchs_check_cmd_ack_status(chip_role, cosys_ctrl->ctrl_type) == ACK_LATER){
            dchs_response_cmd_ack(cosys_ctrl->ctrl_type, SYNC_SUCCESS);
        }
    }else if(cosys_ctrl->ctrl_type == AUDIO_UART_COSYS_DL_CLOSE){
        audio_scenario_type_t scenario_type = cosys_ctrl->ctrl_param.dchs_dl_close_param.scenario_type;
        dchs_dl_chip_role_t   chip_role     = cosys_ctrl->chip_role;
        U32 mask;
        mcu2dsp_sync_msg.sync_type     = SYNC_SCENARIO_CLOSE;
        mcu2dsp_sync_msg.scenario_type = scenario_type;

        hal_nvic_save_and_set_interrupt_mask(&mask);
        remove_running_scenario(scenario_type);
        hal_nvic_restore_interrupt_mask(mask);
        if(dchs_get_device_mode() == DCHS_MODE_RIGHT){
            DCHS_DL_LOG_I("send sceanrio close msg to dsp,sceanrio type:%d", 1, scenario_type); 
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_MIXER_STREAM_MSG_SYNC, 0, (U32)&mcu2dsp_sync_msg, true);
        }
        if(check_any_scenario_running()){
            DCHS_DL_LOG_I("other scenario is running, ignore close, scenario_type:%d", 1, scenario_type);
            if(dchs_check_cmd_ack_status(chip_role, cosys_ctrl->ctrl_type) == ACK_WAITING){
                dchs_loop_waiting_cmd_ack(chip_role, cosys_ctrl->ctrl_type);
            }else if(dchs_check_cmd_ack_status(chip_role, cosys_ctrl->ctrl_type) == ACK_LATER){
                dchs_response_cmd_ack(cosys_ctrl->ctrl_type, SYNC_SUCCESS);
            }
            return;
        }
        //reset running flag
        hal_nvic_save_and_set_interrupt_mask(&mask);
        dchs_dl_running_status.is_running    = false;
        dchs_dl_running_status.scenario_type = 0;
        hal_nvic_restore_interrupt_mask(mask);
        //
        if(dchs_get_device_mode() == DCHS_MODE_LEFT){
            if(dchs_dl_running_status.latch_timer_start_flag){
                mux_ll_latch_timer_stop();
                dchs_dl_running_status.latch_timer_start_flag = false;
                DCHS_DL_LOG_I("[MCU CMD Execute] mcu dl latch timer stoped",0);
            }
            DCHS_DL_LOG_I("send sceanrio close msg to dsp,sceanrio type:%d", 1, scenario_type);
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_MIXER_STREAM_MSG_SYNC, 0, (U32)&mcu2dsp_sync_msg, true);
        }
        //unlock DVFS MID
        hal_dvfs_lock_control(HAL_DVFS_OPP_MID, HAL_DVFS_UNLOCK);

        ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_MIXER_STREAM, NULL, false);
        //stop
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_MIXER_STREAM_STOP, 0, 0, true);
        //close
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_MIXER_STREAM_CLOSE, 0, 0, true);
        DCHS_DL_LOG_I("[MCU CMD Execute]: dl stop done,scenario_type=%d, chip_role=%d", 2, scenario_type, chip_role);
        //ack
        if(chip_role == LOCAL_CHIP && dchs_check_cmd_ack_status(chip_role, cosys_ctrl->ctrl_type) != ACK_RECEIVED && dchs_check_cmd_ack_status(chip_role, cosys_ctrl->ctrl_type) != ACK_NO){
            dchs_set_cmd_ack_status(chip_role, cosys_ctrl->ctrl_type, ACK_WAITING);
        }
        if(dchs_check_cmd_ack_status(chip_role, cosys_ctrl->ctrl_type) == ACK_WAITING){
            dchs_loop_waiting_cmd_ack(chip_role, cosys_ctrl->ctrl_type);
        }else if(dchs_check_cmd_ack_status(chip_role, cosys_ctrl->ctrl_type) == ACK_LATER){
            dchs_response_cmd_ack(cosys_ctrl->ctrl_type, SYNC_SUCCESS);
        }
    }else if(cosys_ctrl->ctrl_type == AUDIO_UART_COSYS_DL_SET_GAIN) {
        mcu2dsp_sync_msg.sync_type    = SYNC_SET_GAIN_VALUE;
        mcu2dsp_sync_msg.vol_gain     = cosys_ctrl->ctrl_param.dchs_dl_gain_param.vol_gain;
        mcu2dsp_sync_msg.memory_agent = HAL_AUDIO_MEM_SUB;
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_MIXER_STREAM_MSG_SYNC, 0, (U32)&mcu2dsp_sync_msg, true);
        DCHS_DL_LOG_I("[set rx gain]memory_agent:%d, gain:%d", 2 ,HAL_AUDIO_MEM_SUB, mcu2dsp_sync_msg.vol_gain);

    }else if(cosys_ctrl->ctrl_type == AUDIO_UART_COSYS_UL_OPEN) {
        if(dchs_get_device_mode() == DCHS_MODE_LEFT){  // master latch slave clock
            if(!latch_timer_start_flag){
                mux_ll_latch_timer_start(false, 100);
                latch_timer_start_flag = true;
                TRANSMITTER_LOG_I("[DCHS UL][MCU CMD Execute] mcu dl latch timer started",0);
            }
        }
        memset(&transmitter_config_ul, 0, sizeof(audio_transmitter_config_t));
        transmitter_config_ul.scenario_type = AUDIO_TRANSMITTER_DCHS;
        transmitter_config_ul.scenario_sub_id = AUDIO_TRANSMITTER_DCHS_UART_UL;
        transmitter_config_ul.scenario_config.dchs_config.frame_size = cosys_ctrl->ctrl_param.dchs_ul_open_param.frame_size;
        transmitter_config_ul.scenario_config.dchs_config.sampling_rate = cosys_ctrl->ctrl_param.dchs_ul_open_param.sampling_rate;
        transmitter_config_ul.scenario_config.dchs_config.frame_number  = cosys_ctrl->ctrl_param.dchs_ul_open_param.frame_number;
        transmitter_config_ul.scenario_config.dchs_config.format        = cosys_ctrl->ctrl_param.dchs_ul_open_param.format;
        transmitter_config_ul.scenario_config.dchs_config.codec_type    = cosys_ctrl->ctrl_param.dchs_ul_open_param.codec_type;
        transmitter_config_ul.scenario_config.dchs_config.scenario_type = cosys_ctrl->ctrl_param.dchs_ul_open_param.scenario_type;
        transmitter_config_ul.scenario_config.dchs_config.scenario_type = cosys_ctrl->ctrl_param.dchs_ul_open_param.scenario_type;
        am_audio_side_tone_enable();
        dchs_lock_bt_sleep(false);
        //ul open flow
        void *p_param_share;
        dchs_dl_chip_role_t   chip_role     = cosys_ctrl->chip_role;
        mcu2dsp_open_param_t open_param = {0};
        audio_transmitter_dchs_open_playback(&transmitter_config_ul, &open_param);
        if(dchs_check_cmd_ack_status(chip_role, cosys_ctrl->ctrl_type) == ACK_LATER){
            dchs_response_cmd_ack(cosys_ctrl->ctrl_type, SYNC_SUCCESS);
        }
        ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_DCHS_UART_UL, &open_param, true);
        p_param_share = hal_audio_dsp_controller_put_paramter(&open_param, sizeof(mcu2dsp_open_param_t), AUDIO_MESSAGE_TYPE_AUDIO_TRANSMITTER);
        uint16_t scenario_and_id = ((AUDIO_TRANSMITTER_DCHS) << 8) + AUDIO_TRANSMITTER_DCHS_UART_UL;
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_AUDIO_TRANSMITTER_OPEN, scenario_and_id, (uint32_t)p_param_share, true);
        TRANSMITTER_LOG_I("[DCHS UL][MCU CMD Execute]: ul scenario type %d, codec_type:0x%x open config done", 2, cosys_ctrl->ctrl_param.dchs_ul_open_param.scenario_type, cosys_ctrl->ctrl_param.dchs_ul_open_param.codec_type);
        dchs_ul_running_scenario_type = cosys_ctrl->ctrl_param.dchs_ul_open_param.scenario_type;
        dchs_ul_running_codec_type    = cosys_ctrl->ctrl_param.dchs_ul_open_param.codec_type;
        dchs_ul_running_sample_rate   = cosys_ctrl->ctrl_param.dchs_ul_open_param.sampling_rate;

    }else if(cosys_ctrl->ctrl_type == AUDIO_UART_COSYS_UL_START){
        mcu2dsp_start_param_t start_param = {0};
        void *p_param_share;
        uint16_t scenario_and_id = ((AUDIO_TRANSMITTER_DCHS) << 8) + AUDIO_TRANSMITTER_DCHS_UART_UL;
        audio_transmitter_dchs_start_playback(&transmitter_config_dl, &start_param);
        p_param_share = hal_audio_dsp_controller_put_paramter(&start_param, sizeof(mcu2dsp_start_param_t), AUDIO_MESSAGE_TYPE_AUDIO_TRANSMITTER);
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_AUDIO_TRANSMITTER_START, scenario_and_id, (uint32_t)p_param_share, true);
        
        TRANSMITTER_LOG_I("[DCHS UL][MCU CMD Execute] ul start success,frame_size=%d,frame_number=%d,format=%d,sampling_rate=%d", 4,
                            transmitter_config_ul.scenario_config.dchs_config.frame_size,
                            transmitter_config_ul.scenario_config.dchs_config.frame_number,
                            transmitter_config_ul.scenario_config.dchs_config.format,
                            transmitter_config_ul.scenario_config.dchs_config.sampling_rate);
    }else if(cosys_ctrl->ctrl_type == AUDIO_UART_COSYS_UL_CLOSE){
        am_audio_side_tone_disable();
        dchs_dl_chip_role_t   chip_role     = cosys_ctrl->chip_role;
       uint16_t scenario_and_id = ((AUDIO_TRANSMITTER_DCHS) << 8) + AUDIO_TRANSMITTER_DCHS_UART_UL;
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_AUDIO_TRANSMITTER_STOP, scenario_and_id, 0, true);
        //close
        if(dchs_check_cmd_ack_status(chip_role, cosys_ctrl->ctrl_type) == ACK_LATER){
            dchs_response_cmd_ack(cosys_ctrl->ctrl_type, SYNC_SUCCESS);
        }
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_AUDIO_TRANSMITTER_CLOSE, scenario_and_id, 0, true);
        ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_DCHS_UART_UL, NULL, false);
        hal_dvfs_lock_control(HFP_DVFS_INHOUSE, HAL_DVFS_UNLOCK);
        dchs_ul_running_scenario_type = AUDIO_SCENARIO_TYPE_COMMON;
        dchs_ul_running_codec_type    = BT_HFP_CODEC_TYPE_NONE;
        dchs_ul_running_sample_rate   = 0;
    
    } else if (cosys_ctrl->ctrl_type == AUDIO_UART_COSYS_RACE_CMD) {

        void *ptr;
        audio_dchs_relay_cmd_param_t dchs_relay_cmd_param;
        dchs_relay_cmd_param = cosys_ctrl->ctrl_param.dchs_relay_cmd_param;
        ptr = race_cmd_dsprealtime_handler((ptr_race_pkt_t)dchs_relay_cmd_param.payload, dchs_relay_cmd_param.payload_length, dchs_relay_cmd_param.channel_id);

        if (ptr) {
            race_send_pkt_t *race_send_pkt;
            race_send_pkt = (void *)race_pointer_cnv_pkt_to_send_pkt(ptr);

            dchs_relay_cmd_param.header.ctrl_type = AUDIO_UART_COSYS_RACE_CMD;
            dchs_relay_cmd_param.header.param_size =  sizeof(audio_dchs_relay_cmd_param_t) - sizeof(audio_uart_cmd_header_t);
            memcpy(dchs_relay_cmd_param.payload, (uint8_t *)&race_send_pkt->race_data, race_send_pkt->length);
            dchs_relay_cmd_param.is_ack_cmd = true;
            dchs_relay_cmd_param.payload_length = race_send_pkt->length;
            mcu_uart_tx(UART_AUDIO_CMD, (U8 *)&dchs_relay_cmd_param, sizeof(audio_dchs_relay_cmd_param_t));

            //Free evt mem
            race_mem_free(race_send_pkt);
        }

    } else if (cosys_ctrl->ctrl_type == AUDIO_UART_COSYS_DETACHABLE_MIC) {
        U32 voice_mic_type = cosys_ctrl->ctrl_param.dchs_detachable_mic_param.voice_mic_type;
        TRANSMITTER_LOG_I("[DCHS][MCU CMD Execute] dchs detachable mic sync, mic type:%d", 1, voice_mic_type);
        #ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
        ami_set_voice_mic_type(voice_mic_type); //function flow
        #endif
    } else if (cosys_ctrl->ctrl_type == AUDIO_UART_COSYS_CMD_WAITING_ACK) {
        uart_cmd_type_t cmd_type = cosys_ctrl->ctrl_param.dchs_wait_ack_param.cmd_type;
        TRANSMITTER_LOG_I("[DCHS][MCU CMD Execute]: dchs waiting ack, cmd_type:%d, ack status:%d", 2, cmd_type, dchs_check_cmd_ack_status(LOCAL_CHIP, cmd_type));
        if(dchs_check_cmd_ack_status(LOCAL_CHIP, cmd_type) == ACK_WAIT_LATER){
            uint32_t mask;
            hal_nvic_save_and_set_interrupt_mask(&mask);
            dchs_set_cmd_ack_status(LOCAL_CHIP, cmd_type, ACK_WAITING);
            hal_nvic_restore_interrupt_mask(mask);
            dchs_loop_waiting_cmd_ack(LOCAL_CHIP, cmd_type);
        }
    } else if(cosys_ctrl->ctrl_type == AUDIO_UART_COSYS_UL_MUTE){
        TRANSMITTER_LOG_I("[DCHS UL][MCU CMD Execute] ul set mute:%d",1,cosys_ctrl->ctrl_param.audio_set_mute_param.mute);
        hal_audio_mute_stream_in_by_scenario(HAL_AUDIO_STREAM_IN_SCENARIO_HFP,cosys_ctrl->ctrl_param.audio_set_mute_param.mute);
    } else if(cosys_ctrl->ctrl_type == AUDIO_UART_COSYS_UL_SW_GAIN){
        TRANSMITTER_LOG_I("[DCHS UL][MCU CMD Execute] ul set sw gain",0);
        mcu2dsp_audio_transmitter_runtime_config_param_t runtime_config_param;
        void *p_param_share;
        runtime_config_param.config_operation = cosys_ctrl->ctrl_param.audio_set_sw_gain_param.config_operation;
        memcpy(runtime_config_param.config_param, cosys_ctrl->ctrl_param.audio_set_sw_gain_param.config_param, 40);
        p_param_share = hal_audio_dsp_controller_put_paramter(&runtime_config_param, sizeof(mcu2dsp_audio_transmitter_runtime_config_param_t), AUDIO_MESSAGE_TYPE_AUDIO_TRANSMITTER);
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_AUDIO_TRANSMITTER_CONFIG, 0x0d01, (uint32_t)p_param_share, true);
    }
}

void dchs_cosys_ctrl_cmd_relay(uart_cmd_type_t ctrl_type, audio_scenario_type_t scenario_type, mcu2dsp_open_param_t *open_param, mcu2dsp_start_param_t * start_param)
{
    if(dchs_get_device_mode() == DCHS_MODE_SINGLE || check_support_scenario_type(ctrl_type, scenario_type) == false){
        return;
    }
    TRANSMITTER_LOG_I("[DCHS][CMD Relay]ctrl_type:%d, scenario_type:%d",2,ctrl_type,scenario_type);
    if(ctrl_type == AUDIO_UART_COSYS_DL_OPEN){
        if(open_param) {
            U32 context_type = DUMMPY_CONTENT_TYPE;
            #ifdef AIR_BT_CODEC_BLE_ENABLED
            context_type = open_param->stream_in_param.ble.context_type;
            #endif
            if(scenario_type == AUDIO_SCENARIO_TYPE_BLE_DL) {
                scenario_type = get_ble_scenario_type(context_type);
            }
            //save scneario msg
            if(dchs_get_device_mode() == DCHS_MODE_RIGHT){ 
                sceanrio_mcu2dsp_sync_msg_t * sync_msg = get_idle_msg_handle();
                sync_msg->sync_type     = SYNC_SCENARIO_OPEN;
                sync_msg->scenario_type = scenario_type;
                sync_msg->memory_agent  = open_param->stream_out_param.afe.memory;
                sync_msg->sample_rate   = g_mixer_stream_sample_rate;//open_param->stream_out_param.afe.sampling_rate;
                sync_msg->format_bytes  = (open_param->stream_out_param.afe.format >= HAL_AUDIO_PCM_FORMAT_S24_LE ? 4 : 2);
            }
            //relay dchs dl open param to the other chip
            audio_dchs_dl_open_param_t dl_open_cosys_ctrl_param = {0};
            dl_open_cosys_ctrl_param.scenario_type     = scenario_type;
            dl_open_cosys_ctrl_param.memory_agent      = open_param->stream_out_param.afe.memory;
            dl_open_cosys_ctrl_param.format_bytes      = (open_param->stream_out_param.afe.format >= HAL_AUDIO_PCM_FORMAT_S24_LE ? 4 : 2);
            dl_open_cosys_ctrl_param.header.ctrl_type  = ctrl_type;
            dl_open_cosys_ctrl_param.header.ack_type   = ACK_LATER;
            dl_open_cosys_ctrl_param.header.param_size = sizeof(audio_dchs_dl_open_param_t) - sizeof(audio_uart_cmd_header_t);
            dchs_set_cmd_ack_status(LOCAL_CHIP, ctrl_type, ACK_WAITING);
            mcu_uart_tx(UART_AUDIO_CMD, (U8 *)&dl_open_cosys_ctrl_param, sizeof(audio_dchs_dl_open_param_t));
            DCHS_DL_LOG_I("relay dl open param, scenario_type=%d", 1, scenario_type);

            //add running scenario
            U32 mask;
            if(check_any_scenario_running()){
                hal_nvic_save_and_set_interrupt_mask(&mask);
                add_running_scenario(scenario_type);
                hal_nvic_restore_interrupt_mask(mask);
                DCHS_DL_LOG_I("dchs dl alreay running, ignore open, scenario_type:%d, context type:0x%x", 4, scenario_type, context_type);
                return;
            }
            hal_nvic_save_and_set_interrupt_mask(&mask);
            dchs_dl_running_status.is_running    = true;
            dchs_dl_running_status.scenario_type = scenario_type;
            add_running_scenario(scenario_type);
            hal_nvic_restore_interrupt_mask(mask);
            //------
            /***************************************************************************************
            *    important : change other scenario sample rate align with Mixer stream sample rate *
            ****************************************************************************************/
            open_param->stream_out_param.afe.sampling_rate = g_mixer_stream_sample_rate;

            //set hw gain
            U32 data32 = (0x80 << 16) | (0x80 & 0xFFFF);
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SET_OUTPUT_DEVICE_VOLUME, HAL_AUDIO_STREAM_OUT4, data32, false);
            //config local chip open param
            audio_dchs_cosys_ctrl_t  cosys_ctrl;
            cosys_ctrl.ctrl_type  = ctrl_type;
            cosys_ctrl.chip_role  = LOCAL_CHIP;
            cosys_ctrl.ctrl_param.dchs_dl_open_param = dl_open_cosys_ctrl_param;
            dchs_cosys_ctrl_cmd_execute(&cosys_ctrl);
        } else {
            //for expand
        }
    }else if (ctrl_type == AUDIO_UART_COSYS_DL_START){
        if (start_param){
            audio_dchs_dl_start_param_t dl_start_cosys_ctrl_param = {0};
            if(scenario_type == AUDIO_SCENARIO_TYPE_BLE_DL) {
                scenario_type = dchs_dl_running_status.scenario_type;
            }
            dl_start_cosys_ctrl_param.scenario_type      = scenario_type;
            dl_start_cosys_ctrl_param.header.ctrl_type   = ctrl_type;
            dl_start_cosys_ctrl_param.header.ack_type    = ACK_LATER;
            dl_start_cosys_ctrl_param.header.param_size  = sizeof(audio_dchs_dl_start_param_t) - sizeof(audio_uart_cmd_header_t);
            dchs_set_cmd_ack_status(LOCAL_CHIP, ctrl_type, ACK_WAITING);
            //relay dchs dl start param to the other chip
            mcu_uart_tx(UART_AUDIO_CMD, (U8 *)&dl_start_cosys_ctrl_param, sizeof(audio_dchs_dl_start_param_t));
            DCHS_DL_LOG_I("relay dl start param cmd success, scenario_type=%d", 1, scenario_type);
            // mcu to dsp scenario msg sync
            sceanrio_mcu2dsp_sync_msg_t * sync_msg = get_msg_handle(scenario_type);
            sync_msg->need_play_en = !start_param->stream_out_param.afe.mce_flag;
            start_param->stream_out_param.afe.mce_flag         = true; //enable play en
            start_param->stream_out_param.afe.aws_sync_request = false;//disable sw trigger
            DCHS_pka_lock_bt_sleep(dchs_bt_lock_sleep_done_callback, (void *)PLAY_DELAY_MS);
            //create local chip dchs dl path
            audio_dchs_cosys_ctrl_t  cosys_ctrl;
            cosys_ctrl.ctrl_type  = ctrl_type;
            cosys_ctrl.chip_role  = LOCAL_CHIP;
            cosys_ctrl.ctrl_param.dchs_dl_start_param = dl_start_cosys_ctrl_param;
            dchs_cosys_ctrl_cmd_execute(&cosys_ctrl);
        } else {
            //for expand
        }
    }else if(ctrl_type == AUDIO_UART_COSYS_DL_CLOSE){   
        if(scenario_type == AUDIO_SCENARIO_TYPE_BLE_DL) {
            scenario_type = dchs_dl_running_status.scenario_type;
        }
        if(check_running_scenario(scenario_type)){
            audio_dchs_dl_close_param_t dl_close_cosys_ctrl_param = {0};
            dl_close_cosys_ctrl_param.scenario_type = scenario_type;
            dl_close_cosys_ctrl_param.header.ctrl_type  = ctrl_type;
            dl_close_cosys_ctrl_param.header.ack_type   = ACK_LATER;
            dl_close_cosys_ctrl_param.header.param_size = sizeof(audio_dchs_dl_close_param_t) - sizeof(audio_uart_cmd_header_t);
            dchs_set_cmd_ack_status(LOCAL_CHIP, ctrl_type, ACK_WAITING);
            mcu_uart_tx(UART_AUDIO_CMD, (U8 *)&dl_close_cosys_ctrl_param, sizeof(audio_dchs_dl_close_param_t));
            DCHS_DL_LOG_I("relay dl close param ,scenario_type=%d", 2, scenario_type);

            dchs_dl_running_status.is_running    = false;
            dchs_dl_running_status.scenario_type = 0;

            audio_dchs_cosys_ctrl_t  cosys_ctrl;
            cosys_ctrl.ctrl_type  = ctrl_type;
            cosys_ctrl.chip_role  = LOCAL_CHIP;
            cosys_ctrl.ctrl_param.dchs_dl_close_param = dl_close_cosys_ctrl_param;        
            dchs_cosys_ctrl_cmd_execute(&cosys_ctrl);
        } else {
            DCHS_DL_LOG_I("scenario_type:%d already close", 1, scenario_type);
        }
    } else if(ctrl_type == AUDIO_UART_COSYS_UL_OPEN) {
        audio_dchs_ul_open_param_t ul_open_cosys_ctrl_param;
        memset(&ul_open_cosys_ctrl_param, 0, sizeof(audio_dchs_ul_open_param_t));
        dchs_lock_bt_sleep(false);
        ul_open_cosys_ctrl_param.scenario_type = scenario_type;
        ul_open_cosys_ctrl_param.sampling_rate = open_param->stream_in_param.afe.sampling_rate;
        ul_open_cosys_ctrl_param.frame_size    = open_param->stream_in_param.afe.frame_size;
        ul_open_cosys_ctrl_param.frame_number  = open_param->stream_in_param.afe.frame_number;
        ul_open_cosys_ctrl_param.format        = open_param->stream_in_param.afe.format;
        ul_open_cosys_ctrl_param.codec_type    = open_param->stream_out_param.hfp.codec_type;
        if(scenario_type == AUDIO_SCENARIO_TYPE_BLE_UL){
            #ifdef AIR_BT_CODEC_BLE_ENABLED
            ul_open_cosys_ctrl_param.codec_type  = open_param->stream_out_param.ble.context_type;
            #endif
        }
        dchs_ul_running_codec_type = ul_open_cosys_ctrl_param.codec_type;
        ul_open_cosys_ctrl_param.header.ctrl_type  = ctrl_type;
        ul_open_cosys_ctrl_param.header.ack_type   = ACK_LATER;
        ul_open_cosys_ctrl_param.header.param_size = sizeof(audio_dchs_ul_open_param_t) - sizeof(audio_uart_cmd_header_t);
        dchs_set_cmd_ack_status(LOCAL_CHIP, ctrl_type, ACK_WAITING);
        mcu_uart_tx(UART_AUDIO_CMD, (U8 *)&ul_open_cosys_ctrl_param, sizeof(audio_dchs_ul_open_param_t));
        dchs_loop_waiting_cmd_ack(LOCAL_CHIP, ctrl_type);
        TRANSMITTER_LOG_I("[DCHS UL] relay ul open param cmd success, scenario_type:%d, context_type:%d ", 2, scenario_type, ul_open_cosys_ctrl_param.codec_type);
} else if(ctrl_type == AUDIO_UART_COSYS_UL_START) {
        audio_dchs_ul_start_param_t ul_start_cosys_ctrl_param;
        memset(&ul_start_cosys_ctrl_param, 0, sizeof(audio_dchs_ul_start_param_t));
        ul_start_cosys_ctrl_param.scenario_type      = scenario_type;
        ul_start_cosys_ctrl_param.header.ctrl_type   = ctrl_type;
        ul_start_cosys_ctrl_param.header.param_size  = sizeof(audio_dchs_ul_start_param_t) - sizeof(audio_uart_cmd_header_t);
        mcu_uart_tx(UART_AUDIO_CMD, (U8 *)&ul_start_cosys_ctrl_param, sizeof(audio_dchs_ul_start_param_t));
        TRANSMITTER_LOG_I("[DCHS UL] relay ul start param cmd success ",0);
    } else if(ctrl_type == AUDIO_UART_COSYS_UL_CLOSE) {
        audio_dchs_ul_close_param_t ul_close_cosys_ctrl_param;
        memset(&ul_close_cosys_ctrl_param, 0, sizeof(audio_dchs_ul_close_param_t));
        ul_close_cosys_ctrl_param.scenario_type = scenario_type;
        ul_close_cosys_ctrl_param.header.ctrl_type  = ctrl_type;
        ul_close_cosys_ctrl_param.header.param_size = sizeof(audio_dchs_ul_close_param_t) - sizeof(audio_uart_cmd_header_t);
        mcu_uart_tx(UART_AUDIO_CMD, (U8 *)&ul_close_cosys_ctrl_param, sizeof(audio_dchs_ul_close_param_t));
        dchs_ul_running_codec_type = 0;
        TRANSMITTER_LOG_I("[DCHS UL] relay ul close param cmd success ",0);
    } else if(ctrl_type == AUDIO_UART_COSYS_UL_VOLUME) {
        audio_dchs_ul_volume_param_t ul_open_cosys_ctrl_volume_param;
        memset(&ul_open_cosys_ctrl_volume_param, 0, sizeof(audio_dchs_ul_volume_param_t));
        ul_open_cosys_ctrl_volume_param.scenario_type = scenario_type;
        ul_open_cosys_ctrl_volume_param.codec         = open_param->stream_in_param.vol_info.codec;
        ul_open_cosys_ctrl_volume_param.dev_in        = open_param->stream_in_param.vol_info.dev_in;
        ul_open_cosys_ctrl_volume_param.dev_out       = open_param->stream_in_param.vol_info.dev_out;
        ul_open_cosys_ctrl_volume_param.lev_in        = open_param->stream_in_param.vol_info.lev_in;
        ul_open_cosys_ctrl_volume_param.lev_out       = open_param->stream_in_param.vol_info.lev_out;
        ul_open_cosys_ctrl_volume_param.header.ctrl_type  = ctrl_type;
        ul_open_cosys_ctrl_volume_param.header.param_size = sizeof(audio_dchs_ul_volume_param_t) - sizeof(audio_uart_cmd_header_t);
        mcu_uart_tx(UART_AUDIO_CMD, (U8 *)&ul_open_cosys_ctrl_volume_param, sizeof(audio_dchs_ul_volume_param_t));
        TRANSMITTER_LOG_I("[DCHS UL] relay ul volume param cmd success ",0);
    }
}


#endif //AIR_DCHS_MODE_ENABLE
