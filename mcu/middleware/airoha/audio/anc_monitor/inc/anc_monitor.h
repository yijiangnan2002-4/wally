/* Copyright Statement:
 *
 * (C) 2020  Airoha Technology Corp. All rights reserved.
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

#ifndef __ANC_MONITOR_H__
#define __ANC_MONITOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_audio.h"
#include "audio_transmitter_playback_port.h"
#include "audio_log.h"
#include "bt_sink_srv_ami.h"
#include "audio_transmitter_mcu_dsp_common.h"

#if defined(MTK_ANC_SURROUND_MONITOR_ENABLE)
#include "audio_nvdm_coef.h"
#include "audio_nvdm_common.h"
#include "hal_audio_message_struct.h"
#ifdef MTK_ANC_V2
#include "anc_control_api.h"
#if defined(AIR_ANC_USER_UNAWARE_ENABLE) || defined(AIR_ANC_WIND_DETECTION_ENABLE) || defined(AIR_ANC_ENVIRONMENT_DETECTION_ENABLE)
#define ADAPIVE_ANC_STREAM_CONTROL TRUE //for MP testing
#endif
#else
/*
typedef anc_control_result_t audio_anc_control_result_t;
typedef anc_control_event_t  audio_anc_control_event_t;
#define AUDIO_ANC_CONTROL_EXECUTION_SUCCESS ANC_CONTROL_EXECUTION_SUCCESS
#define AUDIO_ANC_CONTROL_EXECUTION_NOT_ALLOWED ANC_CONTROL_EXECUTION_NOT_ALLOWED
*/
#endif

// ======== Static Structure ========
#ifdef AIR_ANC_USER_UNAWARE_ENABLE
typedef struct {
    U8 ENABLE;                      /**< @Value   0x01 @Desc 1 */
    U8 REVISION;                    /**< @Value   0x01 @Desc 1 */

    S16 alpha_par;                  /**< @Value   983      @Desc alpha par*/
    S32 thd;                        /**< @Value   500000 @Desc switch threshold */
    S32 switch_par;                 /**< @Value   18761   @Desc switch par */
    U16 pause_attenuation;          /**< @Value     0  @Desc Set 1 to suspend apply gain attenuation*/
    U16 _reserved0;                 /**< @Value     0  @Desc 0 */
} PACKED user_unaware_para_t;
#endif

#if defined(AIR_ANC_USER_UNAWARE_ENABLE) || defined(AIR_ANC_WIND_DETECTION_ENABLE) || defined(AIR_ANC_ENVIRONMENT_DETECTION_ENABLE)
typedef struct stru_adaptive_anc_status_s {
    uint8_t wnd_enable;
    uint8_t uu_enable;
    uint8_t ed_enable;

    uint8_t wnd_suspend;
    uint8_t uu_suspend;
    uint8_t ed_suspend;
} PACKED audio_anc_monitor_adaptive_anc_status_t;
#endif

typedef struct {
    audio_transmitter_id_t receive_id;
    bool is_enable;
} audio_transmitter_monitor_config_t;

typedef enum {
    AUDIO_ANC_MONITOR_FEATURE_TYPE_WIND_DETECT = 1,
    AUDIO_ANC_MONITOR_FEATURE_TYPE_USER_UNAWARE = 2,
    AUDIO_ANC_MONITOR_FEATURE_TYPE_ENVIRONMENT_DETECT = 3,
    AUDIO_ANC_MONITOR_FEATURE_TYPE_NUM,
    AUDIO_ANC_MONITOR_FEATURE_TYPE_MAX = 0xFFFF,
} audio_anc_monitor_feature_type_t;

typedef struct {
    uint16_t type; //enum: audio_anc_monitor_feature_type_t
    int16_t data[2];
} audio_anc_monitor_inform_t, *audio_anc_monitor_inform_ptr_t;

typedef enum {
    AUDIO_USER_UNAWARE_INFORM_SUBTYPE_SET_STAT  = 0,
    AUDIO_USER_UNAWARE_INFORM_SUBTYPE_ENABLE,
    AUDIO_USER_UNAWARE_INFORM_SUBTYPE_MAX = 0xFFFF,
} audio_user_unaware_inform_subtype_t;

typedef enum {
    AUDIO_USER_UNAWARE_GET_INFO_TYPE_GAIN_INFO = 0,
    AUDIO_USER_UNAWARE_GET_INFO_TYPE_ENABLE_STAT,
} audio_user_unaware_get_info_type_t;

typedef enum {
    AUDIO_ANC_MONITOR_SET_INFO = 1,
    AUDIO_ANC_MONITOR_GET_INFO = 2,
    AUDIO_ANC_MONITOR_SET_PARAMS = 3,
    AUDIO_ANC_MONITOR_EVENT_MAX,
} audio_anc_monitor_event_t;

typedef enum {
    AUDIO_ANC_MONI_SCENARIO_TYPE_ANC            = 0,
    AUDIO_ANC_MONI_SCENARIO_TYPE_PT             = 1,
    AUDIO_ANC_MONI_SCENARIO_TYPE_SIDETONE       = 2,
    AUDIO_ANC_MONI_SCENARIO_TYPE_HA_PSAP        = 3,
    AUDIO_ANC_MONI_SCENARIO_TYPE_SW_VIVID_PT    = 4,
    AUDIO_ANC_MONI_SCENARIO_TYPE_HW_VIVID_PT    = 5,
    AUDIO_ANC_MONI_SCENARIO_TYPE_LLF            = 6,
    AUDIO_ANC_MONI_SCENARIO_TYPE_MAX,
} audio_anc_monitor_scenario_type_t;



#define AUDIO_USER_UNAWARE_RACE_EXTEND_SUBMODE_GET_EN_STAT    (0)

#ifdef AIR_ANC_USER_UNAWARE_ENABLE
typedef struct {
    int log0[2];
    int log2;
    int log3;
    int log4;
} user_unaware_info;
#endif

// ======== variable Declaration ========
extern uint32_t anc_monitor_message_date;


// ======== Public functions ========
void audio_anc_monitor_anc_init(void);
void audio_anc_monitor_anc_callback(audio_anc_control_event_t event_id, audio_anc_control_result_t result);
void audio_anc_monitor_set_info(audio_anc_monitor_set_info_t set_type, uint32_t para);
uint32_t audio_anc_monitor_get_info(audio_anc_monitor_get_info_t info);
void audio_anc_monitor_transmitter_send_ccni(uint16_t nvkey_id);
void audio_anc_monitor_get_anc_status(uint8_t *enable, audio_anc_monitor_scenario_type_t *anc_type, uint32_t *misc1, uint32_t *misc2);


#if defined(AIR_ANC_USER_UNAWARE_ENABLE) || defined(AIR_ANC_WIND_DETECTION_ENABLE) || defined(AIR_ANC_ENVIRONMENT_DETECTION_ENABLE)
void audio_anc_monitor_init_adaptive_anc_status(void);
void audio_anc_monitor_adaptive_anc_save_suspend_status_to_nvdm(audio_anc_monitor_feature_type_t feature, bool suspend);
void audio_anc_monitor_adaptive_anc_save_enable_status_to_nvdm(audio_anc_monitor_feature_type_t feature, bool enable);
void audio_anc_monitor_adaptive_anc_get_status_from_nvdm(audio_anc_monitor_feature_type_t feature, uint8_t *enable, uint8_t *suspend);
#endif

#ifdef ADAPIVE_ANC_STREAM_CONTROL
void audio_anc_monitor_adaptive_anc_stream_enable(bool enable);
void audio_anc_monitor_adaptive_anc_stream_get_status(uint8_t *enable);
#endif

#ifdef AIR_ANC_WIND_DETECTION_ENABLE
void audio_wind_detection_enable(bool enable);
void audio_wind_detection_suspend_procedure(bool suspend);
#endif

#ifdef AIR_ANC_USER_UNAWARE_ENABLE
void audio_user_unaware_save_enable_status_to_nvdm(bool enable);
void audio_user_unaware_suspend_procedure(bool suspend);
void audio_user_unaware_enable(bool enable);
U8 audio_user_unaware_get_enable_state(void);
void audio_user_unaware_get_gain_info(uint8_t *info_ptr);
#endif

#ifdef AIR_ANC_ENVIRONMENT_DETECTION_ENABLE
void audio_environment_detection_suspend_procedure(bool suspend);
int16_t audio_environment_detection_get_stationary_noise(void);
void audio_environment_detection_enable(bool enable);
#endif

#endif //MTK_ANC_SURROUND_MONITOR_ENABLE

#ifdef __cplusplus
}
#endif


#endif  /*__ANC_MONITOR_H__*/
