/* Copyright Statement:
 *
 * (C) 2017  Airoha Technology Corp. All rights reserved.
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

#ifndef __BT_HFP_CODEC_INTERNAL_H__
#define __BT_HFP_CODEC_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "syslog.h"
#include <string.h>

#include "bt_codec.h"
#include "audio_nvdm_coef.h"
#include "hal_audio.h"

#ifndef HAL_AUDIO_MODULE_ENABLED
#error "please turn on audio feature option on hal_feature_config.h"
#endif

#include "hal_audio_cm4_dsp_message.h"
#include "hal_audio_internal.h"

typedef enum {
    BT_HFP_MODE_NONE = 0,
    BT_HFP_MODE_SPEECH,
    BT_HFP_MODE_TX_ONLY,
    BT_HFP_MODE_RX_ONLY,
    BT_HFP_MODE_LOOPBACK_WITH_CODEC,
    BT_HFP_MODE_LOOPBACK_WITHOUT_CODEC
} bt_hfp_mode_t;

typedef void (*bt_codec_callback_t)(void *parameter);

typedef struct {
    bt_media_handle_t                  handle;
    bt_hfp_audio_codec_type_t          codec_type;
    bt_hfp_mode_t                      mode;
    bool                                aws_flag;
} bt_hfp_codec_internal_handle_t;

bool bt_hfp_codec_query_is_running(void);
hal_audio_sampling_rate_t bt_hfp_codec_query_sampling_rate(void);
hal_audio_channel_number_t bt_hfp_codec_query_channel_number(void);

#ifdef AIR_HFP_FEATURE_MODE_ENABLE
typedef enum {
    HFP_FEATURE_MODE_NORMAL,
    HFP_FEATURE_MODE_PSAP_0,
    HFP_FEATURE_MODE_MAX,
} hfp_feature_mode_t;

hfp_feature_mode_t hfp_get_feature_mode(void);
bool hfp_set_feature_mode(hfp_feature_mode_t mode);
#endif

#if defined(AIR_HFP_FEATURE_MODE_ENABLE) || defined(AIR_AUDIO_DETACHABLE_MIC_ENABLE)
void hfp_replace_feature_mode_nvkey_id(bt_hfp_audio_codec_type_t codec_type, DSP_FEATURE_TYPE_LIST *p_feature_list);
uint16_t hfp_restore_feature_mode_nvkey_id(uint16_t nvkey_id);
#endif

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif  /*__BT_HFP_CODEC_INTERNAL_H__*/
