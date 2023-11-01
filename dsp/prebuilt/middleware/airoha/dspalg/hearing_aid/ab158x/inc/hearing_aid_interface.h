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

#ifndef _HEARING_AID_INTERFACE_H_
#define _HEARING_AID_INTERFACE_H_

/* Includes ------------------------------------------------------------------*/
#include "dsp_sdk.h"
#include "dsp_utilities.h"
#include "ha_mp_nvkey_struct.h"
#include "ha_algo_nvkey_struct.h"
#include "ha_user_setting_nvkey_struct.h"

/* Public define -------------------------------------------------------------*/
#define HA_MAX(A,B) ((A)>(B)?(A):(B))
#define HA_MIN(A,B) ((A)<(B)?(A):(B))

#define HA_BLOCK_SIZE       (25)
#define HA_BLOCK_ALIGN_SIZE (32)
#define HA_FFT_SIZE         (100)
#define HA_FFT_BAND_SIZE    (50)
#define HA_DRC_BAND_SIZE    HA_FFT_BAND_SIZE

#define HA_LEVEL_TABLE_BYTES        (sizeof(psap_level_nvdm_t))
#define HA_DRC_TABLE_BYTES          (HA_DRC_BAND_SIZE * sizeof(psap_drc_band_nvdm_t))
#define HA_DRC_EQ_TABLE_BYTES       (HA_DRC_BAND_SIZE * sizeof(S32))
#define HA_SCRATCH_BUFFER_BYTES     HA_MAX(HA_MAX(HA_LEVEL_TABLE_BYTES, HA_DRC_EQ_TABLE_BYTES), HA_DRC_TABLE_BYTES)

#define HA_1KHZ_INDEX               (4)
#define HA_MIC_DRC_MAX_INDEX        (49) /* 12250Hz */

#define HA_HS_IMPROVEMENT       (0)

typedef struct {
    U8                      afc_enabled;
    U8                      inr_enabled;
    U8                      lowcut_enabled;
    U8                      nr_enabled;
    U8                      bfm_enabled;
    U8                      wnr_enabled;
    U8                      wnr_gain_enabled;
    U8                      reserved[3];
    U8                      aea_enabled;
    U8                      aea_nr_enabled;
    U8                      hd_enabled;
    U8                      drc_enabled;
    S8                      scenario_ha_gain[3];
    U8                      scenario_drc_enabled[3];
    U8                      isAudioDrcOn;
    U8                      isAudioOn;
    U8                      mute_enabled;
    U8                      pass_through_enabled;
    U8                      drc_level;
    U8                      mic_channel;
    U8                      mic_calibraion_enabled;
    U8                      dual_mic_enabled;
    U8                      puretone_enable;
    S8                      puretone_dBFS;
    U16                     puretone_freq;
    U8                      afc_fs_enabled;
    U8                      inr_detected;
    U8                      nr_target_level;
    U8                      nr_current_level;
    U32                     nr_smooth_level_q8p8;
    U32                     nr_current_level_q8p8;
    S32                     nr_smooth_times;
    S16                     afc_gse_multiplier_shift;
    S16                     afc_gse_multiplier_frac;
} PACKED ha_alg_switch_t;

typedef struct {
    S8  gse_frac_index;
    U8  count;
    U8  gse_shift;
    U8  max_gse_frac_index;
    S32 ref_energy_sum;
} PACKED ha_alg_afc_acs_t;

#if (HA_HS_IMPROVEMENT == 1)
typedef struct {
    U8  mute_state;
    U8  acs_target;
    U8  mute_index;
    S8  mute_gain;
    S32 input_energy_sum;
    S32 feedback_energy_sum;
    S32 detect_times;
    S32 smooth_times;
    S32 detect_stop_times;
} PACKED ha_alg_afc_hs_t;
#else
typedef struct {
    U8  mute_state;
    U8  Reserved[3];
    S32 input_energy_sum;
    S32 feedback_energy_sum;
    S32 detect_times;
} PACKED ha_alg_afc_hs_t;
#endif

typedef struct {
    S32 current;
    S32 target;
    U32 state;
    S32 times;
    S32 step;
} PACKED ha_alg_mic_smooth_t;

typedef struct {
    U8 INR_switch            : 1;
    U8 inr_sensitivity       : 4;
    U8 reserved_15           : 3;
    U8 inr_strength          : 2;
    U8 reserved_16           : 6;
} PACKED ha_alg_inr_t;

typedef struct {
    U8 how_switch;
    U16 how_sup_han;
} PACKED ha_alg_how_t;

typedef struct {
    U8 mode_index;
    psap_mode_nvdm_t* mode_table_ptr;                            /**< @Value 0 @Desc 1 */
} PACKED ha_mode_cmd_t;

typedef struct {
    psap_mp_nvdm_t mp_data;
    psap_afc_coef_nvdm_t afc_coef;
    psap_drc_ctrl_nvdm_t drc_ctrl;
    psap_level_nvdm_t level_data[16];
    psap_para_nvdm_t ha_para;
    psap_low_cut_nvdm_t low_cut;
    psap_mode_ctrl_nvdm_t mode_ctrl;
    psap_mode_eq_nvdm_t mode[8];
    psap_vendor_eq_nvdm_t vendor_eq;
    psap_vol_nvdm_t vol_table;
    psap_aea_coef_nvdm_t AEA_coef;
    psap_bf_nvdm_t bf_para;
    howling_detect_nvdm_t hd_para;
    ha_wnr_nvdm_t wnr;
    psap_bf_sc_nvdm_t bf_sc_para;
    psap_system_nvdm_t ha_system;
    psap_usr_setting_nvdm_t usr_setting;
} PACKED ha_nvkey_t;

/* Public typedef ------------------------------------------------------------*/
typedef struct {
    U32                     memory_check;
    U8                      nvkey_ready;
    U8                      init_done;
    U8                      earbuds_ch;
    U8                      two_mic_exist;
    U8                      dynamic_channel_num;
    U8                      ha_switch;
    ha_alg_inr_t            inr_ctrl;
    ha_alg_switch_t         alg_ctl;
    ha_alg_afc_acs_t        afc_acs_ctl[2];     /* Master, Sec */
    ha_alg_afc_hs_t         afc_hs_ctl[2];      /* Master, Sec */
    ha_alg_mic_smooth_t     mic_out_smooth_ctl;
    U8                      ha_switch_target;
    U8                      pt_switch_target;
    U8                      afc_working_flag;
    U8                      afc_update_flag;
    U8                      deinitial_flag;
    U8                      audio_dump_align;
    U8                      audio_dump_flag;
    U8                      aea_receive_partner_flag;
    U8                      afc_acs_debug;
    U8                      afc_hs_debug;
    U8                      afc_fbd_debug;
    U8                      reversed[3];
    S16                     anc_ff_cal_gain;
    S32                     bypass_gain_frac;
    S32                     bypass_gain_shift;
    S32                     inputmax_1khz_q16;
    U32                     delay_block_count;
    U32                     block_count;
    S32                     out_block_maxabs;
    S32                     block_max_limit_music;
    S32                     block_max_limit_mic;
    bool                    dl_state[3];
    bool                    no_swap_dl_flag[3];
    U32                     aea_period;
    U32                     aea_envIndex;
    S32                     drc_eq_smooth_times;

    /* Mic path */
    S32                     drc_eqtable_current_q8p8[50];
    S32                     drc_eqtable_smooth_q8p8[50];
    DSP_ALIGN4 U8           aea_indicator_partner[20];
    DSP_ALIGN4 S8           drc_eqtable_current[52];
    DSP_ALIGN4 S8           drc_eqtable_target[52];
    DSP_ALIGN4 U8           drc_inputmax[52];
    DSP_ALIGN4 U8           rear_inputmax[52];
    DSP_ALIGN4 U8           front_inputmax[52];
    DSP_ALIGN16 S32         InBuf_rear_align[HA_BLOCK_ALIGN_SIZE];
    DSP_ALIGN16 S32         InBuf_front_align[HA_BLOCK_ALIGN_SIZE];
    /* Audio path */
    DSP_ALIGN4 S8           drc_eqtable_audio[52];
    DSP_ALIGN4 U8           drc_inputmax_audio[52];
    /* Shared */
    DSP_ALIGN4 U8           drc_outputmax[52];
    DSP_ALIGN16 S32         fft_result_master[200];
    DSP_ALIGN16 S32         fft_result_sec[200];
    DSP_ALIGN16 S32         fft_gain[200];
    DSP_ALIGN16 S32         ola_buffer[100];
    DSP_ALIGN16 U8          scratch_buffer[HA_SCRATCH_BUFFER_BYTES];
    DSP_ALIGN16 ha_nvkey_t  nvkey;
    DSP_ALIGN16 U32         scratch_memory;
} ha_instance, *ha_instance_ptr;

#define HA_NVKEY_SIZE     (sizeof(ha_nvkey_t))

#if defined(AIR_HEARTHROUGH_HA_USE_PIC)
#define HA_AFC_MEM_SIZE         1728
#define HA_BFM_MEM_SIZE         1576
#define HA_AEA_MEM_SIZE         128
#define HA_INR_MEM_SIZE         208
#else
#define HA_AFC_MEM_SIZE         1744
#define HA_BFM_MEM_SIZE         1456
#define HA_AEA_MEM_SIZE         112
#define HA_INR_MEM_SIZE         56
#endif
#define HA_AFC_FS_MEM_SIZE      544
#define HA_FFT_SCRATCH_MEM_SIZE 800  /* ver.2301170200: FFT 100, !! Use Scratch mem !! */
#define HA_OLA_MEM_SIZE         816  /* ver.2301170200: OLA 100/25 */
#define HA_ANR_MEM_SIZE         1056
#define HA_DRC_MEM_SIZE         240  /* ver.2302161100: 50 bands */
#define HA_DRC_TABLE_MEM_SIZE   1344 /* ver.2302161100: 50 bands */
#define HA_DRC_SCRATCH_MEM_SIZE 416  /* ver.2302161100: 50 bands, !! Use Scratch mem !! */
#define HA_WNR_MEM_SIZE         1856
#define HA_HOW_MEM_SIZE         944
#define HA_CALIB_MEM_SIZE       640
#define HA_CALIB_SCRATCH_MEM_SIZE 208 /* ver.2305290101: FFT 100 */
#define HA_SCRATCH_MEM_SIZE     HA_MAX(HA_MAX(HA_FFT_SCRATCH_MEM_SIZE, HA_DRC_SCRATCH_MEM_SIZE), HA_CALIB_SCRATCH_MEM_SIZE)
#define GET_ALIGN_SIZE(SIZE)    ((SIZE + 15) & 0x7FFFFFF0)

#define HA_AFC_MASTER_MEM_POSITION      0
#define HA_AFC_SEC_MEM_POSITION         HA_AFC_MASTER_MEM_POSITION      + GET_ALIGN_SIZE(HA_AFC_MEM_SIZE)
#define HA_AFC_FS_MEM_POSITION          HA_AFC_SEC_MEM_POSITION         + GET_ALIGN_SIZE(HA_AFC_MEM_SIZE)
#define HA_INR_MEM_POSITION             HA_AFC_FS_MEM_POSITION          + GET_ALIGN_SIZE(HA_AFC_FS_MEM_SIZE)
#define HA_OLA_MASTER_MEM_POSITION      HA_INR_MEM_POSITION             + GET_ALIGN_SIZE(HA_INR_MEM_SIZE)
#define HA_OLA_SEC_MEM_POSITION         HA_OLA_MASTER_MEM_POSITION      + GET_ALIGN_SIZE(HA_OLA_MEM_SIZE)
#define HA_ANR_MEM_POSITION             HA_OLA_SEC_MEM_POSITION         + GET_ALIGN_SIZE(HA_OLA_MEM_SIZE)
#define HA_BFM_MEM_POSITION             HA_ANR_MEM_POSITION             + GET_ALIGN_SIZE(HA_ANR_MEM_SIZE)
#define HA_MIC_DRC_MEM_POSITION         HA_BFM_MEM_POSITION             + GET_ALIGN_SIZE(HA_BFM_MEM_SIZE)
#define HA_MIC_DRC_TABLE_MEM_POSITION   HA_MIC_DRC_MEM_POSITION         + GET_ALIGN_SIZE(HA_DRC_MEM_SIZE)
#define HA_AEA_MEM_POSITION             HA_MIC_DRC_TABLE_MEM_POSITION   + GET_ALIGN_SIZE(HA_DRC_TABLE_MEM_SIZE)
#define HA_WNR_MEM_POSITION             HA_AEA_MEM_POSITION             + GET_ALIGN_SIZE(HA_AEA_MEM_SIZE)
#define HA_CALIB_MEM_POSITION           HA_WNR_MEM_POSITION             + GET_ALIGN_SIZE(HA_WNR_MEM_SIZE)
#define HA_HOW_MEM_POSITION             HA_CALIB_MEM_POSITION           + GET_ALIGN_SIZE(HA_CALIB_MEM_SIZE)
#define HA_AUDIO_OLA_MEM_POSITION       HA_CALIB_MEM_POSITION           + GET_ALIGN_SIZE(HA_CALIB_MEM_SIZE)
#define HA_AUDIO_DRC_MEM_POSITION       HA_AUDIO_OLA_MEM_POSITION       + GET_ALIGN_SIZE(HA_OLA_MEM_SIZE)
#define HA_AUDIO_DRC_TABLE_MEM_POSITION HA_AUDIO_DRC_MEM_POSITION       + GET_ALIGN_SIZE(HA_DRC_MEM_SIZE)
#define HA_SCRATCH_MEM_POSITION         HA_AUDIO_DRC_TABLE_MEM_POSITION + GET_ALIGN_SIZE(HA_DRC_TABLE_MEM_SIZE)
#define HA_ALG_TOTAL_MEM                HA_SCRATCH_MEM_POSITION         + GET_ALIGN_SIZE(HA_SCRATCH_MEM_SIZE)

#define DSP_HA_MEMSIZE    (sizeof(ha_instance) + HA_ALG_TOTAL_MEM)
#define HA_RUNTIME_SYNC_PARA_ID         (0xE87F)

/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
void stream_function_hearing_aid_set_runtime_config(U8 event, U32 param);
void stream_function_hearing_aid_set_ha_switch(U8 ha_switch);
void stream_function_hearing_aid_set_level_config(U8 level_index);
void stream_function_hearing_aid_set_volume_config(U8 volume_index);
void stream_function_hearing_aid_set_mode_config(U8 mode_index);
void stream_function_hearing_aid_set_mode_table_config(U8 mode_index, psap_mode_nvdm_t* mode_table);
void stream_function_hearing_aid_set_aea_config(psap_aea_config_t* aea_config);
void stream_function_hearing_aid_set_wnr_switch(U8 wnr_switch);
void stream_function_hearing_aid_set_bf_config(psap_bf_config_t* bf_config);
void stream_function_hearing_aid_set_afc_config(psap_afc_config_t *param);
void stream_function_hearing_aid_set_inr_config(ha_inr_config_t* inr_config);
void stream_function_hearing_aid_set_user_eq_switch(psap_user_eq_switch_t* usr_eq_switch);
void stream_function_hearing_aid_set_user_eq_gain(psap_usr_eq_para_t* usr_eq_gain);
void stream_function_hearing_aid_set_puretone_config(U8 pure_tone_enable, U16 pure_tone_freq, S8 pure_tone_dBFS);
void stream_function_hearing_aid_set_mixmode_config(psap_scenario_mix_mode_t* mixmode_config);
void stream_function_hearing_aid_set_hd_config(ha_alg_how_t *how_det_config);
void stream_function_hearing_aid_set_mute(U8 mute_switch);
void stream_function_hearing_aid_set_mic_channel(U8 mic_channel);
void stream_function_hearing_aid_set_passthrough(U8 passthrough_switch);
void stream_function_hearing_aid_set_aea_partner_indicator(U32 *aea_data);
void stream_function_hearing_aid_update_aws_info(U32 aws_state, U32 aws_role);
void stream_function_hearing_aid_set_dl_state(bool *dl_state);
void stream_function_hearing_aid_trial_run(U32 event, U32 data_len, void *data);
void stream_function_hearing_aid_deinitialize(void);
bool stream_function_hearing_aid_load_nvkey(U16 nvkey_id, void *nvkey);
bool stream_function_hearing_aid_initialize(void *para);
bool stream_function_hearing_aid_process(void *para);
void stream_function_hearing_aid_feedback_detect_start();
void stream_function_hearing_aid_feedback_detect_notify(S32 data);
void stream_function_hearing_aid_set_mic_cal_mode(U8 mic_cal_mode);
int stream_function_hearing_aid_get_mic_cal_data(void *data);

#endif /* _HEARING_AID_INTERFACE_H_ */

