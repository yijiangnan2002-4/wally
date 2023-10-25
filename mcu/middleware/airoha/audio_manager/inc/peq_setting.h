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
#ifndef __PEQ_SETTING_H__
#define __PEQ_SETTING_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bt_sink_srv_ami.h"
#include "bt_sink_srv_am_task.h"
#include "audio_nvdm_common.h"
#include "hal_audio_cm4_dsp_message.h"
#include "audio_log.h"

#if defined(MTK_PEQ_ENABLE) || defined(MTK_LINEIN_PEQ_ENABLE) || defined(AIR_ADVANCED_PASSTHROUGH_ENABLE)
void aud_peq_init(void);
int32_t aud_set_peq_param(peq_audio_path_id_t audio_path, bt_sink_srv_am_peq_param_t *ami_peq_param);
peq_audio_path_id_t aud_get_peq_audio_path(bt_sink_srv_am_type_t type);
int32_t aud_set_peq_param(peq_audio_path_id_t audio_path, bt_sink_srv_am_peq_param_t *ami_peq_param);
int32_t aud_set_special_peq_param(uint8_t special_peq_audio_id, uint16_t keyid);
int32_t aud_peq_get_total_mode(peq_audio_path_id_t audio_path, uint8_t phase_id);
uint8_t aud_peq_get_current_sound_mode(peq_audio_path_id_t audio_path, uint8_t phase_id);
#ifdef AIR_ADAPTIVE_EQ_ENABLE
int32_t aud_set_adaptive_aeq_param();
void aud_aeq_save_misc();
void aud_send_aeq_share_info();
uint8_t aud_get_aeq_detect_status(void);
int32_t aud_get_aeq_sound_mode(bt_sink_srv_am_type_t type, uint8_t *peq_info);
uint8_t aud_set_aeq_detect_ip(uint8_t aeq_ip_id);
uint8_t aud_get_aeq_detect_ip_status(void);
uint8_t aud_get_aeq_detect_bypass_status(void);
uint8_t aud_set_aeq_detect_bypass(uint8_t aeq_detect_bypass_id);
#endif
void aud_peq_save_misc_param(void);
#ifdef SUPPORT_PEQ_NVKEY_UPDATE
void aud_peq_chk_nvkey(uint16_t nvkey_id);
uint32_t aud_peq_get_changed_nvkey(uint8_t **packet, uint32_t *total_size);
uint32_t aud_peq_save_changed_nvkey(AMI_AWS_MCE_ATTACH_NVDM_PACKT_t *attach_nvdm_pkt, uint32_t data_size);
#endif
uint32_t aud_peq_get_sound_mode(bt_sink_srv_am_type_t type, uint8_t *peq_info);
uint8_t aud_peq_get_peq_status(peq_audio_path_id_t audio_path, uint8_t phase_id);
#ifdef AIR_PSAP_ENABLE
int32_t music_get_feature_mode(void);
bool music_set_feature_mode(uint8_t mode);
#endif
#endif

extern void audio_set_anc_compensate(bt_sink_srv_am_type_t type, uint32_t event, bt_sink_srv_am_type_t *cur_type);
#ifdef MTK_ANC_ENABLE
extern void audio_set_anc_compensate_phase2(bt_sink_srv_am_type_t type, uint32_t event);
#endif

#if defined(AIR_HEARTHROUGH_VIVID_PT_ENABLE)
extern int32_t aud_set_vivid_pt_peq_param(void);
#endif

bt_sink_srv_am_result_t am_audio_cpd_init(void);

#ifdef __cplusplus
}
#endif

#endif  /*__PEQ_SETTING_H__*/