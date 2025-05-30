/* Copyright Statement:
 *
 * (C) 2005-2017  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */


#include <string.h>
#include "mp3_codec_internal.h"
#include "mp3_codec.h"
#if defined(MTK_AVM_DIRECT)
#include "timers.h"
#include "hal_audio_cm4_dsp_message.h"
#include "hal_audio_internal.h"
#include "hal_dvfs_internal.h"
#include "bt_sink_srv_ami.h"
#else
#include "ff.h"
#include "hal_audio_internal_service.h"
#include "hal_cm4_topsm.h"
#include "hal_audio_internal_pcm.h"
#ifdef HAL_AUDIO_LOW_POWER_ENABLED
#include "hal_audio_low_power.h"
#endif
#endif

#include "memory_attribute.h"

#include "task_def.h"
#include "audio_codec.h"

#if defined(MTK_AUDIO_MIXER_SUPPORT)
#include "prompt_control.h"
#endif

#if defined(HAL_DVFS_MODULE_ENABLED) && !defined(MTK_AVM_DIRECT)
#include "hal_dvfs.h"
#define MP3_CPU_FREQ_L_BOUND (104000)
#endif

#ifdef AIR_PROMPT_SOUND_ENABLE
#include "prompt_control.h"
#endif

#include "syslog.h"
#include "task.h"
#define UNUSED(x)  ((void)(x))

//======== Log related ========
log_create_module(MP3_CODEC, PRINT_LEVEL_INFO);
#define DEBUG_USE_MSGID_LOG
#ifdef DEBUG_USE_MSGID_LOG
#define MP3_LOG_D(fmt, arg...)          LOG_MSGID_D(MP3_CODEC, fmt, ##arg);
#define MP3_LOG_I(fmt, arg...)          LOG_MSGID_I(MP3_CODEC, fmt, ##arg);
#define MP3_LOG_W(fmt, arg...)          LOG_MSGID_W(MP3_CODEC, fmt, ##arg);
#define MP3_LOG_E(fmt, arg...)          LOG_MSGID_E(MP3_CODEC, fmt, ##arg);
#else
#define MP3_LOG_D(fmt, arg...)          LOG_E(MP3_CODEC, fmt, ##arg);
#define MP3_LOG_I(fmt, arg...)          LOG_I(MP3_CODEC, fmt, ##arg);
#define MP3_LOG_W(fmt, arg...)          LOG_W(MP3_CODEC, fmt, ##arg);
#define MP3_LOG_E(fmt, arg...)          LOG_E(MP3_CODEC, fmt, ##arg);
#endif

#define MINIMUM_MP3_FRAME_SIZE (24) // in bytes
#define MAXIMUM_MP3_FRAME_SIZE (1441) // in bytes
#define MSG_MCU2DSP_PROMPT_CLEAR_AUDIO_BUFFER  (0x55U)
#if defined(MTK_AUDIO_MIXER_SUPPORT)
#else
static mp3_codec_media_handle_t *global_mp3_handle = NULL; // backward compatibility for "mp3_codec_set_memory2(void)"
#endif

static volatile bool blIsRegister = false;
static volatile int32_t mp3_codec_open_counter = 0;

#if defined(MTK_AVM_DIRECT)
extern xSemaphoreHandle g_xSemaphore_Audio;
void mp3_audio_mutex_lock(xSemaphoreHandle handle)
{
    if (handle != NULL) {
        xSemaphoreTake(handle, portMAX_DELAY);
    }
}
void mp3_audio_mutex_unlock(xSemaphoreHandle handle)
{
    if (handle != NULL) {
        xSemaphoreGive(handle);
    }
}
#if defined(AIR_MP3_STEREO_SUPPORT_ENABLE)
static uint8_t mp3_codec_decode[27456];
#else
static uint8_t mp3_codec_decode[15908];
#endif
#ifdef AIR_MP3_TASK_DEDICATE_ENABLE
#define VP_MASK_VP_HAPPENING       0x00000004
#define VP_MASK_DL1_HAPPENING      0x00000008
extern volatile uint32_t g_vp_task_mask;
extern prompt_control_callback_t g_app_callback;
extern prompt_control_callback_t g_app_internal_callback;
#endif
#endif

void mp3_codec_enter_suspend(void *data);
void mp3_codec_enter_resume(void *data);

//pcm playback use
#ifdef HAL_AUDIO_LOW_POWER_ENABLED
#define AUDIO_INTERNAL_RING_BUFFER_SIZE (1024 * 12)
#else
#define AUDIO_INTERNAL_RING_BUFFER_SIZE (4096)
#endif
//mp3 decoder lib use

#ifdef HAL_AUDIO_LOW_POWER_ENABLED
#define MP3_DECODE_BUFFER_SIZE (41000 + 15000 + 4096)
#else
#define MP3_DECODE_BUFFER_SIZE (41000 + 1024) // since we add share_buffer_data_amount <= SHARE_BUFFER_TOO_LESS_AMOUNT feature so add 1024 to compensate it.
#endif

/* for calculate MCPS
volatile int *DWT_CONTROL = (int *)0xE0001000;
volatile int *DWT_CYCCNT = (int *)0xE0001004;
volatile int *DEMCR = (int *)0xE000EDFC;
volatile uint32_t count_test = 0;
volatile uint32_t offset = 0;
#define CPU_RESET_CYCLECOUNTER do { *DEMCR = *DEMCR | 0x01000000; \
*DWT_CYCCNT = 0; \
*DWT_CONTROL = *DWT_CONTROL | 1 ; } while(0)
*/

#define ID3V2_HEADER_LENGTH 10  // in bytes
#define MPEG_AUDIO_FRAME_HEADER_LENGTH 4 // in bytes
static int32_t MP3SilenceFrameSize = 0; // in bytes
static uint8_t MP3SilenceFrameHeader[MPEG_AUDIO_FRAME_HEADER_LENGTH];

static char mp3_silence_frame_pattern[] = "This is a silence frame."; // sizeof(mp3_silence_frame_pattern) = 25, sizeof(char) = 1
#define MP3_SILENCE_FRAME_PATTERN_LENGTH (25) // in bytes
extern volatile uint8_t g_audio_dl_suspend_by_user;

/*
#define IS_MP3_HEAD(head) (!( ((head & 0xfff80000) != 0xfff80000) || \
  ( ((head>>17)&3)!= 1) || \
  ( ((head>>12)&0xf) == 0xf) || ( ((head>>12)&0xf) == 0x0) || \
( ((head>>10)&0x3) == 0x3 )))
*/

#define IS_MP3_HEAD(head) (!( (((head & 0xfff00000) != 0xfff00000) && ((head & 0xfff80000) != 0xffe00000) ) || \
  ( ((head>>17)&3)== 3) || (((head>>17)&3)== 0) || \
  ( ((head>>12)&0xf) == 0xf) || ( ((head>>12)&0xf) == 0x0) || \
( ((head>>10)&0x3) == 0x3 )))


#define MP3_MPEG_AUDIO_FRAME_BIT_RATE_NA    0
#define MP3_MPEG_AUDIO_FRAME_BIT_RATE_32    1
#define MP3_MPEG_AUDIO_FRAME_BIT_RATE_40    2
#define MP3_MPEG_AUDIO_FRAME_BIT_RATE_48    3
#define MP3_MPEG_AUDIO_FRAME_BIT_RATE_56    4
#define MP3_MPEG_AUDIO_FRAME_BIT_RATE_64    5
#define MP3_MPEG_AUDIO_FRAME_BIT_RATE_80    6
#define MP3_MPEG_AUDIO_FRAME_BIT_RATE_96    7
#define MP3_MPEG_AUDIO_FRAME_BIT_RATE_112    8
#define MP3_MPEG_AUDIO_FRAME_BIT_RATE_128    9
#define MP3_MPEG_AUDIO_FRAME_BIT_RATE_160    0xa
#define MP3_MPEG_AUDIO_FRAME_BIT_RATE_192    0xb
#define MP3_MPEG_AUDIO_FRAME_BIT_RATE_224    0xc
#define MP3_MPEG_AUDIO_FRAME_BIT_RATE_256    0xd
#define MP3_MPEG_AUDIO_FRAME_BIT_RATE_320    0xe
#define MP3_MPEG_AUDIO_FRAME_BIT_RATE_NA2    0xf


#define SHARE_BUFFER_TOO_LESS_AMOUNT 1000        // maybe can be more precisely
#define SHARE_BUFFER_TOO_LESS_SLEEP_TIME_IN_MS 30
#define SHARE_BUFFER_TOO_LESS_SLEEP_LOOP_TIMES 10

#ifdef AIR_MP3_TASK_DEDICATE_ENABLE
volatile mp3_codec_media_handle_t   *g_mp3_codec_task_handle = NULL;
#endif

#ifdef  HAL_DVFS_MODULE_ENABLED
#ifdef HAL_DVFS_416M_SOURCE
#ifdef AIR_BT_ULTRA_LOW_LATENCY_ENABLE
#define MP3_CODEC_DVFS_DEFAULT_SPEED  HAL_DVFS_HIGH_SPEED_208M
#else
#define MP3_CODEC_DVFS_DEFAULT_SPEED  HAL_DVFS_HIGH_SPEED_208M
#endif
#elif defined(HAL_DVFS_312M_SOURCE)
#define MP3_CODEC_DVFS_DEFAULT_SPEED  DVFS_78M_SPEED
#endif
#endif
IPCOMMON_PLUS

void mp3_codec_event_send_from_isr(mp3_codec_queue_event_id_t id, void *parameter);

#if defined(HAL_DVFS_MODULE_ENABLED) && !defined(MTK_AVM_DIRECT)
static bool mp3_dvfs_valid(uint32_t voltage, uint32_t frequency)
{
    if (frequency < MP3_CPU_FREQ_L_BOUND) {
        return false;
    } else {
        return true;
    }
}

static dvfs_notification_t mp3_dvfs_desc = {
    .domain = "VCORE",
    .module = "CM_CK0",
    .addressee = "mp3_dvfs",
    .ops = {
        .valid = mp3_dvfs_valid,
    }
};

static void mp3_codec_register_mp3_dvfs(bool flag)
{
    if (flag) {
        dvfs_register_notification(&mp3_dvfs_desc);
        dvfs_target_cpu_frequency(MP3_CPU_FREQ_L_BOUND, HAL_DVFS_FREQ_RELATION_L);
    } else {
        dvfs_deregister_notification(&mp3_dvfs_desc);
    }

}
#endif /*HAL_DVFS_MODULE_ENABLED*/

#if defined(MTK_AUDIO_MIXER_SUPPORT)
void mp3_codec_set_track_role(mp3_codec_media_handle_t *handle, audio_mixer_role_t role)
{
    handle->mixer_track_role = role;
    return;
}
#endif

// ======== Private lock functions ========
static void mp3_codec_mutex_lock(mp3_codec_media_handle_t *handle)
{
    mp3_codec_internal_handle_t *internal_handle = mp3_handle_ptr_to_internal_ptr(handle);

    if (internal_handle->semaphore_handle != NULL) {
#if defined(MTK_AUDIO_MP3_DEBUG)
        MP3_LOG_I("[MP3 Codec] mp3_codec_mutex_lock() +\r\n", 0);
#endif
        xSemaphoreTake(internal_handle->semaphore_handle, portMAX_DELAY);
#if defined(MTK_AUDIO_MP3_DEBUG)
        MP3_LOG_I("[MP3 Codec] mp3_codec_mutex_lock() -\r\n", 0);
#endif
    }
}

static void mp3_codec_mutex_unlock(mp3_codec_media_handle_t *handle)
{
    mp3_codec_internal_handle_t *internal_handle = mp3_handle_ptr_to_internal_ptr(handle);

    if (internal_handle->semaphore_handle != NULL) {
#if defined(MTK_AUDIO_MP3_DEBUG)
        MP3_LOG_I("[MP3 Codec] mp3_codec_mutex_unlock()\r\n", 0);
#endif
        xSemaphoreGive(internal_handle->semaphore_handle);
    }
}

/*  share buffer operation function */
static mp3_codec_function_return_state_t mp3_codec_set_bitstream_buffer(mp3_codec_media_handle_t *handle, uint8_t *buffer, uint32_t length)
{
    length = (length + 3) & ~0x3;
    handle->share_buff.buffer_base = buffer;
    handle->share_buff.buffer_size = length;
    handle->share_buff.write = length;
    handle->share_buff.read = 0;
    handle->waiting = false;
    handle->underflow = false;
    handle->linear_buff = true;

    return MP3_CODEC_RETURN_OK;
}

static void mp3_codec_set_share_buffer(mp3_codec_media_handle_t *handle, uint8_t *buffer, uint32_t length)
{
    handle->share_buff.buffer_base = buffer;
    //length &= ~0x1; // make buffer size even
    handle->share_buff.buffer_size = length;
    handle->share_buff.write = 0;
    handle->share_buff.read = 0;
    handle->waiting = false;
    handle->underflow = false;
    handle->linear_buff = false;
}

static void mp3_codec_get_share_buffer_write_information(mp3_codec_media_handle_t *handle, uint8_t **buffer, uint32_t *length)
{
    int32_t count = 0;

    if (handle->share_buff.read > handle->share_buff.write) {
        count = handle->share_buff.read - handle->share_buff.write - 1;
    } else if (handle->share_buff.read == 0) {
        count = handle->share_buff.buffer_size - handle->share_buff.write - 1;
    } else {
        count = handle->share_buff.buffer_size - handle->share_buff.write;
    }
    *buffer = handle->share_buff.buffer_base + handle->share_buff.write;
    *length = count;
}


static void mp3_codec_get_share_buffer_read_information(mp3_codec_media_handle_t *handle, uint8_t **buffer, uint32_t *length)
{
    int32_t count = 0;

    if (handle->share_buff.write >= handle->share_buff.read) {
        count = handle->share_buff.write - handle->share_buff.read;
    } else {
        count = handle->share_buff.buffer_size - handle->share_buff.read;
    }
    *buffer = handle->share_buff.buffer_base + handle->share_buff.read;
    *length = count;
}


static void mp3_codec_share_buffer_write_data_done(mp3_codec_media_handle_t *handle, uint32_t length)
{
    handle->share_buff.write += length;
    if (handle->share_buff.write == handle->share_buff.buffer_size) {
        handle->share_buff.write = 0;
    }
}

static int32_t mp3_codec_get_share_buffer_data_count(mp3_codec_media_handle_t *);
static void mp3_codec_finish_write_data(mp3_codec_media_handle_t *handle)
{
    handle->waiting = false;
    handle->underflow = false;

    if (MP3_CODEC_STATE_PLAY == handle->state) {
        int32_t share_buffer_data_amount = mp3_codec_get_share_buffer_data_count(handle);
        if (mp3_handle_ptr_to_previous_mp3_frame_size(handle) > 0) {
            if (share_buffer_data_amount > mp3_handle_ptr_to_previous_mp3_frame_size(handle)) {
                mp3_codec_event_send_from_isr(MP3_CODEC_QUEUE_EVENT_DECODE, handle);
            }
        } else if (share_buffer_data_amount > SHARE_BUFFER_TOO_LESS_AMOUNT) {
            mp3_codec_event_send_from_isr(MP3_CODEC_QUEUE_EVENT_DECODE, handle);
        }
    }

    uint32_t current_cnt = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_cnt);
#if defined(MTK_AUDIO_MIXER_SUPPORT)
    MP3_LOG_I("[MP3 Codec] finish_write_data role%d: %d ms\n", 2, handle->mixer_track_role, (current_cnt - mp3_handle_ptr_to_share_buffer_waiting_cnt(handle)) / 1000);
#else
    MP3_LOG_I("[MP3 Codec] finish_write_data: %d ms\n", 1, (current_cnt - mp3_handle_ptr_to_share_buffer_waiting_cnt(handle)) / 1000);
#endif

}

static void mp3_codec_flush(mp3_codec_media_handle_t *handle, int32_t flush_data_flag)
{
    handle->flush_data_flag = flush_data_flag;
    MP3_LOG_I("[MP3 Codec] flush_data_flag = %d\n", 1, handle->flush_data_flag);
}

static void mp3_codec_get_data_status(mp3_codec_media_handle_t *handle, mp3_codec_data_type_t data_type, int32_t *data_status)
{
    *data_status = 0;

    switch (data_type) {
        case MP3_CODEC_DATA_TYPE_AUDIO:
            if (handle->state == MP3_CODEC_STATE_PLAY) {
                mp3_codec_internal_handle_t *internal_handle = mp3_handle_ptr_to_internal_ptr(handle);
                uint8_t *out_buf_ptr    = NULL;
                uint32_t stream_out_pcm_buffer_data_count = 0;

                ring_buffer_get_read_information(&internal_handle->stream_out_pcm_buff, &out_buf_ptr, &stream_out_pcm_buffer_data_count);
                if (stream_out_pcm_buffer_data_count == 0) {
                    uint32_t sample_count = 0;

#if defined(MTK_AVM_DIRECT)
                    {
                        n9_dsp_share_info_t *p_info;

                        p_info = (n9_dsp_share_info_t *)hal_audio_query_share_info(internal_handle->message_type);

                        sample_count = hal_audio_buf_mgm_get_free_byte_count(p_info);
                    }
#elif defined(MTK_AUDIO_MIXER_SUPPORT)
                    audio_mixer_query_free_count(handle->mixer_track_id, &sample_count);
#else
                    hal_audio_get_stream_out_sample_count(&sample_count);
#endif

#if defined(MTK_AUDIO_MIXER_SUPPORT)
                    *data_status = 1;
#else
                    if (mp3_handle_ptr_to_total_sample_count(handle) <= sample_count) {
                        *data_status = 1;
                    }
#endif
                    UNUSED(sample_count);
                }
            }
            break;
        case MP3_CODEC_DATA_TYPE_SAMPLES_PER_CHANNEL:
            if (handle->state == MP3_CODEC_STATE_PLAY) {
                mp3_codec_internal_handle_t *internal_handle = mp3_handle_ptr_to_internal_ptr(handle);
                *data_status = internal_handle->mp3_handle->PCMSamplesPerCH;
            }
            break;
    }
}

static void mp3_codec_reset_share_buffer(mp3_codec_media_handle_t *handle)
{
    /*
      If share_buff is provided by users, it is suggested to clear content by users.
      Otherwise, it could have risk to erase mp3 file table.
     */
    if (handle->linear_buff == false) {
        memset(handle->share_buff.buffer_base, 0, handle->share_buff.buffer_size);  // do this or it will have previous data. we met that when change to bad mp3 file, but at this time app still use write_data_done, it will have previous header
    }
    handle->share_buff.write = 0;
    handle->share_buff.read = 0;
    handle->waiting = false;
    handle->underflow = false;
}


static void mp3_codec_share_buffer_read_data_done(mp3_codec_media_handle_t *handle, uint32_t length)
{
    handle->share_buff.read += length;
    if (handle->share_buff.read == handle->share_buff.buffer_size) {
        handle->share_buff.read = 0;
    }
}


static int32_t mp3_codec_get_share_buffer_free_space(mp3_codec_media_handle_t *handle)
{
    int32_t count = 0;

    count = handle->share_buff.read - handle->share_buff.write - 2;
    if (count < 0) {
        count += handle->share_buff.buffer_size;
    }
    return count;
}

static int32_t mp3_codec_get_share_buffer_data_count(mp3_codec_media_handle_t *handle)
{
    int32_t count = 0;

    count = handle->share_buff.write - handle->share_buff.read;
    if (count < 0) {
        count += handle->share_buff.buffer_size;
    }
    return count;
}

static void mp3_codec_reset_stream_out_pcm_buffer(mp3_codec_media_handle_t *handle)
{
    mp3_codec_internal_handle_t *internal_handle = mp3_handle_ptr_to_internal_ptr(handle);
    internal_handle->stream_out_pcm_buff.read_pointer = 0;
    internal_handle->stream_out_pcm_buff.write_pointer = 0;
}

static int32_t mp3_codec_set_silence_frame_information(mp3_codec_media_handle_t *handle, silence_frame_information_t *frm_info)
{
    if (frm_info->frame_size > 0) {
        MP3SilenceFrameSize = frm_info->frame_size;
        MP3SilenceFrameHeader[0] = frm_info->frame[0];
        MP3SilenceFrameHeader[1] = frm_info->frame[1];
        MP3SilenceFrameHeader[2] = frm_info->frame[2];
        MP3SilenceFrameHeader[3] = frm_info->frame[3];

        MP3_LOG_I("MP3SilenceFrameSize = %d, MP3SilenceFrameHeader = 0x%X %X %X %X \n", 5, MP3SilenceFrameSize,
                  MP3SilenceFrameHeader[0], MP3SilenceFrameHeader[1], MP3SilenceFrameHeader[2], MP3SilenceFrameHeader[3]);

        return 0;
    }

    return -1;
}

static int32_t mp3_codec_get_silence_frame_information(mp3_codec_media_handle_t *handle, int32_t *byte_count)
{
    if (MP3SilenceFrameSize > 0) {
        *byte_count = MP3SilenceFrameSize;
    }

    return 0;
}

static int32_t mp3_codec_fill_silence_frame(mp3_codec_media_handle_t *handle, uint8_t *buffer)
{
    if (MP3SilenceFrameSize > 0) {
        memset(buffer, 0, MP3SilenceFrameSize);
        buffer[0] = MP3SilenceFrameHeader[0];
        buffer[1] = MP3SilenceFrameHeader[1];
        buffer[2] = MP3SilenceFrameHeader[2];
        buffer[3] = MP3SilenceFrameHeader[3];
        strncpy((char *)(buffer + MPEG_AUDIO_FRAME_HEADER_LENGTH), mp3_silence_frame_pattern, sizeof(mp3_silence_frame_pattern));
    }

    return 0;
}

#if defined(MTK_BT_AWS_ENABLE) && !defined(MTK_AVM_DIRECT)
void mp3_codec_aws_callback(aws_event_t event, void *user_data)
{
    mp3_codec_media_handle_t *handle = (mp3_codec_media_handle_t *)user_data;

    switch (event) {
        case CODEC_AWS_CHECK_CLOCK_SKEW:
            handle->handler(handle, MP3_CODEC_AWS_CHECK_CLOCK_SKEW);
            break;
        case CODEC_AWS_CHECK_UNDERFLOW:
            handle->handler(handle, MP3_CODEC_AWS_CHECK_UNDERFLOW);
            break;
        default:
            break;
    }
    return;
}
#endif

#if defined(MTK_BT_AWS_ENABLE) && !defined(MTK_AVM_DIRECT)
mp3_codec_function_return_state_t mp3_codec_aws_init(mp3_codec_media_handle_t *handle)
{
    int32_t aws_buf_size = 0;
    uint8_t *p_aws_buf;

    aws_buf_size = audio_service_aws_get_buffer_size(AWS_CODEC_TYPE_PCM_FORMAT);
    if (aws_buf_size > 0) {
        p_aws_buf = (uint8_t *)pvPortMalloc(aws_buf_size);
    } else {
        MP3_LOG_E("mp3_codec_aws_init failed!!! audio_service_aws_get_buffer_size = %d < 0\n", 1, aws_buf_size);
        return MP3_CODEC_RETURN_ERROR;
    }
    mp3_handle_ptr_to_p_aws_buf(handle) = p_aws_buf;
    MP3_LOG_I("mp3_codec_aws_init: p_aws_buf = 0x%X, aws_buf_size = %d\n", 2, p_aws_buf, aws_buf_size);
    if (p_aws_buf) {
        aws_buf_size = audio_service_aws_init(p_aws_buf, AWS_CODEC_TYPE_PCM_FORMAT, mp3_codec_aws_callback, (void *)handle);
        MP3_LOG_I("audio_service_aws_init mp3_codec_aws_callback(0x%X)\n", 1, mp3_codec_aws_callback);
        MP3_LOG_I("audio_service_aws_init return %d\n", 1, aws_buf_size);
    } else {
        return MP3_CODEC_RETURN_ERROR;
    }

    return MP3_CODEC_RETURN_OK;
}
#endif

#ifdef MTK_BT_AWS_ENABLE
mp3_codec_function_return_state_t mp3_codec_aws_deinit(mp3_codec_media_handle_t *handle)
{
    MP3_LOG_I("mp3_codec_aws_deinit: Free p_aws_buf\r\n", 0);
    vPortFree(mp3_handle_ptr_to_p_aws_buf(handle));
    mp3_handle_ptr_to_p_aws_buf(handle) = NULL;

    audio_service_aws_deinit();
    return MP3_CODEC_RETURN_OK;
}
#endif

#if defined(MTK_BT_AWS_ENABLE) && !defined(MTK_AVM_DIRECT)
mp3_codec_function_return_state_t mp3_codec_aws_set_clock_skew_compensation_value(mp3_codec_media_handle_t *handle, int32_t sample_count)
{
    audio_service_aws_set_clock_skew_compensation_value(sample_count);

    return MP3_CODEC_RETURN_OK;
}

mp3_codec_function_return_state_t mp3_codec_aws_get_clock_skew_status(mp3_codec_media_handle_t *handle, int32_t *aws_clock_skew_status)
{
    *aws_clock_skew_status = audio_service_aws_get_clock_skew_status();

    return MP3_CODEC_RETURN_OK;
}

mp3_codec_function_return_state_t mp3_codec_aws_set_clock_skew(mp3_codec_media_handle_t *handle, bool flag)
{
    if (flag) {
        audio_service_aws_set_clock_skew(true);
    } else {
        audio_service_aws_set_clock_skew(false);
    }
    return MP3_CODEC_RETURN_OK;
}

#endif

static void mp3_codec_buffer_function_init(mp3_codec_media_handle_t *handle)
{
    handle->set_share_buffer   = mp3_codec_set_share_buffer;
    handle->get_write_buffer   = mp3_codec_get_share_buffer_write_information;
    handle->get_read_buffer    = mp3_codec_get_share_buffer_read_information;
    handle->write_data_done    = mp3_codec_share_buffer_write_data_done;
    handle->finish_write_data  = mp3_codec_finish_write_data;
    handle->reset_share_buffer = mp3_codec_reset_share_buffer;
    handle->read_data_done     = mp3_codec_share_buffer_read_data_done;
    handle->get_free_space     = mp3_codec_get_share_buffer_free_space;
    handle->get_data_count     = mp3_codec_get_share_buffer_data_count;
    handle->flush              = mp3_codec_flush;
    handle->get_data_status    = mp3_codec_get_data_status;
    handle->set_bitstream_buffer = mp3_codec_set_bitstream_buffer;
    handle->set_silence_frame_information = mp3_codec_set_silence_frame_information;
    handle->get_silence_frame_information = mp3_codec_get_silence_frame_information;
    handle->fill_silence_frame = mp3_codec_fill_silence_frame;
#if defined(MTK_BT_AWS_ENABLE) && !defined(MTK_AVM_DIRECT)
    handle->aws_init            = mp3_codec_aws_init;
    handle->aws_deinit          = mp3_codec_aws_deinit;
    handle->aws_set_clock_skew_compensation_value = mp3_codec_aws_set_clock_skew_compensation_value;
    handle->aws_get_clock_skew_status = mp3_codec_aws_get_clock_skew_status;
    handle->aws_set_clock_skew = mp3_codec_aws_set_clock_skew;
#endif
#if defined(MTK_AUDIO_MIXER_SUPPORT)
    handle->set_track_role     = mp3_codec_set_track_role;
#endif
}


static uint32_t mp3_codec_get_bytes_from_share_buffer(mp3_codec_media_handle_t *handle, uint8_t *destination_buffer, uint32_t get_bytes_amount, bool want_move_read_ptr)
{
    uint8_t *share_buffer_read_address;
    uint32_t share_buffer_data_length;
    uint32_t share_buffer_read_index_original = 0;
    uint32_t bytes_amount_temp = get_bytes_amount;
    uint32_t got_bytes_amount = 0;  // real got bytes amount from share buffer
    uint32_t get_bytes_amount_mini;

    share_buffer_read_index_original = handle->share_buff.read; // store original share_buffer read pointer


    uint16_t loop_idx = 0;
    for (loop_idx = 0; loop_idx < 2; loop_idx++) {
        mp3_codec_get_share_buffer_read_information(handle, &share_buffer_read_address, &share_buffer_data_length);
        if (share_buffer_data_length > 0) {
            get_bytes_amount_mini = MINIMUM(bytes_amount_temp, share_buffer_data_length);
            memcpy(destination_buffer, share_buffer_read_address, get_bytes_amount_mini);
            bytes_amount_temp -= get_bytes_amount_mini;
            destination_buffer += get_bytes_amount_mini;
            mp3_codec_share_buffer_read_data_done(handle, get_bytes_amount_mini);

            if (bytes_amount_temp == 0) {
                break;
            }
        } else {
            // share buffer empty
            break;
        }
    }


    got_bytes_amount = get_bytes_amount - bytes_amount_temp;  // real read amount

    if (got_bytes_amount != get_bytes_amount) {
        MP3_LOG_I("[MP3 Codec]mp3_codec_get_bytes_from_share_buffer: got_bytes_amount(%ld) != get_bytes_amount(%ld)\n", 2, got_bytes_amount, get_bytes_amount);
    }


    if (want_move_read_ptr == false) {
        handle->share_buff.read = share_buffer_read_index_original;
    }

    return got_bytes_amount;
}



static uint32_t mp3_codec_discard_bytes_of_share_buffer(mp3_codec_media_handle_t *handle, uint32_t discard_bytes_amount)
{
    uint8_t *share_buffer_read_address;
    uint32_t share_buffer_data_length;
    uint32_t bytes_amount_temp = discard_bytes_amount;
    uint32_t discarded_bytes_amount = 0;


    uint16_t loop_idx = 0;
    for (loop_idx = 0; loop_idx < 2; loop_idx++) {
        mp3_codec_get_share_buffer_read_information(handle, &share_buffer_read_address, &share_buffer_data_length);
        if (share_buffer_data_length > 0) {
            uint32_t get_bytes_amount = MINIMUM(bytes_amount_temp, share_buffer_data_length);
            bytes_amount_temp -= get_bytes_amount;
            mp3_codec_share_buffer_read_data_done(handle, get_bytes_amount);

            if (bytes_amount_temp == 0) {
                break;
            }
        } else {
            // share buffer empty
            break;
        }
    }

    discarded_bytes_amount = discard_bytes_amount - bytes_amount_temp;  // real read amount

    if (discarded_bytes_amount != discard_bytes_amount) {
        MP3_LOG_I("[MP3 Codec]mp3_codec_discard_bytes_of_share_buffer : discarded_bytes_amount(%ld) != discard_bytes_amount(%ld)\n", 2, discarded_bytes_amount, discard_bytes_amount);
    }


    return discarded_bytes_amount;
}

static mp3_codec_function_return_state_t mp3_codec_request_data_to_share_buffer(mp3_codec_media_handle_t *handle)
{
    // return MP3_CODEC_RETURN_OK:          request success
    // return  MP3_CODEC_RETURN_ERROR:    already request and waiting feed back

    mp3_codec_internal_handle_t *internal_handle = mp3_handle_ptr_to_internal_ptr(handle);

    if (handle->linear_buff == 1) {
        UNUSED(internal_handle);
        return MP3_CODEC_RETURN_OK; // Just return in Linear Buffer
    }

    if (!handle->waiting) {
#if defined(MTK_AUDIO_MIXER_SUPPORT)
        MP3_LOG_I("[MP3 Codec] role%d request data: share = %d, pcm = %d\r\n", 3, handle->mixer_track_role, mp3_codec_get_share_buffer_data_count(handle), ring_buffer_get_space_byte_count(&internal_handle->stream_out_pcm_buff));
#else
        MP3_LOG_I("[MP3 Codec] request data: share = %d, pcm = %d\r\n", 2, mp3_codec_get_share_buffer_data_count(handle), ring_buffer_get_space_byte_count(&internal_handle->stream_out_pcm_buff));
#endif
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &(mp3_handle_ptr_to_share_buffer_waiting_cnt(handle)));
        handle->waiting = true;
        handle->handler(handle, MP3_CODEC_MEDIA_REQUEST);
        UNUSED(internal_handle);
        return MP3_CODEC_RETURN_OK;
    } else {
#if defined(MTK_AUDIO_MIXER_SUPPORT)
        MP3_LOG_I("[MP3 Codec] role%d request data again: share = %d, pcm = %d\r\n", 3, handle->mixer_track_role, mp3_codec_get_share_buffer_data_count(handle), ring_buffer_get_space_byte_count(&internal_handle->stream_out_pcm_buff));
#else
        MP3_LOG_I("[MP3 Codec] request data again: share = %d, pcm = %d\r\n", 2, mp3_codec_get_share_buffer_data_count(handle), ring_buffer_get_space_byte_count(&internal_handle->stream_out_pcm_buff));
#endif
        UNUSED(internal_handle);
        return MP3_CODEC_RETURN_ERROR;
    }
}

static uint32_t mp3_codec_combine_four_bytes_buffer_to_uint32_value(uint8_t *buffer)
{
    uint32_t uint32_value = 0;

    uint32_value = *buffer;
    uint32_value = uint32_value << 8 | *(buffer + 1);
    uint32_value = uint32_value << 8 | *(buffer + 2);
    uint32_value = uint32_value << 8 | *(buffer + 3);

    return uint32_value;
}

static mp3_codec_function_return_state_t mp3_codec_reach_next_frame_and_get_audio_frame_header(mp3_codec_media_handle_t *handle, uint32_t *audio_frame_header, uint32_t maximum_check_bytes, uint32_t want_skip_frame_after_got_header)
{
    uint8_t check_mpeg_header_buffer[MPEG_AUDIO_FRAME_HEADER_LENGTH] = {0};
    uint32_t temp_mpeg_header = 0;
    uint32_t discard_bytes_amount = 0;
    uint32_t temp_uint32_t = 0;
    uint32_t temp_maximum_check_bytes = maximum_check_bytes;
    uint32_t maximum_request_data_time = maximum_check_bytes / handle->share_buff.buffer_size + 2;  // 2: arbitrarily selected


    do {
        if (mp3_codec_get_share_buffer_data_count(handle) < MPEG_AUDIO_FRAME_HEADER_LENGTH) {

            if (mp3_codec_request_data_to_share_buffer(handle) == MP3_CODEC_RETURN_OK) {
                maximum_request_data_time--;
            }

            if (mp3_codec_get_share_buffer_data_count(handle) < MPEG_AUDIO_FRAME_HEADER_LENGTH) {
                return MP3_CODEC_RETURN_ERROR;
            }
        }

        mp3_codec_get_bytes_from_share_buffer(handle, check_mpeg_header_buffer, MPEG_AUDIO_FRAME_HEADER_LENGTH, 0);
        temp_mpeg_header = mp3_codec_combine_four_bytes_buffer_to_uint32_value(check_mpeg_header_buffer);

        if (IS_MP3_HEAD(temp_mpeg_header)) {
            // find MP3 HEAD
            *audio_frame_header = temp_mpeg_header;

            if (want_skip_frame_after_got_header) {
                discard_bytes_amount = 4;
                temp_uint32_t = mp3_codec_discard_bytes_of_share_buffer(handle, discard_bytes_amount);
                if (temp_uint32_t < discard_bytes_amount) {  // share buffer didn't have enoungh data to discared
                    return MP3_CODEC_RETURN_ERROR;
                }
            }
            MP3_LOG_I("[MP3 Codec]mp3_codec_reach_next_frame: find mp3 header=%x\n", 1, *audio_frame_header);
            return MP3_CODEC_RETURN_OK;
        }

        discard_bytes_amount = 1;
        temp_uint32_t = MINIMUM(discard_bytes_amount, temp_maximum_check_bytes);
        temp_uint32_t = mp3_codec_discard_bytes_of_share_buffer(handle, temp_uint32_t);
        if (temp_uint32_t < discard_bytes_amount) {  // share buffer didn't have enoungh data to discared
            return MP3_CODEC_RETURN_ERROR;
        }

        temp_maximum_check_bytes -= temp_uint32_t;

    } while (temp_maximum_check_bytes != 0 && maximum_request_data_time != 0);


    MP3_LOG_I("[MP3 Codec]mp3_codec_reach_next_frame: not find mp3 header\n", 0);
    *audio_frame_header = 0;

    return MP3_CODEC_RETURN_ERROR;
}


static void mp3_codec_get_id3v2_info_and_skip(mp3_codec_media_handle_t *handle, uint32_t file_size)
{
    uint32_t want_get_bytes = 0;
    uint8_t temp_buffer[10] = {0};
    uint32_t id3v2_remain_tagesize = 0; // not include ID3v2 header size, refert to ID3v2 spec
    uint32_t id3v2_tage_size = 0;   // total ID3v2 tage size which include ID3v2 header
    uint32_t remain_file_data_size = file_size; // in bytes

    handle->jump_file_to_specified_position = 0;    // asume from file begin
    handle->id3v2_information.has_id3v2 = false;
    handle->id3v2_information.id3v2_tage_length = 0;

    while (1) {
        want_get_bytes = ID3V2_HEADER_LENGTH;
        if (mp3_codec_get_bytes_from_share_buffer(handle, temp_buffer, want_get_bytes, 0) != want_get_bytes) {
            MP3_LOG_I("[MP3 Codec]mp3_codec_get_id3v2_info_and_skip: share buffer data amount less than ID3v2 header length\n", 0);
            return;    // just return
        }

        if (strncmp((const char *)temp_buffer, "ID3", 3) == 0) {
            id3v2_remain_tagesize = ((temp_buffer[6] & 0x7f) << 21) | ((temp_buffer[7] & 0x7f) << 14) | ((temp_buffer[8] & 0x7f) <<  7) | ((temp_buffer[9] & 0x7f) <<  0);
            id3v2_tage_size = id3v2_remain_tagesize + ID3V2_HEADER_LENGTH;
            MP3_LOG_I("[MP3 Codec]find id3v2: id3v2_tagesize=%ld, id3v2_remain_tagesize =%ld\n", 2, id3v2_tage_size, id3v2_remain_tagesize);


            if (remain_file_data_size < id3v2_tage_size) {
                // the tag size calculate form ID3v2 may wrong
                return;
            }


            handle->id3v2_information.has_id3v2 = true;
            handle->id3v2_information.id3v2_tage_length += id3v2_tage_size;


            // Although the remaing data in share buffer can be used,
            // but the fast and clear way to skip ID3v2 is just ask user to jump file to specific position and refill the share buffer
            mp3_codec_reset_share_buffer(handle);   // since we want to ask user to jump file to specific position and get data, thus remaining data is no use.
            handle->jump_file_to_specified_position += id3v2_tage_size;
            handle->handler(handle, MP3_CODEC_MEDIA_JUMP_FILE_TO);

            mp3_codec_request_data_to_share_buffer(handle);


            remain_file_data_size -= id3v2_tage_size;


        } else {
            MP3_LOG_I("[MP3 Codec]done skip ID3v2, has_id3v2=%d, id3v2_tage_length=%d\n", 2, (uint32_t)handle->id3v2_information.has_id3v2, handle->id3v2_information.id3v2_tage_length);
            return;
        }

    }
}


#if 0

mp3_codec_function_return_state_t  mp3_codec_set_memory(uint8_t *memory)
{

    if (mp3_codec_internal_handle == NULL) {
        MP3_LOG_E("[MP3 Codec]mp3_codec_internal_handle = NULL\n", 0);
        return MP3_CODEC_RETURN_ERROR;
    }
    mp3_codec_internal_handle_t *internal_handle = mp3_codec_internal_handle;
    mp3_codec_media_handle_t *handle = &internal_handle->handle;

    if (handle->state != MP3_CODEC_STATE_READY) {
        return MP3_CODEC_RETURN_ERROR;
    }
    uint8_t *memory_base = NULL;
    uint32_t totl_memory_size = 0;
    internal_handle->memory_pool = memory_base = memory;

    totl_memory_size = internal_handle->share_buff_size
                       + internal_handle->decode_pcm_buffer_size
                       + internal_handle->working_buff1_size
                       + internal_handle->working_buff2_size
                       + internal_handle->stream_out_pcm_buff_size;

    //set share buffer
    mp3_codec_set_share_buffer(handle, memory_base, internal_handle->share_buff_size);
    memory_base += internal_handle->share_buff_size;

    // set PCM buffer
    internal_handle->decode_pcm_buff.buffer_base_pointer = memory_base;
    internal_handle->decode_pcm_buff.buffer_byte_count = internal_handle->decode_pcm_buffer_size;
    internal_handle->decode_pcm_buff.read_pointer = 0;
    internal_handle->decode_pcm_buff.write_pointer = 0;
    memory_base += internal_handle->decode_pcm_buffer_size;

    //set working buffer
    internal_handle->working_buff1 = memory_base;
    memory_base += internal_handle->working_buff1_size;
    internal_handle->working_buff2 = memory_base;
    memory_base += internal_handle->working_buff2_size;


    // set PCM buffer   // piter add
    internal_handle->stream_out_pcm_buff.buffer_base_pointer = memory_base;
    internal_handle->stream_out_pcm_buff.buffer_byte_count = internal_handle->stream_out_pcm_buff_size;
    internal_handle->stream_out_pcm_buff.read_pointer = 0;
    internal_handle->stream_out_pcm_buff.write_pointer = 0;


    // STEP 2 : Get MP3 Handler
    internal_handle->mp3_handle = MP3Dec_Init(internal_handle->working_buff1, internal_handle->working_buff2);
    if (internal_handle->mp3_handle == NULL) {
        MP3_LOG_E("[MP3 Codec]MP3Dec_Init fail", 0);
    }

#if 1
    MP3_LOG_I("[MP3 Codec]set_memory memory range : start=%08x end=%08x", 2,
              memory, (memory + totl_memory_size));
    MP3_LOG_I("[MP3 Codec]set_memory share buffer : base=%08x size=%u", 2,
              handle->share_buff.buffer_base, (unsigned int)handle->share_buff.buffer_size);
    MP3_LOG_I("[MP3 Codec]set_memory decode_pcm_buff.buffer : base=%08x size=%u", 2,
              internal_handle->decode_pcm_buff.buffer_base_pointer, (unsigned int)internal_handle->decode_pcm_buff.buffer_byte_count);
    MP3_LOG_I("[MP3 Codec]set_memory work1 buffer : base=%08x size=%u", 2,
              internal_handle->working_buff1, (unsigned int)internal_handle->working_buff1_size);
    MP3_LOG_I("[MP3 Codec]set_memory work2 buffer : base=%08x size=%u", 2,
              internal_handle->working_buff2, (unsigned int)internal_handle->working_buff2_size);
    MP3_LOG_I("[MP3 Codec]set_memory stream_out_pcm_buff : base=%08x size=%u", 2,
              internal_handle->stream_out_pcm_buff.buffer_base_pointer, (unsigned int)internal_handle->stream_out_pcm_buff.buffer_byte_count);
#endif



    return MP3_CODEC_RETURN_OK;

}

mp3_codec_function_return_state_t mp3_codec_get_memory_size(uint32_t *memory_size)
{

    if (mp3_codec_internal_handle == NULL) {
        MP3_LOG_E("[MP3 Codec]mp3_codec_internal_handle = NULL\n", 0);
        return MP3_CODEC_RETURN_ERROR;
    }
    mp3_codec_internal_handle_t *internal_handle = mp3_codec_internal_handle;
    mp3_codec_media_handle_t *handle = &internal_handle->handle;

    if (handle->state != MP3_CODEC_STATE_READY) {
        return MP3_CODEC_RETURN_ERROR;
    }

    int working_buff1_size,         /* the required Working buffer1 size in byte    */
        working_buff2_size,         /* the required Working buffer2 size in byte    */
        decode_pcm_buffer_size,              /* the required pcm buffer size in byte          */
        share_buff_size;            /* the share buffer size      */

    *memory_size = 0;

    /*STEP 1 : Allocate data memory for MP3 decoder*/
    MP3Dec_GetMemSize(&share_buff_size, &decode_pcm_buffer_size, &working_buff1_size, &working_buff2_size);

    //share_buff_size += 1024;  // piter delete

    //4bytes aligned
    share_buff_size = (share_buff_size + 3) & ~0x3;
    decode_pcm_buffer_size = (decode_pcm_buffer_size + 3) & ~0x3;
    working_buff1_size = (working_buff1_size + 3) & ~0x3;
    working_buff2_size = (working_buff2_size + 3) & ~0x3;


    internal_handle->share_buff_size = share_buff_size;
    internal_handle->decode_pcm_buffer_size = decode_pcm_buffer_size;
    internal_handle->working_buff1_size = working_buff1_size;
    internal_handle->working_buff2_size = working_buff2_size;
    internal_handle->stream_out_pcm_buff_size = decode_pcm_buffer_size * 3;

    //get total memory size for mp3
    *memory_size = internal_handle->share_buff_size + internal_handle->decode_pcm_buffer_size + internal_handle->working_buff1_size + internal_handle->working_buff2_size + internal_handle->stream_out_pcm_buff_size;

#if 0
    MP3_LOG_I("[MP3 Codec]internal_handle->share_buff_size=%u\n", 1, internal_handle->share_buff_size);
    MP3_LOG_I("[MP3 Codec]internal_handle->pcm_buff_size=%u\n", 1, internal_handle->pcm_buff_size);
    MP3_LOG_I("[MP3 Codec]internal_handle->working_buff1_size=%u\n", 1, internal_handle->working_buff1_size);
    MP3_LOG_I("[MP3 Codec]internal_handle->working_buff2_size=%u\n", 1, internal_handle->working_buff2_size);
#endif

    return MP3_CODEC_RETURN_OK;

}
#endif


#if 1
#if defined(MTK_AUDIO_MIXER_SUPPORT)
mp3_codec_function_return_state_t mp3_codec_set_memory2(mp3_codec_media_handle_t *handle)
#else
mp3_codec_function_return_state_t mp3_codec_set_memory2(void)
#endif
{
#if defined(MTK_AUDIO_MIXER_SUPPORT)
    mp3_codec_internal_handle_t *internal_handle = mp3_handle_ptr_to_internal_ptr(handle);
#else
    mp3_codec_media_handle_t *handle = global_mp3_handle; // backward compatibility for "mp3_codec_set_memory2()"
    mp3_codec_internal_handle_t *internal_handle = mp3_handle_ptr_to_internal_ptr(global_mp3_handle);
#endif
    if (internal_handle == NULL) {
        MP3_LOG_E("[MP3 Codec]mp3_codec_internal_handle = NULL\n", 0);
        return MP3_CODEC_RETURN_ERROR;
    }

#ifdef MTK_AVM_DIRECT
    int channel_number = 0;
    /* STEP 0: Get channel information from MP3 bit stream */
    channel_number = MP3Dec_GetChannelNumber(handle->share_buff.buffer_base, handle->share_buff.buffer_size);

    /* Mono or stereo channel */
    if ((channel_number == 1) || (channel_number == 2)) {
        MP3_LOG_I("[MP3 Codec]MP3 Channel Number = %d\n", 1, channel_number);
#ifndef AIR_MP3_STEREO_SUPPORT_ENABLE
        if (channel_number == 2) {
            MP3_LOG_I("[MP3 Codec]Stereo format is not supported!", 0);
            AUDIO_ASSERT(0);
            //MP3_LOG_E("[MP3 Codec]Stereo format not supported!\n", 0);
            //return MP3_CODEC_RETURN_ERROR;
        }
#endif
    } else {
        MP3_LOG_E("[MP3 Codec]Cannot get MP3 channel information\n", 0);
        return MP3_CODEC_RETURN_ERROR;
    }
#else
    if (handle->state != MP3_CODEC_STATE_READY) {
        return MP3_CODEC_RETURN_ERROR;
    }
#endif

    uint8_t *memory_base = NULL;
    int working_buff1_size,         /* the required Working buffer1 size in byte    */
        working_buff2_size,         /* the required Working buffer2 size in byte    */
        decode_pcm_buffer_size,              /* the required pcm buffer size in byte          */
        share_buff_size;            /* the share buffer size      */

    /*STEP 1 : Allocate data memory for MP3 decoder*/
#ifdef MTK_AVM_DIRECT
    if (channel_number == 1) {
        MP3Dec_GetMemSize_Mono(&share_buff_size, &decode_pcm_buffer_size, &working_buff1_size, &working_buff2_size);
    } else {
        MP3Dec_GetMemSize(&share_buff_size, &decode_pcm_buffer_size, &working_buff1_size, &working_buff2_size);
    }
#else
    MP3Dec_GetMemSize(&share_buff_size, &decode_pcm_buffer_size, &working_buff1_size, &working_buff2_size);
#endif

#ifdef HAL_AUDIO_LOW_POWER_ENABLED
    share_buff_size = share_buff_size + 4096; // = 8 KB = 8192 = 4096 + 4096
#else
    share_buff_size = share_buff_size + 1024;   // since we add share_buffer_data_amount <= SHARE_BUFFER_TOO_LESS_AMOUNT feature so add 1024 to compensate it.
#endif

    //4bytes aligned
    share_buff_size = (share_buff_size + 3) & ~0x3;
    decode_pcm_buffer_size = (decode_pcm_buffer_size + 3) & ~0x3;
    working_buff1_size = (working_buff1_size + 3) & ~0x3;
    working_buff2_size = (working_buff2_size + 3) & ~0x3;

    internal_handle->share_buff_size = share_buff_size;
    internal_handle->decode_pcm_buffer_size = decode_pcm_buffer_size;
    internal_handle->working_buff1_size = working_buff1_size;
    internal_handle->working_buff2_size = working_buff2_size;
#ifdef HAL_AUDIO_LOW_POWER_ENABLED
    internal_handle->stream_out_pcm_buff_size = decode_pcm_buffer_size * 3 * 2;
#else
#ifdef MTK_AVM_DIRECT
    internal_handle->stream_out_pcm_buff_size = decode_pcm_buffer_size * 2;
#else
    internal_handle->stream_out_pcm_buff_size = decode_pcm_buffer_size * 3;
#endif
#endif
    //Calculate required memory space for decoder
    int size_memory_pool = 0;
#ifdef MTK_AVM_DIRECT
    if (handle->linear_buff == 0) {
        size_memory_pool += share_buff_size;
    }

    //Reuse stream_out_pcm_buf as decode_pcm_buf, so no need extra memory for stream_out_buf
    size_memory_pool += (internal_handle->working_buff1_size
                         + internal_handle->working_buff2_size
                         + internal_handle->stream_out_pcm_buff_size);
#else
    size_memory_pool = MP3_DECODE_BUFFER_SIZE;
#endif

    //specify memory pool
#if !defined(MTK_AVM_DIRECT)
    mp3_handle_ptr_to_mp3_decode_buffer(handle) = (uint8_t *)pvPortMalloc(size_memory_pool * sizeof(uint8_t));
    if (NULL == mp3_handle_ptr_to_mp3_decode_buffer(handle)) {
        MP3_LOG_E("[MP3 Codec] mp3_codec_set_memory2 fail: cannot allocate mp3 decode buffer\n", 0);
        return MP3_CODEC_RETURN_ERROR;
    }
#else
    if (size_memory_pool > sizeof(mp3_codec_decode)) {
        MP3_LOG_E("MP3 codec memory size error!!! memory_pool size(%d) > mp3_codec_decode(%d)\n", 2, size_memory_pool, sizeof(mp3_codec_decode));
        MP3_LOG_I("MP3 codec memory size error!!!", 0);
        AUDIO_ASSERT(0);
    }
    memset(mp3_codec_decode, 0, sizeof(mp3_codec_decode));
    mp3_handle_ptr_to_mp3_decode_buffer(handle) = mp3_codec_decode;
#endif
    internal_handle->memory_pool = memory_base = mp3_handle_ptr_to_mp3_decode_buffer(handle);


#ifndef MTK_AVM_DIRECT
    //set share buffer
    mp3_codec_set_share_buffer(handle, memory_base, share_buff_size);
    memory_base += share_buff_size;
    // set decode_pcm_buffer
    internal_handle->decode_pcm_buff.buffer_base_pointer = memory_base;
    internal_handle->decode_pcm_buff.buffer_byte_count = internal_handle->decode_pcm_buffer_size;
    internal_handle->decode_pcm_buff.read_pointer = 0;
    internal_handle->decode_pcm_buff.write_pointer = 0;
    memory_base += internal_handle->decode_pcm_buffer_size;
#endif

    //set working buffer
    internal_handle->working_buff1 = memory_base;
    memory_base += internal_handle->working_buff1_size;
    internal_handle->working_buff2 = memory_base;
    memory_base += internal_handle->working_buff2_size;

    // set PCM buffer
    internal_handle->stream_out_pcm_buff.buffer_base_pointer = memory_base;
    internal_handle->stream_out_pcm_buff.buffer_byte_count = internal_handle->stream_out_pcm_buff_size;
    internal_handle->stream_out_pcm_buff.read_pointer = 0;
    internal_handle->stream_out_pcm_buff.write_pointer = 0;




    /*STEP 2 : Get MP3 Handler */
#ifdef MTK_AVM_DIRECT
    if (channel_number == 1) {
        internal_handle->mp3_handle = MP3Dec_Init_Mono(internal_handle->working_buff1, internal_handle->working_buff2);
    } else {
        internal_handle->mp3_handle = MP3Dec_Init(internal_handle->working_buff1, internal_handle->working_buff2);
    }
#else
    internal_handle->mp3_handle = MP3Dec_Init(internal_handle->working_buff1, internal_handle->working_buff2);
#endif
    if (internal_handle->mp3_handle == NULL) {
        MP3_LOG_E("[MP3 Codec]MP3Dec_Init fail", 0);
        return MP3_CODEC_RETURN_ERROR;
    }

#if 0

    MP3_LOG_I("share_buff_size = %d\r\n", 1, share_buff_size);
    MP3_LOG_I("decode_pcm_buffer_size = %d\r\n", 1, decode_pcm_buffer_size);
    MP3_LOG_I("working_buff1_size = %d\r\n", 1, working_buff1_size);
    MP3_LOG_I("working_buff2_size = %d\r\n", 1, working_buff2_size);

    MP3_LOG_I("internal_handle->share_buff_size = %d\r\n", 1, internal_handle->share_buff_size);
    MP3_LOG_I("internal_handle->decode_pcm_buffer_size = %d\r\n", 1, internal_handle->decode_pcm_buffer_size);
    MP3_LOG_I("internal_handle->working_buff1_size = %d\r\n", 1, internal_handle->working_buff1_size);
    MP3_LOG_I("internal_handle->working_buff2_size = %d\r\n", 1, internal_handle->working_buff2_size);

    MP3_LOG_I("internal_handle->stream_out_pcm_buff_size = %d\r\n", 1, internal_handle->stream_out_pcm_buff_size);

#endif

#if 0
    MP3_LOG_I("[MP3 Codec]set_memory memory range : start=%08x end=%08x", 2,
              &mp3_decode_buffer[0], &mp3_decode_buffer[40999]);
    MP3_LOG_I("[MP3 Codec]set_memory share buffer : base=%08x size=%u", 2,
              handle->share_buff.buffer_base, (unsigned int)handle->share_buff.buffer_size);
    MP3_LOG_I("[MP3 Codec]set_memory decode_pcm_buff.buffer : base=%08x size=%u", 2,
              internal_handle->decode_pcm_buff.buffer_base_pointer, (unsigned int)internal_handle->decode_pcm_buff.buffer_byte_count);
    MP3_LOG_I("[MP3 Codec]set_memory work1 buffer : base=%08x size=%u", 2,
              internal_handle->working_buff1, (unsigned int)internal_handle->working_buff1_size);
    MP3_LOG_I("[MP3 Codec]set_memory work2 buffer : base=%08x size=%u", 2,
              internal_handle->working_buff2, (unsigned int)internal_handle->working_buff2_size);
    MP3_LOG_I("[MP3 Codec]set_memory stream_out_pcm_buff : base=%08x size=%u", 2,
              internal_handle->stream_out_pcm_buff.buffer_base_pointer, (unsigned int)internal_handle->stream_out_pcm_buff.buffer_byte_count);
#endif

    return MP3_CODEC_RETURN_OK;

}
#endif

void mp3_codec_event_send_from_isr(mp3_codec_queue_event_id_t id, void *parameter)
{
    mp3_codec_queue_event_t event;
    event.id        = id;
    event.parameter = parameter;

    if (mp3_handle_ptr_to_queue_handle(parameter) != NULL) {
        if (HAL_NVIC_QUERY_EXCEPTION_NUMBER == HAL_NVIC_NOT_EXCEPTION) { // task level
            if (xQueueSend(mp3_handle_ptr_to_queue_handle(parameter), &event, 0) != pdTRUE) {
                MP3_LOG_I("[MP3 Codec]Send queue error. Queue full (Drop: %d\r\n", 1, id);
            }
            return;
        } else { // interrupt level
            if (pdFALSE == xQueueIsQueueFullFromISR(mp3_handle_ptr_to_queue_handle(parameter))) {
                BaseType_t xHigherPriorityTaskWoken = pdFALSE;
                xQueueSendFromISR(mp3_handle_ptr_to_queue_handle(parameter), &event, &xHigherPriorityTaskWoken);
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            } else {
                MP3_LOG_I("[MP3 Codec]Send queue error. Queue full (Drop: %d\r\n", 1, id);
            }
            return;
        }
    } else {
        MP3_LOG_D("mp3_codec_queue_handle NULL\r\n", 0);
    }
    return;
}

static void mp3_codec_event_register_callback(mp3_codec_media_handle_t *handle, mp3_codec_queue_event_id_t reg_id, mp3_codec_internal_callback_t callback)
{
    uint32_t id_idx;
    for (id_idx = 0; id_idx < MAX_MP3_CODEC_FUNCTIONS; id_idx++) {
        if ((mp3_handle_ptr_to_queue_id_array(handle))[id_idx] == MP3_CODEC_QUEUE_EVENT_NONE) {
            (mp3_handle_ptr_to_queue_id_array(handle))[id_idx] = reg_id;
            (mp3_handle_ptr_to_queue_handler(handle))[id_idx] = callback;
            break;
        }
    }
    return;
}

#ifndef AIR_MP3_TASK_DEDICATE_ENABLE
static void mp3_codec_event_deregister_callback(mp3_codec_media_handle_t *handle, mp3_codec_queue_event_id_t dereg_id)
{
    uint32_t id_idx;
    for (id_idx = 0; id_idx < MAX_MP3_CODEC_FUNCTIONS; id_idx++) {
        if ((mp3_handle_ptr_to_queue_id_array(handle))[id_idx] == dereg_id) {
            (mp3_handle_ptr_to_queue_id_array(handle))[id_idx] = MP3_CODEC_QUEUE_EVENT_NONE;
            break;
        }
    }
    return;
}
#endif //AIR_MP3_TASK_DEDICATE_ENABLE

extern void prompt_control_mp3_callback(mp3_codec_media_handle_t *handle, mp3_codec_event_t event);
static void mp3_codec_decode_open_handler(void *data);
static void mp3_codec_decode_stop_handler(void *data);
static void mp3_codec_deocde_hisr_handler(void *data);
static void mp3_codec_decode_sync_timeout_handler(void *data);
void mp3_codec_task_main(void *arg)
{
    MP3_LOG_I("[MP3 Codec]mp3_codec_task_main create\n", 0);

#ifdef AIR_MP3_TASK_DEDICATE_ENABLE
    mp3_codec_open(prompt_control_mp3_callback);
    arg = (mp3_codec_media_handle_t *)g_mp3_codec_task_handle;

    mp3_codec_event_register_callback(arg, MP3_CODEC_QUEUE_EVENT_DECODE,       mp3_codec_deocde_hisr_handler);
    mp3_codec_event_register_callback(arg, MP3_CODEC_QUEUE_EVENT_PLAY,         mp3_codec_decode_open_handler);
    mp3_codec_event_register_callback(arg, MP3_CODEC_QUEUE_EVENT_STOP,         mp3_codec_decode_stop_handler);
    mp3_codec_event_register_callback(arg, MP3_CODEC_QUEUE_EVENT_SYNC_TRIGGER, mp3_codec_decode_sync_timeout_handler);
#endif
    mp3_codec_queue_event_t event;

    while (1) {
        if (xQueueReceive(mp3_handle_ptr_to_queue_handle(arg), &event, portMAX_DELAY)) {
            //LOGI("[MP3 Codec]xQueueReceive event\n");
            mp3_codec_queue_event_id_t rece_id = event.id;
            uint32_t id_idx;
            for (id_idx = 0; id_idx < MAX_MP3_CODEC_FUNCTIONS; id_idx++) {
                if ((mp3_handle_ptr_to_queue_id_array(arg))[id_idx] == rece_id) {
                    (mp3_handle_ptr_to_queue_handler(arg))[id_idx](event.parameter);
                    break;
                }
            }
        }
    }
}


#ifndef AIR_MP3_TASK_DEDICATE_ENABLE
static void mp3_codec_task_create(mp3_codec_media_handle_t *handle)
{
    if (mp3_handle_ptr_to_task_handle(handle) ==  NULL) {
        //LOGI("[MP3 Codec] create mp3 task\r\n");
        xTaskCreate(mp3_codec_task_main, MP3_CODEC_TASK_NAME, MP3_CODEC_TASK_STACKSIZE / sizeof(StackType_t), handle, MP3_CODEC_TASK_PRIO, &(mp3_handle_ptr_to_task_handle(handle)));
        MP3_LOG_I("[MP3 Codec] create mp3 task(0x%x)\r\n", 1, mp3_handle_ptr_to_task_handle(handle));
    }
}
#endif

static hal_audio_channel_number_t mp3_codec_translate_decoder_ip_channel_number(uint16_t channel_number)
{
    hal_audio_channel_number_t hal_audio_channel_number = HAL_AUDIO_STEREO;

    hal_audio_channel_number = (channel_number == 1) ? HAL_AUDIO_MONO : HAL_AUDIO_STEREO;

    return hal_audio_channel_number;
}

static hal_audio_sampling_rate_t mp3_codec_translate_decoder_ip_sample_rate_index(uint16_t sample_rate_index)
{

    hal_audio_sampling_rate_t hal_audio_sampling_rate = HAL_AUDIO_SAMPLING_RATE_44_1KHZ;

    switch (sample_rate_index) {
        case 0:
            hal_audio_sampling_rate = HAL_AUDIO_SAMPLING_RATE_44_1KHZ;
            break;
        case 1:
            hal_audio_sampling_rate = HAL_AUDIO_SAMPLING_RATE_48KHZ;
            break;
        case 2:
            hal_audio_sampling_rate = HAL_AUDIO_SAMPLING_RATE_32KHZ;
            break;
        case 3:
            hal_audio_sampling_rate = HAL_AUDIO_SAMPLING_RATE_22_05KHZ;
            break;
        case 4:
            hal_audio_sampling_rate = HAL_AUDIO_SAMPLING_RATE_24KHZ;
            break;
        case 5:
            hal_audio_sampling_rate = HAL_AUDIO_SAMPLING_RATE_16KHZ;
            break;
        case 6:
            hal_audio_sampling_rate = HAL_AUDIO_SAMPLING_RATE_11_025KHZ;
            break;
        case 7:
            hal_audio_sampling_rate = HAL_AUDIO_SAMPLING_RATE_12KHZ;
            break;
        case 8:
            hal_audio_sampling_rate = HAL_AUDIO_SAMPLING_RATE_8KHZ;
            break;
    }
    return hal_audio_sampling_rate;
}

#if 0
static uint16_t mp3_codec_get_stream_out_slience_tone(void)
{
    mp3_codec_internal_handle_t *internal_handle = mp3_codec_internal_handle;
    uint16_t silence_tone = 0;

    uint8_t *out_buf_ptr    = NULL;
    uint32_t stream_out_pcm_buffer_data_count = 0;
    ring_buffer_get_read_information(&internal_handle->stream_out_pcm_buff, &out_buf_ptr, &stream_out_pcm_buffer_data_count);

    if ((out_buf_ptr - internal_handle->stream_out_pcm_buff.buffer_base_pointer) >= 2) {
        silence_tone = *((uint16_t *)(out_buf_ptr - 2));
    } else if (out_buf_ptr == internal_handle->stream_out_pcm_buff.buffer_base_pointer) {
        silence_tone = *((uint16_t *)(internal_handle->stream_out_pcm_buff.buffer_base_pointer + internal_handle->stream_out_pcm_buff.buffer_byte_count - 2));
    } else {
        silence_tone = (uint16_t)(*(internal_handle->stream_out_pcm_buff.buffer_base_pointer + internal_handle->stream_out_pcm_buff.buffer_byte_count - 1)) << 0;
        silence_tone = (uint16_t)(*(internal_handle->stream_out_pcm_buff.buffer_base_pointer)) << 8;
    }

    return silence_tone;
}

static void mp3_codec_put_silence_samples_to_stream_out_pcm_buffer(uint32_t sliences_sample_amount)
{
    mp3_codec_internal_handle_t *internal_handle = mp3_codec_internal_handle;

    uint16_t slience_tone = mp3_codec_get_stream_out_slience_tone();

    // TODO: we may get stereo silence, not only mono

    uint32_t loop_idx = 0;
    uint32_t decoded_data_count = sliences_sample_amount * 2 * internal_handle->mp3_handle->CHNumber; // word -> bytes

    decoded_data_count = (decoded_data_count > 0) ? decoded_data_count : (1152 * 2 * 2);    // 1152: mp3 one frame max samples; 2(stereo) * 2 (two bytes)

    for (loop_idx = 0; loop_idx < 2; loop_idx++) {
        uint8_t *out_buf_ptr    = NULL;
        uint32_t out_byte_count = 0;
        ring_buffer_get_write_information(&internal_handle->stream_out_pcm_buff, &out_buf_ptr, &out_byte_count);
        if (out_byte_count > 0) {
            uint32_t consumed_byte_count = MINIMUM(decoded_data_count, out_byte_count);
            uint32_t i = 0;
            for (i = 0; i < consumed_byte_count / 2; i += 2) {
                *((uint16_t *)(out_buf_ptr + i)) = slience_tone;
            }

            decoded_data_count  -= consumed_byte_count;
            ring_buffer_write_done(&internal_handle->stream_out_pcm_buff, consumed_byte_count);
        } else {
            // stream_out_pcm_buffer full, do nothing.
            MP3_LOG_D("mp3_codec_put_one_frame_silence_samples_to_stream_out_pcm_buffer: stream_out_pcm_buffer full\r\n", 0);
            break;
        }
    }
}
#endif

static void mp3_codec_pcm_out_isr_callback(hal_audio_event_t event, void *data)
{
    mp3_codec_media_handle_t *handle = (mp3_codec_media_handle_t *)data;
    if (data == NULL) {
        return;
    } else if (handle->state != MP3_CODEC_STATE_PLAY) {
        return;
    }
#if defined(MTK_AUDIO_MIXER_SUPPORT)
    uint32_t total_stream_out_amount = 0;
    uint32_t current_cnt = 0;
#endif
#ifdef AIR_MP3_TASK_DEDICATE_ENABLE
    mp3_codec_mutex_lock(handle);
#endif
    mp3_codec_internal_handle_t *internal_handle = mp3_handle_ptr_to_internal_ptr(data);
    uint32_t sample_count = 0;
    uint32_t stream_out_amount = 0;
    uint32_t loop_idx = 0;
#if defined(MTK_AUDIO_MP3_DEBUG)
    MP3_LOG_I("[MP3 Codec] isr event(%d)\r\n", 1, event);
#endif
    if (1 == internal_handle->media_bitstream_end) {
        MP3_LOG_I("[MP3 Codec] isr just return becase media_bitstream_end == 1\r\n", 0);
#if defined(MTK_AUDIO_MIXER_SUPPORT)
#ifndef AIR_MP3_TASK_DEDICATE_ENABLE
        handle->handler(handle, MP3_CODEC_MEDIA_BITSTREAM_END);
#endif
#else
        mp3_codec_event_send_from_isr(MP3_CODEC_QUEUE_EVENT_DECODE, data); // "handle->handler(handle, MP3_CODEC_MEDIA_BITSTREAM_END)" is called in "mp3_codec_deocde_hisr_handler"
#endif
#ifdef AIR_MP3_TASK_DEDICATE_ENABLE
        mp3_codec_mutex_unlock(handle);
#endif
        return;
    }

    uint32_t one_frame_samples = internal_handle->mp3_handle->PCMSamplesPerCH * internal_handle->mp3_handle->CHNumber;
    one_frame_samples = (one_frame_samples > 0) ? one_frame_samples : (1152 * 2);    // 1152( mp3 one frame max samples) * 2(stereo)

    uint32_t stream_out_data_amount_bytes = ring_buffer_get_data_byte_count(&internal_handle->stream_out_pcm_buff);
    //printf("1curr_cnt=%d, stream_out_data_amount_bytes=%d, one_frame_bytes=%d\r\n", (curr_cnt/1000), stream_out_data_amount_bytes, one_frame_samples*2);

    if ((stream_out_data_amount_bytes / 2) < (one_frame_samples / 4)) {
#if defined(MTK_AUDIO_MP3_DEBUG)
        uint32_t curr_cnt = 0;
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &curr_cnt);
        MP3_LOG_I("[MP3 Codec] isr(%d), pcm data(%d) less 1/4 frame\r\n", 2, (curr_cnt / 1000), stream_out_data_amount_bytes, (one_frame_samples * 2));
#endif
        //mp3_codec_put_silence_samples_to_stream_out_pcm_buffer(2 * one_frame_samples - (stream_out_data_amount_bytes / 2)); // 2(gave one more frame for buffer to wait decode) *
        //printf("curr_cnt=%d, before=%d, after=%d, perCH=%d, CHN=%d\r\n", (curr_cnt/1000), stream_out_data_amount_bytes, ring_buffer_get_data_byte_count(&internal_handle->stream_out_pcm_buff), internal_handle->mp3_handle->PCMSamplesPerCH, internal_handle->mp3_handle->CHNumber);
    }

    switch (event) {
        case HAL_AUDIO_EVENT_UNDERFLOW:
        case HAL_AUDIO_EVENT_DATA_REQUEST:
#if defined(MTK_AUDIO_MIXER_SUPPORT)
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_cnt);
#endif
            for (loop_idx = 0; loop_idx < 2; loop_idx++) {
                uint8_t *out_buf_ptr    = NULL;
                uint32_t stream_out_pcm_buffer_data_count = 0;

#if defined(MTK_AVM_DIRECT)
                {
                    n9_dsp_share_info_t *p_info;

                    p_info = (n9_dsp_share_info_t *)hal_audio_query_share_info(internal_handle->message_type);

                    sample_count = hal_audio_buf_mgm_get_free_byte_count(p_info);
                }
#elif defined(MTK_AUDIO_MIXER_SUPPORT)
                audio_mixer_query_free_count(handle->mixer_track_id, &sample_count);
#else
                hal_audio_get_stream_out_sample_count(&sample_count);
#endif
                ring_buffer_get_read_information(&internal_handle->stream_out_pcm_buff, &out_buf_ptr, &stream_out_pcm_buffer_data_count);

                if (stream_out_pcm_buffer_data_count > 0) {
                    stream_out_amount = MINIMUM(sample_count, stream_out_pcm_buffer_data_count);
#if defined(MTK_AUDIO_MP3_DEBUG)
                    uint32_t curr_cnt = 0;
                    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &curr_cnt);
                    MP3_LOG_I("[MP3 Codec] curr_cnt=%d, write stream_out_pcm_buff. out_buf_ptr = 0x%x, stream_out_amount =%d \r\n", 3, (curr_cnt / 1000), out_buf_ptr, stream_out_amount);
#endif
#if defined(MTK_AVM_DIRECT)
#ifdef AIR_MP3_TASK_DEDICATE_ENABLE
                    if (!(g_vp_task_mask & VP_LOCAL_MASK_MUTE_SHARED_BUF)) {
#endif
                        hal_audio_write_stream_out_by_type(internal_handle->message_type, out_buf_ptr, stream_out_amount);
#ifdef AIR_MP3_TASK_DEDICATE_ENABLE
                    }
#endif
#elif defined(MTK_AUDIO_MIXER_SUPPORT)
                    audio_mixer_write_data(handle->mixer_track_id, out_buf_ptr, stream_out_amount);
                    total_stream_out_amount += stream_out_amount;
#else
                    hal_audio_write_stream_out(out_buf_ptr, stream_out_amount);
#endif

                    ring_buffer_read_done(&internal_handle->stream_out_pcm_buff, stream_out_amount);

                    if ((sample_count - stream_out_amount) == 0) { // if first time sample_count < stream_out_pcm_buffer_data_count, then this will avoid loop again
                        break;
                    }
                } else {    /* 16kHz speech Tx buffer is empty */
#if defined(MTK_AUDIO_MP3_DEBUG)
                    // stream_out_pcm_buff no data
                    uint32_t curr_cnt = 0;
                    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &curr_cnt);
                    MP3_LOG_I("[MP3 Codec] curr_cnt=%d, stream_out_pcm_buff no data. loop_idx=%d\r\n", 2, (curr_cnt / 1000), loop_idx);
#endif
#if defined(MTK_AVM_DIRECT)
#ifdef AIR_MP3_TASK_DEDICATE_ENABLE
                    if (!(g_vp_task_mask & VP_LOCAL_MASK_MUTE_SHARED_BUF)) {
#endif
                        hal_audio_write_stream_out_by_type(internal_handle->message_type, out_buf_ptr, 0);
#ifdef AIR_MP3_TASK_DEDICATE_ENABLE
                    }
#endif
#elif defined(MTK_AUDIO_MIXER_SUPPORT)
                    audio_mixer_write_data(handle->mixer_track_id, out_buf_ptr, 0);
#else
                    hal_audio_write_stream_out(out_buf_ptr, 0);     // must have this or the isr will not to request again
#endif
                    break;
                }

            }

#if defined(MTK_AUDIO_MIXER_SUPPORT)
            /*            if (mp3_handle_ptr_to_pcm_isr_cnt(handle) > 0) {
                            MP3_LOG_I("[MP3 Codec] role%d, isr(%d), wdc(%d)\n",3,
                                handle->mixer_track_role,
                                (current_cnt - mp3_handle_ptr_to_pcm_isr_cnt(handle)) / 1000,
                                total_stream_out_amount);
                        }*/
            mp3_handle_ptr_to_pcm_isr_cnt(handle) = current_cnt;
#endif

            mp3_codec_event_send_from_isr(MP3_CODEC_QUEUE_EVENT_DECODE, data);

            break;
        case HAL_AUDIO_EVENT_END: {
            uint32_t curr_cnt = 0;
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &curr_cnt);
            MP3_LOG_I("[MP3 Codec] isr(%d), MP3_CODEC_MEDIA_BITSTREAM_END\r\n", 1, (curr_cnt / 1000));
#if defined(MTK_AUDIO_MIXER_SUPPORT)
            MP3_LOG_I("[MP3 Codec] isr(%d): handle->handler(handle, MP3_CODEC_MEDIA_BITSTREAM_END);\r\n", 1, (curr_cnt / 1000));
#endif
        }
        internal_handle->media_bitstream_end = 1;

#if defined(MTK_AUDIO_MIXER_SUPPORT)
        handle->handler(handle, MP3_CODEC_MEDIA_BITSTREAM_END);
#endif
        break;

        case HAL_AUDIO_EVENT_ERROR:
        case HAL_AUDIO_EVENT_NONE:
            MP3_LOG_I("not here\r\n", 0);
            break;
        case HAL_AUDIO_EVENT_DATA_DIRECT:
            MP3_LOG_I("[VPC] VP dsp triggered\r\n", 0);
            if (g_app_callback) {
                MP3_LOG_I("[VPC] aws VP start trigger\n", 0);
                g_app_callback(PROMPT_CONTROL_MEDIA_PLAY);
            }
            break;
    }
#ifdef AIR_MP3_TASK_DEDICATE_ENABLE
    mp3_codec_mutex_unlock(handle);
#endif
#if defined(MTK_AUDIO_MIXER_SUPPORT)
    UNUSED(total_stream_out_amount);
#endif
    return;
}


#if 0
static void mp3_codec_fill_data_hisr_handler(void *data)
{
    MP3_LOG_I("[MP3 Codec]mp3_codec_fill_data_hisr_handler\n", 0);

    uint32_t sample_count = 0;
    uint32_t PCM_length = 0;
    uint32_t segment = 0;
    uint8_t *PCM_Src = NULL;
    mp3_codec_internal_handle_t *internal_handle = mp3_codec_internal_handle;
    PCM_Src = internal_handle->PCM_ring.buffer_base_pointer;
    PCMSmples = internal_handle->decHandle->PCMSamplesPerCH * internal_handle->decHandle->CHNumber * 2;
    MP3_LOG_I("[MP3 Codec]PCMSmples=%u\n", 1, PCMSmples);
    do {
        hal_audio_get_stream_out_sample_count(&sample_count);
        MP3_LOG_I("[MP3 Codec]sample_count=%u\n", 1, sample_count);
        segment = MINIMUM(PCM_length, sample_count);
        hal_audio_write_stream_out(PCM_Src, segment);
        PCM_length -= segment;
        PCM_Src += segment;
        //LOGI("[MP3 Codec]PCMSmples=%u\n",PCMSmples);
    } while (PCM_length != 0);

    //mp3_codec_event_send_from_isr(MP3_CODEC_QUEUE_EVENT_DECODE, NULL);
}
#endif

static int32_t MP3_MPEG1_AUDIO_FRAME_BIT_RATE[] = { // MPEG 1 Layer 3
    0,
    32000,
    40000,
    48000,
    56000,
    64000,
    80000,
    96000,
    112000,
    128000,
    160000,
    192000,
    224000,
    256000,
    320000,
    0,
};

static int32_t MP3_MPEG2_AUDIO_FRAME_BIT_RATE[] = { // MPEG 2 or MPEG 2.5 Layer 3
    0,
    8000,
    16000,
    24000,
    32000,
    40000,
    48000,
    56000,
    64000,
    80000,
    96000,
    112000,
    128000,
    144000,
    160000,
    0,
};

static int32_t MP3_MPEG1_AUDIO_SAMPLING_RATE[] = {44100, 48000, 32000, 0};
static int32_t MP3_MPEG2_AUDIO_SAMPLING_RATE[] = {22050, 24000, 16000, 0};
static int32_t MP3_MPEG25_AUDIO_SAMPLING_RATE[] = {11025, 12000, 8000, 0};

typedef enum {
    MP3_CODEC_RETURN_NEXT_FRAME_SIZE_INVALID_FRAME_SIZE = -2,
    MP3_CODEC_RETURN_NEXT_FRAME_SIZE_INVALID_SECOND_FRAME_HEDAER = -1,
    MP3_CODEC_RETURN_NEXT_FRAME_SIZE_OK = 0,
} mp3_codec_function_return_next_frame_size_t;

#define MPEG1 (3)
#define MPEG2 (2)
#define MPEG25 (0)
#define LAYER3 (1)

static int32_t mp3_codec_get_next_mp3_frame_size(uint8_t *mp3_header, uint8_t *buffer_base, int32_t buffer_size, int32_t *mp3_frame_size)
{
    uint32_t temp_mp3_header = 0;
    int32_t bitrate = 0;
    int32_t samplingRate = 0;
    int32_t paddingBit = 0;
    int32_t frameSize = 0;
    int32_t ret = MP3_CODEC_RETURN_NEXT_FRAME_SIZE_INVALID_FRAME_SIZE; // mp3_header does NOT match MP3 Header
    uint8_t *buffer_end = buffer_base + buffer_size - 1;
    int8_t mpegVer, layerVer;
    int32_t mpegFactor;

    *mp3_frame_size = 0;
    temp_mp3_header = *mp3_header;
    temp_mp3_header = (temp_mp3_header << 8) | (((mp3_header + 1) > (buffer_end)) ? * (buffer_base + (mp3_header + 1 - buffer_end - 1)) : * (mp3_header + 1)); // *(mp3_header + 1);
    temp_mp3_header = (temp_mp3_header << 8) | (((mp3_header + 2) > (buffer_end)) ? * (buffer_base + (mp3_header + 2 - buffer_end - 1)) : * (mp3_header + 2)); // *(mp3_header + 2);
    temp_mp3_header = (temp_mp3_header << 8) | (((mp3_header + 3) > (buffer_end)) ? * (buffer_base + (mp3_header + 3 - buffer_end - 1)) : * (mp3_header + 3)); // *(mp3_header + 3);

    if (IS_MP3_HEAD(temp_mp3_header)) {
        layerVer = (temp_mp3_header >> 17) & 0x3;
        if (layerVer != LAYER3) {
            MP3_LOG_I("[MP3 Codec] mp3_codec_get_next_mp3_frame_size: Cannot parse non-Layer 3!!!\n", 0);
            return MP3_CODEC_RETURN_NEXT_FRAME_SIZE_INVALID_FRAME_SIZE;
        }
        mpegVer = (temp_mp3_header >> 19) & 0x3;
        switch (mpegVer) {
            case MPEG1: // MPEG Version 1
                // LOGI("[MP3 Codec] MPEG1\n");
                bitrate = MP3_MPEG1_AUDIO_FRAME_BIT_RATE[((temp_mp3_header >> 12) & 0xF)];
                samplingRate = MP3_MPEG1_AUDIO_SAMPLING_RATE[(temp_mp3_header >> 10) & 0x3];
                mpegFactor = 144;
                break;
            case MPEG2: // MPEG Version 2
                // LOGI("[MP3 Codec] MPEG2\n");
                bitrate = MP3_MPEG2_AUDIO_FRAME_BIT_RATE[((temp_mp3_header >> 12) & 0xF)];
                samplingRate = MP3_MPEG2_AUDIO_SAMPLING_RATE[(temp_mp3_header >> 10) & 0x3];
                mpegFactor = 72;
                break;
            case MPEG25: // MPEG Version 2.5
                // LOGI("[MP3 Codec] MPEG25\n");
                bitrate = MP3_MPEG2_AUDIO_FRAME_BIT_RATE[((temp_mp3_header >> 12) & 0xF)];
                samplingRate = MP3_MPEG25_AUDIO_SAMPLING_RATE[(temp_mp3_header >> 10) & 0x3];
                mpegFactor = 72;
                break;
            default:
                return MP3_CODEC_RETURN_NEXT_FRAME_SIZE_INVALID_FRAME_SIZE;
        }

        paddingBit = (temp_mp3_header >> 9) & 0x1;
        frameSize = ((mpegFactor * bitrate) / samplingRate) + paddingBit;
        // LOGI("[MP3 Codec] frameSize = %d, bitrate = %d, samplingRate = %d, paddingBit = %d\n", frameSize, bitrate, samplingRate, paddingBit);
        if (frameSize > buffer_size) {
            *mp3_frame_size = 0;
            MP3_LOG_I("[MP3 Codec] Set frameSize to 0 because frameSize(%d) > buffer_size(%d)\n", 2, frameSize, buffer_size);
        }
        // Check the next frame
        mp3_header += frameSize;
        temp_mp3_header = (mp3_header > buffer_end) ? *(buffer_base + (mp3_header - buffer_end - 1)) : *mp3_header; // *mp3_header;
        temp_mp3_header = (temp_mp3_header << 8) | (((mp3_header + 1) > buffer_end) ? * (buffer_base + (mp3_header + 1 - buffer_end - 1)) : * (mp3_header + 1)); // *(mp3_header + 1);
        temp_mp3_header = (temp_mp3_header << 8) | (((mp3_header + 2) > buffer_end) ? * (buffer_base + (mp3_header + 2 - buffer_end - 1)) : * (mp3_header + 2)); // *(mp3_header + 2);
        temp_mp3_header = (temp_mp3_header << 8) | (((mp3_header + 3) > buffer_end) ? * (buffer_base + (mp3_header + 3 - buffer_end - 1)) : * (mp3_header + 3)); // *(mp3_header + 3);

        // LOGI("[MP3 Codec] The next frame header is 0x%04X\n", temp_mp3_header);
        *mp3_frame_size = frameSize;
        if (IS_MP3_HEAD(temp_mp3_header)) {
            // LOGI("[MP3 Codec] bitrate = %d, samplingRate = %d, paddingBit = %d, frameSize = %d\n", bitrate, samplingRate, paddingBit, frameSize);
            ret = MP3_CODEC_RETURN_NEXT_FRAME_SIZE_OK; // the next two frame is available
        } else {
            ret = MP3_CODEC_RETURN_NEXT_FRAME_SIZE_INVALID_SECOND_FRAME_HEDAER; // the next two frame is NOT available
        }
    }

    return ret;
}

#ifdef AIR_MP3_TASK_DEDICATE_ENABLE
extern void prompt_control_gpt_callback(void *user_data);
TickType_t xTimeOUT_VP = 0;
static void mp3_codec_decode_open_handler(void *data)
{
    mp3_codec_internal_handle_t *internal_handle = mp3_handle_ptr_to_internal_ptr(data);
    mp3_codec_media_handle_t *handle = &internal_handle->handle;
    uint32_t savedmask;
    if (handle->state == MP3_CODEC_STATE_PLAY) {
        if (handle->stop(handle) != MP3_CODEC_RETURN_OK) {
            MP3_LOG_E("[VPC]stop mp3 codec fail.\n", 0);
        }
        //Re-in new VP, close Voice Prompt Resource first
        hal_dvfs_lock_control(MP3_CODEC_DVFS_DEFAULT_SPEED, HAL_DVFS_UNLOCK);
        ami_hal_audio_status_set_running_flag(AUDIO_MESSAGE_TYPE_PROMPT, false);
    }

    //Open Voice Prompt Resource
#ifdef HAL_DVFS_MODULE_ENABLED
    hal_dvfs_lock_control(MP3_CODEC_DVFS_DEFAULT_SPEED, HAL_DVFS_LOCK);
#endif
    ami_hal_audio_status_set_running_flag(AUDIO_MESSAGE_TYPE_PROMPT, true);

#if !defined(MTK_AVM_DIRECT)
    prompt_control_set_mixer_volume();
#else
    prompt_control_set_volume();
#endif

    //Reset Buffer and IP Memory
    mp3_codec_reset_share_buffer(handle);
    mp3_codec_reset_stream_out_pcm_buffer(handle);    // if don't do this it will have residue data, and it will be played if you play again
    mp3_codec_set_memory2(handle);
    ami_set_audio_mask(VP_MASK_VP_HAPPENING, true);
    g_app_internal_callback = g_app_callback;

    handle->set_track_role(handle, AUDIO_MIXER_TRACK_ROLE_SIDE);
    if (handle->play(handle) != MP3_CODEC_RETURN_OK) {
        MP3_LOG_E("[VPC]play mp3 codec failed.\r\n", 0);
        handle->state = MP3_CODEC_STATE_ERROR;
        mp3_codec_event_send_from_isr(MP3_CODEC_QUEUE_EVENT_STOP, handle);
        return;
    }
    handle->flush(handle, 1);

    if (handle->aws_sync_request == true) { // check the timer
        hal_nvic_save_and_set_interrupt_mask(&savedmask);
        uint32_t curr_cnt = 0;
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &curr_cnt);
        if (handle->aws_sync_time > curr_cnt) {
            hal_nvic_restore_interrupt_mask(savedmask);
            LOG_MSGID_I(VPC, "[VPC][SYNC]GPT Timer already start.\n", 0);
        } else {
            hal_nvic_restore_interrupt_mask(savedmask);
            MP3_LOG_I("[VPC][SYNC]Timer out. (open ack too late.)\r\n", 0);
        }
    }
    mp3_codec_pcm_out_isr_callback(HAL_AUDIO_EVENT_DATA_REQUEST, (void *)g_mp3_codec_task_handle);
}

static void mp3_codec_decode_stop_handler(void *data)
{
    mp3_codec_internal_handle_t *internal_handle = mp3_handle_ptr_to_internal_ptr(data);
    mp3_codec_media_handle_t *handle = &internal_handle->handle;

    if ((handle->state == MP3_CODEC_STATE_PLAY) || (handle->state == MP3_CODEC_STATE_ERROR)) {
        if (handle->state == MP3_CODEC_STATE_ERROR) {
            MP3_LOG_E("[VPC]Clear mp3 codec flag by error occurred.\n", 0);
            return;
        }
        if (internal_handle->media_bitstream_end == true) {
            if (handle->stop(handle) != MP3_CODEC_RETURN_OK) {
                MP3_LOG_E("[VPC]stop mp3 codec fail.\n", 0);
            }
            //Close Voice Prompt Resource
#ifdef HAL_DVFS_MODULE_ENABLED
            hal_dvfs_lock_control(MP3_CODEC_DVFS_DEFAULT_SPEED, HAL_DVFS_UNLOCK);
#endif
            ami_hal_audio_status_set_running_flag(AUDIO_MESSAGE_TYPE_PROMPT, false);

            ami_set_audio_mask(VP_MASK_VP_HAPPENING, false);
#if defined(MTK_EXTERNAL_DSP_NEED_SUPPORT)
            ami_set_afe_param(STREAM_OUT_2, 0, false);
#endif
            if (g_audio_dl_suspend_by_user == false) {
                am_audio_dl_resume();
            }
            am_audio_ul_resume();
#ifdef AIR_MP3_TASK_DEDICATE_ENABLE
            if (g_app_internal_callback) {
                g_app_internal_callback(PROMPT_CONTROL_MEDIA_END);
                g_app_internal_callback = NULL;
            }
#endif
        } else {
            MP3_LOG_I("[VPC]VP stop request wait vp data ramp down.", 0);
            if (internal_handle->message_type == AUDIO_MESSAGE_TYPE_PROMPT) {
                vTaskDelay(50); // prepare total <60ms for HW GAIN ramp down to mute, in order to remove pop noise before turning off DAC.
                // clear share buffer, speed up the process of streaming out non-zero data
                n9_dsp_share_info_t *pInfo = (n9_dsp_share_info_t *)hal_audio_query_share_info(AUDIO_MESSAGE_TYPE_PROMPT);
                hal_audio_reset_share_info(pInfo);
                pInfo->bBufferIsFull = 0;
                memset((uint32_t *)pInfo->start_addr, 0, pInfo->length);
                // MSG_MCU2DSP_PROMPT_CLEAR_AUDIO_BUFFER is just a special symbol to request vp to clear all audio buffer(src buffer + afe buffer) in dsp side
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_PROMPT_CONFIG, MSG_MCU2DSP_PROMPT_CLEAR_AUDIO_BUFFER, 1, false);
                vTaskDelay(10); // prepare 10ms for DMA streaming out all non-zero data before disable DMA, in order to remove pop noise after turning on DAC next time.
                internal_handle->media_bitstream_end = 1;
                handle->handler(handle, MP3_CODEC_MEDIA_BITSTREAM_END);
            } else {
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_PLAYBACK_CONFIG, AUDIO_PLAYBACK_CONFIG_EOF, 1, false);
            }
        }
    } else {
        MP3_LOG_I("[VPC]VP does not exited no stop mp3 codec.\n", 0);
    }
}

static void mp3_codec_decode_sync_timeout_handler(void *data)
{
    mp3_codec_internal_handle_t *internal_handle = mp3_handle_ptr_to_internal_ptr(data);
    mp3_codec_media_handle_t *handle = &internal_handle->handle;
    //BaseType_t xTimerStatus;

    if (handle->state == MP3_CODEC_STATE_PLAY) {
        //LOG_I(VPC, "[VPC][SYNC]HAL send xTimeNow(%d) Timer OUT(%d).\r\n", xTaskGetTickCount(), xTimeOUT_VP);
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_PROMPT_AWS_SYNC_TRIGGER, 0, 0, false);
    } else {
        MP3_LOG_I("[VPC]VP does not exited no trigger mp3 codec.\n", 0);
    }
#if 0
    xTimerStatus = xTimerDelete(VP_AWS_SYNC_xOneShotTimer, 0);
    if (xTimerStatus != pdPASS) {
        MP3_LOG_E("[VPC][SYNC]Timer Delete error.\r\n", 0);
    }
#endif
}

#endif

static void mp3_codec_deocde_hisr_handler(void *data)
{
#if defined(MTK_AVM_DIRECT)
    mp3_audio_mutex_lock(g_xSemaphore_Audio);
#endif
    int32_t consumeBS = -1;
    uint8_t *share_buff_read_ptr = NULL;

    mp3_codec_internal_handle_t *internal_handle = mp3_handle_ptr_to_internal_ptr(data);
    mp3_codec_media_handle_t *handle = &internal_handle->handle;
    if (handle->state != MP3_CODEC_STATE_PLAY) {
#if defined(MTK_AVM_DIRECT)
        mp3_audio_mutex_unlock(g_xSemaphore_Audio);
#endif
        return;
    }

    uint32_t curr_cnt = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &curr_cnt);

    if (1 == internal_handle->media_bitstream_end) {
        MP3_LOG_I("[MP3 Codec][Race condition] hisr(%d): handle->handler(handle, MP3_CODEC_MEDIA_BITSTREAM_END);\r\n", 1, (curr_cnt / 1000));
#ifndef MTK_AUDIO_MIXER_SUPPORT
        handle->handler(handle, MP3_CODEC_MEDIA_BITSTREAM_END);
#endif
#if defined(MTK_AVM_DIRECT)
        mp3_audio_mutex_unlock(g_xSemaphore_Audio);
#endif
        return;
    }

    uint32_t one_frame_bytes = internal_handle->mp3_handle->PCMSamplesPerCH * internal_handle->mp3_handle->CHNumber * 2;
    one_frame_bytes = (one_frame_bytes > 0) ? one_frame_bytes : (1152 * 2 * 2);    // 1152( mp3 one frame max samples) * 2(stereo) * (word -> bytes)

    if (ring_buffer_get_space_byte_count(&internal_handle->stream_out_pcm_buff) < one_frame_bytes) {
        // if streamout pcm data > decode_pcm_buffer_size just return , or it will throw some data
#if defined(MTK_AUDIO_MP3_DEBUG)
        MP3_LOG_I("[MP3 Codec] hisr(%d): pcm space(%d) < one_frame_bytes(%d), just return\r\n", 3, (curr_cnt / 1000), ring_buffer_get_space_byte_count(&internal_handle->stream_out_pcm_buff), one_frame_bytes);
#endif
#if defined(MTK_AVM_DIRECT)
        mp3_audio_mutex_unlock(g_xSemaphore_Audio);
#endif
        return;
    }


    int32_t share_buffer_data_amount = mp3_codec_get_share_buffer_data_count(handle);

    if ((1 == handle->flush_data_flag) && (share_buffer_data_amount <= MPEG_AUDIO_FRAME_HEADER_LENGTH)) {
        uint8_t *out_buf_ptr    = NULL;
        uint32_t stream_out_pcm_buffer_data_count = 0;
        ring_buffer_get_read_information(&internal_handle->stream_out_pcm_buff, &out_buf_ptr, &stream_out_pcm_buffer_data_count);
        //LOGI("[MP3 Codec]out_pcm count(0x%x)",stream_out_pcm_buffer_data_count);
        if (0 == stream_out_pcm_buffer_data_count) {
#if defined(MTK_AVM_DIRECT)
#if defined(MTK_AUDIO_MP3_DEBUG)
            MP3_LOG_I("[MP3 Codec]Send EOF cmd(%x)\r\n", 1, MSG_MCU2DSP_PROMPT_CONFIG);
#endif
            if (internal_handle->message_type == AUDIO_MESSAGE_TYPE_PROMPT) {
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_PROMPT_CONFIG, AUDIO_PLAYBACK_CONFIG_EOF, 1, false);
            } else {
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_PLAYBACK_CONFIG, AUDIO_PLAYBACK_CONFIG_EOF, 1, false);
            }
#elif defined(MTK_AUDIO_MIXER_SUPPORT)
            MP3_LOG_I("[MP3 Codec] hisr(%d): role%d: audio_mixer_set_eof()\r\n", 2, (curr_cnt / 1000), handle->mixer_track_role);
            audio_mixer_set_eof(handle->mixer_track_id);
#else
            MP3_LOG_I("[MP3 Codec] hisr(%d): audio_pcm_set_eof()\r\n", 1, (curr_cnt / 1000));
            audio_pcm_set_eof();
#endif
        }
    }

    if ((1 == handle->waiting) // wait data from caller
        && (ring_buffer_get_data_byte_count(&internal_handle->stream_out_pcm_buff) > (10240)) // PCM data is enough (> 10KB)
        && (mp3_codec_get_share_buffer_data_count(handle) < (handle->share_buff.buffer_size / 2))) { // Share buffer is not enough (< 1/2 size)
#if defined(MTK_AUDIO_MP3_DEBUG)
        MP3_LOG_I("[MP3 Codec] hisr(%d): wait data just return\r\n", 1, (curr_cnt / 1000));
#endif
#if defined(MTK_AVM_DIRECT)
        mp3_audio_mutex_unlock(g_xSemaphore_Audio);
#endif
        return;
    }

    //update read ptr to share buffer
    share_buff_read_ptr = handle->share_buff.buffer_base + handle->share_buff.read;

    int32_t mp3_frame_size = 0;
    int32_t check_min_share_size = 0;
    if ((handle->flush_data_flag == 1) || (handle->linear_buff == 1)) {
        check_min_share_size = MPEG_AUDIO_FRAME_HEADER_LENGTH;
    } else {
        if (share_buffer_data_amount >= MPEG_AUDIO_FRAME_HEADER_LENGTH) {
            mp3_codec_get_next_mp3_frame_size(share_buff_read_ptr,
                                              handle->share_buff.buffer_base,
                                              handle->share_buff.buffer_size,
                                              &mp3_frame_size);
        }
        mp3_frame_size = (mp3_frame_size > 0) ? mp3_frame_size : mp3_handle_ptr_to_previous_mp3_frame_size(data);
        mp3_handle_ptr_to_previous_mp3_frame_size(data) = mp3_frame_size;
        check_min_share_size = mp3_frame_size;
    }

    if (share_buffer_data_amount < (check_min_share_size + 1)) {
        mp3_codec_request_data_to_share_buffer(handle);
#if defined(MTK_AVM_DIRECT)
        mp3_audio_mutex_unlock(g_xSemaphore_Audio);
#endif
        return;
    }

    //taskENTER_CRITICAL();
    // calculate mcps
    //CPU_RESET_CYCLECOUNTER;
    //__asm volatile("nop");
    //count_test = *DWT_CYCCNT;
    //offset = count_test - 1;
    //CPU_RESET_CYCLECOUNTER;

#ifdef MTK_AVM_DIRECT
    ring_buffer_get_write_information(&internal_handle->stream_out_pcm_buff, &(internal_handle->decode_pcm_buff.buffer_base_pointer), &(internal_handle->decode_pcm_buff.buffer_byte_count));
#endif

    uint8_t one_mp3_silence_frame[MPEG_AUDIO_FRAME_HEADER_LENGTH + MP3_SILENCE_FRAME_PATTERN_LENGTH] = {0};
    uint32_t i = 0;
    for (i = 0; i < MPEG_AUDIO_FRAME_HEADER_LENGTH + MP3_SILENCE_FRAME_PATTERN_LENGTH; i++) {
        if ((handle->share_buff.read + i) >= handle->share_buff.buffer_size) {
            one_mp3_silence_frame[i] = *(share_buff_read_ptr + i - handle->share_buff.buffer_size);
        } else {
            one_mp3_silence_frame[i] = *(share_buff_read_ptr + i);
        }
    }
    one_mp3_silence_frame[MPEG_AUDIO_FRAME_HEADER_LENGTH + MP3_SILENCE_FRAME_PATTERN_LENGTH - 1] = 0;
    if (0 == strncmp((const char *)(one_mp3_silence_frame + MPEG_AUDIO_FRAME_HEADER_LENGTH), mp3_silence_frame_pattern, sizeof(mp3_silence_frame_pattern) / sizeof(char))) {
        uint32_t decoded_data_count = internal_handle->mp3_handle->PCMSamplesPerCH * internal_handle->mp3_handle->CHNumber * 2;
        memset(internal_handle->decode_pcm_buff.buffer_base_pointer, 0, decoded_data_count);
        mp3_codec_get_silence_frame_information(handle, &consumeBS);
    } else {
        /*   Deocde mp3 frame
            *   return: The consumed data size of Bitsream buffer for this  frame
          */
        consumeBS = MP3Dec_Decode(internal_handle->mp3_handle,
                                  internal_handle->decode_pcm_buff.buffer_base_pointer,
                                  handle->share_buff.buffer_base,
                                  handle->share_buff.buffer_size,
                                  share_buff_read_ptr);
    }

    //count_test = *DWT_CYCCNT - offset;
    //LOGI("%d\n", count_test);
    //taskEXIT_CRITICAL();

    int32_t share_data_amount = mp3_codec_get_share_buffer_data_count(handle);
    //printf("hisr:consumeBS=%d, buffer_size=%d, buff.read=%d, buff.write=%d, share_data_amount=%d, pcm_data_amount=%d, one_frame_bytes=%d\r\n", consumeBS, handle->share_buff.buffer_size, handle->share_buff.read, handle->share_buff.write, share_data_amount, ring_buffer_get_data_byte_count(&internal_handle->stream_out_pcm_buff), one_frame_bytes);


    bool decode_abnormal = false;

    if ((consumeBS != mp3_frame_size) || (consumeBS < MINIMUM_MP3_FRAME_SIZE) || (consumeBS > MAXIMUM_MP3_FRAME_SIZE)) {
#if defined(MTK_AUDIO_MIXER_SUPPORT)
#if 0
        MP3_LOG_I("hisr(%d): role%d: mp3 frame[ 0] = 0x%02X %02X %02X %02X\r\n", 6,
                  (curr_cnt / 1000), handle->mixer_track_role,
                  one_mp3_silence_frame[0], one_mp3_silence_frame[1], one_mp3_silence_frame[2], one_mp3_silence_frame[3]);
        MP3_LOG_I("hisr(%d): role%d: mp3 frame[ 5] = 0x%02X %02X %02X %02X\r\n", 6,
                  (curr_cnt / 1000), handle->mixer_track_role,
                  one_mp3_silence_frame[4], one_mp3_silence_frame[5], one_mp3_silence_frame[6], one_mp3_silence_frame[7]);
        MP3_LOG_I("hisr(%d): role%d: mp3 frame[ 8] = 0x%02X %02X %02X %02X\r\n", 6,
                  (curr_cnt / 1000), handle->mixer_track_role,
                  one_mp3_silence_frame[8], one_mp3_silence_frame[9], one_mp3_silence_frame[10], one_mp3_silence_frame[11]);
        MP3_LOG_I("hisr(%d): role%d: mp3 frame[12] = 0x%02X %02X %02X %02X\r\n", 6,
                  (curr_cnt / 1000), handle->mixer_track_role,
                  one_mp3_silence_frame[12], one_mp3_silence_frame[13], one_mp3_silence_frame[14], one_mp3_silence_frame[15]);
#endif
#if defined(MTK_AUDIO_MP3_DEBUG)
        MP3_LOG_I("[MP3 Codec] hisr(%d): role%d: consumeBS = %d, mp3_frame_size = %d, share_buff.read = %d, share_buff.write = %d, share_buff.buffer_size = %d\r\n", 7, (curr_cnt / 1000), handle->mixer_track_role, (int)consumeBS, mp3_frame_size, handle->share_buff.read, handle->share_buff.write, handle->share_buff.buffer_size);
        MP3_LOG_I("[MP3 Codec] hisr(%d): role%d: flush_data_flag = %d, linear_buff = %d\r\n", 4, (curr_cnt / 1000), handle->mixer_track_role, handle->flush_data_flag, handle->linear_buff);
#endif
#else
        MP3_LOG_I("hisr(%d): mp3 frame[ 0] = 0x%02X %02X %02X %02X\r\n", 5,
                  (curr_cnt / 1000),
                  one_mp3_silence_frame[0], one_mp3_silence_frame[1], one_mp3_silence_frame[2], one_mp3_silence_frame[3]);
        MP3_LOG_I("hisr(%d): mp3 frame[ 5] = 0x%02X %02X %02X %02X\r\n", 5,
                  (curr_cnt / 1000),
                  one_mp3_silence_frame[4], one_mp3_silence_frame[5], one_mp3_silence_frame[6], one_mp3_silence_frame[7]);
        MP3_LOG_I("hisr(%d): mp3 frame[ 8] = 0x%02X %02X %02X %02X\r\n", 5,
                  (curr_cnt / 1000),
                  one_mp3_silence_frame[8], one_mp3_silence_frame[9], one_mp3_silence_frame[10], one_mp3_silence_frame[11]);
        MP3_LOG_I("hisr(%d): mp3 frame[12] = 0x%02X %02X %02X %02X\r\n", 5,
                  (curr_cnt / 1000),
                  one_mp3_silence_frame[12], one_mp3_silence_frame[13], one_mp3_silence_frame[14], one_mp3_silence_frame[15]);
        MP3_LOG_I("[MP3 Codec] hisr(%d): consumeBS = %d, mp3_frame_size = %d, share_buff.read = %d, share_buff.write = %d, share_buff.buffer_size = %d\r\n", 6, (curr_cnt / 1000), (int)consumeBS, mp3_frame_size, handle->share_buff.read, handle->share_buff.write, handle->share_buff.buffer_size);
        MP3_LOG_I("[MP3 Codec] hisr(%d): flush_data_flag = %d, linear_buff = %d\r\n", 3, (curr_cnt / 1000), handle->flush_data_flag, handle->linear_buff);
#endif // #if defined(MTK_AUDIO_MIXER_SUPPORT)
    }

    if ((consumeBS != mp3_frame_size) && (handle->flush_data_flag != 1) && (handle->linear_buff == 0)) {
        MP3_LOG_I("[MP3 Codec] consumeBS != mp3_frame_size: consumeBS = %d, mp3_frame_size = %d\n", 2, (int)consumeBS, mp3_frame_size);
    }

    if (consumeBS < 0) {
        MP3_LOG_I("[MP3 Codec] hisr(%d): Invalid return , consumeBS= %d\n", 2, (curr_cnt / 1000), (int)consumeBS);
        //todo, error frame handle
    } else if (consumeBS >= 0 && consumeBS <= share_data_amount) {
        //update read index to share buffer
        handle->share_buff.read += consumeBS;
        if (handle->share_buff.read >= handle->share_buff.buffer_size) {    // although share buffer is a ring buffer, the mp3 decoder ip only code to the end of buffer at most.
            if (handle->linear_buff == 1) {
                handle->share_buff.read = handle->share_buff.buffer_size;
            } else {
                handle->share_buff.read -= handle->share_buff.buffer_size;
            }
        }

        if (consumeBS == 1) { // Actually, it means the end of mp3 audio data!
#if defined(MTK_AUDIO_MIXER_SUPPORT)
            MP3_LOG_I("[MP3 Codec] hisr(%d): role%d: decode abnormal. consumeBS =1, handle->share_buff.read = %d, handle->share_buff.write = %d, handle->share_buff.buffer_size = %d\r\n", 5, (curr_cnt / 1000), handle->mixer_track_role, handle->share_buff.read, handle->share_buff.write, handle->share_buff.buffer_size);
#else
            MP3_LOG_I("[MP3 Codec] hisr(%d): decode abnormal. consumeBS =1, handle->share_buff.read = %d, handle->share_buff.write = %d, handle->share_buff.buffer_size = %d\r\n", 4, (curr_cnt / 1000), handle->share_buff.read, handle->share_buff.write, handle->share_buff.buffer_size);
#endif
        }

    } else if (consumeBS > share_data_amount) {
        consumeBS = share_data_amount;  // It strange here, in fact, mp3 SWIP consumBS must not > share_data_amount. we met consumBS > share_data_amount in playing combine two songs in one mp3 file, so just workaround here.
        handle->share_buff.read += consumeBS;
        if (handle->share_buff.read >= handle->share_buff.buffer_size) {    // although share buffer is a ring buffer, the mp3 decoder ip only code to the end of buffer at most.
            if (handle->linear_buff == 1) {
                handle->share_buff.read = handle->share_buff.buffer_size;
            } else {
                handle->share_buff.read -= handle->share_buff.buffer_size;
            }
        }

#if defined(MTK_AUDIO_MIXER_SUPPORT)
        MP3_LOG_I("[MP3 Codec] hisr(%d): role%d, decode abnormal. consumeBS > share_data_amount\r\n", 2, (curr_cnt / 1000), handle->mixer_track_role);
#else
        MP3_LOG_I("[MP3 Codec] hisr(%d): decode abnormal. consumeBS > share_data_amount\r\n", 1, (curr_cnt / 1000));
#endif
        decode_abnormal = true;
    } else {
        MP3_LOG_I("[MP3 Codec] hisr(%d): never hear\n", 1, (curr_cnt / 1000));
    }
#ifdef MTK_AVM_DIRECT
    if (decode_abnormal == false) {
        if ((internal_handle->mp3_handle->CHNumber != -1) && (internal_handle->mp3_handle->sampleRateIndex != -1)) {
            uint32_t decoded_data_count = internal_handle->mp3_handle->PCMSamplesPerCH * internal_handle->mp3_handle->CHNumber * 2;
            ring_buffer_write_done(&internal_handle->stream_out_pcm_buff, decoded_data_count);
        }
    }
#else
    if (decode_abnormal == false) {
        if ((internal_handle->mp3_handle->CHNumber != -1) && (internal_handle->mp3_handle->sampleRateIndex != -1)) {
            uint32_t loop_idx = 0;
            uint32_t decoded_data_count = internal_handle->mp3_handle->PCMSamplesPerCH * internal_handle->mp3_handle->CHNumber * 2;
            uint32_t decoded_data_index = 0;
            for (loop_idx = 0; loop_idx < 2; loop_idx++) {
                uint8_t *out_buf_ptr    = NULL;
                uint32_t out_byte_count = 0;
                ring_buffer_get_write_information(&internal_handle->stream_out_pcm_buff, &out_buf_ptr, &out_byte_count);
                if (out_byte_count > 0) {
                    uint32_t consumed_byte_count = MINIMUM(decoded_data_count, out_byte_count);
                    uint8_t *p_in_base         = internal_handle->decode_pcm_buff.buffer_base_pointer;
                    uint8_t *p_in_buf          = p_in_base + decoded_data_index;
                    memcpy(out_buf_ptr, p_in_buf, consumed_byte_count);
                    decoded_data_index += consumed_byte_count;
                    decoded_data_count  -= consumed_byte_count;
                    ring_buffer_write_done(&internal_handle->stream_out_pcm_buff, consumed_byte_count);
                } else {
                    // stream_out_pcm_buffer full, do nothing.
                    MP3_LOG_D("[MP3 Codec] hisr(%d): stream_out_pcm_buffer full remain decoded_data_count=%d, stream out pcm data=%d, share buffer data=%d\r\n", 4, (curr_cnt / 1000), decoded_data_count, ring_buffer_get_data_byte_count(&internal_handle->stream_out_pcm_buff), mp3_codec_get_share_buffer_data_count(handle));
                    break;
                }

            }
        }
    }
#endif

    if (mp3_codec_get_share_buffer_data_count(handle) <= (handle->share_buff.buffer_size / 2)) {    // share buufer less than decode bs * 3, we request user to fill data
        mp3_codec_request_data_to_share_buffer(handle);
    }

    if (ring_buffer_get_space_byte_count(&internal_handle->stream_out_pcm_buff) >= one_frame_bytes) {
        mp3_codec_event_send_from_isr(MP3_CODEC_QUEUE_EVENT_DECODE, data);
    }

#if defined(MTK_AVM_DIRECT)
    mp3_audio_mutex_unlock(g_xSemaphore_Audio);
#endif
}




static mp3_codec_function_return_state_t mp3_codec_skip_id3v2_and_reach_next_frame(mp3_codec_media_handle_t *handle, uint32_t file_size)
{
    uint32_t auido_frame_header = 0;
    if (mp3_codec_get_share_buffer_data_count(handle) < ID3V2_HEADER_LENGTH) {
        return MP3_CODEC_RETURN_ERROR;
    }

    mp3_codec_get_id3v2_info_and_skip(handle, file_size);

    if (mp3_codec_reach_next_frame_and_get_audio_frame_header(handle, &auido_frame_header, 2048, 0) != MP3_CODEC_RETURN_OK) {
        return MP3_CODEC_RETURN_ERROR;
    }

    return MP3_CODEC_RETURN_OK;
}

#if defined(MTK_AVM_DIRECT)
#if 0
static uint32_t sampling_rate_enum_to_value(hal_audio_sampling_rate_t hal_audio_sampling_rate_enum)
{
    switch (hal_audio_sampling_rate_enum) {
        case HAL_AUDIO_SAMPLING_RATE_8KHZ:
            return   8000;
        case HAL_AUDIO_SAMPLING_RATE_11_025KHZ:
            return  11025;
        case HAL_AUDIO_SAMPLING_RATE_12KHZ:
            return  12000;
        case HAL_AUDIO_SAMPLING_RATE_16KHZ:
            return  16000;
        case HAL_AUDIO_SAMPLING_RATE_22_05KHZ:
            return  22050;
        case HAL_AUDIO_SAMPLING_RATE_24KHZ:
            return  24000;
        case HAL_AUDIO_SAMPLING_RATE_32KHZ:
            return  32000;
        case HAL_AUDIO_SAMPLING_RATE_44_1KHZ:
            return  44100;
        case HAL_AUDIO_SAMPLING_RATE_48KHZ:
            return  48000;
        case HAL_AUDIO_SAMPLING_RATE_88_2KHZ:
            return  88200;
        case HAL_AUDIO_SAMPLING_RATE_96KHZ:
            return  96000;
        case HAL_AUDIO_SAMPLING_RATE_176_4KHZ:
            return 176400;
        case HAL_AUDIO_SAMPLING_RATE_192KHZ:
            return 192000;

        default:
            return 8000;
    }
}
#endif

static void mp3_codec_play_avm(mp3_codec_media_handle_t *handle)
{
    mp3_codec_internal_handle_t *internal_handle = mp3_handle_ptr_to_internal_ptr(handle);
    mcu2dsp_audio_msg_t open_msg = MSG_MCU2DSP_PLAYBACK_OPEN;
    mcu2dsp_audio_msg_t start_msg = MSG_MCU2DSP_PLAYBACK_START;
    audio_message_type_t msg_type = AUDIO_MESSAGE_TYPE_PLAYBACK;
    void *p_param_share;
    //uint8_t *out_buf_ptr    = NULL;
    //uint32_t sample_count = 0;
    //uint32_t stream_out_amount = 0;
    //uint32_t stream_out_pcm_buffer_data_count = 0;
    //n9_dsp_share_info_t *p_info;

#if defined(MTK_AUDIO_MIXER_SUPPORT)
    if (handle->mixer_track_role == AUDIO_MIXER_TRACK_ROLE_SIDE) {
        open_msg = MSG_MCU2DSP_PROMPT_OPEN;
        start_msg = MSG_MCU2DSP_PROMPT_START;
        msg_type = AUDIO_MESSAGE_TYPE_PROMPT;
    }
#endif
    internal_handle->message_type = msg_type;

    // Collect parameters
    mcu2dsp_open_param_t open_param;

    memset(&open_param, 0, sizeof(open_param));

    open_param.param.stream_in  = STREAM_IN_VP;
    open_param.param.stream_out = STREAM_OUT_AFE;
    open_param.audio_scenario_type = AUDIO_SCENARIO_TYPE_VP;

    open_param.stream_in_param.playback.bit_type = HAL_AUDIO_BITS_PER_SAMPLING_16;
    open_param.stream_in_param.playback.sampling_rate = internal_handle->sampling_rate;
    open_param.stream_in_param.playback.channel_number = internal_handle->channel_number;
    open_param.stream_in_param.playback.codec_type = 0;  //KH: should use AUDIO_DSP_CODEC_TYPE_PCM
    open_param.stream_in_param.playback.p_share_info = (n9_dsp_share_info_t *)hal_audio_query_share_info(msg_type);

    MP3_LOG_I("[MP3]mp3_codec_play_avm SR(%d) ch(%d)\r\n", 2, internal_handle->sampling_rate, internal_handle->channel_number);

    //hal_audio_reset_share_info( open_param.stream_in_param.playback.p_share_info );

    //open_param.stream_in_param.playback.p_share_info->bBufferIsFull = 0;
#if 0
    open_param.stream_out_param.afe.audio_device    = HAL_AUDIO_DEVICE_DAC_DUAL; //HAL_AUDIO_DEVICE_HEADSET;
    open_param.stream_out_param.afe.stream_channel  = HAL_AUDIO_DIRECT;
    if (open_param.stream_out_param.afe.audio_device == HAL_AUDIO_DEVICE_I2S_MASTER) {
        open_param.stream_out_param.afe.misc_parms      = I2S_CLK_SOURCE_DCXO;
    } else {
        open_param.stream_out_param.afe.misc_parms      = DOWNLINK_PERFORMANCE_NORMAL;
    }
#else
    hal_audio_get_stream_out_setting_config(AU_DSP_AUDIO, &open_param.stream_out_param);
#endif
    if (internal_handle->message_type == AUDIO_MESSAGE_TYPE_PROMPT) {
        open_param.stream_out_param.afe.memory          = HAL_AUDIO_MEM2;
    } else {
        open_param.stream_out_param.afe.memory          = HAL_AUDIO_MEM1;
    }
    open_param.stream_out_param.afe.format          = AFE_PCM_FORMAT_S16_LE;
    open_param.stream_out_param.afe.stream_out_sampling_rate   = hal_audio_sampling_rate_enum_to_value(internal_handle->sampling_rate);

    if (aud_fixrate_get_downlink_rate(open_param->audio_scenario_type) == FIXRATE_NONE) {
        open_param.stream_out_param.afe.sampling_rate   = hal_audio_sampling_rate_enum_to_value(internal_handle->sampling_rate);
    } else {
        open_param.stream_out_param.afe.sampling_rate = aud_fixrate_get_downlink_rate(open_param->audio_scenario_type);
    }

#if defined (MTK_FIXED_VP_A2DP_SAMPLING_RATE_TO_48KHZ)
    open_param.stream_out_param.afe.sampling_rate    = 48000;
#endif
    open_param.stream_out_param.afe.irq_period      = 5;
    open_param.stream_out_param.afe.frame_size      = 768;
    open_param.stream_out_param.afe.frame_number    = 4;
    open_param.stream_out_param.afe.hw_gain         = true;

#if defined(HAL_AUDIO_SUPPORT_APLL)
    //Open i2s master out low jitter mode
    hal_audio_i2s_master_out_low_jitter_mode(LOW_JITTER_MODE_MP3_CODEC_PLAY_AVM_OUT, open_param.stream_out_param.afe.audio_device, open_param.stream_out_param.afe.sampling_rate, open_param.stream_out_param.afe.is_low_jitter, true);
#endif

#ifdef AIR_FIXED_SUB_DL_HIGH_RES_ENABLE
    hal_audio_dsp_dl_clkmux_control(AUDIO_MESSAGE_TYPE_PROMPT, open_param.stream_out_param.afe.audio_device, 96000, true);
#else
    hal_audio_dsp_dl_clkmux_control(AUDIO_MESSAGE_TYPE_PROMPT, open_param.stream_out_param.afe.audio_device, open_param.stream_out_param.afe.sampling_rate, true);
#endif

#if defined(MTK_EXTERNAL_DSP_NEED_SUPPORT)
    ami_set_afe_param(STREAM_OUT_2, open_param.stream_out_param.afe.sampling_rate, true);
#endif
    p_param_share = hal_audio_dsp_controller_put_paramter(&open_param, sizeof(mcu2dsp_open_param_t), msg_type);
    // Notify to do dynamic download. Use async wait.
    hal_audio_dsp_controller_send_message(open_msg, AUDIO_DSP_CODEC_TYPE_PCM, (uint32_t)p_param_share, true);

    // Register callback
    hal_audio_service_hook_callback(msg_type, mp3_codec_pcm_out_isr_callback, handle);

    //Pre-transfer audio data after decode to shared buffer.
    // p_info = (n9_dsp_share_info_t *)hal_audio_query_share_info(internal_handle->message_type);
    // sample_count = hal_audio_buf_mgm_get_free_byte_count(p_info);
    // ring_buffer_get_read_information(&internal_handle->stream_out_pcm_buff, &out_buf_ptr, &stream_out_pcm_buffer_data_count);
    // if (stream_out_pcm_buffer_data_count > 0) {
    //     stream_out_amount = MINIMUM(sample_count, stream_out_pcm_buffer_data_count);
    // }
    // hal_audio_write_stream_out_by_type(internal_handle->message_type, out_buf_ptr, stream_out_amount);
    // ring_buffer_read_done(&internal_handle->stream_out_pcm_buff, stream_out_amount);
    // Start playback
    mcu2dsp_start_param_t start_param;

    // Collect parameters
    start_param.param.stream_in     = STREAM_IN_VP;
    start_param.param.stream_out    = STREAM_OUT_AFE;

    start_param.stream_out_param.afe.aws_flag         = false;

    start_param.stream_out_param.afe.aws_sync_request = handle->aws_sync_request;
    start_param.stream_out_param.afe.aws_sync_time    = handle->aws_sync_time;

    p_param_share = hal_audio_dsp_controller_put_paramter(&start_param, sizeof(mcu2dsp_start_param_t), msg_type);
    hal_audio_dsp_controller_send_message(start_msg, 0, (uint32_t)p_param_share, true);
}
#endif

static mp3_codec_function_return_state_t mp3_codec_play_internal(mp3_codec_media_handle_t *handle)
{
    int32_t consumeBS = -1;
    uint8_t *share_buff_read_ptr = NULL;
    int16_t max_loop_times = 20;    // arbitrarily selected
    uint32_t stream_out_pcm_buffer_data_count = 0, sample_count = 0, stream_out_amount = 0;
    n9_dsp_share_info_t *p_info = NULL;
    uint8_t *out_buf_ptr    = NULL;
    p_info = (n9_dsp_share_info_t *)hal_audio_query_share_info(AUDIO_MESSAGE_TYPE_PROMPT);
    hal_audio_reset_share_info(p_info);
    p_info->bBufferIsFull = 0;
#ifndef MTK_AUDIO_MIXER_SUPPORT
    uint32_t memory_size = 0;
#endif
#if defined(MTK_AUDIO_MP3_DEBUG)
    MP3_LOG_I("[MP3 Codec] mp3_codec_play_internal ++\n", 0);
#endif
    /* temp sol. to protect play/resume flow  */
#if defined(MTK_AUDIO_MIXER_SUPPORT) && !defined(MTK_AVM_DIRECT)
    if (handle->mixer_track_role == AUDIO_MIXER_TRACK_ROLE_MAIN) {
        prompt_control_stop_tone();
    }
#endif

    mp3_codec_internal_handle_t *internal_handle = mp3_handle_ptr_to_internal_ptr(handle);

#ifndef AIR_MP3_TASK_DEDICATE_ENABLE
    mp3_codec_event_register_callback(handle, MP3_CODEC_QUEUE_EVENT_DECODE, mp3_codec_deocde_hisr_handler);
#endif

    //lock sleep
    //audio_service_setflag(handle->audio_id);
    do {
        //update read ptr to share buffer
        share_buff_read_ptr = handle->share_buff.buffer_base + handle->share_buff.read;

#ifdef MTK_AVM_DIRECT
        ring_buffer_get_write_information(&internal_handle->stream_out_pcm_buff, &(internal_handle->decode_pcm_buff.buffer_base_pointer), &(internal_handle->decode_pcm_buff.buffer_byte_count));
#endif
        consumeBS = MP3Dec_Decode(internal_handle->mp3_handle,
                                  internal_handle->decode_pcm_buff.buffer_base_pointer,
                                  handle->share_buff.buffer_base,
                                  handle->share_buff.buffer_size,
                                  share_buff_read_ptr);

        int32_t share_data_amount = mp3_codec_get_share_buffer_data_count(handle);
        if (consumeBS < 0) {
            MP3_LOG_I("[MP3 Codec] Invalid return , consumeBS= %d\n", 1, (int)consumeBS);
        } else if ((consumeBS > 1) && (consumeBS < share_data_amount) && (MP3SilenceFrameSize <= 0)) {
            MP3SilenceFrameSize = consumeBS;
            MP3SilenceFrameHeader[0] = share_buff_read_ptr[0];
            MP3SilenceFrameHeader[1] = share_buff_read_ptr[1];
            MP3SilenceFrameHeader[2] = share_buff_read_ptr[2];
            MP3SilenceFrameHeader[3] = share_buff_read_ptr[3];
#if defined(MTK_AUDIO_MP3_DEBUG)
            MP3_LOG_I("[MP3 Codec] MP3SilenceFrameSize = %d, MP3SilenceFrameHeader = 0x%02X %02X %02X %02X \n", 5, MP3SilenceFrameSize,
                      MP3SilenceFrameHeader[0], MP3SilenceFrameHeader[1], MP3SilenceFrameHeader[2], MP3SilenceFrameHeader[3]);
#endif
        }
        MP3_LOG_I("[MP3 Codec] play internal consumeBS= %d\n", 1, (int)consumeBS);
        // TODO: maybe we can break do while when meet consumeBS=1 or consumeBS > share buffer data

        if ((internal_handle->mp3_handle->CHNumber != -1) && (internal_handle->mp3_handle->sampleRateIndex != -1)) {
            {
                //update read index to share buffer
                handle->share_buff.read += consumeBS;
                if (handle->share_buff.read >= handle->share_buff.buffer_size) {
                    handle->share_buff.read -= handle->share_buff.buffer_size;
                }
            }


#ifdef MTK_AVM_DIRECT
            uint32_t decoded_data_count = internal_handle->mp3_handle->PCMSamplesPerCH * internal_handle->mp3_handle->CHNumber * 2;
            if (decoded_data_count <= internal_handle->decode_pcm_buff.buffer_byte_count) {
                ring_buffer_write_done(&internal_handle->stream_out_pcm_buff, decoded_data_count);
            } else {
                MP3_LOG_I("MP3 codec memory size error!!!", 0);
                AUDIO_ASSERT(0);
            }
#else
            uint32_t loop_idx = 0;
            uint32_t decoded_data_count = internal_handle->mp3_handle->PCMSamplesPerCH * internal_handle->mp3_handle->CHNumber * 2;
            uint32_t decoded_data_index = 0;

            for (loop_idx = 0; loop_idx < 2; loop_idx++) {
                uint8_t *out_buf_ptr    = NULL;
                uint32_t out_byte_count = 0;
                ring_buffer_get_write_information(&internal_handle->stream_out_pcm_buff, &out_buf_ptr, &out_byte_count);
                if (out_byte_count > 0) {
                    uint32_t consumed_byte_count = MINIMUM(decoded_data_count, out_byte_count);
                    uint8_t *p_in_base         = internal_handle->decode_pcm_buff.buffer_base_pointer;
                    uint8_t *p_in_buf          = p_in_base + decoded_data_index;
                    memcpy(out_buf_ptr, p_in_buf, consumed_byte_count);
                    decoded_data_index += consumed_byte_count;
                    decoded_data_count  -= consumed_byte_count;
                    ring_buffer_write_done(&internal_handle->stream_out_pcm_buff, consumed_byte_count);
                } else {
                    // stream_out_pcm_buffer full, do nothing.
                    break;
                }

            }
#endif
        }

        //Pre-transfer audio data after decode to shared buffer.
        sample_count = hal_audio_buf_mgm_get_free_byte_count(p_info);
        ring_buffer_get_read_information(&internal_handle->stream_out_pcm_buff, &out_buf_ptr, &stream_out_pcm_buffer_data_count);
        if (stream_out_pcm_buffer_data_count > 0) {
            stream_out_amount = MINIMUM(sample_count, stream_out_pcm_buffer_data_count);
        }
        hal_audio_write_stream_out_by_type(AUDIO_MESSAGE_TYPE_PROMPT, out_buf_ptr, stream_out_amount);
        ring_buffer_read_done(&internal_handle->stream_out_pcm_buff, stream_out_amount);
        if (consumeBS == 1) { // the last MP3 frame, EOF
            break;
        }
        max_loop_times--;
        if (max_loop_times < 0) {
            MP3_LOG_I("[MP3 Codec] mp3_codec_play_internal: decode mp3 data, but fail, please check the file\n", 0);
            return MP3_CODEC_RETURN_ERROR;
        }
#ifdef MTK_AVM_DIRECT
    } while (ring_buffer_get_space_byte_count(&internal_handle->stream_out_pcm_buff) >= internal_handle->mp3_handle->PCMSamplesPerCH * internal_handle->mp3_handle->CHNumber * 2);   // Decode until the buffer is not enough for one frame
#else
    }
    while (ring_buffer_get_data_byte_count(&internal_handle->stream_out_pcm_buff) <= 1152 * 2 * 3);   // one frame max samples * stereo * 3(for buffer three frames)
#endif

    MP3_LOG_I("[MP3 Codec] play internal, share buffer data=%d, stream out pcm data=%d\r\n", 2, mp3_codec_get_share_buffer_data_count(handle), ring_buffer_get_data_byte_count(&internal_handle->stream_out_pcm_buff));
    mp3_codec_request_data_to_share_buffer(handle); // since we decoded some data, thus we request again to fill share buffer.

#if defined(MTK_AVM_DIRECT)
    internal_handle->sampling_rate = mp3_codec_translate_decoder_ip_sample_rate_index((uint16_t)internal_handle->mp3_handle->sampleRateIndex);
    internal_handle->channel_number = mp3_codec_translate_decoder_ip_channel_number(internal_handle->mp3_handle->CHNumber);
#ifdef AIR_MP3_TASK_DEDICATE_ENABLE
    handle->flush_data_flag = 0;
    handle->state = MP3_CODEC_STATE_PLAY;
#endif
    mp3_codec_play_avm(handle);

#elif defined(MTK_AUDIO_MIXER_SUPPORT)
    handle->mixer_track_id = audio_mixer_get_track_id(mp3_codec_translate_decoder_ip_sample_rate_index((uint16_t)internal_handle->mp3_handle->sampleRateIndex),
                                                      mp3_codec_translate_decoder_ip_channel_number(internal_handle->mp3_handle->CHNumber),
                                                      mp3_codec_pcm_out_isr_callback,
                                                      handle,
                                                      handle->mixer_track_role);

    // Check ID
    if (handle->mixer_track_id < 0)
    {
        MP3_LOG_I("[MP3 Codec] mp3_codec_play_internal: invalid track ID %d !!! \n", 1, handle->mixer_track_id);
        return MP3_CODEC_RETURN_ERROR;
    }

    audio_mixer_query_free_count(handle->mixer_track_id, &mp3_handle_ptr_to_total_sample_count(handle));
    audio_mixer_status_t audio_mixer_status;
    mp3_handle_ptr_to_pcm_isr_cnt(handle) = 0;
    mp3_handle_ptr_to_is_bitstream_end_called(handle) = 0;
    audio_mixer_status = audio_mixer_start_track(handle->mixer_track_id);
    MP3_LOG_I("[MP3 Codec] start mixer: %d, handle(%X) role%d\n", 3, audio_mixer_status, handle, handle->mixer_track_role);
    if (AUDIO_MIXER_STATUS_OK != audio_mixer_status)
    {
        return MP3_CODEC_RETURN_ERROR;
    }
#else
    mp3_codec_event_send_from_isr(MP3_CODEC_QUEUE_EVENT_DECODE, handle);  // triger here, or if pcm buffer is too large, it will put all too dsp at first dsp interrupt, than it will has no pcm data at second dsp interrupt.
    hal_audio_set_stream_out_sampling_rate(mp3_codec_translate_decoder_ip_sample_rate_index((uint16_t)internal_handle->mp3_handle->sampleRateIndex));
    hal_audio_set_stream_out_channel_number(mp3_codec_translate_decoder_ip_channel_number(internal_handle->mp3_handle->CHNumber));

    hal_audio_register_stream_out_callback(mp3_codec_pcm_out_isr_callback, handle);
    hal_audio_get_memory_size(&memory_size);
    MP3_LOG_I("[MP3 Codec] pvPortMalloc(%d) +\n", 1, memory_size *sizeof(uint8_t));
    mp3_handle_ptr_to_audio_internal_ring_buffer(handle) = (uint8_t *)pvPortMalloc(memory_size *sizeof(uint8_t));  // 8 byte alignment
    MP3_LOG_I("[MP3 Codec] pvPortMalloc(%d) -, audio_internal_ring_buffer = 0x%X\n", 2, memory_size *sizeof(uint8_t), mp3_handle_ptr_to_audio_internal_ring_buffer(handle));
    if (NULL == mp3_handle_ptr_to_audio_internal_ring_buffer(handle))
    {
        MP3_LOG_I("[MP3 Codec] mp3_codec_play_internal: pvPortMalloc(%d) failed!!! \n", 1, memory_size * sizeof(uint8_t));
        return MP3_CODEC_RETURN_ERROR;
    }
    hal_audio_set_memory(mp3_handle_ptr_to_audio_internal_ring_buffer(handle));
    hal_audio_get_stream_out_sample_count(&mp3_handle_ptr_to_total_sample_count(handle));
    // We don't put data (using hal_audio_write_stream_out) first, since we want  stream_out_pcm_buff have maximum amount of data
    hal_audio_start_stream_out(HAL_AUDIO_PLAYBACK_MUSIC);
#endif
#ifndef AIR_MP3_TASK_DEDICATE_ENABLE
    handle->flush_data_flag = 0;
    handle->state = MP3_CODEC_STATE_PLAY;
#endif

#if defined(MTK_BT_AWS_ENABLE) && !defined(MTK_AVM_DIRECT)
    if (internal_handle->aws_flag)
    {
        MP3_LOG_I("[MP3 Codec] AWS is setted, hold playing until mp3_codec_aws_set_initial_sync\n", 0);
    }
#endif

#if defined(MTK_AUDIO_MP3_DEBUG)
    MP3_LOG_I("[MP3 Codec] mp3_codec_play_internal --\n", 0);
#endif
    return MP3_CODEC_RETURN_OK;
}


#if defined(MTK_BT_AWS_ENABLE) && !defined(MTK_AVM_DIRECT)

static void mp3_codec_aws_play_setting(mp3_codec_internal_handle_t *p_info)
{
    //p_info->accumulated_sample_count = 0;
    //p_info->aws_internal_flag = true;
    audio_service_aws_set_flag(true);
    //{   /* PCM route setting */
    //    uint16_t mask    = AUDIO_DSP_POST_PROCESSING_ENABLE_MASK | AUDIO_DSP_POST_PROCESSING_PCM_ROUTE_MASK;
    //    uint16_t control = AUDIO_DSP_POST_PROCESSING_ENABLE_ON   | AUDIO_DSP_POST_PROCESSING_PCM_ROUTE_ON;
    //    audio_service_set_post_process_control(mask, control);
    //    audio_service_hook_isr(DSP_D2M_PCM_ROUTE_INT, bt_codec_a2dp_aws_pcm_route_isr, (void *)p_info);
    //}
    return;
}

static void mp3_codec_aws_stop_setting(mp3_codec_internal_handle_t *p_info)
{
    //{   /* PCM route setting */
    //    uint16_t mask    = AUDIO_DSP_POST_PROCESSING_ENABLE_MASK | AUDIO_DSP_POST_PROCESSING_PCM_ROUTE_MASK;
    //    uint16_t control = AUDIO_DSP_POST_PROCESSING_ENABLE_OFF  | AUDIO_DSP_POST_PROCESSING_PCM_ROUTE_OFF;
    //    audio_service_unhook_isr(DSP_D2M_PCM_ROUTE_INT);
    //    audio_service_set_post_process_control(mask, control);
    //}
    audio_service_aws_set_flag(false);
    //p_info->aws_internal_flag = false;
    return;
}
#endif  /* defined(MTK_AUDIO_MP3_CODEC_AWS_SUPPORT) */




mp3_codec_function_return_state_t mp3_codec_aws_set_flag(mp3_codec_media_handle_t *handle, bool flag)
{
#ifdef MTK_BT_AWS_ENABLE
    mp3_codec_internal_handle_t *internal_handle = mp3_handle_ptr_to_internal_ptr(handle);
    internal_handle->aws_flag = flag;

    if (internal_handle->aws_flag) {
        MP3_LOG_I("[MP3 Codec] enable mp3 aws feature\n", 0);
        mp3_codec_aws_play_setting(internal_handle);
    } else {
        mp3_codec_aws_stop_setting(internal_handle);
        MP3_LOG_I("[MP3 Codec] stop mp3 aws feature\n", 0);
    }

    return MP3_CODEC_RETURN_OK;
#else
    MP3_LOG_I("[MP3 Codec]Not support mp3 codec aws\n", 0);
    return MP3_CODEC_RETURN_ERROR;
#endif
}

mp3_codec_function_return_state_t mp3_codec_aws_set_initial_sync(mp3_codec_media_handle_t *handle)

{
#if defined(MTK_BT_AWS_ENABLE) && !defined(MTK_AVM_DIRECT)
    mp3_codec_function_return_state_t result = MP3_CODEC_RETURN_ERROR;

    if (mp3_handle_ptr_to_internal_ptr(handle) != NULL) {
        mp3_codec_internal_handle_t *internal_handle = mp3_handle_ptr_to_internal_ptr(handle);

        if ((handle != NULL) && (internal_handle != NULL)) {
            if (internal_handle->aws_flag) {
                audio_service_aws_set_initial_sync();
                result = MP3_CODEC_RETURN_OK;
            }
        }
    }
    return result;
#else
    MP3_LOG_I("[MP3 Codec]Not support mp3 codec aws\n", 0);
    return MP3_CODEC_RETURN_ERROR;
#endif
}


static mp3_codec_function_return_state_t mp3_codec_resume(mp3_codec_media_handle_t *handle)
{
    MP3_LOG_I("[MP3 Codec] resume ++\r\n", 0);
    audio_codec_mutex_lock();
    mp3_codec_mutex_lock(handle);
    if (handle->state != MP3_CODEC_STATE_PAUSE) {
#if defined(MTK_AUDIO_MIXER_SUPPORT)
        MP3_LOG_I("[MP3 Codec] resume --: handle(%X) role%d, cannot resume because state(%d)\r\n", 3, handle, handle->mixer_track_role, handle->state);
#else
        MP3_LOG_I("[MP3 Codec] resume --: cannot resume because state(%d)\r\n", 1, handle->state);
#endif
        mp3_codec_mutex_unlock(handle);
        audio_codec_mutex_unlock();
        return MP3_CODEC_RETURN_ERROR;
    }

    MP3_LOG_I("[MP3 Codec] resume: stream out pcm data=%d, share buffer data=%d\r\n", 2, ring_buffer_get_data_byte_count(&(mp3_handle_ptr_to_internal_ptr(handle))->stream_out_pcm_buff), mp3_codec_get_share_buffer_data_count(handle));

#if defined(MTK_AVM_DIRECT)
    mp3_codec_play_avm(handle);
#elif defined(MTK_AUDIO_MIXER_SUPPORT)
    if (handle->mixer_track_role == AUDIO_MIXER_TRACK_ROLE_MAIN) {  /* temp sol. to protect play/resume flow  */
        prompt_control_stop_tone();
    }

    mp3_handle_ptr_to_pcm_isr_cnt(handle) = 0;

    audio_mixer_status_t audio_mixer_status;
    audio_mixer_status = audio_mixer_start_track(handle->mixer_track_id);
    if (AUDIO_MIXER_STATUS_OK != audio_mixer_status) {
        mp3_codec_mutex_unlock(handle);
        audio_codec_mutex_unlock();
        return MP3_CODEC_RETURN_ERROR;
    }
#else
    hal_audio_start_stream_out(HAL_AUDIO_PLAYBACK_MUSIC);
#endif
    MP3_LOG_I("[MP3 Codec] resume: stream out pcm data=%d, share buffer data=%d\r\n", 2, ring_buffer_get_data_byte_count(&(mp3_handle_ptr_to_internal_ptr(handle))->stream_out_pcm_buff), mp3_codec_get_share_buffer_data_count(handle));

    handle->flush_data_flag = 0;
    handle->state = MP3_CODEC_STATE_PLAY;
    mp3_codec_mutex_unlock(handle);
    audio_codec_mutex_unlock();
    MP3_LOG_I("[MP3 Codec] resume -- \r\n", 0);
    return MP3_CODEC_RETURN_OK;
}

#if defined(MTK_AVM_DIRECT)
static void mp3_codec_stop_avm(mp3_codec_media_handle_t *handle)
{
    mcu2dsp_audio_msg_t stop_msg = MSG_MCU2DSP_PLAYBACK_STOP;
    mcu2dsp_audio_msg_t close_msg = MSG_MCU2DSP_PLAYBACK_CLOSE;
    audio_message_type_t msg_type = AUDIO_MESSAGE_TYPE_PLAYBACK;
#if defined(MTK_AUDIO_MIXER_SUPPORT)
    if (handle->mixer_track_role == AUDIO_MIXER_TRACK_ROLE_SIDE) {
        stop_msg = MSG_MCU2DSP_PROMPT_STOP;
        close_msg = MSG_MCU2DSP_PROMPT_CLOSE;
        msg_type = AUDIO_MESSAGE_TYPE_PROMPT;
    }
#endif

    // Notify to stop
    hal_audio_dsp_controller_send_message(stop_msg, AUDIO_DSP_CODEC_TYPE_PCM, 0, true);

    // Unregister callback
#ifndef AIR_MP3_TASK_DEDICATE_ENABLE
    hal_audio_service_unhook_callback(msg_type);
#endif

    // Notify to release dynamic download
    hal_audio_dsp_controller_send_message(close_msg, AUDIO_DSP_CODEC_TYPE_PCM, 0, true);

#if defined(HAL_AUDIO_SUPPORT_APLL)
    //Close i2s master out low jitter mode
    hal_audio_i2s_master_out_low_jitter_mode(LOW_JITTER_MODE_MP3_CODEC_PLAY_AVM_OUT, 0, 0, 0, false);
#endif

    hal_audio_dsp_dl_clkmux_control(AUDIO_MESSAGE_TYPE_PROMPT, 0, 0, false);

    UNUSED(msg_type);
}
#endif

static mp3_codec_function_return_state_t mp3_codec_pause(mp3_codec_media_handle_t *handle)
{
    MP3_LOG_I("[MP3 Codec] pause ++\r\n", 0);
    mp3_codec_mutex_lock(handle);
    if (handle->state != MP3_CODEC_STATE_PLAY) {
#if defined(MTK_AUDIO_MIXER_SUPPORT)
        MP3_LOG_I("[MP3 Codec] pause --: handle(%X) role%d, cannot pause because state(%d)\r\n", 3, handle, handle->mixer_track_role, handle->state);
#else
        MP3_LOG_I("[MP3 Codec] pause --: cannot pause because state(%d)\r\n", 1, handle->state);
#endif
        mp3_codec_mutex_unlock(handle);
        return MP3_CODEC_RETURN_ERROR;
    }

#if defined(MTK_AVM_DIRECT)
    mp3_codec_stop_avm(handle);
#elif defined(MTK_AUDIO_MIXER_SUPPORT)
    audio_mixer_stop_track(handle->mixer_track_id);
#else
    hal_audio_stop_stream_out();
#endif

    handle->state = MP3_CODEC_STATE_PAUSE;
    mp3_codec_mutex_unlock(handle);
    MP3_LOG_I("[MP3 Codec] pause --\r\n", 0, ring_buffer_get_data_byte_count(&(mp3_handle_ptr_to_internal_ptr(handle))->stream_out_pcm_buff), mp3_codec_get_share_buffer_data_count(handle));
    return MP3_CODEC_RETURN_OK;
}

static mp3_codec_function_return_state_t mp3_codec_play(mp3_codec_media_handle_t *handle)
{
#if defined(MTK_AUDIO_MP3_DEBUG)
    MP3_LOG_I("[MP3 Codec] mp3_codec_play ++\r\n", 0);
#endif
    audio_codec_mutex_lock(); /* temp sol. to protect play/resume flow  */
    mp3_codec_mutex_lock(handle);
    if (handle->state != MP3_CODEC_STATE_READY && handle->state != MP3_CODEC_STATE_STOP) {
        mp3_codec_mutex_unlock(handle);
        audio_codec_mutex_unlock();
        return MP3_CODEC_RETURN_ERROR;
    }

    mp3_codec_internal_handle_t *internal_handle = mp3_handle_ptr_to_internal_ptr(handle);

#ifdef MTK_AVM_DIRECT
    int channel_number = 0;
    channel_number = MP3Dec_GetChannelNumber(handle->share_buff.buffer_base, handle->share_buff.buffer_size);

    if (channel_number == 1) {
        internal_handle->mp3_handle = MP3Dec_Init_Mono(internal_handle->working_buff1, internal_handle->working_buff2);  // must do this, or SW mp3 IP will work wrong
    } else {
        internal_handle->mp3_handle = MP3Dec_Init(internal_handle->working_buff1, internal_handle->working_buff2);  // must do this, or SW mp3 IP will work wrong
    }
#else
    internal_handle->mp3_handle = MP3Dec_Init(internal_handle->working_buff1, internal_handle->working_buff2);  // must do this, or SW mp3 IP will work wrong
#endif
    if (internal_handle->mp3_handle == NULL) {
        mp3_codec_mutex_unlock(handle);
        audio_codec_mutex_unlock(); /* temp sol. to protect play/resume flow  */
        return MP3_CODEC_RETURN_ERROR;
    }

    /*
    #ifdef MTK_BT_AWS_ENABLE
            if (internal_handle->aws_flag) {
                mp3_codec_aws_play_setting(internal_handle);
            }
    #endif
    */
    mp3_handle_ptr_to_internal_ptr(handle)->media_bitstream_end = 0;
    mp3_codec_function_return_state_t ret = mp3_codec_play_internal(handle);

    mp3_codec_mutex_unlock(handle);
    audio_codec_mutex_unlock(); /* temp sol. to protect play/resume flow  */
    // should placed outof audio codec mutex, or will dead lock with vp app layer
    if (!(handle->aws_sync_request)) {
        if (g_app_callback) {
            MP3_LOG_I("[VPC] non-aws VP start trigger\n", 0);
            g_app_callback(PROMPT_CONTROL_MEDIA_PLAY);
        }
    }
#if defined(MTK_AUDIO_MP3_DEBUG)
    MP3_LOG_I("[MP3 Codec] mp3_codec_play --\r\n", 0);
#endif
    return ret;
}


static mp3_codec_function_return_state_t mp3_codec_stop(mp3_codec_media_handle_t *handle)
{
#if defined(MTK_AUDIO_MIXER_SUPPORT)
#if defined(MTK_AUDIO_MP3_DEBUG)
    MP3_LOG_I("[MP3 Codec] stop ++: handle(%X) role%d\r\n", 2, handle, handle->mixer_track_role);
#endif
#else
    MP3_LOG_I("[MP3 Codec] stop ++\r\n", 0);
#endif
    mp3_codec_mutex_lock(handle);
    if (handle->state != MP3_CODEC_STATE_PLAY && handle->state != MP3_CODEC_STATE_PAUSE) {
#if defined(MTK_AUDIO_MIXER_SUPPORT)
        MP3_LOG_I("[MP3 Codec] stop --: handle(%X) role%d, cannot stop because state(%d)\r\n", 3, handle, handle->mixer_track_role, handle->state);
#else
        MP3_LOG_I("[MP3 Codec] stop --: cannot stop because state(%d)\r\n", 1, handle->state);
#endif
        mp3_codec_mutex_unlock(handle);
        return MP3_CODEC_RETURN_ERROR;
    }

#if defined(MTK_AVM_DIRECT)
    mp3_codec_stop_avm(handle);
#elif defined(MTK_AUDIO_MIXER_SUPPORT)
    audio_mixer_stop_track(handle->mixer_track_id);
    audio_mixer_free_track_id(handle->mixer_track_id);
#else
    hal_audio_stop_stream_out();
#endif
    mp3_handle_ptr_to_internal_ptr(handle)->media_bitstream_end = 0;

#ifndef AIR_MP3_TASK_DEDICATE_ENABLE
    mp3_codec_event_deregister_callback(handle, MP3_CODEC_QUEUE_EVENT_DECODE);
#endif
    //audio_service_clearflag(handle->audio_id);

    mp3_codec_reset_share_buffer(handle);
    mp3_codec_reset_stream_out_pcm_buffer(handle);    // if don't do this it will have residue data, and it will be played if you play again
#if defined(MTK_AUDIO_MIXER_SUPPORT)
#else
    vPortFree(mp3_handle_ptr_to_audio_internal_ring_buffer(handle));
    mp3_handle_ptr_to_audio_internal_ring_buffer(handle) = NULL;
#endif

    handle->state = MP3_CODEC_STATE_STOP;
    mp3_codec_mutex_unlock(handle);

#if defined(MTK_AUDIO_MIXER_SUPPORT)
#if defined(MTK_AUDIO_MP3_DEBUG)
    MP3_LOG_I("[MP3 Codec] stop --: handle(%X) role%d\r\n", 2, handle, handle->mixer_track_role);
#endif
#else
    MP3_LOG_I("[MP3 Codec] stop --\r\n", 0);
#endif
    return MP3_CODEC_RETURN_OK;
}


mp3_codec_function_return_state_t mp3_codec_close(mp3_codec_media_handle_t *handle)
{
#if defined(MTK_AUDIO_MIXER_SUPPORT)
    MP3_LOG_I("[MP3 Codec] close++ handle(%X), role%d\n", 2, handle, handle->mixer_track_role);
#else
    MP3_LOG_I("[MP3 Codec] close++\n", 0);
#endif
    mp3_codec_mutex_lock(handle);
    mp3_codec_internal_handle_t *internal_handle = mp3_handle_ptr_to_internal_ptr(handle);
#if defined(MTK_AUDIO_MP3_CODEC_AWS_SUPPORT)
    if (internal_handle->aws_flag) {
        mp3_codec_aws_stop_setting(internal_handle);    // to avoid app forget to stop aws
    }
#endif

    if (handle->state != MP3_CODEC_STATE_STOP && handle->state != MP3_CODEC_STATE_READY) {
        mp3_codec_mutex_unlock(handle);
#if defined(MTK_AUDIO_MIXER_SUPPORT)
        MP3_LOG_I("[MP3 Codec] close--: handle(%X) role%d, cannot close because state(%d)\r\n", 3, handle, handle->mixer_track_role, handle->state);
#else
        MP3_LOG_I("[MP3 Codec] close--: cannot close because state(%d)\r\n", 1, handle->state);
#endif
        return MP3_CODEC_RETURN_ERROR;
    }
    handle->state = MP3_CODEC_STATE_IDLE;

    if (mp3_handle_ptr_to_task_handle(handle) != NULL) {
        MP3_LOG_I("[MP3 Codec] delete mp3 task (0x%x)\n", 1, mp3_handle_ptr_to_task_handle(handle));
        vTaskDelete(mp3_handle_ptr_to_task_handle(handle));
        mp3_handle_ptr_to_task_handle(handle) = NULL;
        MP3_LOG_I("[MP3 Codec] delete mp3 task end.\n", 0);
    }

    if (mp3_handle_ptr_to_queue_handle(handle) != NULL) {
        MP3_LOG_I("[MP3 Codec] delete mp3 queue (0x%x)\n", 1, mp3_handle_ptr_to_queue_handle(handle));
        vQueueDelete(mp3_handle_ptr_to_queue_handle(handle));
        mp3_handle_ptr_to_queue_handle(handle) = NULL;
        MP3_LOG_I("[MP3 Codec] delete mp3 queue end.\n", 0);
    }

#if !defined(MTK_AVM_DIRECT)
    audio_free_id(handle->audio_id);
#endif

    //vPortFree(internal_handle->memory_pool);  // since now we are using static memory, so can't free, but even using dynamic, i think free function also act at app site
#if !defined(MTK_AVM_DIRECT)
    vPortFree(mp3_handle_ptr_to_mp3_decode_buffer(handle));
#endif
    mp3_handle_ptr_to_mp3_decode_buffer(handle) = NULL;

    internal_handle->memory_pool = NULL;
    internal_handle->IsMP3Exit = false;
    mp3_codec_mutex_unlock(handle);
    vSemaphoreDelete(internal_handle->semaphore_handle);
    vPortFree(internal_handle);
    internal_handle = NULL;

    mp3_handle_ptr_to_internal_ptr(handle) = NULL;

#if defined(MTK_AUDIO_MIXER_SUPPORT)
// if mixer enable, audio_lowpower_set_mode is control by mixer driver
#else
#ifdef HAL_AUDIO_LOW_POWER_ENABLED
    audio_lowpower_set_mode(false);
#endif
#endif

    vPortFree(handle->private_data);
    handle->private_data = NULL;


    if (mp3_codec_open_counter < 1) {
        mp3_codec_open_counter = 0;
    } else {
        mp3_codec_open_counter--;
    }

#if defined(MTK_AUDIO_MIXER_SUPPORT)
    MP3_LOG_I("[MP3 Codec] close-- handle(%X), role%d\n", 2, handle, handle->mixer_track_role);
#else
    MP3_LOG_I("[MP3 Codec] close--\n", 0);
#endif

    return MP3_CODEC_RETURN_OK;
}

mp3_codec_media_handle_t *mp3_codec_open(mp3_codec_callback_t mp3_codec_callback)
{
    MP3_LOG_I("[MP3 Codec]Open codec\n", 0);

    mp3_codec_media_handle_t *handle;
    mp3_codec_internal_handle_t *internal_handle; /*internal handler*/
#if defined(MTK_AVM_DIRECT)
    uint16_t audio_id = 0;
#else
    //get audio id
    uint16_t audio_id = audio_get_id();

    if (audio_id > MAX_AUDIO_FUNCTIONS) {
        MP3_LOG_I("[MP3 Codec]Audio ID > MAX AUDIO FUNCTIONS\n", 0);
        return 0;
    }
#endif
    audio_codec_mutex_create(); /* temp sol. to protect play/resume flow  */

#if defined(MTK_AUDIO_MIXER_SUPPORT)
// if mixer enable, audio_lowpower_set_mode is control by mixer driver
#else
#ifdef HAL_AUDIO_LOW_POWER_ENABLED
    audio_lowpower_set_mode(true);
#endif
#endif

    mp3_codec_private_t *private_data;
    private_data = (mp3_codec_private_t *)pvPortMalloc(sizeof(mp3_codec_private_t));
    if (NULL == private_data) {
        MP3_LOG_E("[MP3 Codec] mp3_codec_open failed: cannot allocate private_data\n", 0);
        return NULL;
    }
    memset(private_data, 0, sizeof(mp3_codec_private_t));
    private_data->previous_mp3_frame_size = SHARE_BUFFER_TOO_LESS_AMOUNT;

    /* alloc internal handler space */
    internal_handle = (mp3_codec_internal_handle_t *)pvPortMalloc(sizeof(mp3_codec_internal_handle_t));
    if (NULL == internal_handle) {
        MP3_LOG_E("[MP3 Codec] mp3_codec_open failed: cannot allocate internal_handle\n", 0);
        vPortFree(private_data);
        private_data = NULL;
        return NULL;
    }
    memset(internal_handle, 0, sizeof(mp3_codec_internal_handle_t));
    MP3_LOG_I("mp3_codec_open: internal_handle = 0x%X", 1, internal_handle);

    /* assign internal handler to be global and static handler*/
    private_data->mp3_codec_internal_hdl = internal_handle;

    /* initialize internal handle*/
    internal_handle->share_buff_size = 0;
    internal_handle->decode_pcm_buffer_size = 0;
    internal_handle->stream_out_pcm_buff_size = 0;
    internal_handle->working_buff1_size = 0;
    internal_handle->working_buff2_size = 0;
    internal_handle->IsMP3Exit = true;
    internal_handle->semaphore_handle = xSemaphoreCreateMutex();
    internal_handle->media_bitstream_end = 0;
    handle = &internal_handle->handle;
    handle->audio_id = audio_id;
    handle->handler  = mp3_codec_callback;
    handle->play     = mp3_codec_play;
    handle->pause    = mp3_codec_pause;
    handle->resume   = mp3_codec_resume;
    handle->stop     = mp3_codec_stop;
    handle->close_codec = mp3_codec_close;
    handle->skip_id3v2_and_reach_next_frame = mp3_codec_skip_id3v2_and_reach_next_frame;
    handle->state    = MP3_CODEC_STATE_READY;
    handle->flush_data_flag = 0;
    handle->private_data = private_data;


    handle->aws_sync_request = false;
    handle->aws_sync_time    = 0;

    mp3_codec_buffer_function_init(handle);

#if defined(MTK_AUDIO_MIXER_SUPPORT)
    handle->mixer_track_role = AUDIO_MIXER_TRACK_ROLE_MAIN;
#endif

    if (private_data->mp3_codec_queue_hdl == NULL) {
        private_data->mp3_codec_queue_hdl = xQueueCreate(MP3_CODEC_QUEUE_LENGTH, sizeof(mp3_codec_queue_event_t));
        MP3_LOG_I("[MP3 Codec] create mp3 queue (0x%X)\r\n", 1, private_data->mp3_codec_queue_hdl);
    }

    {   /* Initialize queue registration */
        uint32_t id_idx;
        for (id_idx = 0; id_idx < MAX_MP3_CODEC_FUNCTIONS; id_idx++) {
            (mp3_handle_ptr_to_queue_id_array(handle))[id_idx] = MP3_CODEC_QUEUE_EVENT_NONE;
        }
    }

#ifndef AIR_MP3_TASK_DEDICATE_ENABLE
    //create decode task
    mp3_codec_task_create(handle);
#endif

    if (mp3_codec_open_counter < 0) {
        mp3_codec_open_counter = 1;
    } else {
        mp3_codec_open_counter++;
    }
#if !defined(MTK_AVM_DIRECT)
    if (blIsRegister == false) {
        blIsRegister = true;
        hal_cm4_topsm_register_resume_cb((cm4_topsm_cb)mp3_codec_enter_resume, NULL);
        hal_cm4_topsm_register_suspend_cb((cm4_topsm_cb)mp3_codec_enter_suspend, NULL);
    }
#endif

#if defined(MTK_AUDIO_MIXER_SUPPORT)
#ifdef AIR_MP3_TASK_DEDICATE_ENABLE
    g_mp3_codec_task_handle = handle;
#endif
#else
    global_mp3_handle = handle;
#endif
    return handle;
}


void mp3_codec_enter_resume(void *data)
{
    //LOGI("[MP3 Codec] mp3_codec_enter_resume: mp3_codec_open_counter = %d\n", mp3_codec_open_counter);
    if (mp3_codec_open_counter < 1) {
        return ;
    }

#if defined(HAL_DVFS_MODULE_ENABLED) && !defined(MTK_AVM_DIRECT)
    mp3_codec_register_mp3_dvfs(true);
#endif

}


void mp3_codec_enter_suspend(void *data)
{
    //LOGI("[MP3 Codec] mp3_codec_enter_suspend: mp3_codec_open_counter = %d\n", mp3_codec_open_counter);
    if (mp3_codec_open_counter < 1) {
        return ;
    }

#if defined(HAL_DVFS_MODULE_ENABLED) && !defined(MTK_AVM_DIRECT)
    mp3_codec_register_mp3_dvfs(false);
#endif
}

#if 0
mp3_codec_function_return_state_t mp3_codec_enter_resume(void)
{
    //printf("mp3_codec_enter_resume IsMP3Exit=%d\n",mp3_codec_internal_handle->IsMP3Exit);
    if (mp3_codec_internal_handle->IsMP3Exit == false) {
        return MP3_CODEC_RETURN_ERROR;
    }

#if defined(HAL_DVFS_MODULE_ENABLED) && !defined(MTK_AVM_DIRECT)
    mp3_codec_register_dsp_dvfs(true);
#endif
    return MP3_CODEC_RETURN_OK;
}


mp3_codec_function_return_state_t mp3_codec_enter_suspend(void)
{
    //printf("mp3_codec_enter_suspend IsMP3Exit=%d\n",mp3_codec_internal_handle->IsMP3Exit);
    if (mp3_codec_internal_handle->IsMP3Exit == false) {
        return MP3_CODEC_RETURN_ERROR;
    }

#if defined(HAL_DVFS_MODULE_ENABLED) && !defined(MTK_AVM_DIRECT)
    //mp3_codec_register_dsp_dvfs(false);
#endif
    return MP3_CODEC_RETURN_OK;

}
#endif
