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

#ifndef __AUDIO_SET_DRIVER_H__
#define __AUDIO_SET_DRIVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bt_sink_srv_ami.h"
#include "hal_resource_assignment.h"

#ifdef AIR_KEEP_I2S_ENABLE
void audio_driver_set_device(hal_audio_device_t set_device, hal_audio_interface_t set_interface, bool set_enable);
void hal_audio_set_audio_device(hal_audio_device_t set_device, hal_audio_interface_t set_interface, bool set_enable);
#endif
#if defined(MTK_AMP_DC_COMPENSATION_ENABLE)
void audio_driver_dc_compensation(void);
void hal_audio_set_dc_compensation(void);
#endif

#if defined(AIR_DAC_MODE_RUNTIME_CHANGE)
#ifndef MTK_ANC_SURROUND_MONITOR_ENABLE
typedef enum {
    AUDIO_ANC_TYPE_ANC            = 0,
    AUDIO_ANC_TYPE_PT             = 1,
    AUDIO_ANC_TYPE_SIDETONE       = 2,
    AUDIO_ANC_TYPE_HA_PSAP        = 3,
    AUDIO_ANC_TYPE_SW_VIVID_PT    = 4,
    AUDIO_ANC_TYPE_HW_VIVID_PT    = 5,
    AUDIO_ANC_TYPE_MAX,
} audio_anc_type_t;
#endif
void hal_audio_status_send_update_dac_mode_event_to_am(hal_audio_ha_dac_flag_t ha_dac_flag,  bool enable);
void hal_audio_status_set_ha_flag_and_update_dac_mode(hal_audio_ha_dac_flag_t ha_dac_flag,  bool enable);
void hal_audio_status_update_dac_mode(void);
#endif

#ifdef __cplusplus
}
#endif

#endif  /*__AUDIO_SET_DRIVER_H__*/
