/* Copyright Statement:
 *
 * (C) 2019  Airoha Technology Corp. All rights reserved.
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

#ifndef __RECORD_PLAYBACK_H__
#define __RECORD_PLAYBACK_H__

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "hal_audio_internal.h"
#include "hal_audio_message_struct.h"
#include "hal_audio.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief  This defines the audio stream in callback function prototype.
 *          Register a callback function while audio stream in path is turned on.
 *          For more details about the callback, please refer to #hal_audio_register_stream_in_callback().
 *  @param[in] event is the value defined in #hal_audio_event_t. This parameter is given by the driver to notify the user about the data flow processing behavior.
 *  @param[in] user_data is a user defined parameter provided by #hal_audio_register_stream_in_callback() function.
 */
typedef void (*hal_audio_stream_in_callback_t)(hal_audio_event_t event, void *user_data);

/**
 * @brief     Power on the audio in
 * @param[in] active_type is to set input function type. For more details, please refer to #hal_audio_active_type_t.
 * @return    #HAL_AUDIO_STATUS_OK, if input path type is valid.
 * @sa        #hal_audio_stop_stream_in()
 * @par       Example
 * @code      hal_audio_start_stream_in(HAL_AUDIO_RECORD_VOICE);
 * @endcode
 */
hal_audio_status_t hal_audio_start_stream_in(hal_audio_active_type_t active_type);

/**
 * @brief     Power off the stream in path
 * @sa        #hal_audio_start_stream_in()
 * @par       Example
 * @code      hal_audio_stop_stream_in();
 * @endcode
 */
void hal_audio_stop_stream_in(void);

/**
 * @brief     Receive data from the audio input
 * @param[in] buffer is a pointer to a user defined buffer.
 * @param[in] sample_count is the amount of data received (in bytes).
 * @return    #HAL_AUDIO_STATUS_OK, if OK.
 * @sa        #hal_audio_get_stream_in_sample_count()
 */
hal_audio_status_t hal_audio_read_stream_in(void *buffer, uint32_t sample_count);

/**
 * @brief      Query the available input data sample count.
 * @param[out] sample_count the available amount of received data (in bytes).
 * @return     #HAL_AUDIO_STATUS_OK, if OK.
 * @note       Call this function before #hal_audio_read_stream_in() to check the content availability in an input buffer.
 * @sa         #hal_audio_read_stream_in()
 */
hal_audio_status_t hal_audio_get_stream_in_sample_count(uint32_t *sample_count);

/**
 * @brief     Register the callback function for input data
 * @param[in] callback is the function pointer of callback for input data control.
 * @param[in] user_data is extended parameter for user.
 * @return    #HAL_AUDIO_STATUS_OK, if OK.
 * @par       Example
 * @code      hal_audio_register_stream_in_callback(stream_in_callback, user_data);
 * @endcode
 */
hal_audio_status_t hal_audio_register_stream_in_callback(hal_audio_stream_in_callback_t callback, void *user_data);

/**
 * @brief     Power on the audio in
 * @return    #HAL_AUDIO_STATUS_OK, if input path type is valid.
 * @sa        #hal_audio_start_stream_in_leakage_compensation()
 * @par       Example
 * @code      hal_audio_start_stream_in_leakage_compensation();
 * @endcode
 */
hal_audio_status_t hal_audio_start_stream_in_leakage_compensation(void);

/**
 * @brief     Power off the stream in path
 * @sa        #hal_audio_stop_stream_in_leakage_compensation()
 * @par       Example
 * @code      hal_audio_stop_stream_in_leakage_compensation();
 * @endcode
 */
void hal_audio_stop_stream_in_leakage_compensation(void);
/**
 * @brief     Power on the audio in
 * @return    #HAL_AUDIO_STATUS_OK, if input path type is valid.
 * @sa        #hal_audio_start_stream_in_user_trigger_adaptive_ff()
 * @par       Example
 * @code      hal_audio_start_stream_in_user_trigger_adaptive_ff();
 * @endcode
 */
hal_audio_status_t hal_audio_start_stream_in_user_trigger_adaptive_ff(uint8_t mode);
/**
 * @brief     Power off the stream in path
 * @sa        #hal_audio_stop_stream_in_user_trigger_adaptive_ff()
 * @par       Example
 * @code      hal_audio_stop_stream_in_user_trigger_adaptive_ff();
 * @endcode
 */
void hal_audio_stop_stream_in_user_trigger_adaptive_ff(void);

#ifdef __cplusplus
}
#endif

#endif  /*__RECORD_PLAYBACK_H__*/
