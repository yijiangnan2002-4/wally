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
#ifndef _DSP_PARA_AEC_NR_SWB_H_
#define _DSP_PARA_AEC_NR_SWB_H_

#include "types.h"

/**
 * @brief Parameter for SWB DSP AEC/NR algorithm
 * @KeyID 0xE156
 */

typedef struct stru_dsp_aec_nr_swb_para_s {
    U8  ENABLE;                                         /**< @Value 0x01 @Desc 1 */
    U8  REVISION;                                       /**< @Value 0x01 @Desc 1 */

    S16 CH5_REF_GAIN;       /**< @Value 0x00A0 @Desc 1 */
    S16 CH5_REF2_GAIN;      /**< @Value 0x00A0 @Desc 1 */
    S16 IE_CH5_REF_GAIN;    /**< @Value 0x028F @Desc 1 */

    S16 M2_VAD_THRD_14;             /**< @Value 0x2A3D @Desc 1 */
    S16 M2_VAD_THRD_24;             /**< @Value 0x15C3 @Desc 1 */
    S16 M2_VAD_THRD_34;             /**< @Value 0x0B85 @Desc 1 */
    S16 M2_VAD_THRD_44;             /**< @Value 0x051F @Desc 1 */
    U16 M2_PHASE_COMB_B5;       /**< @Value 0x0015 @Desc 1 */
    U16 M2_VAD4IN5;             /**< @Value 0x0000     @Desc 1 */

    S32 VOICE_SWB_IE_HP_COEF_0;                     /**< @Value 0x0076FE9F  @Desc 1 */
    S32 VOICE_SWB_IE_HP_COEF_1;                     /**< @Value 0xFF890301  @Desc 1 */
    S32 VOICE_SWB_IE_HP_COEF_2;                     /**< @Value 0x0076FE9E  @Desc 1 */
    S32 VOICE_SWB_IE_HP_COEF_3;                     /**< @Value 0xFF856EC7  @Desc 1 */
    S32 VOICE_SWB_IE_HP_COEF_4;                     /**< @Value 0x00758D42  @Desc 1 */
    S32 VOICE_SWB_IE_HP_COEF_5;                     /**< @Value 0x007F5D7F  @Desc 1 */
    S32 VOICE_SWB_IE_HP_COEF_6;                     /**< @Value 0xFF80AB5D  @Desc 1 */
    S32 VOICE_SWB_IE_HP_COEF_7;                     /**< @Value 0x007F5D7E  @Desc 1 */
    S32 VOICE_SWB_IE_HP_COEF_8;                     /**< @Value 0xFF80EB0D  @Desc 1 */
    S32 VOICE_SWB_IE_HP_COEF_9;                     /**< @Value 0x007E5FE2  @Desc 1 */
    S32 VOICE_SWB_IE_HP_COEF_10;                    /**< @Value 0x00400000  @Desc 1 */

    S32 VOICE_SWB_RX_HP_COEF_0;      /**< @Value 7798431  @Desc 1 */
    S32 VOICE_SWB_RX_HP_COEF_1;      /**< @Value -7798015 @Desc 1 */
    S32 VOICE_SWB_RX_HP_COEF_2;      /**< @Value 7798430  @Desc 1 */
    S32 VOICE_SWB_RX_HP_COEF_3;      /**< @Value -8032569 @Desc 1 */
    S32 VOICE_SWB_RX_HP_COEF_4;      /**< @Value 7703874  @Desc 1 */
    S32 VOICE_SWB_RX_HP_COEF_5;      /**< @Value 8347007  @Desc 1 */
    S32 VOICE_SWB_RX_HP_COEF_6;      /**< @Value -8344739 @Desc 1 */
    S32 VOICE_SWB_RX_HP_COEF_7;      /**< @Value 8347006  @Desc 1 */
    S32 VOICE_SWB_RX_HP_COEF_8;      /**< @Value -8328435 @Desc 1 */
    S32 VOICE_SWB_RX_HP_COEF_9;      /**< @Value 8282082  @Desc 1 */
    S32 VOICE_SWB_RX_HP_COEF_10;     /**< @Value 4194304  @Desc 1 */

    U16 SWB_NR_TX_VAD_THRD_BANDS_0;         /**< @Value 0x0b0b @Desc 1 */
    U16 SWB_NR_TX_VAD_THRD_BANDS_1;         /**< @Value 0x0b08 @Desc 1 */
    U16 SWB_NR_TX_VAD_THRD_BANDS_2;         /**< @Value 0x0808 @Desc 1 */
    U16 SWB_NR_TX_VAD_THRD_BANDS_3;         /**< @Value 0x0808 @Desc 1 */
    U16 SWB_NR_TX_VAD_THRD_BANDS_4;         /**< @Value 0x0a0a @Desc 1 */
    U16 SWB_NR_TX_VAD_THRD_BANDS_5;         /**< @Value 0x0700 @Desc 1 */
    U16 SWB_NR_TX_EMPH_COEF_0;              /**< @Value 0xFAB0 @Desc 1 */
    U16 SWB_NR_TX_EMPH_COEF_1;              /**< @Value 0xB5C3 @Desc 1 */

    U16 SWB_NR_RX_VAD_THRD_BANDS_0;         /**< @Value 0x0b0b @Desc 1 */
    U16 SWB_NR_RX_VAD_THRD_BANDS_1;         /**< @Value 0x0b08 @Desc 1 */
    U16 SWB_NR_RX_VAD_THRD_BANDS_2;         /**< @Value 0x0808 @Desc 1 */
    U16 SWB_NR_RX_VAD_THRD_BANDS_3;         /**< @Value 0x0808 @Desc 1 */
    U16 SWB_NR_RX_VAD_THRD_BANDS_4;         /**< @Value 0x0a0a @Desc 1 */
    U16 SWB_NR_RX_VAD_THRD_BANDS_5;         /**< @Value 0x0700 @Desc 1 */
    U16 SWB_NR_RX_EMPH_COEF_0;              /**< @Value 0xFAB0 @Desc 1 */
    U16 SWB_NR_RX_EMPH_COEF_1;              /**< @Value 0xB5C3 @Desc 1 */

    U8 SBEC_PF_order_5;         /**< @Value 0xFF @Desc 1 */
    U8 SBEC_DT_ratio_thrd_5;    /**< @Value 0x32 @Desc 1 */
    U8 SBEC_PF_MIN_5;           /**< @Value 0x5A @Desc 1 */
    U8 IE_SBEC_PF_order_5;      /**< @Value 0xFF @Desc 1 */
    U8 IE_SBEC_DT_ratio_thrd_5; /**< @Value 0x32 @Desc 1 */
    U8 IE_SBEC_PF_MIN_5;            /**< @Value 0x5A @Desc 1 */
    S16 M2_REF_MIC_CH5_GAIN;    /**< @Value 0x2000     @Desc 1 */
    U16 SWB_VOICE_TX_GAIN;      /**< @Value 0x0800 @Desc 1 */

    S16 M2_VAD_THRD_14_high;    /**< @Value 0x0 @Desc 1 */
    S16 M2_VAD_THRD_24_high;    /**< @Value 0x0 @Desc 1 */
    S16 M2_VAD_THRD_34_high;    /**< @Value 0x0 @Desc 1 */
    S16 M2_VAD_THRD_44_high;    /**< @Value 0x0 @Desc 1 */

    U16 SWB_NR_RX_NOISE_GAIN_LIMITER;   /**< @Value 0x1eb8 @Desc 1 */
    U16 SWB_MIC_EC_DELAY;               /**< @Value 0x0000 @Desc 1 */

    //Reserve
    U32 RESERVE_1;              /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_2;              /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_3;              /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_4;              /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_5;              /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_6;              /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_7;              /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_8;              /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_9;              /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_10;             /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_11;             /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_12;             /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_13;             /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_14;             /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_15;             /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_16;             /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_17;             /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_18;             /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_19;             /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_20;             /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_21;             /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_22;             /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_23;             /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_24;             /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_25;             /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_26;             /**< @Value 0x00000000 @Desc 1 */
    U32 RESERVE_27;             /**< @Value 0x00000000 @Desc 1 */
} PACKED DSP_PARA_AEC_NR_SWB_STRU;

#endif /* _DSP_PARA_AEC_NR_H_ */
