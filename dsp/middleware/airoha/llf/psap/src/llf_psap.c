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
#include "llf_psap.h"
#include "src_fixed_ratio_interface.h"
#include "preloader_pisplit.h"
#include "dsp_temp.h"
#include "stream_llf.h"
#ifdef AIR_HEARING_AID_ENABLE
#include "hearing_aid_interface.h"
#elif defined(AIR_HEARTHROUGH_PSAP_ENABLE)
#include "hearthrough_psap_interface.h"
#endif
#include "anc_api.h"
#include "hal_audio_message_struct_common.h"
//temp
#include "dllt.h"

extern void hal_llf_set_source_address(SOURCE source, U8 channel_number, U32 data_order[LLF_DATA_TYPE_NUM], U8 earbuds_ch);
extern void hal_llf_get_mic_device(llf_data_type_t mic, hal_audio_control_t *device, hal_audio_interface_t *dev_interface, U8 *enable);
/* Public define -------------------------------------------------------------*/


/* Public typedef ------------------------------------------------------------*/
typedef enum {
    HA_RUNTIME_CONFIG_EVENT_HA_ENABLE       = 0,
    HA_RUNTIME_CONFIG_EVENT_LEVEL_IND       = 1,
    HA_RUNTIME_CONFIG_EVENT_VOL_IND         = 2,
    HA_RUNTIME_CONFIG_EVENT_MODE_IND        = 3,
    HA_RUNTIME_CONFIG_EVENT_MODE_TABLE      = 4,
    HA_RUNTIME_CONFIG_EVENT_AEA_CONFIG      = 5,
    HA_RUNTIME_CONFIG_EVENT_WNR_SWITCH      = 6,
    HA_RUNTIME_CONFIG_EVENT_BF_CONFIG       = 7,
    HA_RUNTIME_CONFIG_EVENT_AFC_CONFIG      = 8,
    HA_RUNTIME_CONFIG_EVENT_INR_CONFIG      = 9,
    HA_RUNTIME_CONFIG_EVENT_USR_EQ_SWITCH   = 10,
    HA_RUNTIME_CONFIG_EVENT_USR_EQ_GAIN     = 11,
    HA_RUNTIME_CONFIG_EVENT_PURETONE_GEN    = 12,
    HA_RUNTIME_CONFIG_EVENT_MIXMODE_CONFIG  = 13,
    HA_RUNTIME_CONFIG_EVENT_MUTE            = 14,
    HA_RUNTIME_CONFIG_EVENT_HOWLING_DET     = 15,
    HA_RUNTIME_CONFIG_EVENT_DET_FB          = 16,
    HA_RUNTIME_CONFIG_EVENT_MIC_CHANNEL     = 17,
    HA_RUNTIME_CONFIG_EVENT_PASSTHRU_SWITCH = 18,
    HA_RUNTIME_CONFIG_EVENT_FADE_OUT        = 19,
    HA_RUNTIME_CONFIG_EVENT_AEA_JUDGEMENT   = 20,
    HA_RUNTIME_CONFIG_EVENT_AWS_INFO        = 21,
    HA_RUNTIME_CONFIG_EVENT_TRIAL_RUN       = 22,
    HA_RUNTIME_CONFIG_EVENT_SET_MIC_CAL_MODE = 23,
    HA_RUNTIME_CONFIG_EVENT_GET_MIC_CAL_DATA = 24,
    HA_RUNTIME_CONFIG_EVENT_NUM,
} ha_runtime_config_event_t;
/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/

src_fixed_ratio_port_t *ha_smp_in_port;
src_fixed_ratio_port_t *ha_smp_out_port;

#ifdef AIR_HEARING_AID_ENABLE
stream_feature_list_t stream_feature_list_HA[] = {
    CODEC_PCM_COPY,
#ifdef AIR_FIXED_RATIO_SRC
    FUNC_SRC_FIXED_RATIO,//50K -->25K
#endif

    FUNC_HA,

#ifdef AIR_FIXED_RATIO_SRC
    FUNC_SRC_FIXED_RATIO,//25K -->50K
#endif
    FUNC_END,
};

#else
stream_feature_list_t stream_feature_list_PSAP[] = {
    CODEC_PCM_COPY,

#ifdef AIR_FIXED_RATIO_SRC
    FUNC_SRC_FIXED_RATIO,//50K -->25K
#endif

    FUNC_HA,

#ifdef AIR_FIXED_RATIO_SRC
    FUNC_SRC_FIXED_RATIO,//25K -->50K
#endif

    FUNC_HEARTHROUGH_POST_PROC,

    FUNC_END,
};

/*            feature_type,                         memory_size, 0(Must equal to zero),                           initialize_entry,                  process_entry*/
stream_feature_function_t  stream_feature_psap_post_proc_function = {            FUNC_HEARTHROUGH_POST_PROC,         0,                     0,          stream_function_hearing_aid_postproc_initialize,      stream_function_hearing_aid_postproc_process};

#endif

extern U32 hal_llf_get_data_buf_idx(llf_data_type_t type);
extern void hal_llf_interrupt_ctrl(bool enable);
extern void hal_llf_swap_dl(bool no_swap);

/* Private functions ----------------------------------------------------------*/
void HA_hearing_aid_features_init(audio_llf_stream_ctrl_t *ctrl)
{
    /* Input sampling rate: 50K --> 25K*/
    src_fixed_ratio_config_t smp_config;
    memset((U8*)&smp_config, 0, sizeof(src_fixed_ratio_config_t));
#ifdef AIR_HEARING_AID_ENABLE
    smp_config.channel_number = 0;
#else
    smp_config.channel_number = ctrl->channel_num - 1;
#endif
    smp_config.in_sampling_rate = 50000;
    smp_config.out_sampling_rate = 25000;
    smp_config.resolution = RESOLUTION_32BIT;
    smp_config.multi_cvt_mode = SRC_FIXED_RATIO_PORT_MUTI_CVT_MODE_ALTERNATE;
    smp_config.cvt_num = 2;
    smp_config.quality_mode = SRC_FIXED_RATIO_PORT_NORMAL_QUALITY;
    smp_config.max_frame_buff_size= ctrl->frame_size;
    ha_smp_in_port = stream_function_src_fixed_ratio_get_port(ctrl->source);
    stream_function_src_fixed_ratio_init(ha_smp_in_port, &smp_config);

    /* Output sampling rate: 25K --> 50K*/
    memset((U8*)&smp_config, 0, sizeof(src_fixed_ratio_config_t));
    smp_config.channel_number = 1;
    smp_config.in_sampling_rate = 25000;
    smp_config.out_sampling_rate = 50000;
    smp_config.resolution = RESOLUTION_32BIT;
    smp_config.multi_cvt_mode = SRC_FIXED_RATIO_PORT_MUTI_CVT_MODE_ALTERNATE;
    smp_config.cvt_num = 2;
    smp_config.quality_mode = SRC_FIXED_RATIO_PORT_NORMAL_QUALITY;
    ha_smp_out_port = stream_function_src_fixed_ratio_get_2nd_port(ctrl->source);
    stream_function_src_fixed_ratio_init(ha_smp_out_port, &smp_config);


#if defined(AIR_HEARTHROUGH_PSAP_ENABLE)
    dsp_sdk_add_feature_table(&stream_feature_psap_post_proc_function);
#endif

}


void HA_hearing_aid_features_modify_input_channel_number(U32 channel_num)
{
    if (ha_smp_in_port) {
        stream_function_src_fixed_ratio_configure_channel_number(ha_smp_in_port, channel_num);
    }
}

void HA_hearing_aid_suspend(audio_llf_stream_ctrl_t *ctrl)
{
    UNUSED(ctrl);
}

void HA_hearing_aid_resume(audio_llf_stream_ctrl_t *ctrl)
{
    DSP_STREAMING_PARA_PTR  pStream = DSP_Streaming_Get(ctrl->source, ctrl->sink);
    if (pStream) {
        pStream->streamingStatus = STREAMING_DEINIT;
    }
    stream_function_hearing_aid_deinitialize();
    DSP_MW_LOG_I("[HEARING_AID] HA_hearing_aid_resume", 0);
}

void HA_hearing_aid_runtime_config(audio_llf_runtime_config_t *param)
{
    DSP_LLF_LOG_I("[HEARING_AID] HA_hearing_aid_runtime_config, event:%d, setting:%d", 2, param->config_event, param->setting);
    switch (param->config_event) {
        case HA_RUNTIME_CONFIG_EVENT_HA_ENABLE: {
            //param->setting == 0 -->disable,  else enable
            DSP_LLF_LOG_I("[HEARING_AID]set HA enabled(%d)", 1, param->setting);
            stream_function_hearing_aid_set_ha_switch((U8)param->setting);
            break;
        }
        case HA_RUNTIME_CONFIG_EVENT_LEVEL_IND: {
            //param->setting == level
            DSP_LLF_LOG_I("[HEARING_AID]set level index(%d)", 1, param->setting);
            stream_function_hearing_aid_set_level_config((U8)param->setting);
            break;
        }
        case HA_RUNTIME_CONFIG_EVENT_VOL_IND: {
            //param->setting == vol
            DSP_LLF_LOG_I("[HEARING_AID]set volume index(%d)", 1, param->setting);
            stream_function_hearing_aid_set_volume_config((U8)param->setting);
            break;
        }
        case HA_RUNTIME_CONFIG_EVENT_MODE_IND: {
            //param->setting == mode
            DSP_LLF_LOG_I("[HEARING_AID]set mode index(%d)", 1, param->setting);
            stream_function_hearing_aid_set_mode_config((U8)param->setting);
            break;
        }
        case HA_RUNTIME_CONFIG_EVENT_MODE_TABLE: {
            // param->setting = mode index
            psap_mode_nvdm_t* table = (psap_mode_nvdm_t*)((U8*)param + sizeof(audio_llf_runtime_config_t));
            DSP_LLF_LOG_I("[HEARING_AID]set mode table(index %d) table(0x%x) mfa_switch(%d %d) low_cut_switch(%d %d) nr_switch(%d) nr_level(%d) beamforming_switch(%d)", 9, param->setting, table,
                table->mfa_switch_l,
                table->mfa_switch_r,
                table->low_cut_switch_l,
                table->low_cut_switch_r,
                table->nr_switch,
                table->nr_level,
                table->beamforming_switch);
            stream_function_hearing_aid_set_mode_table_config((U8)param->setting, table);
            break;
        }
        case HA_RUNTIME_CONFIG_EVENT_AEA_CONFIG: {
            psap_aea_config_t* aea_config = (psap_aea_config_t*)((U8*)param + sizeof(audio_llf_runtime_config_t));
            DSP_LLF_LOG_I("[HEARING_AID]AEA config aea_switch(%d) aea_nr_switch(%d) aea_nr_level(%d) aea_det_period(%d)", 4, aea_config->aea_switch, aea_config->aea_nr_switch, aea_config->aea_nr_level, aea_config->aea_det_period);
            stream_function_hearing_aid_set_aea_config(aea_config);
            break;
        }
        case HA_RUNTIME_CONFIG_EVENT_WNR_SWITCH: {
            //param->setting == switch
            DSP_LLF_LOG_I("[HEARING_AID]set wnr switch(%d)", 1, param->setting);
            stream_function_hearing_aid_set_wnr_switch((U8) param->setting);
            break;
        }
        case HA_RUNTIME_CONFIG_EVENT_BF_CONFIG: {
            //param->setting : bit0:bf_switch, bit1:bf_switch_mode_ctrl
            psap_bf_config_t* bf_config_t = (psap_bf_config_t*) &param->setting;
            DSP_LLF_LOG_I("[HEARING_AID]set bf switch(%d) bf switch mode(%d)", 2, bf_config_t->bf_switch, bf_config_t->bf_switch_mode_ctrl);
            stream_function_hearing_aid_set_bf_config(bf_config_t);
            break;
        }
        case HA_RUNTIME_CONFIG_EVENT_AFC_CONFIG: {
            //param->setting : bit0:afc_ctrl_switch_l, bit1:afc_ctrl_switch_r
            psap_afc_config_t* afc_config_t = (psap_afc_config_t*) &param->setting;
            DSP_LLF_LOG_I("[HEARING_AID]afc switch l(%d) r(%d)", 2, afc_config_t->afc_ctrl_switch_l, afc_config_t->afc_ctrl_switch_r);
            stream_function_hearing_aid_set_afc_config(afc_config_t);
            break;
        }
        case HA_RUNTIME_CONFIG_EVENT_INR_CONFIG: {
            ha_inr_config_t* inr_config = (ha_inr_config_t*)((U8*)param + sizeof(audio_llf_runtime_config_t));
            DSP_LLF_LOG_I("[HEARING_AID]INR config, inr_switch_l(%d) inr_sensitivity_l(%d) inr_strength_l(%d) inr_switch_r(%d) inr_sensitivity_r(%d) inr_strength_r(%d)", 6,
                inr_config->inr_switch_l, inr_config->inr_sensitivity_l, inr_config->inr_strength_l,
                inr_config->inr_switch_r, inr_config->inr_sensitivity_r, inr_config->inr_strength_r);
            stream_function_hearing_aid_set_inr_config(inr_config);
            break;
        }
        case HA_RUNTIME_CONFIG_EVENT_USR_EQ_SWITCH: {
            psap_user_eq_switch_t* usr_eq_switch = (psap_user_eq_switch_t*)&param->setting;
            DSP_LLF_LOG_I("[HEARING_AID]USR_EQ_SWITCH, L(%d), R(%d)", 2,
                usr_eq_switch->psap_user_eq_switch_l, usr_eq_switch->psap_user_eq_switch_r);
            stream_function_hearing_aid_set_user_eq_switch(usr_eq_switch);
            break;
        }
        case HA_RUNTIME_CONFIG_EVENT_USR_EQ_GAIN: {
            psap_usr_eq_para_t* usr_eq_gain = (psap_usr_eq_para_t*)((U8*)param + sizeof(audio_llf_runtime_config_t));
            stream_function_hearing_aid_set_user_eq_gain(usr_eq_gain);
            break;
        }
        case HA_RUNTIME_CONFIG_EVENT_PURETONE_GEN: {
            ha_puretone_gen_t* pure_tone = (ha_puretone_gen_t*)((U8*)param + sizeof(audio_llf_runtime_config_t));
            DSP_LLF_LOG_I("[HEARING_AID]puretone gen, enable(%d) freq(%d) dBFS(%d)", 3, pure_tone->enable, pure_tone->freq, pure_tone->dBFS);
            stream_function_hearing_aid_set_puretone_config(pure_tone->enable, pure_tone->freq, pure_tone->dBFS);
            break;
        }
        case HA_RUNTIME_CONFIG_EVENT_MIXMODE_CONFIG: {
            psap_scenario_mix_mode_t* mix_mode = (psap_scenario_mix_mode_t*)((U8*)param + sizeof(audio_llf_runtime_config_t));
            DSP_LLF_LOG_I("[HEARING_AID]a2dp_mix_mode_switch(%d) a2dp_drc_switch_l(%d) a2dp_mfa_switch_l(%d) a2dp_mix_mode_psap_gain_l(%d)", 4, mix_mode->a2dp_mix_mode_switch, mix_mode->a2dp_drc_switch_l, mix_mode->a2dp_mfa_switch_l, mix_mode->a2dp_mix_mode_psap_gain_l);
            stream_function_hearing_aid_set_mixmode_config(mix_mode);
            break;
        }
        case HA_RUNTIME_CONFIG_EVENT_MUTE: {
            //param->setting == mute
            DSP_LLF_LOG_I("[HEARING_AID]mute(%d)", 1, param->setting);
            stream_function_hearing_aid_set_mute((U8)param->setting);
            break;
        }
        case HA_RUNTIME_CONFIG_EVENT_HOWLING_DET: {
            ha_alg_how_t* how_det_config = (ha_alg_how_t*)((U8*)param + sizeof(audio_llf_runtime_config_t));
            DSP_LLF_LOG_I("[HEARING_AID]hd enable(%d) sup_han(%d)", 2, how_det_config->how_switch, how_det_config->how_sup_han);
            stream_function_hearing_aid_set_hd_config(how_det_config);
            break;
        }
        case HA_RUNTIME_CONFIG_EVENT_DET_FB: {
            stream_function_hearing_aid_feedback_detect_start();
            break;
        }
        case HA_RUNTIME_CONFIG_EVENT_MIC_CHANNEL: {
            //param->setting == mic
            DSP_LLF_LOG_I("[HEARING_AID]mic channel(%d)", 1, param->setting);
            stream_function_hearing_aid_set_mic_channel(param->setting);
            break;
        }
        case HA_RUNTIME_CONFIG_EVENT_PASSTHRU_SWITCH: {
            //param->setting == switch
            DSP_LLF_LOG_I("[HEARING_AID]pass thru(%d)", 1, param->setting);
            stream_function_hearing_aid_set_passthrough((U8)param->setting);
            break;
        }
        case HA_RUNTIME_CONFIG_EVENT_FADE_OUT: {
            stream_function_hearing_aid_set_ha_switch(false);//fade out
            break;
        }
        case HA_RUNTIME_CONFIG_EVENT_AEA_JUDGEMENT: {
            //param->setting == data length
            U32* aea_data = (U32*)((U8*)param + sizeof(audio_llf_runtime_config_t));
            DSP_LLF_LOG_I("[HEARING_AID]receive aea judgement, data_len:%d, aea_data(0~4)=%d %d %d %d %d", 6, param->setting, aea_data[0], aea_data[1], aea_data[2], aea_data[3], aea_data[4]);
            stream_function_hearing_aid_set_aea_partner_indicator(aea_data);
            break;
        }
        case HA_RUNTIME_CONFIG_EVENT_AWS_INFO: {
            //this event occur when dsp_llf_notify(HA_DSP_NOTIFY_EVENT_QUERY_AWS_INFO, NULL, 0);
            U32 aws_state = (param->setting >> 8) & 0xFF;
            U32 aws_role = param->setting & 0xFF;
            DSP_LLF_LOG_I("[HEARING_AID]aws state:0x%x, role:0x%x", 2, aws_state, aws_role);
            stream_function_hearing_aid_update_aws_info(aws_state, aws_role);
            break;
        }
        case HA_RUNTIME_CONFIG_EVENT_TRIAL_RUN: {
            U32 event = param->setting >> 16;
            U32 data_len = param->setting & 0xFF;
            void* data = (U32*)((U8*)param + sizeof(audio_llf_runtime_config_t));
            DSP_LLF_LOG_I("[HEARING_AID]Trial Run event(%d), data_len(%d)", 2, event, data_len);
            stream_function_hearing_aid_trial_run(event, data_len, data);
            break;
        }
        case HA_RUNTIME_CONFIG_EVENT_SET_MIC_CAL_MODE: {
            DSP_LLF_LOG_I("[HEARING_AID]set mic cal mode(%d)", 1, param->setting);
            stream_function_hearing_aid_set_mic_cal_mode((U8)param->setting);
            break;
        }
        case HA_RUNTIME_CONFIG_EVENT_GET_MIC_CAL_DATA: {
            DSP_LLF_LOG_I("[HEARING_AID]get mic cal data", 0);
            n9_dsp_share_info_t* share_info = dsp_llf_query_share_info();
            U32* data_len_p = (U32*)share_info->start_addr;
            U8* dst = (U8*)(data_len_p + 1);
            *data_len_p = stream_function_hearing_aid_get_mic_cal_data((void *)dst);
//            DSP_LLF_LOG_I("[HEARING_AID] get data(len %d): %x%x%x%x%x%x%x%x%x%x", 11, *data_len_p,
//                    *dst, *(dst+1), *(dst+2), *(dst+3), *(dst+4), *(dst+5), *(dst+6), *(dst+7), *(dst+8), *(dst+9));
            break;
        }
        default:
            stream_function_hearing_aid_set_runtime_config(param->config_event, param->setting);
            break;
    }
}

/* Public functions ----------------------------------------------------------*/
void psap_features_init(audio_llf_stream_ctrl_t *ctrl)
{
    switch (ctrl->sub_mode) {
        case HA_SUB_MODE_HEARING_AID:
        case HA_SUB_MODE_PSAP:
        {
            HA_hearing_aid_features_init(ctrl);
            break;
        }
        default:
            break;
    }

}

void psap_features_deinit(SOURCE source, SINK sink)
{
    UNUSED(source);
    UNUSED(sink);

    if (ha_smp_in_port) {
        stream_function_src_fixed_ratio_deinit(ha_smp_in_port);
        ha_smp_in_port = NULL;
    }
    if (ha_smp_out_port) {
        stream_function_src_fixed_ratio_deinit(ha_smp_out_port);
        ha_smp_out_port = NULL;
    }

}

void psap_suspend(audio_llf_stream_ctrl_t *ctrl)
{
    switch (ctrl->sub_mode) {
        case HA_SUB_MODE_HEARING_AID:
        case HA_SUB_MODE_PSAP:
        {
            HA_hearing_aid_suspend(ctrl);
            break;
        }
        default:
            break;
    }
}

void psap_resume(audio_llf_stream_ctrl_t *ctrl)
{
    switch (ctrl->sub_mode) {
        case HA_SUB_MODE_HEARING_AID:
        case HA_SUB_MODE_PSAP:
        {
            HA_hearing_aid_resume(ctrl);
            break;
        }
        default:
            break;
    }
}

stream_feature_list_t* psap_features_get_list(llf_type_t scenario_id, U32 sub_id)
{
    UNUSED(scenario_id);
#ifdef AIR_HEARING_AID_ENABLE
    if (sub_id == HA_SUB_MODE_HEARING_AID) {
        DSP_LLF_LOG_I("[HEARING_AID]stream_feature_list_HA", 0);
        return stream_feature_list_HA;
    }
#else
    if (sub_id == HA_SUB_MODE_PSAP) {
        return stream_feature_list_PSAP;
        DSP_LLF_LOG_I("[HEARING_AID]stream_feature_list_PSAP", 0);
    }
#endif
    return NULL;
}

void psap_runtime_config(audio_llf_runtime_config_t *param)
{
    if (param->sub_mode < HA_SUB_MODE_NUM) {
        HA_hearing_aid_runtime_config(param);
    }
}

void psap_set_dl_state(bool *dl_state)
{
    stream_function_hearing_aid_set_dl_state(dl_state);
}

