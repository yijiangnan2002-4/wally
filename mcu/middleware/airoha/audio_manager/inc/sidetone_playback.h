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

#ifndef __SIDETONE_PLAYBACK_H__
#define __SIDETONE_PLAYBACK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_audio.h"
#include "bt_sink_srv_ami.h"
#include "bt_sink_srv_am_task.h"
#include "audio_nvdm_common.h"
#include "hal_audio_cm4_dsp_message.h"
#include "audio_log.h"



/*********************************************************
                                        Macro
**********************************************************/

//#define SIDETONE_API_UT                 (0)
#define SIDETONE_BIQUAD_BAND_NUM_MAX    (10)

/*********************************************************
                                        Structure
**********************************************************/

typedef struct {
    uint32_t b0;    //Q31
    uint32_t b1;    //Q1.30
    uint32_t b2;    //Q31
    uint32_t a1;    //Q1.30
    uint32_t a2;    //Q31
} afe_sidetone_biquad_coef_t;

typedef struct {
    uint16_t flag;
    uint16_t band_num;
    uint32_t biquad_out_gain;
    afe_sidetone_biquad_coef_t biquad[SIDETONE_BIQUAD_BAND_NUM_MAX];
} afe_sidetone_biquad_t;

typedef enum {
    AFE_SIDETONE_AMIC0_L = 0,
    AFE_SIDETONE_AMIC0_R = 1,
    AFE_SIDETONE_AMIC1_L = 2,
    AFE_SIDETONE_AMIC1_R = 3,
    AFE_SIDETONE_AMIC2_L = 4,
    AFE_SIDETONE_AMIC2_R = 5,
    AFE_SIDETONE_STATIC_ZERO = 6,
    AFE_SIDETONE_DMIC0_L = 8,
    AFE_SIDETONE_DMIC0_R = 9,
    AFE_SIDETONE_DMIC1_L = 10,
    AFE_SIDETONE_DMIC1_R = 11,
    AFE_SIDETONE_DMIC2_L = 12,
    AFE_SIDETONE_DMIC2_R = 13,
    AFE_SIDETONE_MIC_DUMMY = 0x7FFFFFFF,
} afe_sidetone_mic_sel_t;


/*********************************************************
                                        Function
**********************************************************/

extern void audio_side_tone_disable_hdlr(bt_sink_srv_am_amm_struct *amm_ptr);
extern void audio_side_tone_enable_hdlr(bt_sink_srv_am_amm_struct *amm_ptr);
extern void audio_side_tone_set_volume_hdlr(bt_sink_srv_am_amm_struct *amm_ptr);

#ifdef __cplusplus
}
#endif

#endif  /*__SIDETONE_PLAYBACK_H__*/
