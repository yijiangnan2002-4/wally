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

#ifndef _PEQ_H_
#define _PEQ_H_

#include "dsp_feature_interface.h"
#include "dsp_utilities.h"
#include "common.h"
#include "peq_nvkey_struct.h"

#define DSP_PEQ_MEMSIZE             ((DSP_PEQ_INSTANCE_MEMSIZE * 2) + DSP_PEQ_OVERLAP_BUFSIZE)
#define DSP_PEQ_INSTANCE_MEMSIZE    ((sizeof(PEQ_ST) + 3) & (~0x3))
#define DSP_PEQ_OVERLAP_BUF_SAMPLES (1024)
#define DSP_PEQ_OVERLAP_BUFSIZE     (DSP_PEQ_OVERLAP_BUF_SAMPLES << 2)
#define PEQ_SWITCH_STEP_PERCENTAGE  (1)
#define PEQ_SWITCH_TOTAL_PERCENTAGE (100)
#ifdef MTK_GAMING_MODE_HEADSET
#define PEQ_OVERLAP_FRAME_SIZE      (8)
#else
#define PEQ_OVERLAP_FRAME_SIZE      (8)
#endif
#define PEQ_DISABLE_ALL                 (0xF1)
#define PEQ_ON_ALL                      (0xF2)
#ifdef MTK_DEQ_ENABLE
#define DEQ_MAX_DELAY_SAMPLES       (4)
#define DSP_DEQ_MEMSIZE             ((DSP_PEQ_INSTANCE_MEMSIZE * 2) + DSP_PEQ_OVERLAP_BUFSIZE + (DEQ_MAX_DELAY_SAMPLES << 2))
#endif

#define DSP_PEQ_SYNC_WITH_BT_CLOCK_PTS

#define DSP_BT_LOG_D(_message, arg_cnt, ...)   LOG_W(MPLOG,_message, ##__VA_ARGS__)

typedef struct dsp_peq_sub_ctrl_s {
    PEQ_ST *peq_instance;
    U8 status;
    S16 overlap_progress;
} DSP_PEQ_SUB_CTRL_t;

typedef struct dsp_peq_ctrl_s {
    DSP_PEQ_NVKEY_t peq_nvkey_param;
    S16             *p_peq_inter_param;
    U16             peq_nvkey_id;
    U8              need_update;
    U8              sample_rate;
    BOOL            enable;
    U8              phase_id;
    U8              proc_ch_mask;

    U8              trigger_drc;
    U32             target_timestamp;

    DSP_PEQ_SUB_CTRL_t sub_ctrl[2];
    S32             *p_overlap_buffer;
    S32             peq_mode;   //0: with DRC, 1: without DRC
    U8              audio_path;
} DSP_PEQ_CTRL_t;

typedef struct dsp_peq_ctrl_adaptive_s {
    DSP_PEQ_NVKEY_t peq_nvkey_param;
    S16             *p_peq_inter_param;
    U16             peq_nvkey_id;
} DSP_ADEQ_CTRL_t;

typedef struct dsp_peq_sync_ctrl_s {
    U8              started;
    U8              use_seqno;
    U32             prepare_bt_clk;
    U32             anchor_bt_clk;
    U32             anchor_timestamp;
    S32             asi_cnt;
    U32             current_timestamp;
} DSP_PEQ_SYNC_CTRL_t;

typedef struct {
    U16 elementID;
    U16 numOfParameter;
    U16 parameter[1];
} PEQ_ELEMENT_STRU;

typedef struct {
    U16 numOfElement;
    U16 peqAlgorithmVer;
    PEQ_ELEMENT_STRU peqElement[1];
} PEQ_PARAMETER_STRU;

typedef struct {
    uint8_t aeq_sound_mode;
} aeq_share_info_t;

enum { //elementID
    PEQ_32K = 0x00, //sync with tool, but no use
    PEQ_44_1K,
    PEQ_48K,
    PEQ_16K,    //sync with tool, but no use
    PEQ_8K,     //sync with tool, but no use
    PEQ_88_2K,
    PEQ_96K,
    PEQ_24K,
    PEQ_50K,
    PEQ_192K,
};

typedef enum {
    PEQ_STATUS_OFF = 0,
    PEQ_STATUS_ON,
    PEQ_STATUS_FADE_OUT,
    PEQ_STATUS_FADE_IN,
} PEQ_STATUS_t;

typedef enum {
    PEQ_AUDIO_PATH_A2DP = 0x0,
    PEQ_AUDIO_PATH_LINEIN,
    PEQ_AUDIO_PATH_ADVANCED_PASSTHROUGH,
    PEQ_AUDIO_PATH_ADAPTIVE_EQ,
    PEQ_AUDIO_PATH_VP,
    PEQ_AUDIO_PATH_VP_AEQ,
    PEQ_AUDIO_PATH_MIC,
    PEQ_AUDIO_PATH_ADVANCED_RECORD,
    PEQ_AUDIO_PATH_USB,
    PEQ_AUDIO_PATH_VIVID_PT,
} peq_audio_path_id_t;

EXTERN bool stream_function_peq_initialize(void *para);
EXTERN bool stream_function_instant_peq_process(void *para);
EXTERN bool stream_function_peq_process(void *para);
EXTERN bool stream_function_peq2_initialize(void *para);
EXTERN bool stream_function_peq2_process(void *para);
EXTERN bool stream_function_peq3_initialize(void *para);
EXTERN bool stream_function_peq3_process(void *para);
EXTERN bool stream_function_peq4_initialize(void *para);
EXTERN bool stream_function_peq4_process(void *para);
EXTERN bool stream_function_adaptive_eq_initialize(void *para);
EXTERN bool stream_function_adaptive_eq_process(void *para);
EXTERN bool stream_function_mic_peq_initialize(void *para);
EXTERN bool stream_function_mic_peq_process(void *para);
EXTERN bool stream_function_advanced_record_peq_initialize(void *para);
EXTERN bool stream_function_advanced_record_peq_process(void *para);
#ifdef MTK_LINEIN_PEQ_ENABLE
EXTERN bool stream_function_wired_usb_peq_initialize(void *para);
EXTERN bool stream_function_wired_usb_peq_process(void *para);
#endif

#ifdef AIR_VP_PEQ_ENABLE
EXTERN bool stream_function_vp_peq_initialize(void *para);
EXTERN bool stream_function_vp_peq_process(void *para);
EXTERN bool stream_function_vp_peq2_initialize(void *para);
EXTERN bool stream_function_vp_peq2_process(void *para);
#ifdef AIR_ADAPTIVE_EQ_ENABLE
EXTERN bool stream_function_vp_aeq_initialize(void *para);
EXTERN bool stream_function_vp_aeq_process(void *para);
#endif
EXTERN bool peq_copy_updated_param(DSP_PEQ_CTRL_t *p_src_peq_ctrl, DSP_PEQ_CTRL_t *p_dst_peq_ctrl);
#endif
#ifdef AIR_HEARTHROUGH_VIVID_PT_ENABLE
bool stream_function_vivid_peq_initialize(void *para);
bool stream_function_vivid_peq_process(void *para);
#endif
#ifdef MTK_DEQ_ENABLE
EXTERN bool stream_function_deq_initialize(void *para);
EXTERN bool stream_function_deq_process(void *para);
EXTERN bool stream_function_deq_mute_initialize(void *para);
EXTERN bool stream_function_deq_mute_process(void *para);
EXTERN void dsp_deq_set_param(hal_ccni_message_t msg, hal_ccni_message_t *ack);
#endif
EXTERN U8 peq_get_trigger_drc(U8 phase_id, U8 type);
EXTERN void PEQ_Reset_Info(void);
EXTERN void PEQ_Set_Param(hal_ccni_message_t msg, hal_ccni_message_t *ack, BOOL BypassTimestamp);
EXTERN void Audio_Peq_Enable_Control(hal_ccni_message_t msg);
EXTERN void PEQ_Update_Info(U32 anchor_bt_clk, U32 anchor_timestamp);
EXTERN void Adaptive_Eq_Set_Share_Info(hal_ccni_message_t msg, hal_ccni_message_t *ack);
EXTERN void adaptive_eq_update_sound_mode(void);
EXTERN void Adaptive_Eq_Set_Param(hal_ccni_message_t msg, hal_ccni_message_t *ack);
EXTERN bool peq_pause_process (peq_audio_path_id_t path_id, bool is_pause);
#endif /* _PEQ_H_ */