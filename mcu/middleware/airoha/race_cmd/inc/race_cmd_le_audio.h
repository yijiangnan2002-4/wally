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


#ifndef __RACE_CMD_LE_AUDIO_H__
#define __RACE_CMD_LE_AUDIO_H__


#include "race_cmd_feature.h"
#ifdef MTK_RACE_CMD_ENABLE
#include "stdint.h"
#include "bt_type.h"
#include "bt_hfp.h"
#include "race_cmd.h"

#if defined(AIR_LE_AUDIO_BIS_ENABLE) && !defined(AIR_BLE_AUDIO_DONGLE_ENABLE)
#include "ble_pacs.h"
#include "ble_bap.h"
#include "bt_sink_srv_le_cap_stream.h"
#include "bt_sink_srv_le_cap.h"
#endif


////////////////////////////////////////////////////////////////////////////////
// CONSTANT DEFINITIONS ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#define RACE_LEAUDIO_SCAN_BROADCAST_SOURCE              0x2200
#define RACE_LEAUDIO_SELECT_BROADCAST_SOURCE            0x2201
#define RACE_LEAUDIO_PLAY_BIS                           0x2202
#define RACE_LEAUDIO_STOP_BIS                           0x2203
#define RACE_LEAUDIO_BIS_RESET                          0x2204
#define RACE_LEAUDIO_SET_DONGLE_STREAM_MODE             0x2205
#define RACE_LEAUDIO_GET_DONGLE_STREAM_MODE             0x2206
#define RACE_LEAUDIO_DONGLE_BROADCAST_DEVICE            0x2207
#define RACE_LEAUDIO_GET_BIS_STATE                      0x2208
#define RACE_LEAUDIO_SET_BROADCAST_CODE                 0x2209

#define RACE_LEAUDIO_EA_REPORT                          0x2200
#define RACE_LEAUDIO_PA_REPORT                          0x2201
#define RACE_LEAUDIO_BIG_SYNC_IND                       0x2202
#define RACE_LEAUDIO_BIG_TERMINATED_IND                 0x2203
#define RACE_LEAUDIO_BIG_INFO                           0x2209

#define RACE_LE_AUDIO_SCAN_MODE_STOP                        (0)
#define RACE_LE_AUDIO_SCAN_MODE_FIRST                       (1)
#define RACE_LE_AUDIO_SCAN_MODE_MAX_RSSI                    (2)
#define RACE_LE_AUDIO_SCAN_MODE_ALL                         (3)
typedef uint8_t race_le_audio_scan_mode;

#define RACE_LE_AUDIO_SCAN_STATUS_ONGOING                   (0)
#define RACE_LE_AUDIO_SCAN_STATUS_STOP                      (1)
typedef uint8_t race_le_audio_scan_status;

enum
{
    RACE_LE_AUDIO_RETRY_FLAG_IDLE,
    RACE_LE_AUDIO_RETRY_FLAG_SCAN,
    RACE_LE_AUDIO_RETRY_FLAG_SCAN_AFTER_PA_TERMINATE,
    RACE_LE_AUDIO_RETRY_FLAG_SCAN_AFTER_BIG_TERMINATE,
};

////////////////////////////////////////////////////////////////////////////////
// TYPE DEFINITIONS ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#if defined(AIR_BLE_AUDIO_DONGLE_ENABLE)
typedef void *(*race_cmd_le_audio_dongle_callback_t)(ptr_race_pkt_t pRaceHeaderCmd, uint16_t length, uint8_t channel_id);
#endif

typedef struct {
    uint8_t status;
    uint8_t state_type;
} PACKED race_le_audio_bis_state_t;

typedef struct {
    uint8_t status;
} PACKED race_le_audio_status_t;

typedef struct {
    uint8_t status;
    uint8_t mode;
} PACKED race_le_audio_dongle_mode_t;

typedef struct {
    uint8_t status;
    uint8_t opt;
} PACKED race_le_audio_dongle_broadcast_option_t;

typedef struct {
    race_le_audio_scan_status scan_status;
    uint8_t addr_type;
    bt_bd_addr_t addr;
    uint8_t advertising_sid;
    int8_t rssi;
    uint8_t data_length;
    uint8_t data[1];
} PACKED race_le_audio_ea_report_t;

typedef struct {
    uint8_t sync_status;
    uint8_t num_subgroup;
    uint8_t subgroup_data[1];
} PACKED race_le_audio_pa_report_t;

typedef struct {
    uint8_t encryption;
} PACKED race_le_audio_big_info_report_t;

////////////////////////////////////////////////////////////////////////////////
// FUNCTION DECLARATIONS /////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/*!
  @brief Process LE Audio related RACE commands.

  @param pRaceHeaderCmd This parameter represents the raw data such as "05 5A...".
  @param Lenth Total bytes of this RACE command.
  @param channel_id Channel identifier
*/
void *RACE_CmdHandler_LE_AUDIO(ptr_race_pkt_t pRaceHeaderCmd, uint16_t length, uint8_t channel_id);
void *RACE_CmdHandler_GET_RSSI(ptr_race_pkt_t pCmdMsg, uint16_t length, uint8_t channel_id);
#if defined(AIR_LE_AUDIO_BIS_ENABLE) && !defined(AIR_BLE_AUDIO_DONGLE_ENABLE)
bt_status_t race_le_audio_notify_ea(uint8_t status, bt_sink_srv_cap_event_base_broadcast_audio_announcements_t *msg);
bt_status_t race_le_audio_notify_pa(uint8_t status, bt_sink_srv_cap_event_base_basic_audio_announcements_t *msg);
bt_status_t race_le_audio_notify_big_info(bt_sink_srv_cap_event_base_biginfo_adv_report_t *msg);
bt_status_t race_le_audio_notify_pa_sync_ind(uint8_t status, ble_bap_periodic_adv_sync_established_notify_t *msg);
bt_status_t race_le_audio_notify_big_sync_ind(uint8_t status);
bt_status_t race_le_audio_notify_pa_terminated_ind(uint8_t status);
bt_status_t race_le_audio_notify_big_terminated_ind(uint8_t status);
void race_le_audio_notify_scan_stopped_ind(void);
void race_le_audio_play_bis_retry(void);
bool race_le_audio_check_race_bt_is_connected(void);
#elif defined(AIR_BLE_AUDIO_DONGLE_ENABLE)
void race_le_audio_dongle_register_callback(race_cmd_le_audio_dongle_callback_t callback);
#endif
#endif /* RACE_BT_CMD_ENABLE */
#endif /* __RACE_CMD_LE_AUDIO_H__ */

