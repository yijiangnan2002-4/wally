/* Copyright Statement:
 *
 * (C) 2021  Airoha Technology Corp. All rights reserved.
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

#if defined(AIR_VOLUME_ESTIMATOR_ENABLE)

/* Includes ------------------------------------------------------------------*/
#include "dsp_feature_interface.h"
#include "dsp_audio_process.h"
#include "volume_estimator_interface.h"
#include "volume_estimator_portable.h"
#include "dsp_dump.h"
#include "memory_attribute.h"
#include "preloader_pisplit.h"
#ifdef AIR_AUDIO_VOLUME_MONITOR_ENABLE
#include "dsp_audio_msg.h"
#include "dtm.h"
#include "hal_audio_cm4_dsp_message.h"
#endif

/* Private define ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static volume_estimator_port_t volume_estimator_port[VOLUME_ESTIMATOR_PORT_MAX];
/* Public variables ----------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
static int32_t volume_estimator_covert_sample_rate_to_mode(uint32_t sample_rate)
{
    int32_t mode = 0;

    switch (sample_rate)
    {
        case 16000:
            mode = 0;
            break;

        case 32000:
            mode = 1;
            break;

        case 48000:
            mode = 2;
            break;

        case 96000:
            mode = 3;
            break;

        case 192000:
            mode = 4;
            break;

        case 8000:
            mode = 5;
            break;

        case 24000:
            mode = 6;
            break;

        default:
            AUDIO_ASSERT(0);
            break;
    }

    return mode;
}

/* Public functions ----------------------------------------------------------*/
ATTR_TEXT_IN_RAM_FOR_MASK_IRQ volume_estimator_port_t *volume_estimator_get_port(void *owner)
{
    int32_t i;
    volume_estimator_port_t *port = NULL;
    uint32_t saved_mask;

    /* Find out a port for this owner */
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    for (i = VOLUME_ESTIMATOR_PORT_MAX - 1; i >= 0; i--) {
        /* Check if there is unused port */
        if (volume_estimator_port[i].owner == NULL) {
            port = &volume_estimator_port[i];
            continue;
        }

        /* Check if this owner has already owned a afc */
        if (volume_estimator_port[i].owner == owner) {
            port = &volume_estimator_port[i];
            break;
        }
    }
    hal_nvic_restore_interrupt_mask(saved_mask);

    if (port == NULL) {
        AUDIO_ASSERT(0 && "[VOL] Port not enough!");
        return port;
    }

    port->owner = owner;

    return port;
}

volume_estimator_status_t volume_estimator_init(volume_estimator_port_t *port, volume_estimator_config_t *config)
{
    /* check port */
    if (port == NULL) {
        AUDIO_ASSERT(0 && "[VOL] Port is NULL!");
        return VOLUME_ESTIMATOR_STATUS_ERROR;
    }

    /* check status */
    if (port->status != VOLUME_ESTIMATOR_PORT_STATUS_DEINIT) {
        DSP_MW_LOG_E("[VOL] status is error, %d!", 1, port->status);
        AUDIO_ASSERT(0);
        return VOLUME_ESTIMATOR_STATUS_ERROR;
    }

    /* config setting */
    port->channel_num   = config->channel_num;
    /*if (config->resolution != RESOLUTION_32BIT) {
        DSP_MW_LOG_E("[VOL] resolution is not supported, %d!", 1, config->resolution);
        AUDIO_ASSERT(0);
        return VOLUME_ESTIMATOR_STATUS_ERROR;
    }*/
    port->resolution    = config->resolution;
    if (config->mode >= VOLUME_ESTIMATOR_MODE_MAX) {
        DSP_MW_LOG_E("[VOL] mode is not supported, %d!", 1, config->mode);
        AUDIO_ASSERT(0);
        return VOLUME_ESTIMATOR_STATUS_ERROR;
    }
    port->mode          = config->mode;
    port->sample_rate   = config->sample_rate;
    if (config->frame_size != (config->sample_rate/1000*sizeof(int32_t)*5/2))
    {
        DSP_MW_LOG_E("[VOL] frame size is not supported for 2.5ms interval, %d, %d!", 2, config->frame_size, config->sample_rate);
        //AUDIO_ASSERT(0);
        //return VOLUME_ESTIMATOR_STATUS_ERROR;
    }
    port->frame_size    = config->frame_size;
    if (config->nvkey_para == NULL) {
        AUDIO_ASSERT(0 && "[VOL] nvkey para is NULL!");
        return VOLUME_ESTIMATOR_STATUS_ERROR;
    }
    port->nvkey_para    = config->nvkey_para;
    port->status        = VOLUME_ESTIMATOR_PORT_STATUS_INIT;
#if defined(AIR_AUDIO_VOLUME_MONITOR_ENABLE)
    port->internal_buffer      = config->internal_buffer;
    port->internal_buffer_size = config->internal_buffer_size;
    if (config->internal_buffer_size / config->channel_num % config->frame_size) {
        AUDIO_ASSERT(0 && "[VOL] internal buffer size must be multiple of 2.5ms!");
    }
    DSP_MW_LOG_I("[VOL] interbuf 0x%x, size %d", 2, port->internal_buffer, port->internal_buffer_size);
    port->internal_buffer_wo   = 0;
#endif
    return VOLUME_ESTIMATOR_STATUS_OK;
}

volume_estimator_status_t volume_estimator_deinit(volume_estimator_port_t *port)
{
    /* check port */
    if (port == NULL) {
        DSP_MW_LOG_E("[VOL] Port is NULL!", 0);
        return VOLUME_ESTIMATOR_STATUS_ERROR;
    }

    /* check status */
    if ((port->status != VOLUME_ESTIMATOR_PORT_STATUS_INIT) && (port->status != VOLUME_ESTIMATOR_PORT_STATUS_RUNNING)) {
        DSP_MW_LOG_E("[VOL] status is error, %d!", 1, port->status);
        return VOLUME_ESTIMATOR_STATUS_ERROR;
    }

    /* config setting */
    port->status        = VOLUME_ESTIMATOR_PORT_STATUS_DEINIT;
    port->channel_num   = 0;
    port->frame_size    = 0;
    port->resolution    = 0;
    port->mode          = 0;
    port->sample_rate   = 0;
    port->nvkey_para    = NULL;

    /* free working memory */
    if (port->handle) {
        preloader_pisplit_free_memory(port->handle);
    }
    port->handle = NULL;
    port->owner = NULL;

    return VOLUME_ESTIMATOR_STATUS_OK;
}

volume_estimator_status_t volume_estimator_process(volume_estimator_port_t *port, void *data_buf, uint32_t data_size, int32_t *out_db)
{
    int mem_size;
    int out_data;
    uint32_t i;
    void *handle;

    /* check parameters */
    if ((port == NULL) || (data_buf == NULL)) {
        DSP_MW_LOG_E("[VOL] Port or data is NULL!", 0);
        return VOLUME_ESTIMATOR_STATUS_ERROR;
    }
    if (data_size != port->frame_size*port->channel_num) {
        DSP_MW_LOG_E("[VOL] data size is not right, %d, %d!", 2, port->frame_size*port->channel_num, data_size);
        return VOLUME_ESTIMATOR_STATUS_ERROR;
    }

    if (port->status == VOLUME_ESTIMATOR_PORT_STATUS_INIT) {
        /* get working memory */
        mem_size = get_chat_vol_memsize();
        port->handle = preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, port->channel_num*mem_size);
        if (port->handle == NULL) {
            AUDIO_ASSERT(0);
        }
        port->mem_size = mem_size;

        /* feature init */
        for (i = 0; i < port->channel_num; i++)
        {
            handle = (void *)((uint8_t *)(port->handle) + i*port->mem_size);
            chat_vol_init(handle, port->nvkey_para, volume_estimator_covert_sample_rate_to_mode(port->sample_rate), 1);
            DSP_MW_LOG_I("[VOL] vol init done, 0x%x, %d!", 2, handle, volume_estimator_covert_sample_rate_to_mode(port->sample_rate));
        }

        port->status = VOLUME_ESTIMATOR_PORT_STATUS_RUNNING;
    }

    for (i = 0; i < port->channel_num; i++)
    {
        /* get handle */
        handle = (void *)((uint8_t *)(port->handle) + i*port->mem_size);

        /* process data */
        switch (port->mode) {
            case VOLUME_ESTIMATOR_CHAT_NORMAL_MODE:
                out_data = chat_vol_prcs((int32_t *)((uint32_t)data_buf+port->frame_size*i), handle);
                out_db[i] = out_data * 100 / 256;
                break;

            case VOLUME_ESTIMATOR_CHAT_INSTANT_MODE:
                out_data = chat_vol_prcs_instant((int32_t *)((uint32_t)data_buf+port->frame_size*i), handle);
                out_db[i] = out_data * 100 / 256;
                break;

            default:
                AUDIO_ASSERT(0);
                break;
        }
    }

    return VOLUME_ESTIMATOR_STATUS_OK;
}

/* ------------------------------------------------------------------------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------------------------------------------------------------------ */
/* -----------------------------------------------------------  Audio Spectrum Meter  ------------------------------------------------------------- */
/* ------------------------------------------------------------------------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------------------------------------------------------------------ */
#if defined(AIR_AUDIO_VOLUME_MONITOR_ENABLE)
static volume_estimator_port_t *volume_estimator_get_the_port(DSP_STREAMING_PARA_PTR stream_ptr)
{
    volume_estimator_port_t *port = NULL;
    /* get the port form stream */
    int32_t i = 0;
    for (i = VOLUME_ESTIMATOR_PORT_MAX - 1; i >= 0; i--) {
        /* Check if this source or sink has already owned a volume_estimator */
        if ((volume_estimator_port[i].owner == stream_ptr->source) ||
            (volume_estimator_port[i].owner == stream_ptr->sink)) {
            port = &volume_estimator_port[i];
            break;
        }
    }
    if (port == NULL) {
        DSP_MW_LOG_E("[Audio Spectrum Meter] Port is not enough!", 0);
        AUDIO_ASSERT(FALSE);
        return NULL;
    }
    return port;
}

volume_estimator_port_t *volume_estimator_get_the_port_by_scenario_type(audio_scenario_type_t type)
{
    volume_estimator_port_t *port = NULL;
    /* get the port form stream */
    int32_t i = 0;
    for (i = VOLUME_ESTIMATOR_PORT_MAX - 1; i >= 0; i--) {
        if (volume_estimator_port[i].owner) {
            SOURCE source = (SOURCE) volume_estimator_port[i].owner;
            SINK sink   = (SINK) volume_estimator_port[i].owner;
            if ((source->scenario_type == type) || (sink->scenario_type == type)) {
                port = &volume_estimator_port[i];
                break;
            }
        }
    }
    if (port == NULL) {
        DSP_MW_LOG_E("[Audio Spectrum Meter] Port is not found by type %d", 1, type);
        // AUDIO_ASSERT(FALSE);
        return NULL;
    }
    return port;
}

/**
 * @brief This function is used to initialize the audio spectrum meter run-time environment.
 *
 * @param para is the input parameters.
 * @return true means there is a error.
 * @return false means there is no error.
 */
bool stream_function_audio_spectrum_meter_initialize(void *para)
{
    DSP_STREAMING_PARA_PTR  stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    volume_estimator_port_t *port      = NULL;
    uint8_t                 ch         = 0;
    /* get the port form stream */
    port = volume_estimator_get_the_port(stream_ptr);
    /* init the working buffer */
    if (port->status == VOLUME_ESTIMATOR_PORT_STATUS_INIT) {
        /* get working memory */
        uint32_t mem_size = get_chat_vol_memsize();
        port->handle = preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, port->channel_num * mem_size);
        if (port->handle == NULL) {
            AUDIO_ASSERT(0);
        }
        port->mem_size = mem_size;
        if (port->channel_num >= 2) {
            DSP_MW_LOG_W("[Audio Spectrum Meter] channel is %d over 2, type %d", 2, port->channel_num, stream_ptr->source->scenario_type);
            ch = 2;
        } else if (port->channel_num == 0) {
            AUDIO_ASSERT(0 && "[Audio Spectrum Meter] port channel is 0!");
        } else {
            ch = port->channel_num;
        }

        /* feature init */
        for (uint32_t i = 0; i < ch; i++) {
            void *handle = (void *)((uint8_t *)(port->handle) + i * port->mem_size);
            chat_vol_init(handle, port->nvkey_para, volume_estimator_covert_sample_rate_to_mode(port->sample_rate), 1); // mono mode
            DSP_MW_LOG_I("[Audio Spectrum Meter] vol init, ch %d 0x%x! mode %d", 3, i, handle, volume_estimator_covert_sample_rate_to_mode(port->sample_rate));
        }
        port->status = VOLUME_ESTIMATOR_PORT_STATUS_RUNNING;
        DSP_MW_LOG_I("[Audio Spectrum Meter] vol init done, ch %d mem_size/ch %d", 2, port->channel_num, port->mem_size);
    }
    return false;
}

/**
 * @brief This function is used to process data and calculate the volume result.
 *
 * @param para is the input parameters.
 * @return true means there is a error.
 * @return false means there is no error.
 */
bool stream_function_audio_spectrum_meter_process(void *para)
{
    DSP_STREAMING_PARA_PTR  stream_ptr                    = DSP_STREAMING_GET_FROM_PRAR(para);
    uint32_t                in_frame_size                 = stream_function_get_output_size(para);
    uint8_t                 ch                            = 0; //stream_function_get_channel_number(para);
    uint32_t                process_frame_size            = 0;
    uint32_t                process_times                 = 0;
    int32_t                 out_data                      = 0;
    int32_t                 *addr                         = NULL;
    bool                    is_internal_buffer_data_ready = true;
    volume_estimator_port_t *port                         = volume_estimator_get_the_port(stream_ptr);
    uint32_t internal_buffer_size_1ch = port->internal_buffer_size / port->channel_num;
    if (port->internal_buffer) {
        if (port->internal_buffer_wo + in_frame_size < internal_buffer_size_1ch) {
            is_internal_buffer_data_ready = false;
        }
        for (uint8_t ii = 0; ii < port->channel_num; ii ++) {
            memcpy(port->internal_buffer + port->internal_buffer_wo + ii * internal_buffer_size_1ch, stream_function_get_inout_buffer(para, ii + 1), in_frame_size);
            #if AUDIO_VOLUME_MONITOR_DEBUG_ENABLE
            DSP_MW_LOG_D("TEST 0x%x, wo %d ii %d %d", 4, port->internal_buffer, port->internal_buffer_wo, ii, in_frame_size);
            #endif
        }
        port->internal_buffer_wo += in_frame_size;
    }
    if (port->node && is_internal_buffer_data_ready) {
        ch                 = port->channel_num;
        process_frame_size = port->frame_size;
        process_times      = port->internal_buffer ? (internal_buffer_size_1ch / process_frame_size) : (in_frame_size / process_frame_size);
        // LOG_AUDIO_DUMP(stream_function_get_inout_buffer(para, 1), in_frame_size, AUDIO_BT_SRC_DONGLE_UL_USB_OUT);
        if (ch != port->node->ch) {
            AUDIO_ASSERT(0 && "ch is not right");
        }
        /* remapping non-cacheable addr */
        addr = (int32_t *)hal_memview_cm4_to_dsp0((uint32_t)port->node->volume_data);
        // if (port->channel_num != ch) {
        //     AUDIO_ASSERT(0 && "[Audio Spectrum Meter] channel number is not align!");
        // }
        // if (in_frame_size % process_frame_size) {
        //     AUDIO_ASSERT(0 && "[Audio Spectrum Meter] channel input data size should be an integer multiple of 2.5ms!");
        // }
        // DSP_MW_LOG_I("TEST in_frame_size %d %d process_frame_size %d, mode %d", 4, in_frame_size, port->channel_num, port->frame_size, port->mode);
        for (uint32_t i = 0; i < ch; i++) {
            uint8_t *input_buffer = port->internal_buffer ? (port->internal_buffer + port->internal_buffer_size * i) : stream_function_get_inout_buffer(para, i + 1);
            LOG_AUDIO_DUMP(input_buffer, process_times * process_frame_size, AUDIO_VOLUME_MONITOR_INPUT_CH_0 + i);
            /* get handle */
            void *handle = (void *)((uint8_t *)(port->handle) + i * port->mem_size);
            for (uint32_t j = 0; j < process_times; j ++) {
                /* process data each time : 2.5ms */
                switch (port->mode) {
                    case VOLUME_ESTIMATOR_CHAT_NORMAL_MODE:
                        out_data = chat_vol_prcs((int32_t *)((uint32_t)input_buffer), handle);
                        out_data = out_data * 100 / 256;
                        if (port->count_in_frame == 7) {
                            *(addr + port->frame_count + i * port->node->volume_len) = out_data;
                        }
                        if (i == (ch - 1)) {
                            port->count_in_frame ++;
                            if (port->count_in_frame == 8) { // update each 20ms， 8 * 2.5ms
                                port->count_in_frame = 0;
                                port->frame_count ++;
                                if (port->frame_count == port->node->volume_len) {
                                    port->frame_count = 0;
                                    port->node->update_count ++;
                                    if (!port->node->update_flag) {
                                        port->node->update_flag = true;
                                    } else {
                                        #if AUDIO_VOLUME_MONITOR_DEBUG_ENABLE
                                        DSP_MW_LOG_D("[Audio Spectrum Meter] no body get the data in time!", 0);
                                        #endif
                                    }
                                    /* CCNI notify mcu mode */
                                    if (port->node->cb) {
                                        uint32_t ack_msg = 0;
                                        ack_msg  = port->node->type;
                                        ack_msg |= ((MSG_DSP2MCU_AUDIO_VOLUME_MONITOR_START | 0x8000) << 16);
                                        DTM_enqueue(DTM_EVENT_ID_AUDIO_VOLUME_MONITOR_NOTIFY, ack_msg, false);
                                    }
                                    #if AUDIO_VOLUME_MONITOR_DEBUG_ENABLE
                                        DSP_MW_LOG_I("[Audio Spectrum Meter] ch instant %d res %d %d %d %d %d %d %d %d %d ", 10,
                                            i,
                                            *(addr),
                                            *(addr + 1),
                                            *(addr + 2),
                                            *(addr + 3),
                                            *(addr + 4),
                                            *(addr + 5),
                                            *(addr + 6),
                                            *(addr + 7),
                                            port->frame_count
                                            );
                                    #endif /* AUDIO_VOLUME_MONITOR_DEBUG_ENABLE */
                                }
                            }
                        }
                        #if AUDIO_VOLUME_MONITOR_DEBUG_ENABLE
                            DSP_MW_LOG_I("[Audio Spectrum Meter] normal ch %d process times %d result %d %d %d %d %d", 7,
                                i,
                                j,
                                out_data,
                                port->count_in_frame,
                                port->node->update_count,
                                port->node->update_flag,
                                port->internal_buffer_wo
                                );
                        #endif /* AUDIO_VOLUME_MONITOR_DEBUG_ENABLE */
                        break;

                    case VOLUME_ESTIMATOR_CHAT_INSTANT_MODE:
                        out_data = chat_vol_prcs_instant((int32_t *)((uint32_t)input_buffer), handle);
                        out_data = out_data * 100 / 256;
                        if (port->count_in_frame == 7) {
                            *(addr + port->frame_count + i * port->node->volume_len) = out_data;
                        }
                        if (i == (ch - 1)) {
                            port->count_in_frame ++;
                            if (port->count_in_frame == 8) { // update each 20ms， 8 * 2.5ms
                                port->count_in_frame = 0;
                                port->frame_count ++;
                                if (port->frame_count == port->node->volume_len) {
                                    port->frame_count = 0;
                                    port->node->update_count ++;
                                    if (!port->node->update_flag) {
                                        port->node->update_flag = true;
                                    } else {
                                        #if AUDIO_VOLUME_MONITOR_DEBUG_ENABLE
                                        DSP_MW_LOG_D("[Audio Spectrum Meter] no body get the data in time!", 0);
                                        #endif
                                    }
                                    /* CCNI notify mcu mode */
                                    if (port->node->cb) {
                                        uint32_t ack_msg = 0;
                                        ack_msg  = port->node->type;
                                        ack_msg |= ((MSG_DSP2MCU_AUDIO_VOLUME_MONITOR_START | 0x8000) << 16);
                                        DTM_enqueue(DTM_EVENT_ID_AUDIO_VOLUME_MONITOR_NOTIFY, ack_msg, false);
                                    }
                                    #if AUDIO_VOLUME_MONITOR_DEBUG_ENABLE
                                        DSP_MW_LOG_I("[Audio Spectrum Meter] ch instant %d res %d %d %d %d %d %d %d %d %d ", 10,
                                            i,
                                            *(addr),
                                            *(addr + 1),
                                            *(addr + 2),
                                            *(addr + 3),
                                            *(addr + 4),
                                            *(addr + 5),
                                            *(addr + 6),
                                            *(addr + 7),
                                            port->frame_count
                                            );
                                    #endif /* AUDIO_VOLUME_MONITOR_DEBUG_ENABLE */
                                }
                            }
                        }
                        #if AUDIO_VOLUME_MONITOR_DEBUG_ENABLE
                            DSP_MW_LOG_I("[Audio Spectrum Meter] instant ch %d process times %d result %d %d %d %d %d", 7,
                                i,
                                j,
                                out_data,
                                port->count_in_frame,
                                port->node->update_count,
                                port->node->update_flag,
                                port->internal_buffer_wo
                                );
                        #endif /* AUDIO_VOLUME_MONITOR_DEBUG_ENABLE */
                        break;

                    default:
                        AUDIO_ASSERT(0);
                        break;
                }
                input_buffer += process_frame_size;
            }
        }
    } else {
        /* TBD */
    }
    /* reset internal buffer info */
    if (is_internal_buffer_data_ready) {
        port->internal_buffer_wo = 0;
        // memset(port->internal_buffer, 0, port->internal_buffer_size);
    }
    return false;
}
#endif /* AIR_AUDIO_VOLUME_MONITOR_ENABLE */

#endif /* AIR_VOLUME_ESTIMATOR_ENABLE */
