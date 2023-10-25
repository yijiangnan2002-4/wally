/* Copyright Statement:
 *
 * (C) 2017  Airoha Technology Corp. All rights reserved.
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
#ifndef _DSP_PARA_WIND_H_
#define _DSP_PARA_WIND_H_

#include "types.h"

/**
* @NvkeyDefine NVID_DSP_ALG_ANC_WIND_DET, NVID_DSP_ALG_ANC_WIND_DET_PT, NVID_DSP_ALG_ANC_WIND_DET_SIDETONE
* @brief Parameter for DSP Wind detection algorithm
* @KeyID 0xE300, 0xE306, 0xE307
*/

typedef struct stru_dsp_wind_para_s {
    //For algorithm
    U8 ENABLE;                      /**< @Value   0x01 @Desc 1 */
    U8 REVISION;                    /**< @Value   0x01 @Desc 1 */
    S16 _reserved0;                 /**< @Value      0 @Desc 0 */

    S16 option;                     /**< @Value      1 @Desc 0 for regular det mode.; 1 for continuous det mode.*/
    S16 w_thr0;                     /**< @Value  20000 @Desc 0.7; smooth wind first thr. (0 ~ 32767)        */
    S16 w_thr1;                     /**< @Value  11000 @Desc 0.25; smooth wind sec thr.  (0 ~ 32767)        */
    S16 wind_sm_len;                /**< @Value     30 @Desc smooth len for wind. (1 ~ 30)                  */
    S16 egy_sm_len;                 /**< @Value     30 @Desc smooth len for energy. (1 ~ 30)                */
    S16 vad_leave;                  /**< @Value     80 @Desc how many fr does RNN predict for each decision */
    S32 c_thr0;                     /**< @Value  15000000 @Desc smooth energy first thr                        */
    S32 c_thr1;                     /**< @Value   8500 @Desc smooth energy sec thr                          */
    S32 c_thr2;                     /**< @Value   6500 @Desc smooth energy thrd thr                         */

    //For middleware
    U16 _reserved;                  /**< @Value     0  @Desc Set 1 to print detect result*/
    S16 attenuation;                /**< @Value -3000  @Desc attenuation value of 0.01db*/
    U16 attack_count;               /**< @Value    30  @Desc attack counter for wind detection, unit:15ms */
    U16 release_count;              /**< @Value  2000  @Desc release counter for non-wind detection, unit:15ms */
    U16 pause_attenuation;          /**< @Value     0  @Desc Set 1 to suspend apply gain attenuation*/
    U16 _reserved2;                 /**< @Value     0  @Desc 0 */
    U16 _reserved3;                 /**< @Value     0  @Desc 0 */
    U16 _reserved4;                 /**< @Value     0  @Desc 0 */
    U16 _reserved5;                 /**< @Value     0  @Desc 0 */
    U16 _reserved6;                 /**< @Value     0  @Desc 0 */

    //2nd level of wind
    S16 attenuation_L2;             /**< @Value -3000  @Desc attenuation value of 0.01db*/
    U16 _reserved7;                 /**< @Value     0  @Desc 0 */
    U16 _reserved8;                 /**< @Value     0  @Desc 0 */
    S16 _reserved9;                 /**< @Value     0  @Desc 0.7; smooth wind first thr. (0 ~ 32767)        */
    S16 _reserved10;                /**< @Value     0  @Desc 0.25; smooth wind sec thr.  (0 ~ 32767)        */
    S16 _reserved11;                /**< @Value     0  @Desc smooth len for wind. (1 ~ 30)                  */
    S16 _reserved12;                /**< @Value     0  @Desc smooth len for energy. (1 ~ 30)                */
    S16 _reserved13;                /**< @Value     0  @Desc how many fr does RNN predict for each decision */
    S32 _reserved14;                /**< @Value     0  @Desc smooth energy first thr                        */
    S32 _reserved15;                /**< @Value     0  @Desc smooth energy sec thr                          */
    S32 _reserved16;                /**< @Value     0  @Desc smooth energy thrd thr                         */
    U16 _reserved17;                /**< @Value     0  @Desc 0 */
    U16 _reserved18;                /**< @Value     0  @Desc 0 */
    U16 _reserved19;                /**< @Value     0  @Desc 0 */
    U16 _reserved20;                /**< @Value     0  @Desc 0 */
} PACKED wind_detection_para_t;


#endif /* _DSP_PARA_WIND_H_ */

/*
Please sync the following files
\dsp\middleware\airoha\dspalg\wind_detection\inc\dsp_para_wind_nvkey_struct.h
\mcu\middleware\airoha\audio\anc_monitor\inc\dsp_para_wind_nvkey_struct.h
*/

