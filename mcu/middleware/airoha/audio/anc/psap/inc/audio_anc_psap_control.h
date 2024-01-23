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

#ifndef __AUDIO_ANC_PSAP_CONTROL_H__
#define __AUDIO_ANC_PSAP_CONTROL_H__

#include "types.h"
#include "stdbool.h"
#include "audio_log.h"
#include "audio_anc_llf_control_internal.h"
#include "audio_anc_psap_control_internal.h"


/**
 * @addtogroup Audio
 * @{
 * @addtogroup PSAP
 * @{
 *
 * The PSAP is used for Personal Sound Amplification Products. The HA is used for Hearing Aid. The PSAP is a scaled-down version of HA.
 *
 * @section psap_api_usage How to use this module
 *
 * - An example on how to use the PSAP APIs.
 *   - Step1: Call audio_anc_psap_control_init() to initialize the PSAP module.
 *   - Step2: Call llf_control_register_callback() to register the callback for PSAP OPEN/CLOSE event.
 *   - Step3: Call audio_anc_control_enable() to open the PSAP path.
 *   - Step4: Wait for PSAP OPEN event callback.
 *   - Step5: Call audio_anc_psap_control_enable() to enable the mic-in path.
 *   - Step6: Call another effect API to change the effect.
 *   - Step7: Call audio_anc_psap_control_disable() and audio_anc_control_disable() to close the PSAP path.
 *   - Sample code:
 *   @code
 *
 *
 *   audio_anc_psap_control_init();//Initialize PSAP module.
 *   llf_control_register_callback((llf_control_callback_t)app_hear_through_control_callback,
 *                                       LLF_CONTROL_EVENT_ON | LLF_CONTROL_EVENT_OFF,
 *                                       LLF_MAX_CALLBACK_LEVEL_ALL);
 *
 *
 *   { //Open the PSAP framework
 *             U32 mic_config;
 *             audio_anc_psap_control_get_mic_input_path(&mic_config);
 *
 *             audio_anc_control_result_t         control_ret;
 *             audio_anc_control_filter_id_t      target_filter_id;
 *             audio_anc_control_type_t           target_anc_type;
 *             audio_anc_control_misc_t           local_misc = {0};
 *             local_misc.type_mask_param.ANC_path_mask = AUDIO_ANC_CONTROL_RAMP_FF_L |
 *                                                        AUDIO_ANC_CONTROL_RAMP_FB_L; //PSAP with hybrid ANC effect
 *             target_anc_type      = AUDIO_ANC_CONTROL_TYPE_PT_HA_PSAP | mic_config;
 *
 *             target_filter_id     = AUDIO_ANC_CONTROL_HA_PSAP_FILTER_DEFAULT; //1~4
 *             control_ret = audio_anc_control_enable(target_filter_id, target_anc_type, &local_misc);
 *   }
 *
 *   void app_hear_through_control_callback(llf_type_t type, llf_control_event_t event, llf_status_t result)
 *   {
 *      if ((type == AUDIO_LLF_TYPE_HEARING_AID) && (event == LLF_CONTROL_EVENT_ON))
 *      {
 *          audio_anc_psap_control_enable();//Enable the mic-in path.
 *          audio_anc_psap_control_set_level_index(1, 2, AUDIO_PSAP_DEVICE_ROLE_LEFT);//set the level index for left & right device, and tell PSAP module which value to apply (L or R).
 *      }
 *   }
 *
 *
 *   audio_anc_psap_control_disable(); //Disable the mic-in path.
 *   audio_anc_control_disable(NULL); //Close the PSAP framework
 *   @endcode
 *
 */

#ifdef __cplusplus
extern "C" {
#endif


/** @defgroup psap_define Define
  * @{
  */

/** @brief The shorthand of attribute((packed)), means the format is not 4-bytes align. */
#ifndef PACKED
#define PACKED  __attribute__((packed))
#endif

/** @brief The band number of user's EQ. */
#define HA_BAND_NUM    (50)

/** @brief This defines the audio psap return types */
#define    AUDIO_PSAP_STATUS_NONE      (LLF_STATUS_NONE)
#define    AUDIO_PSAP_STATUS_FAIL      (LLF_STATUS_FAIL)    /**< The psap func return failed.   */
#define    AUDIO_PSAP_STATUS_SUCCESS   (LLF_STATUS_SUCCESS)   /**< The psap func return was successful.   */
#define    AUDIO_PSAP_STATUS_CANCEL    (LLF_STATUS_CANCEL)   /**< The psap func return was cancelled.   */

/**
 * @}
 */

/** @defgroup psap_enum Enum
 *  @{
 */

/** @brief This enum defines the current role */
typedef enum {
    AUDIO_PSAP_DEVICE_ROLE_LEFT  = 0,
    AUDIO_PSAP_DEVICE_ROLE_RIGHT = 1,

    AUDIO_PSAP_DEVICE_ROLE_MAX = 0xFFFFFFFF
} audio_psap_device_role_t;

/** @brief This enum defines the notification from the PSAP algorithm */
typedef enum {
    PSAP_NOTI_EVENT_FB_DETECT       = 0,
    PSAP_NOTI_EVENT_AEA_CHANGE_MODE = 1,
    PSAP_NOTI_EVENT_AEA_SYNC_DATA   = 2,
    PSAP_NOTI_EVENT_QUERY_AWS_INFO  = 3,
    PSAP_NOTI_EVENT_NUM,
} psap_noti_event_t;


/**
 * @}
 */

/** @defgroup psap_struct Struct
  * @{
  */

/** @brief The enviroment mode parameters structure*/
typedef struct {
    U8 mfa_switch_l       : 1;              /**< Reserved. This parameter is not working.    */
    U8 mfa_switch_r       : 1;              /**< Reserved. This parameter is not working.    */
    U8 lowcut_switch_l    : 1;              /**< Low-cut switch for left device to apply low-cut gain. 0(off), 1(on).    */
    U8 lowcut_switch_r    : 1;              /**< Low-cut switch for right device to apply low-cut gain. 0(off), 1(on).    */
    U8 nr_switch          : 1;              /**< NR (Noise reduction) function switch. NR reduces stable background noise. 0(off), 1(on).    */
    U8 beamforming_switch : 1;              /**< Beamforming function switch. This control item is not used when "Mode Table Control Beamforming Switch" is off.  0(off), 1(on).   */
    U8 reserved_1         : 2;              /**< Reserved.    */
    U8 nr_level;                            /**< NR (Noise reduction) level with range [0, 31].   */
    U8 reserved_2;                          /**< Reserved.    */
} PACKED psap_mode_table_t;

/** @brief The auto environment adjustment configuration structure*/
typedef struct {
    U8 aea_switch            : 1;           /**< Auto environment adjustment switch. 0(off), 1(on).   */
    U8 aea_nr_switch         : 1;           /**< Whether AEA auto-change NR level is enabled. 0(off), 1(on).   */
    U8 aea_nr_level          : 2;           /**< The NR level when AEA is switched on; The range is [0, 3].   */
    U8 reserved_13           : 4;           /**< Reserved.   */
    U8 aea_det_period;                      /**< The triggered time of AEA to detect the environment. The range is [4, 255].   */
    U8 reserved_14;                         /**< Reserved.    */
} PACKED psap_aea_config_t;

/** @brief The beamforming configuration structure*/
typedef struct {
    U8 bf_switch             : 1;           /**< Beamforming switch. It is not used when "Mode Table Control Beamforming Switch" is on. 0(off), 1(on).   */
    U8 bf_switch_mode_ctrl   : 1;           /**< Mode Table Control Beamforming Switch [0(off), 1(on)]. If 1(on), the beamforming function switch is controlled by Beamforming switch in the mode table.   */
    U8 reserved_20           : 6;           /**< Reserved.    */
} PACKED psap_bf_config_t;

/** @brief The acoustic feedback cancellation configuration structure*/
typedef struct {
    U8 afc_ctrl_switch_l     : 1;           /**< Acoustic feedback cancellation switch for left device. 0(off), 1(on).    */
    U8 afc_ctrl_switch_r     : 1;           /**< Acoustic feedback cancellation switch for right device. 0(off), 1(on).    */
    U8 reserved              : 6;           /**< Reserved.    */
} PACKED psap_afc_config_t;

/** @brief The impulse noise reduction configuration structure*/
typedef struct {
    U8 inr_switch_l          : 1;           /**< Impulse noise reduction switch for left device. 0(off), 1(on).    */
    U8 inr_sensitivity_l     : 4;           /**< The threshold to trigger INR for left device. The more sensitivity, the easier INR will be triggered. The range is [0, 15].    */
    U8 reserved_15           : 3;           /**< Reserved.    */
    U8 inr_strength_l        : 2;           /**< Reduces the amount of impulse noise suppression for the left device. [0, 1, 2, 3] = [-10dB, -15dB, -20dB, -30dB].    */
    U8 reserved_16           : 6;           /**< Reserved.    */
    U8 inr_switch_r          : 1;           /**< Impulse noise reduction switch for right device. 0(off), 1(on).    */
    U8 inr_sensitivity_r     : 4;           /**< The threshold to trigger INR for right device. The more sensitivity, the easier INR will be triggered. The range is [0, 15].    */
    U8 reserved_17           : 3;           /**< Reserved.    */
    U8 inr_strength_r        : 2;           /**< Reduces the amount of impulse noise suppression for the right device. [0, 1, 2, 3] = [-10dB, -15dB, -20dB, -30dB].    */
    U8 reserved_18           : 6;           /**< Reserved.    */
} PACKED ha_inr_config_t;

/** @brief The user EQ switch structure*/
typedef struct {
    U8 psap_user_eq_switch_l   : 1;           /**< User EQ switch for left device. 0(off), 1(on).    */
    U8 psap_user_eq_switch_r   : 1;           /**< User EQ switch for right device. 0(off), 1(on).    */
    U8 reserved_8            : 6;           /**< Reserved.    */
} PACKED psap_user_eq_switch_t;

/** @brief The user EQ parameters structure*/
typedef struct {
    S8 psap_user_eq_overall_l;                /**< Gain setting of left device for all bands. Setting range is [-32, 32].    */
    S8 psap_user_eq_band_l[HA_BAND_NUM];      /**< Gain setting of left device for each band. Setting range is [-32, 32].    */
    S8 psap_user_eq_overall_r;                /**< Gain setting of right device for all bands. Setting range is [-32, 32].    */
    S8 psap_user_eq_band_r[HA_BAND_NUM];      /**< Gain setting of right device for each band. Setting range is [-32, 32].    */
} PACKED psap_usr_eq_para_t;

/** @brief The speaker reference is speaker max output sound pressure for the mobile APP used for the hearing test*/
typedef struct {
    U8 test_spk_ref_L_64;                   /**< The speaker max output sound pressure at 64Hz for left device.    */
    U8 test_spk_ref_L_125;                  /**< The speaker max output sound pressure at 125Hz for left device.    */
    U8 test_spk_ref_L_250;                  /**< The speaker max output sound pressure at 250Hz for left device.    */
    U8 test_spk_ref_L_500;                  /**< The speaker max output sound pressure at 500Hz for left device.    */
    U8 test_spk_ref_L_1000;                 /**< The speaker max output sound pressure at 1000Hz for left device.    */
    U8 test_spk_ref_L_2000;                 /**< The speaker max output sound pressure at 2000Hz for left device.    */
    U8 test_spk_ref_L_4000;                 /**< The speaker max output sound pressure at 4000Hz for left device.    */
    U8 test_spk_ref_L_6000;                 /**< The speaker max output sound pressure at 6000Hz for left device.    */
    U8 test_spk_ref_L_8000;                 /**< The speaker max output sound pressure at 8000Hz for left device.    */
    U8 test_spk_ref_L_12000;                /**< The speaker max output sound pressure at 12000Hz for left device.    */
    U8 test_spk_ref_R_64;                   /**< The speaker max output sound pressure at 64Hz for right device.    */
    U8 test_spk_ref_R_125;                  /**< The speaker max output sound pressure at 125Hz for right device.    */
    U8 test_spk_ref_R_250;                  /**< The speaker max output sound pressure at 250Hz for right device.    */
    U8 test_spk_ref_R_500;                  /**< The speaker max output sound pressure at 500Hz for right device.    */
    U8 test_spk_ref_R_1000;                 /**< The speaker max output sound pressure at 1000Hz for right device.    */
    U8 test_spk_ref_R_2000;                 /**< The speaker max output sound pressure at 2000Hz for right device.    */
    U8 test_spk_ref_R_4000;                 /**< The speaker max output sound pressure at 4000Hz for right device.    */
    U8 test_spk_ref_R_6000;                 /**< The speaker max output sound pressure at 6000Hz for right device.    */
    U8 test_spk_ref_R_8000;                 /**< The speaker max output sound pressure at 8000Hz for right device.    */
    U8 test_spk_ref_R_12000;                /**< The speaker max output sound pressure at 12000Hz for right device.    */
} PACKED psap_test_spk_ref_t;

/** @brief The configuration for the PSAP behavior for all audio paths (e.g., A2DP, eSCO, and voice prompt)*/
typedef struct {
    U8 a2dp_mix_mode_switch  : 1;
    U8 reserved_3            : 2;                          /**< Reserved.    */
    U8 a2dp_drc_switch_l     : 1;                          /**< The A2DP DRC switch for left device. 0(off), 1(on).    */
    U8 a2dp_drc_switch_r     : 1;                          /**< The A2DP DRC switch for right device. 0(off), 1(on).    */
    U8 a2dp_mfa_switch_l     : 1;                          /**< Reserved. This parameter is not working.    */
    U8 a2dp_mfa_switch_r     : 1;                          /**< Reserved. This parameter is not working.    */
    U8 reserved_4            : 1;                          /**< Reserved.    */
    S8 a2dp_mix_mode_psap_gain_l;                            /**< The additional gain for mic signal which is mixed into A2DP signal for the left device. The range is [-12, 12].    */
    S8 a2dp_mix_mode_psap_gain_r;                            /**< The additional gain for mic signal which is mixed into A2DP signal for the right device. The range is [-12, 12].    */

    U8 sco_mix_mode_switch   : 1;                          /**< Reserved.    */
    U8 sco_drc_switch_l      : 1;                          /**< The SCO DRC switch for left device. 0(off), 1(on).    */
    U8 sco_drc_switch_r      : 1;                          /**< The SCO DRC switch for right device. 0(off), 1(on).    */
    U8 sco_mfa_switch_l      : 1;                          /**< Reserved. This parameter is not working.    */
    U8 sco_mfa_switch_r      : 1;                          /**< Reserved. This parameter is not working.    */
    U8 reserved_5            : 3;                          /**< Reserved.    */
    S8 sco_mix_mode_psap_gain_l;                             /**< The additional gain for mic signal which is mixed into SCO signal for the left device. The range is [-12, 12].    */
    S8 sco_mix_mode_psap_gain_r;                             /**< The additional gain for mic signal which is mixed into SCO signal for the right device. The range is [-12, 12].    */

    U8 vp_mix_mode_switch   : 1;                           /**< Reserved.    */
    U8 vp_drc_switch_l      : 1;                           /**< The VP DRC switch for left device. 0(off), 1(on).    */
    U8 vp_drc_switch_r      : 1;                           /**< The VP DRC switch for right device. 0(off), 1(on).    */
    U8 vp_mfa_switch_l      : 1;                           /**< Reserved. This parameter is not working.    */
    U8 vp_mfa_switch_r      : 1;                           /**< Reserved. This parameter is not working.    */
    U8 reserved_6           : 3;                           /**< Reserved.    */
    S8 vp_mix_mode_psap_gain_l;                              /**< The additional gain for mic signal which is mixed into VP signal for the left device. The range is [-12, 12].    */
    S8 vp_mix_mode_psap_gain_r;                              /**< The additional gain for mic signal which is mixed into VP signal for the right device. The range is [-12, 12].    */

} PACKED psap_scenario_mix_mode_t;


/** @brief The howling detection parameters structure*/
typedef struct {
    U8  how_det_enable;                                    /**< The howling detection switch. 0(off), 1(on).    */
    S16 sup_han;                                           /**< The Suppression handover frame number. The range is [1, 327667].   */
} PACKED psap_how_det_t;
/**
 * @}
 */

/** @defgroup psap_typedef Typedef
  * @{
  */

/** @brief This notification is from the PSAP algorithm */
typedef void (*psap_noti_callback_t)(psap_noti_event_t event, uint8_t *extra_data, uint16_t extra_data_len);

/**
  * @}
  */

/* ======================================================
/          Low Latency Framework API
========================================================*/
/**
 * @brief     This function is used to register the callback for PSAP OPEN & CLOSE events.
 * @param[in] callback is the callback function pointer.
 * @param[in] event_mask is to select which event needs callback.
 * @param[in] level is the level mask in LLF callback service.
 * @return    Return LLF_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
extern llf_status_t llf_control_register_callback(llf_control_callback_t callback, llf_control_event_t event_mask, llf_control_callback_level_t level);

/* ======================================================
/          PSAP API
========================================================*/

/**
 * @brief 	This function is used to initialize the PSAP module.
 * @return  Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_psap_status_t audio_anc_psap_control_init(void);

/**
 * @brief     This function is used to enable the mic-in path.
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_psap_status_t audio_anc_psap_control_enable(void);

/**
 * @brief     This function is used to disable the mic-in path.
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_psap_status_t audio_anc_psap_control_disable(void);

/**
 * @brief     This function is used to get the PSAP mic path configuration.
 * @param[out] *sel would be the mask for the mic path configuration.
 */
void audio_anc_psap_control_get_mic_input_path(U32 *sel);

/**
 * @brief     This function is used to get the current PSAP status.
  * @param[out] *enable would be the current status.
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_psap_status_t audio_anc_psap_control_get_status(bool *enable);

/**
 * @brief     This function is used to suspend the PSAP path.
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_psap_status_t audio_anc_psap_control_suspend(void);

/**
 * @brief     This function is used to resume the PSAP path and re-load the parameters saved in flash.
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_psap_status_t audio_anc_psap_control_resume(void);

/**
 * @brief     This function is used to select the level of hearing loss.
 * @param[in] level_index_L is the level of left device. The level index must be less than the maximum level number.
 * @param[in] level_index_R is the level of right device. The level index must be less than the maximum level number.
 * @param[in] role is used to decide which value to apply (level_index_L or level_index_R).
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_psap_status_t audio_anc_psap_control_set_level_index(U8 level_index_L, U8 level_index_R, audio_psap_device_role_t role);

/**
 * @brief     This function is used to get the level of hearing loss.
 * @param[out] *level_index_L is the level of left device.
 * @param[out] *level_index_R is the level of right device.
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_psap_status_t audio_anc_psap_control_get_level_index(U8 *level_index_L, U8 *level_index_R);

/**
 * @brief     This function is used to get the maximum level/mode/volume number.
 * @param[out] *level_max_cnt is the maximum level number.
 * @param[out] *mode_max_cnt is the maximum mode number.
 * @param[out] *vol_max_cnt is the maximum volume number.
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_psap_status_t audio_anc_psap_control_get_level_mode_max_count(U8 *level_max_cnt, U8 *mode_max_cnt, U8 *vol_max_cnt);

/**
 * @brief     This function is used to set the volume index.
 * @param[in] vol_index_L is the volume of left device. The volume index must be less than the maximum volume number.
 * @param[in] vol_index_R is the volume of right device. The volume index must be less than the maximum volume number.
 * @param[in] role is used to decide which value to apply (vol_index_L or vol_index_R).
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_psap_status_t audio_anc_psap_control_set_volume_index(U8 vol_index_L, U8 vol_index_R, audio_psap_device_role_t role);

/**
 * @brief     This function is used to get the volume index.
 * @param[out] *vol_index_L is the volume of left device.
 * @param[out] *vol_index_R is the volume of right device.
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_psap_status_t audio_anc_psap_control_get_volume_index(U8 *vol_index_L, U8 *vol_index_R);

/**
 * @brief     This function is used to select the environment mode.
 * @param[in] mode_index is the environment mode. The mode index must be less than the maximum mode number.
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_psap_status_t audio_anc_psap_control_set_mode_index(U8 mode_index);

/**
 * @brief     This function is used to get the environment mode.
 * @param[out] *mode_index is the environment mode.
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_psap_status_t audio_anc_psap_control_get_mode_index(U8 *mode_index);

/**
 * @brief     This function is used to set the environment mode configuration in a specific mode index.
 * @param[in] mode_index is the specific environment mode index.
 * @param[in] mode_para is the configuration.
 * @param[in] role is used to decide which value to apply (L or R).
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_psap_status_t audio_anc_psap_control_set_specific_mode_table(U8 mode_index, psap_mode_table_t *mode_para, audio_psap_device_role_t role);

/**
 * @brief     This function is used to get the environment mode configuration in specific mode index.
 * @param[out] *mode_index is the specific environment mode index. The mode index must be less than the maximum mode number.
 * @param[out] *mode_para is the configuration.
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_psap_status_t audio_anc_psap_control_get_specific_mode_table(U8 mode_index, psap_mode_table_t *mode_para);

/**
 * @brief     This function is the configuration of Auto Environment Adjustment.
 * @param[in] *aea_config is the configuration.
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 * @sa        #psap_aea_config_t
 */
audio_psap_status_t audio_anc_psap_control_set_aea_configuration(psap_aea_config_t *aea_config);

/**
 * @brief     This function is used to get the configuration of Auto Environment Adjustment.
 * @param[out] *aea_config is the configuration.
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 * @sa        #psap_aea_config_t
 */
audio_psap_status_t audio_anc_psap_control_get_aea_configuration(psap_aea_config_t *aea_config);

/**
 * @brief     This function is used to enable the WNR function. WNR is only for HA.
 * @param[in] enable is the switch. 0(off), 1(on).
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_psap_status_t audio_anc_psap_control_set_wnr_switch(bool enable);

/**
 * @brief     This function is used to get the WNR function switch. WNR is only for HA.
 * @param[out] *enable is the switch. 0(off), 1(on).
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_psap_status_t audio_anc_psap_control_get_wnr_switch(bool *enable);

/**
 * @brief     This function is used to set the beamforming function.
 * @param[in] *BF_config is the configuration.
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 * @sa        #psap_bf_config_t
 */
audio_psap_status_t audio_anc_psap_control_set_beamforming_setting(psap_bf_config_t *BF_config);

/**
 * @brief     This function is used to get the configuration of the beamforming function.
 * @param[out] *BF_config is the configuration.
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 * @sa        #psap_bf_config_t
 */
audio_psap_status_t audio_anc_psap_control_get_beamforming_setting(psap_bf_config_t *BF_config);

/**
 * @brief     This function is used to set the AFC function.
 * @param[in] *afc_config is the configuration.
 * @param[in] role is used to decide which value to apply (L or R).
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 * @sa        #psap_afc_config_t
 */
audio_psap_status_t audio_anc_psap_control_set_afc_configuration(psap_afc_config_t *afc_config, audio_psap_device_role_t role);

/**
 * @brief     This function is used to get the configuration of the AFC function.
 * @param[out] *afc_config is the configuration.
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 * @sa        #psap_afc_config_t
 */
audio_psap_status_t audio_anc_psap_control_get_afc_configuration(psap_afc_config_t *afc_config);

/**
 * @brief     This function is used to set the INR function. INR is only for HA.
 * @param[in] *INR_config is the configuration.
 * @param[in] role is used to decide which value to apply (L or R).
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 * @sa        #ha_inr_config_t
 */
audio_psap_status_t audio_anc_psap_control_set_inr_configuration(ha_inr_config_t *INR_config, audio_psap_device_role_t role);

/**
 * @brief     This function is used to get the configuration of the INR function. INR is only for HA.
 * @param[out] *INR_config is the configuration.
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 * @sa        #ha_inr_config_t
 */
audio_psap_status_t audio_anc_psap_control_get_inr_configuration(ha_inr_config_t *INR_config);

/**
 * @brief     This function is used to enable the user EQ.
 * @param[in] *usr_eq_switch is the configuration.
 * @param[in] role is used to decide which value to apply (L or R).
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 * @sa        #psap_user_eq_switch_t
 */
audio_psap_status_t audio_anc_psap_control_set_user_eq_switch(psap_user_eq_switch_t *usr_eq_switch, audio_psap_device_role_t role);

/**
 * @brief     This function is used to get the switch of the user EQ.
 * @param[out] *usr_eq_switch is the configuration.
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 * @sa        #psap_user_eq_switch_t
 */
audio_psap_status_t audio_anc_psap_control_get_user_eq_switch(psap_user_eq_switch_t *usr_eq_switch);

/**
 * @brief     This function is used to set the user EQ gain.
 * @param[in] *usr_eq_gain is the configuration.
 * @param[in] role is used to decide which value to apply (L or R).
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 * @sa        #psap_usr_eq_para_t
 */
audio_psap_status_t audio_anc_psap_control_set_user_eq_gain(psap_usr_eq_para_t *usr_eq_gain, audio_psap_device_role_t role);

/**
 * @brief     This function is used to get the configuration of the user EQ gain.
 * @param[out] *usr_eq_gain is the configuration.
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 * @sa        #psap_usr_eq_para_t
 */
audio_psap_status_t audio_anc_psap_control_get_user_eq_gain(psap_usr_eq_para_t *usr_eq_gain);

/**
 * @brief     This function is used to get the speaker reference.
 * @param[out] *spk_ref is the speaker max output sound pressure for the mobile APP used for the hearing test.
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 * @sa        #psap_test_spk_ref_t
 */
audio_psap_status_t audio_anc_psap_control_get_speaker_reference(psap_test_spk_ref_t *spk_ref);

/**
 * @brief     This function is used to enable/disable the pure tone generator and only works when the HA/PSAP framework is enabled. The parameters are not saved in user settings.
 * @param[in] enable_l is the switch for the left device. 0(off), 1(on).
 * @param[in] freq_l is the frequency of the pure tone for the left device. The range is [0, 12250] (0 is mute).
 * @param[in] dBFS_l is the pure tone volume for the left device. The range is [-128, 0].
 * @param[in] enable_r is the switch for the right device. 0(off), 1(on).
 * @param[in] freq_r is the frequency of the pure tone for the right device. The range is [0, 12250] (0 is mute).
 * @param[in] dBFS_r is the pure tone volume for the right device. The range is [-128, 0].
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_psap_status_t audio_anc_psap_control_set_puretone_generator(bool enable_l, U16 freq_l, S8 dBFS_l, bool enable_r, U16 freq_r, S8 dBFS_r);

/**
 * @brief     This function is used to get the status of the pure tone generator.
 * @param[out] *enable_l is the switch for the left device. 0(off), 1(on).
 * @param[out] *freq_l is the frequency of the pure tone for the left device. The range is [0, 12250] (0 is mute).
 * @param[out] *dBFS_l is the pure tone volume for the left device. The range is [-128, 0].
 * @param[out] *enable_r is the switch for the right device. 0(off), 1(on).
 * @param[out] *freq_r is the frequency of the pure tone for the right device. The range is [0, 12250] (0 is mute).
 * @param[out] *dBFS_r is the pure tone volume for the right device. The range is [-128, 0].
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_psap_status_t audio_anc_psap_control_get_puretone_generator(bool *enable_l, U16 *freq_l, S8 *dBFS_l, bool *enable_r, U16 *freq_r, S8 *dBFS_r);

/**
 * @brief     This function is used to configure the PSAP behavior for all audio paths (e.g., A2DP, eSCO, and voice prompt).
 * @param[in] *mix_mode_para is the configuration.
 * @param[in] role is used to decide which value to apply (L or R).
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 * @sa        #psap_scenario_mix_mode_t
 */
audio_psap_status_t audio_anc_psap_control_set_mix_mode(psap_scenario_mix_mode_t *mix_mode_para, audio_psap_device_role_t role);

/**
 * @brief     This function is used to get the configuration of the PSAP behavior for all audio paths (e.g., A2DP, eSCO, and voice prompt).
 * @param[out] *mix_mode_para is the configuration.
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 * @sa        #psap_scenario_mix_mode_t
 */
audio_psap_status_t audio_anc_psap_control_get_mix_mode(psap_scenario_mix_mode_t *mix_mode_para);

/**
 * @brief     This function is for user tuning the hearing aid mic-in path frequency response. All adaptive function will be closed when the tuning mode on. Example: AFC, INR, WNR. Beamfoming, NR, MFA, HD, ...etc.
 * @param[in] mode_switch is the switch. 0(off), 1(L on), 2(R on), 3(L/R on)
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_psap_status_t audio_anc_psap_control_set_tuning_mode(U8 mode_switch);//b0: Switch(L)    b1: Switch(R)    b2~b7: Reserved

/**
 * @brief     This function is used to get the tuning mode status.
 * @param[out] *mode_switch is the switch. 0(off), 1(L on), 2(R on), 3(L/R on)
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_psap_status_t audio_anc_psap_control_get_tuning_mode(U8 *mode_switch);//b0: Switch(L)    b1: Switch(R)    b2~b7: Reserved

/**
 * @brief     This function is for MP calibration usage in the factory.  All adaptive function will be closed when the tuning mode on. Example: AFC, INR, WNR. Beamfoming, NR, MFA, HD, ...etc. And change the level index to specific level index.
 * @param[in] mode_switch is the switch. 0(off), 3(on)
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_psap_status_t audio_anc_psap_control_set_mp_test_mode(U8 mode_switch);//0x00(Off), 0x03(On)

/**
 * @brief     This function is used to get the MP test mode status.
 * @param[in] mode_switch is the switch. 0(off), 3(on)
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_psap_status_t audio_anc_psap_control_get_mp_test_mode(U8 *mode_switch);//0x00(Off), 0x03(On)

/**
 * @brief     This function is used to restore the default user settings.
 * @param[in] mode 0: Restores all default user settings. mode 1: Restores part of default user settings, except the level index and user EQ.
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_psap_status_t audio_anc_psap_control_set_restore_setting(U8 mode);


/**
 * @brief     This function is used to do the feedback detection test when the MP test mode is on. This function only be used when the HA/PSAP framework is enabled.
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_psap_status_t audio_anc_psap_control_detect_feedback(void);

/**
 * @brief     This function is used to mute the mic-in path.
 * @param[in] mute_l is the switch for the left device.
 * @param[in] mute_r is the switch for the right device.
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_psap_status_t audio_anc_psap_control_set_mute(bool mute_l, bool mute_r);

/**
 * @brief     This function is used to get the mute status of the mic-in path.
 * @param[out] *mute_l is the switch for the left device. 0(off), 1(on).
 * @param[out] *mute_r is the switch for the right device. 0(off), 1(on).
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_psap_status_t audio_anc_psap_control_get_mute(bool *mute_l, bool *mute_r);

/**
 * @brief     This function is used to set the howling detection function.
 * @param[in] *how_det is the configuration.
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 * @sa        #psap_how_det_t
 */
audio_psap_status_t audio_anc_psap_control_set_howling_detection(psap_how_det_t *how_det);

/**
 * @brief     This function is used to get the configuration of the howling detection function.
 * @param[out] *how_det is the configuration.
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 * @sa        #psap_how_det_t
 */
audio_psap_status_t audio_anc_psap_control_get_howling_detection(psap_how_det_t *how_det);

/**
 * @brief     This function is used to set the main mic of the mic-in path.
 * @param[in] channel 1(FF mic), 2(talk mic)
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_psap_status_t audio_anc_psap_control_set_mic_channel(U8 channel);

/**
 * @brief     This function is used to get the main mic of the mic-in path.
 * @param[out] *channel 1(FF mic), 2(talk mic)
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_psap_status_t audio_anc_psap_control_get_mic_channel(U8 *channel);

/**
 * @brief     This function is used to save the user setting.
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_psap_status_t audio_anc_psap_control_save_setting(void);

/**
 * @brief     This function is used to synchronize the settings between earbuds.
 * @param[in] len is the length of parameters
 * @param[in] *para is the synchronized settings
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
void audio_anc_psap_control_set_runtime_sync_parameter(uint16_t len, uint8_t* para);

/**
 * @brief     This function is used to synchronize the setting between earbuds.
 * @param[out] *len parameter length
 * @return    Return address of the parameter. You must free the memory when the synchronization process is complete.
 */
uint8_t* audio_anc_psap_control_get_runtime_sync_parameter(uint16_t *len);

/**
 * @brief     This function is used to switch the mic-in path passthrough and can only be used when the HA/PSAP framework is enabled. The parameters of this function are not saved in user setting.
 * @param[in] enable_l is the switch for the left device. 0(off), 1(on).
 * @param[in] enable_r is the switch for the right device. 0(off), 1(on).
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_psap_status_t audio_anc_psap_control_set_passthrough_switch(bool enable_l, bool enable_r);

/**
 * @brief     This function is used to get the status of the mic-in path passthrough.
 * @param[in] *enable_l is the switch for the left device. 0(off), 1(on).
 * @param[in] *enable_r is the switch for the right device. 0(off), 1(on).
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_psap_status_t audio_anc_psap_control_get_passthrough_switch(bool *enable_l, bool *enable_r);

/**
 * @brief     This function is used to register the notification callback function. AEA, feedback detection will notify result by callback.
 * @return    Return AUDIO_PSAP_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_psap_status_t audio_anc_psap_control_register_notification_callback(psap_noti_callback_t callback);


#ifdef __cplusplus
}
#endif
/**
* @}
* @}
*/
#endif  /*__AUDIO_ANC_PSAP_CONTROL_H__*/
