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

#ifndef _AUDIO_ANC_VIVID_PT_API_H__
#define _AUDIO_ANC_VIVID_PT_API_H__

#include "types.h"
#include "stdbool.h"
#include "audio_log.h"
#include "audio_anc_llf_control_internal.h"


#ifdef __cplusplus
extern "C" {
#endif

#ifndef UNUSED
#define UNUSED(x)  ((void)(x))
#endif
#ifndef PACKED
#define PACKED  __attribute__((packed))
#endif

/**
 * @addtogroup Audio
 * @{
 * @addtogroup VividPT
 * @{
 *
 * The Vivid PT is used for Vivid Pass Through.
 *
 * @section vivid_pt_api_usage How to use this module
 *
 * - An example on how to use the Vivid PT APIs.
 *   - Step1: Call audio_anc_vivid_pt_init() to initialize vivid PT module.
 *   - Step2: Call llf_control_register_callback() to register the callback for vivid PT OPEN/CLOSE event.
 *   - Step3: Call audio_anc_control_enable() to open the vivid PT path.
 *   - Step4: Wait for vivid PT OPEN event callback.
 *   - Step5: Call audio_anc_vivid_pt_control_afc_set_switch() to enable/disable the AFC function.
 *   - Step6: Call audio_anc_vivid_pt_control_nr_set_switch() to enable/disable the NR function.
 *   - Step7: Call audio_anc_control_disable() to close the vivid PT path.
 *
 *   - Sample code:
 *   @code
 *
 *
 *   audio_anc_vivid_pt_init();//Initialize vivid PT module.
 *   llf_control_register_callback((llf_control_callback_t)app_hear_through_control_callback,
 *                                       LLF_CONTROL_EVENT_ON | LLF_CONTROL_EVENT_OFF,
 *                                       LLF_MAX_CALLBACK_LEVEL_ALL);
 *
 *
 *   { //Open the vivid PT framework
 *             U32 mic_config;
 *             audio_anc_vivid_pt_get_mic_input_path(&mic_config);
 *
 *             audio_anc_control_result_t         control_ret;
 *             audio_anc_control_filter_id_t      target_filter_id;
 *             audio_anc_control_type_t           target_anc_type;
 *             audio_anc_control_misc_t           local_misc = {0};
 *             local_misc.type_mask_param.ANC_path_mask = AUDIO_ANC_CONTROL_RAMP_FF_L |
 *                                                        AUDIO_ANC_CONTROL_RAMP_FB_L; //vivid PT with hybrid ANC effect
 *             target_anc_type      = AUDIO_ANC_CONTROL_TYPE_PT_SW_VIVID | mic_config;
 *
 *             target_filter_id     = AUDIO_ANC_CONTROL_SW_VIVID_PASS_THRU_FILTER_DEFAULT; //1~4
 *             control_ret = audio_anc_control_enable(target_filter_id, target_anc_type, &local_misc);
 *   }
 *
 *   void app_hear_through_control_callback(llf_type_t type, llf_control_event_t event, llf_status_t result)
 *   {
 *      if ((type == AUDIO_LLF_TYPE_VIVID_PT) && (event == LLF_CONTROL_EVENT_ON))
 *      {
 *          audio_anc_vivid_pt_control_afc_set_switch(true);//enable the AFC function.
 *          audio_anc_vivid_pt_control_nr_set_switch(true);//enable the NR function.
 *      }
 *   }
 *
 *
 *   audio_anc_control_disable(NULL); //Close the vivid PT framework
 *   @endcode
 */


/** @defgroup VividPT_define Define
 *  @{
 */

/** @brief This defines the audio vivid PT return types */
#define    AUDIO_VIVID_PT_STATUS_NONE      (LLF_STATUS_NONE)
#define    AUDIO_VIVID_PT_STATUS_FAIL      (LLF_STATUS_FAIL)    /**< The Vivid PT function return failed.   */
#define    AUDIO_VIVID_PT_STATUS_SUCCESS   (LLF_STATUS_SUCCESS)   /**< The Vivid PT function return was successful.   */
#define    AUDIO_VIVID_PT_STATUS_CANCEL    (LLF_STATUS_CANCEL)   /**< The Vivid PT function return was cancelled.   */


/**
* @}
*/

/* ======================================================
/          Low Latency Framework API
========================================================*/
/**
 * @brief     This function is used to register the callback for Vivid PT OPEN & CLOSE events.
 * @param[in] callback is the callback function pointer.
 * @param[in] event_mask is to select which event needs callback.
 * @param[in] level is the level mask in LLF callback service.
 * @return    Return LLF_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
extern llf_status_t llf_control_register_callback(llf_control_callback_t callback, llf_control_event_t event_mask, llf_control_callback_level_t level);

/* ======================================================
/          Vivid PT API
========================================================*/
/**
 * @brief     This function is used to initialize vivid PT.
 */
void audio_anc_vivid_pt_init(void);

/**
 * @brief     This function is used to get vivid PT mic path configuration.
 * @param[out] *sel would be the mask for the mic path configuration.
 */
void audio_anc_vivid_pt_get_mic_input_path(U32 *sel);

/**
 * @brief     This function is used to enable the AFC (Acoustic Feedback Cancellation) function.
 * @param[in] enable is the switch.
 * @return    Return AUDIO_VIVID_PT_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_vivid_pt_status_t audio_anc_vivid_pt_control_afc_set_switch(bool enable);

/**
 * @brief     This function is used to get the status of the AFC (Acoustic Feedback Cancellation) function.
 * @param[out] *enable is the switch.
 * @return    Return AUDIO_VIVID_PT_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_vivid_pt_status_t audio_anc_vivid_pt_control_afc_get_switch(bool *enable);

/**
 * @brief     This function is used to enable the NR (Noise reduction) function.
 * @param[in] enable is the switch.
 * @return    Return AUDIO_VIVID_PT_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_vivid_pt_status_t audio_anc_vivid_pt_control_nr_set_switch(bool enable);

/**
 * @brief     This function is used to get the status of the NR (Noise reduction) function.
 * @param[out] *enable is the switch.
 * @return    Return AUDIO_VIVID_PT_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_vivid_pt_status_t audio_anc_vivid_pt_control_nr_get_switch(bool *enable);

/**
 * @brief     This function is used to bypass the algorithm function.
 * @param[in] enable is the switch.
 * @return    Return AUDIO_VIVID_PT_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_vivid_pt_status_t audio_anc_vivid_pt_control_bypass_set_switch(bool enable);

/**
 * @brief     This function is used to get the status of bypassing the algorithm function.
 * @param[out] *enable is the switch.
 * @return    Return AUDIO_VIVID_PT_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_vivid_pt_status_t audio_anc_vivid_pt_control_bypass_get_switch(bool *enable);

/**
 * @brief     This function is used to save user setting.
 * @return    Return AUDIO_VIVID_PT_STATUS_SUCCESS if the operation is successful. Otherwise, an error occurred.
 */
audio_vivid_pt_status_t audio_anc_vivid_pt_control_save_setting(void);

#ifdef __cplusplus
}
#endif

/**
* @}
* @}
*/
#endif  /*_AUDIO_ANC_VIVID_PT_API_H__*/

