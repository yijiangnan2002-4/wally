/* Copyright Statement:
 *
 * (C) 2023  Airoha Technology Corp. All rights reserved.
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
#include "audio_anc_llf_control.h"

#include "syslog.h"
#include "FreeRTOS.h"
#include "audio_nvdm_common.h"
#include "hal_audio_cm4_dsp_message.h"
#ifdef MTK_AWS_MCE_ENABLE
#include "bt_aws_mce_srv.h"
#endif



/* Private define ------------------------------------------------------------*/
#ifdef AUDIO_USE_MSGID_LOG
#define LOGMSGIDE(fmt,arg...)               LOG_MSGID_E(aud,"[LLF] "fmt,##arg)
#define LOGMSGIDW(fmt,arg...)               LOG_MSGID_W(aud,"[LLF] "fmt,##arg)
#define LOGMSGIDI(fmt,arg...)               LOG_MSGID_I(aud,"[LLF] "fmt,##arg)
#define LOGMSGIDD(fmt,arg...)               LOG_MSGID_D(aud,"[LLF] "fmt,##arg)
#else
#define LOGMSGIDE(fmt,arg...)               LOG_E(aud,"[LLF] "fmt,##arg)
#define LOGMSGIDW(fmt,arg...)               LOG_W(aud,"[LLF] "fmt,##arg)
#define LOGMSGIDI(fmt,arg...)               LOG_I(aud,"[LLF] "fmt,##arg)
#define LOGMSGIDD(fmt,arg...)               LOG_D(aud,"[LLF] "fmt,##arg)
#endif

#define LLF_DELAY_LATENCY    (500) //uniti: ms
#define LLF_CTRL_QUEUE_SIZE  (5)
#define LLF_SYNC_TIME        (100000)//uniti: us
/* Private typedef -----------------------------------------------------------*/
typedef enum {
    LLF_OP_ANC_NONE       = 0,
    LLF_OP_ANC_ON         = (1 << 0),
    LLF_OP_ANC_OFF        = (1 << 1),
    LLF_OP_ANC_BYPASS_ON  = (1 << 2),
    LLF_OP_ANC_BYPASS_OFF = (1 << 3),
} llf_operate_anc_t;

typedef struct {
    U32 timer_available;
    llf_control_event_t event;
    llf_control_cap_t llf_cap;
} llf_delay_ctrl_t;

typedef struct {
    llf_control_event_t event;
    llf_control_cap_t llf_cap;
} llf_ctrl_msg_t;

#define LLF_CTRL_QUEUE_MSG_SIZE    (sizeof(llf_ctrl_msg_t))

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
llf_control_t g_llf_control;
static SemaphoreHandle_t g_llf_mutex = NULL;
static SemaphoreHandle_t g_llf_sharebuf_semaphore = NULL;
static TimerHandle_t g_llf_delay_timer = NULL;
static llf_delay_ctrl_t g_llf_delay_ctrl;
static llf_control_entry_t g_llf_type_entry[LLF_TYPE_ALL];
static llf_control_callback_service_t g_llf_cb_service;
static xQueueHandle x_llf_ctrl_queue = NULL;
/* Private functions ---------------------------------------------------------*/
extern void anc_set_LLF_callback(llf_control_callback_t llf_callback);

void llf_mutex_create(void)
{
    g_llf_mutex = xSemaphoreCreateMutex();
    if (g_llf_mutex == NULL) {
        LOGMSGIDE("llf_mutex_creat error\r\n", 0);
    }
}
void llf_mutex_take(void)
{
    if (g_llf_mutex != NULL) {
        if (xSemaphoreTake(g_llf_mutex, portMAX_DELAY) == pdFALSE) {
            LOGMSGIDE("llf_mutex_take error\r\n", 0);
        }
    }
}
void llf_mutex_give(void)
{
    if (g_llf_mutex != NULL) {
        if (xSemaphoreGive(g_llf_mutex) == pdFALSE) {
            LOGMSGIDE("llf_mutex_give error\r\n", 0);
        }
    }
}

void llf_sharebuf_semaphore_create(void)
{
    if (g_llf_sharebuf_semaphore == NULL) {
        g_llf_sharebuf_semaphore = xSemaphoreCreateBinary();
        if (!g_llf_sharebuf_semaphore) {
            AUDIO_ASSERT(0 && "llf_control_init create get_info_semaphore FAIL \n");
        } else {
            xSemaphoreGive(g_llf_sharebuf_semaphore);
        }
    }
}

bool llf_sharebuf_semaphore_take(void)
{
    if (g_llf_sharebuf_semaphore != NULL) {
        return xSemaphoreTake(g_llf_sharebuf_semaphore, portMAX_DELAY);
    }
    return pdFALSE;
}

void llf_sharebuf_semaphore_give(void)
{
    if (g_llf_sharebuf_semaphore != NULL) {
        xSemaphoreGive(g_llf_sharebuf_semaphore);
    }
}

//static llf_status_t llf_control_queue_send(llf_ctrl_msg_t* msg)
//{
//    BaseType_t ret = -1;
//    if (x_llf_ctrl_queue) {
//        ret = xQueueSend(x_llf_ctrl_queue, msg, pdFALSE);

//        if (ret != pdFALSE) {
//            LOGMSGIDW("send queue event(0x%x) type(%d)", 2, msg->event, msg->llf_cap.type);
//            return LLF_STATUS_SUCCESS;
//        } else {
//            LOGMSGIDE("send queue FAIL!!", 0);
//            return LLF_STATUS_FAIL;
//        }
//    } else {
//        LOGMSGIDE("send queue FAIL!! queue == NULL", 0);
//        return LLF_STATUS_FAIL;
//    }
//}

//static llf_status_t llf_control_queue_receive(llf_ctrl_msg_t* msg)
//{
//    BaseType_t ret = -1;
//    if (x_llf_ctrl_queue) {
//        ret = xQueueReceive(x_llf_ctrl_queue, msg, 0);//no wait
//        if (ret != pdFALSE) {
//            LOGMSGIDI("receive queue event(0x%x) type(%d)", 2, msg->event, msg->llf_cap.type);
//            return LLF_STATUS_SUCCESS;
//        } else {
//            LOGMSGIDI("receive queue empty", 0);
//            return LLF_STATUS_FAIL;
//        }
//    } else {
//        LOGMSGIDE("receive queue FAIL!! queue == NULL", 0);
//        return LLF_STATUS_FAIL;
//    }
//}

void llf_set_feature_parameters(llf_type_t type, U8 sub_mode, U32 is_runtime)
{
    if (g_llf_type_entry[type].set_para_entry) {
        g_llf_type_entry[type].set_para_entry(is_runtime);
    } else {
        LOGMSGIDE("set_feature_parameters, not support type(%d)\r\n", 1, type);
    }

}

static void llf_dsp_callback(hal_audio_event_t event, void *user_data)
{
    U32* data = (U32*)user_data;
    LOGMSGIDI("dsp callback, event:%d, user_data[0]:0x%x, user_data[1]:0x%x (PSAP running type:%d, sub_mode:%d)", 5, event, data[0], data[1], g_llf_control.type, g_llf_control.sub_mode);

    if (g_llf_type_entry[g_llf_control.type].dsp_callback_entry) {
        g_llf_type_entry[g_llf_control.type].dsp_callback_entry(event, user_data);
    }
}

#ifdef AIR_BT_AUDIO_SYNC_ENABLE
static void llf_sync_control_aws_data(llf_sync_aws_capability_t *sync_cap)
{
    U32 local_gpt, cur_gpt;
    //LOGMSGIDI("[LLF SYNC] partner get data: action_type:%u, target_bt_clk[%u %u]", 3, sync_cap->sync_cap.sync_action_type, sync_cap->target_bt_clk.nclk, sync_cap->target_bt_clk.nclk_intra);
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &cur_gpt);
    if (bt_sink_srv_convert_bt_clock_2_gpt_count((const bt_clock_t *) & (sync_cap->target_bt_clk), (uint32_t *)&local_gpt) == BT_STATUS_SUCCESS) {
        if (cur_gpt > local_gpt) {
            local_gpt = cur_gpt;
            LOGMSGIDE("[LLF SYNC] PARTNER: over target time", 0);
        }
    } else {
        local_gpt = cur_gpt;
    }
    sync_cap->sync_cap.target_gpt_cnt = local_gpt;
    LOGMSGIDI("[LLF SYNC] PARTNER: sync_action_type:%u, from_agent:%d, target_time:%d, cur_gpt:%u, target_gpt:%u, bt_clk[%u %u], vol:%u", 8, sync_cap->sync_cap.sync_action_type, 1, local_gpt-cur_gpt, cur_gpt, local_gpt, sync_cap->target_bt_clk.nclk, sync_cap->target_bt_clk.nclk_intra, sync_cap->sync_cap.vol_out.vol_level);
    bt_sink_srv_ami_audio_request_sync(FEATURE_NO_NEED_ID, &(sync_cap->sync_cap));
}
#endif

static void bt_aws_mce_report_llf_callback(bt_aws_mce_report_info_t *para)
{
    aws_mce_report_llf_param_t *param = (aws_mce_report_llf_param_t*)para->param;
    LOGMSGIDI("AWS MCE Report: event(%u) data_len(%d), current_type(%d)", 3, param->event_id, param->param_len, g_llf_control.type);
    if (LLF_AWS_MCE_EVENT_SYNC_CTRL & param->event_id) {
#ifdef AIR_BT_AUDIO_SYNC_ENABLE
        llf_sync_control_aws_data((llf_sync_aws_capability_t*)&(param->param));
#endif
    } else if (g_llf_type_entry[g_llf_control.type].bt_aws_mce_report_callback_entry) {
        g_llf_type_entry[g_llf_control.type].bt_aws_mce_report_callback_entry(para);
    }
}

static void llf_delay_timer_callback(TimerHandle_t xTimer)
{
    g_llf_delay_ctrl.llf_cap.delay_time = 0;
    llf_control(g_llf_delay_ctrl.event, &g_llf_delay_ctrl.llf_cap);
    g_llf_delay_ctrl.timer_available = 1;
}


/* Public functions ----------------------------------------------------------*/
llf_status_t llf_callback_service(llf_type_t type, llf_control_event_t event, llf_status_t result)
{
    U32 i;

    for (i = 0; i < LLF_MAX_CALLBACK_NUM; i++) {
        if ((g_llf_cb_service.callback_list[i] != NULL) && (g_llf_cb_service.event_mask[i] != 0)) {
            if ((result != LLF_STATUS_SUCCESS) && (g_llf_cb_service.cb_level[i] != LLF_MAX_CALLBACK_LEVEL_ALL))
                continue;
            LOGMSGIDI("callback service type(%d) event(%d) result(%d) callback(0x%x)", 4, type, event, result, g_llf_cb_service.callback_list[i]);
            g_llf_cb_service.callback_list[i](type, event, result);
        }
    }
    return LLF_STATUS_SUCCESS;
}

llf_status_t llf_control_register_callback(llf_control_callback_t callback, llf_control_event_t event_mask, llf_control_callback_level_t level)
{
    U32 i;
    for (i = 0; i < LLF_MAX_CALLBACK_NUM; i++) {
        if (g_llf_cb_service.callback_list[i] == NULL) {
            g_llf_cb_service.event_mask[i] = event_mask;
            g_llf_cb_service.cb_level[i] = level;
            g_llf_cb_service.callback_list[i] = callback;
            return LLF_STATUS_SUCCESS;
        }
    }
    return LLF_STATUS_FAIL;
}

bt_status_t llf_control_send_aws_mce_param(U32 event_id, U32 data_len, U8* p_data)
{
    bt_status_t ret;
    U32 sub_param_len = sizeof(aws_mce_report_llf_param_t) - sizeof(U32) + data_len;
    aws_mce_report_llf_param_t *p_sub_param = (aws_mce_report_llf_param_t*)pvPortMalloc(sub_param_len);
    bt_aws_mce_report_info_t param = {
        .module_id = BT_AWS_MCE_REPORT_MODULE_PSAP,
        .is_sync = 0,
        .sync_time = 0,
        .param_len = sub_param_len,
        .param = (void*)p_sub_param
    };
    p_sub_param->event_id = event_id;
    p_sub_param->param_len = data_len;
    memcpy((U8*)&(p_sub_param->param), p_data, data_len);
    ret = bt_aws_mce_report_send_event(&param);
    LOGMSGIDI("send aws mce param, event:%d, len:%d, ret:%d", 3, event_id, data_len, ret);
    vPortFree((void*)p_sub_param);
    return ret;
}

void llf_control_init(void)
{
    llf_mutex_create();
    llf_sharebuf_semaphore_create();
    if (!x_llf_ctrl_queue) {
        x_llf_ctrl_queue = xQueueCreate(LLF_CTRL_QUEUE_SIZE, LLF_CTRL_QUEUE_MSG_SIZE);
    }

    anc_set_LLF_callback((llf_control_callback_t)llf_callback_service);

    memset(&g_llf_control, 0, sizeof(llf_control_t));
    memset(&g_llf_delay_ctrl, 0, sizeof(llf_delay_ctrl_t));
    g_llf_control.running_state = LLF_RUNNING_STATUS_CLOSE;
    g_llf_delay_ctrl.timer_available = 1;

    hal_audio_service_hook_callback(AUDIO_MESSAGE_TYPE_LLF, llf_dsp_callback, NULL);

    bt_aws_mce_report_register_callback(BT_AWS_MCE_REPORT_MODULE_PSAP, bt_aws_mce_report_llf_callback);

    if (g_llf_delay_timer == NULL) {
        g_llf_delay_timer = xTimerCreate("PSAPDelayTimer", (LLF_DELAY_LATENCY / portTICK_PERIOD_MS), pdFALSE, 0, llf_delay_timer_callback);
        if (g_llf_delay_timer == NULL) {
            assert(0 && "create timer FAIL!!");
        }
    }

    LOGMSGIDI("llf_control_init", 0);

}

llf_status_t llf_control_command_handler(llf_control_user_type_t source, llf_control_event_t event, void* command)
{
    llf_status_t llf_ret = LLF_STATUS_NONE;
    U16 cmd_llfType;
    U16 cmd_llfSubmode;
    int8_t sscanf_ret;

    switch(source) {
        case AUDIO_LLF_CONTROL_SOURCE_FROM_ATCI: {
            char* config_s = NULL;
            config_s = strchr((char *)command, '_');
            config_s++;
            if(strstr(config_s, "TYPE_ENABLE,") != NULL){
                sscanf_ret = sscanf(config_s, "%*[^,],%hu,%hu", &cmd_llfType, &cmd_llfSubmode);
                if(sscanf_ret != 2) {
                    LOGMSGIDE("TYPE_ENABLE input parameter error", 0);
                }
                LOGMSGIDI("[AT]TYPE_ENABLE. cmd_llfType(%d), cmd_llfSubmode(%d)", 2, cmd_llfType, cmd_llfSubmode);
                if (cmd_llfType >= LLF_TYPE_ALL) {
                    LOGMSGIDE("TYPE_ENABLE type error", 0);
                    llf_ret = LLF_STATUS_FAIL;
                    break;
                }

                if (cmd_llfType < LLF_TYPE_ALL) {
                    if (g_llf_type_entry[cmd_llfType].open_entry) {
                        llf_ret = g_llf_type_entry[cmd_llfType].open_entry();
                    }
                }

            } else if(strstr(config_s, "DISABLE") != NULL){
                LOGMSGIDI("[AT]DISABLE, LLF: running_state(%d), type(%d), sub_mode(%d)", 3, g_llf_control.running_state, g_llf_control.type, g_llf_control.sub_mode);

                llf_mutex_take();
                if (g_llf_control.running_state < LLF_RUNNING_STATUS_STOP) {

                    llf_mutex_give();
                    if (g_llf_control.type < LLF_TYPE_ALL) {
                        if (g_llf_type_entry[g_llf_control.type].close_entry) {
                            llf_ret = g_llf_type_entry[g_llf_control.type].close_entry();
                        }
                    }

                } else {
                    llf_mutex_give();
                    llf_ret = LLF_STATUS_FAIL;
                }

            } else if (strstr(config_s, "CONFIG") != NULL) {
                U16 config_event;
                signed int param;
                sscanf_ret = sscanf(config_s, "%*[^,],%hu,%hu,%hu,%d", &cmd_llfType, &cmd_llfSubmode, &config_event, &param);
                if(sscanf_ret != 4) {
                    LOGMSGIDE("[AT]CONFIG input parameter error", 0);
                }
                LOGMSGIDI("[AT]CONFIG. g_llf_control.type(%d), cmd_llfType(%d), cmd_llfSubmode(%d), config_event(%d), param(%d)", 5, g_llf_control.type, cmd_llfType, cmd_llfSubmode, config_event, param);

                if (g_llf_type_entry[cmd_llfType].runtime_config_handler_entry) {
                    g_llf_type_entry[cmd_llfType].runtime_config_handler_entry(config_event, param, NULL);;
                }

            }
            break;
        }
        case AUDIO_LLF_CONTROL_SOURCE_FROM_AM: {
            llf_control_cap_t* cap = (llf_control_cap_t*) command;
            if (cap->type > LLF_TYPE_ALL) {
                LOGMSGIDE("Wrong type!!", 0);
            }
            switch(event) {
                case LLF_CONTROL_EVENT_OFF: {
                    LOGMSGIDI("[AM]OFF type(%d) sub_mode(%d)", 2, cap->type, cap->sub_mode);
                    if (g_llf_control.running_state < LLF_RUNNING_STATUS_STOP) {
                        g_llf_control.running_state = LLF_RUNNING_STATUS_STOP;
                        //llf_ret = audio_anc_control_disable(NULL);
                        if (cap->stream_handler) {
                            llf_ret = cap->stream_handler(false);
                        }
                    }
                    break;
                }
                case LLF_CONTROL_EVENT_SUSPEND: {
                    LOGMSGIDI("[AM]suspend type(%d) sub_mode(%d)", 2, cap->type, cap->sub_mode);
                    llf_mutex_take();
                    if (g_llf_control.running_state < LLF_RUNNING_STATUS_SUSPEND) {
                        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_LLF_SUSPEND, 0, 0, true);
                        //llf_mutex_take();
                        g_llf_control.running_state = LLF_RUNNING_STATUS_SUSPEND;
                        //llf_mutex_give();
                        llf_ret = LLF_STATUS_SUCCESS;
                    }
                    llf_mutex_give();
                    break;
                }
                case LLF_CONTROL_EVENT_RESUME: {
                    LOGMSGIDI("[AM]resume type(%d) sub_mode(%d)", 2, cap->type, cap->sub_mode);
                    llf_mutex_take();
                    if (g_llf_control.running_state == LLF_RUNNING_STATUS_SUSPEND) {
                        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_LLF_RESUME, 0, 0, true);
                        llf_set_feature_parameters(cap->type, cap->sub_mode, 0);
                        //llf_mutex_take();
                            g_llf_control.running_state = LLF_RUNNING_STATUS_RUNNING;
                        //llf_mutex_give();
                        llf_ret = LLF_STATUS_SUCCESS;
                    }
                    llf_mutex_give();
                    break;
                }
                case LLF_CONTROL_EVENT_RUNTIME_CONFIG:{

                    llf_control_runtime_config_t* confing = (llf_control_runtime_config_t*)cap->param;
                    LOGMSGIDI("[AM]config type(%d) sub_mode(%d) event(%d)", 3, cap->type, cap->sub_mode, confing->config_event);
                    if (((confing->config_event == 22) && (confing->type == LLF_TYPE_HEARING_AID))) {
                        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_LLF_CONFIG, 0, cap->param, true);
                        llf_sharebuf_semaphore_give();
                    } else {
                        void *malloc_ptr = (void*)cap->param;
                        void *p_param_share;
                        p_param_share = hal_audio_dsp_controller_put_paramter(malloc_ptr, cap->param_len, AUDIO_MESSAGE_TYPE_COMMON);
                        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_LLF_CONFIG, 0, (U32)p_param_share, true);
                        vPortFree(malloc_ptr);
                    }

                    if (((confing->config_event == 24) && (confing->type == LLF_TYPE_HEARING_AID))) {
                        g_llf_control.dsp_share_buffer_write_done = true;
                    }
                    llf_ret = LLF_STATUS_SUCCESS;

                    break;
                }
                case LLF_CONTROL_EVENT_RESET: {
                    LOGMSGIDI("[AM]reset type(%d) sub_mode(%d)", 2, cap->type, cap->sub_mode);
                    if (g_llf_control.running_state < LLF_RUNNING_STATUS_STOP) {
                        g_llf_control.running_state = LLF_RUNNING_STATUS_RESET;
                        if (cap->stream_handler) {
                            llf_ret = cap->stream_handler(false);
                        }
                    }
                    break;
                }
                case LLF_CONTROL_EVENT_UPDATE_PARA: {
                    LOGMSGIDI("[AM]update para, type(%d) sub_mode(%d)", 2, cap->type, cap->sub_mode);
                    llf_set_feature_parameters(cap->type, cap->sub_mode, 1);
                    break;
                }
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }

    return llf_ret;
}

llf_status_t llf_control(llf_control_event_t event, llf_control_cap_t *llf_cap)
{
    //LOGMSGIDI("llf_control, event:%d, type:%d, sub_mode:%d, delay(%d), timer_avalible:%d", 5, event, llf_cap->type, llf_cap->sub_mode, llf_cap->delay_time, g_llf_delay_ctrl.timer_available);
    S32 *delay_time_ptr = &(llf_cap->delay_time);
    if ((*delay_time_ptr > 0) && g_llf_delay_ctrl.timer_available) {
        g_llf_delay_ctrl.event = event;
        memcpy(&g_llf_delay_ctrl.llf_cap, llf_cap, sizeof(llf_control_cap_t));
        if (*delay_time_ptr > LLF_DELAY_LATENCY) {
            *delay_time_ptr = LLF_DELAY_LATENCY;
        }
        if (xTimerChangePeriod(g_llf_delay_timer, (*delay_time_ptr / portTICK_PERIOD_MS), 0) == pdPASS) {
            g_llf_delay_ctrl.timer_available = 0;
            LOGMSGIDI("llf_control delay(%d), event:%d, type:%d, sub_mode:%d", 4, llf_cap->delay_time, event, llf_cap->type, llf_cap->sub_mode);
            return LLF_STATUS_SUCCESS;
        }
//    } else if (((event & (LLF_CONTROL_EVENT_ON | LLF_CONTROL_EVENT_RESET)) && ((g_llf_control.running_state != LLF_RUNNING_STATUS_CLOSE) && (g_llf_control.running_state != LLF_RUNNING_STATUS_RUNNING) && (g_llf_control.running_state != LLF_RUNNING_STATUS_SUSPEND)))) {
//        //unstable state, send request to queue
//        LOGMSGIDI("llf_control, unstable status, event:%d, running_state:%d", 2, event, g_llf_control.running_state);
//        llf_ctrl_msg_t msg;
//        msg.event = event;
//        memcpy(&msg.llf_cap, llf_cap, sizeof(llf_control_cap_t));
//        return llf_control_queue_send(&msg);
//    }  else if (((event & LLF_CONTROL_EVENT_RUNTIME_CONFIG) && ((g_llf_control.running_state != LLF_RUNNING_STATUS_CLOSE) && (g_llf_control.running_state != LLF_RUNNING_STATUS_RUNNING) && (g_llf_control.running_state != LLF_RUNNING_STATUS_SUSPEND)))) {
//        llf_control_runtime_config_t* confing = (llf_control_runtime_config_t*)hal_audio_query_share_info(AUDIO_MESSAGE_TYPE_LLF);
//        LOGMSGIDI("llf_control, unstable status, config_event:%d, running_state:%d", 2, confing->config_event, g_llf_control.running_state);
//        if (((confing->config_event != 19) && (confing->type == LLF_TYPE_HEARING_AID)) || ((confing->config_event != 0) && (confing->type == LLF_TYPE_VIVID_PT))) {
//            llf_ctrl_msg_t msg;
//            msg.event = event;
//            memcpy(&msg.llf_cap, llf_cap, sizeof(llf_control_cap_t));
//            return llf_control_queue_send(&msg);
//        }
    }
    if ((event & LLF_CONTROL_EVENT_RUNTIME_CONFIG) && (g_llf_control.running_state == LLF_RUNNING_STATUS_CLOSE)) {
        LOGMSGIDE("config FAIL due to closed state", 0);
        return LLF_STATUS_FAIL;
    }

    bt_sink_srv_am_feature_t feature_param;
    LOGMSGIDI("llf_control, event:%d, type:%d, sub_mode:%d", 3, event, llf_cap->type, llf_cap->sub_mode);
    memset(&feature_param, 0, sizeof(bt_sink_srv_am_feature_t));
    feature_param.type_mask = AM_LLF;
    feature_param.feature_param.llf_param.event = event;

    switch(event) {
        case LLF_CONTROL_EVENT_ON:
        case LLF_CONTROL_EVENT_OFF:
        case LLF_CONTROL_EVENT_SUSPEND:
        case LLF_CONTROL_EVENT_RESUME:
        case LLF_CONTROL_EVENT_RUNTIME_CONFIG:
        case LLF_CONTROL_EVENT_RESET:
        case LLF_CONTROL_EVENT_UPDATE_PARA:
            memcpy(&(feature_param.feature_param.llf_param.cap), llf_cap, sizeof(llf_control_cap_t));
            break;
        default:
            return LLF_STATUS_FAIL;
    }
    am_audio_set_feature(FEATURE_NO_NEED_ID, &feature_param);

    return LLF_STATUS_SUCCESS;
}

llf_status_t llf_control_suspend(void)
{
    LOGMSGIDI("suspend", 0);
    llf_running_status_t llf_running;
    llf_type_t type;
    U8 sub_mode;
    llf_control_get_status(&llf_running, &type, &sub_mode, NULL);
    llf_control_cap_t llf_cap = {   type,
                                            sub_mode,
                                            0,
                                            0,
                                            g_llf_control.stream_handler,
                                            0,
                                            0
    };
    return llf_control(LLF_CONTROL_EVENT_SUSPEND, &llf_cap);
}

llf_status_t llf_control_resume(void)
{
    LOGMSGIDI("resume", 0);
    llf_running_status_t llf_running;
    llf_type_t type;
    U8 sub_mode;
    llf_control_get_status(&llf_running, &type, &sub_mode, NULL);
    llf_control_cap_t llf_cap = {   type,
                                            sub_mode,
                                            0,
                                            0,
                                            g_llf_control.stream_handler,
                                            300,//wait 300ms for fade out
                                            0
    };
    return llf_control(LLF_CONTROL_EVENT_RESUME, &llf_cap);
}

void llf_control_set_status(llf_running_status_t running, llf_type_t type, U8 sub_mode, void *misc)
{
    llf_mutex_take();
    g_llf_control.running_state = running;
    g_llf_control.type = type;
    g_llf_control.sub_mode = sub_mode;
    llf_mutex_give();
    if ((running == LLF_RUNNING_STATUS_CLOSE) && (g_llf_delay_ctrl.timer_available == 0)) {
        xTimerStop(g_llf_delay_timer, 0);
        g_llf_delay_ctrl.timer_available = 1;
    }
}

void llf_control_get_status(llf_running_status_t *running, llf_type_t *type, U8 *sub_mode, void *misc)
{
    llf_mutex_take();
    if (running) {
        *running = g_llf_control.running_state;
    }
    if (type) {
        *type = g_llf_control.type;
    }
    if (sub_mode) {
        *sub_mode = g_llf_control.sub_mode;
    }
    llf_mutex_give();
}

void llf_control_get_frame_setting(uint32_t *frame_lenth, uint32_t *buffer_length)
{
    if (frame_lenth) {
       *frame_lenth = g_llf_control.frame_length;
    }
    if (buffer_length) {
       *buffer_length = g_llf_control.buffer_length;
    }
}

void llf_control_set_parameter(U16 nvkey_id)
{
    U32 nvkey_length = 0;
    sysram_status_t nvdm_status;
    uint32_t *share_addr = hal_audio_query_share_info(AUDIO_MESSAGE_TYPE_LLF);

    nvdm_status =  flash_memory_query_nvdm_data_length(nvkey_id, &nvkey_length);
    if (nvdm_status || !nvkey_length) {
        LOGMSGIDE(" Read Nvkey length Fail id:0x%x, status:%d ", 2, nvkey_id, nvdm_status);
    }
    nvdm_status = flash_memory_read_nvdm_data(nvkey_id, (uint8_t *)share_addr, &nvkey_length);
    if (nvdm_status || !nvkey_length) {
        LOGMSGIDE(" Read Nvkey data Fail id:0x%x, status:%d ", 2, nvkey_id, nvdm_status);
    }
    //LOGMSGIDI("set parameter nvkey(0x%x), share_addr(0x%x), nvkey_length(%d)", 3, nvkey_id, share_addr, nvkey_length);
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SET_ALGORITHM_PARAM, nvkey_id, (uint32_t)share_addr, true);
}

void llf_control_set_multi_parameters(U16 nvkey_id_list[], U8 key_num)
{
    if (llf_sharebuf_semaphore_take() == pdPASS) {
        U32 nvkey_length = 0, i;
        sysram_status_t nvdm_status;
        uint32_t *share_addr = hal_audio_query_share_info(AUDIO_MESSAGE_TYPE_LLF);
        uint8_t *p_src = (uint8_t*)share_addr;

        for (i=0; i<key_num; i++) {
            nvdm_status =  flash_memory_query_nvdm_data_length(nvkey_id_list[i], &nvkey_length);
            if (nvdm_status || !nvkey_length) {
                LOGMSGIDE(" Read Nvkey length Fail id:0x%x, status:%d ", 2, nvkey_id_list[i], nvdm_status);
                continue;
            }
            nvdm_status = flash_memory_read_nvdm_data(nvkey_id_list[i], p_src, &nvkey_length);
            if (nvdm_status || !nvkey_length) {
                LOGMSGIDE(" Read Nvkey data Fail id:0x%x, status:%d ", 2, nvkey_id_list[i], nvdm_status);
            }
//            LOGMSGIDI("set parameter nvkey(0x%x), p_src(0x%x), length:%d", 3, nvkey_id_list[i], p_src, nvkey_length);
            p_src += nvkey_length;
            if ((p_src - (uint8_t*)hal_audio_query_share_info(AUDIO_MESSAGE_TYPE_LLF)) > SHARE_BUFFER_LLF_INFO_COUNT * 4) {
                LOGMSGIDE("share buffer overflow (0x%x - 0x%x)", 2, p_src, (uint8_t*)hal_audio_query_share_info(AUDIO_MESSAGE_TYPE_LLF));
            }
        }
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SET_ALGORITHM_PARAM, nvkey_id_list[0], (uint32_t)share_addr, true);
        llf_sharebuf_semaphore_give();
    }
}

void llf_control_realtime_set_parameter(U16 nvkey_id, U32 nvkey_length, U8* data)
{
    if (llf_sharebuf_semaphore_take() == pdPASS) {
        uint32_t *share_addr = hal_audio_query_share_info(AUDIO_MESSAGE_TYPE_LLF);
        memcpy((uint8_t *)share_addr, data, nvkey_length);
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SET_ALGORITHM_PARAM, nvkey_id, (uint32_t)share_addr, true);
        llf_sharebuf_semaphore_give();
    }
}

bool llf_control_query_share_buffer(void)
{
    llf_running_status_t running_state = g_llf_control.running_state;

    while ((running_state == LLF_RUNNING_STATUS_RESET) || (running_state == LLF_RUNNING_STATUS_START)) {
        vTaskDelay(1 / portTICK_RATE_MS);
    }
    return llf_sharebuf_semaphore_take();
}

U32 llf_control_get_device_msk_by_data_list(U32 data_list[])
{
    U32 mask = 0;
    if (data_list) {
        for(U32 i = 0; i < LLF_DATA_TYPE_NUM; i++) {
            switch(data_list[i]) {
                case LLF_DATA_TYPE_REAR_L: {
                    mask |= (1 << 0);
                    break;
                }
                case LLF_DATA_TYPE_INEAR_L: {
                    mask |= (1 << 2) | (1 << 4);
                    break;
                }
                case LLF_DATA_TYPE_TALK: {
                    mask |= (1 << 7);
                    break;
                }
                default:
                    break;
            }
        }
    }
    return mask;
}

llf_status_t llf_control_register_entry(llf_type_t type, llf_control_entry_t *entry)
{
    if (type < LLF_TYPE_ALL) {
        memcpy(&g_llf_type_entry[type], entry, sizeof(llf_control_entry_t));
        //LOGMSGIDI("register type(%d) entry control_device_entry:0x%x, set_para_entry:0x%x", 3, type, g_llf_type_entry[type].control_device_entry, g_llf_type_entry[type].set_para_entry);
        LOGMSGIDI("llf_control_register_entry entry 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x", 7,
        g_llf_type_entry[type].open_entry,
        g_llf_type_entry[type].close_entry,
        g_llf_type_entry[type].set_para_entry,
        g_llf_type_entry[type].dsp_callback_entry,
        g_llf_type_entry[type].bt_aws_mce_report_callback_entry,
        g_llf_type_entry[type].runtime_config_handler_entry,
        g_llf_type_entry[type].switch_mode_callback_entry);
        return LLF_STATUS_SUCCESS;
    }
    return LLF_STATUS_FAIL;
}

void llf_control_query_data_wait(void)
{
    g_llf_control.dsp_share_buffer_write_done = false;
}

llf_status_t llf_control_query_data_ready(void)
{
    U32 i;
    U8* data_ready_flag = &g_llf_control.dsp_share_buffer_write_done;
    for (i = 0; ; i++) {
        if (*data_ready_flag) {
            break;
        }
        if ((i % 500) == 0) {
            if (i == 500) {
                configASSERT(0 && "llf_control_query_data_ready 1s timeout");
            }
        }
        hal_gpt_delay_ms(2);
    }
    return LLF_STATUS_SUCCESS;
}

#ifdef AIR_BT_AUDIO_SYNC_ENABLE
void llf_control_sync_control(llf_sync_capability_t *llf_sync_cap)
{
    bt_sink_srv_am_audio_sync_capability_t srv_sync_cap = {0};
    srv_sync_cap.sync_scenario_type = MCU2DSP_SYNC_REQUEST_LLF;
    bt_clock_t bt_clk = {0};
    uint32_t local_gpt = 0, cur_gpt = 0;
    uint32_t target_time = LLF_SYNC_TIME;

    if (llf_sync_cap->action == LLF_SYNC_ACTION_DL_MUTE) {
        srv_sync_cap.sync_action_type = MCU2DSP_SYNC_REQUEST_SET_MUTE;
        srv_sync_cap.vol_out.vol_level = 0;
    } else if (llf_sync_cap->action == LLF_SYNC_ACTION_DL_UNMUTE) {
        srv_sync_cap.sync_action_type = MCU2DSP_SYNC_REQUEST_SET_MUTE;
        srv_sync_cap.vol_out.vol_level = 1;
    } else {
        LOGMSGIDE("[LLF SYNC] not supported sync action:%d", 1, llf_sync_cap->action);
    }

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &cur_gpt);
    target_time = llf_sync_cap->sync_time;

    if (llf_sync_cap->is_sync) {

        //if ((bt_connection_manager_device_local_info_get_aws_role() == BT_AWS_MCE_ROLE_AGENT) && (bt_sink_srv_cm_get_aws_link_state() == BT_AWS_MCE_AGENT_STATE_ATTACHED))
        if (bt_sink_srv_cm_get_aws_link_state() == BT_AWS_MCE_AGENT_STATE_ATTACHED) {
            if (bt_connection_manager_device_local_info_get_aws_role() == BT_AWS_MCE_ROLE_AGENT) {

                if (bt_sink_srv_bt_clock_addition(&bt_clk, 0, target_time) != BT_STATUS_SUCCESS) {
                    LOGMSGIDE("[LLF SYNC] get bt_clk fail", 0);
                    return;
                }
                if (bt_sink_srv_convert_bt_clock_2_gpt_count((const bt_clock_t *) & (bt_clk), (uint32_t *)&local_gpt) == BT_STATUS_SUCCESS) {
                    srv_sync_cap.target_gpt_cnt = local_gpt;
                } else {
                    LOGMSGIDE("[LLF SYNC] get gpt count fail", 0);
                    return;
                }
                llf_sync_aws_capability_t sync_aws_cap = {0};
                memcpy((U8*)&sync_aws_cap, (U8*)&srv_sync_cap, sizeof(bt_sink_srv_am_audio_sync_capability_t));
                memcpy(&(sync_aws_cap.target_bt_clk), &bt_clk, sizeof(bt_clock_t));
                llf_control_send_aws_mce_param(LLF_AWS_MCE_EVENT_SYNC_CTRL, sizeof(llf_sync_aws_capability_t), (U8*)&sync_aws_cap);
                bt_sink_srv_ami_audio_request_sync(FEATURE_NO_NEED_ID, &srv_sync_cap);
                LOGMSGIDI("[LLF SYNC] AGENT: sync_action_type:%u, target_time:%u, cur_gpt:%u, target_gpt:%u, bt_clk[%u %u], vol:%u", 7, srv_sync_cap.sync_action_type, target_time, cur_gpt, local_gpt, bt_clk.nclk, bt_clk.nclk_intra, srv_sync_cap.vol_out.vol_level);
            } else if (llf_sync_cap->action == LLF_SYNC_ACTION_DL_UNMUTE) { // Partner unmute DL when later join
                srv_sync_cap.target_gpt_cnt = local_gpt = cur_gpt + target_time + 100000;
                bt_sink_srv_ami_audio_request_sync(FEATURE_NO_NEED_ID, &srv_sync_cap);
                LOGMSGIDI("[LLF SYNC] PARTNER: sync_action_type:%u, from_agent:%d, target_time:%u, cur_gpt:%u, target_gpt:%u, vol:%u", 6, srv_sync_cap.sync_action_type, 0, target_time, cur_gpt, local_gpt, srv_sync_cap.vol_out.vol_level);
            } else {
                LOGMSGIDI("[LLF SYNC] DO NOTHING: sync_action_type:%u, target_time:%u, cur_gpt:%u, target_gpt:%u, vol:%u", 5, srv_sync_cap.sync_action_type, target_time, cur_gpt, local_gpt, srv_sync_cap.vol_out.vol_level);
            }
        } else {
            srv_sync_cap.target_gpt_cnt = local_gpt = cur_gpt + target_time;
            bt_sink_srv_ami_audio_request_sync(FEATURE_NO_NEED_ID, &srv_sync_cap);
            LOGMSGIDI("[LLF SYNC] NOT attached: sync_action_type:%u, target_time:%u, cur_gpt:%u, target_gpt:%u, vol:%u", 5, srv_sync_cap.sync_action_type, target_time, cur_gpt, local_gpt, srv_sync_cap.vol_out.vol_level);
        }

    } else {
        srv_sync_cap.target_gpt_cnt = local_gpt = cur_gpt + target_time;
        bt_sink_srv_ami_audio_request_sync(FEATURE_NO_NEED_ID, &srv_sync_cap);
        LOGMSGIDI("[LLF SYNC] NOT SYNC: sync_action_type:%u, target_time:%u, cur_gpt:%u, target_gpt:%u, vol:%u", 5, srv_sync_cap.sync_action_type, target_time, cur_gpt, local_gpt, srv_sync_cap.vol_out.vol_level);
    }

    //LOGMSGIDI("[LLF SYNC] sync_action_type:%u, target_time:%u, cur_gpt:%u, target_gpt:%u", 4, srv_sync_cap.sync_action_type, target_time, cur_gpt, local_gpt);

}
#endif

