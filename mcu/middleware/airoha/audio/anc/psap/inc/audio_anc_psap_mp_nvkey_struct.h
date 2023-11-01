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
#ifndef _AUDIO_ANC_PSAP_MP_NVKEY_STRUCT_H
#define _AUDIO_ANC_PSAP_MP_NVKEY_STRUCT_H

#include "types.h"


/**
 * @brief Parameter for PSAP/HA MP daa
 * @KeyID 0xE800
 */
typedef struct {
    S8 hw_input_max_l_Overall;                             /**< @Value 0x00 @Desc 1 */
    U8 hw_input_max_l[HA_BAND_NUM];                        /**< @Value 110 @Desc 1 */
    S8 hw_output_max_l_Overall;                            /**< @Value 0x00 @Desc 1 */
    U8 hw_output_max_l[HA_BAND_NUM];                       /**< @Value 110 @Desc 1 */
    S8 hw_input_max_r_Overall;                             /**< @Value 0x00 @Desc 1 */
    U8 hw_input_max_r[HA_BAND_NUM];                        /**< @Value 110 @Desc 1 */
    S8 hw_output_max_r_Overall;                            /**< @Value 0x00 @Desc 1 */
    U8 hw_output_max_r[HA_BAND_NUM];                       /**< @Value 110 @Desc 1 */

    U8 test_spk_ref_l_64;                                  /**< @Value 110 @Desc 1 */
    U8 test_spk_ref_l_125;                                 /**< @Value 110 @Desc 1 */
    U8 test_spk_ref_l_250;                                 /**< @Value 110 @Desc 1 */
    U8 test_spk_ref_l_500;                                 /**< @Value 110 @Desc 1 */
    U8 test_spk_ref_l_1000;                                /**< @Value 110 @Desc 1 */
    U8 test_spk_ref_l_2000;                                /**< @Value 110 @Desc 1 */
    U8 test_spk_ref_l_4000;                                /**< @Value 110 @Desc 1 */
    U8 test_spk_ref_l_6000;                                /**< @Value 110 @Desc 1 */
    U8 test_spk_ref_l_8000;                                /**< @Value 110 @Desc 1 */
    U8 test_spk_ref_l_12000;                               /**< @Value 110 @Desc 1 */
    U8 test_spk_ref_r_64;                                  /**< @Value 110 @Desc 1 */
    U8 test_spk_ref_r_125;                                 /**< @Value 110 @Desc 1 */
    U8 test_spk_ref_r_250;                                 /**< @Value 110 @Desc 1 */
    U8 test_spk_ref_r_500;                                 /**< @Value 110 @Desc 1 */
    U8 test_spk_ref_r_1000;                                /**< @Value 110 @Desc 1 */
    U8 test_spk_ref_r_2000;                                /**< @Value 110 @Desc 1 */
    U8 test_spk_ref_r_4000;                                /**< @Value 110 @Desc 1 */
    U8 test_spk_ref_r_6000;                                /**< @Value 110 @Desc 1 */
    U8 test_spk_ref_r_8000;                                /**< @Value 110 @Desc 1 */
    U8 test_spk_ref_r_12000;                               /**< @Value 110 @Desc 1 */

    S8 rssi_cal_offset;                                    /**< @Value 0 @Desc 1 */
    U8 reserved[7];                                        /**< @Value 0 @Desc 1 */
} PACKED psap_mp_nvdm_t;


#endif /* _AUDIO_ANC_PSAP_MP_NVKEY_STRUCT_H */
