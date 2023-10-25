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
#ifndef _DSP_PARA_NR_H_
#define _DSP_PARA_NR_H_

#include "types.h"

/**
 * @brief Parameter for DSP NR algorithm
 * @KeyID 0xE154
 */

typedef struct stru_dsp_nr_s {
    U16 AVC_ALPHA;                                          /**< @Value 0x799A @Desc 1 */
    U16 AVC_THRD;                                               /**< @Value 0x0037 @Desc 1 */
    U16 AVC_VOL_MAX;                                    /**< @Value 0x7FFF @Desc 1 */

    U16 WB_NR_TX_POW_MIN_BUF_PERIOD;        /**< @Value 0x000B @Desc 1 */
    U16 WB_NR_TX_NOISE_GAIN_LIMITER;    /**< @Value 4078 @Desc 1 */
    U16 WB_NR_TX_VAD_THRD1;             /**< @Value 0x0348 @Desc 1 */
    U16 WB_NR_TX_VAD_THRD_BANDS_0;      /**< @Value 0x0B0B @Desc 1 */
    U16 WB_NR_TX_VAD_THRD_BANDS_1;      /**< @Value 0x0B08 @Desc 1 */
    U16 WB_NR_TX_VAD_THRD_BANDS_2;      /**< @Value 0x0808 @Desc 1 */
    U16 WB_NR_TX_VAD_THRD_BANDS_3;      /**< @Value 0x0808 @Desc 1 */
    U16 WB_NR_TX_VAD_THRD_BANDS_4;      /**< @Value 0x0A07 @Desc 1 */
    U16 WB_NR_TX_OVER_SUPPRESS_FAC;     /**< @Value 0x219A @Desc 1 */
    U16 WB_NR_TX_SPEECH_RETAIN_FAC;     /**< @Value 0x599A @Desc 1 */
    U16 WB_NR_TX_NOISE_LAMDA;           /**< @Value 0x799A @Desc 1 */
    U16 WB_NR_TX_NOISE_FLOOR_MIN;       /**< @Value 0x8400 @Desc 1 */
    U16 WB_NR_FAST_ALPHA;                   /**< @Value 0x747B @Desc 1 */
    U16 WB_NR_SLOW_ALPHA;                   /**< @Value 0x7C29 @Desc 1 */
    U16 WB_NR_NOISE_UPDATE_FAST;        /**< @Value 6554     @Desc 1 */
    U16 WB_NR_NOISE_UPDATE_SLOW;        /**< @Value 983      @Desc 1 */
    U16 WB_NR_NOISE_UPDATE_ULTRASLOW;   /**< @Value 131      @Desc 1 */
    U16 WB_NR_TX_EMPH_COEF_0;                       /**< @Value 0xAE82 @Desc 1 */
    U16 WB_NR_TX_EMPH_COEF_1;                       /**< @Value 0x3DD9 @Desc 1 */
    U16 NB_NR_RX_POW_MIN_BUF_PERIOD;    /**< @Value 0x000D @Desc 1 */
    U16 NB_NR_RX_NOISE_GAIN_LIMITER;    /**< @Value 0x1EB8 @Desc 1 */
    U16 NB_NR_RX_VAD_THRD1;             /**< @Value 0x0348 @Desc 1 */
    U16 NB_NR_RX_VAD_THRD_BANDS_0;          /**< @Value 0x0b0b @Desc 1 */
    U16 NB_NR_RX_VAD_THRD_BANDS_1;          /**< @Value 0x0b08 @Desc 1 */
    U16 NB_NR_RX_VAD_THRD_BANDS_2;          /**< @Value 0x0808 @Desc 1 */
    U16 NB_NR_RX_VAD_THRD_BANDS_3;          /**< @Value 0x0807 @Desc 1 */
    U16 NB_NR_RX_OVER_SUPPRESS_FAC;     /**< @Value 0x2666 @Desc 1 */
    U16 NB_NR_RX_SPEECH_RETAIN_FAC;     /**< @Value 0x6000 @Desc 1 */
    U16 NB_NR_RX_NOISE_LAMDA;           /**< @Value 0x799a @Desc 1 */
    U16 NB_NR_RX_NOISE_FLOOR_MIN;       /**< @Value 0xae00 @Desc 1 */
    U16 NB_NR_RX_EMPH_COEF_0;                       /**< @Value 0xC8C0 @Desc 1 */
    U16 NB_NR_RX_EMPH_COEF_1;                       /**< @Value 0x24AF @Desc 1 */
    U16 WB_NR_RX_POW_MIN_BUF_PERIOD;        /**< @Value 0x000b @Desc 1 */
    U16 WB_NR_RX_NOISE_GAIN_LIMITER;        /**< @Value 0x1eb8 @Desc 1 */
    U16 WB_NR_RX_VAD_THRD1;                         /**< @Value 0x0348 @Desc 1 */
    U16 WB_NR_RX_VAD_THRD_BANDS_0;          /**< @Value 0x0b0b @Desc 1 */
    U16 WB_NR_RX_VAD_THRD_BANDS_1;          /**< @Value 0x0b08 @Desc 1 */
    U16 WB_NR_RX_VAD_THRD_BANDS_2;          /**< @Value 0x0808 @Desc 1 */
    U16 WB_NR_RX_VAD_THRD_BANDS_3;          /**< @Value 0x0808 @Desc 1 */
    U16 WB_NR_RX_VAD_THRD_BANDS_4;          /**< @Value 0x0a07 @Desc 1 */
    U16 WB_NR_RX_OVER_SUPPRESS_FAC;         /**< @Value 0x2800 @Desc 1 */
    U16 WB_NR_RX_SPEECH_RETAIN_FAC;         /**< @Value 0x6000 @Desc 1 */
    U16 WB_NR_RX_NOISE_LAMDA;                       /**< @Value 0x799a @Desc 1 */
    U16 WB_NR_RX_NOISE_FLOOR_MIN;               /**< @Value 0xae00 @Desc 1 */
    U16 WB_NR_RX_EMPH_COEF_0;                       /**< @Value 0xCA16 @Desc 1 */
    U16 WB_NR_RX_EMPH_COEF_1;                       /**< @Value 0x0B29 @Desc 1 */
    U16 PD_NR_TX_OPT;                                       /**< @Value      0 @Desc 1 */
    U16 PD_PEAK_BUF_SIZE;                               /**< @Value      0 @Desc 1 */
    U16 PD_PITCH_VAD_THRD;                          /**< @Value  20480 @Desc 1 */
    U16 PD_PITCH_FAC;                                       /**< @Value  31949 @Desc 1 */
    U16 PD_PEAK_RATIO_FAC;                          /**< @Value  30032 @Desc 1 */
    U16 PD_TRANSIENT_THRD;                          /**< @Value 0x0046 @Desc 1 */
    U16 PD_TRANSIENT_AUG;                               /**< @Value   2048 @Desc 1 */
    U16 PD_PITCH_AUG;                                       /**< @Value    328 @Desc 1 */
    U16 PD_TRANSIENT_THRD2;                         /**< @Value 0x0028 @Desc 1 */
    U16 PD_RESERVE2;                                        /**< @Value      0 @Desc 1 */

    U16 M2_MICDIST;                                         /**< @Value 0x0000 @Desc 1 */
    U16 M2_WIND_THRD;                                       /**< @Value 0x1000 @Desc 1 */
    U16 M2_WIND_TRANS;                                  /**< @Value 0x0640 @Desc 1 */
    U16 M2_WIND_BLOCK;                                  /**< @Value 0x1000 @Desc 1 */
    U16 M2_FILTER_GRD;                                  /**< @Value 0x0002 @Desc 1 */
    U16 M2_DISTANCE_FAC;                                /**< @Value 0x6119 @Desc 1 */
    U16 M2_VAD_THRD_1;                                  /**< @Value 0x1062 @Desc 1 */
    U16 M2_VAD_THRD_2;                                  /**< @Value 0x0C4A @Desc 1 */
    U16 M2_VAD_THRD_3;                                  /**< @Value 0x0852 @Desc 1 */
    U16 M2_VAD_THRD_4;                                  /**< @Value 0x03D7 @Desc 1 */
    U16 M2_VAD_THRD_12;                                 /**< @Value 0x1333 @Desc 1 */
    U16 M2_VAD_THRD_22;                                 /**< @Value 0x0E14 @Desc 1 */
    U16 M2_VAD_THRD_32;                                 /**< @Value 0x0666 @Desc 1 */
    U16 M2_VAD_THRD_42;                                 /**< @Value 0x028F @Desc 1 */
    U16 M2_VAD_THRD_13;                                 /**< @Value 0x2A3D @Desc 1 */
    U16 M2_VAD_THRD_23;                                 /**< @Value 0x15C3 @Desc 1 */
    U16 M2_VAD_THRD_33;                                 /**< @Value 0x0B85 @Desc 1 */
    U16 M2_VAD_THRD_43;                                 /**< @Value 0x051F @Desc 1 */
    U16 M2_NORM_FAC1;                                       /**< @Value 0x0000 @Desc 1 */
    U16 M2_PHASE_COMB;                                  /**< @Value 0x5C02 @Desc 1 */
    U16 M2_PHASE_GAIN;                                  /**< @Value 0x4000 @Desc 1 */
    U16 M2_NE_DURATION;                                 /**< @Value 0x0038 @Desc 1 */
    U16 M2_BEAM1_NORM_LIMIT;                        /**< @Value 0x6000 @Desc 1 */
    U16 M2_OVER_SUPPRESS_FAC2;                  /**< @Value 0x2000 @Desc 1 */
    S16 M2_BAND1_ALPHA;                                 /**< @Value 0x7800 @Desc 1 */
    S16 M2_BAND2_ALPHA;                                 /**< @Value 0x7D71 @Desc 1 */
    S16 M2_BAND3_ALPHA;                                 /**< @Value 0x7F80 @Desc 1 */
    S16 M2_BANDH_ALPHA;                                 /**< @Value 0x7F5C @Desc 1 */

    S16 OFF_PITCH_ALPHA;                                /**< @Value 0x0106 @Desc 1 */
    S16 WIND_DETECT_THRD;                               /**< @Value 0x0014 @Desc 1 */
    S16 PD_WIND_FREQ_RANGE;                         /**< @Value 0x0011 @Desc 1 */
    S16 PD_WIND_LOW_ATTEN;                          /**< @Value 0x65AD @Desc 1 */
    S16 PD_NONSTATIONARY_ALPHA_1;               /**< @Value 0x0106 @Desc 1 */
    S16 PD_NONSTATIONARY_ALPHA_2;               /**< @Value 0x028F @Desc 1 */
    S16 M2_PD_HIGH_ATTEN_GOAL;                  /**< @Value 13045       @Desc 1 */
    S32 M2_PD_SEVERE_NOISE_THRD;                /**< @Value 8388352 @Desc 1 */

    S16 NR_HIGH_ATTEN_DIVIDE;                   /**< @Value 109         @Desc 1 */
    U16 WB_VOICE_TX_GAIN;                               /**< @Value 0x0800 @Desc 1 */
    U16 NB_VOICE_TX_GAIN;                               /**< @Value 0x0800 @Desc 1 */
    S16 PD_VOICE_IN_WIND;               /**< @Value 0x7FFF     @Desc 1 */
    S16 PD_VOICE_AMP_LEVEL;             /**< @Value 0x4000     @Desc 1 */
    S16 M2_HF_NSG2_OVER;                  /**< @Value 0x0000       @Desc 1 */
    S16 M2_DF_RATIO;                                        /**< @Value 0x6CCD     @Desc 1 */
    S16 M2_POW1_ALPHA;                                  /**< @Value 0x0CCD     @Desc 1 */
    S16 M2_POW1_UP_GAIN;                                /**< @Value 0x0028     @Desc 1 */
    S16 M2_POW1_DOWN_GAIN;                          /**< @Value 0x00A4     @Desc 1 */
    S16 M2_POW1_THRD1;                                  /**< @Value 0x30A4     @Desc 1 */
    S16 M2_POW1_THRD2;                                  /**< @Value 0x1333     @Desc 1 */
    U16 M2_VAD3IN4;                                         /**< @Value 0x0000     @Desc 1 */
    S16 M2_POW1_VAD_RATIO;                          /**< @Value 0x4000     @Desc 1 */
    U16 M2_DEEMPH_COEF_1;                               /**< @Value 0x5A3D     @Desc 1 */
    U16 M2_DEEMPH_COEF_2;                               /**< @Value 0x6536     @Desc 1 */
    U16 M2_DEEMPH_COEF_3;                               /**< @Value 0xD3A9     @Desc 1 */
    S16 M2_BAND2_GAIN;                                  /**< @Value 0x2D3F     @Desc 1 */
    S16 M2_BAND3_GAIN;                                  /**< @Value 0x4000     @Desc 1 */
    S16 M2_BANDH_GAIN;                                  /**< @Value 0x2000     @Desc 1 */
    S16 M2_REF_MIC_CH1_GAIN;                        /**< @Value 0x2000     @Desc 1 */
    S16 M2_REF_MIC_CH2_GAIN;                        /**< @Value 0x2000     @Desc 1 */
    S16 M2_REF_MIC_CH3_GAIN;                        /**< @Value 0x2000     @Desc 1 */
    S16 M2_REF_MIC_CH4_GAIN;                        /**< @Value 0x2000     @Desc 1 */
    S16 Mel_scale_band_num;                         /**< @Value 0x0000     @Desc 1 */
    S16 Harmonics_thrd;                                 /**< @Value 0x0000     @Desc 1 */
    U16 M2_lp_coef_attu;                /**< @Value       0x0 @Desc 1 */
    U16 M2_lp_thrd;                     /**< @Value       0x10 @Desc 1 */
    S16 M2_lp_norm_thrd;                /**< @Value       0x0 @Desc 1 */
    S16 low_noise_thrd;                 /**< @Value         0 @Desc low_noise_detection=1 @ (low_noise_thrd!=0), low_noise_detection=0 @ (low_noise_thrd==0) */
    S16 low_noise_slow_alpha;           /**< @Value       163 @Desc 1 */
    S16 low_noise_fast_alpha;           /**< @Value      1638 @Desc 1 */
    S16 HPF_COEF_SW;                    /**< @Value    0x0000 @Desc 1 */

    S16 high_noise_thrd;				/**< @Value   4000 @Desc 1 */
    S16 low_bin_switch_end;				/**< @Value      0 @Desc 1 */
    S16 dual_switch_noise_thrd;  		/**< @Value 4000   @Desc 1 */
    U16 M2_VAD_THRD_1_high ;    		/**< @Value 0x1062 @Desc 1 */
    U16 M2_VAD_THRD_2_high ;     		/**< @Value 0x0C4A @Desc 1 */
    U16 M2_VAD_THRD_3_high ;     		/**< @Value 0x0852 @Desc 1 */
    U16 M2_VAD_THRD_4_high ;     		/**< @Value 0x03D7 @Desc 1 */
    U16 M2_VAD_THRD_12_high;			/**< @Value 0x1333 @Desc 1 */
    U16 M2_VAD_THRD_22_high;   			/**< @Value 0x0E14 @Desc 1 */
    U16 M2_VAD_THRD_32_high;	   		/**< @Value 0x0666 @Desc 1 */
    U16 M2_VAD_THRD_42_high;			/**< @Value 0x028F @Desc 1 */
    U16 M2_VAD_THRD_13_high;	   		/**< @Value 0x2A3D @Desc 1 */
    U16 M2_VAD_THRD_23_high;  			/**< @Value 0x15C3 @Desc 1 */
    U16 M2_VAD_THRD_33_high;	   		/**< @Value 0x0B85 @Desc 1 */
    U16 M2_VAD_THRD_43_high;   			/**< @Value 0x051F @Desc 1 */

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
    U16 SW_BYPASS_AVC;   /**< @Value 0x0000 @Desc 1 */
} PACKED DSP_PARA_NR_STRU;

#endif /* _DSP_PARA_NR_H_ */

