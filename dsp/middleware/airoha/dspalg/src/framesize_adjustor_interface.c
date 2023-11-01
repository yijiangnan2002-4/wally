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
#include "dsp_feature_interface.h"
#include "framesize_adjustor_interface.h"
#include "dsp_audio_process.h"
#include "dsp_dump.h"
#include "memory_attribute.h"
#include "preloader_pisplit.h"

/* Private define ------------------------------------------------------------*/
#define FRAMESIZE_ADJUSTOR_DEBUG_LOG                  0
#define FRAMESIZE_ADJUSTOR_DEBUG_DUMP                 0
#define FRAMESIZE_ADJUSTOR_A2DP_SBC_SIZE              0 //(128*4)//Add for dynamic SBC framesize

/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static framesize_adjustor_port_t framesize_adjustor_port[FRAMESIZE_ADJUSTOR_PORT_MAX];
static framesize_adjustor_list_t framesize_adjustor_list[FRAMESIZE_ADJUSTOR_LIST_MAX];

/* Public variables ----------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
ATTR_TEXT_IN_IRAM __attribute__((noinline))
static void framesize_adjustor_data_copy(void *des, void *src, uint32_t data_size)
{
    uint32_t i, remain_size;

    if (((((uint32_t)des) % 4) == 0) && ((((uint32_t)src) % 4) == 0)) {
        /* word access for better performance */
        remain_size = data_size % 4;
        data_size = data_size / 4;

        for (i = 0; i < data_size; i++) {
            *((uint32_t *)des + i) = *((uint32_t *)src + i);
        }

        if (remain_size == 1) {
            *((uint8_t *)((uint32_t)des + data_size * 4)) = *((uint8_t *)((uint32_t)src + data_size * 4));
        } else if (remain_size == 2) {
            *((uint16_t *)((uint32_t)des + data_size * 4)) = *((uint16_t *)((uint32_t)src + data_size * 4));
        } else if (remain_size == 3) {
            *((uint16_t *)((uint32_t)des + data_size * 4)) = *((uint16_t *)((uint32_t)src + data_size * 4));
            *((uint8_t *)((uint32_t)des + data_size * 4 + 2)) = *((uint8_t *)((uint32_t)src + data_size * 4 + 2));
        } else {
            /* do nothing */
        }
    } else {
        /* half-word access for better performance */
        remain_size = data_size % 2;
        data_size = data_size / 2;

        for (i = 0; i < data_size; i++) {
            *((uint16_t *)des + i) = *((uint16_t *)src + i);
        }

        if (remain_size == 1) {
            *((uint8_t *)((uint32_t)des + data_size * 2)) = *((uint8_t *)((uint32_t)src + data_size * 2));
        } else {
            /* do nothing */
        }
    }
}

static void framesize_adjustor_write_data_to_channel_buf(framesize_adjustor_channel_buf_t *channel_buf, uint8_t *src, uint32_t frame_size) {

    uint32_t remian_size = 0;
    if ((channel_buf->write_to + frame_size) > (channel_buf->buf + channel_buf->buf_size)) {
        /* wrapper case */
        remian_size = (uint32_t)(channel_buf->buf + channel_buf->buf_size) - (uint32_t)(channel_buf->write_to);
        framesize_adjustor_data_copy((void *)(channel_buf->write_to), (void *)src, remian_size);
        channel_buf->write_to = channel_buf->buf;
        framesize_adjustor_data_copy((void *)(channel_buf->write_to), (void *)(src + remian_size), frame_size - remian_size);
        channel_buf->write_to += frame_size - remian_size;
    } else {
        /* normal case */
        framesize_adjustor_data_copy((void *)(channel_buf->write_to), (void *)src, frame_size);
        channel_buf->write_to += frame_size;
        if (channel_buf->write_to == (channel_buf->buf + channel_buf->buf_size)) {
            channel_buf->write_to = channel_buf->buf;
        }
    }
    /* update data size */
    channel_buf->data_size += frame_size;

}

static void framesize_adjustor_read_data_from_channel_buf(framesize_adjustor_channel_buf_t *channel_buf, uint8_t* dst, uint32_t frame_size) {

    uint32_t remian_size = 0;
    if ((channel_buf->read_from + frame_size) > (channel_buf->buf + channel_buf->buf_size)) {
        /* wrapper case */
        remian_size = (uint32_t)(channel_buf->buf + channel_buf->buf_size) - (uint32_t)(channel_buf->read_from);
        framesize_adjustor_data_copy((void *)dst, (void *)(channel_buf->read_from), remian_size);
        channel_buf->read_from = channel_buf->buf;
        framesize_adjustor_data_copy((void *)(dst + remian_size), (void *)(channel_buf->read_from), frame_size - remian_size);
        channel_buf->read_from += frame_size - remian_size;
    } else {
        /* normal case */
        framesize_adjustor_data_copy((void *)dst, (void *)(channel_buf->read_from), frame_size);
        channel_buf->read_from += frame_size;
        if (channel_buf->read_from == (channel_buf->buf + channel_buf->buf_size)) {
            channel_buf->read_from = channel_buf->buf;
        }
    }
    /* update data size */
    channel_buf->data_size -= frame_size;
}


static void framesize_adjustor_write_data_to_channel_buf_v2(framesize_adjustor_port_t *port) {
    uint32_t process_frame_size = port->process_size;
    framesize_adjustor_channel_t *channel_l, *channel_r;
    framesize_adjustor_channel_buf_t *channel_buf_l , *channel_buf_r;
    channel_l = port->channel;
    channel_r = port->channel + 1;
    channel_buf_l = &(channel_l->out);
    channel_buf_r = &(channel_r->out);
    uint16_t process_sample_resolution = port->process_sample_resolution;
    uint32_t i, j;

    uint32_t remian_size = 0;
    if ((channel_buf_l->write_to + process_frame_size) > (channel_buf_l->buf + channel_buf_l->buf_size)) {
        /* wrapper case */
        remian_size = (uint32_t)(channel_buf_l->buf + channel_buf_l->buf_size) - (uint32_t)(channel_buf_l->write_to);
        //framesize_adjustor_data_copy((void *)(channel_buf_l->write_to), (void *)src, remian_size);
        if (process_sample_resolution == RESOLUTION_16BIT) {
            uint32_t sample_cnt = remian_size>>1;
            for (i = 0; i < sample_cnt; i++ ) {
                *((uint16_t*)channel_buf_l->write_to + i) = *((uint16_t*)channel_buf_l->temp + 2*i);
                *((uint16_t*)channel_buf_r->write_to + i) = *((uint16_t*)channel_buf_l->temp + 2*i+1);
            }
        } else if (process_sample_resolution == RESOLUTION_32BIT) {
            uint32_t sample_cnt = remian_size>>2;
            for (i = 0; i < sample_cnt; i++ ) {
                *((uint32_t*)channel_buf_l->write_to + i) = *((uint32_t*)channel_buf_l->temp + 2*i);
                *((uint32_t*)channel_buf_r->write_to + i) = *((uint32_t*)channel_buf_l->temp + 2*i+1);
            }
        }
        channel_buf_l->write_to = channel_buf_l->buf;
        channel_buf_r->write_to = channel_buf_r->buf;

        //framesize_adjustor_data_copy((void *)(channel_buf_l->write_to), (void *)(src + remian_size), frame_size - remian_size);
        if (process_sample_resolution == RESOLUTION_16BIT) {
             uint32_t sample_cnt = (process_frame_size - remian_size)>>1;
             for (j = 0; j < sample_cnt; j++ ) {
                *((uint16_t*)channel_buf_l->write_to + j) = *((uint16_t*)channel_buf_l->temp + 2*(i+j));
                *((uint16_t*)channel_buf_r->write_to + j) = *((uint16_t*)channel_buf_l->temp + 2*(i+j)+1);
                }
         } else if (process_sample_resolution == RESOLUTION_32BIT) {
            uint32_t sample_cnt = (process_frame_size - remian_size)>>2;
             for (j = 0; j < sample_cnt; j++ ) {
                 *((uint32_t*)channel_buf_l->write_to + j) = *((uint32_t*)channel_buf_l->temp + 2*(i+j));
                 *((uint32_t*)channel_buf_r->write_to + j) = *((uint32_t*)channel_buf_l->temp + 2*(i+j)+1);
             }
         }
        channel_buf_l->write_to += process_frame_size - remian_size;
        channel_buf_r->write_to += process_frame_size - remian_size;
    } else {
        /* normal case */
        //framesize_adjustor_data_copy((void *)(channel_buf->write_to), (void *)src, frame_size);
        if (process_sample_resolution == RESOLUTION_16BIT) {
            uint32_t sample_cnt = process_frame_size>>1;
            for (i = 0; i < sample_cnt; i++ ) {
                *((uint16_t*)channel_buf_l->write_to + i) = *((uint16_t*)channel_buf_l->temp + 2*i);
                *((uint16_t*)channel_buf_r->write_to + i) = *((uint16_t*)channel_buf_l->temp + 2*i+1);
            }
        } else if (process_sample_resolution == RESOLUTION_32BIT) {
            uint32_t sample_cnt = process_frame_size>>2;
            for (i = 0; i < sample_cnt; i++ ) {
                *(((uint32_t*)channel_buf_l->write_to) + i) = *(((uint32_t*)channel_buf_l->temp) + 2*i);
                *(((uint32_t*)channel_buf_r->write_to) + i) = *(((uint32_t*)channel_buf_l->temp) + 2*i+1);
            }
        }
        channel_buf_l->write_to += process_frame_size;
        if (channel_buf_l->write_to == channel_buf_l->buf + channel_buf_l->buf_size) {
            channel_buf_l->write_to = channel_buf_l->buf;
        }
        channel_buf_r->write_to += process_frame_size;
        if (channel_buf_r->write_to == channel_buf_r->buf + channel_buf_r->buf_size) {
            channel_buf_r->write_to = channel_buf_r->buf;
        }
    }
    /* update data size */
    channel_buf_l->data_size += process_frame_size;
    channel_buf_r->data_size += process_frame_size;
}

static void framesize_adjustor_read_data_from_channel_buf_v2(framesize_adjustor_port_t *port) {
    uint32_t process_frame_size = port->process_size;
    framesize_adjustor_channel_t *channel_l, *channel_r;
    framesize_adjustor_channel_buf_t *channel_buf_l , *channel_buf_r;
    channel_l = port->channel;
    channel_r = port->channel + 1;
    channel_buf_l = &(channel_l->in);
    channel_buf_r = &(channel_r->in);
    uint32_t i, j;
    uint16_t process_sample_resolution = port->process_sample_resolution;

    uint32_t remian_size = 0;
    if ((channel_buf_l->read_from + process_frame_size) > (channel_buf_l->buf + channel_buf_l->buf_size)) {
        /* wrapper case */
        remian_size = (uint32_t)(channel_buf_l->buf + channel_buf_l->buf_size) - (uint32_t)(channel_buf_l->read_from);
        //framesize_adjustor_data_copy((void *)dst, (void *)(channel_buf->read_from), remian_size);
        if (process_sample_resolution == RESOLUTION_16BIT) {
            uint32_t sample_cnt = remian_size>>1;
            for (i = 0; i < sample_cnt; i++ ) {
                *((uint16_t*)channel_buf_l->temp + 2*i)  = *((uint16_t*)channel_buf_l->read_from + i);
                *((uint16_t*)channel_buf_l->temp + 2*i+1)  = *((uint16_t*)channel_buf_r->read_from + i);
            }
        } else if (process_sample_resolution == RESOLUTION_32BIT) {
            uint32_t sample_cnt = remian_size>>2;
            for (i = 0; i < sample_cnt; i++ ) {
                *((uint32_t*)channel_buf_l->temp + 2*i)  = *((uint32_t*)channel_buf_l->read_from + i);
                *((uint32_t*)channel_buf_l->temp + 2*i+1)  = *((uint32_t*)channel_buf_r->read_from + i);
            }
        }
        channel_buf_l->read_from = channel_buf_l->buf;
        channel_buf_r->read_from = channel_buf_r->buf;
        //framesize_adjustor_data_copy((void *)(dst + remian_size), (void *)(channel_buf_l->read_from), frame_size - remian_size);
        if (process_sample_resolution == RESOLUTION_16BIT) {
            uint32_t sample_cnt = (process_frame_size - remian_size)>>1;
            for (j = 0; j < sample_cnt; j++ ) {
                *((uint16_t*)channel_buf_l->temp + 2*(i + j))  = *((uint16_t*)channel_buf_l->read_from + j);
                *((uint16_t*)channel_buf_l->temp + 2*(i + j)+1)  = *((uint16_t*)channel_buf_r->read_from + j);
            }
        } else if (process_sample_resolution == RESOLUTION_32BIT) {
            uint32_t sample_cnt = (process_frame_size - remian_size)>>2;
            for (j = 0; j < sample_cnt; j++ ) {
                *((uint32_t*)channel_buf_l->temp + 2*(i + j))  = *((uint32_t*)channel_buf_l->read_from + j);
                *((uint32_t*)channel_buf_l->temp + 2*(i + j)+1)  = *((uint32_t*)channel_buf_r->read_from + j);
            }
        }
        channel_buf_l->read_from += process_frame_size - remian_size;
        channel_buf_r->read_from += process_frame_size - remian_size;
    } else {
        /* normal case */
        //framesize_adjustor_data_copy((void *)dst, (void *)(channel_buf_l->read_from), frame_size);

        if (process_sample_resolution == RESOLUTION_16BIT) {
            uint32_t sample_cnt = process_frame_size>>1;
            for (i = 0; i < sample_cnt; i++ ) {
                *((uint16_t*)channel_buf_l->temp + 2*i)  = *((uint16_t*)channel_buf_l->read_from + i);
                *((uint16_t*)channel_buf_l->temp + 2*i+1)  = *((uint16_t*)channel_buf_r->read_from + i);
            }
        } else if (process_sample_resolution == RESOLUTION_32BIT) {
            uint32_t sample_cnt = process_frame_size>>2;
            for (i = 0; i < sample_cnt; i++ ) {
                *((uint32_t*)channel_buf_l->temp + 2*i)  = *((uint32_t*)channel_buf_l->read_from + i);
                *((uint32_t*)channel_buf_l->temp + 2*i+1)  = *((uint32_t*)channel_buf_r->read_from + i);
            }
        }

        channel_buf_l->read_from += process_frame_size;
        if (channel_buf_l->read_from == (channel_buf_l->buf + channel_buf_l->buf_size)) {
            channel_buf_l->read_from = channel_buf_l->buf;
        }
        channel_buf_r->read_from += process_frame_size;
        if (channel_buf_r->read_from == (channel_buf_r->buf + channel_buf_r->buf_size)) {
            channel_buf_r->read_from = channel_buf_r->buf;
        }
    }
    /* update data size */
    channel_buf_l->data_size -= process_frame_size;
    channel_buf_r->data_size -= process_frame_size;
}


ATTR_TEXT_IN_IRAM static framesize_adjustor_port_t *stream_function_framesize_adjustor_find_out_port(DSP_STREAMING_PARA_PTR stream_ptr)
{
    int32_t i;
    framesize_adjustor_port_t *port = NULL;
    framesize_adjustor_list_t *list = NULL;

    /* At first, check if this stream use multi-buffer list */
    for (i = 0; i < FRAMESIZE_ADJUSTOR_LIST_MAX; i++) {
        /* Check if this source or sink has already owned a framesize adjustor */
        if ((framesize_adjustor_list[i].owner == stream_ptr->source) ||
            (framesize_adjustor_list[i].owner == stream_ptr->sink)) {
            list = &framesize_adjustor_list[i];
            break;
        }
    }
    if (list != NULL) {
        port = list->port_list[list->current_index];
        list->current_index = (list->current_index + 1) % (list->buffer_total);
    } else {
        /* The stream does not use multi-buffer list, so need to find out the buffer one by one */
        for (i = 0; i < FRAMESIZE_ADJUSTOR_PORT_MAX; i++) {
            /* Check if this source or sink has already owned a framesize adjustor */
            if ((framesize_adjustor_port[i].owner == stream_ptr->source) ||
                (framesize_adjustor_port[i].owner == stream_ptr->sink)) {
                port = &framesize_adjustor_port[i];
                break;
            }
        }
    }
    if (port == NULL) {
        DSP_MW_LOG_E("[FRAMESIZE_ADJUSTOR] Port is not found!", 0);
        AUDIO_ASSERT(FALSE);
    }

    return port;
}

/* Public functions ----------------------------------------------------------*/
ATTR_TEXT_IN_RAM_FOR_MASK_IRQ framesize_adjustor_port_t *stream_function_framesize_adjustor_get_port(void *owner)
{
    int32_t i;
    framesize_adjustor_port_t *port = NULL;
    uint32_t saved_mask;

    /* Find out a port for this owner */
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    for (i = FRAMESIZE_ADJUSTOR_PORT_MAX - 1; i >= 0; i--) {
        /* Check if there is unused port */
        if (framesize_adjustor_port[i].owner == NULL) {
            port = &framesize_adjustor_port[i];
            continue;
        }

        /* Check if this owner has already owned a framesize adjustor */
        if (framesize_adjustor_port[i].owner == owner) {
            port = &framesize_adjustor_port[i];
            break;
        }
    }
    hal_nvic_restore_interrupt_mask(saved_mask);

    if (port == NULL) {
        DSP_MW_LOG_E("[FRAMESIZE_ADJUSTOR] Port not enough!", 0);
        AUDIO_ASSERT(0);
        return port;
    }

    port->owner = owner;

    return port;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ framesize_adjustor_port_t *stream_function_framesize_adjustor_get_unused_port(void *owner)
{
    int32_t i;
    framesize_adjustor_port_t *port = NULL;
    uint32_t saved_mask;

    /* Find out a unued port for this owner */
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    for (i = 0; i < FRAMESIZE_ADJUSTOR_PORT_MAX; i++) {
        /* Check if there is unused port */
        if (framesize_adjustor_port[i].owner == NULL) {
            port = &framesize_adjustor_port[i];
            break;
        }
    }
    hal_nvic_restore_interrupt_mask(saved_mask);

    if (port == NULL) {
        DSP_MW_LOG_E("[FRAMESIZE_ADJUSTOR] All ports are used!", 0);
        return port;
    }

    port->owner = owner;

    return port;
}

framesize_adjustor_status_t stream_function_framesize_adjustor_init(framesize_adjustor_port_t *port, framesize_adjustor_config_t *config, uint32_t MemSize, CusFunc_initialize fun_initialize, CusFunc_process fun_process)
{
    uint32_t i;
    framesize_adjustor_channel_t *channel;
    uint16_t prefill_samples, prefill_size, watermark_max_size;

    /* check port */
    if (port == NULL) {
        DSP_MW_LOG_E("[FRAMESIZE_ADJUSTOR] Port is NULL!", 0);
        AUDIO_ASSERT(0);
        return FRAMESIZE_ADJUSTOR_STATUS_ERROR;
    }

    if (config->type == FRAMESIZE_ADJUSTOR_FUNCTION_INTERLEAVED && config->total_channels != 2) {
        DSP_MW_LOG_W("[FRAMESIZE_ADJUSTOR] interleaved FUCNTION_TYPE is not match with tota_channels %d! Force to DEINTERLEAVED", 1, config->total_channels);
        config->type = FRAMESIZE_ADJUSTOR_FUNCTION_DEINTERLEAVED;
        //AUDIO_ASSERT(0);
    }

    /* get channels' setting space */
    port->total_channels = config->total_channels;
    port->channel = preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, sizeof(framesize_adjustor_channel_t) * config->total_channels);
    if (port->channel == NULL) {
        AUDIO_ASSERT(0);
        return FRAMESIZE_ADJUSTOR_STATUS_ERROR;
    }
    /* calculate buffer settings*/
    prefill_samples = stream_function_framesize_adjustor_get_prefill_sample(config->inout_sample_num, config->process_sample_num);
    prefill_size = prefill_samples<<(config->process_sample_resolution==RESOLUTION_16BIT?1:2);
    watermark_max_size = ((prefill_samples + config->inout_sample_num)<<(config->process_sample_resolution==RESOLUTION_16BIT?1:2)) + FRAMESIZE_ADJUSTOR_A2DP_SBC_SIZE;

    /* config buffer settings */
    port->mode = config->mode;
    port->type = config->type;
    port->process_sample_resolution = config->process_sample_resolution;
    port->inout_size = config->inout_sample_num<<(config->process_sample_resolution==RESOLUTION_16BIT?1:2);
    port->process_size = config->process_sample_num<<(config->process_sample_resolution==RESOLUTION_16BIT?1:2);

    port->fun_initialize = fun_initialize;
    port->fun_process = fun_process;
    port->MemSize = MemSize;
    switch (config->mode) {
        case FRAMESIZE_ADJUSTOR_MODE_FIXED_INOUT_LENGTH:
        case FRAMESIZE_ADJUSTOR_MODE_MULTI_BUFFERS:
        case FRAMESIZE_ADJUSTOR_MODE_CHANGE_INOUT_LENGTH:
            for (i = 0; i < config->total_channels; i++) {
                channel = port->channel + i;

                channel->in.buf_size = watermark_max_size;
                channel->in.data_size = 0;
                channel->out.buf_size = watermark_max_size;
                channel->out.data_size = 0;
                channel->in.buf = preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, watermark_max_size);
                channel->out.buf = preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, watermark_max_size);

                if (channel->in.buf == NULL || channel->out.buf == NULL) {
                    AUDIO_ASSERT(0);
                }
                if (config->type == FRAMESIZE_ADJUSTOR_FUNCTION_DEINTERLEAVED) {
                    channel->in.temp = preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, port->process_size);
                    channel->out.temp = preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, port->process_size);
                    if (channel->in.temp == NULL || channel->out.temp == NULL) {
                        AUDIO_ASSERT(0);
                    }
                } else if (config->type == FRAMESIZE_ADJUSTOR_FUNCTION_INTERLEAVED) {
                    if (i == 0) {
                        channel->in.temp = preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, (port->process_size)<<1);
                        channel->out.temp = preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, (port->process_size)<<1);
                        if (channel->in.temp == NULL || channel->out.temp == NULL) {
                            AUDIO_ASSERT(0);
                        }
                    } else {
                        channel->in.temp = NULL;
                        channel->out.temp = NULL;
                    }
                }
                channel->in.read_from = channel->in.buf;
                channel->in.write_to = channel->in.buf;
                channel->out.read_from = channel->out.buf;
                channel->out.write_to = channel->out.buf;
            }
            port->watermark_max_size = watermark_max_size;
            break;

        default:
            AUDIO_ASSERT(0);
            break;
    }
    if (port->inout_size < port->process_size) {
        stream_function_framesize_adjustor_config_channel_prefill_size(port, 0, prefill_size, true, FRAMESIZE_ADJUSTOR_BUFFER_INPUT);
    } else {
        stream_function_framesize_adjustor_config_channel_prefill_size(port, 0, prefill_size, true, FRAMESIZE_ADJUSTOR_BUFFER_OUTPUT);
    }
    DSP_MW_LOG_I("[FRAMESIZE_ADJUSTOR] Init Done, inout_size = %d process_size = %d prefill_samples = %d", 3, port->inout_size, port->process_size, prefill_samples);

    /* update status */
    port->status = FRAMESIZE_ADJUSTOR_PORT_STATUS_INIT;

    return FRAMESIZE_ADJUSTOR_STATUS_OK;
}

uint16_t stream_function_framesize_adjustor_gcd(uint16_t m, uint16_t n) {
    while(n != 0) {
        uint16_t r = m % n;
        m = n;
        n = r;
    }
    return m;
}

uint16_t stream_function_framesize_adjustor_get_prefill_sample(uint16_t inout_sample_num, uint16_t process_sample_num) {

    uint16_t num = 0;
    /*Calculate framesize adjustor temp buffer size*/
    if (inout_sample_num < process_sample_num) {
        num = MAX(inout_sample_num, process_sample_num);
    } else {
        num = MIN(inout_sample_num, process_sample_num);
    }
    uint16_t gcb = stream_function_framesize_adjustor_gcd(inout_sample_num, process_sample_num);
    uint16_t prefill_samples = num - gcb;
    DSP_MW_LOG_I("[FRAMESIZE_ADJUSTOR] prefill_samples = %d\r\n", 1, prefill_samples);

    return prefill_samples;
}

framesize_adjustor_status_t stream_function_framesize_adjustor_deinit(framesize_adjustor_port_t *port)
{
    uint32_t i;
    framesize_adjustor_channel_t *channel;
    /* check port */
    if (port == NULL) {
        DSP_MW_LOG_E("[FRAMESIZE_ADJUSTOR] Port is NULL!", 0);
        AUDIO_ASSERT(0);
        return FRAMESIZE_ADJUSTOR_STATUS_ERROR;
    }

    for (i = 0; i < port->total_channels; i++) {
        channel = port->channel + i;

        channel->in.data_size = 0;
        channel->out.data_size = 0;
        preloader_pisplit_free_memory(channel->in.buf);
        preloader_pisplit_free_memory(channel->out.buf);
        if (port->type == FRAMESIZE_ADJUSTOR_FUNCTION_DEINTERLEAVED) {
            preloader_pisplit_free_memory(channel->in.temp);
            preloader_pisplit_free_memory(channel->out.temp);
        } else if (port->type == FRAMESIZE_ADJUSTOR_FUNCTION_INTERLEAVED) {
            if (i == 0) {
                preloader_pisplit_free_memory(channel->in.temp);
                preloader_pisplit_free_memory(channel->out.temp);
            } else {
                /* do nothing*/
            }

        }
    }

    preloader_pisplit_free_memory(port->channel);
    port->channel = NULL;
    port->owner = NULL;
    port->stream = NULL;
    port->mode = FRAMESIZE_ADJUSTOR_MODE_END;
    port->type = FRAMESIZE_ADJUSTOR_FUNCTION_END;
    port->total_channels = 0;
    port->status = FRAMESIZE_ADJUSTOR_PORT_STATUS_DEINIT;
    DSP_MW_LOG_I("[FRAMESIZE_ADJUSTOR] Deinit port 0x%x Done", 1, port);

    return FRAMESIZE_ADJUSTOR_STATUS_OK;
}

ATTR_TEXT_IN_IRAM uint32_t stream_function_framesize_adjustor_get_channel_used_size(framesize_adjustor_port_t *port, uint16_t channel_number, framesize_adjustor_buffer_type_t buffer_type)
{
    framesize_adjustor_channel_t *channel;

    /* check port and channel_number */
    if ((port == NULL) || (channel_number == 0)) {
        DSP_MW_LOG_E("[FRAMESIZE_ADJUSTOR] Port is NULL or Channel is 0!", 0);
        AUDIO_ASSERT(0);
        return FRAMESIZE_ADJUSTOR_STATUS_ERROR;
    }

    channel = port->channel + (channel_number - 1);
    if (buffer_type == FRAMESIZE_ADJUSTOR_BUFFER_INPUT) {
        return (channel->in.data_size);
    }
    return (channel->out.data_size);
}

uint32_t stream_function_framesize_adjustor_get_channel_free_size(framesize_adjustor_port_t *port, uint16_t channel_number, framesize_adjustor_buffer_type_t buffer_type)
{
    framesize_adjustor_channel_t *channel;

    /* check port and channel_number */
    if ((port == NULL) || (channel_number == 0)) {
        DSP_MW_LOG_E("[FRAMESIZE_ADJUSTOR] Port is NULL or Channel is 0!", 0);
        AUDIO_ASSERT(0);
        return FRAMESIZE_ADJUSTOR_STATUS_ERROR;
    }

    channel = port->channel + (channel_number - 1);

    if (buffer_type == FRAMESIZE_ADJUSTOR_BUFFER_INPUT) {
        return (channel->in.buf_size - channel->in.data_size);
    }
    return (channel->out.buf_size - channel->out.data_size);
}

uint8_t *stream_function_framesize_adjustor_get_channel_read_pointer(framesize_adjustor_port_t *port, uint16_t channel_number, framesize_adjustor_buffer_type_t buffer_type)
{
    framesize_adjustor_channel_t *channel;

    /* check port and channel_number */
    if ((port == NULL) || (channel_number == 0)) {
        DSP_MW_LOG_E("[FRAMESIZE_ADJUSTOR] Port is NULL or Channel is 0!", 0);
        AUDIO_ASSERT(0);
        return NULL;
    }

    channel = port->channel + (channel_number - 1);

    if (buffer_type == FRAMESIZE_ADJUSTOR_BUFFER_INPUT) {
        return (channel->in.read_from);
    }
    return (channel->out.read_from);
}

uint8_t *stream_function_framesize_adjustor_get_channel_write_pointer(framesize_adjustor_port_t *port, uint16_t channel_number, framesize_adjustor_buffer_type_t buffer_type)
{
    framesize_adjustor_channel_t *channel;

    /* check port and channel_number */
    if ((port == NULL) || (channel_number == 0)) {
        DSP_MW_LOG_E("[FRAMESIZE_ADJUSTOR] Port is NULL or Channel is 0!", 0);
        AUDIO_ASSERT(0);
        return NULL;
    }

    channel = port->channel + (channel_number - 1);

    if (buffer_type == FRAMESIZE_ADJUSTOR_BUFFER_INPUT) {
        return (channel->in.write_to);
    }
    return (channel->out.write_to);
}

uint8_t *stream_function_framesize_adjustor_get_channel_start_pointer(framesize_adjustor_port_t *port, uint16_t channel_number, framesize_adjustor_buffer_type_t buffer_type)
{
    framesize_adjustor_channel_t *channel;

    /* check port and channel_number */
    if ((port == NULL) || (channel_number == 0)) {
        DSP_MW_LOG_E("[FRAMESIZE_ADJUSTOR] Port is NULL or Channel is 0!", 0);
        AUDIO_ASSERT(0);
        return NULL;
    }

    channel = port->channel + (channel_number - 1);

    if (buffer_type == FRAMESIZE_ADJUSTOR_BUFFER_INPUT) {
        return (channel->in.buf);
    }

    return (channel->out.buf);
}

uint8_t *stream_function_framesize_adjustor_get_channel_end_pointer(framesize_adjustor_port_t *port, uint16_t channel_number, framesize_adjustor_buffer_type_t buffer_type)
{
    framesize_adjustor_channel_t *channel;

    /* check port and channel_number */
    if ((port == NULL) || (channel_number == 0)) {
        DSP_MW_LOG_E("[FRAMESIZE_ADJUSTOR] Port is NULL or Channel is 0!", 0);
        AUDIO_ASSERT(0);
        return NULL;
    }

    channel = port->channel + (channel_number - 1);

    if (buffer_type == FRAMESIZE_ADJUSTOR_BUFFER_INPUT) {
        return (channel->in.buf + channel->in.buf_size);
    }
    return (channel->out.buf + channel->out.buf_size);
}

framesize_adjustor_status_t stream_function_framesize_adjustor_config_channel_prefill_size(framesize_adjustor_port_t *port, uint16_t channel_number, uint16_t perfill_size, bool set_zeros, framesize_adjustor_buffer_type_t buffer_type)
{
    framesize_adjustor_channel_t *channel;
    framesize_adjustor_channel_buf_t *channel_buf;

    uint32_t i;

    /* check port and channel_number */
    if (port == NULL) {
        DSP_MW_LOG_E("[FRAMESIZE_ADJUSTOR] Port is NULL or Channel is 0!", 0);
        AUDIO_ASSERT(0);
        return FRAMESIZE_ADJUSTOR_STATUS_ERROR;
    }

    if (channel_number != 0) {
        channel = port->channel + (channel_number - 1);
        if (buffer_type == FRAMESIZE_ADJUSTOR_BUFFER_INPUT) {
            channel_buf = &(channel->in);
        } else {
            channel_buf = &(channel->out);
        }
        if (set_zeros == true) {
            /* set zeros into the channel buffer */
            memset(channel_buf->buf, 0, perfill_size);
        }
        channel_buf->data_size = perfill_size;
        channel_buf->write_to = channel_buf->buf + perfill_size;
    } else {
        for (i = 0; i < port->total_channels; i++) {
            channel = port->channel + i;
            if (buffer_type == FRAMESIZE_ADJUSTOR_BUFFER_INPUT) {
                channel_buf = &(channel->in);
            } else {
                channel_buf = &(channel->out);
            }
            if (set_zeros == true) {
                /* set zeros into the channel buffer */
                memset(channel_buf->buf, 0, perfill_size);
            }
            channel_buf->data_size = perfill_size;
            channel_buf->write_to = channel_buf->buf + perfill_size;
        }
    }
    return FRAMESIZE_ADJUSTOR_STATUS_OK;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ framesize_adjustor_status_t stream_function_framesize_adjustor_reset_channel_buffer(framesize_adjustor_port_t *port, uint16_t channel_number, bool set_zeros, framesize_adjustor_buffer_type_t buffer_type)
{
    framesize_adjustor_channel_t *channel;
    framesize_adjustor_channel_buf_t *channel_buf;
    uint32_t saved_mask;

    /* check port and channel_number */
    if ((port == NULL) || (channel_number == 0)) {
        DSP_MW_LOG_E("[FRAMESIZE_ADJUSTOR] Port is NULL or Channel is 0!", 0);
        AUDIO_ASSERT(0);
        return FRAMESIZE_ADJUSTOR_STATUS_ERROR;
    }

    channel = port->channel + (channel_number - 1);
    if (buffer_type == FRAMESIZE_ADJUSTOR_BUFFER_INPUT) {
        channel_buf = &(channel->in);
    } else {
        channel_buf = &(channel->out);
    }
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    channel_buf->data_size = 0;
    channel_buf->read_from = channel_buf->buf;
    channel_buf->write_to = channel_buf->buf;

    if (set_zeros == true) {
        /* set zeros into the channel buffer */
        memset(channel_buf->buf, 0, channel_buf->buf_size);
    }
    hal_nvic_restore_interrupt_mask(saved_mask);

    return FRAMESIZE_ADJUSTOR_STATUS_OK;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ framesize_adjustor_list_t *stream_function_framesize_adjustor_get_list(void *owner)
{
    int32_t i;
    framesize_adjustor_list_t *list = NULL;
    uint32_t saved_mask;

    /* Find out a list for this owner */
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    for (i = FRAMESIZE_ADJUSTOR_LIST_MAX - 1; i >= 0; i--) {
        /* Check if there is unused list */
        if (framesize_adjustor_list[i].owner == NULL) {
            list = &framesize_adjustor_list[i];
            continue;
        }

        /* Check if this owner has already owned a framesize adjustor */
        if (framesize_adjustor_list[i].owner == owner) {
            list = &framesize_adjustor_list[i];
            break;
        }
    }
    hal_nvic_restore_interrupt_mask(saved_mask);

    if (list == NULL) {
        DSP_MW_LOG_E("[FRAMESIZE_ADJUSTOR] list not enough!", 0);
        AUDIO_ASSERT(0);
        return list;
    }

    list->owner = owner;

    return list;
}

framesize_adjustor_status_t stream_function_framesize_adjustor_list_init(framesize_adjustor_list_t *list, uint16_t buffer_total_number)
{
    uint32_t i;

    /* check list */
    if ((list == NULL) || (buffer_total_number > FRAMESIZE_ADJUSTOR_PORT_MAX)) {
        DSP_MW_LOG_E("[FRAMESIZE_ADJUSTOR] List is NULL!", 0);
        AUDIO_ASSERT(0);
        return FRAMESIZE_ADJUSTOR_STATUS_ERROR;
    }

    list->current_index = 0;
    list->buffer_total = buffer_total_number;
    list->port_list = NULL;
    list->port_list = preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, buffer_total_number * sizeof(framesize_adjustor_port_t *));
    if (list->port_list == NULL) {
        AUDIO_ASSERT(0);
        return FRAMESIZE_ADJUSTOR_STATUS_ERROR;
    }
    for (i = 0; i < buffer_total_number; i++) {
        list->port_list[i] = NULL;
    }
    DSP_MW_LOG_I("[FRAMESIZE_ADJUSTOR] Init list 0x%x Done", 1, list);

    return FRAMESIZE_ADJUSTOR_STATUS_OK;
}

framesize_adjustor_status_t stream_function_framesize_adjustor_list_deinit(framesize_adjustor_list_t *list)
{
    /* check list */
    if (list == NULL) {
        DSP_MW_LOG_E("[FRAMESIZE_ADJUSTOR] List is NULL!", 0);
        AUDIO_ASSERT(0);
        return FRAMESIZE_ADJUSTOR_STATUS_ERROR;
    }
    for(int port_index = 0; port_index < list->buffer_total; port_index++) {
        DSP_MW_LOG_I("[FRAMESIZE_ADJUSTOR] Deinit port[%d] 0x%x", 2, port_index, list->port_list[port_index]);
        stream_function_framesize_adjustor_deinit(list->port_list[port_index]);
    }

    list->owner = NULL;
    list->current_index = 0;
    list->buffer_total = 0;
    preloader_pisplit_free_memory(list->port_list);
    list->port_list = NULL;
    DSP_MW_LOG_I("[FRAMESIZE_ADJUSTOR] Deinit list 0x%x Done", 1, list);

    return FRAMESIZE_ADJUSTOR_STATUS_OK;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ framesize_adjustor_status_t stream_function_framesize_adjustor_list_insert_buffer(framesize_adjustor_list_t *list, framesize_adjustor_port_t *port, uint16_t buffer_index)
{
    uint32_t saved_mask;

    /* check parameters */
    if ((list == NULL) || (port == NULL) || (buffer_index >= FRAMESIZE_ADJUSTOR_PORT_MAX)) {
        DSP_MW_LOG_E("[FRAMESIZE_ADJUSTOR] Paramters are not right!", 0);
        AUDIO_ASSERT(0);
        return FRAMESIZE_ADJUSTOR_STATUS_ERROR;
    }

    if (list->owner != port->owner) {
        DSP_MW_LOG_E("[FRAMESIZE_ADJUSTOR] Owners are not the same!", 0);
        AUDIO_ASSERT(0);
        return FRAMESIZE_ADJUSTOR_STATUS_ERROR;
    }

    if (list->port_list == NULL) {
        DSP_MW_LOG_E("[FRAMESIZE_ADJUSTOR] List is NULL!", 0);
        AUDIO_ASSERT(0);
        return FRAMESIZE_ADJUSTOR_STATUS_ERROR;
    }

    if (port->mode != FRAMESIZE_ADJUSTOR_MODE_MULTI_BUFFERS) {
        DSP_MW_LOG_E("[FRAMESIZE_ADJUSTOR] Buffer mode are not right!", 0);
        AUDIO_ASSERT(0);
        return FRAMESIZE_ADJUSTOR_STATUS_ERROR;
    }

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    list->port_list[buffer_index] = port;
    hal_nvic_restore_interrupt_mask(saved_mask);

    return FRAMESIZE_ADJUSTOR_STATUS_OK;
}

bool stream_function_framesize_adjustor_initialize(void *para)
{
    framesize_adjustor_port_t *port;
    DSP_STREAMING_PARA_PTR stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    DSP_ENTRY_PARA_PTR entry_para = (DSP_ENTRY_PARA_PTR)para;

    /* Find out the framesize adjustor port of this stream */
    port = stream_function_framesize_adjustor_find_out_port(stream_ptr);
    CusFunc_initialize framesize_adjustor_CusFunc_initialize = port->fun_initialize;

    /* status check */
    if (port->status == FRAMESIZE_ADJUSTOR_PORT_STATUS_INIT) {
        /*Malloc for working buffer*/
        extern void *DSPMEM_tmalloc(TaskHandle_t  DSPTask, SIZE Size, VOID *usingPtr);
        port->MemPtr = DSPMEM_tmalloc(entry_para->DSPTask, port->MemSize, stream_ptr);
        /*Initialize*/
        framesize_adjustor_CusFunc_initialize(para, port->MemPtr);
        port->status = FRAMESIZE_ADJUSTOR_PORT_STATUS_RUNNING;
    } else if (port->status == FRAMESIZE_ADJUSTOR_PORT_STATUS_RUNNING) {
        return false;
    } else {
        DSP_MW_LOG_E("[FRAMESIZE_ADJUSTOR] error status, %d!", 1, port->status);
        AUDIO_ASSERT(FALSE);
        return true;
    }

    port->stream = stream_ptr;

    DSP_MW_LOG_I("[FRAMESIZE_ADJUSTOR] port 0x%x init done, owner = 0x%x, stream = 0x%x", 3, port, port->owner, port->stream);

    return false;
}

ATTR_TEXT_IN_IRAM bool stream_function_framesize_adjustor_process(void *para)
{
    int32_t i;
    framesize_adjustor_port_t *port;
    framesize_adjustor_channel_t *channel;
    framesize_adjustor_channel_buf_t *channel_buf;
    DSP_STREAMING_PARA_PTR stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    uint32_t channel_number = stream_function_get_channel_number(para);
    uint32_t in_frame_size = stream_function_get_output_size(para);
    uint8_t *buf = NULL;
    uint32_t process_frame_size;
    uint32_t out_size;
    /* Find out the framesize adjustor port of this stream */
    port = stream_function_framesize_adjustor_find_out_port(stream_ptr);
    process_frame_size = port->process_size;
    out_size = port->inout_size;

    if (in_frame_size == 0) {
        return false;
    }

    if (channel_number != port->total_channels) {
        channel_number = (channel_number<port->total_channels)?channel_number:port->total_channels;
        //DSP_MW_LOG_E("[FRAMESIZE_ADJUSTOR] channel number is not right %d %d!", 2, channel_number, port->total_channels);
        //AUDIO_ASSERT(FALSE);
        //return true;
    }

    if (stream_function_get_output_resolution(para) != port->process_sample_resolution) {
        DSP_MW_LOG_E("[FRAMESIZE_ADJUSTOR] resolution is not match!", 0);
        AUDIO_ASSERT(FALSE);
        return true;
    }

    if (in_frame_size != port->inout_size) {
        DSP_MW_LOG_W("[FRAMESIZE_ADJUSTOR] in frame size %d is not match to config size %d!", 2, in_frame_size, port->inout_size);
    }

    /*if ((port->type == FRAMESIZE_ADJUSTOR_FUNCTION_INTERLEAVED) && (channel_number != 2)) {
        DSP_MW_LOG_E("[FRAMESIZE_ADJUSTOR] function type: interleaved but channel number is %d!", 1, channel_number);
        AUDIO_ASSERT(FALSE);
        return true;
    }*/

    switch (port->mode) {
        case FRAMESIZE_ADJUSTOR_MODE_FIXED_INOUT_LENGTH:
        case FRAMESIZE_ADJUSTOR_MODE_MULTI_BUFFERS:
        case FRAMESIZE_ADJUSTOR_MODE_CHANGE_INOUT_LENGTH:;

            CusFunc_process framesize_adjustor_Fun = port->fun_process;
            void* working_buf = port->MemPtr;
            if (port->type == FRAMESIZE_ADJUSTOR_FUNCTION_DEINTERLEAVED) {
                /* Summary:
                    Step1: write data from inout buffer to internal input circular buffer(deinterleaved). (Should always have enough space in internal input circular buffer.)

                    Step2: while internal input circular buffer > processing data size, do processing, or skip this step.
                             a. write data from input circular buffer(deinterleaved) to input temp buffer(deinterleaved).
                             b. if enough, do process. (customer's function)
                             c. write data from output temp buffer(deinterleaved) to internal output circular buffer(deinterleaved). (Should always have enought space in internal output circular buffer)
                    Step3: read data from internal output circular buffer to inout buffer. (Should always have enought data in internal output circular buffer)
                 */

                 /* copy data into buffer */
                for (i = 0; (uint32_t)i < channel_number; i++) {
                    channel = port->channel + i;

                    channel_buf = &(channel->in);
                    buf = (uint8_t *)stream_function_get_inout_buffer(para, i + 1);

                    if (in_frame_size > (channel_buf->buf_size - channel_buf->data_size)) {
                        DSP_MW_LOG_E("[FRAMESIZE_ADJUSTOR][0][in] port 0x%x not enough space, %d, %d", 3, port, in_frame_size, (channel_buf->buf_size - channel_buf->data_size));
                        AUDIO_ASSERT(FALSE);
                    }
#ifdef AIR_AUDIO_DUMP_ENABLE
                    LOG_AUDIO_DUMP(buf, in_frame_size, AUDIO_FRAMESIZE_ADJUSTOR_IN_CH1+i);
#endif
                    /* Step1: write data from inout buffer to internal input circular buffer(deinterleaved). (Should always have enough space in internal input circular buffer.)*/
                    framesize_adjustor_write_data_to_channel_buf(channel_buf, buf, in_frame_size);
                    //DSP_MW_LOG_I("[FRAMESIZE_ADJUSTOR][0][in] port 0x%x write data to c buffer, %d, remain %d", 3, port, in_frame_size, channel_buf->buf_size );

                    /* Step2: if internal input circular buffer > processing data szie, do processing, or skip this step.*/
                    while (channel->in.data_size >= process_frame_size) {
                        channel_buf = &(channel->in);
                        /* a. read data from input circular buffer to input temp buffer.*/
                        framesize_adjustor_read_data_from_channel_buf(channel_buf, channel_buf->temp,process_frame_size);

                        /* b. do process. (customer's function)*/
#ifdef AIR_AUDIO_DUMP_ENABLE
                        LOG_AUDIO_DUMP(channel->in.temp, port->process_size, AUDIO_FRAMESIZE_ADJUSTOR_CUS_IN_CH1+i);
#endif
                        framesize_adjustor_Fun(channel->in.temp, channel->out.temp, port->process_size, working_buf);
#ifdef AIR_AUDIO_DUMP_ENABLE
                        LOG_AUDIO_DUMP(channel->out.temp, port->process_size, AUDIO_FRAMESIZE_ADJUSTOR_CUS_OUT_CH1+i);
#endif

                        /*c. write data from output temp buffer to internal output circular buffer. (Should always have enought space in internal output circular buffer)*/
                        channel_buf = &(channel->out);
                        if (process_frame_size > (channel_buf->buf_size - channel_buf->data_size)) {
                            DSP_MW_LOG_E("[FRAMESIZE_ADJUSTOR][0][out][1] port 0x%x not enough space, %d, %d", 3, port, process_frame_size, (channel_buf->buf_size - channel_buf->data_size));
                            AUDIO_ASSERT(FALSE);
                        }
                        framesize_adjustor_write_data_to_channel_buf(channel_buf, channel_buf->temp, process_frame_size);
                    }

                    /* Step3: read data from internal output circular buffer to inout buffer. (Should always have enought data in internal output circular buffer) */
                    channel_buf = &(channel->out);
                    if (port->mode == FRAMESIZE_ADJUSTOR_MODE_CHANGE_INOUT_LENGTH ){

                        if (channel_buf->data_size < process_frame_size) {
                            out_size = 0;
                        }
                        else {
                            out_size = process_frame_size;
                            framesize_adjustor_read_data_from_channel_buf(channel_buf, buf, process_frame_size);
#ifdef AIR_AUDIO_DUMP_ENABLE
                            LOG_AUDIO_DUMP(buf, process_frame_size, AUDIO_FRAMESIZE_ADJUSTOR_OUT_CH1+i);
#endif
                        }
                    }
                    else
                    {
                        if (channel_buf->data_size < port->inout_size) {
                            //DSP_MW_LOG_W("[FRAMESIZE_ADJUSTOR][0][out][2] port 0x%x not enough data, %d < %d, remain space: %d", 4, port, channel_buf->data_size, port->inout_size, (channel_buf->buf_size - channel_buf->data_size));
                            out_size = 0;
                            //AUDIO_ASSERT(FALSE);
                        } else {
                            framesize_adjustor_read_data_from_channel_buf(channel_buf, buf, port->inout_size);
                            //DSP_MW_LOG_I("[FRAMESIZE_ADJUSTOR][0][out][2] port 0x%x is enough data, output: %d, remain data: %d", 3, port, port->inout_size, channel_buf->data_size);
#ifdef AIR_AUDIO_DUMP_ENABLE
                            LOG_AUDIO_DUMP(buf, port->inout_size, AUDIO_FRAMESIZE_ADJUSTOR_OUT_CH1+i);
#endif
                        }
                    }

                }
            } else if (port->type == FRAMESIZE_ADJUSTOR_FUNCTION_INTERLEAVED) {
                /* Summary:
                    Step1: write stereo channel data from inout buffer to internal input circular buffer(deinterleaved). (Should always have enough space in internal input circular buffer.)

                    Step2: while internal input circular buffer > processing data size, do processing, or skip this step.
                             a. write data from input circular buffer(deinterleaved) to input temp buffer(deinterleaved).
                             b. if enough, do process. (customer's function)
                             c. write data from output temp buffer(deinterleaved) to internal output circular buffer(deinterleaved). (Should always have enought space in internal output circular buffer)
                    Step3: read data from internal output circular buffer to inout buffer. (Should always have enought data in internal output circular buffer)
                 */
                /* copy stereo data into buffer */
                for (i = 0; (uint32_t)i < channel_number; i++) {
                    channel = port->channel + i;
                    channel_buf = &(channel->in);
                    buf = (uint8_t *)stream_function_get_inout_buffer(para, i + 1);
                    if (in_frame_size > (channel_buf->buf_size - channel_buf->data_size)) {
                        DSP_MW_LOG_E("[FRAMESIZE_ADJUSTOR][1][in] port 0x%x not enough space, %d > %d", 3, port, in_frame_size, (channel_buf->buf_size - channel_buf->data_size));
                        AUDIO_ASSERT(FALSE);
                    }
                    /* Step1: write stereo channel data from inout buffer to internal input circular buffer(deinterleaved). (Should always have enough space in internal input circular buffer.)*/
#ifdef AIR_AUDIO_DUMP_ENABLE
                    LOG_AUDIO_DUMP(buf, in_frame_size, AUDIO_FRAMESIZE_ADJUSTOR_IN_CH1+i);
#endif
                    framesize_adjustor_write_data_to_channel_buf(channel_buf, buf, in_frame_size);
                }

                channel = port->channel;
                /* Step2: if internal input circular buffer >= processing data size, do processing, or skip this step.*/
                while (channel->in.data_size >= process_frame_size) {
                    channel_buf = &(channel->in);
                    /* a. read data from L/R input circular buffer to input temp buffer(interleaved).*/
                    framesize_adjustor_read_data_from_channel_buf_v2(port);

                    /* b. do process. (customer's function)*/
#ifdef AIR_AUDIO_DUMP_ENABLE
                    LOG_AUDIO_DUMP(channel->in.temp, port->process_size*2, AUDIO_FRAMESIZE_ADJUSTOR_CUS_IN_CH1);
#endif
                    framesize_adjustor_Fun(channel->in.temp, channel->out.temp, port->process_size*2, working_buf);
#ifdef AIR_AUDIO_DUMP_ENABLE
                    LOG_AUDIO_DUMP(channel->out.temp, port->process_size*2, AUDIO_FRAMESIZE_ADJUSTOR_CUS_OUT_CH1);
#endif
                    /*c. write data from output temp buffer(interleaved) to internal output circular buffer(deinterleaved). (Should always have enought space in internal output circular buffer)*/
                    channel_buf = &(channel->out);
                    if (process_frame_size > (channel_buf->buf_size - channel_buf->data_size)) {
                        DSP_MW_LOG_E("[FRAMESIZE_ADJUSTOR][1][out][1] port 0x%x not enough space, %d > %d", 3, port, process_frame_size, (channel_buf->buf_size - channel_buf->data_size));
                        AUDIO_ASSERT(FALSE);
                    }
                    framesize_adjustor_write_data_to_channel_buf_v2(port);
                }
                /* Step3: read data from internal output circular buffer to inout buffer. (Should always have enought data in internal output circular buffer*/
                for (i = 0; (uint32_t)i < channel_number; i++) {
                    channel = port->channel + i;
                    channel_buf = &(channel->out);
                    buf = (uint8_t *)stream_function_get_inout_buffer(para, i + 1);

                    if (port->mode == FRAMESIZE_ADJUSTOR_MODE_CHANGE_INOUT_LENGTH) {

                        if (channel_buf->data_size < process_frame_size) {
                            out_size = 0;
                        }
                        else {
                            out_size = process_frame_size;
                            framesize_adjustor_read_data_from_channel_buf(channel_buf, buf, process_frame_size);
#ifdef AIR_AUDIO_DUMP_ENABLE
                            LOG_AUDIO_DUMP(buf, process_frame_size, AUDIO_FRAMESIZE_ADJUSTOR_OUT_CH1+i);
#endif
                        }
                    }
                    else
                    {
                        if (channel_buf->data_size < port->inout_size) {
                            //DSP_MW_LOG_W("[FRAMESIZE_ADJUSTOR][1][out][2] port 0x%x not enough data, %d < %d, remain space: %d", 4, port, channel_buf->data_size , port->inout_size, (channel_buf->buf_size - channel_buf->data_size));
                            out_size = 0;
                            //AUDIO_ASSERT(FALSE);
                        } else {
                            framesize_adjustor_read_data_from_channel_buf(channel_buf, buf, port->inout_size);
#ifdef AIR_AUDIO_DUMP_ENABLE
                            LOG_AUDIO_DUMP(buf, port->inout_size, AUDIO_FRAMESIZE_ADJUSTOR_OUT_CH1+i);
#endif

                        }
                    }
                }
            } else {
                DSP_MW_LOG_E("[FRAMESIZE_ADJUSTOR] function type is not set!", 0);
                AUDIO_ASSERT(FALSE);
            }
            if (port->mode == FRAMESIZE_ADJUSTOR_MODE_CHANGE_INOUT_LENGTH)
            {
                bool result = FALSE;
                result = stream_function_modify_output_size(para, out_size);
                if (!result) {
                    DSP_MW_LOG_E("[FRAMESIZE_ADJUSTOR] Buffer isn't enough, please check in/out buffer malloc size %d %d !", 2, out_size,((DSP_ENTRY_PARA_PTR)para)->out_malloc_size);
                }
                if (out_size == 0) {
                    return false;
                }
            }
            if (out_size == 0) {
                stream_function_modify_output_size(para, out_size);
                return false;
            }
            break;
        default:
            AUDIO_ASSERT(FALSE);
            break;
    }

    return false;
}

void framesize_adjustor_sample_Fun1_process(uint8_t* in_buf, uint8_t* out_buf, uint32_t size, void* working_buf) {
    //DSP_MW_LOG_I("[FRAMESIZE_ADJUSTOR] Fun1_process",0);
    UNUSED(working_buf);
    memcpy(out_buf, in_buf, size);
    return;
}

void framesize_adjustor_sample_Fun2_process(uint8_t* in_buf, uint8_t* out_buf, uint32_t size, void* working_buf) {
    //DSP_MW_LOG_I("[FRAMESIZE_ADJUSTOR] Fun2_process",0);
    UNUSED(working_buf);
    memcpy(out_buf, in_buf, size);
    return;
}

bool framesize_adjustor_sample_Fun1_initialize(void* para, void* working_buf) {
    UNUSED(para);
    UNUSED(working_buf);
    DSP_MW_LOG_I("[FRAMESIZE_ADJUSTOR] Fun1_initialize",0);
    return FALSE;
}

bool framesize_adjustor_sample_Fun2_initialize(void* para, void* working_buf) {
    UNUSED(para);
    UNUSED(working_buf);
    DSP_MW_LOG_I("[FRAMESIZE_ADJUSTOR] Fun2_initialize",0);
    return FALSE;
}


