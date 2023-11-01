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
 * File: app_music_utils.h
 *
 * Description: This file defines the common structure and functions of music app.
 *
 * Note: See doc/AB1565_AB1568_Earbuds_Reference_Design_User_Guide.pdf for more detail.
 *
 */


#ifndef __APP_MUSIC_UTILS_H__
#define __APP_MUSIC_UTILS_H__

#include "bt_sink_srv.h"

#include "ui_shell_manager.h"
#include "ui_shell_activity.h"
#include "apps_config_key_remapper.h"

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
#include "bt_ull_le_service.h"
#endif



#define APP_MUSIC_UTILS "[Music_APP]utils"
#define AVRCP_OPERATION_STA_IDLE 0
#define AVRCP_OPERATION_STA_FAST_FORWARD_PRESS 1
#define AVRCP_OPERATION_STA_FAST_REWIND_PRESS 2

#define APP_MUSIC_STEAMING_STATE_SINK_STATE      (0x01)
#define APP_MUSIC_STEAMING_STATE_AVRCP_STATE     (0x02)
#define APP_MUSIC_STEAMING_STATE_ULL_STATE       (0x04)
typedef uint8_t app_music_streaming_state_t;

#define APP_MUISC_AVRCP_RECORD_MAX_NUM    2

#define APP_MUSIC_ADJUST_VOLUME_VIA_ADDRESS_ENABLE

#ifdef MTK_AWS_MCE_ENABLE
/**
 *  @brief This enumeration defines the music mixing mode.
 */
typedef enum {
    MUSIC_STEREO = 0,       /**<  Stereo mode. */
    MUSIC_MONO              /**<  Mono mode. */
} music_mix_state_t;
#endif


/**
 *  @brief This structure defines the music app's context
 */
typedef struct {
    bool music_playing;                 /**<  Indicates whether the music is playing. */
#ifdef MTK_AWS_MCE_ENABLE
    bool isPartnerCharging;             /**<  Indicates whether the partner is charging. */
    bool isPartnerConnected;            /**<  Indicates whether the partner is connected. */
    music_mix_state_t currMixState;     /**<  Record current mixing mode. */
    bool isSame;                  /**<  Indicates whether the wearing state of agent and partner are same. */
#endif
    bool isAutoPaused;                  /**<  Indicates whether the music is suspended by the music app itself. */
    uint32_t avrcp_op_sta;              /**<  Record the last avrcp operation. */
    //bt_bd_addr_t activate_con_addr;     /**<  Record activated music address. */
    app_music_streaming_state_t music_streaming_state;      /**<  Record music streaming state.*/
} apps_music_local_context_t;


typedef struct {
    bool           is_playing;
    bt_bd_addr_t   remote_addr;
} app_music_avrcp_record_item_t;

typedef struct {
    uint8_t                        avrcp_num;
    app_music_avrcp_record_item_t  avrcp_device[APP_MUISC_AVRCP_RECORD_MAX_NUM];
} app_music_avrcp_status_t;


/**
 *  @brief This structure defines the music in ear configuration.
 */

#ifdef MTK_IN_EAR_FEATURE_ENABLE
typedef enum {
    APP_MUSIC_IN_EAR_NONE = 0,                /**< NONE. */
    APP_MUSIC_IN_EAR_DISABLE,                 /**< Indicates in ear detection disable. */
    APP_MUSIC_IN_EAR_ONLY_AUTO_PAUSE,         /**< Indicates only enable auto-pause feature. */
    APP_MUSIC_IN_EAR_AUTO_PAUSE_RESUME        /**< Indicates enable auto-pause and auto-resume features. */
} app_music_in_ear_config_t;
#endif


#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
extern bool app_music_ull2_uplink_enable;
extern bool app_music_ull2_downlink_enable;

#endif



#ifdef MTK_AWS_MCE_ENABLE
/**
* @brief      This function is used to set the mixing mode according to the current state.
* @param[in]  cntx, the context pointer of the activity.
* @return     None.
*/
void app_music_checkAudioState(apps_music_local_context_t *cntx);


/**
* @brief      This function is used to handle the events come from partner.
* @param[in]  self, the context pointer of the activity.
* @param[in]  event_id, the current event ID to be handled.
* @param[in]  extra_data, extra data pointer of the current event, NULL means there is no extra data.
* @param[in]  data_len, the length of the extra data. 0 means extra_data is NULL.
* @return     If return true, the current event cannot be handle by the next activity.
*/
bool app_music_idle_proc_aws_data_events(ui_shell_activity_t *self,
                                         uint32_t event_id,
                                         void *extra_data,
                                         size_t data_len);
#endif

/**
* @brief      This function is used to handle the key action.
* @param[in]  self, the context pointer of the activity.
* @param[in]  event_id, the current event ID to be handled.
* @param[in]  extra_data, extra data pointer of the current event, NULL means there is no extra data.
* @param[in]  data_len, the length of the extra data. 0 means extra_data is NULL.
* @return     If return true, the current event cannot be handle by the next activity.
*/
apps_config_key_action_t app_music_utils_proc_key_events(ui_shell_activity_t *self,
                                                         uint32_t event_id,
                                                         void *extra_data,
                                                         size_t data_len);

/**
* @brief      This function is used to handle the event come from sink service module.
* @param[in]  self, the context pointer of the activity.
* @param[in]  event_id, the current event ID to be handled.
* @param[in]  extra_data, extra data pointer of the current event, NULL means there is no extra data.
* @param[in]  data_len, the length of the extra data. 0 means extra_data is NULL.
* @return     If return true, the current event cannot be handle by the next activity.
*/
bool app_music_idle_proc_bt_sink_events(ui_shell_activity_t *self,
                                        uint32_t event_id,
                                        void *extra_data,
                                        size_t data_len);


/**
* @brief      This function is used to handle the event come from bt connection manager.
* @param[in]  self, the context pointer of the activity.
* @param[in]  event_id, the current event ID to be handled.
* @param[in]  extra_data, extra data pointer of the current event, NULL means there is no extra data.
* @param[in]  data_len, the length of the extra data. 0 means extra_data is NULL.
* @return     If return true, the current event cannot be handle by the next activity.
*/
bool app_music_idle_proc_bt_cm_events(ui_shell_activity_t *self,
                                      uint32_t event_id,
                                      void *extra_data,
                                      size_t data_len);

#ifdef AIR_GSOUND_ENABLE
/**
* @brief      This function is used to handle the sink service action which rejected by gsound app.
* @param[in]  self, the context pointer of the activity.
* @param[in]  sink_action, the sink service action.
* @return     If return true, the current event cannot be handle by the next activity.
*/
bool app_music_idle_proc_gsound_reject_action(ui_shell_activity_t *self, bt_sink_srv_action_t sink_action);
#endif

/**
* @brief      This function is used to handle the app internal events.
* @param[in]  self, the context pointer of the activity.
* @param[in]  event_id, the current event ID to be handled.
* @param[in]  extra_data, extra data pointer of the current event, NULL means there is no extra data.
* @param[in]  data_len, the length of the extra data. 0 means extra_data is NULL.
* @return     If return true, the current event cannot be handle by the next activity.
*/
bool app_music_idle_proc_apps_internal_events(ui_shell_activity_t *self,
                                              uint32_t event_id,
                                              void *extra_data,
                                              size_t data_len);

#ifdef MTK_IN_EAR_FEATURE_ENABLE
/**
* @brief      This function is used to obtain the status of the function of controlling music through in ear detection.
* @return     Return the configuration of music in ear.
*/
uint8_t app_music_get_in_ear_control_state();

/**
* @brief      This function is used to set the function of controlling music through ear detection.
* @param[in]  music_in_ear_cfg, music in ear configuration.
* @param[in]  isSync, whether sync music_in_ear_cfg to other side. false for RACE CMD, true for the other application.
* @return     If return true, set successfully.
*/
bool app_music_set_in_ear_control(uint8_t music_in_ear_cfg, bool isSync);

/**
* @brief      This function is used to notify state to peer.
*/
void app_music_notify_state_to_peer();
/**
* @brief      This function is used to update music mix state.
* @param[in]  self, the context pointer of the activity.
* @param[in]  extra_data, extra data pointer of the current event, NULL means there is no extra data.
* @return     If return true, the current event cannot be handle by the next activity.
*/
bool app_music_in_ear_update_mix_state(struct _ui_shell_activity *self, void *extra_data);
#endif

/**
* @brief      This function is used to set the music/hfp volume level.
* @param[in]  isUp, true for volume level up, false for volume level down.
* @param[in]  value, The level values range from 0 to 15.
*                    for example, The volume was reduced by 20 percent, and the level is reduced by about 3 levels.
*                    app_music_set_volume_value(false, 3);
* @return     If return true, volume set success.
*/
bool app_music_set_volume_value(bool isUp, uint32_t value);

/**
* @brief      This function is used to get the connection state of the current link.
* @return     If return true, the link was connected. Such as A2DP, ULL, CIS and BIS.
*/
bool app_music_get_curr_link_is_connected(void);

/**
* @brief      This function is used to get the streaming state of the ull link.
* @return     If return true, the ull is streaming.
*/
bool app_music_get_ull_is_streaming(void);

/**
* @brief      This function is used to set the volume of the ull link.
* @return     If return true, set the volume success.
*/
bool app_music_set_ull_volume(bool volume_up, uint32_t volume);

/**
* @brief      This function is used to send the AIRD action for LE Audio dongle.
* @return     If return true, send success.
*/
bool app_music_send_le_audio_aird_action(apps_config_key_action_t action, uint32_t volume);

/**
* @brief      This function is used to get the last address of device or the actived decive.
* @param[out] volume_addr, use this address to adjust the volume of device.
* @return     If return true, get the address success.
*/
#ifdef APP_MUSIC_ADJUST_VOLUME_VIA_ADDRESS_ENABLE
bool app_music_get_volume_device_addr(bt_sink_srv_action_volume_t *volume_addr);
#endif

/**
* @brief      This function is used to add the avrcp status of the remote device after received the play status.
* @param[in]  sink_event, use this event to add the status.
* @return     If return true, add success.
*/
bool app_music_add_avrcp_status(bt_sink_srv_event_param_t *sink_event);

/**
* @brief      This function is used to remove the avrcp status of the remote device after received the paused or stoped status.
* @param[in]  addr, the address of the avrcp status.
* @return     If return true, remove success.
*/
bool app_music_remove_avrcp_status(bt_bd_addr_t *addr);

/**
* @brief      This function is used to get the recorded avrcp status of the headset.
* @return     The recorded avrcp status.
*/
app_music_avrcp_status_t *app_music_get_avrcp_status(void);
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
/**
* @brief      This function is update ull2.up/down link status
* @param[in]  sink_event, use this event to remove the status.
* @return     If return true, remove success.
*/
void app_music_update_ull2_link_state(uint32_t event_id, bt_ull_le_streaming_start_ind_t *bt_ull2_link_type);
/**
* @brief      This function is clear ull2.up/down link status
* @param[in]  sink_event, use this event to remove the status.
* @return     none.
*/
void app_music_clear_ull2_link_state();
//
bool app_music_get_ull2_is_call();
//
bool app_music_get_ull2_is_music();


#endif

bt_status_t app_music_send_actions_by_address(bt_sink_srv_action_t action);

#ifdef AIR_ROTARY_ENCODER_ENABLE
bool app_music_proc_rotary_event_group(ui_shell_activity_t *self,
                                       uint32_t event_id,
                                       void *extra_data,
                                       size_t data_len);
#endif
#endif /*__APP_MUSIC_UTILS_H__*/

