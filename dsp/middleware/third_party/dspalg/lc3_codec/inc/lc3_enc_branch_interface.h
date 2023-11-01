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

#ifndef _LC3_ENC_BRANCH_INTERFACE_H_
#define _LC3_ENC_BRANCH_INTERFACE_H_

#ifdef AIR_BT_CODEC_BLE_V2_ENABLED

/* Includes ------------------------------------------------------------------*/
#include "lc3_codec_api.h"
#ifdef MTK_BT_A2DP_LC3_USE_PIC
#include "lc3_codec_portable.h"
#endif

/* Public define -------------------------------------------------------------*/
#define LC3_ENC_BRANCH_USER_COUNT       4

/* Public typedef ------------------------------------------------------------*/
typedef enum {
    LC3_ENC_BRANCH_INDEX_0,
    LC3_ENC_BRANCH_INDEX_MAX,
} lc3_enc_branch_index_t;

typedef enum {
    LC3_ENC_BRANCH_STATUS_DEINIT = 0,
    LC3_ENC_BRANCH_STATUS_INIT = 1,
    LC3_ENC_BRANCH_STATUS_RUNNING = 2
} lc3_enc_branch_status_t;

typedef struct {
    uint16_t sample_bits;
    uint16_t channel_no;
    uint32_t sample_rate;
    uint32_t bit_rate;
    uint16_t frame_interval;
    LC3_DELAY_COMPENSATION_T delay;
    uint16_t buffer_size;
} lc3_enc_branch_config_t;

typedef struct {
    uint8_t enc_count;
    lc3_enc_branch_status_t enc_status;
    uint16_t sample_bits;
    uint16_t channel_no;
    uint32_t sample_rate;
    uint32_t bit_rate;
    uint16_t frame_interval;
    LC3_DELAY_COMPENSATION_T delay;
    uint32_t in_frame_size;
    uint32_t out_frame_size;
    uint32_t input_samples;
    uint16_t work_buffer_size;
    uint16_t remain_buffer_size;
    void    *work_mem_ptr;
    void    *remain_mem_ptr;
    uint32_t finish_gpt_count;
    void    *user[LC3_ENC_BRANCH_USER_COUNT];
} lc3_enc_branch_para_t;

/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
extern void stream_codec_encoder_lc3_branch_init(lc3_enc_branch_index_t index, void *user, lc3_enc_branch_config_t *config);
extern void stream_codec_encoder_lc3_branch_deinit(lc3_enc_branch_index_t index, void *user);
extern void stream_codec_encoder_lc3_branch_get_data_info(lc3_enc_branch_index_t index, uint32_t channel, uint8_t **in_buffer_address, uint32_t *in_frame_size);
extern bool stream_codec_encoder_lc3_branch_initialize(void *para);
extern bool stream_codec_encoder_lc3_branch_process(void *para);

#endif /* AIR_BT_CODEC_BLE_V2_ENABLED */

#endif /* _LC3_ENC_BRANCH_INTERFACE_H_ */
