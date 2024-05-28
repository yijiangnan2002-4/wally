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
#ifndef __VOICE_NR_INTERFACE_H__
#define __VOICE_NR_INTERFACE_H__

#include "dsp_feature_interface.h"
#include "dsp_utilities.h"
#include "voice_nr_driver.h"
#include "dsp_share_memory.h"

/* Voice NR control: Query Lib version */
uint32_t voice_nr_query_version(dsp_alg_lib_type_t type, uint8_t *version);

/* Voice NR control: CM4 Set NR feature */
void voice_nr_set_param(hal_ccni_message_t msg, hal_ccni_message_t *ack);

/* Voice NR control: airdump */
#ifdef AIR_AIRDUMP_ENABLE
typedef struct {
    uint32_t read_offset;
    uint32_t write_offset;
    uint16_t length;
    uint8_t  notify_count;
    uint8_t  data[270];
} AIRDUMPCTRL_t;
void voice_nr_airdump_on_off(bool state);
void voice_nr_airdump(void);
#endif

/* Voice NR control: NR on/off control */
void voice_nr_enable(hal_ccni_message_t msg, hal_ccni_message_t *ack);
bool voice_nr_check_enable(void);
bool voice_nr_check_mp(void);		// richard for patch from Airoha

/* Voice NR control: Config tool get the reference gain value */
void voice_nr_get_ref_gain(int16_t *gain_addr);

/* Voice NR control: AVC volume control */
void voice_avc_vol_send_update(int16_t pre_vol, int16_t post_vol, int16_t scenario_type);
void voice_avc_vol_receive_update(int16_t avc_vol);
void voice_avc_set_enable(bool enable);
bool voice_avc_check_enable(void);

/* Feature init and process for ECNR */
bool stream_function_aec_nr_initialize(void *para);
void stream_function_aec_nr_deinitialize(bool is_aec_only);
bool stream_function_aec_process(void *para);
#if defined(AIR_3RD_PARTY_NR_ENABLE) && !defined(AIR_BTA_IC_STEREO_HIGH_G3)
bool stream_function_tx_eq_initialize(void *para);
void stream_function_tx_eq_deinitialize(bool is_aec_only);
bool stream_function_tx_eq_process(void *para);
#if defined(AIR_ECNR_PREV_PART_ENABLE)
uint8_t stream_function_ecnr_prev_get_postec_gain(void);
bool stream_function_ecnr_prev_initialize(void *para);
void stream_function_ecnr_prev_deinitialize(bool is_aec_only);
bool stream_function_ecnr_prev_process(void *para);
#endif
#if defined(AIR_ECNR_POST_PART_ENABLE)
bool stream_function_ecnr_post_initialize(void *para);
void stream_function_ecnr_post_deinitialize(bool is_aec_only);
bool stream_function_ecnr_post_process(void *para);
#endif
#endif

/* Feature init and process for RXNR */
bool stream_function_nr_initialize(void *para);
bool stream_function_nr_process(void *para);


#endif /* _AEC_NR_INTERFACE_H_ */
