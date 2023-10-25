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

#ifndef __BT_ULL_LE_AUDIO_MANAGER_H__
#define __BT_ULL_LE_AUDIO_MANAGER_H__
#include "bt_type.h"
#include "bt_ull_utility.h"
#include "bt_ull_audio_manager.h"
#include "bt_ull_le_audio_transmitter.h"
#include "audio_src_srv.h"
#include "bt_sink_srv_ami.h"
#include "bt_ull_le_utility.h"

BT_EXTERN_C_BEGIN



/**
 * @brief This structure defines the ULL LE audio mode on client 
 */
typedef uint8_t bt_ull_le_am_mode_t; 
#define BT_ULL_LE_AM_UL_MODE           0x00
#define BT_ULL_LE_AM_DL_MODE           0x01

/**
 *  @brief Define the audio manager event type of ULL LE.
 */
typedef uint32_t bt_ull_le_am_event_t;
#define BT_ULL_LE_AM_PLAY_IND           0x01
#define BT_ULL_LE_AM_STOP_IND           0x02

/**
 *  @brief Define the audio manager result notify strcture.
 */
typedef struct
{
    bt_ull_le_am_mode_t mode;
    bt_status_t result;
} bt_ull_le_am_result_t;

//callback
typedef void (*bt_ull_le_am_callback_t)(bt_ull_le_am_event_t event, void *data, uint32_t data_len);


/**
 * @brief   This function is used for init am.
 * @param[in] callback    is callback function.
 * @return    void
 *                        
 */
void bt_ull_le_am_init(bt_ull_le_am_callback_t callback);

/**
 * @brief   This function is used for deinit am.
 * @param[in] mode is dl mode or ul mode    .
 * @return    void
 *                        
 */
void bt_ull_le_am_deinit(bt_ull_le_am_mode_t mode);

/**
 * @brief   This function is used for get alow or disallow of am play.
 * @param[in] void    .
 * @return    true: am allow play
 * @return    false:am disallow play                       
 */
bool bt_ull_le_am_is_allow_play(void);

/**
 * @brief   This function is used for notify am play audio.
 * @param[in] mode: dl mode or ul mode.
 * @param[in] codec: codec type.
 * @return    #BT_STATUS_SUCCESS, the operation completed successfully.
 *            #BT_STATUS_FAIL, the operation has failed.                  
 */
bt_status_t bt_ull_le_am_play(bt_ull_le_am_mode_t mode, bt_ull_codec_t codec, bool is_app_request);

/**
 * @brief   This function is used for notify am stop audio.
 * @param[in] mode: dl mode or ul mode.
 * @param[in] codec: codec type.
 * @return    #BT_STATUS_SUCCESS, the operation completed successfully.
 *            #BT_STATUS_FAIL, the operation has failed.                  
 */
bt_status_t bt_ull_le_am_stop(bt_ull_le_am_mode_t mode, bool is_app_request);

/**
 * @brief   This function is used for notify am restart audio.
 * @param[in] mode: dl mode or ul mode.
 * @return    #BT_STATUS_SUCCESS, the operation completed successfully.
 *            #BT_STATUS_FAIL, the operation has failed.                  
 */
bt_status_t bt_ull_le_am_restart(bt_ull_le_am_mode_t mode);

/**
 * @brief   This function is used for notify am suspend audio.
 * @param[in] mode: dl mode or ul mode.
 * @return    #BT_STATUS_SUCCESS, the operation completed successfully.
 *            #BT_STATUS_FAIL, the operation has failed.                  
 */
bt_status_t bt_ull_le_am_suspend(bt_ull_le_am_mode_t mode);

/**
 * @brief   This function is used for set mute audio.
 * @param[in] mode: dl mode or ul mode.
 * @param[in] is_mute: mute or unmute
 * @return    #BT_STATUS_SUCCESS, the operation completed successfully.
 *            #BT_STATUS_FAIL, the operation has failed.                  
 */
bt_status_t bt_ull_le_am_set_mute(bt_ull_le_am_mode_t mode, bool is_mute);

/**
 * @brief   This function is used for set audio volume.
 * @param[in] mode: dl mode or ul mode.
 * @param[in] volume: 
 * @return    #BT_STATUS_SUCCESS, the operation completed successfully.
 *            #BT_STATUS_FAIL, the operation has failed.                  
 */
bt_status_t bt_ull_le_am_set_volume(bt_ull_le_am_mode_t mode,uint8_t volume, bt_ull_audio_channel_t channel);
void bt_ull_le_am_sync_volume(bt_ull_original_duel_volume_t *volume);

/**
 * @brief   This function is used for get playing status.
 * @param[in] mode: dl mode or ul mode.
 * @return    true: am is playing
 *            false: am is not playing                  
 */
bool bt_ull_le_am_is_playing(bt_ull_le_am_mode_t mode);

/**
 * @brief   This function is used for update the priority of downlink, and it will be affected by the presence of Uplink or not .
 * @param[in] is_raise: raise the priority or lower the priority.
 * @return   None.                  
 */
void bt_ull_le_am_update_dl_priority(bool is_raise);

void bt_ull_le_am_set_sidetone_switch(bool is_sidetone_enable);
void bt_ull_le_am_sync_sidetone_status(bool is_ul_port_streaming);

/**
 * @brief   This function is used for add/remove waiting list for a2dp re-sync .
 * @param[in] add_waiting_list: true: add waiting list   false: remove waiting list
 * @return   None.                  
 */
void bt_ull_le_am_operate_waiting_list_for_a2dp_resync(bool add_waiting_list);
bool bt_ull_le_am_is_stop_ongoing(bt_ull_le_am_mode_t mode);

BT_EXTERN_C_END

#endif

