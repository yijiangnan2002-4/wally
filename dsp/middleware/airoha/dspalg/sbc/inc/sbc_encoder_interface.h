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

#ifndef _SBC_ENCODER_INTERFACE_H_
#define _SBC_ENCODER_INTERFACE_H_

/* Includes ------------------------------------------------------------------*/
#include "sbc_encoder.h"
#include "sbc_encoder_interface.h"
#include "types.h"
#include "dsp_feature_interface.h"

/* Public define -------------------------------------------------------------*/
#define SBC_ENC_USE_MSGID_SEND_LOG
#ifdef SBC_ENC_USE_MSGID_SEND_LOG
#define SBC_ENC_LOG_E(_message, arg_cnt, ...)  LOG_MSGID_E(sbc_enc_log,_message, arg_cnt, ##__VA_ARGS__)
#define SBC_ENC_LOG_W(_message, arg_cnt, ...)  LOG_MSGID_W(sbc_enc_log,_message, arg_cnt, ##__VA_ARGS__)
#define SBC_ENC_LOG_I(_message, arg_cnt, ...)  LOG_MSGID_I(sbc_enc_log,_message, arg_cnt, ##__VA_ARGS__)
#define SBC_ENC_LOG_D(_message, arg_cnt, ...)  LOG_MSGID_D(sbc_enc_log,_message, arg_cnt, ##__VA_ARGS__)
#else
#define SBC_ENC_LOG_E(_message, arg_cnt, ...)  LOG_E(sbc_enc_log,_message, ##__VA_ARGS__)
#define SBC_ENC_LOG_W(_message, arg_cnt, ...)  LOG_W(sbc_enc_log,_message, ##__VA_ARGS__)
#define SBC_ENC_LOG_I(_message, arg_cnt, ...)  LOG_I(sbc_enc_log,_message, ##__VA_ARGS__)
#define SBC_ENC_LOG_D(_message, arg_cnt, ...)  LOG_D(sbc_enc_log,_message, ##__VA_ARGS__)
#endif
/* SBC ABR Control */
#define AUDIO_A2DP_SBC_VBR_ENABLE               (1)

/* -------------------------------------------------- Bit Pool Presetting ------------------------------------------- */
/* Basic setting */
#define SBC_MIN_BITPOOL                         (2)
#define SBC_MAX_BITPOOL                         (250)

/* A2DP Spec Qos setting */
#define SBC_BITPOOL_MQ_MONO_44100               (19) /* 141kbps */
#define SBC_BITPOOL_MQ_MONO_48000               (18) /* 147kbps */
#define SBC_BITPOOL_HQ_MONO_44100               (31) /* 207kbps */
#define SBC_BITPOOL_HQ_MONO_48000               (29) /* 213kbps */

#define SBC_BITPOOL_MQ_JOINT_STEREO_44100       (35) /* 229kbps */
#define SBC_BITPOOL_MQ_JOINT_STEREO_48000       (33) /* 237kbps */
#define SBC_BITPOOL_HQ_JOINT_STEREO_44100       (53) /* 328kbps */
#define SBC_BITPOOL_HQ_JOINT_STEREO_48000       (51) /* 345kbps */

/* Super quality setting(Bluez) */
#define SBC_BITPOOL_SQ_JOINT_STEREO_44100       (86) /* 510kbps */
#define SBC_BITPOOL_SQ_JOINT_STEREO_48000       (78) /* 507kbps */

/* Fast stream setting */
#define SBC_BITPOOL_LQ_JOINT_STEREO_44100       (31) /* kbps */
#define SBC_BITPOOL_LQ_JOINT_STEREO_48000       (29) /* 212kbps */
#define SBC_BITPOOL_LQ_MONO_16000               (32) /* 72kbps */
/* ------------------------------------------------------------------------------------------------------------------ */

/* Public typedef ------------------------------------------------------------*/
typedef enum {
    SBC_ENCODER_ONE_SHOT_MODE = 0,
    SBC_ENCODER_BUFFER_MODE,
} sbc_enc_mode;
typedef struct {
    void *handle;
    sbc_enc_mode mode;
    sbc_encoder_initial_parameter_t param;
    sbc_encoder_runtime_parameter_t bit_pool;
    uint32_t working_buffer_size;
    uint16_t frame_cnt;
    uint16_t user_cnt;
    uint32_t input_working_buffer_size;
    uint32_t output_working_buffer_size;
    uint8_t  *input_working_buffer;
    uint8_t  *output_working_buffer;
    uint8_t  *internal_buffer_l;
    uint8_t  *internal_buffer_r;
    uint32_t internal_buffer_wo;
    uint32_t sbc_frame_size;
    uint32_t sbc_bit_stream_size;
    uint8_t  ScratchMemory[0];
} sbc_enc_handle_t;

typedef struct {
    sbc_encoder_initial_parameter_t param;
    sbc_encoder_runtime_parameter_t bit_pool;
    uint32_t input_working_buffer_size;
    uint32_t output_working_buffer_size;
    sbc_enc_mode mode;
    bool init;
} sbc_enc_param_config_t;
/* Public macro --------------------------------------------------------------*/
#define DSP_MEM_SBC_ECN_SIZE        (sizeof(sbc_enc_handle_t))
/* Public variables ----------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
void stream_codec_encoder_sbc_init(sbc_enc_param_config_t *config);
void stream_codec_encoder_sbc_deinit(void);

bool stream_codec_encoder_sbc_initialize(void *para);
bool stream_codec_encoder_sbc_process(void *para);
uint32_t stream_codec_encoder_sbc_get_bit_stream_size(void);
#if AUDIO_A2DP_SBC_VBR_ENABLE
void stream_codec_encoder_sbc_abr_control(uint32_t queue_length, uint32_t total_length);
#endif /* AUDIO_A2DP_SBC_VBR_ENABLE */
#endif /* _SBC_ENCODER_INTERFACE_H_ */
