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

#ifndef _LC3_DEC_INTERFACE_V2_H_
#define _LC3_DEC_INTERFACE_V2_H_

#ifdef AIR_BT_CODEC_BLE_V2_ENABLED

/* Includes ------------------------------------------------------------------*/
#include "dsp_utilities.h"
#include "dsp_buffer.h"
#include "dsp_feature_interface.h"
#include "lc3_codec_api.h"
#ifdef MTK_BT_A2DP_LC3_USE_PIC
#include "lc3_codec_portable.h"
#endif /* MTK_BT_A2DP_LC3_USE_PIC */

/* Public define -------------------------------------------------------------*/
#define LC3_DEC_USER_COUNT       4

/* Public typedef ------------------------------------------------------------*/
typedef enum {
    LC3_DEC_STATUS_ERROR = 0,
    LC3_DEC_STATUS_OK = 1
} lc3_dec_status_t;

typedef enum {
    LC3_DEC_PORT_0,
    LC3_DEC_PORT_MAX,
} lc3_dec_port_t;

typedef enum {
    LC3_DEC_CHANNEL_MODE_MONO = 0,
    LC3_DEC_CHANNEL_MODE_STEREO,
    LC3_DEC_CHANNEL_MODE_DUAL_MONO
} lc3_dec_channel_mode_t;

typedef enum {
    LC3_DEC_PORT_STATUS_DEINIT = 0,
    LC3_DEC_PORT_STATUS_INIT = 1,
    LC3_DEC_PORT_STATUS_RUNNING = 2
} lc3_dec_port_status_t;

typedef enum {
    LC3_DEC_FRAME_STATUS_NORMAL = 0,
    LC3_DEC_FRAME_STATUS_PLC = 1,
    LC3_DEC_FRAME_STATUS_BYPASS_DECODER = 2
} lc3_dec_frame_status_t;

typedef struct {
    uint32_t sample_rate;
    uint32_t sample_bits;
    uint32_t bit_rate;
    LC3_DEC_MODE_T dec_mode;
    lc3_dec_channel_mode_t channel_mode;
    LC3_DELAY_COMPENSATION_T delay;
    uint32_t in_channel_num;
    uint32_t out_channel_num;
    uint32_t frame_interval;
    uint32_t frame_size;
    uint32_t frame_samples;
    uint32_t plc_enable;
    LC3_PLC_MODE_T plc_mode;
} lc3_dec_port_config_t;

typedef struct {
    uint8_t count;
    lc3_dec_port_status_t status;
    void   *user[LC3_DEC_USER_COUNT];
    uint32_t finish_gpt_count;
    uint32_t sample_rate;
    uint32_t sample_bits;
    uint32_t bit_rate;
    uint32_t frame_samples;
    uint32_t frame_interval;
    uint32_t frame_size;
    LC3_DEC_MODE_T dec_mode;
    lc3_dec_channel_mode_t channel_mode;
    LC3_DELAY_COMPENSATION_T delay;
    uint32_t in_channel_num;
    uint32_t out_channel_num;
    LC3_PLC_MODE_T plc_mode;
    uint32_t plc_enable;
    uint32_t work_instance_count;
    uint32_t work_buffer_size;
    void   *work_mem_ptr;
} lc3_dec_port_para_t;

/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
extern lc3_dec_status_t stream_codec_decoder_lc3_v2_init(lc3_dec_port_t port, void *user, lc3_dec_port_config_t *config);
extern lc3_dec_status_t stream_codec_decoder_lc3_v2_deinit(lc3_dec_port_t port, void *user);
extern bool stream_codec_decoder_lc3_v2_initialize(void *para);
extern bool stream_codec_decoder_lc3_v2_process(void *para);

#endif /* AIR_BT_CODEC_BLE_V2_ENABLED */

#endif /* _LC3_DEC_INTERFACE_V2_H_ */
