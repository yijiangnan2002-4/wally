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
#ifndef _HA_ALGO_NVKEY_STRUCT_H
#define _HA_ALGO_NVKEY_STRUCT_H

#include "types.h"
#include "ha_mp_nvkey_struct.h"

/** ========================================================================================================
 * @brief Parameter for HA AFC
 * @KeyID 0xE801
========================================================================================================== */
typedef struct {
    /** AFC Coeff **/
    U8  afc_filter_type;                                    /**< @Value 0 @Desc 1 */
    U8  feedback_shift;                                     /**< @Value 0 @Desc 1 */
    U8  revision;                                           /**< @Value 2 @Desc 1 */
    U8  fs_cutting_freq_index;                              /**< @Value 0 @Desc 1 */
    U32 nlms_epsilon;                                       /**< @Value 0x0863 @Desc 1 */
    U32 ges_smooth_factor;                                  /**< @Value 0x0CCCCCCD @Desc 1 */
    S32 step_size;                                          /**< @Value 0x0CCCCCCD @Desc 1 */
    U32 reserved_2;                                         /**< @Value 0 @Desc 1 */

    /** AFC LBN **/
    U8  lbn_enable;                                         /**< @Value 1 @Desc 1 */
    U8  reserved_3;                                         /**< @Value 0 @Desc 1 */
    S16 lbn_ref_noise_smoothing_alpha;                      /**< @Value 0x0831 @Desc 1 */
    S16 lbn_ref_noise_gain;                                 /**< @Value 0x0400 @Desc 1 */
    S16 lbn_ref_gain;                                       /**< @Value 0x0CCD @Desc 1 */
    U32 reserved_4;                                         /**< @Value 0 @Desc 1 */
    U32 reserved_5;                                         /**< @Value 0 @Desc 1 */
    U32 reserved_6;                                         /**< @Value 0 @Desc 1 */
    U32 reserved_7;                                         /**< @Value 0 @Desc 1 */

    /** FBD **/
    U32 fdb_trigger_block_count;                            /**< @Value 0x00000032 @Desc 1 */
    U32 fdb_wait_block_count;                               /**< @Value 0x000002EE @Desc 1 */
    U32 fdb_measure_block_count;                            /**< @Value 0x000009C4 @Desc 1 */
    U32 reserved_8;                                         /**< @Value 0 @Desc 1 */
    U32 reserved_9;                                         /**< @Value 0 @Desc 1 */
    U32 reserved_10;                                        /**< @Value 0 @Desc 1 */
    U32 reserved_11;                                        /**< @Value 0 @Desc 1 */

    /** AFC HS **/
    U8  hs_enable;                                          /**< @Value 1 @Desc 1 */
    U8  hs_debug;                                           /**< @Value 0 @Desc 1 */
    U8  hs_version;                                         /**< @Value 0 @Desc 1 */
    U8  reserved_14;                                        /**< @Value 0 @Desc 1 */
    S32 hs_input_smoothing_alpha;                           /**< @Value 0x73333332 @Desc 1 */
    S32 hs_feedback_smoothing_alpha;                        /**< @Value 0x73333332 @Desc 1 */
    S32 hs_threshold;                                       /**< @Value 0x0CCCCCCD @Desc 1 */
    S32 hs_supression_gain;                                 /**< @Value 0x00000000 @Desc 1 */
    U16 hs_smooth_down_time;                                /**< @Value 0x00C8 @Desc 1 */
    U16 hs_supression_time;                                 /**< @Value 0x0BB8 @Desc 1 */
    U16 hs_smooth_up_time;                                  /**< @Value 0x00C8 @Desc 1 */
    U16 hs_detect_times_threshold;                          /**< @Value 0x012C @Desc 1 */
    U8  hs_afc_convergence_speed_target;                    /**< @Value 0 @Desc 1 */
    U8  hs_afc_convergence_speed_add_step;                  /**< @Value 0 @Desc 1 */
    U8  hs_afc_convergence_speed_sub_step;                  /**< @Value 0 @Desc 1 */
    U8  hs_db_threshold;                                    /**< @Value 0 @Desc 1 */
    U8  hs_suppression_gain_mode;                           /**< @Value 0 @Desc 1 */
    U8  reserved_16[3];                                     /**< @Value 0 @Desc 1 */
    U32 hs_smoothdown_detect_threshold;                     /**< @Value 0 @Desc 1 */
    U32 reserved_18;                                        /**< @Value 0 @Desc 1 */

    /** AFC ACS **/
    U8  acs_enable;                                         /**< @Value 1 @Desc 1 */
    U8  acs_gse_frac_array_size;                            /**< @Value 0x28 @Desc 1 */
    U8  acs_trigger_block_count;                            /**< @Value 0x04 @Desc 1 */
    U8  acs_debug;                                          /**< @Value 0 @Desc 1 */
    S32 acs_ref_smoothing_alpha;                            /**< @Value 0x73333332 @Desc 1 */
    S32 acs_threshold_1;                                    /**< @Value 0x000249F0 @Desc 1 */
    S32 acs_threshold_2;                                    /**< @Value 0x00013880 @Desc 1 */
    S32 acs_threshold_3;                                    /**< @Value 0x00004E20 @Desc 1 */
    U8  acs_target_value_1;                                 /**< @Value 0x15 @Desc 1 */
    U8  acs_target_value_2;                                 /**< @Value 0x11 @Desc 1 */
    U8  acs_target_value_3;                                 /**< @Value 0x0F @Desc 1 */
    U8  acs_target_value_4;                                 /**< @Value 0x0B @Desc 1 */
    U8  acs_add_step_1;                                     /**< @Value 0x03 @Desc 1 */
    U8  acs_add_step_2;                                     /**< @Value 0x03 @Desc 1 */
    U8  acs_add_step_3;                                     /**< @Value 0x03 @Desc 1 */
    U8  acs_sub_step;                                       /**< @Value 0x01 @Desc 1 */
    S16 acs_gse_frac_1;                                     /**< @Value 0x4000 @Desc 1 */
    S16 acs_gse_frac_2;                                     /**< @Value 0x411E @Desc 1 */
    S16 acs_gse_frac_3;                                     /**< @Value 0x4242 @Desc 1 */
    S16 acs_gse_frac_4;                                     /**< @Value 0x436A @Desc 1 */
    S16 acs_gse_frac_5;                                     /**< @Value 0x4498 @Desc 1 */
    S16 acs_gse_frac_6;                                     /**< @Value 0x45CB @Desc 1 */
    S16 acs_gse_frac_7;                                     /**< @Value 0x4703 @Desc 1 */
    S16 acs_gse_frac_8;                                     /**< @Value 0x4841 @Desc 1 */
    S16 acs_gse_frac_9;                                     /**< @Value 0x4984 @Desc 1 */
    S16 acs_gse_frac_10;                                    /**< @Value 0x4ACD @Desc 1 */
    S16 acs_gse_frac_11;                                    /**< @Value 0x4C1C @Desc 1 */
    S16 acs_gse_frac_12;                                    /**< @Value 0x4D71 @Desc 1 */
    S16 acs_gse_frac_13;                                    /**< @Value 0x4ECB @Desc 1 */
    S16 acs_gse_frac_14;                                    /**< @Value 0x502C @Desc 1 */
    S16 acs_gse_frac_15;                                    /**< @Value 0x5192 @Desc 1 */
    S16 acs_gse_frac_16;                                    /**< @Value 0x52FF @Desc 1 */
    S16 acs_gse_frac_17;                                    /**< @Value 0x5473 @Desc 1 */
    S16 acs_gse_frac_18;                                    /**< @Value 0x55ED @Desc 1 */
    S16 acs_gse_frac_19;                                    /**< @Value 0x576D @Desc 1 */
    S16 acs_gse_frac_20;                                    /**< @Value 0x58F4 @Desc 1 */
    S16 acs_gse_frac_21;                                    /**< @Value 0x5A82 @Desc 1 */
    S16 acs_gse_frac_22;                                    /**< @Value 0x5C17 @Desc 1 */
    S16 acs_gse_frac_23;                                    /**< @Value 0x5DB4 @Desc 1 */
    S16 acs_gse_frac_24;                                    /**< @Value 0x5F57 @Desc 1 */
    S16 acs_gse_frac_25;                                    /**< @Value 0x6102 @Desc 1 */
    S16 acs_gse_frac_26;                                    /**< @Value 0x62B4 @Desc 1 */
    S16 acs_gse_frac_27;                                    /**< @Value 0x646D @Desc 1 */
    S16 acs_gse_frac_28;                                    /**< @Value 0x662F @Desc 1 */
    S16 acs_gse_frac_29;                                    /**< @Value 0x67F8 @Desc 1 */
    S16 acs_gse_frac_30;                                    /**< @Value 0x69C9 @Desc 1 */
    S16 acs_gse_frac_31;                                    /**< @Value 0x6BA2 @Desc 1 */
    S16 acs_gse_frac_32;                                    /**< @Value 0x6D84 @Desc 1 */
    S16 acs_gse_frac_33;                                    /**< @Value 0x6F6E @Desc 1 */
    S16 acs_gse_frac_34;                                    /**< @Value 0x7161 @Desc 1 */
    S16 acs_gse_frac_35;                                    /**< @Value 0x735C @Desc 1 */
    S16 acs_gse_frac_36;                                    /**< @Value 0x7560 @Desc 1 */
    S16 acs_gse_frac_37;                                    /**< @Value 0x776E @Desc 1 */
    S16 acs_gse_frac_38;                                    /**< @Value 0x7984 @Desc 1 */
    S16 acs_gse_frac_39;                                    /**< @Value 0x7BA4 @Desc 1 */
    S16 acs_gse_frac_40;                                    /**< @Value 0x7DCD @Desc 1 */
    U32 reserved_20;                                        /**< @Value 0 @Desc 1 */
    U32 reserved_21;                                        /**< @Value 0 @Desc 1 */
    U32 reserved_22;                                        /**< @Value 0 @Desc 1 */
    U32 reserved_23;                                        /**< @Value 0 @Desc 1 */
} PACKED psap_afc_coef_nvdm_t;


/** ========================================================================================================
 * @brief Parameter for HA DRC
 * @KeyID 0xE802
========================================================================================================== */
typedef struct {
    U8  drc_level_max_count;                                /**< @Value 5 @Desc 1 */
    U8  drc_Level_index_MP_test_mode;                       /**< @Value 3 @Desc 1 */
    U8  drc_band_count;                                     /**< @Value 50 @Desc 1 */
    U8  drc_knee_point_count;                               /**< @Value 4 @Desc 1 */
    U8  reserved[8];                                        /**< @Value 0, 0, 0, 0, 0, 0, 0, 0 @Desc 1 */
} PACKED psap_drc_ctrl_nvdm_t;


/** ========================================================================================================
 * @brief Parameter for HA level
 * @KeyID 0xE803~0xE812 (level 1~16)
========================================================================================================== */
typedef struct {
    U8  drc_band_attack_time;                               /**< @Value 20 @Desc 1 */
    U8  drc_band_release_time;                              /**< @Value 20 @Desc 1 */
    U8  drc_band_knee_point1_input;                         /**< @Value 90 @Desc 1 */
    S8  drc_band_knee_point1_gain;                          /**< @Value 0 @Desc 1 */
    U8  drc_band_knee_point2_input;                         /**< @Value 80 @Desc 1 */
    S8  drc_band_knee_point2_gain;                          /**< @Value 0 @Desc 1 */
    U8  drc_band_knee_point3_input;                         /**< @Value 50 @Desc 1 */
    S8  drc_band_knee_point3_gain;                          /**< @Value 0 @Desc 1 */
    U8  drc_band_knee_point4_input;                         /**< @Value 0 @Desc 1 */
    S8  drc_band_knee_point4_gain;                          /**< @Value 0 @Desc 1 */
    U8  drc_band_MPO;                                       /**< @Value 127 @Desc 1 */
} PACKED psap_drc_band_nvdm_t;


typedef struct {
    U8  mfa_shift;                                          /**< @Value 2 @Desc 1 */
    psap_drc_band_nvdm_t drc_band[HA_BAND_NUM];
    U8  afc_performance_para;                               /**< @Value 11 @Desc 1 */
    U8  multimedia_copy_band;                               /**< @Value 0 @Desc 1 */
    U8  reserved[7];                                        /**< @Value 0 @Desc 1 */
} PACKED psap_level_nvdm_t;

/** ========================================================================================================
 * @brief Parameter for HA para
 * @KeyID 0xE813
========================================================================================================== */
typedef struct {
    S8  a2dp_drc_extra_gain;                                /**< @Value 0 @Desc 1 */
    S8  sco_drc_extra_gain;                                 /**< @Value 0 @Desc 1 */
    S8  vp_drc_extra_gain;                                  /**< @Value 0 @Desc 1 */
    U8  reserved[9];                                        /**< @Value 0, 0, 0, 0, 0, 0, 0, 0, 0 @Desc 1 */
} PACKED psap_para_nvdm_t;


/** ========================================================================================================
 * @brief Parameter for HA low cut
 * @KeyID 0xE814
========================================================================================================== */
typedef struct {
    S8  low_cut_off_gain;                                   /**< @Value 0 @Desc 1 */
    S8  low_cut_on_gain;                                    /**< @Value -20 @Desc 1 */
    U8  reserved[6];                                        /**< @Value 0, 0, 0, 0, 0, 0 @Desc 1 */
} PACKED psap_low_cut_nvdm_t;


/** ========================================================================================================
 * @brief Parameter for HA mode
 * @KeyID 0xE815
========================================================================================================== */
typedef struct {
    U8  mode_count;                                         /**< @Value 2 @Desc 1 */
    U8  mode_eq_switch : 1;                                 /**< @Value 0 @Desc 1 */
    U8  reserved_1     : 7;                                 /**< @Value 0 @Desc 1 */
    U8  reserved_2[2];                                      /**< @Value 0, 0 @Desc 1 */
} PACKED psap_mode_ctrl_nvdm_t;


/** ========================================================================================================
 * @brief Parameter for HA mode EQ
 * @KeyID 0xE816 ~ 0xE81D
========================================================================================================== */
typedef struct {
    S8  eq_overall;                                         /**< @Value 0 @Desc 1 */
    S8  eq_band[HA_BAND_NUM];                               /**< @Value 0 @Desc 1 */
    U8  reserved[9];                                        /**< @Value 0 @Desc 1 */
} PACKED psap_mode_eq_nvdm_t;


/** ========================================================================================================
 * @brief Parameter for HA vendor EQ
 * @KeyID 0xE81F
========================================================================================================== */
typedef struct {
    U8  vendor_eq_switch : 1;                               /**< @Value 0 @Desc 1 */
    U8  reserved_1       : 7;                               /**< @Value 0 @Desc 1 */

    S8  vendor_eq_overall;                                  /**< @Value 0 @Desc 1 */
    S8  vendor_eq_band[HA_BAND_NUM];                        /**< @Value 0 @Desc 1 */
    U8  reserved_2[8];                                      /**< @Value 0 @Desc 1 */
} PACKED psap_vendor_eq_nvdm_t;


/** ========================================================================================================
 * @brief Parameter for HA volume
 * @KeyID 0xE820
========================================================================================================== */
typedef struct {
    U8  psap_vol_max_count;                                   /**< @Value 15 @Desc 1 */

    S8  ha_vol_table[16];                                   /**< @Value -128, -42, -39, -36, -33, -30, -27, -24, -21, -18, -15, -12, -9, -6, -3, 0 @Desc 1 */
    U8  reserved[7];                                        /**< @Value 0, 0, 0, 0, 0, 0, 0 @Desc 1 */
} PACKED psap_vol_nvdm_t;


/** ========================================================================================================
 * @brief Parameter for HA AEA COEF
 * @KeyID 0xE821
========================================================================================================== */
typedef struct {
    U8  aea_eq_switch : 1;                                  /**< @Value 0 @Desc 1 */
    U8  reserved_1    : 7;                                  /**< @Value 0 @Desc 1 */
    S8  aea_eq_overall;                                     /**< @Value 0 @Desc 1 */
    S8  aea_eq_band[HA_BAND_NUM];                           /**< @Value 0 @Desc 1 */
    U8  aea_table[8];                                       /**< @Value 0 @Desc 1 */
    U8  reserved_2[8];                                      /**< @Value 0 @Desc 1 */
} PACKED psap_aea_coef_nvdm_t;


/** ========================================================================================================
 * @brief Parameter for HA Beamforming
 * @KeyID 0xE822
========================================================================================================== */
typedef struct {
    U8  bf_master_mic_ch : 1;                                  /**< @Value 0 @Desc 1 */
    U8  reserved_1       : 7;                                  /**< @Value 0 @Desc 1 */
    U8  revision;                                              /**< @Value 0 @Desc 1 */
    U16 bf_mic_distance;                                       /**< @Value 215 @Desc 1 */
    U32 bf_distance_table[102];                                /**< @Value 0 @Desc 1 */
    S16 mu;                                                    /**< @Value 3277 @Desc 1 */
    U8  das;                                                   /**< @Value 0 @Desc 1 */
    U8  reserved_3[5];                                         /**< @Value 0 @Desc 1 */
} PACKED psap_bf_nvdm_t;


/** ========================================================================================================
 * @brief Parameter for HOW DET
 * @KeyID 0xE823
========================================================================================================== */
typedef struct {
    U8  reserved_1;                                        /**< @Value 1 @Desc 1 */
    U8  revision;                                          /**< @Value 0 @Desc 1 */
    S16 DET_BIN;                                           /**< @Value 46 @Desc 1 */
    S16 STA_LEN;                                           /**< @Value 1 @Desc 1 */
    S16 HOW_STD_TH;                                        /**< @Value 3 @Desc 1 */
    S16 NTH_0;                                             /**< @Value 15 @Desc 1 */
    S16 NTH_1;                                             /**< @Value 15 @Desc 1 */
    S16 reserved_2;                                        /**< @Value 2048 @Desc 1 */
    S16 mode;                                              /**< @Value 0 @Desc 1 */
    S32 HOW_TH;                                            /**< @Value 1000000 @Desc 1 */
    U8  DET_DB_0;                                          /**< @Value 0 @Desc 1 */
    U8  DET_DB_1;                                          /**< @Value 0 @Desc 1 */
    U8  DET_DB_2;                                          /**< @Value 0xff @Desc 1 */
    U8  DET_DB_3;                                          /**< @Value 0xff @Desc 1 */
    U8  DET_DB_4;                                          /**< @Value 0xf0 @Desc 1 */
    U8  DET_DB_5;                                          /**< @Value 0 @Desc 1 */
    U8  DET_DB_6;                                          /**< @Value 0 @Desc 1 */
    U8  DET_DB_7;                                          /**< @Value 0 @Desc 1 */
    S16 atten;                                             /**< @Value 0 @Desc 1 */
    S16 NUM_SUP_BD;                                        /**< @Value 1 @Desc 1 */
    S32 HOW_TH2;                                           /**< @Value 1000000 @Desc 1 */
    S32 HOW_TH3;                                           /**< @Value 1000000 @Desc 1 */
    S32 HOW_TH4;                                           /**< @Value 100000 @Desc 1 */
    S32 reserved_3;                                        /**< @Value 0 @Desc 1 */
    S32 reserved_4;                                        /**< @Value 0 @Desc 1 */
    S32 reserved_5;                                        /**< @Value 0 @Desc 1 */
    S32 reserved_6;                                        /**< @Value 0 @Desc 1 */
} PACKED howling_detect_nvdm_t;


/** ========================================================================================================
 * @brief Parameter for HA SYSTEM
 * @KeyID 0xE824
========================================================================================================== */
typedef struct {
    U16 reserved;                                          /**< @Value 0 @Desc 1 */
    U8  bootup_turn_on_psap_delay_time;                      /**< @Value 0 @Desc 1 */
    U8  inear_turn_on_psap_delay_time;                       /**< @Value 0 @Desc 1 */
    U8  BLE_advertising_timeout;                           /**< @Value 0 @Desc 1 */
    S8  RSSI_thd;                                          /**< @Value 0 @Desc 1 */
    U8  RSSI_pwr_off_timer;                                /**< @Value 0 @Desc 1 */
    U8  dedicate_mac_addr[6];                              /**< @Value 0 @Desc 1 */
    U8  ANC_mode_with_VividPT;                             /**< @Value 0 @Desc 1 */
    U8  ANC_mode_with_HA_PSAP;                             /**< @Value 0 @Desc 1 */
    U8  reserved_1[5];                                     /**< @Value 0, 0, 0, 0, 0, 0, 0 @Desc 1 */

    U8  HA_on_mode_VP_switch                : 1;           /**< @Value 1 @Desc 1 */
    U8  HA_level_up_circular_max_VP_switch  : 1;           /**< @Value 0 @Desc 1 */
    U8  HA_mode_up_circular_max_VP_switch   : 1;           /**< @Value 0 @Desc 1 */
    U8  HA_volume_up_circular_max_VP_switch : 1;           /**< @Value 0 @Desc 1 */
    U8  reserved_2                          : 4;           /**< @Value 0 @Desc 1 */

    S8  HA_mic_AD_gain_l;                                  /**< @Value 0 @Desc 1 */
    S8  HA_mic_AD_gain_r;                                  /**< @Value 0 @Desc 1 */
    U8  config_tool_band_count;                            /**< @Value 16 @Desc 1 */
    U8  ptg_bg_frequency;                                  /**< @Value 0 @Desc 1 */
    S8  ptg_bg_dbfs;                                       /**< @Value 0 @Desc 1 */
    U8  mfa_total_enabled                   : 1;           /**< @Value 0 @Desc 1 */
    U8  reserved_4                          : 7;           /**< @Value 0 @Desc 1 */
    U8  reserved_3[5];                                     /**< @Value 0, 0, 0, 0, 0 @Desc 1 */
} PACKED psap_system_nvdm_t;


/** ========================================================================================================
 * @brief Parameter for HA WNR
 * @KeyID 0xE827
========================================================================================================== */
typedef struct {
    U8  revision;                                          /**< @Value 1 @Desc 1 */
    U8  HW_input_max;                                      /**< @Value 0 @Desc 1 */
    S8  sensitivity;                                       /**< @Value 0 @Desc 1 */
    U8  not_apply_gain_switch;                             /**< @Value 0 @Desc 1 */
    S32 gain_smooth_alpha;                                 /**< @Value 0x0CCCCCCD @Desc 1 */
    S32 psd_smooth_alpha;                                  /**< @Value 0x00800000 @Desc 1 */
    S32 detect_thd_for_inr;                                /**< @Value 429496730 @Desc 1 */
    U32 Reserve_3;                                         /**< @Value 0 @Desc 1 */
    U32 Reserve_4;                                         /**< @Value 0 @Desc 1 */
    U32 Reserve_5;                                         /**< @Value 0 @Desc 1 */
} PACKED ha_wnr_nvdm_t;


/** ========================================================================================================
 * @brief Parameter for HA Beamforming Step Control
 * @KeyID 0xE828
========================================================================================================== */
typedef struct {
    U8  version;                                                /**< @Value 0 @Desc 1 */
    U8  enabled;                                                /**< @Value 0 @Desc 1 */
    S16 phase_difference_updating_alpha;                        /**< @Value 31457 @Desc 1 */
    S16 phase_difference_updating_attenuation;                  /**< @Value 29491 @Desc 1 */
    S16 phase_difference_updating_attenuation_threshold_amp;    /**< @Value 328 @Desc 1 */
    U8  subband_range[11];                                      /**< @Value 0, 2, 4, 8, 12, 16, 20, 24, 32, 40, 50 @Desc 1 */
    U8  reserved_1;                                             /**< @Value 0 @Desc 1 */
    S16 subband_vad_thrd1[10];                                  /**< @Value 20, 20, 123, 102, 819, 1024, 1638, 4096, 5120, 1638 @Desc 1 */
    S16 subband_vad_thrd2[10];                                  /**< @Value 10, 10, 82, 61, 410, 1024, 1229, 3072, 4096, 1024  @Desc 1 */
    S16 subband_step1[10];                                      /**< @Value 3277, 3277, 3277, 3277, 3277, 3277, 3277, 3277, 3277, 3277 @Desc 1 */
    S16 subband_step2[10];                                      /**< @Value 328, 328, 328, 328, 328, 328, 328, 328, 328, 328 @Desc 1 */
    S32 reserved_2[25];                                         /**< @Value 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 @Desc 1 */
} PACKED psap_bf_sc_nvdm_t;

#endif /* _HA_ALGO_NVKEY_STRUCT_H */


