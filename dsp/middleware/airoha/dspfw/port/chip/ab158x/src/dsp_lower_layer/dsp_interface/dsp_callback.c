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

#include "config.h"
#include "types.h"
#include "dlist.h"
#include "dsp_memory.h"
#include "source.h"
#include "sink.h"
#include "source_inter.h"
#include "sink_inter.h"
#include "dllt.h"
#include "dsp_drv_dfe.h"
#include "stream_audio.h"
#include "stream.h"
#include "audio_config.h"
#include "dsp_audio_ctrl.h"
#include "dsp_stream_task.h"
#include "hal_audio_volume.h"
#include "dsp_audio_msg.h"
#include "dsp_task.h"

#ifdef MTK_BT_A2DP_SBC_ENABLE
#include "sbc_interface.h"
#endif /* MTK_BT_A2DP_SBC_ENABLE */

#include "dsp_sdk.h"
#include "dsp_audio_process.h"
#include "dsp_feature_interface.h"
#include "dsp_callback.h"
#include "dsp_audio_msg_define.h"
//#include "sbc_interface.h"
//#include "mp3_dec_interface.h"
//#include "aac_dec_interface.h"
#include <string.h>
#include "FreeRTOS.h"
#include "stream_audio_afe.h"
#ifdef AIR_SOFTWARE_MIXER_ENABLE
#include "sw_mixer_interface.h"
#endif /* AIR_SOFTWARE_MIXER_ENABLE */

#ifdef MTK_CELT_DEC_ENABLE
#include "celt_dec_interface.h"
#endif

#ifdef AIR_BT_CODEC_BLE_V2_ENABLED
#include "lc3_dec_interface_v2.h"
#endif /* AIR_BT_CODEC_BLE_V2_ENABLED */

#ifdef AIR_BT_HFP_ENABLE
#ifdef AIR_FIXED_RATIO_SRC
#include "stream_n9sco.h"
#endif
#endif

#if defined(AIR_WIRED_AUDIO_ENABLE)
#include "scenario_wired_audio.h"
#endif

#ifdef MTK_BT_A2DP_ENABLE
#include "stream_n9_a2dp.h"
#endif

#include "dsp_memory_region.h"
#include "preloader_pisplit_internal.h"

#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
#include "scenario_ull_audio_v2.h"
#endif /* AIR_ULL_AUDIO_V2_DONGLE_ENABLE */

#if defined(AIR_WIRELESS_MIC_RX_ENABLE)
#include "scenario_wireless_mic_rx.h"
#endif /* AIR_WIRELESS_MIC_RX_ENABLE */

#if defined(AIR_BLE_FIXED_RATIO_SRC_ENABLE) && defined(AIR_FIXED_RATIO_SRC)
#include "stream_n9ble.h"
#endif

#ifdef AIR_DCHS_MODE_ENABLE
#include "stream_dchs.h"
#endif

#ifdef AIR_BT_AUDIO_DONGLE_ENABLE
#include "scenario_bt_audio.h"
#endif /* AIR_BT_AUDIO_DONGLE_ENABLE */

#ifdef AIR_FULL_ADAPTIVE_ANC_ENABLE
#include "full_adapt_anc_api.h"
#endif

#ifdef AIR_HW_VIVID_PT_ENABLE
#include "hw_vivid_passthru_api.h"
#endif

#ifdef AIR_MIXER_STREAM_ENABLE
#include "stream_mixer.h"
#endif

////////////////////////////////////////////////////////////////////////////////
// Constant Definitions ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#define DSP_CALLBACK_SKIP_ALL_PROCESS   (0xFF)

#define AUDIO_DSP_STREAM_CALLBACK_PROCESS_DEBUG_ENABLE            (0)

////////////////////////////////////////////////////////////////////////////////
// External Function Prototypes/////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
EXTERN VOID DSPMEM_CheckFeatureMemory(VOID *VOLATILE para, DSP_MEMORY_CHECK_TIMING timing);
EXTERN VOID vend_bc_write_loop_callback(VOID);
extern bool ScoDlStopFlag;
extern bool BleDlStopFlag;

#ifdef AIR_PROMPT_SOUND_MEMORY_DEDICATE
extern U8* g_vp_memptr;
#endif
EXTERN task_manage_ptr_t task_manage_table_ptr;
#ifdef MTK_PROMPT_SOUND_ENABLE
extern volatile uint32_t vp_sram_empty_flag;
extern volatile uint32_t vp_config_flag;
extern volatile uint32_t vp_data_request_flag;
#endif
////////////////////////////////////////////////////////////////////////////////
BOOL DSP_Callback_Undo(DSP_ENTRY_PARA_PTR entry_para, DSP_FEATURE_TABLE_PTR feature_table_ptr);
BOOL DSP_Callback_Malloc(DSP_ENTRY_PARA_PTR entry_para, DSP_FEATURE_TABLE_PTR feature_table_ptr);
BOOL DSP_Callback_Init(DSP_ENTRY_PARA_PTR entry_para, DSP_FEATURE_TABLE_PTR feature_table_ptr);
BOOL DSP_Callback_Handler(DSP_ENTRY_PARA_PTR entry_para, DSP_FEATURE_TABLE_PTR feature_table_ptr);
BOOL DSP_Callback_ZeroPadding(DSP_ENTRY_PARA_PTR entry_para, DSP_FEATURE_TABLE_PTR feature_table_ptr);
VOID DSP_Callback_StreamingRateConfig(SOURCE source, SINK sink);


////////////////////////////////////////////////////////////////////////////////
// Global Variables ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#ifdef MTK_BT_A2DP_ENABLE
volatile bool g_a2dp_hwsrc_ng_flag = false;
#endif

const CALLBACK_STATE_ENTRY DSP_CallbackEntryTable[] = {
    DSP_Callback_Undo,              /*CALLBACK_DISABLE*/
    DSP_Callback_Malloc,            /*CALLBACK_MALLOC*/
    DSP_Callback_Init,              /*CALLBACK_INIT*/
    DSP_Callback_Undo,              /*CALLBACK_SUSPEND*/
    DSP_Callback_Handler,           /*CALLBACK_HANDLER*/
    DSP_Callback_ZeroPadding,       /*CALLBACK_BYPASSHANDLER*/
    DSP_Callback_ZeroPadding,       /*CALLBACK_ZEROPADDING*/
    DSP_Callback_Undo,              /*CALLBACK_WAITEND*/
};



////////////////////////////////////////////////////////////////////////////////
// DSP FUNCTION DECLARATIONS ///////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/**
 * DSP_Callback_Undo
 *
 * @Author :  BrianChen <BrianChen@airoha.com.tw>
 */
BOOL DSP_Callback_Undo(DSP_ENTRY_PARA_PTR entry_para, DSP_FEATURE_TABLE_PTR feature_table_ptr)
{
    UNUSED(entry_para);
    UNUSED(feature_table_ptr);
    return FALSE;
}

/**
 * DSP_Callback_Malloc
 *
 * @Author :  BrianChen <BrianChen@airoha.com.tw>
 */
BOOL DSP_Callback_Malloc(DSP_ENTRY_PARA_PTR entry_para, DSP_FEATURE_TABLE_PTR feature_table_ptr)
{
#ifndef AIR_DSP_MEMORY_REGION_ENABLE
    DSP_STREAMING_PARA_PTR  stream_ptr;
    stream_ptr = DSP_STREAMING_GET_FROM_PRAR(entry_para);
    entry_para->number.field.process_sequence = 1;
    if (feature_table_ptr != NULL) {
        while (feature_table_ptr->ProcessEntry != stream_function_end_process && feature_table_ptr->ProcessEntry != NULL) {
            DSP_MW_LOG_I("[DSP_RESOURCE][DSP_Callback_Malloc] scenario: %d Feature ID : 0x%x, working buffer malloc size %d", 3
                , stream_ptr && stream_ptr->source ? stream_ptr->source->scenario_type : AUDIO_SCEANRIO_TYPE_MAX
                , feature_table_ptr->FeatureType
                , feature_table_ptr->MemSize
            );
            feature_table_ptr->MemPtr = DSPMEM_tmalloc(entry_para->DSPTask, feature_table_ptr->MemSize, stream_ptr);
            feature_table_ptr++;
            entry_para->number.field.process_sequence++;
        }
    }
#else
    UNUSED(entry_para);
    UNUSED(feature_table_ptr);
#endif
    return FALSE;
}

/**
 * DSP_CleanUpCallbackOutBuf
 *
 * @Author :  BrianChen <BrianChen@airoha.com.tw>
 */
void DSP_CleanUpCallbackOutBuf(DSP_ENTRY_PARA_PTR entry_para)
{
    U32 i;
    for (i = 0 ; i < entry_para->out_channel_num ; i++) {
        memset(entry_para->out_ptr[i],
               0,
               entry_para->out_malloc_size);
    }
    if ((entry_para->out_channel_num == 1) && (entry_para->out_ptr[1] != NULL)) { //Also clean up codec out buffer
        memset(entry_para->out_ptr[1],
               0,
               entry_para->out_malloc_size);
    }
}

/**
 * DSP_CallbackCodecRecord
 *
 * @Author :  BrianChen <BrianChen@airoha.com.tw>
 */
ATTR_TEXT_IN_IRAM_LEVEL_1 void DSP_CallbackCodecRecord(DSP_ENTRY_PARA_PTR entry_para)
{
    if (entry_para->number.field.process_sequence == CODEC_ALLOW_SEQUENCE) {
        entry_para->pre_codec_out_sampling_rate = entry_para->codec_out_sampling_rate;
        entry_para->pre_codec_out_size = entry_para->codec_out_size;
        entry_para->pre_out_channel_num = entry_para->out_channel_num;
    }
}

/**
 * DSP_CallbackCheckResolution
 *
 * @Author :  BrianChen <BrianChen@airoha.com.tw>
 */
ATTR_TEXT_IN_IRAM_LEVEL_1 void DSP_CallbackCheckResolution(DSP_ENTRY_PARA_PTR entry_para)
{
    if (entry_para->number.field.process_sequence == CODEC_ALLOW_SEQUENCE) {
        if (entry_para->resolution.process_res != entry_para->resolution.feature_res) {
            //Warning MSG
        }
    }
}


/**
 * DSP_Callback_Init
 *
 * @Author :  BrianChen <BrianChen@airoha.com.tw>
 */
BOOL DSP_Callback_Init(DSP_ENTRY_PARA_PTR entry_para, DSP_FEATURE_TABLE_PTR feature_table_ptr)
{
    DSP_STREAMING_PARA_PTR  stream_ptr;
    stream_ptr = DSP_STREAMING_GET_FROM_PRAR(entry_para);

    entry_para->number.field.process_sequence = 1;
    entry_para->resolution.process_res = entry_para->resolution.feature_res;

    if (feature_table_ptr != NULL) {
        while (feature_table_ptr->ProcessEntry != NULL) {
            DSP_MW_LOG_I("[DSP_RESOURCE][DSP_Callback_Init] scenario: %d Feature ID: 0x%x", 2
                , stream_ptr && stream_ptr->source ? stream_ptr->source->scenario_type : AUDIO_SCEANRIO_TYPE_MAX
                , feature_table_ptr->FeatureType
            );
            entry_para->mem_ptr = feature_table_ptr->MemPtr;
            if ((feature_table_ptr->InitialEntry(entry_para))) {
                DSP_CleanUpCallbackOutBuf(entry_para);
                return TRUE;
            }
            if (feature_table_ptr->ProcessEntry == stream_function_end_process) {
                break;
            }
            DSP_CallbackCodecRecord(entry_para);
            entry_para->number.field.process_sequence++;
            feature_table_ptr++;
        }
        DSP_MW_LOG_I("[DSP_RESOURCE][DSP_Callback_Init] scenario: %d Feature PIC init end", 1
            , stream_ptr && stream_ptr->source ? stream_ptr->source->scenario_type : AUDIO_SCEANRIO_TYPE_MAX
        );
    }
    return FALSE;
}

/**
 * DSP_Callback_Handler
 *
 * @Author :  BrianChen <BrianChen@airoha.com.tw>
 */
ATTR_TEXT_IN_IRAM_LEVEL_1 BOOL DSP_Callback_Handler(DSP_ENTRY_PARA_PTR entry_para, DSP_FEATURE_TABLE_PTR feature_table_ptr)
{
    DSP_STREAMING_PARA_PTR  stream_ptr = DSP_STREAMING_GET_FROM_PRAR(entry_para);
    BOOL result = FALSE;
    entry_para->number.field.process_sequence = 1;
    entry_para->codec_out_sampling_rate = entry_para->pre_codec_out_sampling_rate;
    entry_para->codec_out_size = entry_para->pre_codec_out_size;
    entry_para->resolution.process_res = entry_para->resolution.source_in_res;
    entry_para->out_channel_num = entry_para->pre_out_channel_num;

    if (entry_para->ch_swapped) {
        S32 *BufL = (S32 *)stream_function_get_1st_inout_buffer(entry_para);
        S32 *BufR = (S32 *)stream_function_get_2nd_inout_buffer(entry_para);
        if ((BufL != NULL) && (BufR != NULL)) {
            entry_para->out_ptr[0] = BufR - entry_para->out_ptr_offset;
            entry_para->out_ptr[1] = BufL - entry_para->out_ptr_offset;
        }
        entry_para->ch_swapped = FALSE;
    }

    if (feature_table_ptr != NULL) {
        while (feature_table_ptr->ProcessEntry != NULL) {
            entry_para->mem_ptr = feature_table_ptr->MemPtr;
            if (DSP_Callback_SRC_Triger_Chk(&stream_ptr->callback)) {
                if (!(entry_para->skip_process == entry_para->number.field.process_sequence)) {
#ifdef DSP_MIPS_FEATURE_PROFILE
                    stream_feature_entry_t processEntry = feature_table_ptr->ProcessEntry;
                    bool process_ret;
                    U32 gpt_time_start, gpt_time_end, gpt_time_duration;
                    U32 mask;

                    hal_nvic_save_and_set_interrupt_mask(&mask);

                    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_time_start);
                    process_ret = processEntry(entry_para);
                    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_time_end);

                    hal_nvic_restore_interrupt_mask(mask);

                    gpt_time_duration = gpt_time_end - gpt_time_start;

                    feature_table_ptr->FeatureTimeSum += gpt_time_duration;
                    feature_table_ptr->FeatureTimeCnt++;

                    if(gpt_time_duration > feature_table_ptr->FeatureTimeMax)
                        feature_table_ptr->FeatureTimeMax = gpt_time_duration;

                    #if 0
                    DSP_MW_LOG_I("[DSP_MIPS] Feature ID: 0x%X, time: %d us, MAX time: %d us", 3
                        , feature_table_ptr->FeatureType
                        , gpt_time_duration
                        , feature_table_ptr->FeatureTimeMax
                    );
                    #endif

                    if(feature_table_ptr->FeatureTimeCnt >= DSP_MIPS_FEATURE_CNT)
                    {
                        DSP_MW_LOG_I("[DSP_MIPS] scenario: %d Feature ID: 0x%X, AVG time: %d us, MAX time: %d us", 4
                            , stream_ptr && stream_ptr->source ? stream_ptr->source->scenario_type : AUDIO_SCEANRIO_TYPE_MAX
                            , feature_table_ptr->FeatureType
                            , feature_table_ptr->FeatureTimeSum >> DSP_MIPS_FEATURE_DIV_SHIFT
                            , feature_table_ptr->FeatureTimeMax
                        );
                        feature_table_ptr->FeatureTimeSum = 0;
                        feature_table_ptr->FeatureTimeCnt = 0;
                        feature_table_ptr->FeatureTimeMax = 0;
                    }

                    if (process_ret) {
#else
                     if ((feature_table_ptr->ProcessEntry(entry_para))) {
#endif

#ifdef AIR_SOFTWARE_MIXER_ENABLE
                        if (feature_table_ptr->ProcessEntry == stream_function_sw_mixer_process) {
                            /* in here, it means software mixer wants to bypass all subsequent features */
                            result = FALSE;
                            break;
                        }
#endif /* AIR_SOFTWARE_MIXER_ENABLE */

                        DSP_CleanUpCallbackOutBuf(entry_para);
                        DSP_MW_LOG_I("handler return true", 0);
                        result = TRUE;
                        break;
                    }
                    DSP_CallbackCodecRecord(entry_para);
                } else {
                    DSP_CleanUpCallbackOutBuf(entry_para);
                }
            }
            DSP_CallbackCheckResolution(entry_para);
            if (feature_table_ptr->ProcessEntry == stream_function_end_process) {
                break;
            }
            entry_para->number.field.process_sequence++;
            feature_table_ptr++;
        }
    } else {
        stream_pcm_copy_process(entry_para);
        result = TRUE;
    }
    return result;
}

/**
 * DSP_Callback_ZeroPadding
 *
 * @Author :  BrianChen <BrianChen@airoha.com.tw>
 */
BOOL DSP_Callback_ZeroPadding(DSP_ENTRY_PARA_PTR entry_para, DSP_FEATURE_TABLE_PTR feature_table_ptr)
{
    DSP_STREAMING_PARA_PTR  stream_ptr = DSP_STREAMING_GET_FROM_PRAR(entry_para);
    BOOL result = FALSE;

    entry_para->number.field.process_sequence = 1;
    entry_para->codec_out_sampling_rate = entry_para->pre_codec_out_sampling_rate;
    entry_para->codec_out_size = entry_para->pre_codec_out_size;
    entry_para->resolution.process_res = entry_para->resolution.feature_res;
    entry_para->out_channel_num = entry_para->pre_out_channel_num;

    DSP_CleanUpCallbackOutBuf(entry_para);

    if (feature_table_ptr != NULL) {
        while (feature_table_ptr->ProcessEntry != NULL) {
            entry_para->mem_ptr = feature_table_ptr->MemPtr;

            if (DSP_Callback_SRC_Triger_Chk(&stream_ptr->callback)) {
                if (!(entry_para->skip_process == entry_para->number.field.process_sequence)) {
                    if (feature_table_ptr->ProcessEntry(entry_para)) {
                        if (entry_para->number.field.process_sequence != 1) {
                            DSP_CleanUpCallbackOutBuf(entry_para);
                            break;
                        } else {
                            entry_para->codec_out_size = 0;
                        }
                    }
                    DSP_CallbackCodecRecord(entry_para);
                    if ((entry_para->codec_out_size == 0) && (entry_para->bypass_mode != BYPASS_CODEC_MODE)) {
                        DSP_CleanUpCallbackOutBuf(entry_para);
                        entry_para->codec_out_size = entry_para->pre_codec_out_size;
                        result = TRUE;
                    }
                } else {
                    result = TRUE;
                }
            }
            DSP_CallbackCheckResolution(entry_para);
            if (feature_table_ptr->ProcessEntry == stream_function_end_process) {
                break;
            }
            entry_para->number.field.process_sequence++;
            feature_table_ptr++;
        }
    }
    return result;
}

/**
 * DSP_Callback_Config
 *
 * @Author :  BrianChen <BrianChen@airoha.com.tw>
 */
TaskHandle_t  DSP_Callback_Config(SOURCE source, SINK sink, VOID *feature_list_ptr, BOOL isEnable)
{
    TaskHandle_t  dsp_task_id = NULL_TASK_ID;
    task_manage_ptr_t scenario_table_ptr;

#if (ForceDSPCallback)

#ifdef AIR_AUDIO_HARDWARE_ENABLE
    Audio_Default_setting_init();
#endif /* AIR_AUDIO_HARDWARE_ENABLE */

//    BOOL IsSourceAudio = TRUE;
//    BOOL IsSinkAudio = TRUE;

    if (isEnable) {
        /* isEnable == true */
    if (sink->type == SINK_TYPE_VP_AUDIO) {
        source->taskId = DPR_TASK_ID;
        sink->taskid = DPR_TASK_ID;
#ifdef MTK_PROMPT_SOUND_DUMMY_SOURCE_ENABLE
        if (source->type == SOURCE_TYPE_CM4_VP_DUMMY_SOURCE_PLAYBACK) {
            source->taskId = DHP_TASK_ID;
            sink->taskid = DHP_TASK_ID;
        }
#endif
#if defined (AIR_WIRED_AUDIO_ENABLE)
        } else if ((sink->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_0) ||
                    (sink->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_1) ||
                    (sink->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_IN) ||
                    (sink->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_DUAL_CHIP_LINE_IN_MASTER) ||
                    (sink->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_DUAL_CHIP_LINE_IN_SLAVE)) {
            source->taskId = DHP_TASK_ID;
            sink->taskid = DHP_TASK_ID;
#if !defined(AIR_DCHS_MODE_ENABLE)
        } else if ((sink->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_OUT) ||
                    (sink->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_OUT) ||
                    (sink->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_DUAL_CHIP_LINE_OUT_MASTER)) {
            source->taskId = DAV_TASK_ID;
            sink->taskid = DAV_TASK_ID;

#endif
#endif
#ifndef MTK_BT_A2DP_AIRO_CELT_ENABLE
    } else if (sink->type == SINK_TYPE_CM4RECORD) {
        source->taskId = DPR_TASK_ID;
        sink->taskid = DPR_TASK_ID;
#endif
#ifdef AIR_MULTI_MIC_STREAM_ENABLE
    } else if ((source->type >= SOURCE_TYPE_SUBAUDIO_MIN) && (source->type <= SOURCE_TYPE_SUBAUDIO_MAX)) {
        //source->taskId = DHP_TASK_ID;
        //sink->taskid = DHP_TASK_ID;
#endif
#ifdef MTK_BT_A2DP_AIRO_CELT_ENABLE
    } else if ((source->type == SOURCE_TYPE_A2DP)
               && (source->param.n9_a2dp.codec_info.codec_cap.type == BT_A2DP_CODEC_AIRO_CELT)) {
        source->taskId = DHP_TASK_ID;
        sink->taskid = DHP_TASK_ID;
#endif
    #if defined(AIR_BT_CODEC_BLE_ENABLED)
        } else if (source->type == SOURCE_TYPE_N9BLE) {
            source->taskId = DHP_TASK_ID;
            sink->taskid = DHP_TASK_ID;
#ifdef AIR_WIRELESS_MIC_TX_ENABLE
        } else if (sink->type == SINK_TYPE_N9BLE) {
        source->taskId = DHP_TASK_ID;
        sink->taskid = DHP_TASK_ID;
#endif
#endif
#ifdef AIR_FULL_ADAPTIVE_ANC_ENABLE
    } else if (source->type == SOURCE_TYPE_ADAPT_ANC) {
        source->taskId = DHP_TASK_ID;
        sink->taskid   = DHP_TASK_ID;
#endif
#ifdef AIR_HW_VIVID_PT_ENABLE
    } else if (source->type == SOURCE_TYPE_HW_VIVID_PT) {
        source->taskId = DHP_TASK_ID;
        sink->taskid   = DHP_TASK_ID;
#endif
#if defined(AIR_HEARTHROUGH_MAIN_ENABLE) || defined(AIR_ADVANCED_PASSTHROUGH_ENABLE) || defined(AIR_CUSTOMIZED_LLF_ENABLE)
        } else if ((source->type == SOURCE_TYPE_LLF)
                   || ((sink->type == SINK_TYPE_AUDIO_DL3) && (source->param.audio.scenario_id == AUDIO_TRANSMITTER_ADVANCED_PASSTHROUGH))) {
        source->taskId = DLL_TASK_ID;
        sink->taskid   = DLL_TASK_ID;
#endif
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
        } else if ((source->type == SOURCE_TYPE_TDMAUDIO) || (sink->type == SINK_TYPE_TDMAUDIO)) {
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_TASK_ENABLE
            source->taskId = DTDM_TASK_ID;
            sink->taskid = DTDM_TASK_ID;
#else
            source->taskId = DHP_TASK_ID;
            sink->taskid = DHP_TASK_ID;
#endif
#endif
        }
#if defined (AIR_DCHS_MODE_ENABLE)
        if(source->scenario_type == AUDIO_SCENARIO_TYPE_DCHS_UART_UL){
            source->taskId = DAV_TASK_ID;
            sink->taskid = DAV_TASK_ID;
        }
#endif
        if((source->type >= SOURCE_TYPE_DSP_VIRTUAL_MIN)&&(source->type <= SOURCE_TYPE_DSP_VIRTUAL_MAX)){
            //sink->taskid = DAV_TASK_ID;  //TODO
            source->taskId = sink->taskid;
    }
    /* check if both of source task id and sink task id are not configured */
    if ((source->taskId != DPR_TASK_ID) &&
        (sink->taskid   != DPR_TASK_ID) &&
        (source->taskId != DAV_TASK_ID) &&
        (sink->taskid   != DAV_TASK_ID) &&
        (source->taskId != DHP_TASK_ID) &&
        (sink->taskid   != DHP_TASK_ID)
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_TASK_ENABLE
        && (source->taskId != DTDM_TASK_ID)
        && (sink->taskid   != DTDM_TASK_ID)
#endif
#if defined(AIR_ADVANCED_PASSTHROUGH_ENABLE) || defined(AIR_HEARTHROUGH_MAIN_ENABLE) || defined(AIR_CUSTOMIZED_LLF_ENABLE)
        && (source->taskId != DLL_TASK_ID)
        && (sink->taskid   != DLL_TASK_ID)
#endif
#ifdef AIR_MIXER_STREAM_ENABLE
            && (source->taskId != DDCHS_TASK_ID)
            && (sink->taskid   != DDCHS_TASK_ID)
#endif
       ) {
        /* if yes, set them to the default AV task ID */
        source->taskId = DAV_TASK_ID;
        sink->taskid   = DAV_TASK_ID;
    }

    /* check task id status */
    if ((source->taskId == DHP_TASK_ID) || (sink->taskid == DHP_TASK_ID)) {
        /* if anyone is DHP_TASK_ID, this stream should be run on HP Task */
        source->taskId = DHP_TASK_ID;
        sink->taskid   = DHP_TASK_ID;
    } else if ((source->taskId == DPR_TASK_ID) || (sink->taskid == DPR_TASK_ID)) {
        /* if anyone is DPR_TASK_ID, this stream should be run on PR Task */
        source->taskId = DPR_TASK_ID;
        sink->taskid   = DPR_TASK_ID;
    } else if ((source->taskId == DAV_TASK_ID) || (sink->taskid == DAV_TASK_ID)) {
        /* other case, this stream should be run on AV Task */
        source->taskId = DAV_TASK_ID;
        sink->taskid   = DAV_TASK_ID;
#if defined(AIR_ADVANCED_PASSTHROUGH_ENABLE) || defined(AIR_HEARTHROUGH_MAIN_ENABLE) || defined(AIR_CUSTOMIZED_LLF_ENABLE)
        } else if ((source->taskId == DLL_TASK_ID) || (sink->taskid == DLL_TASK_ID)) {
        /* if anyone is DLL_TASK_ID, this stream should be run on LL Task */
        source->taskId = DLL_TASK_ID;
        sink->taskid   = DLL_TASK_ID;
#endif
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_TASK_ENABLE
        } else if ((source->taskId == DTDM_TASK_ID) || (sink->taskid == DTDM_TASK_ID)) {
            /* other case, this stream should be run on TDM Task */
        source->taskId = DTDM_TASK_ID;
        sink->taskid = DTDM_TASK_ID;
#endif
#ifdef AIR_MIXER_STREAM_ENABLE
        } else if ((source->taskId == DDCHS_TASK_ID) || (sink->taskid == DDCHS_TASK_ID)) {
            /* other case, this stream should be run on DCHS Task */
            source->taskId = DDCHS_TASK_ID;
            sink->taskid   = DDCHS_TASK_ID;
#endif
        }
    } else {
        /* isEnable == false */
        if (sink->type == SINK_TYPE_DSP_VIRTUAL) {
            sink->taskid = source->taskId;
        }
    }
#else

    BOOL IsSourceAudio = ((source->type == SOURCE_TYPE_AUDIO))
                         ? TRUE
                         : FALSE;
    BOOL IsSinkAudio   = ((sink->type == SINK_TYPE_AUDIO) ||
                          (sink->type == SINK_TYPE_VP_AUDIO) ||
                          (sink->type == SINK_TYPE_AUDIO_DL3) ||
                          (sink->type == SINK_TYPE_AUDIO_DL12))
                         ? TRUE
                         : FALSE;
    BOOL IsBranchJoint = ((sink->type == SINK_TYPE_DSP_JOINT) || (source->type == SOURCE_TYPE_DSP_BRANCH));
#endif
    scenario_table_ptr = task_manage_table_ptr;
    while (scenario_table_ptr->scenario_type != AUDIO_SCENARIO_TYPE_END) {
            if (sink->scenario_type == scenario_table_ptr->scenario_type) {
                if (isEnable){
                    DSP_MW_LOG_I("[Stream Task] scenario_type:%d, task_id:%d", 2,scenario_table_ptr->scenario_type,scenario_table_ptr->task_id);
                }
                if (pStreamTaskHandler[scenario_table_ptr->task_id] == NULL){
                    DSP_MW_LOG_E("[Stream Task] stream_task_%d not opened!",1,scenario_table_ptr->task_id);
                    assert(0);
                }

                source->taskId = pStreamTaskHandler[scenario_table_ptr->task_id];
                sink->taskid = pStreamTaskHandler[scenario_table_ptr->task_id];
            }
            scenario_table_ptr++;
    }

    DSP_CALLBACK_STREAM_CONFIG stream_config;
    stream_config.is_enable = isEnable;
    stream_config.source = source;
    stream_config.sink = sink;
    stream_config.feature_list_ptr = feature_list_ptr;
#if (ForceDSPCallback)
    dsp_task_id = NULL_TASK_ID;
#else
    if ((IsSourceAudio && IsSinkAudio && (source->taskId != sink->taskid)) ||
        ((!IsSourceAudio) && (!IsSinkAudio) && (!IsBranchJoint))) {
        dsp_task_id = DAV_TASK_ID;
    }else
#endif
#if defined(AIR_ADVANCED_PASSTHROUGH_ENABLE) || defined(AIR_HEARTHROUGH_MAIN_ENABLE) || defined(AIR_CUSTOMIZED_LLF_ENABLE)
    if ((source->taskId == DLL_TASK_ID)||(sink->taskid == DLL_TASK_ID))
    {
        dsp_task_id = DLLT_StreamingConfig(&stream_config);
    }else
#endif
    {
        dsp_task_id=dsp_stream_configure_streaming(&stream_config,source->taskId);
    }


    return dsp_task_id;
}

VOID DSP_Callback_StreamingInit(DSP_STREAMING_PARA_PTR stream_ptr, U8 stream_number, TaskHandle_t  task)
{
    U8 i, j;
    for (i = 0 ; i < stream_number ; i++) {
        stream_ptr[i].source                                   = NULL;
        stream_ptr[i].sink                                     = NULL;
        stream_ptr[i].callback.Status                          = CALLBACK_DISABLE;
        stream_ptr[i].callback.EntryPara.DSPTask               = task;
        stream_ptr[i].callback.EntryPara.with_encoder          = FALSE;
        stream_ptr[i].callback.EntryPara.with_src              = FALSE;
        stream_ptr[i].callback.EntryPara.in_malloc_size        = 0;
        stream_ptr[i].callback.EntryPara.out_malloc_size       = 0;
        stream_ptr[i].callback.EntryPara.bypass_mode           = 0;
        for (j = 0 ; j < CALLBACK_INPUT_PORT_MAX_NUM ; j++) {
            stream_ptr[i].callback.EntryPara.in_ptr[j]         = NULL;
        }
        for (j = 0 ; j < CALLBACK_OUTPUT_PORT_MAX_NUM ; j++) {
            stream_ptr[i].callback.EntryPara.out_ptr[j]        = NULL;
        }
        stream_ptr[i].callback.FeatureTablePtr                 = NULL;
        stream_ptr[i].callback.Src.src_ptr                     = NULL;
        stream_ptr[i].callback.EntryPara.scenario_type         = AUDIO_SCENARIO_TYPE_COMMON;
        /*
        stream_ptr[i].driftCtrl.para.comp_mode                 = DISABLE_COMPENSATION;
        stream_ptr[i].driftCtrl.para.src_ptr                   = NULL;
        stream_ptr[i].driftCtrl.SourceLatch                    = NonLatch;
        stream_ptr[i].driftCtrl.SinkLatch                      = NonLatch;
        */
        stream_ptr[i].DspReportEndId                           = MSG_DSP_NULL_REPORT;
        stream_ptr[i].streamingStatus                          = STREAMING_DISABLE;
    }
}

VOID DSP_Callback_StreamClean(VOID *ptr)
{
    DSP_CALLBACK_STREAM_CLEAN_PTR clean_ptr     = ptr;

    if ((clean_ptr->stream_ptr->callback.Status != CALLBACK_SUSPEND) &&
        (clean_ptr->stream_ptr->callback.Status != CALLBACK_DISABLE) &&
        (clean_ptr->stream_ptr->callback.Status != CALLBACK_WAITEND)) {
        clean_ptr->is_clean                         = FALSE;
    } else {
        clean_ptr->is_clean                         = TRUE;
        clean_ptr->stream_ptr->source               = NULL;
        clean_ptr->stream_ptr->sink                 = NULL;
        clean_ptr->stream_ptr->callback.Status      = CALLBACK_DISABLE;
        clean_ptr->stream_ptr->streamingStatus      = STREAMING_END;
    }
}

TaskHandle_t  DSP_Callback_StreamConfig(DSP_CALLBACK_STREAM_CONFIG_PTR stream_config_ptr)
{
    U8 i;
    stream_feature_list_ptr_t featureListPtr = stream_config_ptr->feature_list_ptr;
    UNUSED(featureListPtr);
    if (stream_config_ptr->is_enable) {
        for (i = 0 ; i < stream_config_ptr->stream_number ; i++) {
            if ((stream_config_ptr->stream_ptr[i].source == stream_config_ptr->source) &&
                (stream_config_ptr->stream_ptr[i].sink == stream_config_ptr->sink)) {
                return stream_config_ptr->task;
            }
        }
        for (i = 0 ; i < stream_config_ptr->stream_number ; i++) {
            if (stream_config_ptr->feature_list_ptr == NULL) {
                return NULL_TASK_ID;
            }
            if (stream_config_ptr->stream_ptr[i].streamingStatus == STREAMING_DISABLE) {
#if defined(AIR_WIRED_AUDIO_ENABLE)
                if ((stream_config_ptr->source->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_MAINSTREAM)
                ||(stream_config_ptr->source->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_OUT_IEM)) {
                    uint32_t j;
                    /* The main stream MUST locate in the last position, or the latency will increase 1 frame interval. */
                    for (j = stream_config_ptr->stream_number; j > 0; j--) {
                        if (stream_config_ptr->stream_ptr[j - 1].streamingStatus == STREAMING_DISABLE) {
                            i = j - 1;
                            DSP_MW_LOG_I("[wired_audio]: main stream create with index %d of HPStreaming[]", 1, i);
                            break;
                        }
                    }
                }
#endif
                DSP_Callback_StreamingRateConfig(stream_config_ptr->source, stream_config_ptr->sink); // Sink rate follows only now
                stream_config_ptr->stream_ptr[i].source                      = stream_config_ptr->source;
                stream_config_ptr->stream_ptr[i].sink                        = stream_config_ptr->sink;
                stream_config_ptr->stream_ptr[i].streamingStatus             = STREAMING_START;
                DSP_MW_LOG_I("DSP - Create Callback Stream, Source:%d, Sink:%d, Codec:%x", 3, stream_config_ptr->source->type, stream_config_ptr->sink->type, (*featureListPtr) & 0xFF);
                DSP_Callback_FeatureConfig((DSP_STREAMING_PARA_PTR)&stream_config_ptr->stream_ptr[i], stream_config_ptr->feature_list_ptr);
                dsp_sdk_set_scenario_status(stream_config_ptr->source->scenario_type, true);
                return stream_config_ptr->task;
            }
        }
    } else {
        for (i = 0 ; i < stream_config_ptr->stream_number ; i++) {
            if ((stream_config_ptr->stream_ptr[i].streamingStatus != STREAMING_DISABLE) &&
                (stream_config_ptr->stream_ptr[i].source == stream_config_ptr->source) &&
                (stream_config_ptr->stream_ptr[i].sink == stream_config_ptr->sink)) {
                /*
                DSP_CALLBACK_STREAM_CLEAN clean_stru;
                clean_stru.is_clean = FALSE;
                clean_stru.stream_ptr = &(stream_config_ptr->stream_ptr[i]);
                stream_config_ptr->stream_ptr[i].streamingStatus = STREAMING_END;


                do
                {
                    OS_CRITICAL(DSP_Callback_StreamClean, &clean_stru);
                    if (!clean_stru.is_clean)
                    {
                        osTaskTaskingRequest();
                        DSP_LOG_WarningPrint(DSP_WARNING_STREAMING_DISABLE,
                                             2,
                                             stream_config_ptr->source->type,
                                             stream_config_ptr->sink->type);
                    }
                } while (!clean_stru.is_clean);
                */
                DSP_MW_LOG_I("DSP - Close Callback Stream, Source:%d, Sink:%d", 2, stream_config_ptr->source->type, stream_config_ptr->sink->type);
#ifdef AIR_AUDIO_HARDWARE_ENABLE
                DSP_DRV_SRC_END(stream_config_ptr->stream_ptr[i].callback.Src.src_ptr);
#endif /* AIR_AUDIO_HARDWARE_ENABLE */
                dsp_sdk_set_scenario_status(stream_config_ptr->source->scenario_type, false);
                DSPMEM_Free(stream_config_ptr->task, (DSP_STREAMING_PARA_PTR)&stream_config_ptr->stream_ptr[i]);
                DSP_Callback_StreamingInit((DSP_STREAMING_PARA_PTR)&stream_config_ptr->stream_ptr[i],
                                           1,
                                           stream_config_ptr->stream_ptr[i].callback.EntryPara.DSPTask);
#ifdef AIR_AUDIO_DOWNLINK_SW_GAIN_ENABLE
                #include "dsp_audio_process.h"
                extern sw_gain_port_t *g_DL_SW_gain_port, *g_DL_SW_gain_port_2, *g_DL_SW_gain_port_3;
                if(stream_config_ptr->sink->type == SINK_TYPE_AUDIO){
                    if (g_DL_SW_gain_port != NULL) {
                        stream_function_sw_gain_deinit(g_DL_SW_gain_port);
                        g_DL_SW_gain_port = NULL;
                        DSP_MW_LOG_I("[DL_SW_GAIN] Port is deinit, for type %d", 1, stream_config_ptr->sink->type);
                    }
                } else if (stream_config_ptr->sink->type == SINK_TYPE_VP_AUDIO){
                    if (g_DL_SW_gain_port_2 != NULL) {
                        stream_function_sw_gain_deinit(g_DL_SW_gain_port_2);
                        g_DL_SW_gain_port_2 = NULL;
                        DSP_MW_LOG_I("[DL_SW_GAIN] Port2 is deinit, for type %d", 1, stream_config_ptr->sink->type);
                    }
                } else if (stream_config_ptr->sink->type == SINK_TYPE_AUDIO_DL3){
                    if (g_DL_SW_gain_port_3 != NULL) {
                        stream_function_sw_gain_deinit(g_DL_SW_gain_port_3);
                        g_DL_SW_gain_port_3 = NULL;
                        DSP_MW_LOG_I("[DL_SW_GAIN] Port3 is deinit, for type %d", 1, stream_config_ptr->sink->type);
                    }
                }
#endif
                return stream_config_ptr->task;
            }
        }
    }
    return NULL_TASK_ID;
}
VOID DSP_Callback_StreamingRateConfig(SOURCE source, SINK sink)// Sink rate follows only now
{
#ifdef AIR_AUDIO_HARDWARE_ENABLE
    //U8 Output_Rate = PeripheralOutputSamplingRate_Get(sink);
    U8 Output_Rate = Audio_setting->Rate.Sink_Output_Sampling_Rate;


    switch (source->type) {
        case SOURCE_TYPE_USBAUDIOCLASS :
            if (Output_Rate > source->param.USB.sampling_rate) { //Cant be 44.1 Hz
                SinkConfigure(sink, AUDIO_SINK_UPSAMPLE_RATE, DSP_UpValue2Rate(Output_Rate / source->param.USB.sampling_rate));
            } else {
                SinkConfigure(sink, AUDIO_SINK_UPSAMPLE_RATE, UPSAMPLE_BY1);
            }
            if (Audio_setting->Audio_sink.Frame_Size != (source->param.USB.frame_size * 2 / 3)) {
                SinkConfigure(sink, AUDIO_SINK_FRAME_SIZE, (source->param.USB.frame_size * 2 / 3));
            }
            SinkConfigure(sink, AUDIO_SINK_FORCE_START, 0);
            break;
        default:
            break;
    }
    switch (sink->type) {
        //U8 Input_Rate = PeripheralInputSamplingRate_Get(source);
        /*
        U8 Input_Rate = Audio_setting->Rate.Source_Input_Sampling_Rate;
        case SINK_TYPE_USBAUDIOCLASS :
            if (Input_Rate > source->param.USB.sampling_rate)//Cant be 44.1 Hz
            {
                SourceConfigure(source,AUDIO_SOURCE_DOWNSAMP_RATE,DSP_DownValue2Rate(sink->param.USB.sampling_rate/Input_Rate));
            }
            else
            {
                SourceConfigure(source,AUDIO_SOURCE_DOWNSAMP_RATE,DOWNSAMPLE_BY1);
            }
            if (Audio_setting->Audio_source.Frame_Size != (source->param.USB.frame_size*2/3))
            {
                SourceConfigure(source,AUDIO_SOURCE_FRAME_SIZE,(source->param.USB.frame_size*2/3));
            }

        break;
            */
        default:
            break;
    }
#else
    UNUSED(source);
    UNUSED(sink);
#endif /* AIR_AUDIO_HARDWARE_ENABLE */
}

/**
 * DSP_Callback_FeatureConfig
 *
 * Get memory and copy feature function entry
 *
 * @Author :  BrianChen <BrianChen@airoha.com.tw>
 */
VOID DSP_Callback_FeatureConfig(DSP_STREAMING_PARA_PTR stream, stream_feature_list_ptr_t feature_list_ptr)
{
    U32  featureEntrySize;
    stream_feature_type_ptr_t  featureTypePtr;
    VOID*  mem_ptr;
    U32 i, featureEntryNum = 0;
    U32 codecOutSize, codecOutResolution;

    if ((featureTypePtr = stream->callback.EntryPara.feature_ptr = feature_list_ptr)!= NULL) {
        for(featureEntryNum = 1 ; *(featureTypePtr) != FUNC_END ; ++featureTypePtr) {
            if ((*(featureTypePtr) & 0xFF) == DSP_SRC) {
                DSP_Callback_SRC_Config(stream, featureTypePtr, featureEntryNum);
            }
            featureEntryNum++;
        }
    }
    featureEntrySize = featureEntryNum * sizeof(DSP_FEATURE_TABLE);
    stream->callback.EntryPara.number.field.feature_number = featureEntryNum;

#ifdef AIR_DSP_MEMORY_REGION_ENABLE
    stream->callback.EntryPara.scenario_type = stream->source->scenario_type;
    GroupMemoryInfo_t *fw_memory_addr;
    SubComponentType_t subcomponent_type_of_fw;
    DspMemoryManagerReturnStatus_t memory_return_status;

    subcomponent_type_of_fw = dsp_memory_get_fw_subcomponent_by_scenario(stream->callback.EntryPara.scenario_type);
    fw_memory_addr = dsp_memory_get_fw_memory_info_by_scenario(stream->callback.EntryPara.scenario_type);
    memory_return_status = DspMemoryManager_AcquireGroupMemory(dsp_memory_get_component_by_scenario(stream->callback.EntryPara.scenario_type), subcomponent_type_of_fw, fw_memory_addr);
    if (memory_return_status != DSP_MEMORY_MANAGEMENT_PROCESS_OK) {
        DSP_MW_LOG_E("DSP callback FeatureConfig FW malloc FAIL (%d) !!!", 1, memory_return_status);

        assert(0);
    }
    mem_ptr = fw_memory_addr->DramAddr;
#else
    DSP_MW_LOG_I("[DSP_RESOURCE][DSP_Callback_FeatureConfig] scenario: %d featureEntrySize:%d", 2
        , stream && stream->source ? stream->source->scenario_type : AUDIO_SCEANRIO_TYPE_MAX
        , featureEntrySize
    );
    DSP_FEATURE_TABLE_PTR featurePtr;
#ifdef AIR_PROMPT_SOUND_MEMORY_DEDICATE
    if (stream->source->type == SOURCE_TYPE_CM4_VP_PLAYBACK) {
        mem_ptr = g_vp_memptr + 4096;
    } else {
        mem_ptr = DSPMEM_tmalloc(stream->callback.EntryPara.DSPTask, featureEntrySize, stream);
    }
#else
    mem_ptr = DSPMEM_tmalloc(stream->callback.EntryPara.DSPTask, featureEntrySize, stream);
#endif
#endif

    if (featureEntrySize > 0) {
        stream->callback.FeatureTablePtr  = mem_ptr;
        featureTypePtr = feature_list_ptr;
        for(i = 0 ; i < featureEntryNum ; i++) {
#ifndef AIR_DSP_MEMORY_REGION_ENABLE
            featurePtr = (DSP_FEATURE_TABLE_PTR)&stream_feature_table[((U32)(*(featureTypePtr)&0xFF))];
            *(stream->callback.FeatureTablePtr + i) = *featurePtr;
            if (i == 0 || i == featureEntryNum - 2) {
                if ((*(featureTypePtr) & 0xFFFF0000) != 0) {
                    codecOutSize = (*(featureTypePtr)) >> 16;
                    ((stream->callback.FeatureTablePtr + i)->MemPtr) = (VOID *)codecOutSize;
                } else {
                    codecOutSize = (U32)((stream->callback.FeatureTablePtr + i)->MemPtr);
                }
            }
#else
            //Store codec output size at FUNC_END
            if (i==featureEntryNum-1) {
                if ((*(featureTypePtr)&0xFFFF0000) != 0)
                {
                    codecOutSize = (*(featureTypePtr))>>16;
                    ((stream->callback.FeatureTablePtr + i)->MemPtr) = (VOID*)codecOutSize;
                }
            }
#endif
            if (i==0)
            {
                codecOutResolution = (*(featureTypePtr)&0xFF00)>>8;
                stream->callback.EntryPara.resolution.feature_res = ((codecOutResolution==RESOLUTION_16BIT) || (codecOutResolution==RESOLUTION_32BIT))
                                                                    ? codecOutResolution
                                                                    : RESOLUTION_16BIT;
            }

            featureTypePtr++;
        }

        DSP_Callback_ParaSetup(stream);

        if (stream->callback.EntryPara.with_src) {
            (stream->callback.FeatureTablePtr + (U32)(stream->callback.EntryPara.with_src - 1))->MemSize +=
                2 * (stream->callback.EntryPara.out_malloc_size * (DSP_CALLBACK_SRC_BUF_FRAME + DSP_CALLBACK_SRC_IN_FRAME) +
                     stream->callback.Src.out_frame_size * (DSP_CALLBACK_SRC_OUT_FRAME));
            if(stream->sink->param.audio.hwsrc_type == HAL_AUDIO_HWSRC_IN_STREAM){
                (stream->callback.FeatureTablePtr + (U32)(stream->callback.EntryPara.with_src - 1))->MemSize += 64;//modify for asrc, for src_ptr+16, inSRC_mem_ptr+16, outSRC_mem_ptr+16, buf_mem_ptr+16;
            }

        }


        stream->callback.Status = CALLBACK_MALLOC;
        vTaskResume(stream->callback.EntryPara.DSPTask);
        // while (stream->callback.Status!=CALLBACK_SUSPEND)
        // portYIELD();
    } else {
        stream->callback.FeatureTablePtr  = NULL;
        DSP_MW_LOG_I("DSP - Warning:Feature Ptr Null. Source:%d, Sink:%d", 2, stream->source->type, stream->sink->type);
    }
}

#ifdef AIR_DSP_MEMORY_REGION_ENABLE
void dsp_callback_free_stream_memory(audio_scenario_type_t scenario_type)
{
    TotalComponentType_t component_type;
    SubComponentType_t sub_component_type;
    GroupMemoryInfo_t *memory_info_ptr;

    component_type = dsp_memory_get_component_by_scenario(scenario_type);

    //Free FW memory
    sub_component_type = dsp_memory_get_fw_subcomponent_by_scenario(scenario_type);
    memory_info_ptr = dsp_memory_get_fw_memory_info_by_scenario(scenario_type);
    if ((sub_component_type != DSP_MEMORY_NO_USE) && (memory_info_ptr->DramAddr != NULL)) {
        DspMemoryManager_ReleaseGroupMemory(component_type, sub_component_type, memory_info_ptr);
        memory_info_ptr->DramAddr = NULL;
            }

    //Free stream in  memory
    sub_component_type = dsp_memory_get_stream_in_subcomponent_by_scenario(scenario_type);
    memory_info_ptr = dsp_memory_get_stream_in_memory_info_by_scenario(scenario_type);
    if ((sub_component_type != DSP_MEMORY_NO_USE) && (memory_info_ptr->DramAddr != NULL)) {
        DspMemoryManager_ReleaseGroupMemory(component_type, sub_component_type, memory_info_ptr);
        memory_info_ptr->DramAddr = NULL;
        }

    //Free stream out memory
    sub_component_type = dsp_memory_get_stream_out_subcomponent_by_scenario(scenario_type);
    memory_info_ptr = dsp_memory_get_stream_out_memory_info_by_scenario(scenario_type);
    if ((sub_component_type != DSP_MEMORY_NO_USE) && (memory_info_ptr->DramAddr != NULL)) {
        DspMemoryManager_ReleaseGroupMemory(component_type, sub_component_type, memory_info_ptr);
        memory_info_ptr->DramAddr = NULL;
    }

}
#endif

VOID DSP_Callback_PreloaderConfig(stream_feature_list_ptr_t feature_list_ptr, audio_scenario_type_t scenario_type)

{
    UNUSED(scenario_type);
    stream_feature_ctrl_load_entry featureOpenPtr;
    uint32_t pic_dram_usage;
    void *code_addr = NULL;
    void *data_addr = NULL;

    DSP_MW_LOG_I("DSP DSP_Callback_PreloaderConfig ScenarioType:%d, feature list ptr:0x%x", 2, scenario_type, feature_list_ptr);
    if (feature_list_ptr != NULL) {
#ifdef AIR_DSP_MEMORY_REGION_ENABLE
        //Allocate FW DRAM
        DSP_FEATURE_TABLE_PTR feature_table_ptr;
        GroupMemoryInfo_t *fw_memory_addr;
        dsp_memory_feature_subcomponent_info_t subcomponent_type_of_feature;
        DspMemoryManagerReturnStatus_t memory_return_status;
        TotalComponentType_t component_type;
        SubComponentType_t subcomponent_type_of_fw;

        component_type = dsp_memory_get_component_by_scenario(scenario_type);
        dsp_memory_set_status_by_scenario(scenario_type, DSP_MEMORY_STATUS_OCCUPIED);
        subcomponent_type_of_fw = dsp_memory_get_fw_subcomponent_by_scenario(scenario_type);
        fw_memory_addr = dsp_memory_get_fw_memory_info_by_scenario(scenario_type);
        memory_return_status = DspMemoryManager_AcquireGroupMemory(component_type, subcomponent_type_of_fw, fw_memory_addr);
        if (memory_return_status != DSP_MEMORY_MANAGEMENT_PROCESS_OK) {
            DSP_MW_LOG_E("DSP callback PreloaderConfig FW malloc FAIL (%d) com:%d, SubComponentType_t:%d !!!", 3, memory_return_status, component_type, subcomponent_type_of_fw);
            assert(0);
        }
        feature_table_ptr = fw_memory_addr->DramAddr;
#endif
        //Allocate Feature RAM

#ifndef AIR_DSP_MEMORY_REGION_ENABLE
#if defined (AIR_WIRED_AUDIO_ENABLE)
        uint32_t curr_feature_cnt, max_feature_cnt;

        /* Load the CLKSKEW/DRC library firstly to avoid the memory fragement */
        if ((scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_0) ||
            (scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_1) ||
            (scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_OUT)) {
            for (max_feature_cnt = 0; (*(feature_list_ptr)&0xFF) != FUNC_END; feature_list_ptr++) {
                if (((*(feature_list_ptr)&0xFF) == FUNC_INS) ||
                    ((*(feature_list_ptr)&0xFF) == FUNC_USB_INS) ||
                    ((*(feature_list_ptr)&0xFF) == FUNC_SW_CLK_SKEW)) {
#ifdef PRELOADER_ENABLE
                    if ((featureOpenPtr = DSP_FeatureControl[((U32)(*(feature_list_ptr)&0xFF))].open_entry) != NULL) {
                        featureOpenPtr(code_addr, data_addr, &pic_dram_usage);
                    }
#endif
                }
                max_feature_cnt++;
            }
            /* Reset the feature list ptr */
            for (curr_feature_cnt = 0; curr_feature_cnt < max_feature_cnt; curr_feature_cnt++) {
                feature_list_ptr--;
            }
            curr_feature_cnt = 0;
        }
#endif
#endif

#ifdef AIR_DSP_MEMORY_REGION_ENABLE
        for(;  ; feature_list_ptr++, feature_table_ptr++) {
#else
        for(;  ; feature_list_ptr++) {
#endif
            DSP_MW_LOG_I("[DSP_RESOURCE][DSP_Callback_PreloaderConfig] scenario: %d Feature ID load: 0x%X", 2
                , scenario_type
                , (*(feature_list_ptr)&0xFF)
            );

#ifdef AIR_DSP_MEMORY_REGION_ENABLE
            DSP_FEATURE_TABLE_PTR featurePtr;
            featurePtr = (DSP_FEATURE_TABLE_PTR)&stream_feature_table[((U32)(*(feature_list_ptr)&0xFF))];
            *feature_table_ptr = *featurePtr;

            if ((*(feature_list_ptr)&0xFF)==FUNC_END) {
                break;
            }
            dsp_memory_feature_subcomponent_info_t separated_instance_dram;
            subcomponent_type_of_feature = dsp_memory_get_memory_id_by_feature_type(scenario_type, (*(feature_list_ptr)&0xFF), &separated_instance_dram);
            if(subcomponent_type_of_feature.subcomponent_type == DSP_MEMORY_NO_USE) {
                continue;
            }
            memset(subcomponent_type_of_feature.subcomponent_memory_info_ptr, 0, sizeof(GroupMemoryInfo_t));
            memory_return_status = DspMemoryManager_AcquireGroupMemory(component_type, subcomponent_type_of_feature.subcomponent_type, subcomponent_type_of_feature.subcomponent_memory_info_ptr);
            if ((memory_return_status != DSP_MEMORY_MANAGEMENT_PROCESS_OK) && (memory_return_status != DSP_MEMORY_MANAGEMENT_PROCESS_NO_USE)) {
                DSP_MW_LOG_E("DSP callback PreloaderConfig feature:0x%x malloc FAIL (%d) !!!", 2, (*(feature_list_ptr)&0xFF), memory_return_status);
                assert(0);
            }
            pic_dram_usage = 0;
            DSP_MW_LOG_I("DSP AcquireGroupMemory for feature:0x%x, IRAM:0x%x, DRAM:0x%x, SYSRAM:0x%x", 4, (*(feature_list_ptr)&0xFF),
                                                                                                        subcomponent_type_of_feature.subcomponent_memory_info_ptr->IramAddr,
                                                                                                        subcomponent_type_of_feature.subcomponent_memory_info_ptr->DramAddr,
                                                                                                        subcomponent_type_of_feature.subcomponent_memory_info_ptr->SysramAddr);

            code_addr = (subcomponent_type_of_feature.subcomponent_memory_info_ptr->IramAddr)
                          ? subcomponent_type_of_feature.subcomponent_memory_info_ptr->IramAddr
                          : subcomponent_type_of_feature.subcomponent_memory_info_ptr->SysramAddr;
            data_addr = subcomponent_type_of_feature.subcomponent_memory_info_ptr->DramAddr;
#else
            if ((*(feature_list_ptr)&0xFF)==FUNC_END) {
                break;
            }
#endif

#ifndef AIR_DSP_MEMORY_REGION_ENABLE
#if defined (AIR_WIRED_AUDIO_ENABLE)
            if ((scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_0) ||
                (scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_1) ||
                (scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_OUT)) {
                if (((*(feature_list_ptr)&0xFF) == FUNC_INS) ||
                    ((*(feature_list_ptr)&0xFF) == FUNC_USB_INS) ||
                    ((*(feature_list_ptr)&0xFF) == FUNC_SW_CLK_SKEW)) {
                    continue;
                }
            }
#endif
#endif

#ifdef PRELOADER_ENABLE
            if ((featureOpenPtr = DSP_FeatureControl[((U32)(*(feature_list_ptr)&0xFF))].open_entry) != NULL) {
                featureOpenPtr(code_addr, data_addr, &pic_dram_usage);
            }
#endif
             //Update to feature
#ifdef AIR_DSP_MEMORY_REGION_ENABLE
            if (separated_instance_dram.subcomponent_type == DSP_MEMORY_NO_USE) {
                feature_table_ptr->MemPtr = (void *)NARROW_UP_TO_16_BYTES_ALIGN(((uint32_t)subcomponent_type_of_feature.subcomponent_memory_info_ptr->DramAddr) + pic_dram_usage);
                DSP_MW_LOG_I("DSP AcquireGroupMemory for feature working buffer:0x%x", 1, feature_table_ptr->MemPtr);
                //Check feature  memory
                if (feature_table_ptr->MemSize > (uint32_t)subcomponent_type_of_feature.subcomponent_memory_info_ptr->DramSizeInBytes - NARROW_UP_TO_16_BYTES_ALIGN(pic_dram_usage)) {
                    DSP_MW_LOG_E("DSP callback Feature(0x%x) memory insufficient. Allocated:%d, PIC usage:%d !!!, working buffer :%d", 4, *(feature_list_ptr), subcomponent_type_of_feature.subcomponent_memory_info_ptr->DramSizeInBytes, NARROW_UP_TO_8_BYTES_ALIGN(pic_dram_usage), feature_table_ptr->MemSize);
                    assert(0);
                }
            } else {
                pic_dram_usage = 0;
                memset(subcomponent_type_of_feature.subcomponent_memory_info_ptr, 0, sizeof(GroupMemoryInfo_t));
                memory_return_status = DspMemoryManager_AcquireGroupMemory(component_type, separated_instance_dram.subcomponent_type, separated_instance_dram.subcomponent_memory_info_ptr);
                if ((memory_return_status != DSP_MEMORY_MANAGEMENT_PROCESS_OK) && (memory_return_status != DSP_MEMORY_MANAGEMENT_PROCESS_NO_USE)) {
                    DSP_MW_LOG_E("DSP callback PreloaderConfig Separated instance DRAM feature:0x%x malloc FAIL (%d) !!!", 2, (*(feature_list_ptr)&0xFF), memory_return_status);
                    assert(0);
                }
                feature_table_ptr->MemPtr = (void *)NARROW_UP_TO_16_BYTES_ALIGN((uint32_t)(separated_instance_dram.subcomponent_memory_info_ptr->DramAddr));
                DSP_MW_LOG_I("DSP AcquireGroupMemory for separated feature:0x%x, IRAM:0x%x, DRAM:0x%x, SYSRAM:0x%x", 4, (*(feature_list_ptr)&0xFF),
                                                                                                                        separated_instance_dram.subcomponent_memory_info_ptr->IramAddr,
                                                                                                                        separated_instance_dram.subcomponent_memory_info_ptr->DramAddr,
                                                                                                                        separated_instance_dram.subcomponent_memory_info_ptr->SysramAddr);
            }
#endif
        }

        //Check FW memory
#ifdef AIR_DSP_MEMORY_REGION_ENABLE
        if ((uint32_t)feature_table_ptr > (uint32_t)fw_memory_addr->DramAddr + fw_memory_addr->DramSizeInBytes) {
            DSP_MW_LOG_E("DSP callback FW memory insufficient. Allocated:%d, Usage:%d !!!", 2, fw_memory_addr->DramSizeInBytes, (uint32_t)feature_table_ptr-(uint32_t)fw_memory_addr->DramAddr);
            assert(0);
        }
#endif

        DSP_MW_LOG_I("[DSP_RESOURCE][DSP_Callback_PreloaderConfig] scenario: %d Feature PIC load end", 1
            , scenario_type
        );
    }
}


/*
 * DSP_Callback_ResolutionConfig
 *
 * Configure Callback parametsers resolution
 *
 * @Author :  BrianChen <BrianChen@airoha.com.tw>
 */
VOID DSP_Callback_ResolutionConfig(DSP_STREAMING_PARA_PTR stream)
{
    stream->callback.EntryPara.resolution.source_in_res = RESOLUTION_16BIT;
    stream->callback.EntryPara.resolution.sink_out_res = RESOLUTION_16BIT;
    stream->callback.EntryPara.resolution.process_res = RESOLUTION_16BIT;

    /* Configure Callback in resolution */
#if defined(AIR_MULTI_MIC_STREAM_ENABLE) || defined(AIR_WIRED_AUDIO_ENABLE) || defined(AIR_ADVANCED_PASSTHROUGH_ENABLE)|| defined(AIR_HEARTHROUGH_MAIN_ENABLE) || defined(AIR_CUSTOMIZED_LLF_ENABLE)
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
    if (stream->source->type == SOURCE_TYPE_AUDIO || (stream->source->type >= SOURCE_TYPE_SUBAUDIO_MIN && stream->source->type <= SOURCE_TYPE_SUBAUDIO_MAX) || (stream->source->type == SOURCE_TYPE_LLF) || (stream->source->type == SOURCE_TYPE_TDMAUDIO)) {
#else
    if (stream->source->type == SOURCE_TYPE_AUDIO || (stream->source->type >= SOURCE_TYPE_SUBAUDIO_MIN && stream->source->type <= SOURCE_TYPE_SUBAUDIO_MAX) || (stream->source->type == SOURCE_TYPE_LLF)) {
#endif
#else
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
    if (stream->source->type == SOURCE_TYPE_AUDIO || stream->source->type == SOURCE_TYPE_AUDIO2 || (stream->source->type == SOURCE_TYPE_TDMAUDIO)) {
#else
    if (stream->source->type == SOURCE_TYPE_AUDIO || stream->source->type == SOURCE_TYPE_AUDIO2) {
#endif
#endif
        //stream->callback.EntryPara.resolution.source_in_res = Audio_setting->resolution.AudioInRes;
        stream->callback.EntryPara.resolution.source_in_res = (stream->source->param.audio.format <= HAL_AUDIO_PCM_FORMAT_U16_BE)
                                                              ? RESOLUTION_16BIT
                                                              : RESOLUTION_32BIT;
    }
#ifdef MTK_PROMPT_SOUND_DUMMY_SOURCE_ENABLE
    else if (stream->source->type == SOURCE_TYPE_CM4_VP_DUMMY_SOURCE_PLAYBACK) {
        audio_bits_per_sample_t bit_type = stream->source->param.cm4_playback.info.bit_type;
        if (AUDIO_BITS_PER_SAMPLING_24 == bit_type) {
            stream->callback.EntryPara.resolution.source_in_res = RESOLUTION_32BIT;
        } else {
            stream->callback.EntryPara.resolution.source_in_res = RESOLUTION_16BIT;
        }
    }
#endif
#ifdef AIR_MIXER_STREAM_ENABLE
    else if (stream->source->type == SOURCE_TYPE_MIXER) {
        stream->callback.EntryPara.resolution.source_in_res = RESOLUTION_32BIT;
        stream->callback.EntryPara.resolution.feature_res = stream->callback.EntryPara.resolution.source_in_res;
    }
#endif
#ifdef AIR_WIRED_AUDIO_ENABLE
    else if ((stream->source->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_0) ||
               (stream->source->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_1)) {
        wired_audio_resolution_config(stream);
    }
#endif

    /* Configure Callback out resolution */
#if 0
    if (stream->sink->type == SINK_TYPE_AUDIO) {
        stream->callback.EntryPara.resolution.sink_out_res = (stream->sink->param.audio.AfeBlkControl.u4asrcflag)
                                                             ? Audio_setting->resolution.SRCInRes
                                                             : Audio_setting->resolution.AudioOutRes;
    } else if (stream->sink->type == SINK_TYPE_VP_AUDIO) {
        //stream->callback.EntryPara.resolution.sink_out_res = Audio_setting->Audio_VP.Fade.Resolution;//Audio_setting->resolution.AudioOutRes;
        stream->callback.EntryPara.resolution.sink_out_res = (stream->sink->param.audio.AfeBlkControl.u4asrcflag)
                                                             ? Audio_setting->resolution.SRCInRes
                                                             : Audio_setting->resolution.AudioOutRes;
    }
#else
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
    if ((stream->sink->type == SINK_TYPE_AUDIO) || (stream->sink->type == SINK_TYPE_VP_AUDIO) || (stream->sink->type == SINK_TYPE_AUDIO_DL3) || (stream->sink->type == SINK_TYPE_AUDIO_DL12) || (stream->sink->type == SINK_TYPE_TDMAUDIO) || (stream->sink->type == SINK_TYPE_LLF) )
#else
    if ((stream->sink->type == SINK_TYPE_AUDIO) || (stream->sink->type == SINK_TYPE_VP_AUDIO) || (stream->sink->type == SINK_TYPE_AUDIO_DL3) || (stream->sink->type == SINK_TYPE_AUDIO_DL12) || (stream->sink->type == SINK_TYPE_LLF))
#endif
    {
        stream->callback.EntryPara.resolution.sink_out_res = (stream->sink->param.audio.format <= HAL_AUDIO_PCM_FORMAT_U16_BE)
                                                             ? RESOLUTION_16BIT
                                                             : RESOLUTION_32BIT;
    }
#endif
#if defined AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE || defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE ||defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE
    if ((stream->sink->scenario_type == AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_LINE_IN) ||
        (stream->sink->scenario_type == AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_MST_IN_0) ||
        (stream->sink->scenario_type == AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0)) {
            ull_audio_v2_dongle_resolution_config(stream);
        }
#endif

#ifdef AIR_WIRED_AUDIO_ENABLE
    if (stream->sink->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_OUT) {
        wired_audio_resolution_config(stream);
    }
#endif

    stream->callback.EntryPara.resolution.process_res = stream->callback.EntryPara.resolution.source_in_res;

#ifdef AIR_BT_CODEC_BLE_ENABLED
#ifdef AIR_WIRELESS_MIC_TX_ENABLE
    if ((stream->sink->type == SINK_TYPE_N9BLE)&&(stream->sink->param.n9ble.context_type == BLE_CONTENT_TYPE_WIRELESS_MIC)){
        stream->callback.EntryPara.resolution.sink_out_res = RESOLUTION_32BIT;
        stream->callback.EntryPara.resolution.process_res = RESOLUTION_32BIT;
        stream->callback.EntryPara.resolution.source_in_res = RESOLUTION_32BIT;
    }
#endif
#endif
#ifdef AIR_RECORD_ADVANCED_ENABLE
    if (stream->sink->scenario_type == AUDIO_SCENARIO_TYPE_ADVANCED_RECORD_N_MIC) {
        stream->callback.EntryPara.resolution.sink_out_res = stream->callback.EntryPara.resolution.feature_res;
    }
#endif
#ifdef AIR_WIRED_AUDIO_ENABLE
    if (stream->sink->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_OUT) {
        stream->callback.EntryPara.resolution.sink_out_res = stream->callback.EntryPara.resolution.feature_res;
    }
#endif

#if defined AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
    if (stream->sink->scenario_type == AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_I2S_IN) {
        // i2s slave need 32 bit in, but ble only support 16bit
        stream->callback.EntryPara.resolution.source_in_res = (stream->source->param.audio.format <= HAL_AUDIO_PCM_FORMAT_U16_BE)
                                                    ? RESOLUTION_16BIT
                                                    : RESOLUTION_32BIT;
        stream->callback.EntryPara.resolution.feature_res = RESOLUTION_16BIT;
        stream->callback.EntryPara.resolution.sink_out_res = RESOLUTION_16BIT;
        stream->callback.EntryPara.resolution.process_res = RESOLUTION_16BIT;
    }
#endif
    if (stream->sink->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_OUT_IEM) {
        stream->callback.EntryPara.resolution.source_in_res = RESOLUTION_32BIT;
        stream->callback.EntryPara.resolution.feature_res = RESOLUTION_32BIT;
        stream->callback.EntryPara.resolution.sink_out_res = RESOLUTION_32BIT;
        stream->callback.EntryPara.resolution.process_res = RESOLUTION_32BIT;
    }
    if (stream->sink->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_MAINSTREAM) {
        stream->callback.EntryPara.resolution.source_in_res = RESOLUTION_32BIT;
        stream->callback.EntryPara.resolution.feature_res = RESOLUTION_32BIT;
        stream->callback.EntryPara.resolution.sink_out_res = RESOLUTION_32BIT;
        stream->callback.EntryPara.resolution.process_res = RESOLUTION_32BIT;
    }
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
    if ((stream->sink->scenario_type >= AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0) &&
        (stream->sink->scenario_type <= AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2)) {
            bt_audio_dongle_resolution_config(stream);
        }
#endif /* BT Audio Dongle AFE IN */
#if AIR_BT_CODEC_BLE_ENABLED
#if AIR_BT_ULL_FB_ENABLE
    /*Add for ULL_BLE 24bit*/
    if ((stream->sink->type == SINK_TYPE_N9BLE) && (stream->sink->param.n9ble.context_type == BLE_CONTENT_TYPE_ULL_BLE) && (stream->sink->param.n9ble.sampling_rate == 48000)) {
        stream->callback.EntryPara.resolution.sink_out_res = RESOLUTION_32BIT;
        stream->callback.EntryPara.resolution.process_res = RESOLUTION_32BIT;
        stream->callback.EntryPara.resolution.source_in_res = RESOLUTION_32BIT;
        stream->callback.EntryPara.resolution.feature_res = RESOLUTION_32BIT;
    }
#endif
#if AIR_AUDIO_ULD_DECODE_ENABLE
    if ((stream->source->param.n9ble.context_type == BLE_CONTENT_TYPE_ULL_BLE) && (stream->source->param.n9ble.codec_type == BT_BLE_CODEC_ULD)) {
       stream->callback.EntryPara.resolution.sink_out_res = RESOLUTION_32BIT;
       stream->callback.EntryPara.resolution.process_res = RESOLUTION_32BIT;
       stream->callback.EntryPara.resolution.source_in_res = RESOLUTION_32BIT;
       stream->callback.EntryPara.resolution.feature_res = RESOLUTION_32BIT;
    }
#endif
#endif
}
#ifdef PRELOADER_ENABLE
/**
 * DSP_Callback_FeatureDeinit
 *
 * Deinit entry of stream
 *
 * @Author :  BrianChen <BrianChen@airoha.com.tw>
 */
VOID DSP_Callback_FeatureDeinit(DSP_STREAMING_PARA_PTR stream)
{
    stream_feature_type_ptr_t  featureTypePtr;
    stream_feature_ctrl_unload_entry featureClosePtr;

    if ((featureTypePtr = stream->callback.EntryPara.feature_ptr) != NULL) {
        for (; * (featureTypePtr) != FUNC_END ; ++featureTypePtr) {
            if ((featureClosePtr = DSP_FeatureControl[((U32)(*(featureTypePtr) & 0xFF))].close_entry) != NULL) {
                featureClosePtr();
            }
        }
    }
}

VOID DSP_PIC_FeatureDeinit(stream_feature_type_ptr_t featureTypePtr)
{
    stream_feature_ctrl_unload_entry featureClosePtr;

    if (featureTypePtr != NULL) {
        for (; *featureTypePtr != FUNC_END ; ++featureTypePtr) {
            if ((featureClosePtr = DSP_FeatureControl[((U32)(*(featureTypePtr) & 0xFF))].close_entry) != NULL) {
                featureClosePtr();
            }
        }
    }
}

VOID DSP_Callback_UnloaderConfig(stream_feature_list_ptr_t feature_list_ptr, audio_scenario_type_t scenario_type)
{
    stream_feature_ctrl_unload_entry featureClosePtr;
    uint32_t pic_user_counter = 0;

    UNUSED(scenario_type);
    DSP_MW_LOG_I("DSP callback UnloaderConfig feature list ptr 0x%x", 1,feature_list_ptr);

    if (feature_list_ptr != NULL) {
#ifdef AIR_DSP_MEMORY_REGION_ENABLE
        DspMemoryManagerReturnStatus_t memory_return_status = DSP_MEMORY_MANAGEMENT_PROCESS_NO_USE;
        TotalComponentType_t component_type;
        component_type = dsp_memory_get_component_by_scenario(scenario_type);
#endif
        for( ; (*(feature_list_ptr)&0xFF) != FUNC_END ; ++feature_list_ptr) {
#ifdef PRELOADER_ENABLE
            if ((featureClosePtr = DSP_FeatureControl[((U32)(*(feature_list_ptr)&0xFF))].close_entry) != NULL) {
                pic_user_counter = featureClosePtr();
            }
#endif
#ifdef AIR_DSP_MEMORY_REGION_ENABLE
            dsp_memory_feature_subcomponent_info_t sub_component_type, separated_instance_ram;
            sub_component_type = dsp_memory_get_memory_id_by_feature_type(scenario_type, *feature_list_ptr&0xFF, &separated_instance_ram);
            if(sub_component_type.subcomponent_type == DSP_MEMORY_NO_USE) {
                continue;
            }
            //Release
            if (pic_user_counter == 0) {
                memory_return_status = DspMemoryManager_ReleaseGroupMemory(component_type, sub_component_type.subcomponent_type, sub_component_type.subcomponent_memory_info_ptr);
            }
            if ((memory_return_status == DSP_MEMORY_MANAGEMENT_PROCESS_OK) || (memory_return_status == DSP_MEMORY_MANAGEMENT_PROCESS_NO_USE)) {
                if(separated_instance_ram.subcomponent_type != DSP_MEMORY_NO_USE) {
                    memory_return_status = DspMemoryManager_ReleaseGroupMemory(component_type, separated_instance_ram.subcomponent_type, separated_instance_ram.subcomponent_memory_info_ptr);
                }
            }
            if ((memory_return_status != DSP_MEMORY_MANAGEMENT_PROCESS_OK) && (memory_return_status != DSP_MEMORY_MANAGEMENT_PROCESS_NO_USE)) {
                DSP_MW_LOG_E("DSP callback UnloaderConfig feature:0x%x FAIL (%d) !!!", 2, (*feature_list_ptr&0xFF), memory_return_status);
                assert(0);
            }
#endif
        }
    }

#ifdef AIR_DSP_MEMORY_REGION_ENABLE
    dsp_callback_free_stream_memory(scenario_type);
    dsp_memory_set_status_by_scenario(scenario_type, DSP_MEMORY_STATUS_UNUSED);
#endif
}


#endif

/**
 * DSP_CallbackTask_Get
 *
 * @Author :  BrianChen <BrianChen@airoha.com.tw>
 */

TaskHandle_t  DSP_CallbackTask_Get(SOURCE source, SINK sink)
{
    return DSP_Callback_Config(source, sink, NULL, TRUE);
}

/**
 * DSP_Callback_Get
 *
 * Get DSP callback ptr
 *
 * @Author : Brian <BrianChen@airoha.com.tw>
 */
ATTR_TEXT_IN_IRAM_LEVEL_1 DSP_CALLBACK_PTR DSP_Callback_Get(SOURCE source, SINK sink)
{
    DSP_CALLBACK_PTR callback_ptr = NULL;
#if defined(AIR_ADVANCED_PASSTHROUGH_ENABLE) || defined(AIR_HEARTHROUGH_MAIN_ENABLE) || defined(AIR_CUSTOMIZED_LLF_ENABLE)
    if (callback_ptr == NULL)
    {
        callback_ptr = DLLT_Callback_Get(source, sink);
    }
#endif
    if (callback_ptr == NULL) {
        callback_ptr =dsp_stream_get_callback(source,sink);
    }
    return callback_ptr;
}

/**
 * DSP_Callback_BypassModeCtrl
 *
 * Get DSP callback ptr
 *
 * @Author : Brian <BrianChen@airoha.com.tw>
 */
ATTR_TEXT_IN_IRAM_LEVEL_1 VOID DSP_Callback_BypassModeCtrl(SOURCE source, SINK sink, DSP_CALLBACK_BYPASS_CODEC_MODE mode)
{
    DSP_CALLBACK_PTR callback_ptr;

    callback_ptr =dsp_stream_get_callback(source,sink);

    if (callback_ptr != NULL) {
        callback_ptr->EntryPara.bypass_mode = mode;
    }
}

/**
 * DSP_Callback_BypassModeCtrl
 *
 * Get DSP callback ptr
 *
 * @Author : Brian <BrianChen@airoha.com.tw>
 */
ATTR_TEXT_IN_IRAM_LEVEL_1 DSP_CALLBACK_BYPASS_CODEC_MODE DSP_Callback_BypassModeGet(SOURCE source, SINK sink)
{
    DSP_CALLBACK_BYPASS_CODEC_MODE reportmode = 0;
    DSP_CALLBACK_PTR callback_ptr;

    callback_ptr =dsp_stream_get_callback(source,sink);

    if (callback_ptr != NULL) {
        reportmode = callback_ptr->EntryPara.bypass_mode;
    }
    return reportmode;
}


/**
 * DSP_Callback_ChangeStreaming2Deinit
 *
 * Set streaming to de-initial status
 *
 * @Author : Brian <BrianChen@airoha.com.tw>
 */
VOID DSP_Callback_ChangeStreaming2Deinit(VOID *para)
{
    DSP_STREAMING_PARA_PTR stream_ptr;
    stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    //stream_ptr->callback.Status = CALLBACK_INIT;
    if (stream_ptr->streamingStatus == STREAMING_ACTIVE) {
        stream_ptr->streamingStatus = STREAMING_DEINIT;
    }

}

/**
 * DSP_ChangeStreaming2Deinit
 *
 * Set streaming to de-initial status
 *
 * @Author : Machi <MachiWu@airoha.com.tw>
 */
VOID DSP_ChangeStreaming2Deinit(TRANSFORM transform)
{
    DSP_STREAMING_PARA_PTR stream_ptr;
    stream_ptr = DSP_Streaming_Get(transform->source, transform->sink);
    if (stream_ptr != NULL) {
        if (stream_ptr->streamingStatus == STREAMING_ACTIVE) {
            stream_ptr->streamingStatus = STREAMING_DEINIT;
        }
    }

}


/**
 * DSP_Streaming_Get
 *
 * Get DSP Streaming ptr
 *
 * @Author : Brian <BrianChen@airoha.com.tw>
 */
ATTR_TEXT_IN_IRAM_LEVEL_1 DSP_STREAMING_PARA_PTR DSP_Streaming_Get(SOURCE source, SINK sink)
{
    DSP_STREAMING_PARA_PTR streaming_ptr = NULL;
    DSP_CALLBACK_PTR callback_ptr;
    callback_ptr = DSP_Callback_Get(source, sink);

    if (callback_ptr != NULL) {
        streaming_ptr = DSP_CONTAINER_OF(callback_ptr, DSP_STREAMING_PARA, callback);
    }

    return streaming_ptr;
}


/**
 * DSP_FuncMemPtr_Get
 *
 * Get DSP function memory ptr
 *
 * @Author : Brian <BrianChen@airoha.com.tw>
 */
VOID *DSP_FuncMemPtr_Get(DSP_CALLBACK_PTR callback, stream_feature_function_entry_t entry, U32 feature_seq)
{
    VOID *funcMemPtr = NULL;
    DSP_FEATURE_TABLE_PTR featureTablePtr;

    if ((callback != NULL) && (entry != NULL)) {
        featureTablePtr = callback->FeatureTablePtr;
        if (feature_seq != DSP_UPDATE_COMMAND_FEATURE_PARA_SEQUENCE_AUTODETECT) {
            featureTablePtr += feature_seq;
        }
        if (featureTablePtr != NULL) {
            while (featureTablePtr->ProcessEntry != stream_function_end_process && featureTablePtr->ProcessEntry != NULL) {
                if (featureTablePtr->ProcessEntry == entry) {
                    funcMemPtr = featureTablePtr->MemPtr;
                    break;
                } else if (feature_seq != DSP_UPDATE_COMMAND_FEATURE_PARA_SEQUENCE_AUTODETECT) {
                    break;
                }
                featureTablePtr++;
            }
        }
    }
    return funcMemPtr;
}

/**
 * DSP_Callback_ParaSetup
 *
 * @Author :  BrianChen <BrianChen@airoha.com.tw>
 */
VOID DSP_Callback_ParaSetup(DSP_STREAMING_PARA_PTR stream)
{
    U8 i;
    SOURCE source = stream->source;
    SINK sink  = stream->sink;
    U8 *mem_ptr;
    U32 mallocSize, chNum, frameSize = 0;

    /* Configure Callback para  by Source type */
    if (source->type == SOURCE_TYPE_DSP_0_AUDIO_PATTERN) {
        stream->callback.EntryPara.in_size        = 20;
        stream->callback.EntryPara.in_channel_num = 1;

    } else if (source->type == SOURCE_TYPE_USBAUDIOCLASS) {
        stream->callback.EntryPara.in_size        = source->param.USB.frame_size;
        stream->callback.EntryPara.in_channel_num = 1;
    }
#ifdef AIR_BT_HFP_ENABLE
    else if (source->type == SOURCE_TYPE_N9SCO) {
        stream->callback.EntryPara.in_size        = 240;
        stream->callback.EntryPara.in_channel_num = 1;
        stream->callback.EntryPara.in_sampling_rate  = FS_RATE_16K;
#ifdef AIR_FIXED_RATIO_SRC
        Sco_DL_Fix_Sample_Rate_Init();
#endif
    }
#endif
#ifdef AIR_BT_CODEC_BLE_ENABLED
    else if (source->type == SOURCE_TYPE_N9BLE) {
        switch(source->param.n9ble.sampling_rate)
        {
            case 16000:
                stream->callback.EntryPara.in_size        = 160;
                stream->callback.EntryPara.in_sampling_rate = FS_RATE_16K;
            break;
            case 24000:
                stream->callback.EntryPara.in_size        = 240;
                stream->callback.EntryPara.in_sampling_rate = FS_RATE_24K;
            break;
            case 32000:
                stream->callback.EntryPara.in_size        = 320;
                stream->callback.EntryPara.in_sampling_rate = FS_RATE_32K;
            break;
            case 44100:
                stream->callback.EntryPara.in_size        = 441;
                stream->callback.EntryPara.in_sampling_rate = FS_RATE_44_1K;
            break;
            case 48000:
                stream->callback.EntryPara.in_size        = 960;
                stream->callback.EntryPara.in_sampling_rate = FS_RATE_48K;
            break;
            case 96000:
                stream->callback.EntryPara.in_size        = 960;
                stream->callback.EntryPara.in_sampling_rate = FS_RATE_96K;
            break;
            default:
                configASSERT(0);
            break;
        }
                stream->callback.EntryPara.in_channel_num = 1;
#if defined(AIR_CELT_DEC_V2_ENABLE) || defined(AIR_AUDIO_ULD_DECODE_ENABLE)
        if ((source->param.n9ble.codec_type == BT_BLE_CODEC_VENDOR || source->param.n9ble.codec_type == BT_BLE_CODEC_ULD) && source->param.n9ble.dual_cis_status != DUAL_CIS_DISABLED) {
            stream->callback.EntryPara.in_channel_num = 2;
        }
#endif
#if defined(AIR_BLE_FIXED_RATIO_SRC_ENABLE) && defined(AIR_FIXED_RATIO_SRC)
        N9Ble_DL_SWB_Sample_Rate_Init();
#endif
#if AIR_AUDIO_ULD_DECODE_ENABLE
        if ((stream->source->param.n9ble.context_type == BLE_CONTENT_TYPE_ULL_BLE) && (stream->source->param.n9ble.codec_type == BT_BLE_CODEC_ULD)) {
           stream->callback.EntryPara.resolution.sink_out_res = RESOLUTION_32BIT;
           stream->callback.EntryPara.resolution.process_res = RESOLUTION_32BIT;
           stream->callback.EntryPara.resolution.source_in_res = RESOLUTION_32BIT;
           stream->callback.EntryPara.resolution.feature_res = RESOLUTION_32BIT;
        }
#endif

    }
#endif
    else if ((source->type == SOURCE_TYPE_FILE) || (source->type == SOURCE_TYPE_USBCDCCLASS) || (source->type == SOURCE_TYPE_MEMORY)) {
        stream->callback.EntryPara.in_size        = 512;
        stream->callback.EntryPara.in_channel_num = 1;
        stream->callback.EntryPara.in_sampling_rate = FS_RATE_16K;
    } else if (source->type == SOURCE_TYPE_A2DP) {
#if defined(MTK_BT_A2DP_VENDOR_2_ENABLE)
        stream->callback.EntryPara.in_size          = 1314;
#else
        stream->callback.EntryPara.in_size          = 1024;
#endif
        stream->callback.EntryPara.in_channel_num   = 1;
        stream->callback.EntryPara.in_sampling_rate = source->streamBuffer.AVMBufferInfo.SampleRate / 1000;;
    } else if (source->type == SOURCE_TYPE_N9_A2DP) {
        stream->callback.EntryPara.in_size          = 1024;
        stream->callback.EntryPara.in_channel_num   = 1;
        stream->callback.EntryPara.in_sampling_rate = FS_RATE_48K;
    }
#ifdef MTK_PROMPT_SOUND_DUMMY_SOURCE_ENABLE
    else if ((source->type == SOURCE_TYPE_CM4_PLAYBACK) || (source->type == SOURCE_TYPE_CM4_VP_PLAYBACK) || (source->type == SOURCE_TYPE_CM4_VP_DUMMY_SOURCE_PLAYBACK))
#else
    else if ((source->type == SOURCE_TYPE_CM4_PLAYBACK) || (source->type == SOURCE_TYPE_CM4_VP_PLAYBACK))
#endif
    {
        stream->callback.EntryPara.in_size          = 2048;
        stream->callback.EntryPara.in_channel_num   = source->param.cm4_playback.info.source_channels;
        stream->callback.EntryPara.in_sampling_rate = source->param.cm4_playback.info.sampling_rate / 1000;
    }
#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
    else if ((source->type >= SOURCE_TYPE_AUDIO_TRANSMITTER_MIN) && (source->type <= SOURCE_TYPE_AUDIO_TRANSMITTER_MAX)) {
        if (source->param.data_dl.scenario_type == AUDIO_TRANSMITTER_A2DP_SOURCE) {

        }
#if defined(MTK_GAMING_MODE_HEADSET) || defined(AIR_GAMING_MODE_DONGLE_ENABLE)
        else if (source->param.data_dl.scenario_type == AUDIO_TRANSMITTER_GAMING_MODE) {
            if ((source->param.data_dl.scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_0) ||
                (source->param.data_dl.scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_1)) {
                // TODO: usb in
                stream->callback.EntryPara.in_size        = 6 * 96 * 4 * 2; /* Max 3ms-period/96K/24bit/2ch */
                stream->callback.EntryPara.in_channel_num = 2;
                stream->callback.EntryPara.in_sampling_rate = FS_RATE_48K;
            }
        }
#endif /* MTK_GAMING_MODE_HEADSET || AIR_GAMING_MODE_DONGLE_ENABLE */
#if defined(AIR_WIRED_AUDIO_ENABLE)
        else if (source->param.data_dl.scenario_type == AUDIO_TRANSMITTER_WIRED_AUDIO) {
            wired_audio_para_setup(source->param.data_dl.scenario_sub_id, stream, source, sink);
        }
#endif /* AIR_WIRED_AUDIO_ENABLE */
#if defined(AIR_BLE_AUDIO_DONGLE_ENABLE)
        else if (source->param.data_dl.scenario_type == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE) {
            if ((source->param.data_dl.scenario_sub_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0) ||
                (source->param.data_dl.scenario_sub_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_1)) {
                stream->callback.EntryPara.in_size          = 20 * 96 * 2 * 2;
                stream->callback.EntryPara.in_channel_num   = 2;
                stream->callback.EntryPara.in_sampling_rate = source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.codec_param.pcm.sample_rate / 1000;
            }
        }
#endif /* AIR_BLE_AUDIO_DONGLE_ENABLE */
#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
        else if (source->param.data_dl.scenario_type == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE)
        {
            if ((source->param.data_dl.scenario_sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0) ||
                (source->param.data_dl.scenario_sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1))
            {
                stream->callback.EntryPara.in_size          = ull_audio_v2_dongle_dl_get_stream_in_max_size_each_channel(source, sink);
                stream->callback.EntryPara.in_channel_num   = ull_audio_v2_dongle_dl_get_stream_in_channel_number(source, sink);
                stream->callback.EntryPara.in_sampling_rate = ull_audio_v2_dongle_dl_get_stream_in_sampling_rate_each_channel(source, sink);
            }
        }
#endif /* AIR_ULL_AUDIO_V2_DONGLE_ENABLE */
        else if (source->param.data_dl.scenario_type == AUDIO_TRANSMITTER_GSENSOR) {

        } else if (source->param.data_dl.scenario_type == AUDIO_TRANSMITTER_MULTI_MIC_STREAM) {

        } else {

        }
    }
#endif /* AIR_AUDIO_TRANSMITTER_ENABLE */
#ifdef AIR_AUDIO_BT_COMMON_ENABLE
    else if ((source->type >= SOURCE_TYPE_BT_COMMON_MIN) && (source->type <= SOURCE_TYPE_BT_COMMON_MAX)) {
        if (source->param.bt_common.scenario_type == AUDIO_TRANSMITTER_A2DP_SOURCE) {

        }
#if defined(MTK_GAMING_MODE_HEADSET) || defined(AIR_GAMING_MODE_DONGLE_ENABLE)
        else if (source->param.bt_common.scenario_type == AUDIO_TRANSMITTER_GAMING_MODE) {
            if (source->param.bt_common.scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_VOICE_DONGLE_USB_OUT) {
                // TODO: bt in
                stream->callback.EntryPara.in_size        = 50;
                stream->callback.EntryPara.in_channel_num = 1;
                stream->callback.EntryPara.in_sampling_rate = FS_RATE_16K;
            }
#ifdef AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE
            else if (source->scenario_type == AUDIO_SCENARIO_TYPE_GAMING_MODE_VOICE_DONGLE_LINE_OUT) {
                // TODO: bt in
                stream->callback.EntryPara.in_size        = 50;
                stream->callback.EntryPara.in_channel_num = 1;
                stream->callback.EntryPara.in_sampling_rate = FS_RATE_16K;
            }
#endif /* AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE */
        }

#endif /* MTK_GAMING_MODE_HEADSET || AIR_GAMING_MODE_DONGLE_ENABLE */
#if defined(AIR_BLE_AUDIO_DONGLE_ENABLE)
        else if (source->param.bt_common.scenario_type == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE) {
            if (source->param.bt_common.scenario_sub_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_VOICE_USB_OUT) {
                stream->callback.EntryPara.in_size          = 4 + (source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.codec_param.lc3.frame_size + 3) / 4 * 4; //payload size + frame status 4B
                stream->callback.EntryPara.in_channel_num   = 2;
                stream->callback.EntryPara.in_sampling_rate = source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.codec_param.lc3.sample_rate / 1000;
            }
        }
#endif /* AIR_BLE_AUDIO_DONGLE_ENABLE */
#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
        else if (source->param.bt_common.scenario_type == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE)
        {
            if ((source->param.bt_common.scenario_sub_id >= AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_USB_OUT_0) && (source->param.bt_common.scenario_sub_id <= AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_SLV_OUT_0))
            {
                stream->callback.EntryPara.in_size          = ull_audio_v2_dongle_ul_get_stream_in_max_size_each_channel(source, sink);
                stream->callback.EntryPara.in_channel_num   = ull_audio_v2_dongle_ul_get_stream_in_channel_number(source, sink);
                stream->callback.EntryPara.in_sampling_rate = ull_audio_v2_dongle_ul_get_stream_in_sampling_rate_each_channel(source, sink);
            }
        }
#endif /* AIR_ULL_AUDIO_V2_DONGLE_ENABLE */
#if defined(AIR_WIRELESS_MIC_RX_ENABLE)
        else if (source->param.bt_common.scenario_type == AUDIO_TRANSMITTER_WIRELESS_MIC_RX)
        {
            if ((source->param.bt_common.scenario_sub_id >= AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_USB_OUT_0) && (source->param.bt_common.scenario_sub_id <= AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_I2S_SLV_OUT_0))
            {
                stream->callback.EntryPara.in_size          = wireless_mic_rx_ul_get_stream_in_max_size_each_channel(source, sink);
                stream->callback.EntryPara.in_channel_num   = wireless_mic_rx_ul_get_stream_in_channel_number(source, sink);
                stream->callback.EntryPara.in_sampling_rate = wireless_mic_rx_ul_get_stream_in_sampling_rate_each_channel(source, sink);
            }
        }
#endif /* AIR_WIRELESS_MIC_RX_ENABLE */
        else {

        }
    }
#endif /* AIR_AUDIO_BT_COMMON_ENABLE */
#if defined(AIR_HEARTHROUGH_MAIN_ENABLE) || defined(AIR_CUSTOMIZED_LLF_ENABLE)
    else if (source->type == SOURCE_TYPE_LLF) {
        stream->callback.EntryPara.in_size        = source->param.audio.frame_size;
        stream->callback.EntryPara.in_channel_num = source->param.audio.channel_num;
        stream->callback.EntryPara.in_sampling_rate = 50; //50K
    }
#endif /* AIR_HEARING_AID_ENABLE */
#ifdef AIR_MIXER_STREAM_ENABLE
    else if (source->scenario_type == AUDIO_SCENARIO_TYPE_MIXER_STREAM) {
        stream->callback.EntryPara.in_size          = source->param.audio.frame_size * source->param.audio.format_bytes;
        stream->callback.EntryPara.in_channel_num   = 2 * MAX_MIXER_NUM;
        stream->callback.EntryPara.in_sampling_rate = source->param.audio.rate / 1000;
        DSP_MW_LOG_I("[Mixer Stream][dsp callback] in_size=%d, in_channel_num=%d, in_sampling_rate=%d", 3, stream->callback.EntryPara.in_size, stream->callback.EntryPara.in_channel_num, stream->callback.EntryPara.in_sampling_rate);
    }
#endif
#ifdef AIR_AUDIO_HARDWARE_ENABLE
    else if (source->param.audio.channel_num > 0) {
        stream->callback.EntryPara.in_size            = source->param.audio.frame_size * 2;
        stream->callback.EntryPara.in_channel_num     =
                                                        #ifdef AIR_ECHO_MEMIF_IN_ORDER_ENABLE
                                                        source->param.audio.channel_num;
                                                        #else
                                                        (source->param.audio.echo_reference)
                                                        ? source->param.audio.channel_num + 1
                                                        : source->param.audio.channel_num;
                                                        #endif
        #ifndef AIR_ECHO_MEMIF_IN_ORDER_ENABLE
        if (source->param.audio.mem_handle.memory_select == HAL_AUDIO_MEMORY_UL_AWB2) { // Echo path only
            stream->callback.EntryPara.in_channel_num = 2;
        }
        #endif
        #ifdef  AIR_HWSRC_RX_TRACKING_ENABLE
        if(source->param.audio.audio_device == HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE){
            stream->callback.EntryPara.in_sampling_rate  =  source->param.audio.src_rate/1000;
        } else if (source->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_DUAL_CHIP_LINE_OUT_MASTER) {
            stream->callback.EntryPara.in_sampling_rate  =  source->param.audio.rate/1000;
        } else {
            stream->callback.EntryPara.in_sampling_rate  =  source->param.audio.rate/1000;
        }
        #else
        stream->callback.EntryPara.in_sampling_rate   = (source->param.audio.src != NULL) //Source VDM SRC
                                                        ? DSP_SRCOutRateChange2Value(DSP_GetSRCOutRate(source->param.audio.src->src_ptr)) / 1000
                                                        : source->param.audio.rate / 1000; //AudioSourceSamplingRate_Get();
        #endif
    }
#endif /* AIR_AUDIO_HARDWARE_ENABLE */

#ifdef AIR_FULL_ADAPTIVE_ANC_ENABLE
    else if (source->type == SOURCE_TYPE_ADAPT_ANC) {
        stream->callback.EntryPara.in_size        = 64 * 4;
#if 0//def ANC_PATTERN_TEST
        //#error
        stream->callback.EntryPara.in_channel_num = 2; //Temp for Pattern test
#else
        stream->callback.EntryPara.in_channel_num = 4;
#endif
        stream->callback.EntryPara.in_sampling_rate = 50; //50K
    }
#endif
#ifdef AIR_HW_VIVID_PT_ENABLE
    else if (source->type == SOURCE_TYPE_HW_VIVID_PT) {
        stream->callback.EntryPara.in_size        = 64 * 4;
        stream->callback.EntryPara.in_channel_num = 4;
        stream->callback.EntryPara.in_sampling_rate = 50; //50K
    }
#endif

    if ((source->type >= SOURCE_TYPE_DSP_VIRTUAL_MIN)&&(source->type <= SOURCE_TYPE_DSP_VIRTUAL_MAX)) {
        if(source->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_OUT_IEM){
            stream->callback.EntryPara.in_channel_num = source->param.virtual_para.channel_num;
            stream->callback.EntryPara.in_size = source->param.virtual_para.mem_size;
            stream->callback.EntryPara.codec_out_size = source->param.virtual_para.mem_size;
            DSP_MW_LOG_I("[wired_audio] USB_IN_OUT_IEM SOURCE_TYPE_DSP_VIRTUAL in_channel_num %d,in_size%d",2,stream->callback.EntryPara.in_channel_num,stream->callback.EntryPara.in_size);
        } else if(source->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_MAINSTREAM){
            stream->callback.EntryPara.in_channel_num = source->param.virtual_para.channel_num;
            stream->callback.EntryPara.in_size = source->param.virtual_para.mem_size;
            stream->callback.EntryPara.codec_out_size = source->param.virtual_para.mem_size;
            DSP_MW_LOG_I("[wired_audio] MAIN_STREAM SOURCE_TYPE_DSP_VIRTUAL in_channel_num %d,in_size%d",2,stream->callback.EntryPara.in_channel_num,stream->callback.EntryPara.in_size);
        }
    }

/************************************************************************** ATTENTION ***************************************************************/
/**************************************************** Config callback para of input by sceanrio type ************************************************/
/************************************************************************** ATTENTION ***************************************************************/
    switch (source->scenario_type) {
        /* BT Source Dongle ------------------------------------------------------------------------------------------------------------------------*/
#ifdef AIR_BT_AUDIO_DONGLE_ENABLE
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_1:
            stream->callback.EntryPara.in_size          = bt_audio_dongle_ul_get_stream_in_max_size_each_channel(source, sink);
            stream->callback.EntryPara.in_channel_num   = bt_audio_dongle_ul_get_stream_in_channel_number(source, sink);
            stream->callback.EntryPara.in_sampling_rate = bt_audio_dongle_ul_get_stream_in_sampling_rate_each_channel(source, sink);
            break;
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1:
            stream->callback.EntryPara.in_size          = bt_audio_dongle_dl_get_stream_in_max_size_each_channel(source, sink);
            stream->callback.EntryPara.in_channel_num   = bt_audio_dongle_dl_get_stream_in_channel_number(source, sink);
            stream->callback.EntryPara.in_sampling_rate = bt_audio_dongle_dl_get_stream_in_sampling_rate_each_channel(source, sink);
            break;
#endif
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_2:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2:
            /* For uplink, use src_out rate, not src_in rate. */
            if (source->param.audio.audio_device == HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) {
                stream->callback.EntryPara.in_sampling_rate = source->param.audio.src_rate / 1000;
            }
            break;
#endif /* afe in*/
#endif

        /* ULL V2 Dongle ---------------------------------------------------------------------------------------------------------------------------*/
#if defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE || defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE
        case AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0:
        case AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_MST_IN_0:
            /* For uplink, use src_out rate, not src_in rate. */
            stream->callback.EntryPara.in_sampling_rate = source->param.audio.src_rate / 1000;
            break;
#endif
        default:
            break;
    }
/**************************************************************************** END *******************************************************************/
/* ------------------------------------------------------------------------------------------------------------------------------------------------ */


    /* Configure Callback para  by Sink type */
    if (sink->type == SINK_TYPE_N9SCO) {
#ifdef AIR_BT_HFP_ENABLE
        stream->callback.EntryPara.out_channel_num         = stream->callback.EntryPara.in_channel_num;
        stream->callback.EntryPara.codec_out_size          = 1000; //480; Clk skew temp
        stream->callback.EntryPara.codec_out_sampling_rate  = FS_RATE_16K;
#if defined(AIR_FIXED_RATIO_SRC)
        Sco_UL_Fix_Sample_Rate_Init();
#endif
#if defined(AIR_MUTE_MIC_DETECTION_ENABLE)
        Sco_UL_Volume_Estimator_Init(sink);
#endif
#endif
    }
#ifdef AIR_BT_CODEC_BLE_ENABLED
    else if (sink->type == SINK_TYPE_N9BLE) {
        stream->callback.EntryPara.out_channel_num         = stream->callback.EntryPara.in_channel_num;
        stream->callback.EntryPara.codec_out_size          = 1000; //480; Clk skew temp
        switch(sink->param.n9ble.sampling_rate)
        {
            case 16000:
                stream->callback.EntryPara.codec_out_sampling_rate  = FS_RATE_16K;
            break;
            case 24000:
                stream->callback.EntryPara.codec_out_sampling_rate  = FS_RATE_24K;
            break;
            case 32000:
                stream->callback.EntryPara.codec_out_sampling_rate  = FS_RATE_32K;
            break;
            case 44100:
                stream->callback.EntryPara.codec_out_sampling_rate  = FS_RATE_44_1K;
            break;
            case 48000:
                stream->callback.EntryPara.codec_out_sampling_rate  = FS_RATE_48K;
            break;
            case 96000:
                stream->callback.EntryPara.codec_out_sampling_rate  = FS_RATE_96K;
            break;
            default:
                configASSERT(0);
            break;
        }
#ifdef AIR_WIRELESS_MIC_TX_ENABLE
        if (sink->param.n9ble.codec_type == BT_BLE_CODEC_ULD) {
            stream->callback.EntryPara.out_channel_num = (stream->callback.EntryPara.in_channel_num>sink->param.n9ble.out_channel)?
                                                            stream->callback.EntryPara.in_channel_num : sink->param.n9ble.out_channel;
        } else if (sink->param.n9ble.codec_type == BT_BLE_CODEC_LC3PLUS) {
        stream->callback.EntryPara.out_channel_num = 2;
        } else {
            assert(0);
        }
#endif
        sink->param.n9ble.out_channel = stream->callback.EntryPara.out_channel_num;
#if defined(AIR_BLE_FIXED_RATIO_SRC_ENABLE) && defined(AIR_FIXED_RATIO_SRC)
        N9Ble_UL_SWB_Sample_Rate_Init();
#endif

        if ((sink->param.n9ble.context_type != BLE_CONTENT_TYPE_ULL_BLE) && (sink->param.n9ble.context_type != BLE_CONTENT_TYPE_WIRELESS_MIC)) {
#if defined(AIR_MUTE_MIC_DETECTION_ENABLE) && defined(AIR_LE_AUDIO_ENABLE)
            N9Ble_UL_Volume_Estimator_Init(sink);
#endif
#if defined(AIR_BLE_UL_SW_GAIN_CONTROL_ENABLE) && defined(AIR_SOFTWARE_GAIN_ENABLE)
            N9Ble_UL_SW_Gain_Init();
#endif
        }
#if AIR_BT_ULL_FB_ENABLE
        /*Add for ULL_BLE 24bit*/
        if (stream->sink->param.n9ble.context_type == BLE_CONTENT_TYPE_ULL_BLE) {
            if (stream->sink->param.n9ble.sampling_rate == 48000) {
                stream->callback.EntryPara.resolution.sink_out_res = RESOLUTION_32BIT;
                stream->callback.EntryPara.resolution.process_res = RESOLUTION_32BIT;
                stream->callback.EntryPara.resolution.source_in_res = RESOLUTION_32BIT;
                stream->callback.EntryPara.resolution.feature_res = RESOLUTION_32BIT;
            } else {
                stream->callback.EntryPara.resolution.sink_out_res = RESOLUTION_16BIT;
                stream->callback.EntryPara.resolution.process_res = RESOLUTION_16BIT;
                stream->callback.EntryPara.resolution.source_in_res = RESOLUTION_16BIT;
                stream->callback.EntryPara.resolution.feature_res = RESOLUTION_16BIT;
            }
        }
#endif
    }
#endif
    else if (sink->type == SINK_TYPE_DSP_VIRTUAL) {
        if ((source->type != SOURCE_TYPE_ADAPT_ANC) && (source->type != SOURCE_TYPE_HW_VIVID_PT)) {
            if ((source->scenario_type != AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_0) &&
                (source->scenario_type != AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_1)) {
                stream->callback.EntryPara.out_channel_num         = 1;
                stream->callback.EntryPara.codec_out_size          = sink->param.virtual_para.mem_size;
                stream->callback.EntryPara.codec_out_sampling_rate = FS_RATE_16K;
            }
        } else {
#if 0//def ANC_PATTERN_TEST
            //#error
            stream->callback.EntryPara.out_channel_num         = 2; //Temp for Pattern test
#else
            stream->callback.EntryPara.out_channel_num         = 4;
#endif
            stream->callback.EntryPara.codec_out_size          = 64 * 4;
            stream->callback.EntryPara.codec_out_sampling_rate = 50;
        }
    } else if (sink->type == SINK_TYPE_MEMORY) {
        stream->callback.EntryPara.out_channel_num     = 1;
        stream->callback.EntryPara.codec_out_size          = 512;
        stream->callback.EntryPara.codec_out_sampling_rate = FS_RATE_16K;
    } else if (sink->type == SINK_TYPE_CM4RECORD) {
        stream->callback.EntryPara.out_channel_num         = stream->callback.EntryPara.in_channel_num;
        stream->callback.EntryPara.codec_out_size          = MAX(stream->callback.EntryPara.in_size, 512);
        stream->callback.EntryPara.codec_out_sampling_rate = stream->callback.EntryPara.in_sampling_rate;
    }
#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
    else if ((sink->type >= SINK_TYPE_AUDIO_TRANSMITTER_MIN) && (sink->type <= SINK_TYPE_AUDIO_TRANSMITTER_MAX)) {
        if (sink->param.data_ul.scenario_type == AUDIO_TRANSMITTER_A2DP_SOURCE) {

        }
#if defined(MTK_GAMING_MODE_HEADSET) || defined(AIR_GAMING_MODE_DONGLE_ENABLE)
        else if (sink->param.data_ul.scenario_type == AUDIO_TRANSMITTER_GAMING_MODE) {
            if (sink->param.data_ul.scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_VOICE_DONGLE_USB_OUT) {
                // TODO: usb out
                stream->callback.EntryPara.out_channel_num          = 2;
                stream->callback.EntryPara.codec_out_size           = 720;
                /* set codec_out_sampling_rate by opus decoder for SRC */
                stream->callback.EntryPara.codec_out_sampling_rate  = FS_RATE_16K;
            } else if (sink->param.data_ul.scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_VOICE_HEADSET) {
                stream->callback.EntryPara.out_channel_num         = stream->callback.EntryPara.in_channel_num;
                stream->callback.EntryPara.codec_out_size          = 1000; //480; Clk skew temp
                stream->callback.EntryPara.codec_out_sampling_rate  = FS_RATE_16K;
            }
        }
#endif /* MTK_GAMING_MODE_HEADSET || AIR_GAMING_MODE_DONGLE_ENABLE */
#if defined(AIR_BLE_AUDIO_DONGLE_ENABLE)
        else if (sink->param.data_ul.scenario_type == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE) {
            if (sink->param.data_ul.scenario_sub_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_VOICE_USB_OUT) {
                stream->callback.EntryPara.out_channel_num          = 2;
                stream->callback.EntryPara.codec_out_size           = 2 * 20 * 48 * sizeof(int16_t);
                stream->callback.EntryPara.codec_out_sampling_rate  = sink->param.data_ul.scenario_param.usb_out_broadcast_param.usb_out_param.codec_param.pcm.sample_rate / 1000;
            }
        }
#endif /* AIR_BLE_AUDIO_DONGLE_ENABLE */
#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
        else if (sink->param.data_ul.scenario_type == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE)
        {
            if ((sink->scenario_type >= AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_UL_USB_OUT_0) && (sink->scenario_type <= AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_UL_I2S_SLV_OUT_0))
            {
                stream->callback.EntryPara.out_channel_num          = ull_audio_v2_dongle_ul_get_stream_out_channel_number(source, sink);
                stream->callback.EntryPara.codec_out_size           = ull_audio_v2_dongle_ul_get_stream_out_max_size_each_channel(source, sink);
                stream->callback.EntryPara.codec_out_sampling_rate  = ull_audio_v2_dongle_ul_get_stream_out_sampling_rate_each_channel(source, sink);
            }
        }
#endif /* AIR_ULL_AUDIO_V2_DONGLE_ENABLE */
#if defined(AIR_WIRELESS_MIC_RX_ENABLE)
        else if ((sink->scenario_type >= AUDIO_SCENARIO_TYPE_WIRELESS_MIC_RX_UL_USB_OUT_0) && (sink->scenario_type <= AUDIO_SCENARIO_TYPE_WIRELESS_MIC_RX_UL_I2S_SLV_OUT_0))
        {
            stream->callback.EntryPara.out_channel_num          = wireless_mic_rx_ul_get_stream_out_channel_number(source, sink);
            stream->callback.EntryPara.codec_out_size           = wireless_mic_rx_ul_get_stream_out_max_size_each_channel(source, sink);
            stream->callback.EntryPara.codec_out_sampling_rate  = wireless_mic_rx_ul_get_stream_out_sampling_rate_each_channel(source, sink);
        }
#endif /* AIR_WIRELESS_MIC_RX_ENABLE */
        else if (sink->param.data_ul.scenario_type == AUDIO_TRANSMITTER_GSENSOR) {

        } else if (sink->param.data_ul.scenario_type == AUDIO_TRANSMITTER_MULTI_MIC_STREAM) {

        }
#if defined(AIR_WIRED_AUDIO_ENABLE)
        else if (sink->param.data_ul.scenario_type == AUDIO_TRANSMITTER_WIRED_AUDIO) {
            wired_audio_para_setup(sink->param.data_ul.scenario_sub_id, stream, source, sink);
        }
#endif
#if defined(AIR_RECORD_ADVANCED_ENABLE)
        else if (sink->param.data_ul.scenario_type == AUDIO_TRANSMITTER_ADVANCED_RECORD) {
            if (sink->param.data_ul.scenario_sub_id == AUDIO_TRANSMITTER_ADVANCED_RECORD_N_MIC) {
                stream->callback.EntryPara.codec_out_size          = source->param.audio.frame_size;
                stream->callback.EntryPara.out_channel_num         = stream->callback.EntryPara.in_channel_num;
            }
        }
#endif
        else {

        }
    }
#endif /* AIR_AUDIO_TRANSMITTER_ENABLE */

#ifdef AIR_AUDIO_BT_COMMON_ENABLE
    else if ((sink->type >= SINK_TYPE_BT_COMMON_MIN) && (sink->type <= SINK_TYPE_BT_COMMON_MAX)) {
        if (sink->param.bt_common.scenario_type == AUDIO_TRANSMITTER_A2DP_SOURCE) {

        }
#if defined(MTK_GAMING_MODE_HEADSET) || defined(AIR_GAMING_MODE_DONGLE_ENABLE)
        else if (sink->param.bt_common.scenario_type == AUDIO_TRANSMITTER_GAMING_MODE) {
            if ((sink->param.bt_common.scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_0) ||
                (sink->param.bt_common.scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_1)) {
                // TODO: bt out
                stream->callback.EntryPara.out_channel_num          = 2;
                stream->callback.EntryPara.codec_out_size           = 6 * 48 * 2 * 2;
                /* set codec_out_sampling_rate to usb in sampling rate for SRC */
                stream->callback.EntryPara.codec_out_sampling_rate  = FS_RATE_48K;
            }
        }
        #ifdef AIR_GAMING_MODE_DONGLE_LINE_IN_ENABLE
            else if (sink->scenario_type == AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_LINE_IN) {
                stream->callback.EntryPara.out_channel_num          = 2;
                stream->callback.EntryPara.codec_out_size           = 6*48*2*2;
                stream->callback.EntryPara.codec_out_sampling_rate  = FS_RATE_48K;
            }
        #endif /* AIR_GAMING_MODE_DONGLE_LINE_IN_ENABLE */
        #ifdef AIR_GAMING_MODE_DONGLE_I2S_IN_ENABLE
            else if (sink->scenario_type == AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_I2S_IN) {
                stream->callback.EntryPara.out_channel_num          = 2;
                stream->callback.EntryPara.codec_out_size           = 6*48*2*2;
                stream->callback.EntryPara.codec_out_sampling_rate  = FS_RATE_48K;
            }
        #endif /* AIR_GAMING_MODE_DONGLE_I2S_IN_ENABLE */
#endif /* MTK_GAMING_MODE_HEADSET || AIR_GAMING_MODE_DONGLE_ENABLE */
#if defined(AIR_BLE_AUDIO_DONGLE_ENABLE)
        else if (sink->param.bt_common.scenario_type == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE) {
            if ((sink->param.bt_common.scenario_sub_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0) ||
                (sink->param.bt_common.scenario_sub_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_1)
                #ifdef AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE
                    || (sink->param.bt_common.scenario_sub_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_LINE_IN)
                #endif /* AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE */
                #ifdef AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
                    || (sink->param.bt_common.scenario_sub_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_I2S_IN)
                #endif /* AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE */
                ) {
                stream->callback.EntryPara.out_channel_num          = 2;
                stream->callback.EntryPara.codec_out_size           = sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.codec_param.lc3.frame_size;
                stream->callback.EntryPara.codec_out_sampling_rate  = sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.codec_param.lc3.sample_rate / 1000;
            }
        }
#endif /* AIR_BLE_AUDIO_DONGLE_ENABLE */
#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
        else if (sink->param.bt_common.scenario_type == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE)
        {
            if ((sink->param.bt_common.scenario_sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0) ||
                (sink->param.bt_common.scenario_sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1)
            #if defined AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE
                || (sink->param.bt_common.scenario_sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_LINE_IN)
            #endif /* AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE */
            #if defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE
                || (sink->param.bt_common.scenario_sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_I2S_MST_IN_0)
            #endif /* AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE */
            #if defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE
                || (sink->param.bt_common.scenario_sub_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0)
            #endif /* AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE */
                )
            {
                stream->callback.EntryPara.out_channel_num          = ull_audio_v2_dongle_dl_get_stream_out_channel_number(source, sink);
                stream->callback.EntryPara.codec_out_size           = ull_audio_v2_dongle_dl_get_stream_out_max_size_each_channel(source, sink);
                stream->callback.EntryPara.codec_out_sampling_rate  = ull_audio_v2_dongle_dl_get_stream_out_sampling_rate_each_channel(source, sink);
            }
        }
#endif /* AIR_ULL_AUDIO_V2_DONGLE_ENABLE */
        else {

        }
    }
#endif /* AIR_AUDIO_BT_COMMON_ENABLE */
#if defined(AIR_MIXER_STREAM_ENABLE)
    else if (source->type == SOURCE_TYPE_MIXER) {
        stream->callback.EntryPara.out_channel_num            = MIXER_STREAM_OUT_CHANNEL;
        stream->callback.EntryPara.codec_out_size             = source->param.audio.frame_size * source->param.audio.format_bytes;
        stream->callback.EntryPara.codec_out_sampling_rate    = source->param.audio.rate / 1000;
    }
#endif
#if defined(AIR_HEARTHROUGH_MAIN_ENABLE) || defined(AIR_CUSTOMIZED_LLF_ENABLE)
    else if (sink->type == SINK_TYPE_LLF) {
        stream->callback.EntryPara.out_channel_num         = sink->param.audio.channel_num;
        stream->callback.EntryPara.codec_out_size          = sink->param.audio.frame_size;//(U32)(stream->callback.FeatureTablePtr->MemPtr);
        stream->callback.EntryPara.codec_out_sampling_rate = 50;
        frameSize = sink->param.audio.frame_size;
    }
#endif
    else if ((stream->callback.EntryPara.codec_out_size = (U32)(stream_feature_table[stream->callback.FeatureTablePtr->FeatureType].codec_output_size)) != 0) {
        stream->callback.EntryPara.codec_out_sampling_rate    = (source->type == SOURCE_TYPE_AUDIO || source->type == SOURCE_TYPE_AUDIO2)
                                                                ? stream->callback.EntryPara.in_sampling_rate *
                                                                stream->callback.EntryPara.codec_out_size /
                                                                stream->callback.EntryPara.in_size
                                                                : stream->callback.EntryPara.in_sampling_rate;

        if ((sink->type == SINK_TYPE_AUDIO) || (sink->type == SINK_TYPE_VP_AUDIO) || (sink->type == SINK_TYPE_DSP_JOINT) || (sink->type == SINK_TYPE_AUDIO_DL3) || (sink->type == SINK_TYPE_AUDIO_DL12) || (sink->type == SINK_TYPE_LLF)) {
            stream->callback.EntryPara.out_channel_num = sink->param.audio.channel_num;
            frameSize = sink->param.audio.frame_size;
#if defined(AIR_WIRELESS_MIC_RX_ENABLE)
            if ((sink->scenario_type >= AUDIO_SCENARIO_TYPE_WIRELESS_MIC_RX_UL_USB_OUT_0) && (sink->scenario_type <= AUDIO_SCENARIO_TYPE_WIRELESS_MIC_RX_UL_I2S_SLV_OUT_0))
            {
                stream->callback.EntryPara.out_channel_num          = wireless_mic_rx_ul_get_stream_out_channel_number(source, sink);
                stream->callback.EntryPara.codec_out_size           = wireless_mic_rx_ul_get_stream_out_max_size_each_channel(source, sink);
                stream->callback.EntryPara.codec_out_sampling_rate  = wireless_mic_rx_ul_get_stream_out_sampling_rate_each_channel(source, sink);
            }
#endif /* AIR_WIRELESS_MIC_RX_ENABLE */
#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
            if ((sink->scenario_type >= AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_UL_USB_OUT_0) && (sink->scenario_type <= AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_UL_I2S_SLV_OUT_0))
            {
                stream->callback.EntryPara.out_channel_num          = ull_audio_v2_dongle_ul_get_stream_out_channel_number(source, sink);
                stream->callback.EntryPara.codec_out_size           = ull_audio_v2_dongle_ul_get_stream_out_max_size_each_channel(source, sink);
                stream->callback.EntryPara.codec_out_sampling_rate  = ull_audio_v2_dongle_ul_get_stream_out_sampling_rate_each_channel(source, sink);
            }
#endif /* defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE) */
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
        } else if (sink->type == SINK_TYPE_TDMAUDIO) {
            stream->callback.EntryPara.out_channel_num = sink->param.audio.channel_sel;
            frameSize = sink->param.audio.frame_size;
#endif
#if defined(AIR_HFP_DL_STREAM_RATE_FIX_TO_48KHZ) || defined(AIR_HFP_DL_STREAM_RATE_FIX_TO_96KHZ)
            if(source->type == SOURCE_TYPE_N9SCO) {
#if defined(AIR_HFP_DL_STREAM_RATE_FIX_TO_48KHZ)
                stream->callback.EntryPara.codec_out_size = (stream->callback.EntryPara.codec_out_size - DSP_SIZE_FOR_CLK_SKEW) * 3 + DSP_SIZE_FOR_CLK_SKEW;
#endif
#if defined(AIR_HFP_DL_STREAM_RATE_FIX_TO_96KHZ)
                stream->callback.EntryPara.codec_out_size = (stream->callback.EntryPara.codec_out_size - DSP_SIZE_FOR_CLK_SKEW) * 6 + DSP_SIZE_FOR_CLK_SKEW;
#endif
            }
#endif
        } else {
            stream->callback.EntryPara.out_channel_num = 2;

        }
    }
#ifdef AIR_AUDIO_HARDWARE_ENABLE
    else if (sink->param.audio.channel_num > 0) {

        stream->callback.EntryPara.out_channel_num            = sink->param.audio.channel_num;
        //setting by codec and application
        stream->callback.EntryPara.codec_out_size             = sink->param.audio.frame_size;//////
        stream->callback.EntryPara.codec_out_sampling_rate    = stream->callback.EntryPara.in_sampling_rate; /////
#if defined(AIR_WIRELESS_MIC_RX_ENABLE)
        if ((sink->scenario_type >= AUDIO_SCENARIO_TYPE_WIRELESS_MIC_RX_UL_USB_OUT_0) && (sink->scenario_type <= AUDIO_SCENARIO_TYPE_WIRELESS_MIC_RX_UL_I2S_SLV_OUT_0))
        {
            stream->callback.EntryPara.out_channel_num          = wireless_mic_rx_ul_get_stream_out_channel_number(source, sink);
            stream->callback.EntryPara.codec_out_size           = wireless_mic_rx_ul_get_stream_out_max_size_each_channel(source, sink);
            stream->callback.EntryPara.codec_out_sampling_rate  = wireless_mic_rx_ul_get_stream_out_sampling_rate_each_channel(source, sink);
        }
#endif /* AIR_WIRELESS_MIC_RX_ENABLE */
#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
        if ((sink->scenario_type >= AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_UL_USB_OUT_0) && (sink->scenario_type <= AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_UL_I2S_SLV_OUT_0))
        {
            stream->callback.EntryPara.out_channel_num          = ull_audio_v2_dongle_ul_get_stream_out_channel_number(source, sink);
            stream->callback.EntryPara.codec_out_size           = ull_audio_v2_dongle_ul_get_stream_out_max_size_each_channel(source, sink);
            stream->callback.EntryPara.codec_out_sampling_rate  = ull_audio_v2_dongle_ul_get_stream_out_sampling_rate_each_channel(source, sink);
        }
#endif /* defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE) */
    }
#endif /* AIR_AUDIO_HARDWARE_ENABLE */

/************************************************************************** ATTENTION ***************************************************************/
/**************************************************** Config callback para of output by sceanrio type ***********************************************/
/************************************************************************** ATTENTION ***************************************************************/
    switch (source->scenario_type) {
        /* BT Source Dongle ------------------------------------------------------------------------------------------------------------------------*/
#ifdef AIR_BT_AUDIO_DONGLE_ENABLE
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_1:
            stream->callback.EntryPara.out_channel_num          = bt_audio_dongle_ul_get_stream_out_channel_number(source, sink);
            stream->callback.EntryPara.codec_out_size           = bt_audio_dongle_ul_get_stream_out_max_size_each_channel(source, sink);
            stream->callback.EntryPara.codec_out_sampling_rate  = bt_audio_dongle_ul_get_stream_out_sampling_rate_each_channel(source, sink);
            break;

        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1:
            stream->callback.EntryPara.out_channel_num          = bt_audio_dongle_dl_get_stream_out_channel_number(source, sink);
            stream->callback.EntryPara.codec_out_size           = bt_audio_dongle_dl_get_stream_out_max_size_each_channel(source, sink);
            stream->callback.EntryPara.codec_out_sampling_rate  = bt_audio_dongle_dl_get_stream_out_sampling_rate_each_channel(source, sink);
            break;
#endif
#endif

        default:
            break;
    }
/**************************************************************************** END *******************************************************************/
/* ------------------------------------------------------------------------------------------------------------------------------------------------ */

    if (((sink->type == SINK_TYPE_AUDIO) || (sink->type == SINK_TYPE_VP_AUDIO) || (sink->type == SINK_TYPE_AUDIO_DL3) || (sink->type == SINK_TYPE_AUDIO_DL12))
        && ((sink->param.audio.audio_device == HAL_AUDIO_CONTROL_DEVICE_INTERNAL_DAC_L)
            || (sink->param.audio.audio_device == HAL_AUDIO_CONTROL_DEVICE_INTERNAL_DAC_R))) {
        stream->callback.EntryPara.device_out_channel_num = 1;

    } else {
        stream->callback.EntryPara.device_out_channel_num = stream->callback.EntryPara.out_channel_num;
        if (sink->type == SINK_TYPE_N9SCO) {
            stream->callback.EntryPara.device_out_channel_num = 1;
        }
    }

    if ((sink->type == SINK_TYPE_AUDIO) || (sink->type == SINK_TYPE_VP_AUDIO) || (sink->type == SINK_TYPE_AUDIO_DL3) || (sink->type == SINK_TYPE_AUDIO_DL12)) {
        stream->callback.EntryPara.software_handled_channel_num = sink->param.audio.sw_channels;

    } else {
        stream->callback.EntryPara.software_handled_channel_num = stream->callback.EntryPara.out_channel_num;

    }

    if (sink->type == SINK_TYPE_N9SCO) {
        stream->callback.EntryPara.encoder_out_size = 240;
    } else {
        stream_feature_type_t encoder_type = (stream->callback.EntryPara.number.field.feature_number>=2)
                                              ? ((stream_feature_ptr_t)(stream->callback.FeatureTablePtr+stream->callback.EntryPara.number.field.feature_number - 2))->feature_type
                                              :((stream_feature_ptr_t)(stream->callback.FeatureTablePtr))->feature_type;

        stream->callback.EntryPara.encoder_out_size = stream_feature_table[encoder_type].codec_output_size;
    }
#ifdef AIR_BT_CODEC_BLE_ENABLED
    if ((source->type == SOURCE_TYPE_N9BLE)&&(source->param.n9ble.context_type != BLE_CONTENT_TYPE_ULL_BLE)&&(source->param.n9ble.context_type != BLE_CONTENT_TYPE_WIRELESS_MIC)

        &&(stream->callback.EntryPara.in_sampling_rate == FS_RATE_96K))
    {
        stream->callback.EntryPara.codec_out_size = 1920*4;
        DSP_MW_LOG_I("[LC3PLUS_DEC] LE with 96K force codec out size  %d",1,stream->callback.EntryPara.codec_out_size);
    }
#endif

    if ((source->type >= SOURCE_TYPE_DSP_VIRTUAL_MIN)&&(source->type <= SOURCE_TYPE_DSP_VIRTUAL_MAX)) {
        if(source->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_OUT_IEM){
            stream->callback.EntryPara.codec_out_size = source->param.virtual_para.mem_size;
            DSP_MW_LOG_I("[wired_audio] USB_IN_OUT_IEM SOURCE_TYPE_DSP_VIRTUAL codec_out_size %d",1,stream->callback.EntryPara.codec_out_size);
        } else if(source->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_MAINSTREAM){
            stream->callback.EntryPara.codec_out_size = source->param.virtual_para.mem_size;
            DSP_MW_LOG_I("[wired_audio] MAIN_STREAM SOURCE_TYPE_DSP_VIRTUAL codec_out_size %d",1,stream->callback.EntryPara.codec_out_size);
        }
    }

    stream->callback.Src.out_frame_size  = (stream->callback.EntryPara.with_src == 0)
                                           ? 0
                                           : stream->callback.Src.out_frame_size;

    /*///////////////////////////////////////////*/

    stream->callback.EntryPara.out_malloc_size = MAX(MAX(MAX(stream->callback.EntryPara.codec_out_size,
                                                             stream->callback.EntryPara.encoder_out_size),
                                                         stream->callback.Src.out_frame_size),
                                                     frameSize);

    //Source VDM SRC
    if (source->type == SOURCE_TYPE_AUDIO || source->type == SOURCE_TYPE_DSP_BRANCH || source->type == SOURCE_TYPE_AUDIO2)
        if (source->param.audio.src != NULL) {
            stream->callback.EntryPara.in_size = stream->callback.EntryPara.out_malloc_size;
        }
#ifdef MTK_SENSOR_SOURCE_ENABLE
    if (source->type == SOURCE_TYPE_GSENSOR) {
        stream->callback.EntryPara.in_channel_num = 1;
        stream->callback.EntryPara.in_size = 384;
    }
#endif
    stream->callback.EntryPara.in_malloc_size = stream->callback.EntryPara.in_size;

#ifdef AIR_DSP_MEMORY_REGION_ENABLE
    GroupMemoryInfo_t *memory_info_ptr;
    SubComponentType_t sub_component_type;
    DspMemoryManagerReturnStatus_t memory_return_status;
#endif
    DSP_MW_LOG_I("[DSP_RESOURCE][DSP_Callback_ParaSetup] scenario: %d DSP stream in/out buf malloc", 1
        , stream && stream->source ? stream->source->scenario_type : AUDIO_SCEANRIO_TYPE_MAX
    );

    if (stream->callback.FeatureTablePtr->FeatureType != CODEC_PCM_COPY) {
        /* malloc in_ptr */
        AUDIO_ASSERT(stream->callback.EntryPara.in_channel_num<=CALLBACK_INPUT_PORT_MAX_NUM);
        mallocSize = stream->callback.EntryPara.in_malloc_size*stream->callback.EntryPara.in_channel_num;
#ifdef AIR_DSP_MEMORY_REGION_ENABLE
        sub_component_type = dsp_memory_get_stream_in_subcomponent_by_scenario(stream->callback.EntryPara.scenario_type);
        memory_info_ptr = dsp_memory_get_stream_in_memory_info_by_scenario(stream->callback.EntryPara.scenario_type);
        memory_return_status = DspMemoryManager_AcquireGroupMemory(dsp_memory_get_component_by_scenario(stream->callback.EntryPara.scenario_type), sub_component_type, memory_info_ptr);
        if (memory_return_status != DSP_MEMORY_MANAGEMENT_PROCESS_OK) {
            DSP_MW_LOG_E("DSP callback ParaSetup In malloc FAIL (%d) !!!", 1, memory_return_status);
            assert(0);
        }
        if (memory_info_ptr->DramSizeInBytes < mallocSize) {
            DSP_MW_LOG_E("DSP callback ParaSetup In memory insufficient Allocated:%d require:%d !!!", 2, memory_info_ptr->DramSizeInBytes , mallocSize);
            assert(0);
        }
        mem_ptr = (uint8_t *)NARROW_UP_TO_8_BYTES_ALIGN((uint32_t)(memory_info_ptr->DramAddr));
#else
        mem_ptr = DSPMEM_tmalloc(stream->callback.EntryPara.DSPTask, mallocSize, stream);
#endif
        memset(mem_ptr, 0, mallocSize);


        for (i = 0 ; i < stream->callback.EntryPara.in_channel_num ; i++) {
            stream->callback.EntryPara.in_ptr[i] = mem_ptr;
            mem_ptr += stream->callback.EntryPara.in_malloc_size;
        }


        /* malloc out_ptr */
        chNum = MAX(stream->callback.EntryPara.out_channel_num, 2);
        AUDIO_ASSERT(chNum <= CALLBACK_OUTPUT_PORT_MAX_NUM);
        mallocSize = stream->callback.EntryPara.out_malloc_size*chNum;
#ifdef AIR_DSP_MEMORY_REGION_ENABLE
        sub_component_type = dsp_memory_get_stream_out_subcomponent_by_scenario(stream->callback.EntryPara.scenario_type);
        memory_info_ptr = dsp_memory_get_stream_out_memory_info_by_scenario(stream->callback.EntryPara.scenario_type);
        memory_return_status = DspMemoryManager_AcquireGroupMemory(dsp_memory_get_component_by_scenario(stream->callback.EntryPara.scenario_type), sub_component_type, memory_info_ptr);
        if (memory_return_status != DSP_MEMORY_MANAGEMENT_PROCESS_OK) {
            DSP_MW_LOG_E("DSP callback ParaSetup Out malloc FAIL (%d) !!!", 1, memory_return_status);
            assert(0);
        }
        if (memory_info_ptr->DramSizeInBytes < mallocSize) {
            DSP_MW_LOG_E("DSP callback ParaSetup Out memory insufficient Allocated:%d require:%d Malloc_size:%d, chNum:%d !!!", 4, memory_info_ptr->DramSizeInBytes , mallocSize, stream->callback.EntryPara.out_malloc_size, chNum);
            assert(0);
        }
        mem_ptr = (uint8_t *)NARROW_UP_TO_8_BYTES_ALIGN((uint32_t)(memory_info_ptr->DramAddr));
#else
        mem_ptr = DSPMEM_tmalloc(stream->callback.EntryPara.DSPTask, mallocSize, stream);
#endif
        memset(mem_ptr, 0, mallocSize);
        for (i = 0 ; i < chNum ; i++) {
            stream->callback.EntryPara.out_ptr[i] = mem_ptr;
            mem_ptr += stream->callback.EntryPara.out_malloc_size;
        }
    } else {
        //CODEC_PCM_COPY
        DSP_MW_LOG_I("[DSP] test out_channel_num%d, in_channel_num%d, ",3,
stream->callback.EntryPara.out_channel_num,stream->callback.EntryPara.in_channel_num,
MAX(MAX(stream->callback.EntryPara.out_channel_num, 1), stream->callback.EntryPara.in_channel_num));
        chNum = MAX(MAX(stream->callback.EntryPara.out_channel_num, 1), stream->callback.EntryPara.in_channel_num);
        AUDIO_ASSERT(chNum <= MIN(CALLBACK_INPUT_PORT_MAX_NUM, CALLBACK_OUTPUT_PORT_MAX_NUM));
        frameSize = MAX(stream->callback.EntryPara.in_malloc_size, stream->callback.EntryPara.out_malloc_size);
        stream->callback.EntryPara.in_malloc_size = stream->callback.EntryPara.out_malloc_size = frameSize;
        if (sink->type != SINK_TYPE_VP_AUDIO) {
            stream->callback.EntryPara.out_channel_num = chNum;
        }
        mallocSize = frameSize * chNum;
        if ((frameSize & 3) && (chNum > 1)) {
            DSP_MW_LOG_I("[DSP] Unaligned Callback Frame Size:%d!!", 1, frameSize);
        }

#ifdef AIR_DSP_MEMORY_REGION_ENABLE
        sub_component_type = dsp_memory_get_stream_in_subcomponent_by_scenario(stream->callback.EntryPara.scenario_type);
        memory_info_ptr = dsp_memory_get_stream_in_memory_info_by_scenario(stream->callback.EntryPara.scenario_type);
        memory_return_status = DspMemoryManager_AcquireGroupMemory(dsp_memory_get_component_by_scenario(stream->callback.EntryPara.scenario_type), sub_component_type, memory_info_ptr);
        if (memory_return_status != DSP_MEMORY_MANAGEMENT_PROCESS_OK) {
            DSP_MW_LOG_E("DSP callback ParaSetup In/Out malloc FAIL (%d) !!!", 1, memory_return_status);
            assert(0);
        }
        if (memory_info_ptr->DramSizeInBytes < mallocSize) {
            DSP_MW_LOG_E("DSP callback ParaSetup In/Out memory insufficient Allocated:%d require:%d !!!", 2, memory_info_ptr->DramSizeInBytes , mallocSize);
            assert(0);
        }
        mem_ptr = (uint8_t *)NARROW_UP_TO_8_BYTES_ALIGN((uint32_t)(memory_info_ptr->DramAddr));;
        memset(mem_ptr, 0, mallocSize);
#else
        if (0) {
    #ifdef AIR_PROMPT_SOUND_MEMORY_DEDICATE
        } else if(source->type == SOURCE_TYPE_CM4_VP_PLAYBACK){
            mem_ptr = g_vp_memptr;
    #endif
        } else {
        mem_ptr = DSPMEM_tmalloc(stream->callback.EntryPara.DSPTask, mallocSize, stream);
        memset(mem_ptr, 0, mallocSize);
        }
#endif
        for (i = 0 ; i < chNum ; i++) {
            stream->callback.EntryPara.out_ptr[i] = mem_ptr;
            stream->callback.EntryPara.in_ptr[i] = mem_ptr;
            mem_ptr += frameSize;
        }
        DSP_MW_LOG_I("[DSP] Callback stream Setup codec is CODEC_PCM_COPY, Frame Size:%d, channel_num:%d", 2, frameSize, chNum);
    }
#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
    if ((source->type >= SOURCE_TYPE_AUDIO_TRANSMITTER_MIN) && (source->type <= SOURCE_TYPE_AUDIO_TRANSMITTER_MAX)) {
        if (source->param.data_dl.scenario_type == AUDIO_TRANSMITTER_A2DP_SOURCE) {

        }
#if defined(AIR_WIRED_AUDIO_ENABLE)
        else if (source->param.data_dl.scenario_type == AUDIO_TRANSMITTER_WIRED_AUDIO) {
            if ((source->param.data_dl.scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) ||
                (source->param.data_dl.scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1)) {
                stream->callback.EntryPara.in_malloc_size = stream->callback.EntryPara.in_size;
            }
        }
#endif /* AIR_WIRED_AUDIO_ENABLE */
    }
#endif /* AIR_AUDIO_TRANSMITTER_ENABLE */

    stream->callback.EntryPara.number.field.process_sequence  = 0;
    stream->callback.EntryPara.number.field.source_type       = (U8)stream->source->type;
    stream->callback.EntryPara.number.field.sink_type         = (U8)stream->sink->type;
    stream->callback.EntryPara.with_encoder                   = FALSE;

    //stream->callback.EntryPara.src_out_size = stream->callback.EntryPara.codec_out_size;
    stream->callback.EntryPara.src_out_sampling_rate =  stream->callback.EntryPara.codec_out_sampling_rate;
}

VOID DSP_Callback_TrigerSourceSRC(SOURCE source, U32 process_length)
{
#ifdef AIR_AUDIO_HARDWARE_ENABLE
    U16 thd_size;
    if (source->type != SOURCE_TYPE_AUDIO && source->type != SOURCE_TYPE_DSP_BRANCH && source->type != SOURCE_TYPE_AUDIO2) {
        return;
    }
    if (source->param.audio.src == NULL) {
        return;
    }
    thd_size = (U16)DSP_GetSRCOutFrameSize(source->param.audio.src->src_ptr);
    source->param.audio.src->accum_process_size += process_length;
    while (source->param.audio.src->accum_process_size >= thd_size) {
        Sink_Audio_Triger_SourceSRC(source);
        source->param.audio.src->accum_process_size -= thd_size;
    }
#else
    UNUSED(source);
    UNUSED(process_length);
#endif /* AIR_AUDIO_HARDWARE_ENABLE */
}

VOID DSP_Callback_TrigerSinkSRC(SINK sink, U32 process_length)
{
#ifdef AIR_AUDIO_HARDWARE_ENABLE
    U16 thd_size;
    if (sink->type != SINK_TYPE_AUDIO &&
        sink->type != SINK_TYPE_VP_AUDIO &&
        sink->type != SINK_TYPE_DSP_JOINT) {
        return;
    }
    if (sink->param.audio.src == NULL) {
        return;
    }
    thd_size = (U16)DSP_GetSRCInFrameSize(sink->param.audio.src->src_ptr);
    sink->param.audio.src->accum_process_size += process_length;
    while (sink->param.audio.src->accum_process_size >= thd_size) {
        Source_Audio_Triger_SinkSRC(sink);
        sink->param.audio.src->accum_process_size -= thd_size;
    }
#else
    UNUSED(sink);
    UNUSED(process_length);
#endif /* AIR_AUDIO_HARDWARE_ENABLE */
}

ATTR_TEXT_IN_IRAM_LEVEL_1 VOID DSP_Callback_CheckSkipProcess(VOID *ptr)
{
    DSP_CALLBACK_HANDLER_PTR handler = ptr;
    handler->stream->callback.EntryPara.skip_process = 0;

    U16 *callbackOutSizePtr;
    callbackOutSizePtr = (handler->stream->callback.EntryPara.with_encoder == TRUE)
                         ? &(handler->stream->callback.EntryPara.encoder_out_size)
                         : (handler->stream->callback.EntryPara.with_src)
                         ? &(handler->stream->callback.EntryPara.src_out_size)
                         : &(handler->stream->callback.EntryPara.codec_out_size);


    if ((handler->handlingStatus == CALLBACK_HANDLER) ||
        (handler->handlingStatus == CALLBACK_ZEROPADDING)) {
        if (*callbackOutSizePtr > (U16)SinkSlack(handler->stream->sink)) {
            handler->stream->callback.EntryPara.in_size = 0;
            *callbackOutSizePtr = 0;
            handler->stream->callback.EntryPara.skip_process = DSP_CALLBACK_SKIP_ALL_PROCESS;
        } else if (handler->stream->callback.EntryPara.in_size == 0) {
            //if ((handler->stream->callback.FeatureTablePtr->ProcessEntry!=MP3_Decoder) &&
            //    (handler->stream->callback.FeatureTablePtr->ProcessEntry!=stream_codec_decoder_sbc_process) &&
            //    (handler->stream->callback.FeatureTablePtr->ProcessEntry!=stream_codec_decoder_aac_process))
            if (handler->stream->callback.FeatureTablePtr->ProcessEntry != stream_pcm_copy_process) {
#ifdef MTK_CELT_DEC_ENABLE
                if (handler->stream->callback.FeatureTablePtr->ProcessEntry != stream_codec_decoder_celt_process)
#endif
                    handler->stream->callback.EntryPara.skip_process = CODEC_ALLOW_SEQUENCE;
            }
#ifdef AIR_BT_CODEC_BLE_V2_ENABLED
            if (handler->stream->callback.FeatureTablePtr->ProcessEntry == stream_codec_decoder_lc3_v2_process) {
                handler->stream->callback.EntryPara.skip_process = 0;
            }
#endif /* AIR_BT_CODEC_BLE_V2_ENABLED */
        }
    } else if (handler->handlingStatus == CALLBACK_BYPASS_CODEC) {
        U32 byte_per_sample;

        byte_per_sample = (handler->stream->callback.EntryPara.resolution.feature_res == RESOLUTION_32BIT)
                          ? 4
                          : 2;
        //if (handler->stream->callback.FeatureTablePtr->ProcessEntry !=stream_codec_decoder_sbc_process)
        {
            handler->stream->callback.EntryPara.skip_process = CODEC_ALLOW_SEQUENCE;
            *callbackOutSizePtr = SourceSize(handler->stream->source) * byte_per_sample;
            handler->stream->callback.EntryPara.pre_codec_out_size = *callbackOutSizePtr;
        }
    }

    handler->stream->callback.EntryPara.force_resume = FALSE;
}


ATTR_TEXT_IN_IRAM_LEVEL_1 VOID DSP_Callback_CheckForceResumeValid(VOID *ptr)
{
    DSP_CALLBACK_HANDLER_PTR handler = ptr;
    U16 *callbackOutSizePtr;
    callbackOutSizePtr = (handler->stream->callback.EntryPara.with_encoder == TRUE)
                         ? &(handler->stream->callback.EntryPara.encoder_out_size)
                         : (handler->stream->callback.EntryPara.with_src)
                         ? &(handler->stream->callback.EntryPara.src_out_size)
                         : &(handler->stream->callback.EntryPara.codec_out_size);

    if (handler->stream->callback.EntryPara.force_resume) {
        if ((handler->handlingStatus == CALLBACK_HANDLER) ||
            (handler->handlingStatus == CALLBACK_ZEROPADDING)) {
            if (*callbackOutSizePtr > (U16)SinkSlack(handler->stream->sink)) {
                handler->stream->callback.EntryPara.force_resume = FALSE;
            }
        }
    }
}


/**
 * DSP_Callback_DropFlushData
 *
 * @Author :  BrianChen <BrianChen@airoha.com.tw>
 */
ATTR_TEXT_IN_IRAM_LEVEL_1 VOID DSP_Callback_DropFlushData(DSP_STREAMING_PARA_PTR stream)
{
    U8 *bufptr;
    U16 callbackOutSize;
    U16 sinkSize, remainSize;
    U16 i;
#ifdef DSP_MIPS_STREAM_PROFILE
    U32 gpt_time_start, gpt_time_end, gpt_time_duration, mask;
#endif
    callbackOutSize = (stream->callback.EntryPara.with_encoder == TRUE)
                      ? (stream->callback.EntryPara.encoder_out_size)
                      : (stream->callback.EntryPara.with_src)
                      ? (stream->callback.EntryPara.src_out_size)
                      : (stream->callback.EntryPara.codec_out_size);

#ifdef MTK_LEAKAGE_DETECTION_ENABLE
    extern bool CM4_Record_leakage_enable;
    if (CM4_Record_leakage_enable && (stream->sink->type == SINK_TYPE_CM4RECORD)) {
        callbackOutSize = 0;
    }
#endif
#ifdef MTK_USER_TRIGGER_FF_ENABLE
    extern bool utff_enable;
    if (utff_enable && (stream->sink->type == SINK_TYPE_CM4RECORD)) {
        callbackOutSize = 0;
    }
#endif

#if AUDIO_DSP_STREAM_CALLBACK_PROCESS_DEBUG_ENABLE
    DSP_MW_LOG_I("[DSP CB][Debug] scenario %d, callbackOutSize %d %d %d %d %d %d", 7,
        stream->sink->scenario_type,
        callbackOutSize,
        stream->callback.EntryPara.with_encoder,
        stream->callback.EntryPara.encoder_out_size,
        stream->callback.EntryPara.with_src,
        stream->callback.EntryPara.src_out_size,
        stream->callback.EntryPara.codec_out_size
        );
#endif /* AUDIO_DSP_STREAM_CALLBACK_PROCESS_DEBUG_ENABLE */

    if (callbackOutSize != 0) {
#if 0
#if 1
        assert(((U16)callbackOutSize <= stream->callback.EntryPara.out_malloc_size));
        while ((U16)callbackOutSize > (U16)SinkSlack(stream->sink)) {
            vTaskSuspend(stream->callback.EntryPara.DSPTask);
            portYIELD();
        }
#endif

        bufptr = ((stream->callback.EntryPara.out_channel_num == 1) ||
                  (stream->callback.EntryPara.with_encoder == TRUE))
                 ? stream->callback.EntryPara.out_ptr[0]
                 : NULL;
        // printf("SinkWriteBuf++\r\n");
        SinkWriteBuf(stream->sink, bufptr, (U32)callbackOutSize);
        // printf("SinkWriteBuf--\r\n");
        SinkFlush(stream->sink, (U32)callbackOutSize);
#else
        //partially flush
        assert(((U16)callbackOutSize <= stream->callback.EntryPara.out_malloc_size));
        remainSize = callbackOutSize;
#if 1
        while (remainSize > 0) {
            while ((U16)SinkSlack(stream->sink) == 0) {
                if ((stream->streamingStatus == STREAMING_END)
#ifdef AIR_BT_HFP_ENABLE
                    || ((ScoDlStopFlag == TRUE) && (stream->source->type == SOURCE_TYPE_N9SCO))
#else
                    || (0)
#endif
#ifdef AIR_BT_CODEC_BLE_ENABLED
                    || ((BleDlStopFlag == TRUE) && (stream->source->type == SOURCE_TYPE_N9BLE))
#endif
                   ) {
                    return;
                }
#ifdef MTK_BT_A2DP_ENABLE
                if ((stream->sink->scenario_type == AUDIO_SCENARIO_TYPE_A2DP) && g_a2dp_hwsrc_ng_flag) {
                    DSP_MW_LOG_I("[DSP CB][Debug] a2dp hwsrc ng, exit dav task.", 0);
                    return;
                }
#endif
#if AUDIO_DSP_STREAM_CALLBACK_PROCESS_DEBUG_ENABLE
                DSP_MW_LOG_I("[DSP CB][Debug] scenario %d source data is enough %d, continue to process with while loop in task %d, then suspend!", 3,
                    stream->sink->scenario_type,
                    remainSize,
                    stream->callback.EntryPara.DSPTask
                    );
#endif
                vTaskSuspend(stream->callback.EntryPara.DSPTask);
            }

#else//Machi test code
        if (remainSize > 0) {
#endif
            sinkSize = (U16)SinkSlack(stream->sink);
            if (remainSize <= (U16)sinkSize) {
                sinkSize = remainSize;
            }

#if 0
            bufptr = ((stream->callback.EntryPara.out_channel_num == 1) ||
                      (stream->callback.EntryPara.with_encoder == TRUE))
                     ? stream->callback.EntryPara.out_ptr[0]
                     : NULL;
#else
            bufptr = ((stream->sink->type == SINK_TYPE_AUDIO) ||
                      (stream->sink->type == SINK_TYPE_VP_AUDIO) ||
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
                      (stream->sink->type == SINK_TYPE_TDMAUDIO) ||
#endif
#if defined(AIR_HEARTHROUGH_MAIN_ENABLE) || defined(AIR_CUSTOMIZED_LLF_ENABLE)
                      (stream->sink->type == SINK_TYPE_LLF) ||
#endif
                      (stream->sink->type == SINK_TYPE_DSP_JOINT) ||
                      (stream->sink->type == SINK_TYPE_AUDIO_DL3) ||
                      (stream->sink->type == SINK_TYPE_AUDIO_DL12))
                     ? NULL
                     : stream->callback.EntryPara.out_ptr[0];
#endif

#ifdef DSP_MIPS_STREAM_PROFILE
            hal_nvic_save_and_set_interrupt_mask(&mask);
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_time_start);
#endif
            SinkWriteBuf(stream->sink, bufptr, (U32)sinkSize);

#ifdef DSP_MIPS_STREAM_PROFILE
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_time_end);
            hal_nvic_restore_interrupt_mask(mask);

            gpt_time_duration = gpt_time_end - gpt_time_start;

            stream->sinkWriteTimeSum += gpt_time_duration;
            stream->sinkWriteTimeCnt++;
            if(gpt_time_duration > stream->sinkWriteTimeMax)
            {
                stream->sinkWriteTimeMax = gpt_time_duration;
            }

            #if 0
            DSP_MW_LOG_I("[DSP_MIPS] scenario: %d SinkWriteBuf() time: %d us, MAX time: %d us", 3
                , stream->source ? stream->source->scenario_type : AUDIO_SCEANRIO_TYPE_DUMMY
                , gpt_time_duration
                , stream->sinkWriteTimeMax
            );
            #endif

            if(stream->sinkWriteTimeCnt >= DSP_MIPS_STREAM_CNT)
            {
                DSP_MW_LOG_I("[DSP_MIPS] scenario: %d SinkWriteBuf() AVG time: %d us, MAX time: %d us", 3
                    , stream->source ? stream->source->scenario_type : AUDIO_SCEANRIO_TYPE_MAX
                    , stream->sinkWriteTimeSum >> DSP_MIPS_STREAM_DIV_SHIFT
                    , stream->sinkWriteTimeMax
                );

                stream->sinkWriteTimeSum = 0;
                stream->sinkWriteTimeCnt = 0;
                stream->sinkWriteTimeMax = 0;
            }
#endif

            SinkFlush(stream->sink, (U32)sinkSize);
#if AUDIO_DSP_STREAM_CALLBACK_PROCESS_DEBUG_ENABLE
                DSP_MW_LOG_I("[DSP CB][Debug] scenario %d sink size %d sink wo[%d] ro[%d]", 4,
                    stream->sink->scenario_type,
                    sinkSize,
                    stream->sink->streamBuffer.BufferInfo.WriteOffset,
                    stream->sink->streamBuffer.BufferInfo.ReadOffset
                    );
#endif
            //SRC vector mode trigger
            DSP_Callback_TrigerSinkSRC(stream->sink, (U32)sinkSize);

            remainSize -= sinkSize;
            if (remainSize > 0) {
                for (i = 0 ; i < stream->callback.EntryPara.out_channel_num ; i++) {
                    memcpy(stream->callback.EntryPara.out_ptr[i],
                           (U8 *)stream->callback.EntryPara.out_ptr[i] + sinkSize,
                           remainSize);
                }
            }
        }
#endif
    }

    //Source Drop
    if (stream->callback.EntryPara.in_size != 0) {
        SourceDrop(stream->source, stream->callback.EntryPara.in_size);
        //SRC vector mode trigger
        DSP_Callback_TrigerSourceSRC(stream->source, (U32)stream->callback.EntryPara.in_size);
#if AUDIO_DSP_STREAM_CALLBACK_PROCESS_DEBUG_ENABLE
        DSP_MW_LOG_I("[DSP CB][Debug] scenario %d source drop size %d source wo[%d] ro[%d]", 4,
            stream->sink->scenario_type,
            stream->callback.EntryPara.in_size,
            stream->source->streamBuffer.BufferInfo.WriteOffset,
            stream->source->streamBuffer.BufferInfo.ReadOffset
            );
#endif
    }
}

/**
 * DSP_Callback_ChangeStatus
 *
 * @Author :  BrianChen <BrianChen@airoha.com.tw>
 */
ATTR_TEXT_IN_IRAM_LEVEL_1 VOID DSP_Callback_ChangeStatus(VOID *ptr) {
    DSP_CALLBACK_HANDLER_PTR handler = ptr;
    if (handler->handlingStatus == handler->stream->callback.Status) {
        handler->stream->callback.Status = handler->nextStatus;
    }
}

/**
 * DSP_Callback_VpAck
 *
 * VP playback ack
 *
 */
VOID DSP_Callback_VpAck(DSP_STREAMING_PARA_PTR stream_ptr) {
    if (stream_ptr->source != NULL) {
#ifdef MTK_PROMPT_SOUND_DUMMY_SOURCE_ENABLE
        if ((stream_ptr->source->type == SOURCE_TYPE_CM4_VP_PLAYBACK) || (stream_ptr->source->type == SOURCE_TYPE_CM4_VP_DUMMY_SOURCE_PLAYBACK))
#else
        if (stream_ptr->source->type == SOURCE_TYPE_CM4_VP_PLAYBACK)
#endif
        {
#ifdef MTK_PROMPT_SOUND_ENABLE
            if (vp_data_request_flag == 1) {
                DSP_MW_LOG_I("[CM4_VP_PB] VP_Request.", 0);
                vp_data_request_flag = 0;
                hal_ccni_message_t msg;
                memset((void *)&msg, 0, sizeof(hal_ccni_message_t));
                msg.ccni_message[0] = 0x707 << 16;
                aud_msg_tx_handler(msg, 0, FALSE);
            }
            if ((vp_config_flag == 1) && (vp_sram_empty_flag == 1)) {
                vp_config_flag = 0;
                vp_sram_empty_flag = 0;
                hal_ccni_message_t ccni_msg;
                memset((void *)&ccni_msg, 0, sizeof(hal_ccni_message_t));
                ccni_msg.ccni_message[0] = (AUDIO_CCNI_MESSAGE_ACK(MSG_MCU2DSP_PROMPT_CONFIG) << 16);  //MSG_DSP2MCU_PROMPT_CONFIG_ACK
                aud_msg_tx_handler(ccni_msg, 0, FALSE);
            }
#endif
        }
    }
}

/**
 * DSP_Callback_StatusUpdate
 *
 * Udpate callback status
 *
 */
VOID DSP_Callback_StatusUpdate(DSP_STREAMING_PARA_PTR stream_ptr) {
    if (stream_ptr->source != NULL) {
        if (stream_ptr->source->type == SOURCE_TYPE_A2DP) {
            if (stream_ptr->sink->param.audio.afe_wait_play_en_cnt == PLAY_EN_TRIGGER_REINIT_MAGIC_NUM) {
                stream_ptr->sink->param.audio.afe_wait_play_en_cnt = PLAY_EN_REINIT_DONE_MAGIC_NUM;
#ifdef AIR_AUDIO_HARDWARE_ENABLE
                afe_volume_digital_set_gain(AFE_HW_DIGITAL_GAIN1, 0);
#endif /* AIR_AUDIO_HARDWARE_ENABLE */
                DSP_MW_LOG_E("AFE wait play en trigger re-sync, scenario_type:%d", 1, stream_ptr->sink->scenario_type);
#ifdef MTK_BT_A2DP_ENABLE
                Au_DL_send_reinit_request(MSG2_DSP2CN4_REINIT_AFE_ABNORMAL, FALSE);
#endif /* MTK_BT_A2DP_ENABLE */
            }
            if ((stream_ptr->callback.Status == CALLBACK_SUSPEND) || (stream_ptr->callback.Status == CALLBACK_WAITEND)) {
                stream_ptr->source->transform->Handler(stream_ptr->source, stream_ptr->sink);
            }
        }
    }
}

/**
 * DSP_Callback_TransformHandle
 *
 * @Author :  BrianChen <BrianChen@airoha.com.tw>
 */
ATTR_TEXT_IN_IRAM_LEVEL_1 VOID DSP_Callback_TransformHandle(VOID *ptr) {
    DSP_STREAMING_PARA_PTR stream = ptr;
#if 0
    U16 sinkFlushSize;
    sinkFlushSize = (stream->callback.EntryPara.with_encoder == TRUE)
                    ? stream->callback.EntryPara.encoder_out_size
                    : (stream->callback.EntryPara.with_src)
                    ? stream->callback.EntryPara.src_out_size
                    : stream->callback.EntryPara.codec_out_size;
    if ((stream->callback.Status == CALLBACK_SUSPEND) &&
        (stream->callback.EntryPara.in_size != 0) &&
        (sinkFlushSize != 0))
#endif
        stream->source->transform->Handler(stream->source, stream->sink);

    if (stream->source->type == SOURCE_TYPE_DSP_BRANCH) {
        stream->source->param.dsp.transform->Handler(stream->source->param.dsp.transform->source,
                                                     stream->source->param.dsp.transform->sink);
    }
    if (stream->sink->type == SINK_TYPE_DSP_JOINT) {
        stream->sink->param.dsp.transform->Handler(stream->sink->param.dsp.transform->source,
                                                   stream->sink->param.dsp.transform->sink);
    }
}

/**
 * DSP_Callback_Processing
 *
 * @Author :  BrianChen <BrianChen@airoha.com.tw>
 */
ATTR_TEXT_IN_IRAM VOID DSP_Callback_Processing(DSP_STREAMING_PARA_PTR stream) {
    SOURCE source = stream->source;
    //SINK sink  = stream->sink;
    DSP_CALLBACK_HANDLER handler;
    U8 *bufptr;
    U16 *callbackOutSizePtr;
    BOOL callbackResult = FALSE;
    handler.stream = stream;
    handler.handlingStatus = stream->callback.Status;
    BOOL reinit_flag;
    BOOL is_prefill;

#ifdef DSP_MIPS_STREAM_PROFILE
    U32 scenario_time_start = 0, scenario_time_end, scenario_time_duration;
#endif


    callbackOutSizePtr = (stream->callback.EntryPara.with_encoder == TRUE)
                         ? &(stream->callback.EntryPara.encoder_out_size)
                         : (stream->callback.EntryPara.with_src)
                         ? &(stream->callback.EntryPara.src_out_size)
                         : &(stream->callback.EntryPara.codec_out_size);

    reinit_flag = false;


    if (handler.handlingStatus == CALLBACK_HANDLER) {
        stream->callback.EntryPara.in_size = SourceSize(source);
        if ((stream->callback.EntryPara.in_size > stream->callback.EntryPara.in_malloc_size)) {
            stream->callback.EntryPara.in_size = stream->callback.EntryPara.in_malloc_size;
        }

#ifdef DSP_MIPS_STREAM_PROFILE
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &scenario_time_start);
#endif

        if (stream->callback.EntryPara.in_size != 0) {
            bufptr = (stream->callback.EntryPara.in_channel_num == 1)
                     ? stream->callback.EntryPara.in_ptr[0]
                     : NULL;
            if (SinkSlack(stream->sink) >= *callbackOutSizePtr) {
#ifdef DSP_MIPS_STREAM_PROFILE
                U32 gpt_time_start, gpt_time_end, gpt_time_duration, mask;
                hal_nvic_save_and_set_interrupt_mask(&mask);
                hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_time_start);
#endif
                SourceReadBuf(source, bufptr, stream->callback.EntryPara.in_size);
#ifdef DSP_MIPS_STREAM_PROFILE
                hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_time_end);
                hal_nvic_restore_interrupt_mask(mask);

                gpt_time_duration = gpt_time_end - gpt_time_start;

                stream->sourceReadTimeSum += gpt_time_duration;
                stream->sourceReadTimeCnt++;
                if(gpt_time_duration > stream->sourceReadTimeMax)
                {
                    stream->sourceReadTimeMax = gpt_time_duration;
                }

                #if 0
                DSP_MW_LOG_I("[DSP_MIPS] scenario: %d SourceReadBuf() time: %d us, MAX time: %d us", 3
                    , stream->source ? stream->source->scenario_type : AUDIO_SCEANRIO_TYPE_DUMMY
                    , gpt_time_duration
                    , stream->sourceReadTimeMax
                );
                #endif

                if(stream->sourceReadTimeCnt >= DSP_MIPS_STREAM_CNT)
                {
                    DSP_MW_LOG_I("[DSP_MIPS] scenario: %d SourceReadBuf() AVG time: %d us, MAX time: %d us", 3
                        , stream->source ? stream->source->scenario_type : AUDIO_SCEANRIO_TYPE_MAX
                        , stream->sourceReadTimeSum >> DSP_MIPS_STREAM_DIV_SHIFT
                        , stream->sourceReadTimeMax
                    );

                    stream->sourceReadTimeSum = 0;
                    stream->sourceReadTimeCnt = 0;
                    stream->sourceReadTimeMax = 0;
                }
#endif
            }
        } else {
            DSP_MW_LOG_I("Callback meet source size 0", 0);
        }

#ifdef MTK_GAMING_MODE_HEADSET
        if (!((stream->sink->type <= SINK_TYPE_AUDIO_TRANSMITTER_MIN) && (stream->sink->type >= SINK_TYPE_AUDIO_TRANSMITTER_MAX) && (stream->sink->param.data_ul.scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_VOICE_HEADSET))) {
#endif
            if (stream->streamingStatus == STREAMING_START || stream->streamingStatus == STREAMING_DEINIT) {
                if (stream->streamingStatus == STREAMING_DEINIT) {
                    reinit_flag = true;
#ifdef AIR_BLE_FEATURE_MODE_ENABLE
                if ((stream->source != NULL) && (stream->source->type == SOURCE_TYPE_N9BLE)) {
                    stream->source->param.n9ble.dl_reinit = true;
                }
                if ((stream->sink != NULL) && (stream->sink->type == SINK_TYPE_N9BLE)) {
                    stream->sink->param.n9ble.ul_reinit = true;
                }
#endif
                }
                #if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE) || defined(AIR_WIRELESS_MIC_RX_ENABLE) || defined(AIR_BLE_AUDIO_DONGLE_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_ENABLE)
                stream->streamingStatus = STREAMING_ACTIVE;
                #else
                stream->streamingStatus = STREAMING_ACTIVE;
                #ifdef AIR_BT_CODEC_BLE_ENABLED
                if (!reinit_flag && ((stream->sink->type == SINK_TYPE_N9BLE)||(stream->source->type == SOURCE_TYPE_N9BLE)))
                {
                    *callbackOutSizePtr = 0;
                }
                else
                #endif
                {
                    handler.handlingStatus = CALLBACK_INIT;
                }
                #endif
#if defined(AIR_WIRED_AUDIO_ENABLE) && !defined(AIR_DCHS_MODE_ENABLE)
                if ((stream->sink->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_IN) ||
                    (stream->sink->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_0) ||
                    (stream->sink->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_1) ||
                    (stream->sink->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_MAINSTREAM)) {
                    handler.handlingStatus = CALLBACK_HANDLER;
                }
#endif
            }
#ifdef MTK_GAMING_MODE_HEADSET
        }
#endif
        if ((stream->streamingStatus == STREAMING_END) &&
            (stream->sink->type != SINK_TYPE_AUDIO)) {
            handler.handlingStatus = CALLBACK_INIT;
        }

#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
        if (((stream->source->scenario_type == AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0) ||
                (stream->source->scenario_type == AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1)) &&
                (stream->callback.EntryPara.in_size == 0)) {
            /* Do hook process in SourceSize() and return 0, then bypass following stream process */
            handler.handlingStatus = CALLBACK_SUSPEND;
            stream->callback.Status = CALLBACK_SUSPEND;
        }
#endif
    } else if (handler.handlingStatus == CALLBACK_INIT) {
        stream->callback.EntryPara.in_size = 0;
    } else if (handler.handlingStatus == CALLBACK_ZEROPADDING) {
        stream->callback.EntryPara.in_size = 0;
    } else if (handler.handlingStatus == CALLBACK_BYPASS_CODEC) {
#if A2DP_DBG_PORT
        hal_gpio_set_output(HAL_GPIO_30, 1);
#endif
        stream->callback.EntryPara.in_size = SourceSize(source);
        if (stream->streamingStatus == STREAMING_START) {
            stream->streamingStatus = STREAMING_ACTIVE;
        }
        AUDIO_ASSERT(stream->callback.EntryPara.out_malloc_size >= SourceSize(source));

    }
    DSP_Callback_CheckSkipProcess(&handler);

    if (!(stream->callback.EntryPara.skip_process == DSP_CALLBACK_SKIP_ALL_PROCESS)) {
        callbackResult = DSP_CallbackEntryTable[handler.handlingStatus](&(stream->callback.EntryPara), stream->callback.FeatureTablePtr);
    }

    switch (handler.handlingStatus) {
        case CALLBACK_MALLOC:
            handler.nextStatus = CALLBACK_INIT;
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
            if (stream->callback.EntryPara.out_channel_num >= 4)
            {
                DSP_MW_LOG_I("[MULTI_STREAM] multi-channels case modify to 2", 0);
                stream->callback.EntryPara.out_channel_num = 2;
            }
#endif
            break;
        case CALLBACK_INIT:
            DSP_Callback_ResolutionConfig(stream);
            //DriftConfiguration(stream, TRUE);
            if (reinit_flag == true) {
                if ((stream->source != NULL) && (stream->source->type == SOURCE_TYPE_N9SCO)) {
                    stream->source->param.n9sco.dl_reinit = true;
                }
                if ((stream->sink != NULL) && (stream->sink->type == SINK_TYPE_N9SCO)) {
                    stream->sink->param.n9sco.ul_reinit = true;
                }
            }
            #ifdef AIR_BT_CODEC_BLE_ENABLED
            if (reinit_flag && ((stream->sink->type == SINK_TYPE_N9BLE)||(stream->source->type == SOURCE_TYPE_N9BLE)))
            {
                handler.nextStatus = CALLBACK_HANDLER;
            }
            else
            #endif
            {
                handler.nextStatus = CALLBACK_SUSPEND;
            }
            break;
        case CALLBACK_BYPASS_CODEC:
        case CALLBACK_ZEROPADDING:
        //DriftConfiguration(stream, FALSE);
        case CALLBACK_HANDLER:
            // printf("DSP_Callback_DropFlushData++\r\n");
            DSP_Callback_DropFlushData(stream);
            // printf("DSP_Callback_DropFlushData--\r\n");
            handler.nextStatus = CALLBACK_SUSPEND;

#ifdef DSP_MIPS_STREAM_PROFILE
            if(scenario_time_start)
            {
                hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &scenario_time_end);

                scenario_time_duration = scenario_time_end - scenario_time_start;

                stream->scenarioTimeSum += scenario_time_duration;
                stream->scenarioTimeCnt++;
                if(scenario_time_duration > stream->scenarioTimeMax)
                {
                    stream->scenarioTimeMax = scenario_time_duration;
                }

                #if 0
                DSP_MW_LOG_I("[DSP_MIPS] scenario: %d Overall_process: time: %d us, MAX time: %d us", 3
                    , stream->source ? stream->source->scenario_type : AUDIO_SCEANRIO_TYPE_MAX
                    , scenario_time_duration
                    , stream->scenarioTimeMax
                );
                #endif

                if(stream->scenarioTimeCnt >= DSP_MIPS_STREAM_CNT)
                {
                    DSP_MW_LOG_I("[DSP_MIPS] scenario: %d Overall_process AVG time: %d us, MAX time: %d us", 3
                        , stream->source ? stream->source->scenario_type : AUDIO_SCEANRIO_TYPE_MAX
                        , stream->scenarioTimeSum >> DSP_MIPS_STREAM_DIV_SHIFT
                        , stream->scenarioTimeMax
                    );

                    stream->scenarioTimeSum = 0;
                    stream->scenarioTimeCnt = 0;
                    stream->scenarioTimeMax = 0;
                }
            }
#endif

            if (callbackResult) {
                if (handler.handlingStatus == CALLBACK_ZEROPADDING) {
                    handler.nextStatus = CALLBACK_WAITEND;
                }
            }
            //DriftCompensation(&(stream->driftCtrl.para));
            break;
        case CALLBACK_SUSPEND:
        case CALLBACK_DISABLE:
        case CALLBACK_WAITEND:
            return;
    }

    DSP_Callback_CheckForceResumeValid(&handler);

#if A2DP_DBG_PORT
    hal_gpio_set_output(HAL_GPIO_30, 0);
#endif
    U32 mask;
    hal_nvic_save_and_set_interrupt_mask(&mask);
    DSP_Callback_ChangeStatus(&handler);
#if defined(AIR_WIRELESS_MIC_RX_ENABLE)
    if (source != NULL) {
        if ((source->scenario_type >= AUDIO_SCENARIO_TYPE_WIRELESS_MIC_RX_UL_USB_OUT_0) &&
            (source->scenario_type <= AUDIO_SCENARIO_TYPE_WIRELESS_MIC_RX_UL_I2S_SLV_OUT_0)) {
            hal_nvic_restore_interrupt_mask(mask);
            return;
        }
    }
#endif

    is_prefill = (stream->source->type == SOURCE_TYPE_A2DP) && (stream->sink->param.audio.sram_empty_fill_size != 0);
    /* If you want the stream to process one frame each irq period, plz bypass the following code. */
    if ((stream->streamingStatus != STREAMING_END) && // stream end ==> bypass
        (
            // case1: [Init Status] fill sink buffer full before streaming ==> continue to process
            (
                (handler.handlingStatus == CALLBACK_INIT) &&
                // which scenario will fill sink buffer full before streaming
                /* scenario owner config */
                (
                    #ifdef MTK_PROMPT_SOUND_DUMMY_SOURCE_ENABLE
                        (stream->source->type == SOURCE_TYPE_CM4_VP_DUMMY_SOURCE_PLAYBACK) ||
                    #endif
                    (stream->source->type == SOURCE_TYPE_A2DP) ||
                    (stream->source->type == SOURCE_TYPE_CM4_VP_PLAYBACK) ||
                    (stream->source->type == SOURCE_TYPE_LLF)

                )
            ) ||
            // case2: [Handler Status] (sink free space > 0) && (source size > 0) ==> continue to process.
            (
                (stream->callback.Status == CALLBACK_SUSPEND) &&
                ((stream->callback.EntryPara.in_size != 0) || (*callbackOutSizePtr != 0) || (stream->callback.EntryPara.force_resume) || (is_prefill == true)) &&
                /* scenario owner config */
                (
                    (stream->source->type != SOURCE_TYPE_N9SCO) &&
                    (stream->sink->type != SINK_TYPE_N9SCO) &&
                    #ifdef AIR_BT_CODEC_BLE_ENABLED
                        (stream->source->type != SOURCE_TYPE_N9BLE) &&
                        (stream->sink->type != SINK_TYPE_N9BLE) &&
                    #endif
                    // Dongle 2.0 line_in/i2s_in bypass
                    #if (defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE)
                        (stream->sink->scenario_type != AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_MST_IN_0) &&
                        (stream->sink->scenario_type != AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_LINE_IN)      &&
                        (stream->sink->scenario_type != AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0) &&
                    #endif
                    // BLE Dongle line_in/i2s_in bypass
                    #if (defined AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE) || (defined AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE)
                        (stream->sink->scenario_type != AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_LINE_IN) &&
                        (stream->sink->scenario_type != AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_I2S_IN)  &&
                    #endif
                    // BLE Dongle USB Out
                    #if (defined AIR_BLE_AUDIO_DONGLE_ENABLE)
                        (stream->sink->scenario_type != AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_VOICE_USB_OUT) &&
                    #endif
                    // Audio transmitter Case
                    #ifdef MTK_GAMING_MODE_HEADSET
                            (!((stream->source->type == SOURCE_TYPE_AUDIO) &&
                            (stream->sink->type >= SINK_TYPE_AUDIO_TRANSMITTER_MIN) &&
                            (stream->sink->type <= SINK_TYPE_AUDIO_TRANSMITTER_MAX) &&
                            (stream->sink->param.data_ul.scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_VOICE_HEADSET))) &&
                    #ifndef AIR_A2DP_PERIODIC_PROCEDURE_V2_EN
                            ((stream->source->type != SOURCE_TYPE_A2DP && stream->source->param.n9_a2dp.codec_info.codec_cap.type != BT_A2DP_CODEC_AIRO_CELT) ||
                            (stream->source->type == SOURCE_TYPE_A2DP && stream->source->param.n9_a2dp.codec_info.codec_cap.type == BT_A2DP_CODEC_AIRO_CELT && (afe_get_dl1_query_data_amount() < 124))) &&
                    #endif
                    #endif
                    // A2DP Case
                    #if defined(AIR_A2DP_PERIODIC_PROCEDURE_V2_EN) && defined(MTK_BT_A2DP_ENABLE)
                            ((stream->source->type != SOURCE_TYPE_A2DP) || (stream->source->type == SOURCE_TYPE_A2DP && !a2dp_dl1_query_data_enough(stream->source))) &&
                    #endif
                    #ifdef AIR_FULL_ADAPTIVE_ANC_ENABLE
                            ((stream->source->type != SOURCE_TYPE_ADAPT_ANC) || (audio_adapt_anc_query_data_amount() >= 2048)) &&
                    #endif
                    #ifdef AIR_HW_VIVID_PT_ENABLE
                            ((stream->source->type != SOURCE_TYPE_HW_VIVID_PT) || (audio_hw_vivid_pt_query_data_amount() >= 2048)) &&
                    #endif
                    // A2DP Case
                    (
                        (stream->source->type != SOURCE_TYPE_A2DP) || (stream->sink->type != SINK_TYPE_AUDIO) || ((Sink_blks[SINK_TYPE_VP_AUDIO] == NULL)
                        #ifdef AIR_AUDIO_HARDWARE_ENABLE
                            || (!stream_audio_check_sink_remain_enough(stream->sink))
                        #endif
                        )
                    )
                )
            )
        )
    ) {
        hal_nvic_restore_interrupt_mask(mask);
        DSP_Callback_TransformHandle(stream);
        hal_nvic_save_and_set_interrupt_mask(&mask);
    }

    hal_nvic_restore_interrupt_mask(mask);
}

