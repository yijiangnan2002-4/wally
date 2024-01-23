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

#ifndef _STREAM_LLF_H_
#define _STREAM_LLF_H_

#if defined(AIR_HEARTHROUGH_MAIN_ENABLE) || defined(AIR_CUSTOMIZED_LLF_ENABLE)

/* Includes ------------------------------------------------------------------*/
#include "dsp_callback.h"


/* Public define -------------------------------------------------------------*/
#define BT_AWS_MCE_AGENT_STATE_ATTACHED     0x60
#define BT_AWS_MCE_ROLE_PARTNER             0x20          /**< Partner Role. */
#define BT_AWS_MCE_ROLE_AGENT               0x40          /**< Agent Role. */
#define NO_OF_LLF_STREAM     (1)
#define AUDIO_SWPT_DUMP_MASK                (0x1 << 3)

#if 1
#define DSP_LLF_LOG_E(_message, arg_cnt, ...)  LOG_MSGID_E(LLF,"[LLF] "_message, arg_cnt, ##__VA_ARGS__)
#define DSP_LLF_LOG_W(_message, arg_cnt, ...)  LOG_MSGID_W(LLF,"[LLF] "_message, arg_cnt, ##__VA_ARGS__)
#define DSP_LLF_LOG_I(_message, arg_cnt, ...)  LOG_MSGID_I(LLF,"[LLF] "_message, arg_cnt, ##__VA_ARGS__)
#define DSP_LLF_LOG_D(_message, arg_cnt, ...)  LOG_MSGID_D(LLF,"[LLF] "_message, arg_cnt, ##__VA_ARGS__)
#else
#define DSP_LLF_LOG_E(_message, arg_cnt, ...)  LOG_E(LLF,"[LLF] "_message, ##__VA_ARGS__)
#define DSP_LLF_LOG_W(_message, arg_cnt, ...)  LOG_W(LLF,"[LLF] "_message, ##__VA_ARGS__)
#define DSP_LLF_LOG_I(_message, arg_cnt, ...)  LOG_I(LLF,"[LLF] "_message, ##__VA_ARGS__)
#define DSP_LLF_LOG_D(_message, arg_cnt, ...)  LOG_D(LLF,"[LLF] "_message, ##__VA_ARGS__)
#endif


/* Public typedef ------------------------------------------------------------*/


/** @brief This enum defines the audio LLF subset */
typedef enum {
    AUDIO_LLF_TYPE_HEARING_AID                 = 0,
    AUDIO_LLF_TYPE_VIVID_PT,
    AUDIO_LLF_TYPE_RESERVED_1,
    AUDIO_LLF_TYPE_RESERVED_2,
    AUDIO_LLF_TYPE_SAMPLE,
    AUDIO_LLF_TYPE_ALL,
    AUDIO_LLF_TYPE_DUMMY                       = 0xFFFFFFFF,
} llf_type_t;

typedef enum {
    LLF_DATA_TYPE_REAR_L = 0,
    LLF_DATA_TYPE_REAR_R,
    LLF_DATA_TYPE_INEAR_L,
    LLF_DATA_TYPE_INEAR_R,
    LLF_DATA_TYPE_TALK,
    LLF_DATA_TYPE_MIC_NUM,
    LLF_DATA_TYPE_MUSIC_VOICE = LLF_DATA_TYPE_MIC_NUM,
    LLF_DATA_TYPE_REF,
    LLF_DATA_TYPE_NUM,
} llf_data_type_t;

typedef enum {
    AUDIO_PSAP_DEVICE_ROLE_HEADSET = 0,
    AUDIO_PSAP_DEVICE_ROLE_EARBUDS_L = 1,
    AUDIO_PSAP_DEVICE_ROLE_EARBUDS_R = 2,

    AUDIO_PSAP_DEVICE_ROLE_MAX = 0xFF
} audio_audio_psap_device_role_t;

typedef struct {
    llf_type_t type;
    U8 sub_mode;
    U8 config_event;
    U32 setting;
} audio_llf_runtime_config_t;

typedef struct {
    llf_type_t type;
    U8 sub_mode;
    U8 channel_num;
    U16 frame_size;
    void *source;
    void *sink;
} audio_llf_stream_ctrl_t;

typedef enum {
   LLF_DL_MIX_TYPE_A2DP = 0,
   LLF_DL_MIX_TYPE_ESCO,
   LLF_DL_MIX_TYPE_VP,
   LLF_DL_MIX_TYPE_NUM,
} audio_llf_dl_mix_type_t;

typedef enum {
    LLF_RUNTIME_CONFIG_EVENT_DL_SWAP_DONE = 100,
} audio_llf_runtime_config_event_t;

typedef stream_feature_list_t* (*dsp_llf_feature_get_list_entry)(llf_type_t scenario_id, U32 sub_id);
typedef void (*dsp_llf_feature_ctrl_entry)(audio_llf_stream_ctrl_t *ctrl);
typedef void (*dsp_llf_feature_common_entry)(SOURCE source, SINK sink);
typedef void (*dsp_llf_runtime_config_entry)(audio_llf_runtime_config_t *param);
typedef void (*dsp_llf_set_dl_state)(bool *dl_state);

typedef struct {
    dsp_llf_feature_get_list_entry feature_list_entry;
    dsp_llf_feature_ctrl_entry     init_entry;
    dsp_llf_feature_common_entry   deinit_entry;
    dsp_llf_feature_ctrl_entry     suspend_entry;
    dsp_llf_feature_ctrl_entry     resume_entry;
    dsp_llf_runtime_config_entry   runtime_config_entry;
    dsp_llf_set_dl_state           set_dl_state_entry;
} dsp_llf_feature_entry_t;

typedef struct {
    U32  sample_per_step_ori[AFE_HW_DIGITAL_GAIN_NUM];
    bool llf_mute_dl_flag;
} llf_dl_mute_ctrl_t;

/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
void llf_sharebuf_semaphore_create(void);
bool llf_sharebuf_semaphore_take(void);
void llf_sharebuf_semaphore_give(void);
SOURCE dsp_open_stream_in_LLF(mcu2dsp_open_param_p open_param);
SINK dsp_open_stream_out_LLF(mcu2dsp_open_param_p open_param);
VOID dsp_llf_callback_processing(DSP_STREAMING_PARA_PTR stream);
void dsp_llf_init(void);
void dsp_llf_open(hal_ccni_message_t msg, hal_ccni_message_t *ack);
void dsp_llf_start(hal_ccni_message_t msg, hal_ccni_message_t *ack);
void dsp_llf_stop(hal_ccni_message_t msg, hal_ccni_message_t *ack);
void dsp_llf_close(hal_ccni_message_t msg, hal_ccni_message_t *ack);
void dsp_llf_suspend(hal_ccni_message_t msg, hal_ccni_message_t *ack);
void dsp_llf_resume(hal_ccni_message_t msg, hal_ccni_message_t *ack);
void dsp_llf_runtime_config(hal_ccni_message_t msg, hal_ccni_message_t *ack);
void dsp_llf_anc_bypass_mode(hal_ccni_message_t msg, hal_ccni_message_t *ack);
n9_dsp_share_info_t* dsp_llf_query_share_info(void);
void dsp_llf_notify(U8 notify_event, U8 *data, U8 data_len);
void dsp_llf_set_audio_dl_status(audio_llf_dl_mix_type_t type, bool is_running);
void dsp_llf_get_audio_dl_status(bool dl_state[LLF_DL_MIX_TYPE_NUM]);
void dsp_llf_set_input_channel_num(U8 num);
U8 dsp_llf_get_input_channel_num(void);
void dsp_llf_mute(bool is_mute);
void dsp_llf_mute_dl(bool is_mute);
U32 dsp_llf_get_data_buf_idx(llf_data_type_t type);
void dsp_llf_wait_dl_swap_ready(void);


/* sub senario API*/
#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
typedef struct {
    U8  enable;
    S8  dBFS;
    U16 freq;
} ha_puretone_gen_t;

typedef enum {
    HA_DSP_NOTIFY_EVENT_DETECT_FB          = 0,
    HA_DSP_NOTIFY_EVENT_AEA_CHANGE_MODE    = 1,
    HA_DSP_NOTIFY_EVENT_AEA_NUMBERS        = 2,
    HA_DSP_NOTIFY_EVENT_QUERY_AWS_INFO     = 3,
    HA_DSP_NOTIFY_EVENT_NUM,
} ha_dsp_notify_event_t;

void HA_hearing_aid_features_modify_input_channel_number(U32 channel_num);
#endif


#endif /* AIR_HEARING_AID_ENABLE */

#endif /* _STREAM_LLF_H_ */
