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
#ifndef _LEAKAGE_DETECTION_NVKEY_STRUCT_H
#define _LEAKAGE_DETECTION_NVKEY_STRUCT_H

#include "types.h"

/**
 * @brief Parameter for Leakage detection
 * @KeyID 0xE700
 */
typedef struct {
    U8  ENABLE;                             /**< @Value 0x01 @Desc 1 */
    U8  REVISION;                           /**< @Value 0x01 @Desc 1 */

    S16 ld_thrd;                            /**< @Value   983 @Desc 1 */
    U16 AEC_MU_FAC;                         /**< @Value 0x399A @Desc 1 */
    //TX REF VAD
    S16 AEC_ENG_VAD_THR;                    /**< @Value 0x0a00 @Desc 1 */
    S16 AEC_ENG_VAD_HANG;                   /**< @Value 0x0005 @Desc 1 */
    S16 AEC_ENG_POW_INIT;                   /**< @Value 0x0400 @Desc 1 */
    S16 AEC_slow_up_alpha;                  /**< @Value 0x0021 @Desc 1 */
    S16 AEC_slow_dn_alpha;                  /**< @Value 0x4000 @Desc 1 */
    S16 AEC_fast_up_alpha;                  /**< @Value 0x6666 @Desc 1 */
    S16 AEC_fast_dn_alpha;                  /**< @Value 0x0666 @Desc 1 */

    U16 RXIN_TXREF_DELAY;                   /**< @Value 0x0005 @Desc 1 */
    U16 MIC_EC_DELAY;                       /**< @Value 0x0014 @Desc 1 */
    U16 AEC_MU_MIN;                         /**< @Value 0x1000 @Desc 1 */

    S16 DIGITAL_GAIN;                       /**< @Value 0x2000 @Desc 1 */
    S16 LPF_en;                             /**< @Value 0x0000 @Desc 1 */
    S32 tone_thr;                           /**< @Value 0x08000000 @Desc 1 */

    U16 reserved;                           /**< @Value 0x0000 @Desc 1 */
    U32 report_thd;                         /**< @Value 6000000 @Desc 1 */
    U32 no_response_thd;                    /**< @Value 7000000 @Desc 1 */

} PACKED anc_leakage_compensation_parameters_nvdm_t;


#endif /* _LEAKAGE_DETECTION_NVKEY_STRUCT_H */

