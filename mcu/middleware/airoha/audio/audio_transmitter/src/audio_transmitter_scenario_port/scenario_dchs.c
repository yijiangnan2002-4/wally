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
#define BIT_SET(var_ptr, p)      (*var_ptr |=  (1U << (uint8_t)p))
#define BIT_RESET(var_ptr, p)    (*var_ptr &= ~(1U << (uint8_t)p))
#define BIT_GET(var_ptr, p)      (((var_ptr) >> (p)) & 1U)
#define SCENARIO_2_POST          (1) //for scenario 2
#define SCENARIO_1_POST          (0) //for scenario 1
#define NO_GAIN                  (0xFFFFFFFF)
#define NO_ID                    (-1)
#define USB_IN_8_CHANNEL         (8)
#define HFP_DVFS_INHOUSE         HAL_DVFS_OPP_HIGH

static bool mux_uart_init_flag = false;
static mux_handle_t audio_cmd_uart_handle = 0;
static uint8_t   dchs_dl_other_chip_exist = 0;
static uint8_t   dchs_dl_local_chip_exist = 0;
static audio_transmitter_config_t transmitter_config_dl;
static audio_transmitter_config_t transmitter_config_ul;
static audio_dchs_ul_volume_param_t g_ul_volume_ctrl_param;
static bool   latch_timer_start_flag   = false;
bt_clock_t g_dchs_anc_target_clk = {0};
uint8_t g_anc_biquad_coef[188];
audio_uart_cmd_ack_type_t g_dchs_cmd_ack_status[AUDIO_UART_COSYS_CMD_MAX] = {0};

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
    U32      frame_size;
    U8       channel_num;
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

typedef enum
{
    DL_AGENT_1 = 0,
    DL_AGENT_2,
    DL_AGENT_3,
    DL_AGENT_MAX,
} dl_agent_type_t;

typedef struct
{
    bool is_running;
    audio_scenario_type_t cur_running_scenario_type;//avoid duplicate dl start
    audio_scenario_type_t next_running_scenario_type;//avoid duplicate dl start
    uint32_t cur_ble_context_type;//avoid duplicate dl start
    uint32_t next_ble_context_type;//avoid duplicate dl start
    bool bt_lock_flag;
    bool wait_set_play_en;
} dchs_dl_status_t;

static dchs_dl_status_t dchs_dl_status = {false, AUDIO_SCENARIO_TYPE_COMMON,AUDIO_SCENARIO_TYPE_COMMON, 0, 0, false, false};
static dual_chip_dl_sync_msg_t dchs_dl_sync_msg[MAX_CHIP] = {0};
static uint32_t ble_context_type[MAX_CHIP] = {0};

audio_uart_cmd_status_t cur_uart_cmd_status = {AUDIO_UART_COSYS_CMD_MIN, 0 , READY_READ};
ATTR_SHARE_ZIDATA static audio_dchs_mcu2dsp_cosys_sync_t dchs_cosys_sync;

U32 g_dchs_dl_sample_rate = 48000;

/******************************************************************************
 *
 * Private Function Define
 *
 ******************************************************************************/
 uart_cmd_type_t dchs_query_any_cmd_waiting_ack(void)
{
    uart_cmd_type_t cmd_type;
    for(cmd_type = AUDIO_UART_COSYS_CMD_MIN; cmd_type < AUDIO_UART_COSYS_CMD_MAX; cmd_type ++){
        if(g_dchs_cmd_ack_status[cmd_type] == ACK_WAITING){
            return cmd_type;
        }
    }
    return AUDIO_UART_COSYS_CMD_MIN;
}

void dchs_set_cmd_to_am_front(uart_cmd_type_t cmd_type , bt_sink_srv_am_feature_t * am_param)
{
    switch (cmd_type) {
        case AUDIO_UART_COSYS_DL_OPEN:
        case AUDIO_UART_COSYS_DL_START:
        case AUDIO_UART_COSYS_DL_CLOSE:
        case AUDIO_UART_COSYS_DL_SET_GAIN:
        case AUDIO_UART_COSYS_DL_SAMPLE_RATE_SYNC:
            am_param->feature_param.cosys_ctrl_dl.is_send_am_front = true;
            break;
        case AUDIO_UART_COSYS_UL_OPEN:
        case AUDIO_UART_COSYS_UL_START:
        case AUDIO_UART_COSYS_UL_CLOSE:
        case AUDIO_UART_COSYS_UL_VOLUME:
            am_param->feature_param.cosys_ctrl_ul.is_send_am_front = true;
            break;
        case AUDIO_UART_COSYS_ANC_CTRL:
        case AUDIO_UART_COSYS_RACE_CMD:
            am_param->feature_param.dchs_param.is_send_am_front = true;
            break;
        default :
            TRANSMITTER_LOG_I("[DCHS] cmd type:%d no need send am front", 1, cmd_type);
    }
}

audio_uart_cmd_ack_type_t dchs_check_cmd_ack_status(uart_cmd_type_t cmd_type)
{
    return g_dchs_cmd_ack_status[cmd_type];
}

void dchs_set_cmd_ack_status(uart_cmd_type_t cmd_type, audio_uart_cmd_ack_type_t ack_type)
{
    g_dchs_cmd_ack_status[cmd_type] = ack_type;
}

void dchs_loop_waiting_cmd_ack(uart_cmd_type_t cmd_type)
{
    U32 i;
    TRANSMITTER_LOG_I("[DCHS] Waiting cmd:%d ack ...", 1, cmd_type);
    for (i = 0; ; i++) {
        if(dchs_check_cmd_ack_status(cmd_type) == ACK_RECEIVED) {
            TRANSMITTER_LOG_I("[DCHS] get sync ack, cmd:%d", 1, cmd_type);
            dchs_set_cmd_ack_status(cmd_type, ACK_NO);
            break;
        } else if(dchs_check_cmd_ack_status(cmd_type) == ACK_WAIT_LATER){
            TRANSMITTER_LOG_I("[DCHS] wait ack later, cmd:%d", 1, cmd_type);
            break;
        }
        if (i == 1000) {
            TRANSMITTER_LOG_I("[DCHS] cmd:%d NO response ack", 1, cmd_type);
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
    dchs_set_cmd_ack_status(cmd_type, ACK_NO);
}

void dchs_bt_lock_sleep_done_callback(void *user_data)
{
    TRANSMITTER_LOG_I("[DCHS]bt lock sleep done,is dl:%d", 1, (bool)user_data);
}

void dchs_dsp2mcu_msg_callback(hal_audio_event_t event, void *data)
{
    DCHS_pka_unlock_bt_sleep();
    bool is_dchs_dl = (bool)data;
    TRANSMITTER_LOG_I("[DCHS]bt unlock sleep done,is dchs dl:%d", 1, is_dchs_dl);
    if(is_dchs_dl){
        dchs_dl_status.bt_lock_flag = false;
    }
}

void dchs_lock_bt_sleep(bool is_dchs_dl)
{
    if(is_dchs_dl){
        hal_audio_service_hook_callback(AUDIO_MESSAGE_TYPE_DCHS_DL, dchs_dsp2mcu_msg_callback, (void *)is_dchs_dl);
        dchs_dl_status.bt_lock_flag = true;
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
            return;
        }
        if(rx_size != buf_size){
            TRANSMITTER_LOG_E("[DCHS DL][uart callback] mux uart rx fail,already_rx_size=%d,need_rx_size=%d", 2, rx_size, buf_size);
            AUDIO_ASSERT(0);
            return;
        }
        TRANSMITTER_LOG_I("[DCHS DL][uart callback] audio cmd rx data size: %d", 1, rx_size);
    } else if (type == UART_UL) {
        /* todo */
    } else {
        TRANSMITTER_LOG_E("[DCHS DL][MUX UART] don't support uart type = %d", 1, type);
    }
}

void dchs_mcu2dsp_msg_sync(dchs_mcu2dsp_sync_type_t sync_type, void *msg)
{
    if(sync_type == DCHS_DL_OTHER_CHIP_DL_EXIST){
        dual_chip_dl_sync_msg_t * dl_msg = (dual_chip_dl_sync_msg_t *)msg;
        dchs_cosys_sync.sync_type = sync_type;
        memcpy(&dchs_cosys_sync.sync_msg.dual_chip_dl_sync , dl_msg, sizeof(dual_chip_dl_sync_msg_t));
        TRANSMITTER_LOG_I("[DCHS DL][MCU] ccni msg sync,OTHER_CHIP_DL_EXIST = %d", 1, dchs_cosys_sync.sync_msg.dual_chip_dl_sync.other_chip_dl_exist);
    }else if(sync_type == DCHS_DL_LOCAL_CHIP_DL_EXIST){
        dual_chip_dl_sync_msg_t * dl_msg = (dual_chip_dl_sync_msg_t *)msg;
        dchs_cosys_sync.sync_type = sync_type;
        memcpy(&dchs_cosys_sync.sync_msg.dual_chip_dl_sync , dl_msg, sizeof(dual_chip_dl_sync_msg_t));
        TRANSMITTER_LOG_I("[DCHS DL][MCU] ccni msg sync,data scenario=%d,ch num=%d,sample_rate=%d,format_bytes=%d,LOCAL_CHIP_DL_EXIST = %d", 5, dl_msg->data_scenario_type, dchs_cosys_sync.sync_msg.dual_chip_dl_sync.channel_num,dl_msg->sample_rate, dl_msg->format_bytes, dl_msg->local_chip_dl_exist);
    }else if(sync_type == DCHS_DL_SET_GAIN_VALUE){
        dual_chip_dl_sync_msg_t * dl_msg = (dual_chip_dl_sync_msg_t *)msg;
        dchs_cosys_sync.sync_type = sync_type;
        memcpy(&dchs_cosys_sync.sync_msg.dual_chip_dl_sync , dl_msg, sizeof(dual_chip_dl_sync_msg_t));
        TRANSMITTER_LOG_I("[DCHS DL][MCU] ccni msg sync,vol_gain=0x%x,operation=%d", 2, dl_msg->vol_gain, dl_msg->operation);

#ifdef AIR_3RD_PARTY_AUDIO_PLATFORM_ENABLE
        vol_type_t vol_type = VOL_TOTAL;
        if(dl_msg->operation == DCHS_DL_CONFIG_OP_SET_UART_SCENARIO_VOL_INFO) {
            vol_type = VOL_A2DP;
        } else if (dl_msg->operation == DCHS_DL_CONFIG_OP_SET_LOCAL_SCENARIO_2_VOL_INFO) {
            vol_type = VOL_VP;
        }

        if (vol_type != VOL_TOTAL) {
            bt_sink_srv_audio_setting_set_audio_platform_output_volume(vol_type, dl_msg->vol_gain);
        }
#endif
    }else if(sync_type == DCHS_DL_SET_PLAY_EN){
        dual_chip_dl_sync_msg_t * dl_msg = (dual_chip_dl_sync_msg_t *)msg;
        dchs_cosys_sync.sync_type = sync_type;
        dchs_cosys_sync.sync_msg.dual_chip_dl_sync.data_scenario_type  = dl_msg->data_scenario_type;
        TRANSMITTER_LOG_I("[DCHS DL][MCU] ccni msg sync, trigger dsp play en, scenario type=%d", 1, dl_msg->data_scenario_type);
    }
    //extend here
    else{
        TRANSMITTER_LOG_W("[DCHS][MCU] dchs ccni msg sync type don't support:%d", 1, sync_type);
    }
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_DCHS_COSYS_SYNC, 0, (uint32_t)&dchs_cosys_sync, true);
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
    if(dchs_check_cmd_ack_status(cmd_type) == ACK_NOW){
        dchs_response_cmd_ack(cmd_type, SYNC_SUCCESS);
    }
}

void dchs_dl_set_scenario_exist_flag(dchs_dl_chip_role_t chip_role, audio_scenario_type_t data_scenario_type, bool is_running)
{
    uint8_t * exist_flag = (chip_role == LOCAL_CHIP ? &dchs_dl_local_chip_exist : &dchs_dl_other_chip_exist);
    if(data_scenario_type == AUDIO_SCENARIO_TYPE_VP){
        is_running ? BIT_SET(exist_flag, SCENARIO_2_POST) : BIT_RESET(exist_flag, SCENARIO_2_POST);
    }else{
        is_running ? BIT_SET(exist_flag, SCENARIO_1_POST) : BIT_RESET(exist_flag, SCENARIO_1_POST);
    }
    TRANSMITTER_LOG_I("[DCHS DL]set scenario exist flag: %d, scenario_type:%d, chip_role=%d", 3, *exist_flag, data_scenario_type, chip_role);
}

uint8_t dchs_dl_check_scenario_exist_flag(dchs_dl_chip_role_t chip_role)
{
    return (chip_role == LOCAL_CHIP ? dchs_dl_local_chip_exist : dchs_dl_other_chip_exist);
}

bool dchs_dl_check_vp_exist_flag(void)
{
    return BIT_GET(dchs_dl_local_chip_exist, SCENARIO_2_POST);
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
                                TRANSMITTER_LOG_E("[DCHS DL][uart callback]  rx invalid header, param_size = %d", 1, cur_uart_cmd_status.param_size);
                                AUDIO_ASSERT(0);
                                return;
                            }
                            dchs_set_cmd_ack_status(cmd_header.ctrl_type, cmd_header.ack_type);
                            cur_uart_cmd_status.cmd_status = WAITING_READ;
                        }else{
                            break;//end while
                        }
                    }
                    if (cur_uart_cmd_status.cmd_status == WAITING_READ){
                        uint32_t uart_data_size =  query_uart_buf_data_size();
                        if(uart_data_size >= cur_uart_cmd_status.param_size){
                            bt_sink_srv_am_feature_t feature_param;
                            memset(&feature_param, 0, sizeof(bt_sink_srv_am_feature_t));
                            uint8_t header_size  = sizeof(audio_uart_cmd_header_t);

                            if(cur_uart_cmd_status.ctrl_type == AUDIO_UART_COSYS_DL_OPEN){
                                audio_dchs_dl_open_param_t dl_open_ctrl_param;
                                mcu_uart_rx(UART_AUDIO_CMD, (uint8_t*)&dl_open_ctrl_param + header_size, sizeof(audio_dchs_dl_open_param_t) - header_size);
                                feature_param.type_mask = AM_UART_COSYS_CONTROL_DL;
                                feature_param.feature_param.cosys_ctrl_dl.ctrl_param.dchs_dl_open_param = dl_open_ctrl_param;
                                feature_param.feature_param.cosys_ctrl_dl.ctrl_param.dchs_dl_open_param.chip_role = OTHER_CHIP;
                                feature_param.feature_param.cosys_ctrl_dl.ctrl_type = AUDIO_UART_COSYS_DL_OPEN;
                                if(dl_open_ctrl_param.scenario_type == AUDIO_SCENARIO_TYPE_BLE_DL){
                                    ble_context_type[OTHER_CHIP] = dl_open_ctrl_param.context_type;
                                }
                                TRANSMITTER_LOG_I("[DCHS DL][rx cmd] chip role=%d,scenario_type=%d,context type=0x%x, dchs dl sample rate=%d,ctrl type = DL_OPEN,", 4, feature_param.feature_param.cosys_ctrl_dl.ctrl_param.dchs_dl_open_param.chip_role, dl_open_ctrl_param.scenario_type,dl_open_ctrl_param.context_type,g_dchs_dl_sample_rate);
                            }else if (cur_uart_cmd_status.ctrl_type == AUDIO_UART_COSYS_DL_START){
                                audio_dchs_dl_start_param_t dl_start_ctrl_param;
                                mcu_uart_rx(UART_AUDIO_CMD, (uint8_t*)&dl_start_ctrl_param + header_size, sizeof(audio_dchs_dl_start_param_t) - header_size);
                                feature_param.type_mask = AM_UART_COSYS_CONTROL_DL;
                                feature_param.feature_param.cosys_ctrl_dl.ctrl_param.dchs_dl_start_param = dl_start_ctrl_param;
                                feature_param.feature_param.cosys_ctrl_dl.ctrl_param.dchs_dl_start_param.chip_role = OTHER_CHIP;
                                if(dl_start_ctrl_param.scenario_type == AUDIO_SCENARIO_TYPE_BLE_DL){
                                    feature_param.feature_param.cosys_ctrl_dl.ctrl_param.dchs_dl_start_param.context_type = ble_context_type[OTHER_CHIP];
                                }
                                feature_param.feature_param.cosys_ctrl_dl.ctrl_type = AUDIO_UART_COSYS_DL_START;
                                TRANSMITTER_LOG_I("[DCHS DL][rx cmd] scenario_type=%d, context type=0x%x,ctrl type = DL_START,other chip scneario exist:0x%x", 3 , dl_start_ctrl_param.scenario_type, ble_context_type[OTHER_CHIP],dchs_dl_check_scenario_exist_flag(OTHER_CHIP));
                            }else if(cur_uart_cmd_status.ctrl_type == AUDIO_UART_COSYS_DL_CLOSE){
                                audio_dchs_dl_close_param_t dl_close_ctrl_param;
                                mcu_uart_rx(UART_AUDIO_CMD, (uint8_t*)&dl_close_ctrl_param + header_size, sizeof(audio_dchs_dl_close_param_t) - header_size);
                                //dchs_dl_set_scenario_exist_flag(OTHER_CHIP, dl_close_ctrl_param.scenario_type, false);
                                TRANSMITTER_LOG_I("[DCHS DL][rx cmd] scenario_type=%d,context type=0x%x, ctrl type = DL_CLOSE,other chip scneario exist:0x%x", 3 , dl_close_ctrl_param.scenario_type, ble_context_type[OTHER_CHIP],dchs_dl_check_scenario_exist_flag(OTHER_CHIP));
                                feature_param.type_mask = AM_UART_COSYS_CONTROL_DL;
                                feature_param.feature_param.cosys_ctrl_dl.ctrl_param.dchs_dl_close_param = dl_close_ctrl_param;
                                feature_param.feature_param.cosys_ctrl_dl.ctrl_param.dchs_dl_close_param.chip_role = OTHER_CHIP;
                                if(dl_close_ctrl_param.scenario_type == AUDIO_SCENARIO_TYPE_BLE_DL){
                                    feature_param.feature_param.cosys_ctrl_dl.ctrl_param.dchs_dl_close_param.context_type = ble_context_type[OTHER_CHIP];
                                }
                                feature_param.feature_param.cosys_ctrl_dl.ctrl_type = AUDIO_UART_COSYS_DL_CLOSE;
                            }else if(cur_uart_cmd_status.ctrl_type == AUDIO_UART_COSYS_DL_SET_GAIN){
                                audio_dchs_dl_gain_param_t dl_gain_ctrl_param;
                                mcu_uart_rx(UART_AUDIO_CMD, (uint8_t*)&dl_gain_ctrl_param + header_size, sizeof(audio_dchs_dl_gain_param_t) - header_size);
                                feature_param.type_mask = AM_UART_COSYS_CONTROL_DL;
                                feature_param.feature_param.cosys_ctrl_dl.ctrl_param.dchs_dl_gain_param = dl_gain_ctrl_param;
                                feature_param.feature_param.cosys_ctrl_dl.ctrl_type = AUDIO_UART_COSYS_DL_SET_GAIN;
                                TRANSMITTER_LOG_I("[DCHS DL][rx cmd] get gain:0x%x,operation:%d", 2 , dl_gain_ctrl_param.vol_gain, dl_gain_ctrl_param.operation);
                            }else if(cur_uart_cmd_status.ctrl_type == AUDIO_UART_COSYS_UL_OPEN){
                                audio_dchs_ul_open_param_t ul_open_ctrl_param;
                                mcu_uart_rx(UART_AUDIO_CMD, (uint8_t*)&ul_open_ctrl_param + header_size, sizeof(audio_dchs_ul_open_param_t) - header_size);
                                feature_param.type_mask = AM_UART_COSYS_CONTROL_UL;
                                feature_param.feature_param.cosys_ctrl_ul.ctrl_param.dchs_ul_open_param = ul_open_ctrl_param;
                                feature_param.feature_param.cosys_ctrl_ul.ctrl_type = AUDIO_UART_COSYS_UL_OPEN;
                                if(ul_open_ctrl_param.scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_OUT){
                                    g_ul_volume_ctrl_param.codec = BT_HFP_CODEC_TYPE_MSBC;
                                    g_ul_volume_ctrl_param.dev_in = HAL_AUDIO_DEVICE_MAIN_MIC;
                                    g_ul_volume_ctrl_param.dev_out = HAL_AUDIO_DEVICE_HEADSET;
                                    g_ul_volume_ctrl_param.lev_in = 0;
                                    g_ul_volume_ctrl_param.lev_out = 0;
                                }
                                TRANSMITTER_LOG_I("[DCHS UL][rx cmd] scenario_type=%d, ctrl type = UL_OPEN,", 1 , ul_open_ctrl_param.scenario_type);
                            }else if (cur_uart_cmd_status.ctrl_type == AUDIO_UART_COSYS_UL_START){
                                audio_dchs_ul_start_param_t ul_start_ctrl_param;
                                mcu_uart_rx(UART_AUDIO_CMD, (uint8_t*)&ul_start_ctrl_param + header_size, sizeof(audio_dchs_ul_start_param_t) - header_size);
                                TRANSMITTER_LOG_I("[DCHS UL][rx cmd] scenario_type=%d, ctrl type = UL_START,", 1 , ul_start_ctrl_param.scenario_type);
                                feature_param.type_mask = AM_UART_COSYS_CONTROL_UL;
                                feature_param.feature_param.cosys_ctrl_ul.ctrl_param.dchs_ul_start_param = ul_start_ctrl_param;
                                feature_param.feature_param.cosys_ctrl_ul.ctrl_type = AUDIO_UART_COSYS_UL_START;
                            }else if(cur_uart_cmd_status.ctrl_type == AUDIO_UART_COSYS_UL_CLOSE){
                                audio_dchs_ul_close_param_t ul_close_ctrl_param;
                                mcu_uart_rx(UART_AUDIO_CMD, (uint8_t*)&ul_close_ctrl_param + header_size, sizeof(audio_dchs_ul_close_param_t) - header_size);
                                TRANSMITTER_LOG_I("[DCHS UL][rx cmd] scenario_type=%d, ctrl type = UL_CLOSE,", 1 , ul_close_ctrl_param.scenario_type);
                                feature_param.type_mask = AM_UART_COSYS_CONTROL_UL;
                                feature_param.feature_param.cosys_ctrl_ul.ctrl_param.dchs_ul_close_param = ul_close_ctrl_param;
                                feature_param.feature_param.cosys_ctrl_ul.ctrl_type = AUDIO_UART_COSYS_UL_CLOSE;
                            }else if(cur_uart_cmd_status.ctrl_type == AUDIO_UART_COSYS_UL_VOLUME){
                                memset(&g_ul_volume_ctrl_param,0,sizeof(audio_dchs_ul_volume_param_t));
                                mcu_uart_rx(UART_AUDIO_CMD, (uint8_t*)&g_ul_volume_ctrl_param + header_size, sizeof(audio_dchs_ul_volume_param_t) - header_size);
                                TRANSMITTER_LOG_I("[DCHS UL][rx cmd] scenario_type=%d, ctrl type = UL_VOLUME,", 1 , g_ul_volume_ctrl_param.scenario_type);
                                reset_cur_cmd_status(cur_uart_cmd_status.ctrl_type);
                                continue;
                            } else if (cur_uart_cmd_status.ctrl_type == AUDIO_UART_COSYS_RACE_CMD) {
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
                                    feature_param.type_mask = AM_DCHS;
                                    feature_param.feature_param.dchs_param.ctrl_param.dchs_relay_cmd_param = calibration_param;
                                    feature_param.feature_param.cosys_ctrl_ul.ctrl_type = AUDIO_UART_COSYS_RACE_CMD;
                                }

                            } else if (cur_uart_cmd_status.ctrl_type == AUDIO_UART_COSYS_ANC_CTRL) {
                                audio_dchs_anc_param_t *anc_ctrl_param;

                                anc_ctrl_param = pvPortMalloc(sizeof(audio_dchs_anc_param_t));
                                mcu_uart_rx(UART_AUDIO_CMD, (uint8_t*)anc_ctrl_param + header_size, sizeof(audio_dchs_anc_param_t) - header_size);
                                feature_param.type_mask = AM_UART_COSYS_CONTROL_ANC;
                                feature_param.feature_param.anc_param.event = anc_ctrl_param->event_id;
                                memcpy(&feature_param.feature_param.anc_param.cap, &anc_ctrl_param->anc_param.cap, sizeof(audio_anc_control_cap_t));
                                //feature_param.feature_param.anc_param.cap = anc_ctrl_param->anc_param.cap;
                                g_dchs_anc_target_clk = anc_ctrl_param->target_clk;

                                for(uint32_t i = 0; i < 188; i++ ) {
                                    g_anc_biquad_coef[i] = anc_ctrl_param->coef[i];
                                }
                                feature_param.feature_param.anc_param.cap.filter_cap.filter_coef = (void *)&g_anc_biquad_coef;

                                TRANSMITTER_LOG_I("[DCHS ANC][rx cmd] type mask %d, event %d Fevent %d", 3, feature_param.type_mask,
                                                                                                            anc_ctrl_param->event_id,
                                                                                                            feature_param.feature_param.anc_param.event);
                                vPortFree(anc_ctrl_param);
                            } else if(cur_uart_cmd_status.ctrl_type == AUDIO_UART_COSYS_DL_SAMPLE_RATE_SYNC) {
                                audio_dchs_dl_sample_rate_param_t dchs_dl_sample_rate_sync;
                                mcu_uart_rx(UART_AUDIO_CMD, (uint8_t*)&dchs_dl_sample_rate_sync + header_size, sizeof(audio_dchs_dl_sample_rate_param_t) - header_size);
                                g_dchs_dl_sample_rate = dchs_dl_sample_rate_sync.dchs_dl_sample_rate;
                                TRANSMITTER_LOG_I("[DCHS][rx cmd] get dchs dl sample rate sync:%d", 1, g_dchs_dl_sample_rate);
                                reset_cur_cmd_status(cur_uart_cmd_status.ctrl_type);
                                continue;
                            } else if(cur_uart_cmd_status.ctrl_type == AUDIO_UART_COSYS_CMD_ACK) {
                                audio_dchs_ack_param_t dchs_cmd_ack;
                                mcu_uart_rx(UART_AUDIO_CMD, (uint8_t*)&dchs_cmd_ack + header_size, sizeof(audio_dchs_ack_param_t) - header_size);
                                if(dchs_cmd_ack.ack_context == SYNC_SUCCESS){
                                    dchs_set_cmd_ack_status(dchs_cmd_ack.ctrl_type, ACK_RECEIVED);
                                }
                                TRANSMITTER_LOG_I("[DCHS][rx cmd] get dchs cmd:%d, ack:%d", 2, dchs_cmd_ack.ctrl_type, dchs_cmd_ack.ack_context);
                                reset_cur_cmd_status(cur_uart_cmd_status.ctrl_type);
                                continue;
                            }
                            /***************
                             *  extend here
                             * ************/
                            else{
                                AUDIO_ASSERT(0 && "[DCHS][rx cmd] invalid ctrl type");
                            }
                            //send to am task to do flow from irq
                            uart_cmd_type_t cmd_type = dchs_query_any_cmd_waiting_ack();
                            //fix deadlock
                            if(cmd_type && dchs_check_cmd_ack_status(cur_uart_cmd_status.ctrl_type) == ACK_LATER){ //if check any cmd waiting ack, send this waiting action msg to AM queue front
                                dchs_set_cmd_to_am_front(cur_uart_cmd_status.ctrl_type, &feature_param); // set cur cmd to AM queue front flag
                                dchs_set_cmd_ack_status(cmd_type, ACK_WAIT_LATER);
                                bt_sink_srv_am_feature_t ack_param;
                                memset(&ack_param, 0, sizeof(bt_sink_srv_am_feature_t));
                                ack_param.type_mask = AM_UART_COSYS_CONTROL_DL;
                                ack_param.feature_param.cosys_ctrl_dl.ctrl_param.dchs_wait_ack_param.cmd_type = cmd_type;
                                ack_param.feature_param.cosys_ctrl_dl.ctrl_type = AUDIO_UART_COSYS_CMD_WAITING_ACK;
                                ack_param.feature_param.cosys_ctrl_dl.is_send_am_front = true;
                                am_audio_set_feature_ISR(FEATURE_NO_NEED_ID, &ack_param);
                                TRANSMITTER_LOG_I("[DCHS] send waiting ack and cur cmd execute to AM queue front, wait ack cmd type:%d, cur execute cmd type:%d", 2, cmd_type, cur_uart_cmd_status.ctrl_type);
                            }
                            am_audio_set_feature_ISR(FEATURE_NO_NEED_ID, &feature_param);
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

void dchs_dl_set_audio_sample_rate(uint32_t sample_rate)
{
    g_dchs_dl_sample_rate = sample_rate;
    TRANSMITTER_LOG_I("[MCU][DCHS DL] ull host set dchs dl sample rate:%d", 1, g_dchs_dl_sample_rate);
    audio_dchs_dl_sample_rate_param_t dchs_dl_sample_rate_sync_param;
    dchs_dl_sample_rate_sync_param.dchs_dl_sample_rate = sample_rate;
    dchs_dl_sample_rate_sync_param.header.ctrl_type  = AUDIO_UART_COSYS_DL_SAMPLE_RATE_SYNC;
    dchs_dl_sample_rate_sync_param.header.param_size = sizeof(audio_dchs_dl_sample_rate_param_t) - sizeof(audio_uart_cmd_header_t);
    mcu_uart_tx(UART_AUDIO_CMD, (U8 *)&dchs_dl_sample_rate_sync_param, sizeof(audio_dchs_dl_sample_rate_param_t));
}

void audio_transmitter_dchs_open_playback(audio_transmitter_config_t *config, mcu2dsp_open_param_t *open_param)
{
    if(config->scenario_sub_id == AUDIO_TRANSMITTER_DCHS_UART_DL){
        open_param->param.stream_in     = STREAM_IN_UART;
        open_param->param.stream_out    = STREAM_OUT_AFE;
        open_param->audio_scenario_type = AUDIO_SCENARIO_TYPE_DCHS_UART_DL;
        audio_scenario_type_t data_scenario_type              = config->scenario_config.dchs_config.scenario_type;
        //stream in
        open_param->stream_in_param.afe.format                = HAL_AUDIO_PCM_FORMAT_S32_LE;
        open_param->stream_in_param.afe.frame_size            = AUDIO_DURATION_TIME * g_dchs_dl_sample_rate / MUX_UART_BUF_SLICE / 1000;
        open_param->stream_in_param.afe.frame_number          = config->scenario_config.dchs_config.frame_number;
        open_param->stream_in_param.afe.sw_channels           = config->scenario_config.dchs_config.channel_num;
        //open_param->stream_in_param.afe.format                = config->scenario_config.dchs_config.format;
        open_param->stream_in_param.afe.sampling_rate         = g_dchs_dl_sample_rate;
        open_param->stream_in_param.data_dl.scenario_type     = data_scenario_type;
        #ifdef AIR_BT_CODEC_BLE_ENABLED
        if(data_scenario_type == AUDIO_SCENARIO_TYPE_BLE_DL){
            if(config->scenario_config.dchs_config.context_type == AUDIO_CONTENT_TYPE_ULL_BLE){ //ULL
                open_param->stream_in_param.data_dl.scenario_type = AUDIO_SCENARIO_TYPE_BLE_MUSIC_DL;
            }
        }
        #endif
        //stream out
        audio_scenario_sel_t cfg_type = AU_DSP_AUDIO;
        if(data_scenario_type == AUDIO_SCENARIO_TYPE_HFP_DL){
            cfg_type = AU_DSP_VOICE;
        }else if(data_scenario_type == AUDIO_SCENARIO_TYPE_A2DP || data_scenario_type == AUDIO_SCENARIO_TYPE_VP || data_scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_0){
            cfg_type = AU_DSP_AUDIO;
        }else if(data_scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_IN){
            cfg_type = AU_DSP_LINEIN;
        }
        #ifdef AIR_BT_CODEC_BLE_ENABLED
        else if(data_scenario_type == AUDIO_SCENARIO_TYPE_BLE_DL){
            if(config->scenario_config.dchs_config.context_type == AUDIO_CONTENT_TYPE_CONVERSATIONAL){
                cfg_type = AU_DSP_VOICE;
            }else{
                cfg_type = AU_DSP_AUDIO;
            }
        }
        #endif
        hal_audio_get_stream_out_setting_config(cfg_type, &open_param->stream_out_param);
        open_param->stream_out_param.afe.audio_device    = HAL_AUDIO_DEVICE_DAC_DUAL;
        open_param->stream_out_param.afe.stream_channel  = HAL_AUDIO_DIRECT;
        open_param->stream_out_param.afe.memory          = HAL_AUDIO_MEM4;//(config->scenario_config.dchs_config.out_memory == HAL_AUDIO_MEM1) ? HAL_AUDIO_MEM3 : HAL_AUDIO_MEM1;
        open_param->stream_out_param.afe.audio_interface = config->scenario_config.dchs_config.out_interface;
        open_param->stream_out_param.afe.format          = HAL_AUDIO_PCM_FORMAT_S32_LE;
        open_param->stream_out_param.afe.sampling_rate   = g_dchs_dl_sample_rate;//48k or 96k
        open_param->stream_out_param.afe.irq_period      = 0;//config->scenario_config.dchs_config.irq_period;
        open_param->stream_out_param.afe.frame_size      = AUDIO_DURATION_TIME * g_dchs_dl_sample_rate / MUX_UART_BUF_SLICE / 1000;//open_param->stream_in_param.afe.frame_size;
        open_param->stream_out_param.afe.frame_number    = 6;
        open_param->stream_out_param.afe.hw_gain         = false;

        if(config->scenario_config.dchs_config.channel_num == USB_IN_8_CHANNEL){
            open_param->stream_in_param.afe.frame_size        = AUDIO_DURATION_TIME * config->scenario_config.dchs_config.sampling_rate / MUX_UART_BUF_SLICE / 1000;
            open_param->stream_in_param.afe.sampling_rate     = config->scenario_config.dchs_config.sampling_rate;
            open_param->stream_out_param.afe.sampling_rate    = config->scenario_config.dchs_config.sampling_rate;
            open_param->stream_out_param.afe.frame_size       = open_param->stream_in_param.afe.frame_size;
        }
        TRANSMITTER_LOG_I("[MCU][DCHS DL] dchs dl scenario_sub_id %d open, sink memory=%d,cfg_type=%d", 3, config->scenario_sub_id,open_param->stream_out_param.afe.memory,cfg_type);
    }
    if(config->scenario_sub_id == AUDIO_TRANSMITTER_DCHS_UART_UL)
    {
        sysram_status_t status;
        DSP_FEATURE_TYPE_LIST AudioFeatureList_eSCO[2];
        if(config->scenario_config.dchs_config.sampling_rate == 16000){
        AudioFeatureList_eSCO[0] = FUNC_TX_NR;
            AudioFeatureList_eSCO[1] = FUNC_RX_NR;
            AudioFeatureList_eSCO[2] = FUNC_END;
        }else{
            AudioFeatureList_eSCO[0] = FUNC_TX_NR_v2;
        AudioFeatureList_eSCO[1] = FUNC_END;
        }
        audio_nvdm_reset_sysram();
        status = audio_nvdm_set_feature(2, AudioFeatureList_eSCO);
        if (status != NVDM_STATUS_NAT_OK) {
            TRANSMITTER_LOG_E("failed to set parameters to share memory - err(%d)\r\n", 1, status);
            AUDIO_ASSERT(0);
        }
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
        bt_sink_srv_audio_setting_vol_info_t vol_info = {0};
        vol_info.type = VOL_HFP;
        vol_info.vol_info.hfp_vol_info.codec = g_ul_volume_ctrl_param.codec;
        vol_info.vol_info.hfp_vol_info.dev_in = g_ul_volume_ctrl_param.dev_in;
        vol_info.vol_info.hfp_vol_info.dev_out = g_ul_volume_ctrl_param.dev_out;
        vol_info.vol_info.hfp_vol_info.lev_in = g_ul_volume_ctrl_param.lev_in;
        vol_info.vol_info.hfp_vol_info.lev_out = g_ul_volume_ctrl_param.lev_out;
        bt_sink_srv_am_set_volume(STREAM_IN, &vol_info);
        TRANSMITTER_LOG_I("[MCU][DCHS UL] dchs ul scenario_sub_id %d open, source memory=%d", 2, config->scenario_sub_id,open_param->stream_in_param.afe.memory);
    }
}

void audio_transmitter_dchs_start_playback(audio_transmitter_config_t *config, mcu2dsp_start_param_t *start_param)
{
    if(config->scenario_sub_id == AUDIO_TRANSMITTER_DCHS_UART_DL){
        start_param->param.stream_in  = STREAM_IN_UART;
        start_param->param.stream_out = STREAM_OUT_AFE;
        memset((void *)&start_param->stream_in_param,  0, sizeof(mcu2dsp_start_stream_in_param_t));
        memset((void *)&start_param->stream_out_param, 0, sizeof(mcu2dsp_start_stream_out_param_t));

        start_param->stream_out_param.afe.mce_flag           = true; //enable play en
        TRANSMITTER_LOG_I("[MCU][DCHS DL] dchs dl scenario_sub_id %d start", 1, config->scenario_sub_id);
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
    if(dchs_get_device_mode() == DCHS_MODE_SINGLE){
        return;
    }
    audio_dchs_dl_gain_param_t dl_gain_cosys_ctrl_param;
    dual_chip_dl_sync_msg_t dl_mcu2dsp_sync_msg;
    if(hw_gain_index == HAL_AUDIO_STREAM_OUT1){
        if(dchs_get_device_mode() == DCHS_MODE_LEFT){
            dl_gain_cosys_ctrl_param.operation = DCHS_DL_CONFIG_OP_SET_UART_SCENARIO_VOL_INFO;
        }else if(dchs_get_device_mode() == DCHS_MODE_RIGHT){
            dl_mcu2dsp_sync_msg.operation = DCHS_DL_CONFIG_OP_SET_LOCAL_SCENARIO_1_VOL_INFO;
        }
    }else if(hw_gain_index == HAL_AUDIO_STREAM_OUT2){
        if(dchs_get_device_mode() == DCHS_MODE_RIGHT){
            dl_gain_cosys_ctrl_param.operation = DCHS_DL_CONFIG_OP_SET_LOCAL_SCENARIO_2_VOL_INFO;
        }
    }else if(hw_gain_index == HAL_AUDIO_STREAM_OUT3){
        if(dchs_get_device_mode() == DCHS_MODE_LEFT){
            dl_gain_cosys_ctrl_param.operation = DCHS_DL_CONFIG_OP_SET_UART_SCENARIO_VOL_INFO;
        }else if(dchs_get_device_mode() == DCHS_MODE_RIGHT){
            dl_mcu2dsp_sync_msg.operation = DCHS_DL_CONFIG_OP_SET_LOCAL_SCENARIO_1_VOL_INFO;
        }
    }else{
        return;
    }
    TRANSMITTER_LOG_I("[DCHS DL][cmd tx] got and relay gain:0x%x, operation=%d,hw_gain_index:%d,digital_volume_index:%d", 4, dl_gain_cosys_ctrl_param.vol_gain, dl_gain_cosys_ctrl_param.operation, hw_gain_index,digital_volume_index);
    if(dchs_get_device_mode() == DCHS_MODE_LEFT){
        dl_gain_cosys_ctrl_param.vol_gain = digital_volume_index;
        dl_gain_cosys_ctrl_param.header.ctrl_type  = AUDIO_UART_COSYS_DL_SET_GAIN;
        dl_gain_cosys_ctrl_param.header.param_size = sizeof(audio_dchs_dl_gain_param_t) - sizeof(audio_uart_cmd_header_t);
        //relay dchs dl open param to the other chip
        mcu_uart_tx(UART_AUDIO_CMD, (U8 *)&dl_gain_cosys_ctrl_param, sizeof(audio_dchs_dl_gain_param_t));
    }else if(dchs_get_device_mode() == DCHS_MODE_RIGHT){
        dl_mcu2dsp_sync_msg.vol_gain = digital_volume_index;
        dchs_mcu2dsp_msg_sync(DCHS_DL_SET_GAIN_VALUE, &dl_mcu2dsp_sync_msg);//send ccni sync msg to dsp
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
        if(header->ack_type == ACK_NOW || header->ack_type == ACK_LATER){
            uint32_t mask;
            hal_nvic_save_and_set_interrupt_mask(&mask);
            dchs_set_cmd_ack_status(header->ctrl_type, ACK_WAITING);
            hal_nvic_restore_interrupt_mask(mask);
        }
        mux_buffer_t uart_tx_buffer;
        uint32_t tx_size = 0;
        uart_tx_buffer.buf_size = buf_size;
        uart_tx_buffer.p_buf    = param_buf;
        mux_status_t status = mux_tx(audio_cmd_uart_handle, &uart_tx_buffer, 1, &tx_size);
        if (status != MUX_STATUS_OK) {
            TRANSMITTER_LOG_E("[DCHS DL][MUX UART] audio cmd uart tx fail, status=%d, uart_handle = 0x%x", 2, status, audio_cmd_uart_handle);
            AUDIO_ASSERT(0);
            return;
        }
        if(tx_size != buf_size) {
            TRANSMITTER_LOG_E("[DCHS DL][MUX UART] audio cmd uart tx fail, uart_handle = 0x%x, buf_size = %d, already send = %d", 3, audio_cmd_uart_handle, buf_size, tx_size);
            AUDIO_ASSERT(0);
            return;
        }
        TRANSMITTER_LOG_I("[DCHS DL][MUX UART][Tx] audio cmd uart tx success! data_size=%d, cmd type:%d,ack type:%d", 3, tx_size, header->ctrl_type, header->ack_type);
    } else if (type == UART_UL) {
        /* todo */
    } else {
        TRANSMITTER_LOG_E("[DCHS DL][MUX UART] don't support uart type = %d", 1, type);
    }
}

void dchs_dl_mcu2dsp_sync_scenario_status(dchs_dl_chip_role_t chip_role, audio_scenario_type_t scenario_type, bool ctrl)
{
    if(ctrl){
        if(chip_role == LOCAL_CHIP){
            dchs_dl_sync_msg[LOCAL_CHIP].chip_role = chip_role;
            if(dchs_get_device_mode() == DCHS_MODE_RIGHT){
                dchs_dl_sync_msg[LOCAL_CHIP].local_chip_dl_exist = true;
                dchs_mcu2dsp_msg_sync(DCHS_DL_LOCAL_CHIP_DL_EXIST, &dchs_dl_sync_msg[LOCAL_CHIP]);
            }else if(dchs_get_device_mode() == DCHS_MODE_LEFT){
                dchs_dl_sync_msg[LOCAL_CHIP].other_chip_dl_exist = true;
                dchs_mcu2dsp_msg_sync(DCHS_DL_OTHER_CHIP_DL_EXIST, &dchs_dl_sync_msg[LOCAL_CHIP]);
            }
        }else{
            dchs_dl_sync_msg[OTHER_CHIP].other_chip_dl_exist = true;
            dchs_dl_sync_msg[OTHER_CHIP].chip_role = chip_role;
            dchs_mcu2dsp_msg_sync(DCHS_DL_OTHER_CHIP_DL_EXIST, &dchs_dl_sync_msg[OTHER_CHIP]);//send ccni sync msg to dsp
        }
    }else{
        dual_chip_dl_sync_msg_t dl_mcu2dsp_sync_msg;
        dl_mcu2dsp_sync_msg.data_scenario_type = scenario_type;
        dl_mcu2dsp_sync_msg.sample_rate = 0;
        dl_mcu2dsp_sync_msg.format_bytes = 0;
        dl_mcu2dsp_sync_msg.chip_role = chip_role;
        if(chip_role == LOCAL_CHIP){
            if(dchs_get_device_mode() == DCHS_MODE_RIGHT){
                dl_mcu2dsp_sync_msg.local_chip_dl_exist = false;
                dchs_mcu2dsp_msg_sync(DCHS_DL_LOCAL_CHIP_DL_EXIST, &dl_mcu2dsp_sync_msg);
            }else if(dchs_get_device_mode() == DCHS_MODE_LEFT && !dchs_dl_check_scenario_exist_flag(OTHER_CHIP) && !dchs_dl_check_scenario_exist_flag(LOCAL_CHIP)){
                dl_mcu2dsp_sync_msg.other_chip_dl_exist = false;
                dchs_mcu2dsp_msg_sync(DCHS_DL_OTHER_CHIP_DL_EXIST, &dl_mcu2dsp_sync_msg);
            }
        }else{
            dl_mcu2dsp_sync_msg.other_chip_dl_exist = false;
            if(dchs_get_device_mode() == DCHS_MODE_RIGHT){
                dchs_mcu2dsp_msg_sync(DCHS_DL_OTHER_CHIP_DL_EXIST, &dl_mcu2dsp_sync_msg);//send ccni sync msg to dsp
            }else if(dchs_get_device_mode() == DCHS_MODE_LEFT && !dchs_dl_check_scenario_exist_flag(LOCAL_CHIP) && !dchs_dl_check_scenario_exist_flag(OTHER_CHIP)){
                dchs_mcu2dsp_msg_sync(DCHS_DL_OTHER_CHIP_DL_EXIST, &dl_mcu2dsp_sync_msg);//send ccni sync msg to dsp
            }
        }
    }
}

void dchs_cosys_ctrl_cmd_execute(audio_dchs_cosys_ctrl_t * cosys_ctrl)
{
    TRANSMITTER_LOG_I("[DCHS][MCU CMD Execute]:ctrl_type:%d,scenario_type:%d", 2, cosys_ctrl->ctrl_type,cosys_ctrl->ctrl_param.dchs_dl_open_param.scenario_type);
    if(cosys_ctrl->ctrl_type == AUDIO_UART_COSYS_DL_OPEN){
        dchs_dl_chip_role_t   chip_role = cosys_ctrl->ctrl_param.dchs_dl_open_param.chip_role;
        audio_scenario_type_t scenario_type = cosys_ctrl->ctrl_param.dchs_dl_open_param.scenario_type;
        memset(&transmitter_config_dl, 0, sizeof(audio_transmitter_config_t));
        transmitter_config_dl.scenario_type = AUDIO_TRANSMITTER_DCHS;
        transmitter_config_dl.scenario_sub_id = AUDIO_TRANSMITTER_DCHS_UART_DL;

        if(scenario_type == AUDIO_SCENARIO_TYPE_A2DP){
            if(cosys_ctrl->ctrl_param.dchs_dl_open_param.a2dp_codec_type == BT_A2DP_CODEC_SBC){
                transmitter_config_dl.scenario_config.dchs_config.frame_size = 128;
            }else if(cosys_ctrl->ctrl_param.dchs_dl_open_param.a2dp_codec_type == BT_A2DP_CODEC_AAC){
                transmitter_config_dl.scenario_config.dchs_config.frame_size = 1024;
            }else if(cosys_ctrl->ctrl_param.dchs_dl_open_param.a2dp_codec_type == BT_A2DP_CODEC_VENDOR){
                transmitter_config_dl.scenario_config.dchs_config.frame_size = (cosys_ctrl->ctrl_param.dchs_dl_open_param.sampling_rate <= 48000) ? 128 : 256;
            }
        }else {
            transmitter_config_dl.scenario_config.dchs_config.frame_size = cosys_ctrl->ctrl_param.dchs_dl_open_param.frame_size;
        }
        transmitter_config_dl.scenario_config.dchs_config.out_interface = cosys_ctrl->ctrl_param.dchs_dl_open_param.out_interface;
        transmitter_config_dl.scenario_config.dchs_config.sampling_rate = cosys_ctrl->ctrl_param.dchs_dl_open_param.sampling_rate;
        transmitter_config_dl.scenario_config.dchs_config.irq_period    = cosys_ctrl->ctrl_param.dchs_dl_open_param.irq_period;
        transmitter_config_dl.scenario_config.dchs_config.frame_number  = cosys_ctrl->ctrl_param.dchs_dl_open_param.frame_number;
        transmitter_config_dl.scenario_config.dchs_config.format        = cosys_ctrl->ctrl_param.dchs_dl_open_param.format;
        transmitter_config_dl.scenario_config.dchs_config.out_memory    = cosys_ctrl->ctrl_param.dchs_dl_open_param.out_memory;
        transmitter_config_dl.scenario_config.dchs_config.scenario_type = cosys_ctrl->ctrl_param.dchs_dl_open_param.scenario_type;
        transmitter_config_dl.scenario_config.dchs_config.channel_num   = cosys_ctrl->ctrl_param.dchs_dl_open_param.channel_num;
        transmitter_config_dl.scenario_config.dchs_config.context_type  = cosys_ctrl->ctrl_param.dchs_dl_open_param.context_type;
        if(dchs_get_device_mode() == DCHS_MODE_LEFT){  // master latch slave clock
            if(!latch_timer_start_flag){
                mux_ll_latch_timer_start(false, 101);
                latch_timer_start_flag = true;
                TRANSMITTER_LOG_I("[DCHS DL][MCU CMD Execute] mcu dl latch timer started",0);
            }
        }
        dual_chip_dl_sync_msg_t dl_mcu2dsp_sync_msg;
        dl_mcu2dsp_sync_msg.sample_rate  = transmitter_config_dl.scenario_config.dchs_config.sampling_rate;
        dl_mcu2dsp_sync_msg.format_bytes = transmitter_config_dl.scenario_config.dchs_config.format >= HAL_AUDIO_PCM_FORMAT_S24_LE ? 4 : 2;
        dl_mcu2dsp_sync_msg.data_scenario_type  = scenario_type;
        dl_mcu2dsp_sync_msg.channel_num  = transmitter_config_dl.scenario_config.dchs_config.channel_num;
        dl_mcu2dsp_sync_msg.frame_size   = transmitter_config_dl.scenario_config.dchs_config.frame_size;
        memcpy(&dchs_dl_sync_msg[chip_role], &dl_mcu2dsp_sync_msg, sizeof(dual_chip_dl_sync_msg_t));
        TRANSMITTER_LOG_I("[DCHS DL][MCU CMD Execute]get open event, scenario type=%d,context_type=0x%x,frame_size=%d,chanel_num=%d,irq_period=%d,frame_number=%d,format=%d,out_memory=%d,sampling_rate=%d", 9,
                            scenario_type,
                            transmitter_config_dl.scenario_config.dchs_config.context_type,
                            transmitter_config_dl.scenario_config.dchs_config.frame_size,
                            transmitter_config_dl.scenario_config.dchs_config.channel_num,
                            transmitter_config_dl.scenario_config.dchs_config.irq_period,
                            transmitter_config_dl.scenario_config.dchs_config.frame_number,
                            transmitter_config_dl.scenario_config.dchs_config.format,
                            transmitter_config_dl.scenario_config.dchs_config.out_memory,
                            transmitter_config_dl.scenario_config.dchs_config.sampling_rate);
        if(dchs_dl_check_scenario_exist_flag(LOCAL_CHIP) || dchs_dl_check_scenario_exist_flag(OTHER_CHIP)){
            dchs_dl_set_scenario_exist_flag(chip_role, scenario_type, true);
            dchs_dl_status.next_running_scenario_type = scenario_type;
            if(scenario_type == AUDIO_SCENARIO_TYPE_BLE_DL){
                dchs_dl_status.next_ble_context_type  = cosys_ctrl->ctrl_param.dchs_dl_open_param.context_type;
            }
            TRANSMITTER_LOG_I("[DCHS DL][MCU CMD Execute] scenario type:%d,context type:0x%x is running, ignore open scenario_type:%d,context type=0x%x", 4, dchs_dl_status.cur_running_scenario_type, dchs_dl_status.cur_ble_context_type, scenario_type,cosys_ctrl->ctrl_param.dchs_dl_open_param.context_type);
            if(dchs_check_cmd_ack_status(cosys_ctrl->ctrl_type) == ACK_WAITING){
                dchs_loop_waiting_cmd_ack(cosys_ctrl->ctrl_type);
            }else if(dchs_check_cmd_ack_status(cosys_ctrl->ctrl_type) == ACK_LATER){
                dchs_response_cmd_ack(cosys_ctrl->ctrl_type, SYNC_SUCCESS);
            }
            return;
        }
        dchs_dl_set_scenario_exist_flag(chip_role, scenario_type, true);
        //lock DVFS MID
        hal_dvfs_lock_control(HAL_DVFS_OPP_MID, HAL_DVFS_LOCK);
        //lock bt sleep
        if(!dchs_dl_status.bt_lock_flag){
            DCHS_pka_lock_bt_sleep(dchs_bt_lock_sleep_done_callback, NULL);
            dchs_dl_status.bt_lock_flag = true;
        }
        //open flow
        void *p_param_share;
        mcu2dsp_open_param_t open_param = {0};
        audio_transmitter_dchs_open_playback(&transmitter_config_dl, &open_param);
        ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_DCHS_UART_DL, &open_param, true);
        p_param_share = hal_audio_dsp_controller_put_paramter(&open_param, sizeof(mcu2dsp_open_param_t), AUDIO_MESSAGE_TYPE_AUDIO_TRANSMITTER);
        uint16_t scenario_and_id = ((AUDIO_TRANSMITTER_DCHS) << 8) + AUDIO_TRANSMITTER_DCHS_UART_DL;
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_AUDIO_TRANSMITTER_OPEN, scenario_and_id, (uint32_t)p_param_share, true);
        TRANSMITTER_LOG_I("[DCHS DL][MCU CMD Execute]: dl open done,scenario_type=%d, context type=0x%x,chip_role=%d", 3, scenario_type, cosys_ctrl->ctrl_param.dchs_dl_open_param.context_type,chip_role);
        //start flow
        mcu2dsp_start_param_t start_param = {0};
        audio_transmitter_dchs_start_playback(&transmitter_config_dl, &start_param);
        p_param_share = hal_audio_dsp_controller_put_paramter(&start_param, sizeof(mcu2dsp_start_param_t), AUDIO_MESSAGE_TYPE_AUDIO_TRANSMITTER);
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_AUDIO_TRANSMITTER_START, scenario_and_id, (uint32_t)p_param_share, true);
        TRANSMITTER_LOG_I("[DCHS DL][MCU CMD Execute]: dl start done,scenario_type=%d,context type=0x%x, chip_role=%d", 3, scenario_type,cosys_ctrl->ctrl_param.dchs_dl_open_param.context_type, chip_role);

        dchs_dl_status.is_running    = true;
        dchs_dl_status.cur_running_scenario_type = scenario_type;
        if(scenario_type == AUDIO_SCENARIO_TYPE_BLE_DL){
            dchs_dl_status.cur_ble_context_type  = cosys_ctrl->ctrl_param.dchs_dl_open_param.context_type;
        }
        if(dchs_check_cmd_ack_status(cosys_ctrl->ctrl_type) == ACK_WAITING){
            dchs_loop_waiting_cmd_ack(cosys_ctrl->ctrl_type);
        }else if(dchs_check_cmd_ack_status(cosys_ctrl->ctrl_type) == ACK_LATER){
            dchs_response_cmd_ack(cosys_ctrl->ctrl_type, SYNC_SUCCESS);
        }
    }else if(cosys_ctrl->ctrl_type == AUDIO_UART_COSYS_DL_START){
        audio_scenario_type_t scenario_type = cosys_ctrl->ctrl_param.dchs_dl_start_param.scenario_type;
        dchs_dl_chip_role_t   chip_role = cosys_ctrl->ctrl_param.dchs_dl_start_param.chip_role;
        //mcu2dsp ccni sync msg
        dchs_dl_mcu2dsp_sync_scenario_status(chip_role, scenario_type, true);
        TRANSMITTER_LOG_I("[DCHS DL][MCU CMD Execute]get start event, scenario type=%d,context_type:0x%x", 2, scenario_type, cosys_ctrl->ctrl_param.dchs_dl_start_param.context_type);
        //start flow
        bool set_play_en = false;
        if(scenario_type == AUDIO_SCENARIO_TYPE_BLE_DL){
            if(dchs_dl_status.is_running && dchs_dl_status.cur_running_scenario_type == scenario_type && dchs_dl_status.cur_ble_context_type == cosys_ctrl->ctrl_param.dchs_dl_start_param.context_type){
                set_play_en = true;
            }
        }else{
            if(dchs_dl_status.is_running && dchs_dl_status.cur_running_scenario_type == scenario_type){
                set_play_en = true;
            }
        }
        if(dchs_check_cmd_ack_status(cosys_ctrl->ctrl_type) == ACK_WAITING){
            dchs_loop_waiting_cmd_ack(cosys_ctrl->ctrl_type);
        }else if(dchs_check_cmd_ack_status(cosys_ctrl->ctrl_type) == ACK_LATER){
            dchs_response_cmd_ack(cosys_ctrl->ctrl_type, SYNC_SUCCESS);
        }
        if(set_play_en){
            //send ccni msg set play en
            if(chip_role == LOCAL_CHIP && (scenario_type == AUDIO_SCENARIO_TYPE_VP || scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_IN
                || scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_0)){
                if(dchs_check_cmd_ack_status(cosys_ctrl->ctrl_type) != ACK_WAIT_LATER){
                    dual_chip_dl_sync_msg_t dl_mcu2dsp_sync_msg;
                    dl_mcu2dsp_sync_msg.data_scenario_type = scenario_type;
                    dchs_mcu2dsp_msg_sync(DCHS_DL_SET_PLAY_EN, &dl_mcu2dsp_sync_msg);
                }else{
                    dchs_dl_status.wait_set_play_en = true;
                    TRANSMITTER_LOG_I("[DCHS DL][MCU CMD Execute] wait set play en, scenario type:%d", 1, scenario_type);
                }
            }
        }else{
            TRANSMITTER_LOG_I("[DCHS DL][MCU CMD Execute] scenario type:%d,context type:0x%x is running, ignore start scenario_type:%d,context type=0x%x", 4, dchs_dl_status.cur_running_scenario_type, dchs_dl_status.cur_ble_context_type, scenario_type,cosys_ctrl->ctrl_param.dchs_dl_start_param.context_type);
        }
    }else if(cosys_ctrl->ctrl_type == AUDIO_UART_COSYS_DL_CLOSE){
        audio_scenario_type_t scenario_type = cosys_ctrl->ctrl_param.dchs_dl_close_param.scenario_type;
        dchs_dl_chip_role_t   chip_role     = cosys_ctrl->ctrl_param.dchs_dl_close_param.chip_role;
        dchs_dl_set_scenario_exist_flag(chip_role, scenario_type, false);
        //send ccni
        dchs_dl_mcu2dsp_sync_scenario_status(chip_role, scenario_type, false);
        if(dchs_dl_check_scenario_exist_flag(LOCAL_CHIP) || dchs_dl_check_scenario_exist_flag(OTHER_CHIP)){
            if(scenario_type == AUDIO_SCENARIO_TYPE_BLE_DL){
                if(dchs_dl_status.is_running && dchs_dl_status.cur_running_scenario_type == scenario_type && dchs_dl_status.cur_ble_context_type == cosys_ctrl->ctrl_param.dchs_dl_close_param.context_type){
                    dchs_dl_status.cur_running_scenario_type = dchs_dl_status.next_running_scenario_type;
                    dchs_dl_status.cur_ble_context_type      = dchs_dl_status.next_ble_context_type;
                }
            }else{
                if(dchs_dl_status.is_running && dchs_dl_status.cur_running_scenario_type == scenario_type){
                    dchs_dl_status.cur_running_scenario_type = dchs_dl_status.next_running_scenario_type;
                    if(dchs_dl_status.next_running_scenario_type == AUDIO_SCENARIO_TYPE_BLE_DL){
                        dchs_dl_status.cur_ble_context_type      = dchs_dl_status.next_ble_context_type;
                    }
                }
            }
            TRANSMITTER_LOG_W("[DCHS DL][MCU CMD Execute] the scenario:%d,context type:0x%x exist, ignore stop,scenario_type=%d, context type=0x%x,other:%d,local=%d", 6, dchs_dl_status.cur_running_scenario_type,dchs_dl_status.cur_ble_context_type, scenario_type,cosys_ctrl->ctrl_param.dchs_dl_close_param.context_type, dchs_dl_check_scenario_exist_flag(OTHER_CHIP),dchs_dl_check_scenario_exist_flag(LOCAL_CHIP));
            if(dchs_check_cmd_ack_status(cosys_ctrl->ctrl_type) == ACK_WAITING){
                dchs_loop_waiting_cmd_ack(cosys_ctrl->ctrl_type);
            }else if(dchs_check_cmd_ack_status(cosys_ctrl->ctrl_type) == ACK_LATER){
                dchs_response_cmd_ack(cosys_ctrl->ctrl_type, SYNC_SUCCESS);
            }
            return;
        }
        if(dchs_dl_status.is_running == false){
            TRANSMITTER_LOG_I("[DCHS DL][MCU CMD Execute] dl already close,no need close",0);
            if(dchs_check_cmd_ack_status(cosys_ctrl->ctrl_type) == ACK_WAITING){
                dchs_loop_waiting_cmd_ack(cosys_ctrl->ctrl_type);
            }else if(dchs_check_cmd_ack_status(cosys_ctrl->ctrl_type) == ACK_LATER){
                dchs_response_cmd_ack(cosys_ctrl->ctrl_type, SYNC_SUCCESS);
            }
            return;
        }
        if(dchs_get_device_mode() == DCHS_MODE_LEFT){
            if(latch_timer_start_flag){
                mux_ll_latch_timer_stop();
                latch_timer_start_flag = false;
                TRANSMITTER_LOG_I("[DCHS DL][MCU CMD Execute] mcu dl latch timer stoped",0);
            }
        }
        //unlock DVFS MID
        hal_dvfs_lock_control(HAL_DVFS_OPP_MID, HAL_DVFS_UNLOCK);
        //if no unlock bt sleep, need unlock
        if(dchs_dl_status.bt_lock_flag){
            DCHS_pka_unlock_bt_sleep();
            dchs_dl_status.bt_lock_flag = false;
            TRANSMITTER_LOG_I("[DCHS DL][MCU CMD Execute] bt unlock sleep done",0);
        }
        ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_DCHS_UART_DL, NULL, false);
        //stop
        uint16_t scenario_and_id = ((AUDIO_TRANSMITTER_DCHS) << 8) + AUDIO_TRANSMITTER_DCHS_UART_DL;
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_AUDIO_TRANSMITTER_STOP, scenario_and_id, 0, true);
        //close
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_AUDIO_TRANSMITTER_CLOSE, scenario_and_id, 0, true);
        TRANSMITTER_LOG_I("[DCHS DL][MCU CMD Execute]: dl stop done,scenario_type=%d, context_type=0x%x,chip_role=%d", 3, scenario_type, cosys_ctrl->ctrl_param.dchs_dl_close_param.context_type, chip_role);
        dchs_dl_status.is_running = false;
        if(dchs_check_cmd_ack_status(cosys_ctrl->ctrl_type) == ACK_WAITING){
            dchs_loop_waiting_cmd_ack(cosys_ctrl->ctrl_type);
        }else if(dchs_check_cmd_ack_status(cosys_ctrl->ctrl_type) == ACK_LATER){
            dchs_response_cmd_ack(cosys_ctrl->ctrl_type, SYNC_SUCCESS);
        }
    }else if(cosys_ctrl->ctrl_type == AUDIO_UART_COSYS_DL_SET_GAIN) {
        dual_chip_dl_sync_msg_t dl_mcu2dsp_sync_msg;
        dl_mcu2dsp_sync_msg.operation  = cosys_ctrl->ctrl_param.dchs_dl_gain_param.operation;
        dl_mcu2dsp_sync_msg.vol_gain   = cosys_ctrl->ctrl_param.dchs_dl_gain_param.vol_gain;
        TRANSMITTER_LOG_I("[DCHS DL][MCU CMD Execute] get gain:0x%x, operation=%d", 2, dl_mcu2dsp_sync_msg.vol_gain, dl_mcu2dsp_sync_msg.operation);
        dchs_mcu2dsp_msg_sync(DCHS_DL_SET_GAIN_VALUE, &dl_mcu2dsp_sync_msg);//send ccni sync msg to dsp

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
        transmitter_config_ul.scenario_config.dchs_config.irq_period    = cosys_ctrl->ctrl_param.dchs_ul_open_param.irq_period;
        transmitter_config_ul.scenario_config.dchs_config.frame_number  = cosys_ctrl->ctrl_param.dchs_ul_open_param.frame_number;
        transmitter_config_ul.scenario_config.dchs_config.format        = cosys_ctrl->ctrl_param.dchs_ul_open_param.format;

        am_audio_side_tone_enable();
        dchs_lock_bt_sleep(false);
        //ul open flow
        void *p_param_share;
        mcu2dsp_open_param_t open_param = {0};
        audio_transmitter_dchs_open_playback(&transmitter_config_ul, &open_param);
        ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_DCHS_UART_UL, &open_param, true);
        p_param_share = hal_audio_dsp_controller_put_paramter(&open_param, sizeof(mcu2dsp_open_param_t), AUDIO_MESSAGE_TYPE_AUDIO_TRANSMITTER);
        uint16_t scenario_and_id = ((AUDIO_TRANSMITTER_DCHS) << 8) + AUDIO_TRANSMITTER_DCHS_UART_UL;
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_AUDIO_TRANSMITTER_OPEN, scenario_and_id, (uint32_t)p_param_share, true);
        TRANSMITTER_LOG_I("[DCHS][MCU CMD Execute]: ul scenario type %d open config done", 1, cosys_ctrl->ctrl_param.dchs_ul_open_param.scenario_type);
        if(dchs_check_cmd_ack_status(cosys_ctrl->ctrl_type) == ACK_WAITING){
            dchs_loop_waiting_cmd_ack(cosys_ctrl->ctrl_type);
        }else if(dchs_check_cmd_ack_status(cosys_ctrl->ctrl_type) == ACK_LATER){
            dchs_response_cmd_ack(cosys_ctrl->ctrl_type, SYNC_SUCCESS);
        }
    }else if(cosys_ctrl->ctrl_type == AUDIO_UART_COSYS_UL_START){
        mcu2dsp_start_param_t start_param = {0};
        void *p_param_share;
        uint16_t scenario_and_id = ((AUDIO_TRANSMITTER_DCHS) << 8) + AUDIO_TRANSMITTER_DCHS_UART_UL;
        audio_transmitter_dchs_start_playback(&transmitter_config_dl, &start_param);
        p_param_share = hal_audio_dsp_controller_put_paramter(&start_param, sizeof(mcu2dsp_start_param_t), AUDIO_MESSAGE_TYPE_AUDIO_TRANSMITTER);
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_AUDIO_TRANSMITTER_START, scenario_and_id, (uint32_t)p_param_share, true);
        
        TRANSMITTER_LOG_I("[DCHS UL][MCU CMD Execute] ul start success,frame_size=%d,irq_period=%d,frame_number=%d,format=%d,sampling_rate=%d", 5,
                            transmitter_config_ul.scenario_config.dchs_config.frame_size,
                            transmitter_config_ul.scenario_config.dchs_config.irq_period,
                            transmitter_config_ul.scenario_config.dchs_config.frame_number,
                            transmitter_config_ul.scenario_config.dchs_config.format,
                            transmitter_config_ul.scenario_config.dchs_config.sampling_rate);
        if(dchs_check_cmd_ack_status(cosys_ctrl->ctrl_type) == ACK_WAITING){
            dchs_loop_waiting_cmd_ack(cosys_ctrl->ctrl_type);
        }else if(dchs_check_cmd_ack_status(cosys_ctrl->ctrl_type) == ACK_LATER){
            dchs_response_cmd_ack(cosys_ctrl->ctrl_type, SYNC_SUCCESS);
        }
    }else if(cosys_ctrl->ctrl_type == AUDIO_UART_COSYS_UL_CLOSE){
        am_audio_side_tone_disable();
        uint16_t scenario_and_id = ((AUDIO_TRANSMITTER_DCHS) << 8) + AUDIO_TRANSMITTER_DCHS_UART_UL;
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_AUDIO_TRANSMITTER_STOP, scenario_and_id, 0, true);
        //close
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_AUDIO_TRANSMITTER_CLOSE, scenario_and_id, 0, true);
        ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_DCHS_UART_UL, NULL, false);
        hal_dvfs_lock_control(HFP_DVFS_INHOUSE, HAL_DVFS_UNLOCK);
        if(dchs_check_cmd_ack_status(cosys_ctrl->ctrl_type) == ACK_WAITING){
            dchs_loop_waiting_cmd_ack(cosys_ctrl->ctrl_type);
        }else if(dchs_check_cmd_ack_status(cosys_ctrl->ctrl_type) == ACK_LATER){
            dchs_response_cmd_ack(cosys_ctrl->ctrl_type, SYNC_SUCCESS);
        }
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

    } else if (cosys_ctrl->ctrl_type == AUDIO_UART_COSYS_CMD_WAITING_ACK) {
        uart_cmd_type_t cmd_type = cosys_ctrl->ctrl_param.dchs_wait_ack_param.cmd_type;
        TRANSMITTER_LOG_I("[DCHS][MCU CMD Execute]: dchs waiting ack, cmd_type:%d, ack status:%d", 2, cmd_type, dchs_check_cmd_ack_status(cmd_type));
        if(dchs_check_cmd_ack_status(cmd_type) == ACK_WAIT_LATER){
            uint32_t mask;
            hal_nvic_save_and_set_interrupt_mask(&mask);
            dchs_set_cmd_ack_status(cmd_type, ACK_WAITING);
            hal_nvic_restore_interrupt_mask(mask);
            dchs_loop_waiting_cmd_ack(cmd_type);
        }
        if(dchs_dl_status.wait_set_play_en){
            dchs_dl_status.wait_set_play_en = false;
            dual_chip_dl_sync_msg_t dl_mcu2dsp_sync_msg;
            dl_mcu2dsp_sync_msg.data_scenario_type = dchs_dl_status.cur_running_scenario_type;
            dchs_mcu2dsp_msg_sync(DCHS_DL_SET_PLAY_EN, &dl_mcu2dsp_sync_msg);
        } 
    }
}

void dchs_cosys_ctrl_cmd_relay(uart_cmd_type_t ctrl_type, audio_scenario_type_t scenario_type, mcu2dsp_open_param_t *open_param, mcu2dsp_start_param_t * start_param)
{
    if(dchs_get_device_mode() == DCHS_MODE_SINGLE){
        return;
    }
    TRANSMITTER_LOG_I("[DCHS][CMD Relay]ctrl_type:%d, scenario_type:%d",2,ctrl_type,scenario_type);
    if(ctrl_type == AUDIO_UART_COSYS_DL_OPEN){
        if (scenario_type == AUDIO_SCENARIO_TYPE_HFP_DL || scenario_type == AUDIO_SCENARIO_TYPE_A2DP || scenario_type == AUDIO_SCENARIO_TYPE_BLE_DL || scenario_type == AUDIO_SCENARIO_TYPE_VP
            || scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_IN || scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_0){ // [hard code]
            audio_dchs_dl_open_param_t dl_open_cosys_ctrl_param;
            memset(&dl_open_cosys_ctrl_param, 0, sizeof(audio_dchs_dl_open_param_t));

            dl_open_cosys_ctrl_param.scenario_type = scenario_type;
            if(scenario_type == AUDIO_SCENARIO_TYPE_A2DP){
                bt_codec_a2dp_audio_t * codec_info = (bt_codec_a2dp_audio_t*)(&(open_param->stream_in_param.a2dp.codec_info));
                dl_open_cosys_ctrl_param.a2dp_codec_type = codec_info->codec_cap.type;
                TRANSMITTER_LOG_I("[DCHS DL] cpy done,a2dp codec type=%d", 1, dl_open_cosys_ctrl_param.a2dp_codec_type);
            }
            dl_open_cosys_ctrl_param.in_memory     = open_param->stream_in_param.afe.memory;
            dl_open_cosys_ctrl_param.in_interface  = open_param->stream_in_param.afe.audio_interface;
            #ifdef AIR_BT_CODEC_BLE_ENABLED
            dl_open_cosys_ctrl_param.context_type  = open_param->stream_in_param.ble.context_type;
            if(scenario_type == AUDIO_SCENARIO_TYPE_BLE_DL){
                ble_context_type[LOCAL_CHIP]       = open_param->stream_in_param.ble.context_type;
            }
            #endif
            dl_open_cosys_ctrl_param.channel_num   = open_param->stream_in_param.data_dl.scenario_param.wired_audio_param.codec_param.pcm.channel_mode;
            dl_open_cosys_ctrl_param.out_memory    = open_param->stream_out_param.afe.memory;
            dl_open_cosys_ctrl_param.out_interface = open_param->stream_out_param.afe.audio_interface;
            dl_open_cosys_ctrl_param.sampling_rate = open_param->stream_out_param.afe.sampling_rate;
            dl_open_cosys_ctrl_param.frame_size    = open_param->stream_out_param.afe.frame_size;
            dl_open_cosys_ctrl_param.frame_number  = open_param->stream_out_param.afe.frame_number;
            dl_open_cosys_ctrl_param.format        = open_param->stream_out_param.afe.format;
            dl_open_cosys_ctrl_param.irq_period    = open_param->stream_out_param.afe.irq_period;
            dl_open_cosys_ctrl_param.chip_role     = LOCAL_CHIP;
            dl_open_cosys_ctrl_param.header.ctrl_type  = ctrl_type;
            dl_open_cosys_ctrl_param.header.ack_type   = ACK_LATER;
            dl_open_cosys_ctrl_param.header.param_size = sizeof(audio_dchs_dl_open_param_t) - sizeof(audio_uart_cmd_header_t);
            //relay dchs dl open param to the other chip
            mcu_uart_tx(UART_AUDIO_CMD, (U8 *)&dl_open_cosys_ctrl_param, sizeof(audio_dchs_dl_open_param_t));
            TRANSMITTER_LOG_I("[DCHS DL] relay dl open param, scenario_type=%d, context_type=0x%x", 2, scenario_type,ble_context_type[LOCAL_CHIP]);
            //config local chip open param
            audio_dchs_cosys_ctrl_t  cosys_ctrl;
            cosys_ctrl.ctrl_type  = ctrl_type;
            cosys_ctrl.ctrl_param.dchs_dl_open_param = dl_open_cosys_ctrl_param;
            dchs_cosys_ctrl_cmd_execute(&cosys_ctrl);
        }
    }else if (ctrl_type == AUDIO_UART_COSYS_DL_START){
        if (scenario_type == AUDIO_SCENARIO_TYPE_HFP_DL || scenario_type == AUDIO_SCENARIO_TYPE_A2DP || scenario_type == AUDIO_SCENARIO_TYPE_BLE_DL || scenario_type == AUDIO_SCENARIO_TYPE_VP
            || scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_IN || scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_0){ // [hard code]
            audio_dchs_dl_start_param_t dl_start_cosys_ctrl_param;
            memset(&dl_start_cosys_ctrl_param, 0, sizeof(audio_dchs_dl_start_param_t));

            dl_start_cosys_ctrl_param.scenario_type      = scenario_type;
            dl_start_cosys_ctrl_param.chip_role          = LOCAL_CHIP;
            if(scenario_type == AUDIO_SCENARIO_TYPE_BLE_DL){
                dl_start_cosys_ctrl_param.context_type = ble_context_type[LOCAL_CHIP];
            }
            dl_start_cosys_ctrl_param.header.ctrl_type   = ctrl_type;
            dl_start_cosys_ctrl_param.header.ack_type    = ACK_LATER;
            dl_start_cosys_ctrl_param.header.param_size  = sizeof(audio_dchs_dl_start_param_t) - sizeof(audio_uart_cmd_header_t);
            //relay dchs dl start param to the other chip
            mcu_uart_tx(UART_AUDIO_CMD, (U8 *)&dl_start_cosys_ctrl_param, sizeof(audio_dchs_dl_start_param_t));
            TRANSMITTER_LOG_I("[DCHS DL] relay dl start param cmd success, scenario_type=%d,context type=0x%x", 2, scenario_type,ble_context_type[LOCAL_CHIP]);
            //create local chip dchs dl path
            audio_dchs_cosys_ctrl_t  cosys_ctrl;
            cosys_ctrl.ctrl_type  = ctrl_type;
            cosys_ctrl.ctrl_param.dchs_dl_start_param = dl_start_cosys_ctrl_param;
            dchs_cosys_ctrl_cmd_execute(&cosys_ctrl);
        }
    }else if(ctrl_type == AUDIO_UART_COSYS_DL_CLOSE){
        if (scenario_type == AUDIO_SCENARIO_TYPE_HFP_DL || scenario_type == AUDIO_SCENARIO_TYPE_A2DP || scenario_type == AUDIO_SCENARIO_TYPE_BLE_DL || scenario_type == AUDIO_SCENARIO_TYPE_VP
            || scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_IN || scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_0){ // [hard code]
            audio_dchs_dl_close_param_t dl_close_cosys_ctrl_param;
            memset(&dl_close_cosys_ctrl_param, 0, sizeof(audio_dchs_dl_close_param_t));

            dl_close_cosys_ctrl_param.scenario_type = scenario_type;
            dl_close_cosys_ctrl_param.header.ctrl_type  = ctrl_type;
            dl_close_cosys_ctrl_param.header.ack_type   = ACK_LATER;
            if(scenario_type == AUDIO_SCENARIO_TYPE_BLE_DL){
                dl_close_cosys_ctrl_param.context_type = ble_context_type[LOCAL_CHIP];
            }
            dl_close_cosys_ctrl_param.header.param_size = sizeof(audio_dchs_dl_close_param_t) - sizeof(audio_uart_cmd_header_t);
            dl_close_cosys_ctrl_param.chip_role         = LOCAL_CHIP;

            mcu_uart_tx(UART_AUDIO_CMD, (U8 *)&dl_close_cosys_ctrl_param, sizeof(audio_dchs_dl_close_param_t));
            TRANSMITTER_LOG_I("[DCHS DL] relay dl stop param ,scenario_type=%d,context_type=0x%x", 2, scenario_type, ble_context_type[LOCAL_CHIP]);

            hal_gpio_set_output(HAL_GPIO_17, HAL_GPIO_DATA_LOW);
            audio_dchs_cosys_ctrl_t  cosys_ctrl;
            cosys_ctrl.ctrl_type  = ctrl_type;
            cosys_ctrl.ctrl_param.dchs_dl_close_param = dl_close_cosys_ctrl_param;
            dchs_cosys_ctrl_cmd_execute(&cosys_ctrl);
        }
    } else if(ctrl_type == AUDIO_UART_COSYS_UL_OPEN) {
        if (scenario_type == AUDIO_SCENARIO_TYPE_HFP_UL || scenario_type == AUDIO_SCENARIO_TYPE_BLE_UL || scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_OUT || scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_OUT){ // [hard code]
            audio_dchs_ul_open_param_t ul_open_cosys_ctrl_param;
            memset(&ul_open_cosys_ctrl_param, 0, sizeof(audio_dchs_ul_open_param_t));
            dchs_lock_bt_sleep(false);
            ul_open_cosys_ctrl_param.scenario_type = scenario_type;
            ul_open_cosys_ctrl_param.sampling_rate = open_param->stream_in_param.afe.sampling_rate;
            ul_open_cosys_ctrl_param.frame_size    = open_param->stream_in_param.afe.frame_size;
            ul_open_cosys_ctrl_param.frame_number  = open_param->stream_in_param.afe.frame_number;
            ul_open_cosys_ctrl_param.format        = open_param->stream_in_param.afe.format;
            ul_open_cosys_ctrl_param.irq_period    = open_param->stream_in_param.afe.irq_period;
            ul_open_cosys_ctrl_param.header.ctrl_type  = ctrl_type;
            ul_open_cosys_ctrl_param.header.param_size = sizeof(audio_dchs_ul_open_param_t) - sizeof(audio_uart_cmd_header_t);
            mcu_uart_tx(UART_AUDIO_CMD, (U8 *)&ul_open_cosys_ctrl_param, sizeof(audio_dchs_ul_open_param_t));
            TRANSMITTER_LOG_I("[DCHS UL] relay ul open param cmd success ",0);
        }
    } else if(ctrl_type == AUDIO_UART_COSYS_UL_START) {
        if (scenario_type == AUDIO_SCENARIO_TYPE_HFP_UL || scenario_type == AUDIO_SCENARIO_TYPE_BLE_UL || scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_OUT || scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_OUT){ // [hard code]
            audio_dchs_ul_start_param_t ul_start_cosys_ctrl_param;
            memset(&ul_start_cosys_ctrl_param, 0, sizeof(audio_dchs_ul_start_param_t));
            ul_start_cosys_ctrl_param.scenario_type      = scenario_type;
            ul_start_cosys_ctrl_param.header.ctrl_type   = ctrl_type;
            ul_start_cosys_ctrl_param.header.param_size  = sizeof(audio_dchs_ul_start_param_t) - sizeof(audio_uart_cmd_header_t);
            mcu_uart_tx(UART_AUDIO_CMD, (U8 *)&ul_start_cosys_ctrl_param, sizeof(audio_dchs_ul_start_param_t));
            TRANSMITTER_LOG_I("[DCHS UL] relay ul start param cmd success ",0);
        }
    } else if(ctrl_type == AUDIO_UART_COSYS_UL_CLOSE) {
        if (scenario_type == AUDIO_SCENARIO_TYPE_HFP_UL || scenario_type == AUDIO_SCENARIO_TYPE_BLE_UL || scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_OUT || scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_OUT){ // [hard code]
            audio_dchs_ul_close_param_t ul_close_cosys_ctrl_param;
            memset(&ul_close_cosys_ctrl_param, 0, sizeof(audio_dchs_ul_close_param_t));
            ul_close_cosys_ctrl_param.scenario_type = scenario_type;
            ul_close_cosys_ctrl_param.header.ctrl_type  = ctrl_type;
            ul_close_cosys_ctrl_param.header.param_size = sizeof(audio_dchs_ul_close_param_t) - sizeof(audio_uart_cmd_header_t);
            mcu_uart_tx(UART_AUDIO_CMD, (U8 *)&ul_close_cosys_ctrl_param, sizeof(audio_dchs_ul_close_param_t));
            TRANSMITTER_LOG_I("[DCHS UL] relay ul close param cmd success ",0);
        }
    } else if(ctrl_type == AUDIO_UART_COSYS_UL_VOLUME) {
        if (scenario_type == AUDIO_SCENARIO_TYPE_HFP_UL || scenario_type == AUDIO_SCENARIO_TYPE_BLE_UL || scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_OUT || scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_OUT){ // [hard code]
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
}


#endif //AIR_DCHS_MODE_ENABLE
