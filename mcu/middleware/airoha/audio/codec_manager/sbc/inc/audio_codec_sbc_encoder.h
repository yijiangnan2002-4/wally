/* Copyright Statement:
 *
 * (C) 2020  Airoha Technology Corp. All rights reserved.
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

#ifndef __AUDIO_CODEC_SBC_ENCODER_H__
#define __AUDIO_CODEC_SBC_ENCODER_H__

#include <stdbool.h>
#include "sbc_encoder.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SBC_CODEC_SUCCESS = 0,
    SBC_CODEC_ERROR = -1,
    SBC_CODEC_LEN_ERROR = -2,
    SBC_CODEC_PARAM_ERROR = -3
} sbc_codec_result_t;

typedef struct {
    uint32_t *sbc_encoder_working_buffer_ptr;
    sbc_encoder_initial_parameter_t param;
    sbc_encoder_runtime_parameter_t runtime_param;
    bool is_skip_header;
} sbc_encoder_config_t;


int32_t audio_codec_sbc_encoder_get_working_buffer_length(void *codec_config, uint32_t *length);
sbc_codec_result_t audio_codec_sbc_encoder_get_bitpool_from_bitrate(uint32_t bitrate, uint32_t *bitpool, void *codec_config);
int32_t audio_codec_sbc_encoder_get_inout_frame_length(void *codec_config, uint32_t *oneframe_input_byte_length, uint32_t *oneframe_output_byte_length);
int32_t audio_codec_sbc_encoder_init(void *codec_config);
int32_t audio_codec_sbc_encoder_process_direct(void *codec_config, uint8_t *input_buffer, uint32_t *input_byte_length, uint8_t *output_buffer, uint32_t *output_byte_length);
int32_t audio_codec_sbc_encoder_deinit(void);





#ifdef __cplusplus
}
#endif

#endif//__AUDIO_CODEC_SBC_ENCODER_H__