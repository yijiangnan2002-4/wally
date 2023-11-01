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

#ifndef _STREAM_NVKEY_STRUCT_H_
#define _STREAM_NVKEY_STRUCT_H_

/* Includes ------------------------------------------------------------------*/
/* Public define -------------------------------------------------------------*/
/* Public typedef ------------------------------------------------------------*/
#ifdef AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
typedef struct {
    int32_t     enable;
    int32_t     effective_threshold_db;
    uint32_t    effective_delay_ms;
    int32_t     failure_threshold_db;
    uint32_t    failure_delay_ms;
    int32_t     adjustment_amount_db;
    int32_t     up_step_db;
    int32_t     down_step_db;
    DSP_PARA_GAME_CHAT_VOL_STRU chat_vol_nvkey;
} gaming_mode_vol_balance_nvkey_t;
#endif /* AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE */

#ifdef AIR_SILENCE_DETECTION_ENABLE
typedef struct {
    int32_t     enable;
    int32_t     effective_threshold_db;
    uint32_t    effective_delay_ms;
    int32_t     failure_threshold_db;
    uint32_t    failure_delay_ms;
    int32_t     reserved_word0;
    int32_t     reserved_word1;
    int32_t     reserved_word2;
    DSP_PARA_GAME_CHAT_VOL_STRU chat_vol_nvkey;
} silence_detection_nvkey_t;
#endif /* AIR_SILENCE_DETECTION_ENABLE */

#ifdef AIR_VOLUME_ESTIMATOR_ENABLE
typedef struct {
    int32_t     enable;
    int32_t     effective_threshold_db;
    uint32_t    effective_delay_ms;
    int32_t     failure_threshold_db;
    uint32_t    failure_delay_ms;
    int32_t     reserved_word0;
    int32_t     reserved_word1;
    int32_t     reserved_word2;
    DSP_PARA_GAME_CHAT_VOL_STRU chat_vol_nvkey;
} audio_spectrum_meter_nvkey_t;
#endif /* AIR_VOLUME_ESTIMATOR_ENABLE */

/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/

#endif /* _STREAM_NVKEY_STRUCT_H_ */
