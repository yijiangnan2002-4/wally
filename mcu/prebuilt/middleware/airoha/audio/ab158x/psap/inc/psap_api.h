/* Copyright Statement:
 *
 * (C) 2020  Airoha Technology Corp. All rights reserved.
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

#ifndef __PSAP_API_H__
#define __PSAP_API_H__

#include "types.h"
#include "stdbool.h"
//#include "hal_audio.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef UNUSED
#define UNUSED(x)  ((void)(x))
#endif

/** @brief The atci cmd source of the audio_psap_control_command_handler. */
#define AUDIO_PSAP_CONTROL_SOURCE_FROM_ATCI         (1)
#define AUDIO_PSAP_CONTROL_SOURCE_FROM_AM           (2)

/** @brief The source of the audio_psap_control_command_handler. */
typedef uint8_t audio_psap_control_user_type_t;


/** @brief This enum defines the audio psap return types */
typedef enum {
    AUDIO_PSAP_STATUS_NONE = -2,
    AUDIO_PSAP_STATUS_FAIL = -1,    /**< The psap func return fail.   */
    AUDIO_PSAP_STATUS_SUCCESS = 0,   /**< The psap func return success.   */
    AUDIO_PSAP_STATUS_CANCEL = 1,   /**< The psap func return cancel.   */
} audio_psap_status_t;

/** @brief psap control event values. */
typedef enum {
    AUDIO_PSAP_CONTROL_EVENT_ON                 = 1 << 0,  /**< The event to enable psap.   */
    AUDIO_PSAP_CONTROL_EVENT_OFF                = 1 << 1,  /**< The event to disable psap.   */
    AUDIO_PSAP_CONTROL_EVENT_SUSPEND            = 1 << 2,  /**< The event to suspend psap.   */
    AUDIO_PSAP_CONTROL_EVENT_RESUME             = 1 << 3,  /**< The event to resume psap.   */
    AUDIO_PSAP_CONTROL_EVENT_RUNTIME_CONFIG     = 1 << 4,  /**< The event to config psap.   */
    AUDIO_PSAP_CONTROL_EVENT_RESET              = 1 << 5,  /**< The event to config psap.   */
} audio_psap_control_event_t;

/** @brief This enum defines the audio psap subset */
typedef enum {
    AUDIO_PSAP_TYPE_HEARING_AID                 = 0,
    AUDIO_PSAP_TYPE_ALL,
    AUDIO_PSAP_TYPE_DUMMY                       = 0xFFFFFFFF,
} audio_psap_type_t;

/** @brief This enum defines the scenario event */
typedef enum {
    /*UL */
    AUDIO_PSAP_SCENARIO_CHANGE_UL_CALL    = 0,
    AUDIO_PSAP_SCENARIO_CHANGE_UL_VA      = 1,
    AUDIO_PSAP_SCENARIO_CHANGE_UL_MAX     = AUDIO_PSAP_SCENARIO_CHANGE_UL_VA,

    /*DL*/
    AUDIO_PSAP_SCENARIO_CHANGE_DL_VP      = 8,
    AUDIO_PSAP_SCENARIO_CHANGE_DL_A2DP    = 9,
    AUDIO_PSAP_SCENARIO_CHANGE_DL_CALL    = 10,
    AUDIO_PSAP_SCENARIO_CHANGE_DL_MAX     = AUDIO_PSAP_SCENARIO_CHANGE_DL_CALL,

} audio_psap_scenario_change_event_t;

/** @brief This function send open/close stream request*/
typedef audio_psap_status_t (*stream_handler_t)(bool enable);

/** @brief This structure support the audio psap path setting */
typedef struct {
    audio_psap_type_t type;
    U8 sub_mode;
    U32 frame_length;
    U32 buffer_length;
    stream_handler_t stream_handler;
    S32 delay_time; //ms
    U32 param;
} audio_psap_control_cap_t;

/** @brief This enum defines the audio psap running status */
typedef enum {
    AUDIO_PSAP_RUNNING_STATUS_START = 0,
    AUDIO_PSAP_RUNNING_STATUS_RUNNING,
    AUDIO_PSAP_RUNNING_STATUS_RESUME,
    AUDIO_PSAP_RUNNING_STATUS_SUSPEND,
    AUDIO_PSAP_RUNNING_STATUS_RESET,
    AUDIO_PSAP_RUNNING_STATUS_STOP,
    AUDIO_PSAP_RUNNING_STATUS_CLOSE,
    AUDIO_PSAP_RUNNING_STATUS_INCASE_SUSPEND,
} audio_psap_running_status_t;


extern void audio_psap_control_init(void);
extern audio_psap_status_t audio_psap_control_command_handler(audio_psap_control_user_type_t source, audio_psap_control_event_t event, void* command);
extern void audio_psap_control_get_status(audio_psap_running_status_t *running, audio_psap_type_t *type, U8 *sub_mode, void *misc);
audio_psap_status_t audio_psap_control_suspend(void);
audio_psap_status_t audio_psap_control_resume(void);
bool audio_psap_control_query_share_buffer(void);


#if defined(AIR_HEARING_AID_ENABLE)
#ifndef PACKED
#define PACKED  __attribute__((packed))
#endif
#define HA_BAND_NUM    (50)

typedef enum {
    HA_DEVICE_ROLE_LEFT  = 0,
    HA_DEVICE_ROLE_RIGHT = 1,

    HA_DEVICE_ROLE_MAX = 0xFFFFFFFF
} ha_device_role_t;

typedef struct {
    U8 MFA_switch_L       : 1;
    U8 MFA_switch_R       : 1;
    U8 LowCut_switch_L    : 1;
    U8 LowCut_switch_R    : 1;
    U8 NR_switch          : 1;
    U8 BeamForming_switch : 1;
    U8 reserved_1         : 2;
    U8 NR_level;
    U8 reserved_2;
} PACKED ha_mode_table_t;

typedef struct {
    U8 AEA_switch            : 1;
    U8 AEA_NR_switch         : 1;
    U8 AEA_NR_level          : 2;
    U8 reserved_13           : 4;
    U8 AEA_det_period;
    U8 reserved_14;
} PACKED ha_aea_config_t;

typedef struct {
    U8 BF_switch             : 1;
    U8 BF_switch_mode_ctrl   : 1;
    U8 reserved_20           : 6;
} PACKED ha_bf_config_t;

typedef struct {
    U8 afc_ctrl_switch_l     : 1;
    U8 afc_ctrl_switch_r     : 1;
    U8 reserved              : 6;
} PACKED ha_afc_config_t;

typedef struct {
    U8 INR_switch_l          : 1;
    U8 INR_sensitivity_l     : 4;
    U8 reserved_15           : 3;
    U8 INR_strength_l        : 2;
    U8 reserved_16           : 6;
    U8 INR_switch_r          : 1;
    U8 INR_sensitivity_r     : 4;
    U8 reserved_17           : 3;
    U8 INR_strength_r        : 2;
    U8 reserved_18           : 6;
} PACKED ha_inr_config_t;

typedef struct {
    U8 ha_user_eq_switch_l   : 1;
    U8 ha_user_eq_switch_r   : 1;
    U8 reserved_8            : 6;
} PACKED ha_user_eq_switch_t;

typedef struct {
    S8 ha_user_eq_overall_l;
    S8 ha_user_eq_band_l[HA_BAND_NUM];
    S8 ha_user_eq_overall_r;
    S8 ha_user_eq_band_r[HA_BAND_NUM];
} PACKED ha_usr_eq_para_t;

typedef struct {
    U8 test_spk_ref_L_64;
    U8 test_spk_ref_L_125;
    U8 test_spk_ref_L_250;
    U8 test_spk_ref_L_500;
    U8 test_spk_ref_L_1000;
    U8 test_spk_ref_L_2000;
    U8 test_spk_ref_L_4000;
    U8 test_spk_ref_L_6000;
    U8 test_spk_ref_L_8000;
    U8 test_spk_ref_L_12000;
    U8 test_spk_ref_R_64;
    U8 test_spk_ref_R_125;
    U8 test_spk_ref_R_250;
    U8 test_spk_ref_R_500;
    U8 test_spk_ref_R_1000;
    U8 test_spk_ref_R_2000;
    U8 test_spk_ref_R_4000;
    U8 test_spk_ref_R_6000;
    U8 test_spk_ref_R_8000;
    U8 test_spk_ref_R_12000;
} PACKED ha_test_spk_ref_t;


typedef struct {
    U8 a2dp_mix_mode_switch  : 1;                          /**< @Value 0 @Desc 1 */
    U8 reserved_3            : 2;                          /**< @Value 0 @Desc 1 */
    U8 a2dp_drc_switch_l     : 1;                          /**< @Value 0 @Desc 1 */
    U8 a2dp_drc_switch_r     : 1;                          /**< @Value 0 @Desc 1 */
    U8 a2dp_mfa_switch_l     : 1;                          /**< @Value 0 @Desc 1 */
    U8 a2dp_mfa_switch_r     : 1;                          /**< @Value 0 @Desc 1 */
    U8 reserved_4            : 1;                          /**< @Value 0 @Desc 1 */
    S8 a2dp_mix_mode_HA_gain_l;                            /**< @Value 0 @Desc 1 */
    S8 a2dp_mix_mode_HA_gain_r;                            /**< @Value 0 @Desc 1 */

    U8 sco_mix_mode_switch   : 1;                          /**< @Value 0 @Desc 1 */
    U8 sco_drc_switch_l      : 1;                          /**< @Value 0 @Desc 1 */
    U8 sco_drc_switch_r      : 1;                          /**< @Value 0 @Desc 1 */
    U8 sco_mfa_switch_l      : 1;                          /**< @Value 0 @Desc 1 */
    U8 sco_mfa_switch_r      : 1;                          /**< @Value 0 @Desc 1 */
    U8 reserved_5            : 3;                          /**< @Value 0 @Desc 1 */
    S8 sco_mix_mode_HA_gain_l;                             /**< @Value 0 @Desc 1 */
    S8 sco_mix_mode_HA_gain_r;                             /**< @Value 0 @Desc 1 */

    U8 vp_mix_mode_switch   : 1;                           /**< @Value 0 @Desc 1 */
    U8 vp_drc_switch_l      : 1;                           /**< @Value 0 @Desc 1 */
    U8 vp_drc_switch_r      : 1;                           /**< @Value 0 @Desc 1 */
    U8 vp_mfa_switch_l      : 1;                           /**< @Value 0 @Desc 1 */
    U8 vp_mfa_switch_r      : 1;                           /**< @Value 0 @Desc 1 */
    U8 reserved_6           : 3;                           /**< @Value 0 @Desc 1 */
    S8 vp_mix_mode_HA_gain_l;                              /**< @Value 0 @Desc 1 */
    S8 vp_mix_mode_HA_gain_r;                              /**< @Value 0 @Desc 1 */

} PACKED ha_scenario_mix_mode_t;

typedef struct {
    U8 how_det_enable;                                     /**< @Value 1 @Desc 1 */
    S16 SUP_HAN;                                           /**< @Value 2048 @Desc 1 */
} PACKED ha_how_det_t;

typedef struct {
    U8 drc_MPO_adjust_switch : 1;                          /**< @Value 0 @Desc 1 */
    S8 drc_MPO_adjust_value  : 7;                          /**< @Value 0 @Desc 1 */
} PACKED ha_mpo_adjust_t;

typedef enum {
    HA_NOTI_EVENT_FB_DETECT       = 0,
    HA_NOTI_EVENT_AEA_CHANGE_MODE = 1,
    HA_NOTI_EVENT_AEA_SYNC_DATA   = 2,
    HA_NOTI_EVENT_QUERY_AWS_INFO = 3,
    HA_NOTI_EVENT_NUM,
} ha_noti_event_t;

typedef void (*ha_noti_callback_t)(ha_noti_event_t event, uint8_t *extra_data, uint16_t extra_data_len);

extern audio_psap_status_t audio_psap_ha_control_init(void);
extern audio_psap_status_t audio_psap_ha_control_open_framework(void);
extern audio_psap_status_t audio_psap_ha_control_close_framework(void);
extern audio_psap_status_t audio_psap_ha_control_reset_framework(void);
extern audio_psap_status_t audio_psap_ha_control_enable(void);
extern audio_psap_status_t audio_psap_ha_control_disable(void);
extern audio_psap_status_t audio_psap_ha_control_get_status(U8 *enable);
extern audio_psap_status_t audio_psap_ha_control_set_usr_switch(bool enable);
extern audio_psap_status_t audio_psap_ha_control_get_usr_switch(bool *enable);
extern audio_psap_status_t audio_psap_ha_control_suspend(void);
extern audio_psap_status_t audio_psap_ha_control_resume(void);
extern audio_psap_status_t audio_psap_ha_control_set_level_index(U8 level_index_L, U8 level_index_R, ha_device_role_t role);
extern audio_psap_status_t audio_psap_ha_control_get_level_index(U8 *level_index_L, U8 *level_index_R);
extern audio_psap_status_t audio_psap_ha_control_set_level_sync_switch(U8 enable);
extern audio_psap_status_t audio_psap_ha_control_get_level_sync_switch(U8 *enable);
extern audio_psap_status_t audio_psap_ha_control_get_level_mode_max_count(U8 *level_max_cnt, U8 *mode_max_cnt, U8 *vol_max_cnt);
extern audio_psap_status_t audio_psap_ha_control_set_volume_index(U8 vol_index_L, U8 vol_index_R, ha_device_role_t role);
extern audio_psap_status_t audio_psap_ha_control_get_volume_index(U8 *vol_index_L, U8 *vol_index_R);
extern audio_psap_status_t audio_psap_ha_control_set_volume_sync_switch(U8 enable);
extern audio_psap_status_t audio_psap_ha_control_get_volume_sync_switch(U8 *enable);
extern audio_psap_status_t audio_psap_ha_control_set_mode_index(U8 mode_index);
extern audio_psap_status_t audio_psap_ha_control_get_mode_index(U8 *mode_index);
extern audio_psap_status_t audio_psap_ha_control_set_specific_mode_table(U8 mode_index, ha_mode_table_t *mode_para, ha_device_role_t role);
extern audio_psap_status_t audio_psap_ha_control_get_specific_mode_table(U8 mode_index, ha_mode_table_t *mode_para);
extern audio_psap_status_t audio_psap_ha_control_set_aea_configuration(ha_aea_config_t *aea_config);
extern audio_psap_status_t audio_psap_ha_control_get_aea_configuration(ha_aea_config_t *aea_config);
extern audio_psap_status_t audio_psap_ha_control_set_wnr_switch(U8 enable);
extern audio_psap_status_t audio_psap_ha_control_get_wnr_switch(U8 *enable);
extern audio_psap_status_t audio_psap_ha_control_set_beamforming_setting(ha_bf_config_t *BF_config);
extern audio_psap_status_t audio_psap_ha_control_get_beamforming_setting(ha_bf_config_t *BF_config);
extern audio_psap_status_t audio_psap_ha_control_set_afc_configuration(ha_afc_config_t *afc_config, ha_device_role_t role);
extern audio_psap_status_t audio_psap_ha_control_get_afc_configuration(ha_afc_config_t *afc_config);
extern audio_psap_status_t audio_psap_ha_control_set_inr_configuration(ha_inr_config_t *INR_config, ha_device_role_t role);
extern audio_psap_status_t audio_psap_ha_control_get_inr_configuration(ha_inr_config_t *INR_config);
extern audio_psap_status_t audio_psap_ha_control_set_user_eq_switch(ha_user_eq_switch_t *usr_eq_switch, ha_device_role_t role);
extern audio_psap_status_t audio_psap_ha_control_get_user_eq_switch(ha_user_eq_switch_t *usr_eq_switch);
extern audio_psap_status_t audio_psap_ha_control_set_user_eq_gain(ha_usr_eq_para_t *usr_eq_gain, ha_device_role_t role);
extern audio_psap_status_t audio_psap_ha_control_get_user_eq_gain(ha_usr_eq_para_t *usr_eq_gain);
extern audio_psap_status_t audio_psap_ha_control_get_speaker_reference(ha_test_spk_ref_t *spk_ref);
extern audio_psap_status_t audio_psap_ha_control_set_puretone_generator(U8 enable_l, U16 freq_l, S8 dBFS_l, U8 enable_r, U16 freq_r, S8 dBFS_r);
extern audio_psap_status_t audio_psap_ha_control_get_puretone_generator(U8 *enable_l, U16 *freq_l, S8 *dBFS_l, U8 *enable_r, U16 *freq_r, S8 *dBFS_r);
extern audio_psap_status_t audio_psap_ha_control_set_mix_mode(ha_scenario_mix_mode_t *mix_mode_para, ha_device_role_t role);
extern audio_psap_status_t audio_psap_ha_control_get_mix_mode(ha_scenario_mix_mode_t *mix_mode_para);
extern audio_psap_status_t audio_psap_ha_control_set_tuning_mode(U8 mode_switch);//b0: Switch(L)    b1: Switch(R)    b2~b7: Reserved
extern audio_psap_status_t audio_psap_ha_control_get_tuning_mode(U8 *mode_switch);//b0: Switch(L)    b1: Switch(R)    b2~b7: Reserved
extern audio_psap_status_t audio_psap_ha_control_set_mp_test_mode(U8 mode_switch);//0x00(Off), 0x03(On)
extern audio_psap_status_t audio_psap_ha_control_get_mp_test_mode(U8 *mode_switch);//0x00(Off), 0x03(On)
extern audio_psap_status_t audio_psap_ha_control_set_restore_setting(U8 mode);
extern audio_psap_status_t audio_psap_ha_control_detect_feedback(void);
extern audio_psap_status_t audio_psap_ha_control_set_mute(U8 mute_l, U8 mute_r);
extern audio_psap_status_t audio_psap_ha_control_get_mute(U8 *mute_l, U8 *mute_r);
extern audio_psap_status_t audio_psap_ha_control_set_howling_detection(ha_how_det_t *how_det);
extern audio_psap_status_t audio_psap_ha_control_get_howling_detection(ha_how_det_t *how_det);
extern audio_psap_status_t audio_psap_ha_control_set_mpo_adjustment(ha_mpo_adjust_t *mpo_adj);
extern audio_psap_status_t audio_psap_ha_control_get_mpo_adjustment(ha_mpo_adjust_t *mpo_adj);
extern audio_psap_status_t audio_psap_ha_control_set_mic_channel(U8 channel);
extern audio_psap_status_t audio_psap_ha_control_get_mic_channel(U8 *channel);
extern audio_psap_status_t audio_psap_ha_control_set_inear_detect_switch(U8 enable);
extern audio_psap_status_t audio_psap_ha_control_get_inear_detect_switch(U8 *enable);
extern audio_psap_status_t audio_psap_ha_control_save_customer_setting(void);
extern void audio_psap_ha_control_set_runtime_sync_parameter(uint16_t len, uint8_t* para);
extern uint8_t* audio_psap_ha_control_get_runtime_sync_parameter(uint16_t *len);
extern audio_psap_status_t audio_psap_ha_control_set_passthrough_swtich(U8 enable_l, U8 enable_r);
extern audio_psap_status_t audio_psap_ha_control_get_passthrough_swtich(U8 *enable_l, U8 *enable_r);
extern audio_psap_status_t audio_psap_ha_control_register_notification_callback(ha_noti_callback_t callback);
extern void audio_psap_ha_control_senario_notification(audio_psap_scenario_change_event_t scenario, bool is_on);
#endif

#ifdef __cplusplus
}
#endif
#endif  /*__PSAP_API_H__*/

