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

/* Includes ------------------------------------------------------------------*/
#include "sbc_encoder_interface.h"
#include "sbc_encoder_portable.h"
#include "dsp_sdk.h"
#include "dsp_dump.h"
#include "preloader_pisplit.h"
#include "scenario_audio_common.h"
#include "dsp_callback.h"
#include "dsp_memory.h"
#include "hal_gpt.h"
#include "common.h"
/* Private define ------------------------------------------------------------*/
#define SBC_ENCODER_DEBUG_LOG_ENABLE                     (0)
/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
sbc_enc_handle_t *sbc_enc_handle = NULL;
static sbc_enc_param_config_t sbc_enc_config = {0};
/* Public variables ----------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
log_create_module(sbc_enc_log, PRINT_LEVEL_INFO);

static void stream_codec_encoder_sbc_bitrate_init(sbc_encoder_initial_parameter_t *param);
/* Public functions ----------------------------------------------------------*/

void stream_codec_encoder_sbc_init(sbc_enc_param_config_t *config)
{
    if (sbc_enc_config.init) {
        SBC_ENC_LOG_W("sbc enc library config is already done", 0);
    } else {
        memcpy(&sbc_enc_config, config, sizeof(sbc_enc_param_config_t));
    }
}

void stream_codec_encoder_sbc_deinit(void)
{
    sbc_enc_handle->user_cnt --;
    if (sbc_enc_handle->user_cnt == 0) {
        if (sbc_enc_handle->param.channel_mode != SBC_ENCODER_MONO) {
            free(sbc_enc_handle->input_working_buffer);
            sbc_enc_handle->input_working_buffer = NULL;
        }
        if (sbc_enc_handle->internal_buffer_l != NULL) {
            free(sbc_enc_handle->internal_buffer_l);
            sbc_enc_handle->internal_buffer_l = NULL;
        }
        if (sbc_enc_handle->internal_buffer_r != NULL) {
            free(sbc_enc_handle->internal_buffer_r);
            sbc_enc_handle->internal_buffer_r = NULL;
        }
        free(sbc_enc_handle);
        sbc_enc_handle = NULL;
        memset(&sbc_enc_config, 0, sizeof(sbc_enc_param_config_t));
    }
}

void stream_codec_encoder_sbc_check_init(void)
{
    if ((!sbc_enc_handle) || (sbc_enc_handle->user_cnt == 0)) {
        /* Config parameters */
        if (!sbc_enc_config.init) {
            assert(0 && "Error: sbc enc lib has no get config func");
        }
        uint32_t sbc_enc_buffer_size = sbc_enc_get_buffer_size();
        sbc_enc_handle = (sbc_enc_handle_t *)malloc(DSP_MEM_SBC_ECN_SIZE + sbc_enc_buffer_size);
        memset(sbc_enc_handle, 0, sizeof(sbc_enc_handle_t));
        SBC_ENC_LOG_I("sbc enc library config successfully", 0);

        memcpy(&(sbc_enc_handle->param), &(sbc_enc_config.param), sizeof(sbc_encoder_initial_parameter_t));
        stream_codec_encoder_sbc_bitrate_init(&(sbc_enc_config.param));
        sbc_enc_handle->working_buffer_size        = sbc_enc_buffer_size;
        sbc_enc_handle->input_working_buffer_size  = sbc_enc_config.input_working_buffer_size;
        sbc_enc_handle->output_working_buffer_size = sbc_enc_config.output_working_buffer_size;
        sbc_enc_handle->internal_buffer_l          = NULL;
        sbc_enc_handle->internal_buffer_r          = NULL;
        sbc_enc_handle->internal_buffer_wo         = 0;
        sbc_enc_handle->mode                       = sbc_enc_config.mode;
        sbc_enc_handle->sbc_frame_size             = sbc_enc_config.input_working_buffer_size;
        sbc_enc_handle->sbc_bit_stream_size        = 0;
    } else {
        SBC_ENC_LOG_W("sbc enc library config already", 0);
    }

    sbc_enc_handle->user_cnt ++;
}

uint32_t stream_codec_encoder_sbc_get_bit_stream_size(void)
{
    if (!sbc_enc_handle) {
        return 0;
    }
    return sbc_enc_handle->sbc_bit_stream_size;
}

bool stream_codec_encoder_sbc_initialize(void *para)
{
    UNUSED(para);
    // DSP_STREAMING_PARA_PTR stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    // malloc for sbc handle
    stream_codec_encoder_sbc_check_init();
    if (sbc_enc_handle->handle) {
        SBC_ENC_LOG_W("sbc enc library is already init", 0);
    } else {
        uint32_t sbc_frame_size = sbc_enc_handle->sbc_frame_size;  // fixed 16bit
        int32_t  result         = 0;
        uint32_t in_size        = sbc_enc_handle->input_working_buffer_size;
        uint32_t out_size       = sbc_enc_handle->output_working_buffer_size;
        result = sbc_encoder_init(&(sbc_enc_handle->handle), &(sbc_enc_handle->ScratchMemory[0]), &(sbc_enc_handle->param));
        if (result < 0) {
            SBC_ENC_LOG_E("sbc enc library is init fail, error %d", 1, result);
            assert(0);
        }
        if (sbc_enc_handle->param.channel_mode == SBC_ENCODER_MONO) {
            sbc_enc_handle->input_working_buffer_size = in_size;
            sbc_enc_handle->output_working_buffer_size = out_size;
            sbc_enc_handle->internal_buffer_l = (uint8_t *)malloc(sbc_frame_size);
        } else {
            sbc_enc_handle->input_working_buffer_size = in_size * 2; // dual channel
            sbc_enc_handle->input_working_buffer = (uint8_t *)malloc(sbc_enc_handle->input_working_buffer_size);
            if (!sbc_enc_handle->input_working_buffer) {
                assert(0 && "sbc enc malloc input working buffer fail");
            }
            sbc_enc_handle->output_working_buffer_size = out_size; // sbc is always one channel
            // sbc_enc_handle->output_working_buffer = (uint8_t *)malloc(sbc_enc_handle->output_working_buffer_size);
            // if (!sbc_enc_handle->output_working_buffer) {
            //     assert(0 && "sbc enc malloc output working buffer fail");
            // }
        }
        if (sbc_enc_handle->mode == SBC_ENCODER_BUFFER_MODE) {
            sbc_enc_handle->internal_buffer_l = (uint8_t *)malloc(sbc_frame_size);
            if (!sbc_enc_handle->internal_buffer_l) {
                assert(0 && "sbc enc malloc internal working buffer l fail");
            }
            sbc_enc_handle->internal_buffer_r = (uint8_t *)malloc(sbc_frame_size);
            if (!sbc_enc_handle->internal_buffer_r) {
                assert(0 && "sbc enc malloc internal working buffer r fail");
            }
        }
        SBC_ENC_LOG_I("sbc enc library init success: handle[0x%x] mode %d sample_rate %d block %d s_band number %d mode %d method %d bit pool %d in_ptr 0x%x out_ptr 0x%x in_size %d out_size %d working_buffer size %d sbc frame %d l 0x%x r 0x%x wo %d", 17,
            sbc_enc_handle->handle,
            sbc_enc_handle->mode,
            sbc_enc_handle->param.sampling_rate,
            sbc_enc_handle->param.block_number,
            sbc_enc_handle->param.subband_number,
            sbc_enc_handle->param.channel_mode,
            sbc_enc_handle->param.allocation_method,
            sbc_enc_handle->bit_pool.bitpool_value,
            sbc_enc_handle->input_working_buffer,
            sbc_enc_handle->output_working_buffer,
            sbc_enc_handle->input_working_buffer_size,
            sbc_enc_handle->output_working_buffer_size,
            sbc_enc_handle->working_buffer_size,
            sbc_frame_size,
            sbc_enc_handle->internal_buffer_l,
            sbc_enc_handle->internal_buffer_r,
            sbc_enc_handle->internal_buffer_wo
            );
    }
    return FALSE;
}

// Attention: this ip can't support processing mono data channel by channel.

ATTR_TEXT_IN_IRAM_LEVEL_2 bool stream_codec_encoder_sbc_process(void *para)
{
    uint32_t channel_number      = stream_function_get_channel_number(para);
    uint32_t length              = stream_codec_get_input_size(para);
    uint32_t out_put_buffer_size = sbc_enc_handle->output_working_buffer_size;
    if (stream_codec_get_resolution(para) == RESOLUTION_32BIT) {
        assert(0 && "sbc enc lib doesn't support 32bit format");
    }
    if (sbc_enc_handle->mode != SBC_ENCODER_BUFFER_MODE) {
        if (sbc_enc_handle->param.channel_mode == SBC_ENCODER_MONO) {
            sbc_enc_handle->input_working_buffer = stream_codec_get_input_buffer(para, 1);
            sbc_enc_handle->output_working_buffer = stream_codec_get_output_buffer(para, 1);
        } else {
            uint8_t *input_buffer_l  = stream_codec_get_input_buffer(para, 1);
            //uint8_t *output_buffer_l = stream_codec_get_output_buffer(para, 1);
            uint8_t *input_buffer_r  = stream_codec_get_input_buffer(para, 2);
            //uint8_t *output_buffer_r = stream_codec_get_output_buffer(para, 2);
            // LLL + RRR -> LRLRLR
            if (channel_number == 1) {
                // LLL -> LL LL LL
                input_buffer_r  = input_buffer_l;
            }
            ShareBufferCopy_D_16bit_to_I_16bit_2ch((uint16_t *)input_buffer_l, (uint16_t *)input_buffer_r, (uint16_t *)sbc_enc_handle->input_working_buffer,
                sbc_enc_handle->input_working_buffer_size >> 2);
        }
        int32_t result = 0;
        // DUMP Codec Input Buffer
    #ifdef AIR_AUDIO_DUMP_ENABLE
        LOG_AUDIO_DUMP(sbc_enc_handle->input_working_buffer, sbc_enc_handle->input_working_buffer_size, AUDIO_BT_SRC_DONGLE_DL_ENC_IN);
    #endif
    #if SBC_ENCODER_DEBUG_LOG_ENABLE
        uint32_t count1, count2, count3;
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &count1);
        SBC_ENC_LOG_I("sbc enc process enter, handle 0x%x input_buffer 0x%x in_size %d output_buffer 0x%x out_size %d bit_pool %d mode %d", 7,
            sbc_enc_handle->handle,
            sbc_enc_handle->input_working_buffer,
            sbc_enc_handle->input_working_buffer_size,
            sbc_enc_handle->output_working_buffer,
            out_put_buffer_size,
            sbc_enc_handle->bit_pool,
            sbc_enc_handle->mode
            );
    #endif
        result = sbc_encoder_process(
            sbc_enc_handle->handle,
            (int16_t *)sbc_enc_handle->input_working_buffer,
            &(sbc_enc_handle->input_working_buffer_size),
            sbc_enc_handle->output_working_buffer,
            &(out_put_buffer_size),
            &(sbc_enc_handle->bit_pool)
            );
        if (result < 0) {
            SBC_ENC_LOG_E("sbc enc process fail, error %d frame %d", 2, result, sbc_enc_handle->frame_cnt + 1);
            assert(0);
        } else {
    #if SBC_ENCODER_DEBUG_LOG_ENABLE
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &count2);
            hal_gpt_get_duration_count(count1, count2, &count3);
            SBC_ENC_LOG_I("sbc enc process exit, handle 0x%x input_buffer 0x%x in_size %d output_buffer 0x%x out_size %d bit_pool %d time %d", 7,
                sbc_enc_handle->handle,
                sbc_enc_handle->input_working_buffer,
                sbc_enc_handle->input_working_buffer_size,
                sbc_enc_handle->output_working_buffer,
                out_put_buffer_size,
                sbc_enc_handle->bit_pool,
                count3
                );
    #endif
        }
        sbc_enc_handle->frame_cnt ++;
        // DUMP Codec Output Buffer
    #ifdef AIR_AUDIO_DUMP_ENABLE
        LOG_AUDIO_DUMP(sbc_enc_handle->output_working_buffer, out_put_buffer_size, AUDIO_BT_SRC_DONGLE_DL_ENC_OUT);
    #endif
        if (channel_number == 2) {
            // LRLRLR -> LLL + RRR
            // ShareBufferCopy_I_16bit_to_D_16bit_2ch(sbc_enc_handle->output_working_buffer, )
            memcpy(stream_codec_get_output_buffer(para, 1), sbc_enc_handle->output_working_buffer, out_put_buffer_size);
        }
        // change next params for next feature: resolution, output size, input size, channel number etc.
        stream_codec_modify_output_size(para, out_put_buffer_size);
    } else {
        /* buffer mode */
        uint32_t sbc_frame_size  = sbc_enc_handle->sbc_frame_size;  // fixed 16bit
        uint32_t process_times   = (length + sbc_enc_handle->internal_buffer_wo) / sbc_frame_size;
        uint32_t extern_wo       = (length + sbc_enc_handle->internal_buffer_wo) % sbc_frame_size;
        uint8_t  *input_buffer_l = NULL;
        uint8_t  *input_buffer_r = NULL;
        uint32_t out_put_size    = 0;
        uint32_t out_put_buffer_size = 0;
        sbc_enc_handle->output_working_buffer = stream_codec_get_output_buffer(para, 1);
        if (sbc_enc_handle->param.channel_mode == SBC_ENCODER_MONO) {
            input_buffer_l = stream_codec_get_input_buffer(para, 1);
            if (sbc_enc_handle->internal_buffer_wo > 0) {
                memmove((uint8_t *)(input_buffer_l + sbc_enc_handle->internal_buffer_wo), input_buffer_l, length);
                memcpy(input_buffer_l, sbc_enc_handle->internal_buffer_l, sbc_enc_handle->internal_buffer_wo);
            }
        } else {
            input_buffer_l  = stream_codec_get_input_buffer(para, 1);
            input_buffer_r  = stream_codec_get_input_buffer(para, 2);
            if (sbc_enc_handle->internal_buffer_wo > 0) {
                memmove((uint8_t *)(input_buffer_l + sbc_enc_handle->internal_buffer_wo), input_buffer_l, length);
                memcpy(input_buffer_l, sbc_enc_handle->internal_buffer_l, sbc_enc_handle->internal_buffer_wo);
                if (channel_number > 1) {
                    memmove((uint8_t *)(input_buffer_r + sbc_enc_handle->internal_buffer_wo), input_buffer_r, length);
                    memcpy(input_buffer_r, sbc_enc_handle->internal_buffer_r, sbc_enc_handle->internal_buffer_wo);
                }
            }
        }
        for (uint32_t i = 0; i < process_times; i ++) {
            if (sbc_enc_handle->param.channel_mode == SBC_ENCODER_MONO) {
                sbc_enc_handle->input_working_buffer = input_buffer_l + (i * sbc_frame_size);
                sbc_enc_handle->input_working_buffer_size = sbc_frame_size;
            } else {
                input_buffer_l = (stream_codec_get_input_buffer(para, 1) + i * sbc_frame_size);
                input_buffer_r = (stream_codec_get_input_buffer(para, 2) + i * sbc_frame_size);
                sbc_enc_handle->input_working_buffer_size = sbc_frame_size * 2;
                if (channel_number == 1) {
                    // LLL -> LL LL LL
                    input_buffer_r  = input_buffer_l;
                }
                ShareBufferCopy_D_16bit_to_I_16bit_2ch((uint16_t *)input_buffer_l, (uint16_t *)input_buffer_r, (uint16_t *)sbc_enc_handle->input_working_buffer,
                    sbc_enc_handle->input_working_buffer_size >> 2);
            }
            int32_t result = 0;
            // DUMP Codec Input Buffer
            #ifdef AIR_AUDIO_DUMP_ENABLE
                LOG_AUDIO_DUMP(sbc_enc_handle->input_working_buffer, sbc_enc_handle->input_working_buffer_size, AUDIO_BT_SRC_DONGLE_DL_ENC_IN);
            #endif
            #if SBC_ENCODER_DEBUG_LOG_ENABLE
                uint32_t count1, count2, count3;
                hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &count1);
                SBC_ENC_LOG_I("sbc enc process enter, handle 0x%x input_buffer 0x%x in_size %d output_buffer 0x%x out_size %d bit_pool %d mode %d", 7,
                    sbc_enc_handle->handle,
                    sbc_enc_handle->input_working_buffer,
                    sbc_enc_handle->input_working_buffer_size,
                    sbc_enc_handle->output_working_buffer,
                    sbc_enc_handle->output_working_buffer_size,
                    sbc_enc_handle->bit_pool,
                    sbc_enc_handle->mode
                    );
            #endif
            out_put_buffer_size = sbc_enc_handle->output_working_buffer_size;
            result = sbc_encoder_process(
                sbc_enc_handle->handle,
                (int16_t *)sbc_enc_handle->input_working_buffer,
                &(sbc_enc_handle->input_working_buffer_size),
                sbc_enc_handle->output_working_buffer,
                &(out_put_buffer_size),
                &(sbc_enc_handle->bit_pool)
                );
            if (result < 0) {
                SBC_ENC_LOG_E("sbc enc process fail, error %d frame %d", 2, result, sbc_enc_handle->frame_cnt + 1);
                assert(0);
            } else {
            #if SBC_ENCODER_DEBUG_LOG_ENABLE
                hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &count2);
                hal_gpt_get_duration_count(count1, count2, &count3);
                SBC_ENC_LOG_I("sbc enc process exit, handle 0x%x input_buffer 0x%x in_size %d output_buffer 0x%x out_size %d bit_pool %d time %d", 7,
                    sbc_enc_handle->handle,
                    sbc_enc_handle->input_working_buffer,
                    sbc_enc_handle->input_working_buffer_size,
                    sbc_enc_handle->output_working_buffer,
                    out_put_buffer_size,
                    sbc_enc_handle->bit_pool,
                    count3
                    );
            #endif
            }
            sbc_enc_handle->frame_cnt ++;
            if (channel_number == 2) {
                // LRLRLR -> LLL + RRR
                // ShareBufferCopy_I_16bit_to_D_16bit_2ch(sbc_enc_handle->output_working_buffer, )
                memcpy(stream_codec_get_output_buffer(para, 2), sbc_enc_handle->output_working_buffer, out_put_buffer_size);
            }
            /* 4B Align */
            sbc_enc_handle->output_working_buffer += (out_put_buffer_size + 3) / 4 * 4;
            sbc_enc_handle->sbc_bit_stream_size = out_put_buffer_size;
            out_put_size += (out_put_buffer_size + 3) / 4 * 4;
        }
        if (extern_wo > 0) {
            memcpy(sbc_enc_handle->internal_buffer_l, (uint8_t *)(stream_codec_get_input_buffer(para, 1) + process_times * sbc_frame_size), extern_wo);
            if (sbc_enc_handle->param.channel_mode != SBC_ENCODER_MONO) {
                if (channel_number > 1) {
                    memcpy(sbc_enc_handle->internal_buffer_r, (uint8_t *)(stream_codec_get_input_buffer(para, 2) + process_times * sbc_frame_size), extern_wo);
                }
            }
        }
        sbc_enc_handle->internal_buffer_wo = extern_wo;
        // change next params for next feature: resolution, output size, input size, channel number etc.
        stream_codec_modify_output_size(para, out_put_size);
        #ifdef AIR_AUDIO_DUMP_ENABLE
            LOG_AUDIO_DUMP(stream_codec_get_input_buffer(para, 1), out_put_size, AUDIO_BT_SRC_DONGLE_DL_ENC_OUT);
        #endif
    }
    return FALSE;
}

/* |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| */
/* ----------------------------------------------- SBC ABR Control -------------------------------------------------- */
static void stream_codec_encoder_sbc_bitrate_init(sbc_encoder_initial_parameter_t *param)
{
    uint8_t min_bit_pool = 0;
    uint8_t max_bit_pool = 0;
    uint8_t pre_max_bit_pool = 0;
    uint8_t pre_min_bit_pool = 0;

    if (!sbc_enc_handle) {
        return;
    }
    if (param->max_bit_pool < param->min_bit_pool) {
        AUDIO_ASSERT(0 && "sbc enc bit pool error");
    }
    /* get the preset setting of bitrate */
    switch (param->sampling_rate) {
        case SBC_ENCODER_SAMPLING_RATE_48000HZ:
            pre_min_bit_pool = SBC_BITPOOL_LQ_JOINT_STEREO_48000;
            if (param->max_bit_pool >= SBC_BITPOOL_SQ_JOINT_STEREO_48000) {
                pre_max_bit_pool = SBC_BITPOOL_SQ_JOINT_STEREO_48000;
            } else if (param->max_bit_pool >= SBC_BITPOOL_HQ_JOINT_STEREO_48000) {
                pre_max_bit_pool = SBC_BITPOOL_HQ_JOINT_STEREO_48000;
            } else if (param->max_bit_pool >= SBC_BITPOOL_MQ_JOINT_STEREO_48000) {
                pre_max_bit_pool = SBC_BITPOOL_MQ_JOINT_STEREO_48000;
            } else if (param->max_bit_pool >= SBC_BITPOOL_LQ_JOINT_STEREO_48000) {
                pre_max_bit_pool = SBC_BITPOOL_LQ_JOINT_STEREO_48000;
            } else {
                SBC_ENC_LOG_E("[sbc enc] max bitpool is too small %d %d", 2,
                    param->max_bit_pool,
                    param->min_bit_pool
                    );
                pre_max_bit_pool = param->max_bit_pool;
                pre_min_bit_pool = param->min_bit_pool;
            }
            break;
        case SBC_ENCODER_SAMPLING_RATE_44100HZ:
            pre_min_bit_pool = SBC_BITPOOL_LQ_JOINT_STEREO_44100;
            if (param->max_bit_pool >= SBC_BITPOOL_SQ_JOINT_STEREO_44100) {
                pre_max_bit_pool = SBC_BITPOOL_SQ_JOINT_STEREO_44100;
            } else if (param->max_bit_pool >= SBC_BITPOOL_HQ_JOINT_STEREO_44100) {
                pre_max_bit_pool = SBC_BITPOOL_HQ_JOINT_STEREO_44100;
            } else if (param->max_bit_pool >= SBC_BITPOOL_MQ_JOINT_STEREO_44100) {
                pre_max_bit_pool = SBC_BITPOOL_MQ_JOINT_STEREO_44100;
            } else if (param->max_bit_pool >= SBC_BITPOOL_LQ_JOINT_STEREO_44100) {
                pre_max_bit_pool = SBC_BITPOOL_LQ_JOINT_STEREO_44100;
            } else {
                SBC_ENC_LOG_E("[sbc enc] max bitpool is too small %d %d", 2,
                    param->max_bit_pool,
                    param->min_bit_pool
                    );
                pre_max_bit_pool = param->max_bit_pool;
                pre_min_bit_pool = param->min_bit_pool;
            }
            break;
        case SBC_ENCODER_SAMPLING_RATE_32000HZ:
        case SBC_ENCODER_SAMPLING_RATE_16000HZ:
            pre_max_bit_pool = param->max_bit_pool;
            pre_min_bit_pool = param->min_bit_pool;
            break;
        default:
            AUDIO_ASSERT(0 && "sbc enc sample rate error");
            break;
    }
    /* check the bit pool range */
    max_bit_pool = MIN(param->max_bit_pool, pre_max_bit_pool);
    min_bit_pool = MAX(param->min_bit_pool, pre_min_bit_pool);
    if (max_bit_pool < min_bit_pool) {
        if (param->max_bit_pool <= pre_min_bit_pool) { // lower than the lowest quality
            min_bit_pool = max_bit_pool;
        } else {
            max_bit_pool = min_bit_pool;
        }
    }
    sbc_enc_handle->bit_pool.bitpool_value = max_bit_pool;
    /* update bitpool for abr control */
    sbc_enc_handle->param.max_bit_pool = max_bit_pool;
    sbc_enc_handle->param.min_bit_pool = min_bit_pool;
    SBC_ENC_LOG_I("[sbc enc] bit pool init max %d min %d", 2,
        sbc_enc_handle->param.max_bit_pool,
        sbc_enc_handle->param.min_bit_pool
        );
}

#if AUDIO_A2DP_SBC_VBR_ENABLE
/* period: 5ms */
// down threhold 150ms: 20block
static int8_t speed = 0;
static uint32_t queue_len_pre = 0;
// static uint32_t queue_buffer[10] = {0};
void stream_codec_encoder_sbc_abr_control(uint32_t queue_length, uint32_t total_length)
{
    uint32_t threshold_level_1 = total_length / 5 * 1;
    uint32_t level = queue_length / threshold_level_1;
    uint8_t coeff = 0;
    if (!sbc_enc_handle) {
        return;
    }
    /* compute the speed */
    if (level > 0) {
        if (queue_length > queue_len_pre) {
            /* data is increasing */
            coeff = (1 << level) + 1;
        } else {
            coeff = 1;
        }
    }
    queue_len_pre = queue_length;
    if (coeff > 0) {
        speed += coeff;
    } else {
        speed --;
    }
    if (speed <= -50) {
        // increase bit rate
        if (sbc_enc_handle->bit_pool.bitpool_value + 5 <= sbc_enc_handle->param.max_bit_pool) {
            sbc_enc_handle->bit_pool.bitpool_value += 5;
        } else {
            sbc_enc_handle->bit_pool.bitpool_value = sbc_enc_handle->param.max_bit_pool;
        }
        speed = 0;
    }

    if (speed >= 20) {
        // decrease bit rate
        if (sbc_enc_handle->bit_pool.bitpool_value - 5 >= sbc_enc_handle->param.min_bit_pool) {
            sbc_enc_handle->bit_pool.bitpool_value -= 5;
        } else {
            sbc_enc_handle->bit_pool.bitpool_value = sbc_enc_handle->param.min_bit_pool;
        }
        speed = 0;
    }
    SBC_ENC_LOG_I("[sbc enc] abr change: len %d pre %d, coeff %d, speed %d bit pool %d", 5,
        queue_length,
        queue_len_pre,
        coeff,
        speed,
        sbc_enc_handle->bit_pool.bitpool_value
        );
}
#endif
/* ----------------------------------------------- SBC ABR Control -------------------------------------------------- */
/* |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| */