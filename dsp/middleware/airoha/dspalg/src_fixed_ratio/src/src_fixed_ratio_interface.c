/* Copyright Statement:
*
* (C) 2020 Airoha Technology Corp. All rights reserved.
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
#include "src_fixed_ratio_interface.h"
#include "dsp_feature_interface.h"
#include "dsp_dump.h"
#include "preloader_pisplit.h"
#include "hal_audio_common.h"
#ifdef AIR_FIXED_RATIO_SRC_USE_PIC
#include "src_fixed_ratio_portable.h"
#endif /* AIR_FIXED_RATIO_SRC_USE_PIC */

/* Private define ------------------------------------------------------------*/
#define SRC_FIXED_RATIO_PORT_MAX    (10)
#define SRC_FIXED_RATIO_CVT_MAX     (5)
#define SRC_FIXED_RATIO_MAX         (3)
#define SRC_FIXED_RATIO_DEBUG_LOG   (0)
#define ENDIAN_RVRS(A) (A)
#define SRC_FIXED_RATIO_DUMP_ENABLE (0)
/* Private typedef -----------------------------------------------------------*/
typedef enum {
    SRC_FIXED_RATIO_CH_NUM_UPDATE_LATER = 0,
    SRC_FIXED_RATIO_CH_NUM_EXCLUDE_REF = 0xFFFF,
} src_fixed_ratio_channel_num_e;

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static src_fixed_ratio_port_t src_fixed_ratio_port[SRC_FIXED_RATIO_PORT_MAX];
const S16 dsp_sampler_coef[96] = {
    // NORM_QUALITY 
    //[B,A]=ellip(6,.2,75,.43256, 'low');
    ENDIAN_RVRS((S16)0x1D8B),    // LP2O_COFEB16_00
    ENDIAN_RVRS((S16)0x1FA9),    // LP2O_COEFB16_01
    ENDIAN_RVRS((S16)0x3B36),    // LP2O_COEFB16_02
    ENDIAN_RVRS((S16)0xDAC7),    // LP2O_COEFB16_03
    ENDIAN_RVRS((S16)0x1FA9),    // LP2O_COEFB16_04
    ENDIAN_RVRS((S16)0x1FA9),    // LP2O_COEFB16_05
    ENDIAN_RVRS((S16)0x1366),    // LP2O_COEFB16_06
    ENDIAN_RVRS((S16)0x1FA9),    // LP2O_COEFB16_07
    ENDIAN_RVRS((S16)0x2554),    // LP2O_COEFB16_08
    ENDIAN_RVRS((S16)0xB82B),    // LP2O_COEFB16_09
    ENDIAN_RVRS((S16)0x0D03),    // LP2O_COEFB16_10
    ENDIAN_RVRS((S16)0x1FA9),    // LP2O_COEFB16_11
    ENDIAN_RVRS((S16)0x153E),    // LP2O_COEFB16_12
    ENDIAN_RVRS((S16)0x926A),    // LP2O_COEFB16_13
    ENDIAN_RVRS((S16)0x1FA9),    // LP2O_COEFB24_14
    ENDIAN_RVRS((S16)0),         // LP2O_Reserved       
    // [B,A]=ellip(6, .1, 52,.28, 'low');
    ENDIAN_RVRS((S16)0x12CC),    // LP3O_COFEB16_00
    ENDIAN_RVRS((S16)0x1D43),    // LP3O_COEFB16_01
    ENDIAN_RVRS((S16)0x4B0B),    // LP3O_COEFB16_02
    ENDIAN_RVRS((S16)0xCDAF),    // LP3O_COEFB16_03
    ENDIAN_RVRS((S16)0x1D43),    // LP3O_COEFB16_04
    ENDIAN_RVRS((S16)0x1D43),    // LP3O_COEFB16_05
    ENDIAN_RVRS((S16)0xFB40),    // LP3O_COEFB16_06
    ENDIAN_RVRS((S16)0x1D43),    // LP3O_COEFB16_07
    ENDIAN_RVRS((S16)0x4934),    // LP3O_COEFB16_08
    ENDIAN_RVRS((S16)0xACFC),    // LP3O_COEFB16_09
    ENDIAN_RVRS((S16)0xF4CE),    // LP3O_COEFB16_10
    ENDIAN_RVRS((S16)0x1D43),    // LP3O_COEFB16_11
    ENDIAN_RVRS((S16)0x4A0F),    // LP3O_COEFB16_12
    ENDIAN_RVRS((S16)0x8D79),    // LP3O_COEFB16_13
    ENDIAN_RVRS((S16)0x1D43),    // LP3O_COEFB24_14
    ENDIAN_RVRS((S16)0     ),    // LP3O_Reserved   

    // HIGH_QUALITY 
    // [B,A] = ellip(12, 0.3, 100, 0.465); 
    // B = conv(B1, B2);
    // A = conv(A1, A2);
    ENDIAN_RVRS((S16)0x0EE3),    // LP2_COFEB16_00
    ENDIAN_RVRS((S16)0x0FE3),    // LP2_COEFB16_01
    ENDIAN_RVRS((S16)0x4F52),    // LP2_COEFB16_02
    ENDIAN_RVRS((S16)0xC813),    // LP2_COEFB16_03
    ENDIAN_RVRS((S16)0x0FE3),    // LP2_COEFB16_04
    ENDIAN_RVRS((S16)0x0FE3),    // LP2_COEFB16_05
    ENDIAN_RVRS((S16)0x0983),    // LP2_COEFB16_06
    ENDIAN_RVRS((S16)0x0FE3),    // LP2_COEFB16_07
    ENDIAN_RVRS((S16)0x3C98),    // LP2_COEFB16_08
    ENDIAN_RVRS((S16)0xB57B),    // LP2_COEFB16_09
    ENDIAN_RVRS((S16)0x04CA),    // LP2_COEFB16_10
    ENDIAN_RVRS((S16)0x0FE3),    // LP2_COEFB16_11
    ENDIAN_RVRS((S16)0x277F),    // LP2_COEFB16_12
    ENDIAN_RVRS((S16)0xA05F),    // LP2_COEFB16_13
    ENDIAN_RVRS((S16)0x0FE3),    // LP2_COEFB24_14
    ENDIAN_RVRS((S16)0     ),    // LP2_Reserved

    ENDIAN_RVRS((S16)0x0222),    // LP2_COFEB16_00_
    ENDIAN_RVRS((S16)0x1000),    // LP2_COEFB16_01_
    ENDIAN_RVRS((S16)0x18E8),    // LP2_COEFB16_02_
    ENDIAN_RVRS((S16)0x915D),    // LP2_COEFB16_03_
    ENDIAN_RVRS((S16)0x1000),    // LP2_COEFB16_04_
    ENDIAN_RVRS((S16)0x1000),    // LP2_COEFB16_05_
    ENDIAN_RVRS((S16)0x00D1),    // LP2_COEFB16_06_
    ENDIAN_RVRS((S16)0x1000),    // LP2_COEFB16_07_
    ENDIAN_RVRS((S16)0x10E6),    // LP2_COEFB16_08_
    ENDIAN_RVRS((S16)0x8836),    // LP2_COEFB16_09_
    ENDIAN_RVRS((S16)0x0049),    // LP2_COEFB16_10_
    ENDIAN_RVRS((S16)0x1000),    // LP2_COEFB16_11_
    ENDIAN_RVRS((S16)0x0D92),    // LP2_COEFB16_12_
    ENDIAN_RVRS((S16)0x8268),    // LP2_COEFB16_13_
    ENDIAN_RVRS((S16)0x1000),    // LP2_COEFB24_14_
    ENDIAN_RVRS((S16)0     ),    // LP2_Reserved_
    
    // [B,A] = ellip(12, 0.3, 100, 0.31);
    // B = conv(B1, B2);
    // A = conv(A1, A2);
    ENDIAN_RVRS((S16)0x06EE),    // LP3_COFEB16_00
    ENDIAN_RVRS((S16)0x0859),    // LP3_COEFB16_01
    ENDIAN_RVRS((S16)0x61D7),    // LP3_COEFB16_02
    ENDIAN_RVRS((S16)0xB1F6),    // LP3_COEFB16_03
    ENDIAN_RVRS((S16)0x0859),    // LP3_COEFB16_04
    ENDIAN_RVRS((S16)0x0859),    // LP3_COEFB16_05
    ENDIAN_RVRS((S16)0x015F),    // LP3_COEFB16_06
    ENDIAN_RVRS((S16)0x0859),    // LP3_COEFB16_07
    ENDIAN_RVRS((S16)0x5AFF),    // LP3_COEFB16_08
    ENDIAN_RVRS((S16)0xA6F8),    // LP3_COEFB16_09
    ENDIAN_RVRS((S16)0xFE3D),    // LP3_COEFB16_10
    ENDIAN_RVRS((S16)0x0859),    // LP3_COEFB16_11
    ENDIAN_RVRS((S16)0x526E),    // LP3_COEFB16_12
    ENDIAN_RVRS((S16)0x9908),    // LP3_COEFB16_13
    ENDIAN_RVRS((S16)0x0859),    // LP3_COEFB24_14
    ENDIAN_RVRS((S16)0     ),    // LP3_Reserved 
    
    ENDIAN_RVRS((S16)0xFA09),    // LP3_COFEB16_00_
    ENDIAN_RVRS((S16)0x1000),    // LP3_COEFB16_01_
    ENDIAN_RVRS((S16)0x4BF1),    // LP3_COEFB16_02_
    ENDIAN_RVRS((S16)0x8E04),    // LP3_COEFB16_03_
    ENDIAN_RVRS((S16)0x1000),    // LP3_COEFB16_04_
    ENDIAN_RVRS((S16)0x1000),    // LP3_COEFB16_05_
    ENDIAN_RVRS((S16)0xF8EE),    // LP3_COEFB16_06_
    ENDIAN_RVRS((S16)0x1000),    // LP3_COEFB16_07_
    ENDIAN_RVRS((S16)0x484F),    // LP3_COEFB16_08_
    ENDIAN_RVRS((S16)0x86CA),    // LP3_COEFB16_09_
    ENDIAN_RVRS((S16)0xF882),    // LP3_COEFB16_10_
    ENDIAN_RVRS((S16)0x1000),    // LP3_COEFB16_11_
    ENDIAN_RVRS((S16)0x4725),    // LP3_COEFB16_12_
    ENDIAN_RVRS((S16)0x8202),    // LP3_COEFB16_13_
    ENDIAN_RVRS((S16)0x1000),    // LP3_COEFB24_14_
    ENDIAN_RVRS((S16)0     )     // LP3_Reserved_

};

/* Private functions ---------------------------------------------------------*/
ATTR_TEXT_IN_IRAM static uint32_t src_fixed_ratio_fs_converter(stream_samplerate_t fs)
{
    switch (fs) {
        case FS_RATE_44_1K:
            return 44100;

        case FS_RATE_8K:
        case FS_RATE_16K:
        case FS_RATE_24K:
        case FS_RATE_25K:
        case FS_RATE_32K:
        case FS_RATE_48K:
        case FS_RATE_50K:
        case FS_RATE_96K:
        case FS_RATE_192K:
            return fs * 1000;

        default:
            AUDIO_ASSERT(FALSE && "[SRC_FIXED_RATIO] sample rate is not supported!");
            return fs;
    }
}

/* Public functions ----------------------------------------------------------*/
/**
 * @brief This function is used to get the src port.
 *        If the owner does not have a src port, it will malloc a port for this owner.
 *        If the owner have a src port, it will return the port directly.
 *        If the owner is NULL, it will return the first unused port.
 *
 * @param owner is who want to get or query a src port or NULL.
 * @return src_fixed_ratio_port_t* is the result.
 */
ATTR_TEXT_IN_RAM_FOR_MASK_IRQ src_fixed_ratio_port_t *stream_function_src_fixed_ratio_get_port(void *owner)
{
    int32_t i;
    uint32_t saved_mask;
    src_fixed_ratio_port_t *port = NULL;

    /* Find out a port for this owner */
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    for (i = SRC_FIXED_RATIO_PORT_MAX - 1; i >= 0; i--) {
        /* Check if there is unused port */
        if (src_fixed_ratio_port[i].owner == NULL) {
            port = &src_fixed_ratio_port[i];
            break;
        }

        /* Check if this owner has already owned a sw src */
        if (src_fixed_ratio_port[i].owner == owner) {
            port = &src_fixed_ratio_port[i];
            break;
        }
    }
    hal_nvic_restore_interrupt_mask(saved_mask);

    DSP_MW_LOG_I("[SRC_FIXED_RATIO] get port%d, owner:0x%x", 2, i, owner);

    if (port == NULL) {
        DSP_MW_LOG_E("[SRC_FIXED_RATIO] Port not enough!", 0);
        return port;
    }

    port->owner = owner;
    return port;
}

/**
 * @brief This function is used to get the src port.
 *        If the owner does not have a src port, it will malloc a port for this owner.
 *        If the owner have a src port, it will return the port directly.
 *        If the owner is NULL, it will return the first unused port.
 *
 * @param owner is who want to get or query a src port or NULL.
 * @return src_fixed_ratio_port_t* is the result.
 */
ATTR_TEXT_IN_RAM_FOR_MASK_IRQ src_fixed_ratio_port_t *stream_function_src_fixed_ratio_get_2nd_port(void *owner)
{
    int32_t i;
    int32_t j = 0;
    uint32_t saved_mask;
    src_fixed_ratio_port_t *port = NULL;

    /* Find out a port for this owner */
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    for (i = SRC_FIXED_RATIO_PORT_MAX - 1; i >= 0; i--) {
        if (src_fixed_ratio_port[i].owner == owner) {
            j++;
        }
        if ((src_fixed_ratio_port[i].owner == NULL) && (j == 1)) {
            port = &src_fixed_ratio_port[i];
            break;
        }
    }
    hal_nvic_restore_interrupt_mask(saved_mask);

    if (port == NULL) {
        DSP_MW_LOG_E("[SRC_FIXED_RATIO] 2nd Port not available!", 0);
        return port;
    }
    port->owner = owner;
    DSP_MW_LOG_I("[SRC_FIXED_RATIO] get 2nd port%d, owner:0x%x", 2, i, owner);
    return port;
}


/**
 * @brief This function is used to get the src port.
 *        If the owner does not have a src port, it will malloc a port for this owner.
 *        If the owner have a src port, it will return the port directly.
 *        If the owner is NULL, it will return the first unused port.
 *
 * @param owner is who want to get or query a src port or NULL.
 * @return src_fixed_ratio_port_t* is the result.
 */

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ src_fixed_ratio_port_t *stream_function_src_fixed_ratio_get_3rd_port(void *owner)
{
    int32_t i;
    int32_t j = 0;
    uint32_t saved_mask;
    src_fixed_ratio_port_t *port = NULL;

    /* Find out a port for this owner */
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    for (i = SRC_FIXED_RATIO_PORT_MAX - 1; i >= 0; i--) {
        if (src_fixed_ratio_port[i].owner == owner) {
            j++;
        }
        if ((src_fixed_ratio_port[i].owner == NULL) && (j == 2)) {
            port = &src_fixed_ratio_port[i];
            break;
        }
    }
    hal_nvic_restore_interrupt_mask(saved_mask);

    if (port == NULL) {
        DSP_MW_LOG_E("[SRC_FIXED_RATIO] 3rd Port not available!", 0);
        return port;
    }
    port->owner = owner;
    DSP_MW_LOG_I("[SRC_FIXED_RATIO] get 3rd port%d, owner:0x%x", 2, i, owner);
    return port;
}

/**
 * @brief This function is used to get the src port.
 *        If the owner does not have a src port, it will malloc a port for this owner.
 *        If the owner have a src port, it will return the port directly.
 *        If the owner is NULL, it will return the first unused port.
 *
 * @param owner is who want to get or query a src port or NULL.
 * @return src_fixed_ratio_port_t* is the result.
 */

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ src_fixed_ratio_port_t *stream_function_src_fixed_ratio_get_number_port(void *owner, int32_t port_number)
{
    int32_t i;
    int32_t j = 0;
    uint32_t saved_mask;
    src_fixed_ratio_port_t *port = NULL;

    /* Find out a port for this owner */
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    for (i = SRC_FIXED_RATIO_PORT_MAX - 1; i >= 0; i--) {
        if (src_fixed_ratio_port[i].owner == owner) {
            j++;
        }
        if ((src_fixed_ratio_port[i].owner == NULL) && (j == (port_number - 1))) {
            port = &src_fixed_ratio_port[i];
            break;
        }
    }
    hal_nvic_restore_interrupt_mask(saved_mask);

    if (port == NULL) {
        DSP_MW_LOG_E("[SRC_FIXED_RATIO] No.%d Port not available!", 1, port_number);
        return port;
    }
    port->owner = owner;
    DSP_MW_LOG_I("[SRC_FIXED_RATIO] get No.%d port%d, owner:0x%x", 3, port_number, i, owner);
    return port;
}


/**
 * @brief This function is used to configure the src port.
 *
 * @param port is which port needs to be configured.
 * @param config is pointer to the configuration settings.
 */
void stream_function_src_fixed_ratio_init(src_fixed_ratio_port_t *port, src_fixed_ratio_config_t *config)
{
    int32_t is_upsample=0, factor=1;
    /* check port */
    if (port == NULL) {
        AUDIO_ASSERT(FALSE && "[SRC_FIXED_RATIO] Port is NULL!");
        return;
    }
    /* check sampling rate */
    if (config->in_sampling_rate > config->out_sampling_rate) {
        is_upsample = 0;
        if (config->in_sampling_rate % config->out_sampling_rate != 0) {
            AUDIO_ASSERT(FALSE && "[SRC_FIXED_RATIO] sampling rate incorrect");
            return;
        } else {
            factor = config->in_sampling_rate / config->out_sampling_rate;
            if ((factor < 2) || (factor > 3)) {
                AUDIO_ASSERT(FALSE && "[SRC_FIXED_RATIO] sampling rate incorrect");
                return;
            }
        }
    } else if (config->in_sampling_rate < config->out_sampling_rate) {
        is_upsample = 1;
        if (config->out_sampling_rate % config->in_sampling_rate != 0) {
            AUDIO_ASSERT(FALSE && "[SRC_FIXED_RATIO] sampling rate incorrect");
            return;
        }  else {
            factor = config->out_sampling_rate / config->in_sampling_rate;
            if ((factor < 2) || (factor > 3)) {
                AUDIO_ASSERT(FALSE && "[SRC_FIXED_RATIO] sampling rate incorrect");
                return;
            }
        }
    } else if (config->in_sampling_rate == config->out_sampling_rate) {
        /* do nothing */
    }
    /* config */
    port->status = SRC_FIXED_RATIO_PORT_STAT_INIT;
    port->channel_number = config->channel_number;
    port->in_sampling_rate = config->in_sampling_rate;
    port->out_sampling_rate = config->out_sampling_rate;
    port->resolution = config->resolution;
    port->is_upsample = is_upsample;
    port->factor = factor;
    port->multi_cvt_mode = SRC_FIXED_RATIO_PORT_MUTI_CVT_MODE_SINGLE;
    port->cvt_num = 1;
    port->init_cnt = 0;
    if ((config->multi_cvt_mode > SRC_FIXED_RATIO_PORT_MUTI_CVT_MODE_SINGLE)
        && (config->multi_cvt_mode < SRC_FIXED_RATIO_PORT_MUTI_CVT_MODE_MAX)
        && (config->cvt_num <= SRC_FIXED_RATIO_CVT_MAX)) {
        port->multi_cvt_mode = config->multi_cvt_mode;
        port->cvt_num = config->cvt_num;
    }

    if(config->multi_cvt_mode == SRC_FIXED_RATIO_PORT_MUTI_CVT_MODE_CONSECUTIVE_AND_ALTERNATE && port->cvt_num == 0) {
        port->cvt_num = 1;
    }

    port->with_codec = config->with_codec;
    if ((config->max_frame_buff_size > 0) && ((config->max_frame_buff_size & 0xFFFF0000) == 0)) {
        port->tmp_buff_size = config->max_frame_buff_size;
        port->tmp_buff = (U8*)preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, port->tmp_buff_size);
    }
    port->quality_mode = (config->quality_mode == SRC_FIXED_RATIO_PORT_HIGH_QUALITY) ? SRC_FIXED_RATIO_PORT_HIGH_QUALITY : SRC_FIXED_RATIO_PORT_NORMAL_QUALITY;


    DSP_MW_LOG_I("[SRC_FIXED_RATIO] INIT, status:%d, ch:%d, in_sampling_rate:%d, out_sampling_rate:%d, is_upsample:%d, factor:%d, multi_cvt_mode:%d, cvt_num:%d, buff_size:%d, buff:0x%x, quality_mode:%d", 11, port->status, port->channel_number, port->in_sampling_rate, port->out_sampling_rate, port->is_upsample, port->factor, port->multi_cvt_mode, port->cvt_num, port->tmp_buff_size, port->tmp_buff, port->quality_mode);
}

/**
 * @brief This function is used to deinit the src port.
 *
 * @param port is which port needs to be freed.
 */
void stream_function_src_fixed_ratio_deinit(src_fixed_ratio_port_t *port)
{
    int i;
    /* check port */
    if (port == NULL) {
        AUDIO_ASSERT(FALSE && "[SRC_FIXED_RATIO] Port is NULL!");
        return;
    }
    DSP_MW_LOG_I("[SRC_FIXED_RATIO]DEINIT, owner:0x%x", 1, port->owner);
    port->status = SRC_FIXED_RATIO_PORT_STAT_DEINIT;
    port->owner = NULL;
    port->channel_number = 0;
    port->in_sampling_rate = 0;
    port->out_sampling_rate = 0;
    port->resolution = 0;
    port->is_upsample = 0;
    port->factor = 0;
    port->multi_cvt_mode = 0;
    port->cvt_num = 1;
    port->with_codec = 0;
    for (i = SMP_MAX_CH_NUM - 1; i >= 0; i--) {
        if (port->smp_instance_ptr[i]) {
            //free memory
            preloader_pisplit_free_memory(port->smp_instance_ptr[i]);
            port->smp_instance_ptr[i] = NULL;
        }
    }
    if (port->tmp_buff) {
        preloader_pisplit_free_memory(port->tmp_buff);
        port->tmp_buff = NULL;
        port->tmp_buff_size = 0;
    }
    port->quality_mode = 0;
}

void stream_function_src_fixed_ratio_configure_channel_number(src_fixed_ratio_port_t *port, U32 channel_num)
{
    if (port) {
        port->channel_number = channel_num;
    }
}
/**
 * @brief This function is used to change in_sampling_rate of the src port.
 *
 * @param port is which port needs to be freed.
 * @param in_sampling_rate is the new in_sampling_rate.
 */
void stream_function_src_fixed_ratio_change_in_sampling_rate(src_fixed_ratio_port_t *port, U32 in_sampling_rate)
{
    int32_t is_upsample=0, factor=1;
    /* check port */
    if (port == NULL) {
        DSP_MW_LOG_E("[SRC_FIXED_RATIO] Port is NULL!", 0);
        AUDIO_ASSERT(FALSE);
        return;
    }
    /* check sampling rate */
    if (in_sampling_rate > port->out_sampling_rate) {
        is_upsample = 0;
        if (in_sampling_rate % port->out_sampling_rate != 0) {
            DSP_MW_LOG_E("[SRC_FIXED_RATIO] sampling rate incorrect", 0);
            AUDIO_ASSERT(FALSE);
            return;
        } else {
            factor = in_sampling_rate / port->out_sampling_rate;
            if ((factor < 2) || (factor > 3)) {
                DSP_MW_LOG_E("[SRC_FIXED_RATIO] sampling rate incorrect", 0);
                AUDIO_ASSERT(FALSE);
                return;
            }
        }
    } else if (in_sampling_rate < port->out_sampling_rate) {
        is_upsample = 1;
        if (port->out_sampling_rate % in_sampling_rate != 0) {
            DSP_MW_LOG_E("[SRC_FIXED_RATIO] sampling rate incorrect", 0);
            AUDIO_ASSERT(FALSE);
            return;
        }  else {
            factor = port->out_sampling_rate / in_sampling_rate;
            if ((factor < 2) || (factor > 3)) {
                DSP_MW_LOG_E("[SRC_FIXED_RATIO] sampling rate incorrect", 0);
                AUDIO_ASSERT(FALSE);
                return;
            }
        }
    } else if (in_sampling_rate == port->out_sampling_rate) {
        /* do nothing */
    } else {
        DSP_MW_LOG_E("[SRC_FIXED_RATIO] sampling rate incorrect", 0);
#ifndef AIR_BLE_FIXED_RATIO_SRC_ENABLE
        AUDIO_ASSERT(FALSE);
#endif
        return;
    }
    /* config */
    //port->status = SRC_FIXED_RATIO_PORT_STAT_INIT;
    port->in_sampling_rate = in_sampling_rate;
    port->is_upsample = is_upsample;
    port->factor = factor;

    DSP_MW_LOG_I("[SRC_FIXED_RATIO] CONFIG, status:%d, ch:%d, in_sampling_rate:%d, out_sampling_rate:%d, is_upsample:%d, factor:%d, multi_cvt_mode:%d, cvt_num:%d", 8, port->status, port->channel_number, port->in_sampling_rate, port->out_sampling_rate, port->is_upsample, port->factor, port->multi_cvt_mode, port->cvt_num);
}

/**
 * @brief This function is used to initialize the src run-time environment.
 *
 * @param para is the input parameters.
 * @return true means there is a error.
 * @return false means there is no error.
 */
bool stream_function_src_fixed_ratio_initialize(void *para)
{
    src_fixed_ratio_processing_num_config_t *processing_num_config = (src_fixed_ratio_processing_num_config_t *)stream_function_get_working_buffer(para);
    int32_t i, j, k;
    src_fixed_ratio_port_t *port = NULL;
    DSP_STREAMING_PARA_PTR stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    uint32_t channel_number;
    uint32_t instance_size = 0, quality_mode = 0;
    void* smp_instance_ptr = NULL;
    uint32_t *port_ch_num;
    src_fixed_ratio_port_status_e *port_status;
    uint16_t cvt_num;

    /* Find out the port of this stream */
    for (i = SRC_FIXED_RATIO_PORT_MAX - 1; i >= 0; i--) {
        /* Check if this source or sink has already owned a sw src */
        if ((src_fixed_ratio_port[i].owner == stream_ptr->source) ||
            (src_fixed_ratio_port[i].owner == stream_ptr->sink)) {
            port = &src_fixed_ratio_port[i];
            break;
        }
    }
    if (port == NULL) {
        AUDIO_ASSERT(FALSE && "[SRC_FIXED_RATIO] Port is not found!");
        return TRUE;
    }
#if SRC_FIXED_RATIO_DEBUG_LOG
    else {
        DSP_MW_LOG_I("[SRC_FIXED_RATIO] Port %d found!", 1, i);
    }
#endif

    if(port->multi_cvt_mode == SRC_FIXED_RATIO_PORT_MUTI_CVT_MODE_CONSECUTIVE_AND_ALTERNATE && processing_num_config->processing_num == 0) {
        port->init_cnt++;
        processing_num_config->processing_num = port->init_cnt;
        DSP_MW_LOG_I("[SRC_FIXED_RATIO] mode:SRC_FIXED_RATIO_PORT_MUTI_CVT_MODE_CONSECUTIVE_AND_ALTERNATE, owner:%d, feature_num:%d", 2, src_fixed_ratio_port[i].owner, processing_num_config->processing_num);
    }

    if (port->with_codec) {
        if (((DSP_ENTRY_PARA_PTR)para)->number.field.process_sequence == ((DSP_ENTRY_PARA_PTR)para)->number.field.feature_number - 1) {
            channel_number = stream_function_get_device_channel_number(para);
            stream_codec_get_input_size(para); /* Must mark the encoder flag here by call codec APIs. */
        } else if (((DSP_ENTRY_PARA_PTR)para)->number.field.process_sequence == CODEC_ALLOW_SEQUENCE) {
            channel_number = stream_codec_get_input_channel_number(para);
        } else {
            channel_number = stream_function_get_channel_number(para);
        }
    } else {
        channel_number = stream_function_get_channel_number(para);
    }

    /* status check */
    port_status = &(port->status);
    if ((*port_status == SRC_FIXED_RATIO_PORT_STAT_RUNNING) || (*port_status == SRC_FIXED_RATIO_PORT_STAT_BYPASS)) {
        return FALSE;
    } else if (*port_status == SRC_FIXED_RATIO_PORT_STAT_INIT) {
        *port_status = SRC_FIXED_RATIO_PORT_STAT_RUNNING;
    } else {
        AUDIO_ASSERT(0 && "[SRC_FIXED_RATIO] error status:%d");
        return TRUE;
    }

    port_ch_num = &(port->channel_number);
    if (*port_ch_num != channel_number) {
        if (*port_ch_num == SRC_FIXED_RATIO_CH_NUM_UPDATE_LATER) {
            *port_ch_num = channel_number;
        } else if (*port_ch_num == SRC_FIXED_RATIO_CH_NUM_EXCLUDE_REF) {
#if SRC_FIXED_RATIO_DEBUG_LOG
            DSP_MW_LOG_W("[SRC_FIXED_RATIO] ch number exclude REF", 0);
#endif
            *port_ch_num = channel_number - 1;
//        } else {
//                DSP_MW_LOG_E("[SRC_FIXED_RATIO] ch number is not right! %d != %d", 2, port->channel_number, channel_number);
//                OS_ASSERT(FALSE);
//                return TRUE;
        }
    }

    /*smp instance init*/
    quality_mode = port->quality_mode;
    instance_size = get_updn_samp_memsize(quality_mode);
    for (k = *port_ch_num - 1; k >= 0; k--) {
        smp_instance_ptr = port->smp_instance_ptr[k];
        if (smp_instance_ptr) {
            preloader_pisplit_free_memory(smp_instance_ptr);
        }
        smp_instance_ptr = preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, instance_size);
        if (smp_instance_ptr == NULL) {
            AUDIO_ASSERT(0 && "[SRC_FIXED_RATIO] malloc fail!");
        }
        updn_samp_init(smp_instance_ptr, (void*)&dsp_sampler_coef, quality_mode);
        port->smp_instance_ptr[k] = smp_instance_ptr;
    }

    cvt_num = port->cvt_num;

    if(port->multi_cvt_mode == SRC_FIXED_RATIO_PORT_MUTI_CVT_MODE_CONSECUTIVE_AND_ALTERNATE) {
        int32_t next_proc_index = i - (port->cvt_num);
        if(src_fixed_ratio_port[next_proc_index].owner != src_fixed_ratio_port[i].owner) {
#if SRC_FIXED_RATIO_DEBUG_LOG
            DSP_MW_LOG_I("[SRC_FIXED_RATIO] get error next_proc_index:%d, 1st cvt_num:%d, first_proc_index:%d", 3, next_proc_index, port->cvt_num, i);
#endif
            AUDIO_ASSERT(0 && "[SRC_FIXED_RATIO] get error next_proc_index!!!");
        } else {
            cvt_num += src_fixed_ratio_port[next_proc_index].cvt_num;
        }
    }

    if (cvt_num > 1) { // initialize other ports of same owner
        src_fixed_ratio_port_t *other_port = NULL;
        for (j = i - 1; j >= 0; j--) {
            /* Check if this source or sink has already owned a sw src */
            if (src_fixed_ratio_port[j].owner == src_fixed_ratio_port[i].owner) {
                other_port = &src_fixed_ratio_port[j];
#if SRC_FIXED_RATIO_DEBUG_LOG
                DSP_MW_LOG_I("[SRC_FIXED_RATIO] other Port %d found!", 1, j);
#endif
                /* status check */
                port_status = &(other_port->status);
                if ((*port_status == SRC_FIXED_RATIO_PORT_STAT_RUNNING) || (*port_status == SRC_FIXED_RATIO_PORT_STAT_BYPASS)) {

                } else if (*port_status == SRC_FIXED_RATIO_PORT_STAT_INIT) {
                    *port_status = SRC_FIXED_RATIO_PORT_STAT_RUNNING;
                } else {
                    AUDIO_ASSERT(0 && "[SRC_FIXED_RATIO] error status!!!");
                    return TRUE;
                }

                port_ch_num = &(other_port->channel_number);
                if (*port_ch_num != channel_number) {
                    if ((*port_ch_num == SRC_FIXED_RATIO_CH_NUM_UPDATE_LATER) || (other_port->multi_cvt_mode == SRC_FIXED_RATIO_PORT_MUTI_CVT_MODE_CONSECUTIVE)) {
                        *port_ch_num = channel_number;
                    } else if (*port_ch_num == SRC_FIXED_RATIO_CH_NUM_EXCLUDE_REF) {
                        *port_ch_num = channel_number - 1;
                    }
                }

                quality_mode = other_port->quality_mode;
                instance_size = get_updn_samp_memsize(quality_mode);
                /*smp instance init*/
                for (k = *port_ch_num - 1; k >= 0; k--) {
                    smp_instance_ptr = other_port->smp_instance_ptr[k];
                    if (smp_instance_ptr) {
                        preloader_pisplit_free_memory(smp_instance_ptr);
                    }
                    smp_instance_ptr = preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, instance_size);
                    if (smp_instance_ptr == NULL) {
                        AUDIO_ASSERT(0 && "[SRC_FIXED_RATIO] malloc fail!");
                    }
                    updn_samp_init(smp_instance_ptr, (void*)&dsp_sampler_coef, quality_mode);
                    other_port->smp_instance_ptr[k] = smp_instance_ptr;
                }
            }
        }

        if (other_port == NULL) {
            AUDIO_ASSERT(0 && "[SRC_FIXED_RATIO] other Ports are not found!");
        }
    }
    DSP_MW_LOG_I("[SRC_FIXED_RATIO] initialize, version:%x", 1, get_updn_samp_version());
    return 0;
}

uint32_t src_fixed_ratio_cal_max_ratio(src_fixed_ratio_port_t *port[SRC_FIXED_RATIO_CVT_MAX], uint32_t port_num)
{
    uint32_t begin_rate = port[0]->in_sampling_rate;
    uint32_t max_rate = 0, max_ratio = 1;
    uint32_t i = 0;

    for(i = 0; i < port_num; i++) {
        if(port[i]->out_sampling_rate > max_rate) {
            max_rate = port[i]->out_sampling_rate;
        }
    }

    if(max_rate > begin_rate) {
        max_ratio = max_rate / begin_rate;
    }
    return max_ratio;
}

/**
 * @brief This function is used to process the src in run-time
 *
 * @param para is the input parameters.
 * @return true means there is a error.
 * @return false means there is no error.
 */
ATTR_TEXT_IN_IRAM bool stream_function_src_fixed_ratio_process(void *para)
{
    src_fixed_ratio_port_t *port[SRC_FIXED_RATIO_CVT_MAX] = {NULL};
    src_fixed_ratio_processing_num_config_t *processing_num_config = (src_fixed_ratio_processing_num_config_t *)stream_function_get_working_buffer(para);
    uint32_t cvt_start_index = 0;
    DSP_STREAMING_PARA_PTR stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    SOURCE source = stream_ptr->source;
    SINK sink = stream_ptr->sink;
    int32_t i, j, res;
    int m, n = 0;
    uint32_t in_frame_size;
    uint8_t in_sampling_rate;
    uint32_t channel_number;
    uint32_t process_buff_size;
    uint8_t *in_data_buffer;
    int16_t out_frame_cnt;
    uint32_t sample_size;
    uint32_t max_ratio;

#if SRC_FIXED_RATIO_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
#endif /* SW_SRC_DEBUG_LOG */

    /* Find out the port of this stream */
    for (i = SRC_FIXED_RATIO_PORT_MAX - 1; i >= 0; i--) {
        /* Check if this source or sink has already owned a sw src */
        if ((src_fixed_ratio_port[i].owner == source) ||
            (src_fixed_ratio_port[i].owner == sink)) {
            port[cvt_start_index] = &src_fixed_ratio_port[i];
#if SRC_FIXED_RATIO_DEBUG_LOG
            DSP_MW_LOG_I("[SRC_FIXED_RATIO]port %d is found, owner:0x%x", 2, i, src_fixed_ratio_port[i].owner);
#endif
            break;
        }
    }
    if (port[cvt_start_index] == NULL) {
        AUDIO_ASSERT(FALSE && "[SRC_FIXED_RATIO] Port is not found!");
        return TRUE;
    }

    if (port[cvt_start_index]->status == SRC_FIXED_RATIO_PORT_STAT_BYPASS) {
        return false;
    }

    if (port[cvt_start_index]->with_codec) {
        if (((DSP_ENTRY_PARA_PTR)para)->number.field.process_sequence == ((DSP_ENTRY_PARA_PTR)para)->number.field.feature_number - 1) {

            channel_number = stream_function_get_device_channel_number(para);

            in_sampling_rate = stream_codec_get_input_samplingrate(para);

            in_frame_size = stream_codec_get_input_size(para);

        } else if (((DSP_ENTRY_PARA_PTR)para)->number.field.process_sequence == CODEC_ALLOW_SEQUENCE) {

            channel_number = stream_codec_get_input_channel_number(para);

            in_sampling_rate = stream_codec_get_input_samplingrate(para);

            in_frame_size = stream_codec_get_input_size(para);

        } else {

            channel_number = stream_function_get_channel_number(para);

            in_sampling_rate = stream_function_get_samplingrate(para);

            in_frame_size = stream_function_get_output_size(para);

        }
    } else {
        in_frame_size = stream_function_get_output_size(para);
        in_sampling_rate = stream_function_get_samplingrate(para);
        channel_number = stream_function_get_channel_number(para);
    }

    //check other port
    for (m = i - 1; m >= 0; m--) {
        /* Check if this source or sink  own other sw src */
        if (src_fixed_ratio_port[m].owner == src_fixed_ratio_port[i].owner) {
            n++;
            port[n] = &src_fixed_ratio_port[m];
            if (port[n - 1]->out_sampling_rate != port[n]->in_sampling_rate) {
#if SRC_FIXED_RATIO_DEBUG_LOG
                DSP_MW_LOG_E("[SRC_FIXED_RATIO] converter in/out sampling rate are not match! %d != %d", 2, port[n - 1]->out_sampling_rate, port[n]->in_sampling_rate);
#endif
                AUDIO_ASSERT(0 && "[SRC_FIXED_RATIO] converter in/out sampling rate are not match!");
            }
#if SRC_FIXED_RATIO_DEBUG_LOG
            DSP_MW_LOG_I("[SRC_FIXED_RATIO]port %d is found, owner:0x%x, port index %d ptr 0x%x", 4, m, src_fixed_ratio_port[m].owner, n, port[n]);
#endif

        }
    }

    max_ratio = src_fixed_ratio_cal_max_ratio(port, n+1);
    process_buff_size = in_frame_size * max_ratio;

#if SRC_FIXED_RATIO_DEBUG_LOG
    DSP_MW_LOG_I("[SRC_FIXED_RATIO][START]in_frame_size:%d, in_sampling_rate:%d, channel_number:%d, tmp_buff_size:%d, mode:%d", 5, in_frame_size, in_sampling_rate, channel_number, process_buff_size, port[cvt_start_index]->multi_cvt_mode);
    DSP_MW_LOG_I("[SRC_FIXED_RATIO][start]: %d, %u", 2, in_frame_size, current_timestamp);
#endif /* SW_SRC_DEBUG_LOG */

    if (!in_frame_size) {
        // DSP_MW_LOG_I("[SW_SRC] Input length is 0!", 0);
        return FALSE;
    }

//    if (port[0]->channel_number != channel_number)
//    {
//        DSP_MW_LOG_E("[SRC_FIXED_RATIO] ch number is not right!", 0);
//        AUDIO_ASSERT(FALSE);
//        return TRUE;
//    }

    if((port[cvt_start_index]->multi_cvt_mode) == SRC_FIXED_RATIO_PORT_MUTI_CVT_MODE_CONSECUTIVE_AND_ALTERNATE) {
        cvt_start_index = (processing_num_config->processing_num - 1) * (port[0]->cvt_num);
    }

    if ((port[cvt_start_index]->resolution != RESOLUTION_16BIT) && (port[cvt_start_index]->resolution != RESOLUTION_32BIT)) {
        AUDIO_ASSERT(FALSE && "[SRC_FIXED_RATIO] resolution is not right!");
        return TRUE;
    }

    /* check if the in_sampling_rate is matched */
    if ((port[cvt_start_index]->in_sampling_rate != src_fixed_ratio_fs_converter(in_sampling_rate)) && (port[cvt_start_index]->cvt_num <= 1)) {
#if SRC_FIXED_RATIO_DEBUG_LOG
        DSP_MW_LOG_E("[SRC_FIXED_RATIO] input sampling rate is not right, %u, %u!", 2, port[cvt_start_index]->in_sampling_rate, src_fixed_ratio_fs_converter(in_sampling_rate));
#endif
        AUDIO_ASSERT(FALSE && "[SRC_FIXED_RATIO] input sampling rate is not right");
        return TRUE;
    }

    /* check if in_sampling_rate and out_sampling_rate are same */
    if ((port[cvt_start_index]->in_sampling_rate == port[cvt_start_index]->out_sampling_rate) && ((port[cvt_start_index]->cvt_num <= 1) || (port[cvt_start_index]->multi_cvt_mode == SRC_FIXED_RATIO_PORT_MUTI_CVT_MODE_ALTERNATE))) {
        return FALSE;
    }

    if ((port[cvt_start_index]->tmp_buff == NULL) || (port[cvt_start_index]->tmp_buff_size < in_frame_size)) {
        port[cvt_start_index]->tmp_buff = (U8 *)preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, process_buff_size);
        if (port[cvt_start_index]->tmp_buff == NULL) {
            AUDIO_ASSERT(FALSE && "[SRC_FIXED_RATIO] tmp buff malloc FAIL!");
        }
        port[cvt_start_index]->tmp_buff_size = process_buff_size;
    }

    if (port[cvt_start_index]->tmp_buff_size < in_frame_size) {
        AUDIO_ASSERT(FALSE && "[SRC_FIXED_RATIO] tmp_buff_size < in_frame_size");
    }

    in_data_buffer = port[cvt_start_index]->tmp_buff;

    if ((port[cvt_start_index]->cvt_num <= 1) || (port[cvt_start_index]->multi_cvt_mode == SRC_FIXED_RATIO_PORT_MUTI_CVT_MODE_ALTERNATE)) {
        int32_t port_select = -1;
        for (m = 0; m < port[cvt_start_index]->cvt_num; m++) {
            if (port[m]->in_sampling_rate == src_fixed_ratio_fs_converter(in_sampling_rate)) {
                port_select = m;
                break;
            }
        }

        if (port_select < 0) {
            AUDIO_ASSERT(0 && "[SRC_FIXED_RATIO] no port can be used");
            return true;
        }
        //DSP_MW_LOG_I("[SRC_FIXED_RATIO]port_select %d, m %d %d %d", 4, port_select, m, 59+2*m, 60+2*m);

        sample_size = (port[port_select]->resolution == RESOLUTION_16BIT) ? sizeof(int16_t) : sizeof(int32_t);

        for (j = 0; j < (int32_t)port[port_select]->channel_number; j++) {
#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE) || defined(AIR_HEARTHROUGH_MAIN_ENABLE) || defined(AIR_CUSTOMIZED_PSAP_ENABLE)
            uint32_t k;
            uint32_t *src_buf = stream_function_get_inout_buffer(para, j + 1);
            for (k = 0; k < (in_frame_size/4); k++) {
                *((uint32_t *)in_data_buffer+k) = *(src_buf+k);
            }
            if ((in_frame_size%4) == 0) {
                /* do nothing */
            }
            else if ((in_frame_size%4) == 2) {
                /* sample is 32bit or 16bit, so in here it means sample num is odd and sample is 16bit */
                *((uint16_t *)in_data_buffer+(in_frame_size/4)*2) = *(src_buf+(in_frame_size/4)*2);
            }
            else {
                AUDIO_ASSERT(0 && "[SRC_FIXED_RATIO] frame size is not right");
                return true;
            }
#else
            memset(in_data_buffer, 0, port[0]->tmp_buff_size);
            memcpy(in_data_buffer, stream_function_get_inout_buffer(para, j + 1), in_frame_size);
#endif /* defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE) */

#if SRC_FIXED_RATIO_DUMP_ENABLE
            LOG_AUDIO_DUMP((U8 *)stream_function_get_inout_buffer(para, j + 1), in_frame_size, SRC_FIXED_RATIO_P1_IN1 + j + (m * SMP_MAX_CH_NUM << 1)); //DUMP ID: SRC_FIXED_RATIO_P(m*8)_IN(j+1)
#endif

            if (port[port_select]->resolution == RESOLUTION_16BIT) {
                out_frame_cnt = updn_samp_prcs_16b(port[port_select]->smp_instance_ptr[j], port[port_select]->is_upsample, port[port_select]->factor, (S16 *)in_data_buffer, (S16 *)stream_function_get_inout_buffer(para, j + 1), in_frame_size / sample_size);
            } else {
                out_frame_cnt = updn_samp_prcs_32b(port[port_select]->smp_instance_ptr[j], port[port_select]->is_upsample, port[port_select]->factor, (S32 *)in_data_buffer, (S32 *)stream_function_get_inout_buffer(para, j + 1), in_frame_size / sample_size);
            }

#if SRC_FIXED_RATIO_DUMP_ENABLE
            LOG_AUDIO_DUMP((U8 *)stream_function_get_inout_buffer(para, j + 1), out_frame_cnt * sample_size, SRC_FIXED_RATIO_P1_OUT1 + j + (m * SMP_MAX_CH_NUM << 1)); //DUMP ID: SRC_FIXED_RATIO_P(m*8)_OUT(j+1)
#endif
            //DSP_MW_LOG_I("[SRC_FIXED_RATIO]m=%d, j=%d, in:%d(dump:%d), out:%d(dump:%d)", 6, m, j, in_frame_size / sample_size, SRC_FIXED_RATIO_P1_IN1 + j + (m * SMP_MAX_CH_NUM << 1), out_frame_cnt, SRC_FIXED_RATIO_P1_OUT1 + j + (m * SMP_MAX_CH_NUM << 1));
        }
        //DSP_MW_LOG_I("[SRC_FIXED_RATIO]out_frame_cnt %d, sample_size %d", 2, out_frame_cnt, sample_size);

        res = stream_function_modify_output_size(para, out_frame_cnt * sample_size);
        ((DSP_ENTRY_PARA_PTR)para)->codec_out_sampling_rate = port[port_select]->out_sampling_rate / 1000;



#if SRC_FIXED_RATIO_DEBUG_LOG
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
        DSP_MW_LOG_I("[SRC_FIXED_RATIO][finish]:%u, %u, %u", 3,
                     ((DSP_ENTRY_PARA_PTR)para)->codec_out_sampling_rate,
                     out_frame_cnt * sample_size,
                     current_timestamp);
#endif /* SW_SRC_DEBUG_LOG */

    } else { // consecutive converters
        if (port[cvt_start_index + 1] == NULL) {
            AUDIO_ASSERT(0 && "[SRC_FIXED_RATIO] 2nd Port is not found!");
        }

        if(port[cvt_start_index]->multi_cvt_mode == SRC_FIXED_RATIO_PORT_MUTI_CVT_MODE_CONSECUTIVE_AND_ALTERNATE) {
            if(cvt_start_index == 0) {
                n = port[cvt_start_index]->cvt_num - 1;
            } else {
                n -= port[0]->cvt_num;
            }
        }
        if (port[cvt_start_index]->cvt_num != (n + 1)) {
#if SRC_FIXED_RATIO_DEBUG_LOG
            DSP_MW_LOG_E("[SRC_FIXED_RATIO] converter number is not match! %d != %d", 2, port[cvt_start_index]->cvt_num, (n + 1));
#endif
            AUDIO_ASSERT(0 && "[SRC_FIXED_RATIO] converter number is not match!");
        }

        sample_size = (port[cvt_start_index]->resolution == RESOLUTION_16BIT) ? sizeof(int16_t) : sizeof(int32_t);

        for (m = cvt_start_index; m < (port[cvt_start_index]->cvt_num + (uint16_t)cvt_start_index); m++) {
            for (j = 0; j < (int32_t)port[m]->channel_number; j++) {
                memset(in_data_buffer, 0, port[0]->tmp_buff_size);
                memcpy(in_data_buffer, stream_function_get_inout_buffer(para, j + 1), in_frame_size);

#if SRC_FIXED_RATIO_DUMP_ENABLE
                LOG_AUDIO_DUMP((U8 *)stream_function_get_inout_buffer(para, j + 1), in_frame_size, SRC_FIXED_RATIO_P1_IN1 + j + (m * SMP_MAX_CH_NUM << 1)); //DUMP ID: SRC_FIXED_RATIO_P(m*8)_IN(j+1)
#endif

                if (port[m]->factor != 1) {
                    if (port[cvt_start_index]->resolution == RESOLUTION_16BIT) {
                        out_frame_cnt = updn_samp_prcs_16b(port[m]->smp_instance_ptr[j], port[m]->is_upsample, port[m]->factor, (S16 *)in_data_buffer, (S16 *)stream_function_get_inout_buffer(para, j + 1), in_frame_size / sample_size);
                    } else {
                        out_frame_cnt = updn_samp_prcs_32b(port[m]->smp_instance_ptr[j], port[m]->is_upsample, port[m]->factor, (S32 *)in_data_buffer, (S32 *)stream_function_get_inout_buffer(para, j + 1), in_frame_size / sample_size);
                    }
                }

#if SRC_FIXED_RATIO_DUMP_ENABLE
                LOG_AUDIO_DUMP((U8 *)stream_function_get_inout_buffer(para, j + 1), out_frame_cnt * sample_size, SRC_FIXED_RATIO_P1_OUT1 + j + (m * SMP_MAX_CH_NUM << 1)); //DUMP ID: SRC_FIXED_RATIO_P(m*8)_OUT(j+1)
#endif

            }
            if (port[m]->factor != 1) {
                in_frame_size = out_frame_cnt * sample_size;
            }
#if SRC_FIXED_RATIO_DEBUG_LOG
            DSP_MW_LOG_I("[SRC_FIXED_RATIO][cvt%d]:%u, %u %d %d", 5,
                         m,
                         port[m]->out_sampling_rate,
                         in_frame_size,
                         out_frame_cnt,
                         sample_size);
#endif
        }

        res = stream_function_modify_output_size(para, in_frame_size);
        ((DSP_ENTRY_PARA_PTR)para)->codec_out_sampling_rate = port[n]->out_sampling_rate / 1000;

#if SRC_FIXED_RATIO_DEBUG_LOG
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
        DSP_MW_LOG_I("[SRC_FIXED_RATIO][finish]:%u, %u, %u", 3,
                     ((DSP_ENTRY_PARA_PTR)para)->codec_out_sampling_rate,
                     in_frame_size,
                     current_timestamp);
#endif /* SW_SRC_DEBUG_LOG */
    }

    return 0;
}
