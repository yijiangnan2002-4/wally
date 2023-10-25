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

#ifndef __HAL_AUDIO_NVKEY_STRUCT_H__
#define __HAL_AUDIO_NVKEY_STRUCT_H__

#include <stdint.h>

#define PACKED __attribute__((packed))

typedef struct HAL_AUDIO_DVFS_CLK_SELECT_s
{
    uint8_t HFP_DVFS_CLK; /* NVkey_0 0x00:26MHz 0x01:39MHz 0x02:78MHz 0x03:156MHz */
    uint8_t RESERVED_1;   /* NVkey_1 0x00 */
    uint8_t RESERVED_2;   /* NVkey_2 0x00 */
    uint8_t RESERVED_3;   /* NVkey_3 0x00 */
    uint8_t RESERVED_4;   /* NVkey_4 0x00 */
    uint8_t RESERVED_5;   /* NVkey_5 0x00 */
    uint8_t RESERVED_6;   /* NVkey_6 0x00 */
    uint8_t RESERVED_7;   /* NVkey_7 0x00 */
    uint8_t RESERVED_8;   /* NVkey_8 0x00 */
    uint8_t RESERVED_9;   /* NVkey_9 0x00 */
} PACKED HAL_AUDIO_DVFS_CLK_SELECT_t;

typedef struct {
    uint8_t  ENABLE;             /**< @Value   0x01 @Desc 1 */
    uint8_t  REVISION;           /**< @Value   0x01 @Desc 1 */
    uint16_t WWE_MODE;           /**< @Value 0x0000 @Desc 1 */
    uint16_t skip_frame_num ;    /**< @Value 0x0000 @Desc 1 */
    uint16_t noisy_thr_h ;       /**< @Value 0x0000 @Desc 1 */
    uint16_t noisy_thr_l ;       /**< @Value 0x0000 @Desc 1 */
    uint16_t noisy_debounce_cnt; /**< @Value 0x0000 @Desc 1 */
    uint16_t silent_thr_h ;      /**< @Value 0x0000 @Desc 1 */
    uint16_t silent_thr_l ;      /**< @Value 0x0000 @Desc 1 */
    uint16_t silent_debounce_cnt;/**< @Value 0x0000 @Desc 1 */
    uint16_t vit_pre ;           /**< @Value 0x0000 @Desc 1 */
    uint16_t RESERVE_1 ;         /**< @Value 0x0000 @Desc 1 */
    uint16_t RESERVE_2 ;         /**< @Value 0x0000 @Desc 1 */
    uint16_t RESERVE_3 ;         /**< @Value 0x0000 @Desc 1 */
    uint16_t RESERVE_4 ;         /**< @Value 0x0000 @Desc 1 */
} DSP_NVKEY_VAD_COMM;

typedef struct {
    int16_t en_vad;                /**< @Value 0x0000 @Desc 1 */
    int16_t gamma;                 /**< @Value 0x0000 @Desc 1 */
    int16_t diff_fac;              /**< @Value 0x0000 @Desc 1 */
    int16_t diff_facb;             /**< @Value 0x0000 @Desc 1 */
    int16_t vad_fac;               /**< @Value 0x0000 @Desc 1 */
    int16_t vad_ph_cc;             /**< @Value 0x0000 @Desc 1 */
    int16_t vad_ph_init;           /**< @Value 0x0000 @Desc 1 */
    int16_t vad_ph_range;          /**< @Value 0x0000 @Desc 1 */
    int16_t vad_corr_th;           /**< @Value 0x0000 @Desc 1 */
    int16_t vad_pitch_th;          /**< @Value 0x0000 @Desc 1 */
    int16_t win_sm_1st;            /**< @Value 0x0000 @Desc 1 */
    int16_t sil_thr   ;            /**< @Value 0x0000 @Desc 1 */
    int16_t sil_thr2  ;            /**< @Value 0x0000 @Desc 1 */
    int16_t fr_thr    ;            /**< @Value 0x0000 @Desc 1 */
    int16_t fr_thr2   ;            /**< @Value 0x0000 @Desc 1 */
    int16_t rec_thr   ;            /**< @Value 0x0000 @Desc 1 */
    int16_t rec_thr2  ;            /**< @Value 0x0000 @Desc 1 */
    int16_t end_thr   ;            /**< @Value 0x0000 @Desc 1 */
    int16_t gar_thr   ;            /**< @Value 0x0000 @Desc 1 */
    int16_t rel_thr   ;            /**< @Value 0x0000 @Desc 1 */
    int16_t ss_thr    ;            /**< @Value 0x0000 @Desc 1 */
    int16_t ms_thr    ;            /**< @Value 0x0000 @Desc 1 */
    int16_t ss_rel_thr;            /**< @Value 0x0000 @Desc 1 */
    int16_t ss_g_thr  ;            /**< @Value 0x0000 @Desc 1 */
    int16_t fa_end_thr;            /**< @Value 0x0000 @Desc 1 */
    int16_t fa_beg_thr;            /**< @Value 0x0000 @Desc 1 */
    int16_t short_thr ;            /**< @Value 0x0000 @Desc 1 */
    int16_t rel_st4_thr;           /**< @Value 0x0000 @Desc 1 */
    int16_t rel_oth_thr;           /**< @Value 0x0000 @Desc 1 */
    int16_t dur_thr   ;            /**< @Value 0x0000 @Desc 1 */
    int16_t max_dur_thr;           /**< @Value 0x0000 @Desc 1 */
    int16_t ms_s_thr  ;            /**< @Value 0x0000 @Desc 1 */
    int16_t vad_leave_th;          /**< @Value 0x0000 @Desc 1 */
    int16_t vad_eng_thr;           /**< @Value 0x0000 @Desc 1 */
    uint16_t RESERVE_A ;            /**< @Value 0x0000 @Desc 1 */
    uint16_t RESERVE_B ;            /**< @Value 0x0000 @Desc 1 */
    uint16_t RESERVE_C ;            /**< @Value 0x0000 @Desc 1 */
    uint16_t RESERVE_D ;            /**< @Value 0x0000 @Desc 1 */
} DSP_NVKEY_VAD_PARA;

typedef struct
{
    /*NVkey_0 0x00:0x0303 0x01:0x1313
              0x02:0x2323 0x03:0x3333
              0x04:0x4343 0x05:0x5353
              0x06:0x6363 0x07:0x7373*/
    uint8_t snr_threshold;
    uint8_t noise_ignore_bits; /*NVkey_1 0x01:0xFFFF0000 0x02:0xFFF00000 0x03:0xFF000000 0x04:0xF0000000 */
    uint8_t alpha_rise;        /*NVkey_3 0x01 - 0x0F */
    uint8_t enable;            /*NVkey_4 0x00:diable 0x01:enable*/
    uint16_t main_mic;         /*NVkey_5 vow use main mic type*/
    uint16_t ref_mic;          /*NVkey_6 vow use ref mic type*/
    uint16_t main_interface;   /*NVkey_7 vow use main interface type*/
    uint16_t ref_interface;    /*NVkey_8 vow use ref interface type*/
}DSP_NVKEY_VOW_PARA;

#endif /* __HAL_AUDIO_NVKEY_STRUCT_H__ */
