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

#ifndef __RECORD_CONTROL_H__
#define __RECORD_CONTROL_H__

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "hal_audio_internal.h"
#include "hal_audio_message_struct.h"
#include "record_playback.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AUDIO_RECORD_REGISTER_ID_TOTAL   7  /*BISTO, AMA, Leakage, UserTriggerFF*3, Reserve*/

typedef int8_t record_id_t;

typedef enum {
    RECORD_CONTROL_EXECUTION_FAIL    = -1,
    RECORD_CONTROL_EXECUTION_SUCCESS =  0,
} record_control_result_t;

typedef struct {
    audio_dsp_codec_type_t codec_type;
    encoder_bitrate_t bit_rate;
    wwe_mode_t wwe_mode;
    uint32_t wwe_language_mode_address;
    uint32_t wwe_language_mode_length;
} record_encoder_cability_t;

typedef struct {
    record_id_t am_id;
    record_encoder_cability_t encoder_codec;
    void *Reserve_callback;
    void *Reserve_callback_user_data;
    bool is_used;
} record_control_aud_id_type_t;

void record_init(void);

/**
 * @brief     This function is to init the record middelware.
 *               Including: register AM ID, hook CCNI Event,
 *               hang callback function which would be used to communicate between AM and User.
 *
 * @param[in] ccni_callback     is the CCNI callback which hook in hal level.
 * @param[in] user_data         is the handler in CCNI event.
 * @param[in] cb_handler        is the callback when AM want to notice User. (Task Queue id would be better.) (for now bt_sink_srv_am_notify_callback)
 * @return    record_id_t         AM ID.
 * @sa        #audio_record_control_init()
 */
record_id_t audio_record_control_init(hal_audio_stream_in_callback_t ccni_callback,
                                      void *user_data,
                                      void *cb_handler);

/**
 * @brief     This function is to init the record middelware.
 *               Including: register AM ID, hook CCNI Event,
 *               hang callback function which would be used to communicate between AM and User.
 *
 * @param[in] ccni_callback     is the CCNI callback which hook in hal level.
 * @param[in] user_data         is the handler in CCNI event.
 * @param[in] cb_handler        is the callback when AM want to notice User. (Task Queue id would be better.) (for now bt_sink_srv_am_notify_callback)
 * @param[in] encoder_capability        is the encodeer setting about encode type and bitrate.
 * @return    record_id_t         AM ID.
 * @sa        #audio_record_control_init()
 */
record_id_t audio_record_control_enabling_encoder_init(hal_audio_stream_in_callback_t ccni_callback,
                                                       void *user_data,
                                                       void *cb_handler,
                                                       record_encoder_cability_t *encoder_capability);



/**
 * @brief     This function is to deinit the record middelware.
 *               Including: de-register AM ID, unhook CCNI Event.
 *
 * @param[in] aud_id     AM ID.
 * @return      RECORD_CONTROL_EXECUTION_SUCCESS on success or RECORD_CONTROL_EXECUTION_FAIL on failure
 * @sa        #audio_record_control_deinit()
 */
record_control_result_t audio_record_control_deinit(record_id_t aud_id);

/**
 * @brief     This function is to start record.
 *               Send start record event to Audio_Manager.
 *
 * @param[in] aud_id     AM ID.
 * @return      RECORD_CONTROL_EXECUTION_SUCCESS on success or RECORD_CONTROL_EXECUTION_FAIL on failure
 * @sa        #audio_record_control_start()
 */
record_control_result_t audio_record_control_start(record_id_t aud_id);

/**
 * @brief     This function is to stop record.
 *               Send stop record event to Audio_Manager.
 *
 * @param[in] aud_id     AM ID.
 * @return      RECORD_CONTROL_EXECUTION_SUCCESS on success or RECORD_CONTROL_EXECUTION_FAIL on failure
 * @sa        #audio_record_control_stop()
 */
record_control_result_t audio_record_control_stop(record_id_t aud_id);

/**
 * @brief     This function is to read record Shared Buffer.
 *               Call Hal API.
 *
 * @param[in] *buffer     Buffer Address which we want to copy.
 * @param[in] size         Size which we want to copy.
 * @return      RECORD_CONTROL_EXECUTION_SUCCESS on success or RECORD_CONTROL_EXECUTION_FAIL on failure
 * @sa        #audio_record_control_read_data()
 */
record_control_result_t audio_record_control_read_data(void *buffer, uint32_t size);

/**
 * @brief     This function is to get record Shared Buffer data count.
 *               Call Hal API.
 *
 * @return   Shared Buffer data count
 * @sa        #audio_record_control_get_share_buf_data_byte_count()
 */

uint32_t audio_record_control_get_share_buf_data_byte_count(void);

record_control_result_t audio_record_control_airdump(bool Enable, uint8_t dump_scenario);
#ifdef __cplusplus
}
#endif

#endif  /*__RECORD_CONTROL_H__*/
