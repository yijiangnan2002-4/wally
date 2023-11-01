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

/**
 * File: app_audio_manager.h
 *
 * Description: This file defines the interface of audio manager.
 *
 */

#ifndef __APP_AUDIO_TRANS_MGR_H__
#define __APP_AUDIO_TRANS_MGR_H__

#include "audio_source_control.h"
#include "audio_transmitter_control.h"
#include "audio_src_srv_resource_manager_config.h"
#include "audio_src_srv.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    APPS_EVENTS_INTERACTION_APP_AUDIO_TRANS_MGR_TRANSMITTER_EV    = 0,
    APPS_EVENTS_INTERACTION_APP_AUDIO_TRANS_MGR_RESOURCE_CTRL_EV  = 1,
    APPS_EVENTS_INTERACTION_APP_AUDIO_TRANS_MGR_SEUDO_DEV_EV      = 2,
    APPS_EVENTS_INTERACTION_APP_AUDIO_TRANS_MGR_SELF_CMD          = 3,
    APPS_EVENTS_INTERACTION_APP_AUDIO_TRANS_MGR_AUD_SRC_CTRL_EV   = 4,
    APPS_EVENTS_INTERACTION_APP_AUDIO_TRANS_MGR_SALVE_EV          = 5,
} app_audio_trans_mgr_interaction_event_type_t;

typedef enum {
    APP_AUDIO_TRANS_MGR_USR_LINE_IN   = 0,
    APP_AUDIO_TRANS_MGR_USR_LINE_OUT  = 1,
    APP_AUDIO_TRANS_MGR_USR_USB_IN1   = 2,
    APP_AUDIO_TRANS_MGR_USR_USB_IN2   = 3,
    APP_AUDIO_TRANS_MGR_USR_USB_OUT   = 4,
    APP_AUDIO_TRANS_MGR_USR_USB_IN_OUT_MIX = 5,
    APP_AUDIO_TRANS_MGR_USR_MAX_NUM
} app_audio_trans_mgr_usr_type_t;

#define APP_AUDIO_TRANS_MGR_CTRL_TYPE_RESOURCE_CTRL 0
#define APP_AUDIO_TRANS_MGR_CTRL_TYPE_SEUDO_DEVICE 1
#define APP_AUDIO_TRANS_MGR_CTRL_TYPE_AUD_SRC_CTRL 2
#define APP_AUDIO_TRANS_MGR_CTRL_TYPE_NONE 0xff

typedef uint8_t app_audio_trans_mgr_ctrl_type_t;

typedef struct {
    app_audio_trans_mgr_usr_type_t usr;
    uint32_t event;
} app_audio_trans_mgr_interaction_event_t;

typedef struct {
    audio_transmitter_config_t trans_cfg;
    audio_src_srv_resource_type_t resource_type;
    uint8_t priority;
    audio_src_srv_pseudo_device_t pseudo_type;
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
    audio_source_control_usr_type_t source_ctrl_type;
#endif
} app_audio_trans_mgr_usr_cfg_t;

typedef void (*app_audio_get_cfg_callback)(app_audio_trans_mgr_usr_type_t type, app_audio_trans_mgr_usr_cfg_t *cfg);

typedef struct {
    app_audio_trans_mgr_usr_type_t type;
    app_audio_get_cfg_callback cfg_callback;
    int32_t volume_db_l;
    int32_t volume_db_r;
    uint8_t volume_l;
    uint8_t volume_r;
    uint8_t mix_ratio;
    uint8_t mute;
    uint8_t en_side_tone;
    app_audio_trans_mgr_ctrl_type_t ctrl_type;
    audio_transmitter_runtime_config_type_t rconfig_type;
    const char *name;
    void *usr_data;
} app_audio_trans_mgr_usr_t;

void *app_audio_trans_mgr_register(app_audio_trans_mgr_usr_t *usr);
void app_audio_trans_mgr_init_audio(void *usr);
void app_audio_trans_mgr_start_audio(void *usr);
void app_audio_trans_mgr_set_volume(void *usr, uint8_t l, uint8_t r);
void app_audio_trans_mgrs_set_db(void *usr, int32_t l, int32_t r);
void app_audio_trans_mgr_set_mix_ratio(void *usr, uint8_t ratio);
void app_audio_trans_mgr_set_mute(void *usr, uint8_t mute);
void app_audio_trans_mgr_stop_audio(void *usr);
void app_audio_trans_mgr_deinit_audio(void *usr);
bool app_audio_trans_mgr_wired_audio_playing(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_AUDIO_TRANS_MGR_H__ */

