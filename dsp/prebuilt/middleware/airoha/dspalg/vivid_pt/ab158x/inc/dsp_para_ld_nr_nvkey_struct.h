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
#ifndef _DSP_PARA_VIVID_NR_H_
#define _DSP_PARA_VIVID_NR_H_

#include "types.h"

/**
 * @brief Parameter for NVID_DSP_ALG_VIVID_PT_LDNR
 * @KeyID 0xE8FA
 */

#ifdef WIN32
#pragma pack(1)
#endif

typedef struct stru_dsp_at_ld_para_s {
    U8  ENABLE;                     /**< @Value 0x00       @Desc 1 */
    U8  REVISION;                   /**< @Value 0x00       @Desc 1 */
    U16 padding1;                   /**< @Value 0x0000     @Desc 1 */

    S16 sup_par_type;               /**< @Value 0x0000     @Desc 1 */
    S16 sup_par_alpha;              /**< @Value 0x799A     @Desc 1 */
    S16 sup_par_alpha_1;            /**< @Value 0x0665     @Desc 1 */
    S16 sup_par_gain_limiter;       /**< @Value 0x474A     @Desc 1 */

    S16 ld_alpha_peak_up;           /**< @Value 0x747B     @Desc 1 */
    S16 ld_alpha_peak_dn;           /**< @Value 0x7C29     @Desc 1 */
    S16 ld_alpha_valley_dn;         /**< @Value 0x747B     @Desc 1 */
    S16 ld_alpha_valley_up;         /**< @Value 0x7C29     @Desc 1 */
    S16 ld_pow_diff_thr;            /**< @Value 0x0348     @Desc 1 */
    S16 ld_pow_credit;              /**< @Value 0x03E8     @Desc 1 */
    S16 ld_alpha_sm_fast;           /**< @Value 0x4CCD     @Desc 1 */
    S16 ld_alpha_sm_slow;           /**< @Value 0x6CCD     @Desc 1 */
    S16 ld_noise_update_thr0;       /**< @Value 0x0A80     @Desc 1 */
    S16 ld_noise_update_thr1;       /**< @Value 0x0780     @Desc 1 */
    S16 ld_noise_update_thr2;       /**< @Value 0x0300     @Desc 1 */
    S16 ld_min_period;              /**< @Value 0x000D     @Desc 1 */
    S32 ld_noise_alpha_fast;        /**< @Value 0x66666600 @Desc 1 */
    S32 ld_noise_alpha_slow;        /**< @Value 0x7c28f600 @Desc 1 */
    S32 ld_noise_alpha_ultra_slow;  /**< @Value 0x7f7cee00 @Desc 1 */
    S32 ld_over_est;                /**< @Value 0x19999a00 @Desc 1 */
    S16 SOS_Section;                /**< @Value     0x0003 @Desc 1 */
    S16 ld_init_gain;               /**< @Value      16384 @Desc 1 */
    S32 pitch_thr;                  /**< @Value 0x003C0000 @Desc 1 */
    S32 pitch_thr2;                 /**< @Value 0x00230000 @Desc 1 */
    S16 pitch_idx_st;               /**< @Value     0x000A @Desc 1 */
    S16 pitch_idx_end;              /**< @Value     0x00C8 @Desc 1 */
    S16 max_dis;                    /**< @Value     0x000A @Desc 1 */
    S16 pitch_bin_end;              /**< @Value     0x0100 @Desc 1 */
    S16 pitch_en;                   /**< @Value     0x0002 @Desc 1 */
    S16 Pitch_Structure_Reserve;    /**< @Value     0x0000 @Desc 1 */
    S32 noise_floor_1;              /**< @Value 0x00000417 @Desc 1 */
    S32 noise_floor_2;              /**< @Value 0x00000244 @Desc 1 */
    S32 noise_floor_3;              /**< @Value 0x000000C4 @Desc 1 */
    S32 noise_floor_4;              /**< @Value 0x00000002 @Desc 1 */
    S32 noise_floor_5;              /**< @Value 0x00000000 @Desc 1 */
    S32 noise_floor_6;              /**< @Value 0x00000000 @Desc 1 */
    S32 noise_floor_7;              /**< @Value 0x0000000B @Desc 1 */
    S32 noise_floor_8;              /**< @Value 0x00000000 @Desc 1 */
    S32 noise_floor_9;              /**< @Value 0x00000000 @Desc 1 */
    S32 noise_floor_10;             /**< @Value 0x00000000 @Desc 1 */
    S32 noise_floor_11;             /**< @Value 0x00000000 @Desc 1 */
    S32 noise_floor_12;             /**< @Value 0x00000037 @Desc 1 */
    S32 noise_floor_13;             /**< @Value 0x00000017 @Desc 1 */
    S32 noise_floor_14;             /**< @Value 0x00000000 @Desc 1 */
    S32 noise_floor_15;             /**< @Value 0x00000000 @Desc 1 */
    S32 noise_floor_16;             /**< @Value 0x0000003D @Desc 1 */
    S32 noise_floor_17;             /**< @Value 0x00000012 @Desc 1 */
    S32 noise_floor_18;             /**< @Value 0x00000000 @Desc 1 */
    S32 noise_floor_19;             /**< @Value 0x00000000 @Desc 1 */
    S32 noise_floor_20;             /**< @Value 0x00000092 @Desc 1 */
    S32 noise_floor_21;             /**< @Value 0x000000C5 @Desc 1 */
    S32 noise_floor_22;             /**< @Value 0x00000021 @Desc 1 */
    S32 noise_floor_23;             /**< @Value 0x000000A2 @Desc 1 */
    S32 noise_floor_24;             /**< @Value 0x0000015C @Desc 1 */
    S32 noise_floor_25;             /**< @Value 0x000001CE @Desc 1 */
    S32 noise_floor_26;             /**< @Value 0x000001EE @Desc 1 */
    S32 noise_floor_27;             /**< @Value 0x000001EE @Desc 1 */
    S32 noise_floor_28;             /**< @Value 0x000001EA @Desc 1 */
    S32 noise_floor_29;             /**< @Value 0x000001EA @Desc 1 */
    S32 noise_floor_30;             /**< @Value 0x000001FA @Desc 1 */
    S16 numEngagingFrm;             /**< @Value        488 @Desc 1 */
    S16 numDisengagingFrm;          /**< @Value        488 @Desc 1 */

    S32 SOS_1b0;                    /**< @Value 0x0F234D40 @Desc 1 */
    S32 SOS_1b1;                    /**< @Value 0xE217C980 @Desc 1 */
    S32 SOS_1b2;                    /**< @Value 0x0EC560B0 @Desc 1 */
    S32 SOS_1a1;                    /**< @Value 0xE21EEEA0 @Desc 1 */
    S32 SOS_1a2;                    /**< @Value 0x0DEFD310 @Desc 1 */
    S32 SOS_2b0;                    /**< @Value 0x10DA2040 @Desc 1 */
    S32 SOS_2b1;                    /**< @Value 0xE2656EE0 @Desc 1 */
    S32 SOS_2b2;                    /**< @Value 0x0CF556A0 @Desc 1 */
    S32 SOS_2a1;                    /**< @Value 0xE2656EE0 @Desc 1 */
    S32 SOS_2a2;                    /**< @Value 0x0DCF7700 @Desc 1 */
    S32 SOS_3b0;                    /**< @Value 0x11900360 @Desc 1 */
    S32 SOS_3b1;                    /**< @Value 0xE6254620 @Desc 1 */
    S32 SOS_3b2;                    /**< @Value 0x0A6C0B60 @Desc 1 */
    S32 SOS_3a1;                    /**< @Value 0xE6254620 @Desc 1 */
    S32 SOS_3a2;                    /**< @Value 0x0BFC0ED0 @Desc 1 */
    S32 SOS_gain;                   /**< @Value 0x0B53BEF5 @Desc 1 */

    //Reserve
    U32 RESERVE_1;                  /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_2;                  /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_3;                  /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_4;                  /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_5;                  /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_6;                  /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_7;                  /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_8;                  /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_9;                  /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_10;                 /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_11;                 /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_12;                 /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_13;                 /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_14;                 /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_15;                 /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_16;                 /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_17;                 /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_18;                 /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_19;                 /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_20;                 /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_21;                 /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_22;                 /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_23;                 /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_24;                 /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_25;                 /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_26;                 /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_27;                 /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_28;                 /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_29;                 /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_30;                 /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_31;                 /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_32;                 /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_33;                 /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_34;                 /**< @Value 0x00000000 @Desc 1 */
} PACKED DSP_PARA_AT_LD_STRU;

#ifdef WIN32
#pragma pack()
#endif

#endif /* _DSP_PARA_VIVID_NR_H_ */
