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

#ifndef __AUDIO_SRC_SRV_RESOURCE_MANAGER_CONFIG_H__
#define __AUDIO_SRC_SRV_RESOURCE_MANAGER_CONFIG_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


#define AUDIO_SRC_SRV_RESOURCE_USR_MAX  6

typedef enum {
    AUDIO_SRC_SRV_RESOURCE_TYPE_MIC = 0,
    AUDIO_SRC_SRV_RESOURCE_TYPE_WIRED_AUDIO = 1,
    AUDIO_SRC_SRV_RESOURCE_TYPE_RESERVED = 2,
    AUDIO_SRC_SRV_RESOURCE_TYPE_BT_SOURCE = 3,
    AUDIO_SRC_SRV_RESOURCE_TYPE_MAX
} audio_src_srv_resource_type_t;

#define AUDIO_SRC_SRV_RESOURCE_TYPE_MIC_USER_HFP_DUAL_CHIP     "mic_user_hfp_dual_chip"
#define AUDIO_SRC_SRV_RESOURCE_TYPE_MIC_USER_HFP     "mic_user_hfp"
#define AUDIO_SRC_SRV_RESOURCE_TYPE_MIC_USER_ULL_UL  "mic_user_ull_ul"
#define AUDIO_SRC_SRV_RESOURCE_TYPE_MIC_USER_RECORD  "mic_user_record"

#define AUDIO_SRC_SRV_RESOURCE_TYPE_BT_SOURCE_USER_HFP      "bt_source_user_hfp"
#define AUDIO_SRC_SRV_RESOURCE_TYPE_BT_SOURCE_USER_A2DP     "bt_source_user_a2dp"

#define AUDIO_SRC_SRV_RESOURCE_TYPE_MIC_USER_HFP_PRIORIRT     6
#define AUDIO_SRC_SRV_RESOURCE_TYPE_MIC_USER_ULL_UL_PRIORITY  5
#define AUDIO_SRC_SRV_RESOURCE_TYPE_MIC_USER_RECORD_PRIORITY  4
#define AUDIO_SRC_SRV_RESOURCE_TYPE_MIC_USER_VA_PRIORITY      3

#define AUDIO_SRC_SRV_RESOURCE_TYPE_BT_SOURCE_USER_HFP_PRIORIRTY     6
#define AUDIO_SRC_SRV_RESOURCE_TYPE_BT_SOURCE_USER_A2DP_PRIORIRTY    5


#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_SRC_SRV_RESOURCE_MANAGER_CONFIG_H__ */

