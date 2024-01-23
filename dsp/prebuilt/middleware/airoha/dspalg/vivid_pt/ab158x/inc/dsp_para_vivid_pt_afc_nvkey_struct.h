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
#ifndef _DSP_PARA_VIVID_PT_AFC_H_
#define _DSP_PARA_VIVID_PT_AFC_H_

#include "types.h"

/**
 * @brief Parameter for NVID_DSP_ALG_VIVID_PT_AFC
 * @KeyID 0xE8FB
 */

#ifdef WIN32
#pragma pack(1)
#endif

typedef struct {
    U8  ENABLE;                            /**< @Value 0x01       @Desc 1 */
    U8  revision;                          /**< @Value 0x01       @Desc 1 */
    U8  filter_type;                       /**< @Value 0x00       @Desc 1 */
    U8  reserved_1;                        /**< @Value 0x00       @Desc 1 */
    S32 step_size;                         /**< @Value 0x0CCCCCCD       @Desc 1 */
    S16 gse_smoothing_alpha;               /**< @Value 0x0CCD       @Desc 1 */
    S16 gse_mul_frac;                      /**< @Value 0x4000       @Desc 1 */
    S8  gse_mul_exp;                       /**< @Value 0x0D       @Desc 1 */
    S8  lms_int_bits;                      /**< @Value 0x00       @Desc 1 */
    S16 ref_gain;                          /**< @Value 0x2000       @Desc 1 */
    S32 noisy_ref_gain;                    /**< @Value 0x78000000       @Desc 1 */
    S32 reserved_2;                        /**< @Value 0x00000000       @Desc 1 */
    S32 reserved_3;                        /**< @Value 0x00000000       @Desc 1 */
    S32 reserved_4;                        /**< @Value 0x00000000       @Desc 1 */
    S32 reserved_5;                        /**< @Value 0x00000000       @Desc 1 */
    S32 reserved_6;                        /**< @Value 0x00000000       @Desc 1 */
    S32 reserved_7;                        /**< @Value 0x00000000       @Desc 1 */
    S32 reserved_8;                        /**< @Value 0x00000000       @Desc 1 */
    S32 reserved_9;                        /**< @Value 0x00000000       @Desc 1 */
    S32 reserved_10;                       /**< @Value 0x00000000       @Desc 1 */
    S32 reserved_11;                       /**< @Value 0x00000000       @Desc 1 */
    S32 reserved_12;                       /**< @Value 0x00000000       @Desc 1 */
    S32 reserved_13;                       /**< @Value 0x00000000       @Desc 1 */
    S32 reserved_14;                       /**< @Value 0x00000000       @Desc 1 */

    /** AFC HS **/
    U8  hs_enable;                         /**< @Value 0x01       @Desc 1 */
    U8  reserved_15;                       /**< @Value 0x00       @Desc 1 */
    U8  reserved_16;                       /**< @Value 0x00       @Desc 1 */
    U8  reserved_17;                       /**< @Value 0x00       @Desc 1 */

    S32 hs_input_smoothing_alpha;          /**< @Value 0x73333332       @Desc 1 */
    S32 hs_fedback_smoothing_alpha;        /**< @Value 0x73333332       @Desc 1 */
    S32 hs_threshold;                      /**< @Value 0x0CCCCCCD       @Desc 1 */
    S32 hs_suppression_gain;               /**< @Value 0x0020C49C       @Desc 1 */

    U16 hs_smooth_down_time;               /**< @Value 0x00C8       @Desc 1 */
    U16 hs_supression_time;                /**< @Value 0x0BB8       @Desc 1 */
    U16 hs_sooth_up_time;                  /**< @Value 0x00C8       @Desc 1 */
    U16 hs_detect_times_threshold;         /**< @Value 0x012C       @Desc 1 */

    S32 reserved_18;                       /**< @Value 0x00000000       @Desc 1 */
    S32 reserved_19;                       /**< @Value 0x00000000       @Desc 1 */
    S32 reserved_20;                       /**< @Value 0x00000000       @Desc 1 */
    S32 reserved_21;                       /**< @Value 0x00000000       @Desc 1 */

    /** AFC ACS **/
    U8  acs_enable;                        /**< @Value 0x01       @Desc 1 */
    U8  acs_gse_frac_array_size;           /**< @Value 0x28       @Desc 1 */
    U8  acs_trigger_block_count;           /**< @Value 0x04       @Desc 1 */
    U8  reserved_22;                       /**< @Value 0x00       @Desc 1 */

    S32 acs_ref_smoothing_alpha;           /**< @Value 0x73333332       @Desc 1 */
    S32 acs_threshold_1;                   /**< @Value 0x000249F0       @Desc 1 */
    S32 acs_threshold_2;                   /**< @Value 0x000249F0       @Desc 1 */
    S32 acs_threshold_3;                   /**< @Value 0x000249F0       @Desc 1 */

    U8  acs_target_value_1;                /**< @Value 0x15       @Desc 1 */
    U8  acs_target_value_2;                /**< @Value 0x11       @Desc 1 */
    U8  acs_target_value_3;                /**< @Value 0x0F       @Desc 1 */
    U8  acs_target_value_4;                /**< @Value 0x0B       @Desc 1 */

    U8  acs_add_step_1;                    /**< @Value 0x03       @Desc 1 */
    U8  acs_add_step_2;                    /**< @Value 0x03       @Desc 1 */
    U8  acs_add_step_3;                    /**< @Value 0x03       @Desc 1 */
    U8  acs_sub_step;                      /**< @Value 0x01       @Desc 1 */

    S16 acs_gse_frac_1;                    /**< @Value 0x4000       @Desc 1 */
    S16 acs_gse_frac_2;                    /**< @Value 0x411E       @Desc 1 */
    S16 acs_gse_frac_3;                    /**< @Value 0x4242       @Desc 1 */
    S16 acs_gse_frac_4;                    /**< @Value 0x436A       @Desc 1 */
    S16 acs_gse_frac_5;                    /**< @Value 0x4498       @Desc 1 */
    S16 acs_gse_frac_6;                    /**< @Value 0x45CB       @Desc 1 */
    S16 acs_gse_frac_7;                    /**< @Value 0x4703       @Desc 1 */
    S16 acs_gse_frac_8;                    /**< @Value 0x4841       @Desc 1 */
    S16 acs_gse_frac_9;                    /**< @Value 0x4984       @Desc 1 */
    S16 acs_gse_frac_10;                   /**< @Value 0x4ACD       @Desc 1 */
    S16 acs_gse_frac_11;                   /**< @Value 0x4C1C       @Desc 1 */
    S16 acs_gse_frac_12;                   /**< @Value 0x4D71       @Desc 1 */
    S16 acs_gse_frac_13;                   /**< @Value 0x4ECB       @Desc 1 */
    S16 acs_gse_frac_14;                   /**< @Value 0x502C       @Desc 1 */
    S16 acs_gse_frac_15;                   /**< @Value 0x5192       @Desc 1 */
    S16 acs_gse_frac_16;                   /**< @Value 0x52FF       @Desc 1 */
    S16 acs_gse_frac_17;                   /**< @Value 0x5473       @Desc 1 */
    S16 acs_gse_frac_18;                   /**< @Value 0x55ED       @Desc 1 */
    S16 acs_gse_frac_19;                   /**< @Value 0x576D       @Desc 1 */
    S16 acs_gse_frac_20;                   /**< @Value 0x58F4       @Desc 1 */
    S16 acs_gse_frac_21;                   /**< @Value 0x5A82       @Desc 1 */
    S16 acs_gse_frac_22;                   /**< @Value 0x5C17       @Desc 1 */
    S16 acs_gse_frac_23;                   /**< @Value 0x5DB4       @Desc 1 */
    S16 acs_gse_frac_24;                   /**< @Value 0x5F57       @Desc 1 */
    S16 acs_gse_frac_25;                   /**< @Value 0x6102       @Desc 1 */
    S16 acs_gse_frac_26;                   /**< @Value 0x62B4       @Desc 1 */
    S16 acs_gse_frac_27;                   /**< @Value 0x646D       @Desc 1 */
    S16 acs_gse_frac_28;                   /**< @Value 0x662F       @Desc 1 */
    S16 acs_gse_frac_29;                   /**< @Value 0x67F8       @Desc 1 */
    S16 acs_gse_frac_30;                   /**< @Value 0x69C9       @Desc 1 */
    S16 acs_gse_frac_31;                   /**< @Value 0x6BA2       @Desc 1 */
    S16 acs_gse_frac_32;                   /**< @Value 0x6D84       @Desc 1 */
    S16 acs_gse_frac_33;                   /**< @Value 0x6F6E       @Desc 1 */
    S16 acs_gse_frac_34;                   /**< @Value 0x7161       @Desc 1 */
    S16 acs_gse_frac_35;                   /**< @Value 0x735C       @Desc 1 */
    S16 acs_gse_frac_36;                   /**< @Value 0x7560       @Desc 1 */
    S16 acs_gse_frac_37;                   /**< @Value 0x776E       @Desc 1 */
    S16 acs_gse_frac_38;                   /**< @Value 0x7984       @Desc 1 */
    S16 acs_gse_frac_39;                   /**< @Value 0x7BA4       @Desc 1 */
    S16 acs_gse_frac_40;                   /**< @Value 0x7DCD       @Desc 1 */

    S32 reserved_23;                       /**< @Value 0x00000000       @Desc 1 */
    S32 reserved_24;                       /**< @Value 0x00000000       @Desc 1 */
    S32 reserved_25;                       /**< @Value 0x00000000       @Desc 1 */
    S32 reserved_26;                       /**< @Value 0x00000000       @Desc 1 */

} PACKED DSP_PARA_VIVID_PT_AFC_STRU;

#ifdef WIN32
#pragma pack()
#endif

#endif /* _DSP_PARA_VIVID_PT_AFC_H_ */
