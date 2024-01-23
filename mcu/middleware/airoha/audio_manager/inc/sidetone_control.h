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

#ifndef __SIDETONE_CONTROL_H__
#define __SIDETONE_CONTROL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bt_sink_srv_ami.h"
#include "audio_nvdm_common.h"
#include "audio_log.h"


#define SIDETONE_GAIN_MAGIC_NUM 0xDEADBEEF

typedef enum {
    ST_MECH_FIR_IND = 0,  //Sidetone independent FIR method.
    ST_MECH_IIR_IND,      //Sidetone independent IIR method.
    ST_MECH_FIR_EXTEND,   //Sidetone extend FIR method.
    ST_MECH_IIR_EXTEND,   //Sidetone extend IIR method. Need FB join or not.

    ST_MECH_DUMMY = 0xFF, // 1 byte
} sidetone_mechanism_t;

typedef enum {
    ST_EXECUTION_FAIL    = -1,
    ST_EXECUTION_SUCCESS =  0,
} sidetone_control_result_t;

typedef enum {
    SIDETONE_SCENARIO_COMMON,
    SIDETONE_SCENARIO_HFP,
    SIDETONE_SCENARIO_BLE,
    SIDETONE_SCENARIO_ULL_V1,
    SIDETONE_SCENARIO_ULL_V2,
    SIDETONE_SCENARIO_DUAL_ANT,
    SIDETONE_SCENARIO_AMA,
    SIDETONE_SCENARIO_GSOUND,
    SIDETONE_SCENARIO_LINE_IN,
    SIDETONE_SCENARIO_USB_AUDIO,
    SIDETONE_SCENARIO_BTVA,
    SIDETONE_SCENARIO_RESERVE,
    SIDETONE_SCENARIO_MAX = 15,
} sidetone_scenario_t;


typedef struct {
    uint8_t     sidetone_method;
    uint8_t     sidetone_source_sel_en; // 0:talk mic ; 1:source_sel.     bit[0]:sideTone_source_sel_0  ; bit[1]:sideTone_source_sel_1
    uint8_t     sideTone_source_sel_0;
    uint8_t     sideTone_source_sel_1;
    uint8_t     Reserved[60];
} PACKED sidetone_nvdm_param_t;


typedef struct {
    uint8_t                 enable;
    sidetone_mechanism_t    sidetone_method;
    uint8_t                 sidetone_source_sel_en; // 0:talk mic ; 1:source_sel.     bit[0]:sideTone_source_sel_0  ; bit[1]:sideTone_source_sel_1
    uint8_t                 sideTone_source_sel_0;
    uint8_t                 sideTone_source_sel_1;
} sidetone_control_t;

sidetone_control_result_t am_audio_side_tone_init(void);
/**
 * @brief                   This function is employed to enable side tone.
 * @return                  AUD_EXECUTION_SUCCESS on success or AUD_EXECUTION_FAIL on failure
 */
extern sidetone_control_result_t am_audio_side_tone_enable(void);
extern sidetone_control_result_t am_audio_side_tone_enable_by_scenario(sidetone_scenario_t scenario);

/**
 * @brief                   The API is use to change the side tone gain value with db
 *
 * @param                   side_tone_gain
 * @param                   scenario
 * @return                  AUD_EXECUTION_SUCCESS on success or AUD_EXECUTION_FAIL on failure
 */
sidetone_control_result_t am_audio_side_tone_set_volume_by_scenario(sidetone_scenario_t scenario, int32_t side_tone_gain);

/**
 * @brief                   The API is use to get the side tone gain value with db
 *
 * @param                   scenario
 * @return                  side tone gain value with db
 */
int32_t am_audio_side_tone_get_volume_by_scenario(sidetone_scenario_t scenario);


/**
 * @brief                   The API is use to get the side tone method.
 *
 * @param                  void
 * @return                  Sidetone Filter method.(IIR/FIR/ANC_IIR/ANC_FIR)
 */
sidetone_mechanism_t am_audio_side_tone_query_method(void);


/**
 * @brief                   This function is employed to disable side tone.
 * @return                  AUD_EXECUTION_SUCCESS on success or AUD_EXECUTION_FAIL on failure
 */
extern sidetone_control_result_t am_audio_side_tone_disable(void);
extern sidetone_control_result_t am_audio_side_tone_disable_by_scenario(sidetone_scenario_t scenario);

sidetone_control_result_t ami_sidetone_enable_user_config_mode(int32_t user_volume);
sidetone_control_result_t ami_sidetone_disable_user_config_mode(void);
uint32_t ami_sidetone_adjust_gain_by_user_config(uint32_t sidetone_gain);
bool am_audio_side_tone_get_status(void);

#ifdef __cplusplus
}
#endif

#endif  /*__SIDETONE_CONTROL_H__*/
