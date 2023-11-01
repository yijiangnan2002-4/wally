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

#ifndef __APP_LE_AUDIO_BA_UTILLITY_H__
#define __APP_LE_AUDIO_BA_UTILLITY_H__

#ifdef AIR_LE_AUDIO_ENABLE

#include "bt_type.h"
#include "app_le_audio_ucst_utillity.h"

#include "ble_bass.h"
#include "ble_bap.h"

/**************************************************************************************************
* Define
**************************************************************************************************/
#define APP_LE_AUDIO_BA_LINK_MAX_NUM                    APP_LE_AUDIO_UCST_LINK_MAX_NUM
#define APP_LE_AUDIO_BA_INVALID_LINK_IDX                0xFF

#define APP_LE_AUDIO_BA_NOT_SUPPORT_MODE                0
#define APP_LE_AUDIO_BA_INITIATOR_AND_COMMANDER_MODE    1
#define APP_LE_AUDIO_BA_COMMANDER_ONLY_MODE             2

enum
{
    APP_LE_AUDIO_BA_LINK_STATE_IDLE,
    APP_LE_AUDIO_BA_LINK_STATE_DISCOVERY_COMPLETE,
    APP_LE_AUDIO_BA_LINK_STATE_READ_BROADCAST_RECEIVE_STATE,

    /* Unicast Procedure*/
    //APP_LE_AUDIO_BA_LINK_STATE_EXCHANGE_MTU,
    //APP_LE_AUDIO_BA_LINK_STATE_READ_AUDIO_CONTEXTS,
    //APP_LE_AUDIO_BA_LINK_STATE_READ_SINK_PAC,
    //APP_LE_AUDIO_BA_LINK_STATE_READ_SINK_LOCATION,

    APP_LE_AUDIO_BA_LINK_STATE_READY,
    APP_LE_AUDIO_BA_LINK_STATE_PAST_WAITING,
    APP_LE_AUDIO_BA_LINK_STATE_PAST_PROCESSING,
    APP_LE_AUDIO_BA_LINK_STATE_PAST_FINISHED,
    APP_LE_AUDIO_BA_LINK_STATE_PA_SYNCED,
    APP_LE_AUDIO_BA_LINK_STATE_BIG_SYNCED,
    APP_LE_AUDIO_BA_LINK_STATE_BROADCAST_CODE_REQUIRED,
    APP_LE_AUDIO_BA_LINK_STATE_REMOVE_SOURCE,
    APP_LE_AUDIO_BA_LINK_STATE_NUMBER
};

/**************************************************************************************************
* Structure
**************************************************************************************************/
typedef struct {
    uint32_t sink_location;
    bt_handle_t handle;
    uint8_t ba_state;
    uint8_t source_id;
    uint8_t frames_per_sdu;
    bool sink_pac_is_supported;
    bool remove_source_needed;
    bool add_source_retry;
} app_le_audio_ba_link_info_t;

typedef struct {
    uint8_t advertising_sid;
    uint8_t broadcast_id[BLE_BASS_BROADCAST_ID_SIZE];
    uint16_t octets_per_frame;
    bt_handle_t sync_handle;
    bt_addr_t bms_addr;
    ble_bap_basic_audio_announcements_level_1_t pa_data;
} app_le_audio_ba_stream_info_t;

/**************************************************************************************************
* Public function
**************************************************************************************************/
uint8_t app_le_audio_ba_check_state_if_exist(uint8_t ba_state);

uint8_t app_le_audio_ba_get_link_num(void);

bt_handle_t app_le_audio_ba_get_handle_by_idx(uint8_t link_idx);

uint8_t app_le_audio_ba_get_link_idx_by_handle(bt_handle_t handle);

app_le_audio_ba_link_info_t *app_le_audio_ba_get_link_info_by_handle(bt_handle_t handle);

app_le_audio_ba_link_info_t *app_le_audio_ba_get_link_info_by_idx(uint8_t link_idx);

app_le_audio_ba_stream_info_t *app_le_audio_ba_get_stream_info(void);

void app_le_audio_ba_reset_link_info(uint8_t link_idx);

uint8_t app_le_audio_ba_get_mode(void);

void app_le_audio_ba_set_mode(uint8_t mode);

bool app_le_audio_ba_get_auto_play(void);

void app_le_audio_ba_set_auto_play(bool auto_play);

void app_le_audio_ba_utillity_init(void);

#endif /* AIR_LE_AUDIO_ENABLE */
#endif /* __APP_LE_AUDIO_BA_UTILLITY_H__ */

