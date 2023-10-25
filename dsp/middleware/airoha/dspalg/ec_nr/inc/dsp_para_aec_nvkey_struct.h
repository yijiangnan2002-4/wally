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
#ifndef _DSP_PARA_AEC_H_
#define _DSP_PARA_AEC_H_

#include "types.h"

/**
 * @brief Parameter for DSP AEC algorithm
 * @KeyID 0xE153
 */

typedef struct stru_dsp_aec_s {
    U16 AEC_NR_EN;                /**< @Value 0x0015 @Desc b0: EC/NR switch, b1-2: 0(1-MIC), 1(1-MIC PD), 2(2-MIC), b3: 0(headset),1(speaker), b4: 0(inear off),1(inear on), b13: 0(disable Rx HPF), 1(enable Rx HPF), b15: 0(disable flash read) ,1(enable flash read)*/
    U16 AEC_CNG_GAIN_M;         /**< @Value 0x3333 @Desc 1 */
    U16 AEC_ref_pow_min;        /**< @Value 0x0001 @Desc 1 */
    U16 AEC_ECHO_TAIL_LENGTH;   /**< @Value 0x0006 @Desc 1 */
    U16 AEC_EC_RESIST;                  /**< @Value 0x0000 @Desc 1 */
    U16 AEC_MU_FAC;                         /**< @Value 0x399A @Desc 1 */
    U16 AEC_MU_MIN;                         /**< @Value 0x1000 @Desc 1 */
    U16 AEC_NORM_CAP1;                  /**< @Value 0x1FD7 @Desc 1 */
    U16 AEC_NORM_CAP2;                  /**< @Value 0x3000 @Desc 1 */
    U16 AEC_PF_MIN;                         /**< @Value 0x4000 @Desc 1 */
    S32 AEC_block_percent;          /**< @Value 0x00155555 @Desc 1 */
    U16 AEC_DT_boost;                       /**< @Value 0x0001 @Desc 1 */
    U16 AEC_PF_order;                       /**< @Value 0x0003 @Desc 1 */
    U16 AEC_DT_ratio_thrd;          /**< @Value 0x4000 @Desc 1 */
    U16 AEC_norm_tap;                       /**< @Value 0x0018 @Desc 1 */
    U16 AEC_DT_length;                  /**< @Value 0x0006 @Desc 1 */
    U16 MULT_AFTER_EC;                  /**< @Value 0x1000 @Desc 1 */
    S16 CH1_REF_GAIN;                       /**< @Value 0x0021 @Desc 1 */
    S16 CH2_REF_GAIN;                       /**< @Value 0x0021 @Desc 1 */
    S16 CH3_REF_GAIN;                       /**< @Value 0x0852 @Desc 1 */
    S16 CH4_REF_GAIN;                       /**< @Value 0x299A @Desc 1 */
    S16 CH1_REF2_GAIN;                  /**< @Value 0x0148 @Desc 1 */
    S16 CH2_REF2_GAIN;                  /**< @Value 0x028F @Desc 1 */
    S16 CH3_REF2_GAIN;                  /**< @Value 0x028F @Desc 1 */
    S16 CH4_REF2_GAIN;                  /**< @Value 0x028F @Desc 1 */

    U16 DSP_EC_SW;                                          /**< @Value 0x0000 @Desc 1 */
    U16 AEC_Corel_Ratio1_Sensitive;         /**< @Value 0x0100     @Desc 1 */
    U16 AEC_Corel_Ratio4_Sensitive;         /**< @Value 0x0100     @Desc 1 */
    S16 AEC_Ch4_Percentage;                         /**< @Value 0x0000     @Desc 1 */
    U16 RXIN_TXREF_DELAY;                               /**< @Value 0x0004     @Desc 1 */
    U16 MIC_EC_DELAY    ;                                   /**< @Value 0x003C     @Desc 1 */
    U16 AEC_REF_GAIN_AUTO;                          /**< @Value 0x0000     @Desc 1 */
    S16 AEC_tap_length;                             /**< @Value 0x755F     @Desc 1 */
    S16 AEC_ENG_VAD_THR;                  /**< @Value 0x0a00        @Desc 1 */
    S16 AEC_ENG_VAD_HANG;                  /**< @Value 0x0005       @Desc 1 */
    S16 AEC_ENG_POW_INIT;                  /**< @Value 0x0400       @Desc 1 */
    S16 AEC_slow_up_alpha;                  /**< @Value 0x0021      @Desc 1 */
    S16 AEC_slow_dn_alpha;                  /**< @Value 0x4000      @Desc 1 */
    S16 AEC_fast_up_alpha;                  /**< @Value 0x6666      @Desc 1 */
    S16 AEC_fast_dn_alpha;                  /**< @Value 0x0666      @Desc 1 */

    S16 SBEC_PF_order_12;               /**< @Value    0x0028 @Desc high-byte = 0(sbec_sw off), !=0(sbec_sw on) */
    S16 SBEC_PF_order_34;               /**< @Value    0x08ff @Desc 1 */
    S16 SBEC_DT_ratio_thrd_12;          /**< @Value    0x1e32 @Desc 1 */
    S16 SBEC_DT_ratio_thrd_34;          /**< @Value    0x3232 @Desc 1 */
    S16 SBEC_PF_MIN_12;                 /**< @Value    0x5A40 @Desc 1 */
    S16 SBEC_PF_MIN_34;                 /**< @Value    0x405A @Desc 1 */
    S16 SBEC_noise_paste_gain;          /**< @Value     32767 @Desc 1 */

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
} PACKED DSP_PARA_AEC_STRU;

#endif /* _DSP_PARA_AEC_H_ */

