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

#ifndef _COMPANDER_INTERFACE_SW_H_
#define _COMPANDER_INTERFACE_SW_H_

#if defined(AIR_SOFTWARE_DRC_ENABLE)

/* Includes ------------------------------------------------------------------*/
#include "dsp_utilities.h"
#include "dsp_buffer.h"
#include "dsp_feature_interface.h"
#include "dsp_callback.h"
#include "dsp_sdk.h"
#include "audio_nvdm_common.h"
#include <stdint.h>

/* Public define -------------------------------------------------------------*/
#define SW_COMPANDER_PORT_MAX         3

/* Public typedef ------------------------------------------------------------*/
typedef enum {
    SW_COMPANDER_STATUS_ERROR = 0,
    SW_COMPANDER_STATUS_OK = 1
} sw_compander_status_t;

typedef enum {
    SW_COMPANDER_AUDIO_MODE = 0,
    SW_COMPANDER_VOICE_MODE,
} sw_compander_mode;

typedef enum {
    SW_COMPANDER_VOICE_WORK_MODE_MAX,
} sw_compander_voice_work_mode;

typedef enum {
    SW_COMPANDER_PORT_STATUS_DEINIT = 0,
    SW_COMPANDER_PORT_STATUS_INIT = 1,
    SW_COMPANDER_PORT_STATUS_RUNNING = 2
} sw_compander_port_status_t;

typedef struct {
    sw_compander_mode mode;
    sw_compander_voice_work_mode voice_work_mode;
    uint32_t channel_num;
    uint32_t sample_rate;
    uint32_t frame_base;
    int32_t recovery_gain;
    int32_t vol_default_gain;
    DSP_ALG_NVKEY_e default_nvkey_id;
    void *default_nvkey_mem;
} sw_compander_config_t;

typedef struct {
    sw_compander_port_status_t status;
    void *owner;
    DSP_STREAMING_PARA_PTR stream;
    sw_compander_mode mode;
    sw_compander_voice_work_mode voice_work_mode;
    uint32_t channel_num;
    uint32_t sample_rate;
    uint32_t frame_base;
    int32_t recovery_gain;
    int32_t vol_target;
    DSP_ALG_NVKEY_e nvkey_id;
    void *nvkey_mem_ptr;
    uint32_t work_instance_count;
    uint32_t work_mem_size;
    void *work_mem_ptr;
} sw_compander_port_t;

/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
extern sw_compander_port_t *stream_function_sw_compander_get_port(void *owner);
extern sw_compander_status_t stream_function_sw_compander_init(sw_compander_port_t *port, sw_compander_config_t *config);
extern sw_compander_status_t stream_function_sw_compander_deinit(sw_compander_port_t *port);
extern bool stream_function_sw_compander_initialize(void *para);
extern bool stream_function_sw_compander_process(void *para);

#endif /* AIR_SOFTWARE_DRC_ENABLE */

#endif /* _COMPANDER_INTERFACE_SW_H_ */
