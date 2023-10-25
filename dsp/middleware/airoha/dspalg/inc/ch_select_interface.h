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

#ifndef _CH_SEL_INTERFACE_H_
#define _CH_SEL_INTERFACE_H_

#include "hal_ccni.h"
#include "dsp_feature_interface.h"
#include "dsp_utilities.h"

typedef enum {
    CH_SEL_STEREO = 0,
    CH_SEL_MONO,
    CH_SEL_BOTH_L,
    CH_SEL_BOTH_R,
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
    CH_SEL_BOTH_Ref_Mic3,
    CH_SEL_BOTH_Ref_Mic4,
    CH_SEL_BOTH_Ref_Mic5,
    CH_SEL_BOTH_Ref_Mic6,
#endif
    CH_SEL_SWAP,
    CH_SEL_ONLY_L,
    CH_SEL_ONLY_R,
    CH_SEL_NUM,


    CH_SEL_NOT_USED = 0xFF,

} CH_SEL_MODE;

typedef enum
{
    CH_SEL_A2DP = 0,
    CH_SEL_HFP  = 1,
    CH_SEL_VP   = 2,
    CH_SEL_WIRELESS_MIC = 3,
    CH_SEL_USB_MIC   = 4,
} CH_SEL_SCENARIO_TYPE;

typedef struct ch_sel_ctrl_s
{
    CH_SEL_MODE ch_mode;
} CH_SEL_CTRL_t;

EXTERN bool stream_function_channel_selector_initialize_a2dp (void *para);
EXTERN bool stream_function_channel_selector_initialize_hfp (void *para);
EXTERN bool stream_function_channel_selector_initialize_vp (void *para);
EXTERN bool stream_function_channel_selector_initialize (void *para, CH_SEL_SCENARIO_TYPE type);
EXTERN bool stream_function_channel_selector_process_a2dp (void *para);
EXTERN bool stream_function_channel_selector_process_hfp (void *para);
EXTERN bool stream_function_channel_selector_process_vp (void *para);
EXTERN bool stream_function_channel_selector_process (void *para, CH_SEL_SCENARIO_TYPE type);
#ifdef AIR_WIRELESS_MIC_TX_ENABLE
EXTERN bool stream_function_channel_selector_initialize_wireless_mic (void *para);
EXTERN bool stream_function_channel_selector_process_wireless_mic (void *para);
#endif
#ifdef AIR_WIRED_AUDIO_ENABLE
EXTERN bool stream_function_channel_selector_initialize_usb_mic(void *para);
EXTERN bool stream_function_channel_selector_process_usb_mic(void *para);
#endif
EXTERN void Ch_Select_Set_Param (hal_ccni_message_t msg, hal_ccni_message_t *ack);
EXTERN CH_SEL_MODE Ch_Select_Get_Param (CH_SEL_SCENARIO_TYPE scenario);

#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
EXTERN bool stream_function_channel_selector_2ch_to_4ch_process (void *para);
EXTERN bool stream_function_channel_selector_2ch_to_4ch_initialize (void *para);
EXTERN bool stream_function_channel_selector_2ch_to_4ch_add_latency_process (void *para);
EXTERN bool stream_function_channel_selector_2ch_to_4ch_add_latency_initialize (void *para);
#endif

#endif /* _CH_SEL_INTERFACE_H_ */
