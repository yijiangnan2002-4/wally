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
#ifndef _DSP_PARA_INEAR_H_
#define _DSP_PARA_INEAR_H_

#include "types.h"

/**
 * @brief Parameter for DSP INEAR algorithm
 * @KeyID 0xE155
 */

typedef struct stru_dsp_inear_s {
    S16 IE_main_type;                    /**< @Value 0x0002 @Desc 1 */
    S16 IE_main_alpha;                   /**< @Value 0x7D71 @Desc 1 */
    S16 IE_main_alpha_1;                 /**< @Value 0x028E @Desc 1 */
    S16 IE_main_gain_limiter;            /**< @Value 0x0519 @Desc 1 */
    S16 IE_inear_type;                   /**< @Value 0x0002 @Desc 1 */
    S16 IE_inear_alpha;                  /**< @Value 0x7D71 @Desc 1 */
    S16 IE_inear_alpha_1;                /**< @Value 0x028E @Desc 1 */
    S16 IE_inear_gain_limiter;           /**< @Value 3276   @Desc 1 wenchen */
    S16 IE_ENG_VAD_THR;                 /**< @Value 0x0C00 @Desc 1 wenchen */
    S16 IE_ENG_VAD_HANG;                /**< @Value 0x0009 @Desc 1 wenchen */
    S16 IE_ENG_POW_INIT;                /**< @Value 0xe200 @Desc 1 wenchen */
    S16 IE_slow_up_alpha;               /**< @Value 0x0021 @Desc 1 wenchen */
    S16 IE_slow_dn_alpha;               /**< @Value 0x4000 @Desc 1 wenchen */
    S16 IE_fast_up_alpha;               /**< @Value 0x4000 @Desc 1 wenchen */
    S16 IE_fast_dn_alpha;               /**< @Value 0x028f @Desc 1 wenchen */
    S16 IE_end_inear_wind;              /**< @Value 0x0054 @Desc 1 wenchen */
    S32 IE_msc_alpha;                   /**< @Value 0x40000000 @Desc 1 wenchen */
    S32 IE_msc_sm_up_alpha;             /**< @Value 0x00624E00 @Desc 1 */
    S32 IE_msc_sm_dn_alpha;             /**< @Value 0x6CCCCCCD @Desc 1 wenchen */
    S32 IE_msc_bias;                    /**< @Value 0x00040000 @Desc 1 */
    S32 IE_MSC_WIND_THR2;               /**< @Value 0x00200000 @Desc 1 wenchen */
    S32 IE_MSC_WIND_THR;                /**< @Value 0x00100000 @Desc 1 wenchen */
    S16 IE_mid_eq;                       /**< @Value 0x1000 @Desc 1 wenchen */
    S16 IE_end_inear_est;                /**< @Value 0x0030 @Desc 1 wenchen */
    S16 IE_mid_eq_st;                    /**< @Value 0x000f @Desc 1 wenchen */
    S16 IE_inEar_nor_gain0;              /**< @Value 11627 @Desc 1 */
    S16 IE_inEar_nor_gain1;              /**< @Value 11627 @Desc 1 */
    S16 IE_ff_mel_filter;                   /**< @Value 0x0002 @Desc 0: disable melfilter, 1: enable melfilter 26 bands, 2: enable melfilter 64 bands */
    S16 IE_fb_mel_filter;                   /**< @Value 0x0000 @Desc 0: disable melfilter, 1: enable melfilter 26 bands, 2: enable melfilter 64 bands */
    S16 IE_en_pre_emp;                      /**< @Value 0x04e2 @Desc wenchen bit1: enable low band noise est, bit2: enable low band phase compensation, bit3: low band phase inverse, bit5:use ast eq, bit6:pitch enhancement, bit7: fevad_on switch,  bit9: fail safe, bit10: inear_VAD used high band suppression, bit11: degrade detection, bit12:FB mic NC, bit14: calibration fb mic, bit15: calibration ff mic */
    S16 IE_vad_credit_thr;                  /**< @Value 0x0800 @Desc 1 */
    S16 IE_padding2;                        /**< @Value 0x0000 @Desc 1 */
    S16 IE_main_alpha_peak_up;             /**< @Value 0x747b @Desc 1 */
    S16 IE_main_alpha_peak_dn;             /**< @Value 0x7c29 @Desc 1 */
    S16 IE_main_alpha_valley_dn;           /**< @Value 0x747b @Desc 1 */
    S16 IE_main_alpha_valley_up;           /**< @Value 0x7c29 @Desc 1 */
    S16 IE_main_pow_diff_thr;              /**< @Value 0x0348 @Desc 1 */
    S16 IE_main_pow_credit;                /**< @Value 0x03e8 @Desc 1 */
    S16 IE_main_alpha_sm_fast;             /**< @Value 0x4ccd @Desc 1 */
    S16 IE_main_alpha_sm_slow;             /**< @Value 0x6ccd @Desc 1 */
    S16 IE_main_noise_update_thr0;         /**< @Value 0x0a80 @Desc 1 */
    S16 IE_main_noise_update_thr1;         /**< @Value 0x0780 @Desc 1 */
    S16 IE_main_noise_update_thr2;         /**< @Value 0x0300 @Desc 1 */
    S16 IE_main_padding;                   /**< @Value 0x0000 @Desc 1 */
    S32 IE_main_noise_alpha_fast;          /**< @Value 0x66666600 @Desc 1 */
    S32 IE_main_noise_alpha_slow;          /**< @Value 0x7c28f600 @Desc 1 */
    S32 IE_main_noise_alpha_ultra_slow;    /**< @Value 0x7f7cee00 @Desc 1 */
    S32 IE_main_over_est;                   /**< @Value 0x19999a00 @Desc 1 */
    S16 IE_inear_alpha_peak_up;            /**< @Value 0x747b @Desc 1 */
    S16 IE_inear_alpha_peak_dn;            /**< @Value 0x7c29 @Desc 1 */
    S16 IE_inear_alpha_valley_dn;          /**< @Value 0x747b @Desc 1 */
    S16 IE_inear_alpha_valley_up;          /**< @Value 0x7c29 @Desc 1 */
    S16 IE_inear_pow_diff_thr;             /**< @Value 0x0348 @Desc 1 */
    S16 IE_inear_pow_credit;               /**< @Value 0x03e8 @Desc 1 */
    S16 IE_inear_alpha_sm_fast;            /**< @Value 0x4ccd @Desc 1 */
    S16 IE_inear_alpha_sm_slow;            /**< @Value 0x6ccd @Desc 1 */
    S16 IE_inear_noise_update_thr0;        /**< @Value 0x0a80 @Desc 1 */
    S16 IE_inear_noise_update_thr1;        /**< @Value 0x0780 @Desc 1 */
    S16 IE_inear_noise_update_thr2;        /**< @Value 0x0300 @Desc 1 */
    S16 IE_inear_padding;                  /**< @Value 0x0000 @Desc 1 */

    S32 IE_inear_noise_alpha_fast;         /**< @Value 0x66666600 @Desc 1 */
    S32 IE_inear_noise_alpha_slow;         /**< @Value 0x7c28f600 @Desc 1 */
    S32 IE_inear_noise_alpha_ultra_slow;   /**< @Value 0x7f7cee00 @Desc 1 */
    S32 IE_inear_over_est;                 /**< @Value 0x06660000 @Desc 1  wenchen */
    S32 IE_main_nest_alpha;                 /**< @Value 1503238656 @Desc 1 */
    S32 IE_main_nest_alpha_1;               /**< @Value 644244736 @Desc 1 */
    S32 IE_main_nest_decay;                 /**< @Value 2040109568 @Desc 1 */
    S32 IE_main_nest_over_est;              /**< @Value 429496832 @Desc 1 */
    S32 IE_inear_nest_alpha;                /**< @Value 0x66660000 @Desc 1 wenchen */
    S32 IE_inear_nest_alpha_1;              /**< @Value 429496576 @Desc 1 */
    S32 IE_inear_nest_decay;                /**< @Value 0x73330000 @Desc 1 wenchen */
    S32 IE_inear_nest_over_est;             /**< @Value 0x199a0000 @Desc 1 wenchen */
    S16 IE_de_pop_gain_limiter;             /**< @Value    3277 @Desc 1 */
    S16 IE_fe_vad_thr;                      /**< @Value 0x0000 @Desc 1 */
    S16 IE_PF_MIN;                  /**< @Value 0x0400 @Desc 1 */
    S16 IE_DT_ratio_thrd;           /**< @Value 0x4000 @Desc 1 */
    S16 IE_DT_length;               /**< @Value 0x0016 @Desc 1 */
    S16 IE_FB_MIC_ATTEN;                /**< @Value 0x4000 @Desc 1 */
    S16 IE_MID_WIND_ATTEN;              /**< @Value 0x7FFF @Desc 1 */
    S16 IE_h_1st_max        ;       /**< @Value 0x199A @Desc 1 */
    S16 IE_wind_frozon_han  ;       /**< @Value 0x000A @Desc 1 */
    S16 IE_inEear_input_gain;       /**< @Value 2048   @Desc 1 */
    S32 IE_MSC_frozon_eng   ;       /**< @Value 0x00000078 @Desc 1 wenchen */
    S32 IE_pitch_thr        ;       /**< @Value 0x0007A120 @Desc 1 */
    S32 IE_pitch_thr2       ;       /**< @Value 0x00026480 @Desc 1 */
    S16 IE_pitch_idx_st     ;       /**< @Value 0x0028 @Desc 1 */
    S16 IE_pitch_idx_end    ;       /**< @Value 0x00C8 @Desc 1 */
    S16 IE_max_dis          ;       /**< @Value 0x000A @Desc 1 */
    S16 IE_h_max            ;       /**< @Value 0x3333 @Desc 1 */
    S16 IE_h_min            ;       /**< @Value 0x0500 @Desc 1 */

    //FB MIC reference Gain
    S16 IE_CH1_REF_GAIN;                /**< @Value 0x0148 @Desc 1 */
    S16 IE_CH2_REF_GAIN;                /**< @Value 0x028F @Desc 1 */
    S16 IE_CH3_REF_GAIN;                /**< @Value 0x028F @Desc 1 */
    S16 IE_CH4_REF_GAIN;                /**< @Value 0x028F @Desc 1 */
    S16 IE_Dummy2;                      /**< @Value 0x0000      @Desc 1 */
    S32 IE_main_nest_over_est1;         /**< @Value 1717986816  @Desc 1 */
    S32 IE_inear_nest_over_est1;        /**< @Value 858993408   @Desc 1 */
    S16 IE_pitch_bin_end    ;           /**< @Value 0x0050      @Desc 1 */
    S16 IE_pitch_enhance_end;           /**< @Value 0x0078      @Desc 1 */
    S16 IE_mix_w0;                      /**< @Value 0x7FFF      @Desc 1 */
    S16 IE_mix_w1;                      /**< @Value 0x4000      @Desc 1 */
    S16 IE_mix_w2;                      /**< @Value 0x7333      @Desc 1 */
    S16 IE_Dummy1;                      /**< @Value 0x0000      @Desc 1 */
    S32 IE_main_nest_decay1;            /**< @Value 2147483392 @Desc 1 */
    S32 IE_inear_nest_decay1;           /**< @Value 2147483392 @Desc 1 */
    S32 IE_MSC_VAD_THR;                 /**< @Value  402653184 @Desc 1 */
    S16 IE_h_sub_max;                   /**< @Value      13107 @Desc 1 */
    S16 IE_h_1st_sub_max;               /**< @Value       6554 @Desc 1 */
    S16 IE_MIC_RATIO_THR;               /**< @Value       8192 @Desc 1 */
    S16 IE_BAND_RATIO_THR;              /**< @Value       1229 @Desc 1 wenchen */
    S16 IE_low_st;                      /**< @Value          1 @Desc 1 wenchen */
    S16 IE_high_st;                     /**< @Value         16 @Desc 1 wenchen */
    S16 IE_mic_ratio_alpha;             /**< @Value         80 @Desc 1 */
    S16 IE_band_ratio_alpha;            /**< @Value        164 @Desc 1 wenchen */
    S16 IE_SBEC_PF_order_12;            /**< @Value    0x2828 @Desc 1 */
    S16 IE_SBEC_PF_order_34;            /**< @Value    0x28ff @Desc 1 */
    S16 IE_SBEC_DT_ratio_thrd_12;       /**< @Value    0x3c32 @Desc 1 */
    S16 IE_SBEC_DT_ratio_thrd_34;       /**< @Value    0x3232 @Desc 1 */
    S16 IE_SBEC_PF_MIN_12;              /**< @Value    0x5A5A @Desc 1 */
    S16 IE_SBEC_PF_MIN_34;              /**< @Value    0x5A5A @Desc 1 */
    S16 vad_end_bin;                    /**< @Value        48 @Desc 1 */
    S16 IE_post_type;                   /**< @Value         0 @Desc 1 */
    S16 IE_post_alpha;                  /**< @Value         0 @Desc 1 */
    S16 IE_post_alpha_1;                /**< @Value         0 @Desc 1 */
    S16 IE_post_gain_limiter;           /**< @Value         0 @Desc 1 */
    S16 IE_post_mel_filter;             /**< @Value         0 @Desc 1 */
    S16 IE_post_abs_thr_order;          /**< @Value         0 @Desc 1 */
    S16 ov_st_bin;                      /**< @Value         0 @Desc 1 */
    S16 nc_sig_nor;                     /**< @Value    0x1998 @Desc 1 */
    S16 NC_MU_MIN;                      /**< @Value         8 @Desc 1 */
    S16 IE_wind_pow_thd;                /**< @Value       164 @Desc 1 */

    //Reserve
    U16 RESERVE_1;      /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_2;      /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_3;      /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_4;      /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_5;      /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_6;      /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_7;      /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_8;      /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_9;      /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_10;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_11;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_12;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_13;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_14;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_15;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_16;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_17;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_18;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_19;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_20;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_21;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_22;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_23;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_24;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_25;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_26;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_27;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_28;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_29;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_30;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_31;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_32;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_33;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_34;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_35;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_36;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_37;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_38;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_39;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_40;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_41;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_42;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_43;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_44;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_45;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_46;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_47;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_48;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_49;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_50;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_51;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_52;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_53;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_54;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_55;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_56;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_57;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_58;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_59;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_60;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_61;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_62;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_63;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_64;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_65;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_66;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_67;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_68;   /**< @Value 0x0000 @Desc 1 */
    U16 RESERVE_69;   /**< @Value 0x0000 @Desc 1 */

} PACKED DSP_PARA_INEAR_STRU;

#endif /* _DSP_PARA_INEAR_H_ */

