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

#ifndef __BT_SINK_SRV_NVKEY_STRUCT_H__
#define __BT_SINK_SRV_NVKEY_STRUCT_H__

#ifndef PACKED
#define PACKED  __attribute__((packed))
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "bt_sink_srv_a2dp.h"

/* NvkeyDefine NVID_BT_HOST_MUSIC_AAC_CONFIG */
typedef struct {
    bt_sink_srv_a2dp_aac_object_type_t aac_type; //default is BT_SINK_SRV_A2DP_MPEG_2_AND_4_LC
    uint32_t                           bit_rate; //default is 0x60000 (384k)
} PACKED bt_sink_srv_a2dp_aac_config_parameter_t;

/* NvkeyDefine NVID_BT_HOST_MUSIC_SBC_CONFIG */
typedef struct {
    uint8_t min_bitpool; //default is 8
    uint8_t max_bitpool; //default is 75
} PACKED bt_sink_srv_a2dp_sbc_config_parameter_t;

/* NvkeyDefine NVID_DSP_ALG_AUD_LATENCY */
typedef struct {
    uint8_t  reserved;
    uint16_t normal_latency;
    uint16_t normal_latency_extend;
    uint16_t game_latency;
    uint16_t game_latency_extend;
    uint16_t vendor_latency;
    uint16_t vendor_latency_extend;
    uint16_t vendor_game_latency;
    uint16_t vendor_game_latency_extend;
    uint16_t speaker_latency;
    uint16_t speaker_latency_extend;
} PACKED bt_sink_srv_music_latency_t;

#ifdef __cplusplus
}
#endif

#endif /*__BT_DEVICE_MANAGER_NVKEY_STRUCT_H__*/


