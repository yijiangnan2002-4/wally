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
#include "llf_vivid_pt.h"
#include "vivid_passthru_interface.h"



/* Public define -------------------------------------------------------------*/

/* Public typedef ------------------------------------------------------------*/
typedef enum {
    VIVID_PT_RUNTIME_CONFIG_EVENT_ENABLE_DEVICE       = 0,
    VIVID_PT_RUNTIME_CONFIG_EVENT_AFC_SWTICH          = 1,
    VIVID_PT_RUNTIME_CONFIG_EVENT_NR_SWTICH           = 2,
    VIVID_PT_RUNTIME_CONFIG_EVENT_BYPASS_SWTICH       = 3,
    VIVID_PT_RUNTIME_CONFIG_EVENT_NUM,
} vivid_pt_runtime_config_event_t;

/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
stream_feature_list_t stream_feature_list_vivid_PT[] = {
    CODEC_PCM_COPY,
    FUNC_HEARTHROUGH_VIVID_PT,
    FUNC_HEARTHROUGH_VIVID_PEQ,
    FUNC_HEARTHROUGH_POST_PROC,
    FUNC_END,
};

/*            feature_type,                         memory_size, 0(Must equal to zero),                           initialize_entry,                  process_entry*/
stream_feature_function_t  stream_feature_vivid_PT_post_proc_function = {            FUNC_HEARTHROUGH_POST_PROC,         0,                     0,          stream_function_vivid_passthru_post_proc_initialize, stream_function_vivid_passthru_post_proc_process};

/* Private functions ----------------------------------------------------------*/
extern void hal_llf_get_mic_device(llf_data_type_t mic, hal_audio_control_t *device, hal_audio_interface_t *dev_interface, U8 *enable);

/* Public functions ----------------------------------------------------------*/
stream_feature_list_t* dsp_vivid_PT_features_get_list(llf_type_t scenario_id, U32 sub_id)
{
    UNUSED(scenario_id);
    UNUSED(sub_id);
    return stream_feature_list_vivid_PT;
}

void dsp_vivid_PT_feature_init(audio_llf_stream_ctrl_t *ctrl)
{
    UNUSED(ctrl);
    DSP_LLF_LOG_I("[VIVID_PT]feature init", 0);
    dsp_sdk_add_feature_table(&stream_feature_vivid_PT_post_proc_function);
}

void dsp_vivid_PT_runtime_config(audio_llf_runtime_config_t *param)
{
    DSP_LLF_LOG_I("[VIVID_PT] dsp_vivid_PT_runtime_config, event:%d, setting:%d", 2, param->config_event, param->setting);
    switch (param->config_event) {
        case VIVID_PT_RUNTIME_CONFIG_EVENT_ENABLE_DEVICE: {
            stream_function_vivid_passthru_mute(true);
            break;
        }
        case VIVID_PT_RUNTIME_CONFIG_EVENT_AFC_SWTICH:{
            stream_function_vivid_passthru_set_afc_switch(param->setting);
            break;
        }
        case VIVID_PT_RUNTIME_CONFIG_EVENT_NR_SWTICH:{
            stream_function_vivid_passthru_set_ldnr_switch(param->setting);
            break;
        }
        case VIVID_PT_RUNTIME_CONFIG_EVENT_BYPASS_SWTICH: {
            stream_function_vivid_passthru_set_bypass_switch(param->setting);
            break;
        }
        default:
            break;
    }
}

void dsp_vivid_PT_suspend(audio_llf_stream_ctrl_t *ctrl)
{
    UNUSED(ctrl);
    DSP_LLF_LOG_I("[VIVID_PT]suspend", 0);
}

void dsp_vivid_PT_resume(audio_llf_stream_ctrl_t *ctrl)
{
    DSP_STREAMING_PARA_PTR  pStream = DSP_Streaming_Get(ctrl->source, ctrl->sink);
    if (pStream) {
        pStream->streamingStatus = STREAMING_DEINIT;
    }
    DSP_LLF_LOG_I("[VIVID_PT]resume", 0);
    stream_function_vivid_passthru_deinitialize();
}

