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
#ifndef _HA_USER_SETTING_NVKEY_STRUCT_H
#define _HA_USER_SETTING_NVKEY_STRUCT_H

#include "types.h"

typedef struct {
    U8 afc_ctrl_switch_l  : 1;                             /**< @Value 1 @Desc 1 */
    U8 afc_ctrl_switch_r  : 1;                             /**< @Value 1 @Desc 1 */
    U8 reserved           : 6;                             /**< @Value 0 @Desc 1 */
} PACKED psap_afc_config_t;


typedef struct {
    U8 mfa_switch_l       : 1;                             /**< @Value 0 @Desc 1 */
    U8 mfa_switch_r       : 1;                             /**< @Value 0 @Desc 1 */
    U8 low_cut_switch_l   : 1;                             /**< @Value 0 @Desc 1 */
    U8 low_cut_switch_r   : 1;                             /**< @Value 0 @Desc 1 */
    U8 nr_switch          : 1;                             /**< @Value 1 @Desc 1 */
    U8 beamforming_switch : 1;                             /**< @Value 0 @Desc 1 */
    U8 reserved_1         : 2;                             /**< @Value 0 @Desc 1 */
    U8 nr_level;                                           /**< @Value 4 @Desc 1 */
    U8 reserved_2;                                         /**< @Value 0 @Desc 1 */
} PACKED psap_mode_nvdm_t;

typedef struct {
    U8 aea_switch            : 1;
    U8 aea_nr_switch         : 1;
    U8 aea_nr_level          : 2;
    U8 reserved_13           : 4;
    U8 aea_det_period;
    U8 reserved_14;
} PACKED psap_aea_config_t;

typedef struct {
    U8 bf_switch             : 1;                           /**< @Value 0 @Desc 1 */
    U8 bf_switch_mode_ctrl   : 1;                           /**< @Value 0 @Desc 1 */
    U8 reserved_20           : 6;                           /**< @Value 0 @Desc 1 */
} PACKED psap_bf_config_t;


typedef struct {
    U8 inr_switch_l          : 1;
    U8 inr_sensitivity_l     : 4;
    U8 reserved_15           : 3;
    U8 inr_strength_l        : 2;
    U8 reserved_16           : 6;
    U8 inr_switch_r          : 1;
    U8 inr_sensitivity_r     : 4;
    U8 reserved_17           : 3;
    U8 inr_strength_r        : 2;
    U8 reserved_18           : 6;
} PACKED ha_inr_config_t;

typedef struct {
    U8 psap_user_eq_switch_l   : 1;
    U8 psap_user_eq_switch_r   : 1;
    U8 reserved_8            : 6;
} PACKED psap_user_eq_switch_t;

typedef struct {
    S8 psap_user_eq_overall_l;
    S8 psap_user_eq_band_l[HA_BAND_NUM];
    S8 psap_user_eq_overall_r;
    S8 psap_user_eq_band_r[HA_BAND_NUM];
} PACKED psap_usr_eq_para_t;

typedef struct {
    U8 a2dp_mix_mode_switch  : 1;                          /**< @Value 0 @Desc 1 */
    U8 reserved_3            : 2;                          /**< @Value 0 @Desc 1 */
    U8 a2dp_drc_switch_l     : 1;                          /**< @Value 0 @Desc 1 */
    U8 a2dp_drc_switch_r     : 1;                          /**< @Value 0 @Desc 1 */
    U8 a2dp_mfa_switch_l     : 1;                          /**< @Value 0 @Desc 1 */
    U8 a2dp_mfa_switch_r     : 1;                          /**< @Value 0 @Desc 1 */
    U8 reserved_4            : 1;                          /**< @Value 0 @Desc 1 */
    S8 a2dp_mix_mode_psap_gain_l;                            /**< @Value 0 @Desc 1 */
    S8 a2dp_mix_mode_psap_gain_r;                            /**< @Value 0 @Desc 1 */

    U8 sco_mix_mode_switch   : 1;                          /**< @Value 0 @Desc 1 */
    U8 sco_drc_switch_l      : 1;                          /**< @Value 0 @Desc 1 */
    U8 sco_drc_switch_r      : 1;                          /**< @Value 0 @Desc 1 */
    U8 sco_mfa_switch_l      : 1;                          /**< @Value 0 @Desc 1 */
    U8 sco_mfa_switch_r      : 1;                          /**< @Value 0 @Desc 1 */
    U8 reserved_5            : 3;                          /**< @Value 0 @Desc 1 */
    S8 sco_mix_mode_psap_gain_l;                             /**< @Value 0 @Desc 1 */
    S8 sco_mix_mode_psap_gain_r;                             /**< @Value 0 @Desc 1 */

    U8 vp_mix_mode_switch   : 1;                           /**< @Value 0 @Desc 1 */
    U8 vp_drc_switch_l      : 1;                           /**< @Value 0 @Desc 1 */
    U8 vp_drc_switch_r      : 1;                           /**< @Value 0 @Desc 1 */
    U8 vp_mfa_switch_l      : 1;                           /**< @Value 0 @Desc 1 */
    U8 vp_mfa_switch_r      : 1;                           /**< @Value 0 @Desc 1 */
    U8 reserved_6           : 3;                           /**< @Value 0 @Desc 1 */
    S8 vp_mix_mode_psap_gain_l;                              /**< @Value 0 @Desc 1 */
    S8 vp_mix_mode_psap_gain_r;                              /**< @Value 0 @Desc 1 */
} PACKED psap_scenario_mix_mode_t;

/** ========================================================================================================
 * @brief Parameter for HA user setting
 * @KeyID 0xE825
========================================================================================================== */
typedef struct {
    struct {
        U8 afc_ctrl_switch_l  : 1;                             /**< @Value 1 @Desc 1 */
        U8 afc_ctrl_switch_r  : 1;                             /**< @Value 1 @Desc 1 */
        U8 reserved           : 6;                             /**< @Value 0 @Desc 1 */
    } PACKED psap_afc_config_t;

    U8 drc_level_index_l;                                  /**< @Value 3 @Desc 1 */
    U8 drc_level_index_r;                                  /**< @Value 3 @Desc 1 */
    U8 reserved_1;                          /**< @Value 0 @Desc 1 */

    U8 drc_MPO_adjust_switch : 1;                          /**< @Value 0 @Desc 1 */
    S8 drc_MPO_adjust_value  : 7;                          /**< @Value 0 @Desc 1 */
    U8 reserved_2[7];                                      /**< @Value 0 @Desc 1 */

    struct {
        U8 reserved_3            : 3;                          /**< @Value 0 @Desc 1 */
        U8 a2dp_drc_switch_l     : 1;                          /**< @Value 0 @Desc 1 */
        U8 a2dp_drc_switch_r     : 1;                          /**< @Value 0 @Desc 1 */
        U8 a2dp_mfa_switch_l     : 1;                          /**< @Value 0 @Desc 1 */
        U8 a2dp_mfa_switch_r     : 1;                          /**< @Value 0 @Desc 1 */
        U8 reserved_4            : 1;                          /**< @Value 0 @Desc 1 */
        S8 a2dp_mix_mode_psap_gain_l;                            /**< @Value 0 @Desc 1 */
        S8 a2dp_mix_mode_psap_gain_r;                            /**< @Value 0 @Desc 1 */

        U8 reserved_25           : 1;                          /**< @Value 0 @Desc 1 */
        U8 sco_drc_switch_l      : 1;                          /**< @Value 0 @Desc 1 */
        U8 sco_drc_switch_r      : 1;                          /**< @Value 0 @Desc 1 */
        U8 sco_mfa_switch_l      : 1;                          /**< @Value 0 @Desc 1 */
        U8 sco_mfa_switch_r      : 1;                          /**< @Value 0 @Desc 1 */
        U8 reserved_5            : 3;                          /**< @Value 0 @Desc 1 */
        S8 sco_mix_mode_psap_gain_l;                             /**< @Value 0 @Desc 1 */
        S8 sco_mix_mode_psap_gain_r;                             /**< @Value 0 @Desc 1 */

        U8 reserved_26          : 1;                           /**< @Value 0 @Desc 1 */
        U8 vp_drc_switch_l      : 1;                           /**< @Value 0 @Desc 1 */
        U8 vp_drc_switch_r      : 1;                           /**< @Value 0 @Desc 1 */
        U8 vp_mfa_switch_l      : 1;                           /**< @Value 0 @Desc 1 */
        U8 vp_mfa_switch_r      : 1;                           /**< @Value 0 @Desc 1 */
        U8 reserved_6           : 3;                           /**< @Value 0 @Desc 1 */
        S8 vp_mix_mode_psap_gain_l;                              /**< @Value 0 @Desc 1 */
        S8 vp_mix_mode_psap_gain_r;                              /**< @Value 0 @Desc 1 */
    } PACKED psap_scenario_mix_mode_t;


    U8 reserved_7[6];                                      /**< @Value 0 @Desc 1 */

    U8 mode_index;                                         /**< @Value 0 @Desc 1 */
    psap_mode_nvdm_t psap_mode[8];

    struct {
        U8 psap_user_eq_switch_l   : 1;                          /**< @Value 0 @Desc 1 */
        U8 psap_user_eq_switch_r   : 1;                          /**< @Value 0 @Desc 1 */
        U8 reserved_8            : 6;                          /**< @Value 0 @Desc 1 */
    } PACKED psap_user_eq_switch_t;
    struct {
        S8 psap_user_eq_overall_l;                               /**< @Value 0 @Desc 1 */
        S8 psap_user_eq_band_l[HA_BAND_NUM];                     /**< @Value 0 @Desc 1 */
        S8 psap_user_eq_overall_r;                               /**< @Value 0 @Desc 1 */
        S8 psap_user_eq_band_r[HA_BAND_NUM];                     /**< @Value 0 @Desc 1 */
    } PACKED psap_usr_eq_para_t;
    U8 reserved_9[5];                                      /**< @Value 0 @Desc 1 */

    U8 psap_vol_index_l;                                     /**< @Value 11 @Desc 1 */
    U8 psap_vol_index_r;                                     /**< @Value 11 @Desc 1 */
    U8 reserved_10;                                        /**< @Value 0 @Desc 1 */

    U8 WNR_switch            : 1;                          /**< @Value 0 @Desc 1 */
    U8 reserved_11           : 7;                          /**< @Value 0 @Desc 1 */

    U8 reserved_12[8];                                      /**< @Value 0 @Desc 1 */

    struct {
        U8 aea_switch            : 1;                          /**< @Value 0 @Desc 1 */
        U8 aea_nr_switch         : 1;                          /**< @Value 0 @Desc 1 */
        U8 aea_nr_level          : 2;                          /**< @Value 0 @Desc 1 */
        U8 reserved_13           : 4;                          /**< @Value 0 @Desc 1 */
        U8 aea_det_period;                                     /**< @Value 5 @Desc 1 */
        U8 reserved_14;                                     /**< @Value 0 @Desc 1 */
    } PACKED psap_aea_config_t;                                  /**< @Value 0 @Desc 1 */

    U8 reserved_14_2[5];

    struct {
        U8 inr_switch_l          : 1;                          /**< @Value 1 @Desc 1 */
        U8 inr_sensitivity_l     : 4;                          /**< @Value 3 @Desc 1 */
        U8 reserved_15           : 3;                          /**< @Value 0 @Desc 1 */
        U8 inr_strength_l        : 2;                          /**< @Value 1 @Desc 1 */
        U8 reserved_16           : 6;                          /**< @Value 0 @Desc 1 */
        U8 inr_switch_r          : 1;                          /**< @Value 1 @Desc 1 */
        U8 inr_sensitivity_r     : 4;                          /**< @Value 3 @Desc 1 */
        U8 reserved_17           : 3;                          /**< @Value 0 @Desc 1 */
        U8 inr_strength_r        : 2;                          /**< @Value 1 @Desc 1 */
        U8 reserved_18           : 6;                          /**< @Value 0 @Desc 1 */
    } PACKED ha_inr_config_t;

    U8 reserved_19[8];                                     /**< @Value 0 @Desc 1 */

    struct {
        U8 bf_switch             : 1;                          /**< @Value 0 @Desc 1 */
        U8 bf_switch_mode_ctrl   : 1;                          /**< @Value 0 @Desc 1 */
        U8 reserved_20           : 6;                          /**< @Value 0 @Desc 1 */
    } PACKED psap_bf_config_t;

    U8 reserved_21[7];                                     /**< @Value 0 @Desc 1 */

    U16 reserved_22;                                       /**< @Value 0 @Desc 1 */
    U8 psap_master_mic_ch;                                   /**< @Value 0 @Desc 1 */
    U8 reserved_27;                                        /**< @Value 0 @Desc 1 */
    U8 reserved_28;                                        /**< @Value 1 @Desc 1 */

    U8 reserved_23[3];                                     /**< @Value 0 @Desc 1 */

    U8 how_det_enable;                                     /**< @Value 1 @Desc 1 */
    S16 sup_han;                                           /**< @Value 2048 @Desc 1 */

    U8 reserved_24[21];                                    /**< @Value 0 @Desc 1 */
} PACKED psap_usr_setting_nvdm_t;


#endif /* _HA_USER_SETTING_NVKEY_STRUCT_H */


