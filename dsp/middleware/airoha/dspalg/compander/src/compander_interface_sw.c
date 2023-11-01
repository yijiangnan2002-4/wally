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

#if defined(AIR_SOFTWARE_DRC_ENABLE)

/* Includes ------------------------------------------------------------------*/
#include "dsp_feature_interface.h"
#include "dsp_audio_process.h"
#include "compander_interface_sw.h"
#include "dsp_dump.h"
#include "dsp_memory.h"
#include "memory_attribute.h"
#include "preloader_pisplit.h"
#include "dsp_para_cpd.h"
#ifdef MTK_BT_A2DP_CPD_USE_PIC
#include "cpd_portable.h"
#include "dsp_dump.h"
#endif /* MTK_BT_A2DP_CPD_USE_PIC */

/* Private define ------------------------------------------------------------*/
#define SW_COMPANDER_DEBUG_LOG                0
#define SW_COMPANDER_DEBUG_DUMP               0

/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static sw_compander_port_t sw_compander_port[SW_COMPANDER_PORT_MAX];

/* Public variables ----------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
static sw_compander_port_t *sw_compander_find_out_port(DSP_STREAMING_PARA_PTR stream_ptr)
{
    sw_compander_port_t *port = NULL;
    uint32_t i;

    /* Find out the sw src port of this stream */
    for (i = SW_COMPANDER_PORT_MAX - 1; i >= 0; i--) {
        /* Check if this source or sink has already owned a sw src */
        if ((sw_compander_port[i].owner == stream_ptr->source) ||
            (sw_compander_port[i].owner == stream_ptr->sink)) {
            port = &sw_compander_port[i];
            break;
        }
    }
    if (port == NULL) {
        DSP_MW_LOG_E("[SW_COMPANDER] Port is not found!", 0);
        AUDIO_ASSERT(FALSE);
    }

    return port;
}

static int32_t sw_compander_get_band_sw(uint32_t sample_rate)
{
    int32_t band_sw = 0;

    switch (sample_rate) {
        case 48000:
            band_sw = 0;
            break;

        case 44100:
            band_sw = 1;
            break;

        case 96000:
            band_sw = 2;
            break;

        case 88200:
            band_sw = 3;
            break;

        default:
            DSP_MW_LOG_E("[SW_COMPANDER] sample rate is not found!, %u", 1, sample_rate);
            AUDIO_ASSERT(FALSE);
            break;
    }

    return band_sw;
}

static uint32_t sw_compander_fs_converter(stream_samplerate_t fs)
{
    switch (fs) {
        case FS_RATE_44_1K:
            return 44100;

        case FS_RATE_88_2K:
            return 44100;

        case FS_RATE_8K:
        case FS_RATE_16K:
        case FS_RATE_24K:
        case FS_RATE_32K:
        case FS_RATE_48K:
        case FS_RATE_96K:
            return fs * 1000;

        default:
            DSP_MW_LOG_E("[SW_COMPANDER] sample rate is not supported!", 0);
            AUDIO_ASSERT(FALSE);
            return fs;
    }
}

static int32_t sw_compander_vol_converter(int32_t gain)
{
    int32_t vol;

    /* TODO: covert Q27 format */
#ifndef MTK_BT_A2DP_CPD_USE_PIC
    vol = gain;
#else
    vol = gain;
#endif /* MTK_BT_A2DP_CPD_USE_PIC */

    return vol;
}

/* Public functions ----------------------------------------------------------*/
sw_compander_port_t *stream_function_sw_compander_get_port(void *owner)
{
    int32_t i;
    uint32_t saved_mask;
    sw_compander_port_t *port = NULL;

    /* Find out a port for this owner */
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    for (i = SW_COMPANDER_PORT_MAX - 1; i >= 0; i--) {
        /* Check if there is unused port */
        if (sw_compander_port[i].owner == NULL) {
            port = &sw_compander_port[i];
        }

        /* Check if this owner has already owned a sw src */
        if (sw_compander_port[i].owner == owner) {
            port = &sw_compander_port[i];
            break;
        }
    }
    hal_nvic_restore_interrupt_mask(saved_mask);

    if (port == NULL) {
        DSP_MW_LOG_E("[SW_COMPANDER] Port not enough!", 0);
        AUDIO_ASSERT(0);
        return port;
    }

    port->owner = owner;

    return port;
}

sw_compander_status_t stream_function_sw_compander_init(sw_compander_port_t *port, sw_compander_config_t *config)
{
    /* check port */
    if (port == NULL) {
        DSP_MW_LOG_E("[SW_COMPANDER] Port is NULL!", 0);
        AUDIO_ASSERT(FALSE);
    }

    /* Config SW Compander */
    port->status            = SW_COMPANDER_PORT_STATUS_INIT;
    port->mode              = config->mode;
    if (port->mode != SW_COMPANDER_AUDIO_MODE)
    {
        DSP_MW_LOG_E("Mode is not supported!, %d", 1, port->mode);
        AUDIO_ASSERT(FALSE);
    }
    port->channel_num       = config->channel_num;
    port->sample_rate       = config->sample_rate;
    port->frame_base        = config->frame_base;
    port->recovery_gain     = config->recovery_gain;
    port->vol_target        = sw_compander_vol_converter(config->vol_default_gain);
    port->nvkey_mem_ptr     = preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, sizeof(CPD_AU_NVKEY_STATE));
    if (port->nvkey_mem_ptr == NULL)
    {
        DSP_MW_LOG_E("NVKEY mem is NULL!", 0);
        AUDIO_ASSERT(FALSE);
    }
    if ((config->default_nvkey_mem != NULL) && (config->default_nvkey_id == 0))
    {
        memcpy(port->nvkey_mem_ptr, config->default_nvkey_mem, sizeof(CPD_AU_NVKEY_STATE));
    }
    else if ((config->default_nvkey_id != 0) && (config->default_nvkey_mem == NULL))
    {
        port->nvkey_id = config->default_nvkey_id;
        nvkey_read_full_key(port->nvkey_id, port->nvkey_mem_ptr, sizeof(CPD_AU_NVKEY_STATE));
    }
    else
    {
        AUDIO_ASSERT(FALSE);
    }
    port->work_mem_size = 0;
    port->work_instance_count = 0;

    return SW_COMPANDER_STATUS_OK;
}

sw_compander_status_t stream_function_sw_compander_deinit(sw_compander_port_t *port)
{
    /* check port */
    if (port == NULL) {
        DSP_MW_LOG_E("[SW_COMPANDER] Port is NULL!", 0);
        AUDIO_ASSERT(FALSE);
    }

    /* Reset SW Compander status */
    port->status            = SW_COMPANDER_PORT_STATUS_DEINIT;
    port->owner             = NULL;
    port->stream            = NULL;
    port->mode              = 0;
    port->voice_work_mode   = 0;
    port->channel_num       = 0;
    port->sample_rate       = 0;
    port->frame_base        = 0;
    port->recovery_gain     = 0;
    port->vol_target        = 0;
    port->nvkey_id = 0;
    preloader_pisplit_free_memory(port->nvkey_mem_ptr);
    port->nvkey_mem_ptr = NULL;
    if (port->work_mem_size != 0)
    {
        preloader_pisplit_free_memory(port->work_mem_ptr);
        port->work_mem_size = 0;
    }
    port->work_mem_ptr = NULL;
    port->work_instance_count = 0;

    return SW_COMPANDER_STATUS_OK;
}

bool stream_function_sw_compander_initialize(void *para)
{
    uint32_t i;
    sw_compander_port_t *port = NULL;
    DSP_STREAMING_PARA_PTR stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    uint32_t saved_mask;
    void *working_memory_ptr;

    /* Find out the sw compander port of this stream */
    port = sw_compander_find_out_port(stream_ptr);

    /* status check */
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (port->status == SW_COMPANDER_PORT_STATUS_INIT) {
        port->status = SW_COMPANDER_PORT_STATUS_RUNNING;
    } else if (port->status == SW_COMPANDER_PORT_STATUS_RUNNING) {
        hal_nvic_restore_interrupt_mask(saved_mask);
        return false;
    } else {
        DSP_MW_LOG_I("[SW_COMPANDER] error status:%d", 1, port->status);
        AUDIO_ASSERT(0);
        return true;
    }
    hal_nvic_restore_interrupt_mask(saved_mask);

    /* get working memory */
    port->work_instance_count = (port->channel_num+1)/2;
    port->work_mem_size = get_CPD_memsize(port->mode);
    port->work_mem_size = (port->work_mem_size+7)/8*8; // 8Byte aligned
    port->work_mem_ptr  = preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, port->work_instance_count*port->work_mem_size);

    /* compander init */
    if (port->mode == SW_COMPANDER_AUDIO_MODE)
    {
        ((CPD_AU_NVKEY_STATE *)(port->nvkey_mem_ptr))->band_sw = sw_compander_get_band_sw(port->sample_rate);
        for (i = 0; i < port->work_instance_count; i++)
        {
            working_memory_ptr = (void *)((uint32_t)(port->work_mem_ptr) + i * port->work_mem_size);
            if (port->frame_base == 120)
            {
                compander_AU_SetFrame120_init(working_memory_ptr);
            }
            else if (port->frame_base == 8)
            {
                compander_AU_SetFrame8_init(working_memory_ptr);
            }
            compander_AU_init(working_memory_ptr, port->nvkey_mem_ptr, port->recovery_gain, sw_compander_get_band_sw(port->sample_rate));
        }
    }
    else
    {
        DSP_MW_LOG_E("Mode is not supported!, %d", 1, port->mode);
        AUDIO_ASSERT(FALSE);
    }

    DSP_MW_LOG_I("[SW_COMPANDER] compander open successfully!, %u, %u, %u, %u, %u, %u, 0x%x, 0x%x, 0x%x, %u, %u, 0x%x, 0x%x", 13,
                port->mode,
                port->voice_work_mode,
                port->channel_num,
                port->sample_rate,
                port->frame_base,
                port->recovery_gain,
                port->vol_target,
                port->nvkey_id,
                port->nvkey_mem_ptr,
                port->work_instance_count,
                port->work_mem_size,
                port->work_mem_ptr,
                SVN_version());

    return false;
}

bool stream_function_sw_compander_process(void *para)
{
    uint32_t i;
    sw_compander_port_t *port = NULL;
    DSP_STREAMING_PARA_PTR stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    uint32_t in_sampling_rate = stream_function_get_samplingrate(para);
    uint32_t channel_number = stream_function_get_channel_number(para);
    uint32_t in_frame_size = stream_function_get_output_size(para);
    void *working_memory_ptr;
    int32_t *in_buf1;
    int32_t *in_buf2;
    int16_t channel_mode;

    /* Find out the sw compander port of this stream */
    port = sw_compander_find_out_port(stream_ptr);

    if (!in_frame_size) {
        return false;
    }

    /* check if the sampling_rate and channel number is matched */
    if (port->sample_rate != sw_compander_fs_converter(in_sampling_rate)) {
        DSP_MW_LOG_E("[SW_COMPANDER] input sampling rate is not right, %u, %u!", 2, port->sample_rate, sw_compander_fs_converter(in_sampling_rate));
        AUDIO_ASSERT(FALSE);
        return true;
    }
    // if (port->channel_num != channel_number) {
    //     DSP_MW_LOG_E("[SW_COMPANDER] channel number is not right, %u, %u!", 2, port->channel_num, channel_number);
    //     AUDIO_ASSERT(FALSE);
    //     return true;
    // }
    channel_number = port->channel_num;

    /* covert to 32bit data if the stream is 16bit */
    if (stream_function_get_output_resolution(para) == RESOLUTION_16BIT)
    {
        for (i = 0; i < channel_number; i++)
        {
            in_buf1 = stream_function_get_inout_buffer(para, i+1);
            dsp_converter_16bit_to_32bit(in_buf1, (S16 *)in_buf1, in_frame_size/sizeof(int16_t));
        }
        in_frame_size = in_frame_size*2;
        stream_function_modify_output_size(para, in_frame_size);
        stream_function_modify_output_resolution(para, RESOLUTION_32BIT);
    }

    /* process data two by two channel */
    if (port->mode == SW_COMPANDER_AUDIO_MODE)
    {
#ifdef AIR_AUDIO_DUMP_ENABLE
        LOG_AUDIO_DUMP((U8 *)stream_function_get_inout_buffer(para, 1), (U32)in_frame_size, AUDIO_CPD_IN_L);
        // in_buf2 = stream_function_get_inout_buffer(para, 2);
        // LOG_AUDIO_DUMP((U8 *)in_buf2, (U32)in_frame_size, AUDIO_CPD_IN_R);
#endif
        for (i = 0; i < port->work_instance_count; i++)
        {
            working_memory_ptr = (void *)((uint32_t)(port->work_mem_ptr) + i * port->work_mem_size);
            if ((channel_number-2*i) >= 2)
            {
                in_buf1 = stream_function_get_inout_buffer(para, 2*i+1);
                in_buf2 = stream_function_get_inout_buffer(para, 2*i+2);
                channel_mode = 2;
            }
            else
            {
                in_buf1 = stream_function_get_inout_buffer(para, 2*i+1);
                in_buf2 = NULL;
                channel_mode = 1;
            }
            compander_AU_proc(working_memory_ptr, in_buf1, in_buf2, in_frame_size/sizeof(int32_t), port->vol_target, channel_mode);
        }
#ifdef AIR_AUDIO_DUMP_ENABLE
        LOG_AUDIO_DUMP((U8 *)stream_function_get_inout_buffer(para, 1), (U32)in_frame_size, AUDIO_CPD_OUT_L);
        // LOG_AUDIO_DUMP((U8 *)in_buf2, (U32)in_frame_size, AUDIO_CPD_OUT_R);
#endif
    }
    else
    {
        DSP_MW_LOG_E("Mode is not supported!, %d", 1, port->mode);
        AUDIO_ASSERT(FALSE);
    }

    return false;
}

#endif /* AIR_SOFTWARE_DRC_ENABLE */
