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

#include "config.h"
#include "dsp_task.h"
#include "dsp_interface.h"
#include "dsp_memory.h"
#include "dprt_rt.h"
#include "dprt_vp.h"
#include "dsp_drv_dfe.h"
#include "audio_config.h"
#include "stream.h"
#include "task_def.h"

#include "sink_inter.h"
#include "source_inter.h"
#include "dsp_feature_interface.h"
#include "dsp_update_para.h"
#include "rt_data.h"
//#include "os_dpc.h"
#include <string.h>

#include "dsp_task.h"
#include "stream_audio.h"
#include "long_term_clk_skew.h"
#include "stream_n9_a2dp.h"
#if defined(AIR_LD_NR_ENABLE)
#include "ld_nr_interface.h"
#endif
#if defined(AIR_HEARTHROUGH_VIVID_PT_ENABLE)
#include "vivid_passthru_interface.h"
#endif
/******************************************************************************
* External Variables
******************************************************************************/
EXTERN VOID *pvTaskParameter;

/******************************************************************************
 * Function Declaration
 ******************************************************************************/
VOID                dsp_stream_active_entry(VOID *task_parameter_ptr);
VOID                dsp_stream_default_entry(VOID *parameter_ptr);
dsp_stream_task_pointer_t    dsp_stream_task_get_parameters(TaskHandle_t task_id);
VOID                dsp_stream_init(dsp_stream_task_pointer_t task_parameter_ptr);
VOID                dsp_stream_suspend_request(VOID *parameter_ptr);
BOOL                dsp_stream_check_callback_status(dsp_stream_task_pointer_t task_parameter_ptr);
BOOL                dsp_stream_check_callback_status_disable(dsp_stream_task_pointer_t task_parameter_ptr);
STATIC INLINE VOID  dsp_stream_process(dsp_stream_task_pointer_t task_parameter_ptr);
VOID                dsp_stream_msg_ack(DSP_STREAMING_PARA_PTR stream_ptr);
VOID                dsp_stream_status_update(DSP_STREAMING_PARA_PTR stream_ptr);
/******************************************************************************
 * External Function Prototypes
 ******************************************************************************/


/******************************************************************************
 * Type Definitions
 ******************************************************************************/



/******************************************************************************
 * Constants
 ******************************************************************************/
/* Number of DSP States */

/******************************************************************************
 * Variables
 ******************************************************************************/
TaskHandle_t  pStreamTaskHandler[DSP_STREAM_TASK_MAX_NUMBER];
DSP_STREAMING_PARA_PTR dsp_stream_ptr[DSP_STREAM_TASK_MAX_NUMBER];
dsp_stream_task_t task_parameter[DSP_STREAM_TASK_MAX_NUMBER];
stream_task_config_ptr_t stream_task_config_table_ptr;
task_manage_ptr_t task_manage_table_ptr;
uint32_t    stream_task_num;

/**
 * dsp_stream_task_create
 *
 * Create dsp stream task
 *
 */
VOID dsp_stream_task_create(VOID)
{
    stream_task_config_ptr_t config_table_ptr;
    BaseType_t result;
    uint32_t i;

    for (i = 0 ; i < DSP_STREAM_TASK_MAX_NUMBER ; i++) {
        task_parameter[i].task_id = 0;
    }

    config_table_ptr = stream_task_config_table_ptr;
    for (i = 0 ; i < DSP_STREAM_TASK_MAX_NUMBER ; i++) {
        if (config_table_ptr->task_id == STREAM_TASK_END){
            break;
        }
        DSP_MW_LOG_I("[Stream Task] task create id:%d, stack depth:%d, priority:%d, no_of_stream:%d", 4, config_table_ptr->task_id, config_table_ptr->stack_depth, config_table_ptr->priority, config_table_ptr->no_of_stream);

        result = xTaskCreate((TaskFunction_t)dsp_stream_task, config_table_ptr->task_name, config_table_ptr->stack_depth, pvTaskParameter, config_table_ptr->priority, &pStreamTaskHandler[config_table_ptr->task_id]);
        if (result != pdPASS) {
            DSP_MW_LOG_E("[Stream Task] task create fail!", 0);
            AUDIO_ASSERT(0);
        }

        dsp_stream_ptr[config_table_ptr->task_id] = (DSP_STREAMING_PARA_PTR) pvPortMalloc(sizeof(DSP_STREAMING_PARA) * config_table_ptr->no_of_stream);
        if (dsp_stream_ptr[config_table_ptr->task_id] == NULL) {
            DSP_MW_LOG_E("[Stream Task] Malloc fail!", 0);
            AUDIO_ASSERT(0);
        }

        task_parameter[config_table_ptr->task_id].task_id = pStreamTaskHandler[config_table_ptr->task_id];
        task_parameter[config_table_ptr->task_id].stream_ptr = (DSP_STREAMING_PARA_PTR)dsp_stream_ptr[config_table_ptr->task_id];
        task_parameter[config_table_ptr->task_id].stream_number = config_table_ptr->no_of_stream;
        task_parameter[config_table_ptr->task_id].max_runtime = config_table_ptr->max_runtime;
        task_parameter[config_table_ptr->task_id].task_idx = config_table_ptr->task_id;

        config_table_ptr++;
    }
    stream_task_num = i++;
}
#ifdef AIR_PROMPT_SOUND_MEMORY_DEDICATE
U8* g_vp_memptr;
#endif
/**
 * DSP_VP_Premalloc
 *
 * Dedicate VP Memory in DSP_PR_Task
 * @ mallocSize(4096) = VP_FrameSize(2048)*channelNum(2)
 * @ g_vp_memptr: global memory pointer for VP data
 *
 */
#ifdef AIR_PROMPT_SOUND_MEMORY_DEDICATE
extern stream_feature_list_t stream_feature_list_prompt[];
VOID DSP_VP_Premalloc(VOID)
{
    stream_feature_type_ptr_t  featureTypePtr = stream_feature_list_prompt;
    U32 featureEntryNum = 0;
    U32 featureEntrySize;

    if (featureTypePtr) {
        for(featureEntryNum = 1 ; *(featureTypePtr) != FUNC_END ; ++featureTypePtr) {
            featureEntryNum++;
        }
    }

    featureEntrySize = featureEntryNum * sizeof(DSP_FEATURE_TABLE);

    U32 mallocSize = 4096 + featureEntrySize;
    VOID* stream = NULL;
    g_vp_memptr = DSPMEM_tmalloc(DPR_TASK_ID, mallocSize, stream);
    if(g_vp_memptr == NULL){
        platform_assert("[g_vp_memptr]g_vp_memptr is NULL!", __FILE__, __LINE__);
    }
    memset(g_vp_memptr, 0, mallocSize);
}
#endif
/**
 * dsp_stream_task_init
 *
 * Dsp stream task init
 *
 */
VOID dsp_stream_task_init(VOID)
{
#ifdef AIR_PROMPT_SOUND_MEMORY_DEDICATE
    DSP_VP_Premalloc();
#endif

#ifdef AIR_AUDIO_HARDWARE_ENABLE
    Audio_Default_setting_init();
#endif /* AIR_AUDIO_HARDWARE_ENABLE */

#if defined(AIR_BT_CODEC_BLE_ENABLED) && defined(AIR_VOICE_NR_ENABLE)
    /*Initialize for semaphore*/
    EXTERN SemaphoreHandle_t g_nr_process_xSemaphore;
    if (g_nr_process_xSemaphore == NULL) {
        g_nr_process_xSemaphore = xSemaphoreCreateBinary();
        assert(g_nr_process_xSemaphore != NULL);
        xSemaphoreGive(g_nr_process_xSemaphore);
    }
#endif
}
/**
 * dsp_stream_task
 *
 * Dsp stream task
 *
 */
ATTR_TEXT_IN_IRAM_LEVEL_1 VOID dsp_stream_task(VOID)
{
    TaskHandle_t task_id;
    dsp_stream_task_pointer_t task_parameter_ptr;

    task_id = xTaskGetCurrentTaskHandle();

    /* Do Initialization */
    task_parameter_ptr = dsp_stream_task_get_parameters(task_id);

    dsp_stream_init(task_parameter_ptr);
    while (1) {
        task_parameter_ptr->entry(task_parameter_ptr);

#if defined(AIR_LD_NR_ENABLE) || defined(AIR_HEARTHROUGH_VIVID_PT_ENABLE)
        if (task_id == DPR_TASK_ID){
#if defined(AIR_LD_NR_ENABLE)
            if (ld_nr_bg_entry) {
                ld_nr_bg_entry();
            }
#endif
#if defined(AIR_HEARTHROUGH_VIVID_PT_ENABLE)
            if (vivid_pt_ldnr_bg_entry) {
                vivid_pt_ldnr_bg_entry();
            }
#endif
        }
#endif
        dsp_stream_suspend_request(task_parameter_ptr);
    }
}


/**
 * dsp_stream_default_entry
 *
 * Default Entry for stream_task
 *
 */
VOID dsp_stream_default_entry(VOID *parameter_ptr)
{
    dsp_stream_task_pointer_t task_parameter_ptr;
    task_parameter_ptr = (dsp_stream_task_pointer_t)parameter_ptr;

    if (!dsp_stream_check_callback_status_disable(task_parameter_ptr)) {
        task_parameter_ptr->entry = dsp_stream_active_entry;
    }
}


/**
 * dsp_stream_active_entry
 *
 * Active Entry for stream_task
 *
 */
ATTR_TEXT_IN_IRAM_LEVEL_1 VOID dsp_stream_active_entry(VOID *task_parameter_ptr)
{
    dsp_stream_process((dsp_stream_task_pointer_t)task_parameter_ptr);
}


/**
 * dsp_stream_task_get_parameters
 *
 * Get stream task parameters by task id
 *
 */
dsp_stream_task_pointer_t dsp_stream_task_get_parameters(TaskHandle_t task_id)
{
    U8 i;

    for (i = 0 ; i < stream_task_num ; i++) {
        if (task_id == pStreamTaskHandler[i]) {
            return &task_parameter[i];
        }
    }
    if (i == stream_task_num) {
        DSP_MW_LOG_E("[Stream Task] dsp_stream_task_get_parameters error!", 0);
        AUDIO_ASSERT(0);
    }
    return NULL;
}


/**
 * dsp_stream_init
 *
 * Initialization for dsp_stream_task
 *
 */
void dsp_stream_init(dsp_stream_task_pointer_t task_parameter_ptr)
{
#ifndef AIR_DSP_MEMORY_REGION_ENABLE
    DSPMEM_Init(task_parameter_ptr->task_id);
#endif
    DSP_Callback_StreamingInit(task_parameter_ptr->stream_ptr, task_parameter_ptr->stream_number, task_parameter_ptr->task_id);
    task_parameter_ptr->entry = dsp_stream_default_entry;
}


/**
 * dsp_stream_process
 *
 * Active Entry for dsp_stream_task process
 *
 */
STATIC INLINE VOID dsp_stream_process(dsp_stream_task_pointer_t task_parameter_ptr)
{
    U8 i;
#ifdef AIR_DSP_TASK_PRINT_PROCESS_WARNING_LOG_ENABLE
    uint32_t start_time, end_time;
#endif
    for (i = 0 ; i < task_parameter_ptr->stream_number ; i++) {
#ifdef AIR_DSP_TASK_PRINT_PROCESS_WARNING_LOG_ENABLE
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &start_time);
#endif
        if (task_parameter_ptr->stream_ptr[i].streamingStatus == STREAMING_END) {
#ifdef AIR_AUDIO_HARDWARE_ENABLE
            if ((task_parameter_ptr->stream_ptr[i].sink->type == SINK_TYPE_AUDIO) &&
                (task_parameter_ptr->stream_ptr[i].sink->param.audio.irq_exist) &&
                (Audio_setting->Audio_sink.Zero_Padding_Cnt > 0)) {
                //DSP_MW_LOG_E("DAVT wait zero padding:%d !!!", 1, Audio_setting->Audio_sink.Zero_Padding_Cnt);
            } else
#endif /* AIR_AUDIO_HARDWARE_ENABLE */
            {
                StreamCloseAll(task_parameter_ptr->stream_ptr[i].source->transform, InstantCloseMode);
            }
        }
        dsp_stream_status_update((DSP_STREAMING_PARA_PTR)(task_parameter_ptr->stream_ptr + i));

        DSP_Callback_Processing((DSP_STREAMING_PARA_PTR)(task_parameter_ptr->stream_ptr + i));

#if defined (AIR_USB_AUDIO_IN_AND_OUT_MIX_ENABLE)
        SOURCE source = ((DSP_STREAMING_PARA_PTR)(task_parameter_ptr->stream_ptr + i))->source;
        if((source != NULL) && (((DSP_STREAMING_PARA_PTR)(task_parameter_ptr->stream_ptr + i))->callback.Status == CALLBACK_HANDLER)){
            if(source->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_OUT_IEM){
                extern bool g_wired_audio_main_stream_is_triggered;
                extern void wired_audio_main_stream_trigger_active(void);
                if (g_wired_audio_main_stream_is_triggered == false){
                    g_wired_audio_main_stream_is_triggered = true;
                    wired_audio_main_stream_trigger_active();
                }
            }
        }
#endif

        dsp_stream_msg_ack((DSP_STREAMING_PARA_PTR)(task_parameter_ptr->stream_ptr + i));

#ifdef AIR_DSP_TASK_PRINT_PROCESS_WARNING_LOG_ENABLE
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &end_time);

        if ((uint32_t)(end_time - start_time) > task_parameter_ptr->max_runtime) {
            DSP_MW_LOG_W("[STREAM_TASK_%x] task process time:%d us", 2, task_parameter_ptr->task_idx, (uint32_t)(end_time - start_time));
        }
#endif
    }
}


/**
 * dsp_stream_configure_streaming
 *
 * Configuration for streaming
 *
 */
TaskHandle_t dsp_stream_configure_streaming(DSP_CALLBACK_STREAM_CONFIG_PTR stream_config_ptr, TaskHandle_t task_id)
{
    dsp_stream_task_pointer_t task_parameter_ptr;
    task_parameter_ptr = dsp_stream_task_get_parameters(task_id);
    TaskHandle_t task_id_result;

    stream_config_ptr->stream_ptr = task_parameter_ptr->stream_ptr;
    stream_config_ptr->stream_number = task_parameter_ptr->stream_number;
    stream_config_ptr->task = task_parameter_ptr->task_id;
    task_id_result = DSP_Callback_StreamConfig(stream_config_ptr);
    if ((task_id_result == NULL_TASK_ID) && (stream_config_ptr->is_enable)) {
        DSP_MW_LOG_E("[Stream Task] stream config fail:%x!", 1, task_id);
        AUDIO_ASSERT(0);
    }
    return task_id_result;
}


/**
 * dsp_stream_get_callback
 *
 * Get stream callback ptr
 *
 */
DSP_CALLBACK_PTR dsp_stream_get_callback(SOURCE source, SINK sink)
{
    dsp_stream_task_pointer_t task_parameter_ptr;
    uint32_t i;
    task_parameter_ptr = dsp_stream_task_get_parameters(source->taskId);
    if (task_parameter_ptr->stream_ptr != NULL) {
        for (i = 0 ; i < task_parameter_ptr->stream_number ; i++) {
            if ((task_parameter_ptr->stream_ptr[i].streamingStatus != STREAMING_DISABLE) &&
                (task_parameter_ptr->stream_ptr[i].source == source) &&
                (task_parameter_ptr->stream_ptr[i].sink == sink)) {
                return (DSP_CALLBACK_PTR) & (task_parameter_ptr->stream_ptr[i].callback);
            }
        }
    }
    return NULL;
}


/**
 * dsp_stream_check_callback_status
 *
 * Whether all stream_task callback status SUSPEND/DISABLE
 *
 */
ATTR_TEXT_IN_IRAM_LEVEL_1 BOOL dsp_stream_check_callback_status(dsp_stream_task_pointer_t task_parameter_ptr)
{
    U32 i, stream_end;
    DSP_STREAMING_PARA_PTR stream_ptr;
    DSP_CALLBACK_STATUS status;

    for (i = 0 ; i < task_parameter_ptr->stream_number ; i++) {
        stream_ptr = &(task_parameter_ptr->stream_ptr[i]);
        status = stream_ptr->callback.Status;
        if (stream_ptr->streamingStatus == STREAMING_END) {
#ifdef AIR_AUDIO_HARDWARE_ENABLE
            if ((stream_ptr->sink->type == SINK_TYPE_AUDIO) &&(stream_ptr->sink->param.audio.irq_exist) &&(Audio_setting->Audio_sink.Zero_Padding_Cnt > 0)) {
                stream_end = 0;
            } else
#endif
            {
                stream_end = 1;
            }
        } else {
            stream_end = 0;
        }
        if (((status != CALLBACK_SUSPEND) &&
             (status != CALLBACK_DISABLE) &&
             (status!= CALLBACK_WAITEND)) ||
            stream_end) {
            return FALSE;
        }
    }
    return TRUE;
}

/**
 * dsp_stream_check_callback_status_special_case
 *
 * Whether all stream_task callback status SUSPEND/DISABLE (include special case)
 *
 */
BOOL dsp_stream_check_callback_status_special_case(dsp_stream_task_pointer_t task_parameter_ptr)
{
    U8 i;
    SINK sink;
    SOURCE source;

    for (i = 0 ; i < task_parameter_ptr->stream_number ; i++) {

        sink = task_parameter_ptr->stream_ptr[i].sink;
        source = task_parameter_ptr->stream_ptr[i].source;

        if ((sink == NULL) || (source == NULL)) {
            return FALSE;
        }

        if ((task_parameter_ptr->stream_ptr[i].streamingStatus == STREAMING_END) && (source->type == SOURCE_TYPE_A2DP) &&
            (sink->type == SINK_TYPE_AUDIO) && (sink->param.audio.irq_exist == true)) {
            return TRUE;
        }
    }

    return FALSE;
}


/**
 * dsp_stream_check_callback_status_disable
 *
 * Whether all stream_task callback status DISABLE
 *
 */
BOOL dsp_stream_check_callback_status_disable(dsp_stream_task_pointer_t task_parameter_ptr)
{
    U8 i;
    for (i = 0 ; i < task_parameter_ptr->stream_number ; i++) {
        if (task_parameter_ptr->stream_ptr[i].callback.Status != CALLBACK_DISABLE) {
            return FALSE;
        }
    }
    return TRUE;
}


/**
 * dsp_stream_suspend_request
 *
 * Suspend Request depend on callback status
 *
 */
ATTR_TEXT_IN_IRAM_LEVEL_1 VOID dsp_stream_suspend_request(VOID *parameter_ptr)
{
    dsp_stream_task_pointer_t task_parameter_ptr;
    task_parameter_ptr = (dsp_stream_task_pointer_t)parameter_ptr;
    U32 mask;

    hal_nvic_save_and_set_interrupt_mask(&mask);
    if (dsp_stream_check_callback_status(task_parameter_ptr)) {
        hal_nvic_restore_interrupt_mask(mask);
        vTaskSuspend(task_parameter_ptr->task_id);
        return;
    }
    hal_nvic_restore_interrupt_mask(mask);
}


/**
 * dsp_stream_deinit_all
 *
 * Deinit all active stream
 *
 */
VOID dsp_stream_deinit_all(VOID)
{
    U32 i, j;
    dsp_stream_task_pointer_t stream_task_ptr;

    for (i = 0 ;  i <  stream_task_num ;  i++) {
        stream_task_ptr = task_parameter + i;
        if (stream_task_ptr->task_id != NULL) {
            for (j = 0 ; j < stream_task_ptr->stream_number ; j++) {
                if (stream_task_ptr->stream_ptr[j].streamingStatus == STREAMING_ACTIVE) {
                    stream_task_ptr->stream_ptr[j].streamingStatus = STREAMING_DEINIT;
                }
            }
        }
    }
}


/**
 * dsp_stream_msg_ack
 *
 * message ack
 *
 */
VOID dsp_stream_msg_ack(DSP_STREAMING_PARA_PTR stream_ptr)
{
#ifdef AIR_BT_CLK_SKEW_ENABLE
        lt_clk_skew_notify_cm4();
#endif /* AIR_BT_CLK_SKEW_ENABLE */

    DSP_Callback_VpAck(stream_ptr);
}


/**
 * dsp_stream_status_update
 *
 * Udpate callback status
 *
 */
VOID dsp_stream_status_update(DSP_STREAMING_PARA_PTR stream_ptr)
{
    DSP_Callback_StatusUpdate(stream_ptr);
}


/**
 * dsp_stream_config_table_init
 *
 * Config table pointer
 *
 */
VOID dsp_stream_config_table_init(stream_task_config_ptr_t config_table_ptr, task_manage_ptr_t manage_table_ptr)
{
    stream_task_config_table_ptr = config_table_ptr;
    task_manage_table_ptr = manage_table_ptr;
}

/**
 * @brief This table is used to config task
 *
 * @param task_id is the task ID
 * @param stack_depth is the task stack depth
 * @param priority is the task prioity
 * @param no_of_stream is the number of streams(scenarios) assgined to the task
 * @param max_runtime is for debug. If task runtime>max_runtime, show debug information
 *
 */
const stream_task_config_t stream_task_config_table[DSP_STREAM_TASK_MAX_NUMBER+1] = {
    /*task_id,              task_name,          stack_depth,                           priority,                      no_of_stream,        max_runtime                              */
    {STREAM_TASK_PR,        "DPR_TASK",         0xc00 / sizeof(StackType_t),           TASK_PRIORITY_BELOW_NORMAL,    NO_OF_PR_STREAM,     DPR_TASK_MAX_RUNTIME},        /* DPR     */

#if defined(AIR_GAMING_MODE_DONGLE_ENABLE) || defined(AIR_BLE_AUDIO_DONGLE_ENABLE) || defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE) || defined(AIR_WIRELESS_MIC_RX_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_ENABLE)
    {STREAM_TASK_AV,        "DAV_TASK",         0x1800 / sizeof(StackType_t),          TASK_PRIORITY_SOFT_REALTIME,   NO_OF_AV_STREAM,     DAV_TASK_MAX_RUNTIME},        /* DAV     */
#elif defined(AIR_WIRED_AUDIO_ENABLE) && !defined(AIR_DCHS_MODE_ENABLE) && !defined(AIR_MIXER_STREAM_ENABLE)
    {STREAM_TASK_AV,        "DAV_TASK",         0x2D00 / sizeof(StackType_t),          TASK_PRIORITY_SOFT_REALTIME,   NO_OF_AV_STREAM,     DAV_TASK_MAX_RUNTIME},        /* DAV     */
#else
    {STREAM_TASK_AV,        "DAV_TASK",         0x2D00 / sizeof(StackType_t),          TASK_PRIORITY_NORMAL,          NO_OF_AV_STREAM,     DAV_TASK_MAX_RUNTIME},        /* DAV     */
#endif

#if defined(MTK_GAMING_MODE_HEADSET) || defined(AIR_GAMING_MODE_DONGLE_ENABLE) || defined(AIR_BLE_AUDIO_DONGLE_ENABLE) || defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE) || defined(AIR_ULL_BLE_HEADSET_ENABLE) || defined(AIR_WIRELESS_MIC_RX_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_ENABLE) || (defined(AIR_WIRED_AUDIO_ENABLE) && !defined(AIR_DCHS_MODE_ENABLE))
    {STREAM_TASK_HP,        "DHP_TASK",         0x1000 / sizeof(StackType_t),          TASK_PRIORITY_HARD_REALTIME,   NO_OF_HP_STREAM,     DHP_TASK_MAX_RUNTIME},        /* DHP     */
#else
    {STREAM_TASK_HP,        "DHP_TASK",         0x1000 / sizeof(StackType_t),          TASK_PRIORITY_ABOVE_NORMAL,    NO_OF_HP_STREAM,     DHP_TASK_MAX_RUNTIME},        /* DHP     */
#endif

#ifdef AIR_AUDIO_I2S_SLAVE_TDM_TASK_ENABLE
    {STREAM_TASK_TDM,       "DTDM_TASK",        0x1400 / sizeof(StackType_t),          TASK_PRIORITY_HIGH,            NO_OF_TDM_STREAM,    DTDM_TASK_MAX_RUNTIME},       /* TDM     */
#endif
#if defined AIR_DCHS_MODE_ENABLE || defined AIR_MIXER_STREAM_ENABLE
    {STREAM_TASK_DCHS,      "DDCHS_TASK",       0x1000 / sizeof(StackType_t),          TASK_PRIORITY_HARD_REALTIME,            NO_OF_DCHS_STREAM,   DDCHS_TASK_MAX_RUNTIME},      /* DCHS    */
#endif

    //don't modify below
    {STREAM_TASK_END,      "END_TASK",          0,                                     0,                             0,                   0},                           /* END    */
};

/**
 * @brief This table is used to assign scenarios to the task
 *
 * @param scenario_type is user scenario ID
 * @param task_id is the task ID
 *
 */
const task_manage_t task_manage_table[] = {
    /*scenario_type,                                            task_id,                afe_buffer_threshold(units:the percentage of samples)  */

    //don't modify below
    {AUDIO_SCENARIO_TYPE_END,                                   0,                      (0),  },
};

void stream_task_table_initialize(void)
{
    dsp_stream_config_table_init((stream_task_config_ptr_t)stream_task_config_table, (task_manage_ptr_t)task_manage_table);
}

