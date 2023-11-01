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

#ifndef __BLE_MCP_H__
#define __BLE_MCP_H__

#include "ble_mcs_def.h"
#include "bt_le_audio_type.h"

/**
 * @addtogroup Bluetooth
 * @{
 * @addtogroup BluetoothLeaudio LE_AUDIO
 * @{
 * @addtogroup BluetoothLeAudioMCP MCP
 * @{
 * This section introduces the MCP operation codes, request and response structures, API prototypes.
 *
 * Terms and Acronyms
 * ======
 * |Terms                         |Details                                                                  |
 * |------------------------------|-------------------------------------------------------------------------|
 * |\b MCP                        | Media Control Profile. |
 * |\b MCS                        | Media Control Service. |
 * |\b GMCS                       | Generic media Control Service. |
 * |\b URL                        | Uniform Resource Locator. |
 *
 * @section ble_mcp_api_usage How to use this module
 *   - Send MCP action with #ble_mcp_send_action() to control GMCS or MCS server.
 *   - Sample code:
 *     @code
 *          // send an action to read the media player name
 *          bt_handle_t handle = 0x0001;                        // LE connection handle
 *          uint8_t service_idx = BLE_MCP_SERVICE_INDEX_GMCS;   // The service index for the action. BLE_MCP_SERVICE_INDEX_GMCS is the service index of the GMCS.
 *          ble_mcp_send_action(handle, BLE_MCP_ACTION_READ_MEDIA_PLAYER_NAME, NULL, service_idx);
 *
 *     @endcode
 *
 *   - Implement the event callback to process the related MCP events.
 *   - Sample code:
 *     @code
 *          bt_status_t bt_sink_srv_le_music_mcp_callback(bt_msg_type_t msg, bt_status_t status, void *buffer)
 *          {
 *              switch (msg) {
 *                  case BLE_MCP_READ_MEDIA_PLAYER_NAME_CNF: {
 *                      ble_mcp_read_media_player_name_cnf_t *cnf = (ble_mcp_read_media_player_name_cnf_t *)buffer;
 *                      char conn_string[50] = {0};
 *                      le_audio_log("[MCP][sink_music] READ_MEDIA_PLAYER_NAME_CNF, status:%x handle:%x srv_idx:%x len:%x", 4, status, cnf->handle, cnf->service_idx, cnf->media_player_name_length);
 *                      snprintf((char *)conn_string, 50, "ProviderName(%d): %s", cnf->media_player_name_length, cnf->p_media_player_name);
 *                      bt_app_common_at_cmd_print_report(conn_string);
 *                      break;
 *                  }
 *
 *                  default:
 *                      break;
 *              }
 *
 *              return BT_STATUS_SUCCESS;
 *          }
 *
 *     @endcode
 */

/**
 * @defgroup Bluetoothble_MCP_define Define
 * @{
 * This section defines the MCP action opcode and event IDs.
 */

/**
 *  @brief Defines the MCP events.
 */
#define BLE_MCP_DISCOVER_SERVICE_COMPLETE_IND                   (BT_LE_AUDIO_MODULE_MCP | 0x0000)    /**< This event indicates the MCP service discovery complete with payload #ble_mcp_discover_service_complete_t. */

#define BLE_MCP_CURRENT_TRACK_OBJECT_ID_IND             (BT_LE_AUDIO_MODULE_MCP | 0x0008)    /**< This event indicates the Current Track Object ID changes with payload #ble_mcp_current_track_object_id_ind_t. */
#define BLE_MCP_NEXT_TRACK_OBJECT_ID_IND                (BT_LE_AUDIO_MODULE_MCP | 0x0009)    /**< This event indicates the Next Track Object ID changes with payload #ble_mcp_next_track_object_id_ind_t. */
#define BLE_MCP_PARENT_GROUP_OBJECT_ID_IND              (BT_LE_AUDIO_MODULE_MCP | 0x000A)    /**< This event indicates the Parent Group Object ID changes with payload #ble_mcp_parent_group_object_id_ind_t. */
#define BLE_MCP_CURRENT_GROUP_OBJECT_ID_IND             (BT_LE_AUDIO_MODULE_MCP | 0x000B)    /**< This event indicates the Current Group Object ID changes with payload #ble_mcp_current_group_object_id_ind_t. */
#define BLE_MCP_MEDIA_CONTROL_POINT_IND                 (BT_LE_AUDIO_MODULE_MCP | 0x000E)    /**< This event indicates the Media Control Point changes with payload #ble_mcp_media_control_point_ind_t. */
#define BLE_MCP_SEARCH_CONTROL_POINT_IND                (BT_LE_AUDIO_MODULE_MCP | 0x0010)    /**< This event indicates the Search Control Point changes. */
#define BLE_MCP_SEARCH_RESULTS_OBJECT_ID_IND            (BT_LE_AUDIO_MODULE_MCP | 0x0011)    /**< This event indicates the Search Results Object ID characteristic with payload #ble_mcp_search_results_object_ind_t. */

#define BLE_MCP_READ_MEDIA_PLAYER_ICON_OBJECT_ID_CNF                        (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_READ_MEDIA_ICON_OBJECT_ID)    /**< The result of reading Media Player Icon Object ID with payload #ble_mcp_read_media_player_icon_object_id_cnf_t. */
#define BLE_MCP_READ_CURRENT_TRACK_SEGMENTS_OBJECT_ID_CNF                   (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_READ_CURRENT_TRACK_SEGMENTS_OBJECT_ID)    /**< The result of reading Track Segments Object ID with payload #. */
#define BLE_MCP_READ_CURRENT_TRACK_OBJECT_ID_CNF                            (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_READ_CURRENT_TRACK_OBJECT_ID)    /**< The result of reading Current Track Object ID with payload #ble_mcp_read_current_track_object_id_cnf_t. */
#define BLE_MCP_READ_NEXT_TRACK_OBJECT_ID_CNF                               (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_READ_NEXT_TRACK_OBJECT_ID)    /**< The result of reading Next Track Object ID with payload #ble_mcp_read_next_track_object_id_cnf_t. */
#define BLE_MCP_READ_PARENT_GROUP_OBJECT_ID_CNF                             (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_READ_PARENT_GROUP_OBJECT_ID)    /**< The result of reading Parent Group Object ID with payload #. */
#define BLE_MCP_READ_CURRENT_GROUP_OBJECT_ID_CNF                            (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_READ_CURRENT_GROUP_OBJECT_ID)    /**< The result of reading Current Group Object ID with payload #. */
#define BLE_MCP_READ_SEARCH_RESULTS_OBJECT_ID_CNF                           (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_READ_SEARCH_RESULTS_OBJECT_ID)    /**< The result of reading Search Results Object ID with payload #. */

#define BLE_MCP_SET_CURRENT_TRACK_OBJECT_ID_NOTIFICATION_CNF             (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_SET_CURRENT_TRACK_OBJECT_ID_NOTIFICATION)    /**< The result of setting notification of the Current Track Object ID characteristic with payload #ble_mcp_event_parameter_t. */
#define BLE_MCP_SET_NEXT_TRACK_OBJECT_ID_NOTIFICATION_CNF                (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_SET_NEXT_TRACK_OBJECT_ID_NOTIFICATION)    /**< The result of setting notification of the Next Track Object ID characteristic with payload #ble_mcp_event_parameter_t. */
#define BLE_MCP_SET_PARENT_GROUP_OBJECT_ID_NOTIFICATION_CNF              (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_SET_PARENT_GROUP_OBJECT_ID_NOTIFICATION)    /**< The result of setting notification of the Parent Group Object ID characteristic with payload #ble_mcp_event_parameter_t. */
#define BLE_MCP_SET_CURRENT_GROUP_OBJECT_ID_NOTIFICATION_CNF             (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_SET_CURRENT_GROUP_OBJECT_ID_NOTIFICATION)    /**< The result of setting notification of the Current Group Object ID characteristic with payload #ble_mcp_event_parameter_t. */
#define BLE_MCP_SET_MEDIA_CONTROL_POINT_NOTIFICATION_CNF                 (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_SET_MEDIA_CONTROL_POINT_NOTIFICATION)    /**< The result of setting notification of the Media Control Point characteristic with payload #ble_mcp_event_parameter_t. */
#define BLE_MCP_SET_SEARCH_CONTROL_POINT_NOTIFICATION_CNF                (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_SET_SEARCH_CONTROL_POINT_NOTIFICATION)    /**< The result of setting notification of the Search Control Point characteristic with payload #ble_mcp_event_parameter_t. */
#define BLE_MCP_SET_SEARCH_RESULTS_OBJECT_ID_NOTIFICATION_CNF            (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_SET_SEARCH_RESULTS_OBJECT_ID_NOTIFICATION)    /**< The result of setting notification of the Search Results Object ID characteristic with payload #ble_mcp_event_parameter_t. */

#define BLE_MCP_PLAY_CURRENT_TRACK_CNF                                   (BT_LE_AUDIO_MODULE_MCP | 0x0201)    /**< The result of #ble_mcp_play_current_track_req(). */
#define BLE_MCP_PAUSE_CURRENT_TRACK_CNF                                  (BT_LE_AUDIO_MODULE_MCP | 0x0202)    /**< The result of #ble_mcp_pause_current_track_req(). */
#define BLE_MCP_STOP_CURRENT_TRACK_CNF                                   (BT_LE_AUDIO_MODULE_MCP | 0x0203)    /**< The result of #ble_mcp_stop_current_track_req(). */
#define BLE_MCP_MOVE_TO_NEXT_TRACK_CNF                                   (BT_LE_AUDIO_MODULE_MCP | 0x0204)    /**< The result of #ble_mcp_move_to_next_track_req(). */
#define BLE_MCP_MOVE_TO_PREVIOUS_TRACK_CNF                               (BT_LE_AUDIO_MODULE_MCP | 0x0205)    /**< The result of #ble_mcp_move_to_previous_track_req(). */
#define BLE_MCP_FAST_FORWARD_CNF                                         (BT_LE_AUDIO_MODULE_MCP | 0x0206)    /**< The result of #ble_mcp_fast_forward_req(). */
#define BLE_MCP_FAST_REWIND_CNF                                          (BT_LE_AUDIO_MODULE_MCP | 0x0207)    /**< The result of #ble_mcp_fast_rewind_req(). */
#define BLE_MCP_WRITE_PLAYBACK_SPEED_CNF                                 (BT_LE_AUDIO_MODULE_MCP | 0x0208)    /**< The result of #ble_mcp_write_playback_speed_req(). */
#define BLE_MCP_WRITE_PLAYING_ORDER_CNF                                  (BT_LE_AUDIO_MODULE_MCP | 0x0209)    /**< The result of #ble_mcp_write_playing_order_req(). */
#define BLE_MCP_MOVE_TO_FIRST_TRACK_CNF                                  (BT_LE_AUDIO_MODULE_MCP | 0x020A)    /**< The result of #ble_mcp_move_to_first_track_req(). */
#define BLE_MCP_MOVE_TO_LAST_TRACK_CNF                                   (BT_LE_AUDIO_MODULE_MCP | 0x020B)    /**< The result of #ble_mcp_move_to_last_track_req(). */
#define BLE_MCP_MOVE_TO_TRACK_NUMBER_CNF                                 (BT_LE_AUDIO_MODULE_MCP | 0x020C)    /**< The result of #ble_mcp_move_to_track_number_req(). */
#define BLE_MCP_MOVE_TO_NEXT_GROUP_CNF                                   (BT_LE_AUDIO_MODULE_MCP | 0x020D)    /**< The result of #ble_mcp_move_to_track_number_req(). */
#define BLE_MCP_MOVE_TO_PREVIOUS_GROUP_CNF                               (BT_LE_AUDIO_MODULE_MCP | 0x020E)    /**< The result of #ble_mcp_move_to_previous_group_req(). */
#define BLE_MCP_MOVE_TO_FIRST_GROUP_CNF                                  (BT_LE_AUDIO_MODULE_MCP | 0x020F)    /**< The result of #ble_mcp_move_to_first_group_req(). */
#define BLE_MCP_MOVE_TO_LAST_GROUP_CNF                                   (BT_LE_AUDIO_MODULE_MCP | 0x0210)    /**< The result of #ble_mcp_move_to_last_group_req(). */
#define BLE_MCP_MOVE_TO_GROUP_NUMBER_CNF                                 (BT_LE_AUDIO_MODULE_MCP | 0x0211)    /**< The result of #ble_mcp_move_to_group_number_req(). */
#define BLE_MCP_MOVE_TO_NEXT_SEGMENT_CNF                                 (BT_LE_AUDIO_MODULE_MCP | 0x0212)    /**< The result of #ble_mcp_move_to_next_segment_req(). */
#define BLE_MCP_MOVE_TO_PREVIOUS_SEGMENT_CNF                             (BT_LE_AUDIO_MODULE_MCP | 0x0213)    /**< The result of #ble_mcp_move_to_previous_segment_req(). */
#define BLE_MCP_MOVE_TO_FIRST_SEGMENT_CNF                                (BT_LE_AUDIO_MODULE_MCP | 0x0214)    /**< The result of #ble_mcp_move_to_first_segment_req(). */
#define BLE_MCP_MOVE_TO_LAST_SEGMENT_CNF                                 (BT_LE_AUDIO_MODULE_MCP | 0x0215)    /**< The result of #ble_mcp_move_to_last_segment_req(). */
#define BLE_MCP_MOVE_TO_SEGMENT_NUMBER_CNF                               (BT_LE_AUDIO_MODULE_MCP | 0x0216)    /**< The result of #ble_mcp_move_to_segment_number_req(). */
#define BLE_MCP_SET_ABSOLUTE_TRACK_POSITION_CNF                          (BT_LE_AUDIO_MODULE_MCP | 0x0217)    /**< The result of #ble_mcp_set_absolute_track_position_req(). */
#define BLE_MCP_SET_RELATIVE_TRACK_POSITION_CNF                          (BT_LE_AUDIO_MODULE_MCP | 0x0218)    /**< The result of #ble_mcp_set_relative_track_position_req(). */

#define BLE_MCP_WRITE_SEARCH_CNF                                         (BT_LE_AUDIO_MODULE_MCP | 0x0219)
#define BLE_MCP_TRACK_DISCOVERY_CNF                                      (BT_LE_AUDIO_MODULE_MCP | 0x021A)
#define BLE_MCP_WRITE_MCP_UNSUPPORTED_OPCODE_CNF                         (BT_LE_AUDIO_MODULE_MCP | 0x021B)
#define BLE_MCP_WRITE_TRACK_POSITION_CNF                                 (BT_LE_AUDIO_MODULE_MCP | 0x021C)
#define BLE_MCP_WRITE_NEXT_TRACK_OBJECT_ID_CNF                           (BT_LE_AUDIO_MODULE_MCP | 0x021D)
#define BLE_MCP_WRITE_CURRENT_GROUP_OBJECT_ID_CNF                        (BT_LE_AUDIO_MODULE_MCP | 0x021E)
#define BLE_MCP_WRITE_CURRENT_TRACK_OBJECT_ID_CNF                        (BT_LE_AUDIO_MODULE_MCP | 0x021F)

/**
 *  @brief Defines the MCP actions.
 */
#define BLE_MCP_ACTION_READ_MEDIA_PLAYER_NAME                               (0x00000101)    /**< This action sends a request to read the media player name. */
#define BLE_MCP_ACTION_READ_MEDIA_ICON_OBJECT_ID                            (0x00000102)    /**< This action sends a request to read the Media Player Icon Object ID. */
#define BLE_MCP_ACTION_READ_MEDIA_PLAYER_ICON_URL                           (0x00000103)    /**< This action sends a request to read the media player icon URL. */
#define BLE_MCP_ACTION_READ_TRACK_TITLE                                     (0x00000104)    /**< This action sends a request to read the track title. */
#define BLE_MCP_ACTION_READ_TRACK_DURATION                                  (0x00000105)    /**< This action sends a request to read the track duration. */
#define BLE_MCP_ACTION_READ_TRACK_POSITION                                  (0x00000106)    /**< This action sends a request to read the track position. */
#define BLE_MCP_ACTION_READ_PLAYBACK_SPEED                                  (0x00000107)    /**< This action sends a request to read the playback speed. */
#define BLE_MCP_ACTION_READ_SEEKING_SPEED                                   (0x00000108)    /**< This action sends a request to read the seeking speed. */
#define BLE_MCP_ACTION_READ_CURRENT_TRACK_SEGMENTS_OBJECT_ID                (0x00000109)    /**< This action sends a request to read the Track Segments Object ID. */
#define BLE_MCP_ACTION_READ_PLAYING_ORDER                                   (0x0000010E)    /**< This action sends a request to read the playing order. */
#define BLE_MCP_ACTION_READ_PLAYING_ORDERS_SUPPORTED                        (0x0000010F)    /**< This action sends a request to read the playing orders supported. */
#define BLE_MCP_ACTION_READ_MEDIA_STATE                                     (0x00000110)    /**< This action sends a request to read the media state. */
#define BLE_MCP_ACTION_READ_MEDIA_CONTROL_OPCODES_SUPPORTED                 (0x00000111)    /**< This action sends a request to read the media control opcodes supported. */
#define BLE_MCP_ACTION_READ_CURRENT_TRACK_OBJECT_ID                         (0x0000010A)    /**< This action sends a request to read the Current Track Object ID. */
#define BLE_MCP_ACTION_READ_NEXT_TRACK_OBJECT_ID                            (0x0000010B)    /**< This action sends a request to read the Next Track Object ID. */
#define BLE_MCP_ACTION_READ_PARENT_GROUP_OBJECT_ID                          (0x0000010C)    /**< This action sends a request to read the Parent Group Object ID. */
#define BLE_MCP_ACTION_READ_CURRENT_GROUP_OBJECT_ID                         (0x0000010D)    /**< This action sends a request to read the Current Group Object ID. */
#define BLE_MCP_ACTION_READ_SEARCH_RESULTS_OBJECT_ID                        (0x00000112)    /**< This action sends a request to read the Search Results Object ID. */
#define BLE_MCP_ACTION_READ_CONTENT_CONTROL_ID                              (0x00000113)    /**< This action sends a request to read the content control ID. */
#define BLE_MCP_ACTION_READ_CURRENT_TRACK_OBJECT_INFORMATION                (0x00000114)    /**< This action sends a request to read the current track object information. */
#define BLE_MCP_ACTION_READ_CURRENT_TRACK_OBJECT                            (0x00000115)    /**< This action sends a request to read the Current Track Object. */

#define BLE_MCP_ACTION_SET_MEDIA_PLAYER_NAME_NOTIFICATION                   (0x00000301)    /**< This action sends a request to set notification of the media player name characteristic. */
#define BLE_MCP_ACTION_SET_TRACK_CHANGED_NOTIFICATION                       (0x00000302)    /**< This action sends a request to set notification of the track changed characteristic. */
#define BLE_MCP_ACTION_SET_TRACK_TITLE_NOTIFICATION                         (0x00000303)    /**< This action sends a request to set notification of the track title characteristic. */
#define BLE_MCP_ACTION_SET_TRACK_DURATION_NOTIFICATION                      (0x00000304)    /**< This action sends a request to set notification of the track duration characteristic. */
#define BLE_MCP_ACTION_SET_TRACK_POSITION_NOTIFICATION                      (0x00000305)    /**< This action sends a request to set notification of the track position characteristic. */
#define BLE_MCP_ACTION_SET_PLAYBACK_SPEED_NOTIFICATION                      (0x00000306)    /**< This action sends a request to set notification of the playback speed characteristic. */
#define BLE_MCP_ACTION_SET_SEEKING_SPEED_NOTIFICATION                       (0x00000307)    /**< This action sends a request to set notification of the seeking speed characteristic. */
#define BLE_MCP_ACTION_SET_CURRENT_TRACK_OBJECT_ID_NOTIFICATION             (0x00000308)    /**< This action sends a request to set notification of the Current Track Object ID characteristic. */
#define BLE_MCP_ACTION_SET_NEXT_TRACK_OBJECT_ID_NOTIFICATION                (0x00000309)    /**< This action sends a request to set notification of the Next Track Object ID characteristic. */
#define BLE_MCP_ACTION_SET_PARENT_GROUP_OBJECT_ID_NOTIFICATION              (0x0000030A)    /**< This action sends a request to set notification of the Parent Group Object ID characteristic. */
#define BLE_MCP_ACTION_SET_CURRENT_GROUP_OBJECT_ID_NOTIFICATION             (0x0000030B)    /**< This action sends a request to set notification of the Current Group Object ID characteristic. */
#define BLE_MCP_ACTION_SET_PLAYING_ORDER_NOTIFICATION                       (0x0000030C)    /**< This action sends a request to set notification of the playing order characteristic. */
#define BLE_MCP_ACTION_SET_MEDIA_STATE_NOTIFICATION                         (0x0000030D)    /**< This action sends a request to set notification of the media state characteristic. */
#define BLE_MCP_ACTION_SET_MEDIA_CONTROL_POINT_NOTIFICATION                 (0x0000030E)    /**< This action sends a request to set notification of the Media Control Point characteristic. */
#define BLE_MCP_ACTION_SET_MEDIA_CONTROL_OPCODES_SUPPORTED_NOTIFICATION     (0x0000030F)    /**< This action sends a request to set notification of the media control opcodes supported characteristic. */
#define BLE_MCP_ACTION_SET_SEARCH_CONTROL_POINT_NOTIFICATION                (0x00000310)    /**< This action sends a request to set notification of the Search Control Point characteristic. */
#define BLE_MCP_ACTION_SET_SEARCH_RESULTS_OBJECT_ID_NOTIFICATION            (0x00000311)    /**< This action sends a request to set notification of the Search Results Object ID characteristic. */
#define BLE_MCP_ACTION_INVALID                                              (0xFFFFFFFF)    /**< Invalid action type. */
typedef uint32_t ble_mcp_action_t;                                                      /**< The type of MCP actions.*/

/**
 *  @brief Defines the MCP events.
 */
#define BLE_MCP_MEDIA_PLAYER_NAME_IND                   (BT_LE_AUDIO_MODULE_MCP | 0x0001)    /**< This event indicates the media player name changes with payload #ble_mcp_media_player_name_ind_t. */
#define BLE_MCP_TRACK_CHANGED_IND                       (BT_LE_AUDIO_MODULE_MCP | 0x0002)    /**< This event indicates the track changed changes with payload #ble_mcp_track_changed_ind_t. */
#define BLE_MCP_TRACK_TITLE_IND                         (BT_LE_AUDIO_MODULE_MCP | 0x0003)    /**< This event indicates the track title changes with payload #ble_mcp_track_title_ind_t. */
#define BLE_MCP_TRACK_DURATION_IND                      (BT_LE_AUDIO_MODULE_MCP | 0x0004)    /**< This event indicates the track duration changes with payload #ble_mcp_track_duration_ind_t. */
#define BLE_MCP_TRACK_POSITION_IND                      (BT_LE_AUDIO_MODULE_MCP | 0x0005)    /**< This event indicates the track position changes with payload #ble_mcp_track_position_ind_t. */
#define BLE_MCP_PLAYBACK_SPEED_IND                      (BT_LE_AUDIO_MODULE_MCP | 0x0006)    /**< This event indicates the playback speed changes with payload #ble_mcp_playback_speed_ind_t. */
#define BLE_MCP_SEEKING_SPEED_IND                       (BT_LE_AUDIO_MODULE_MCP | 0x0007)    /**< This event indicates the seeking speed changes with payload #ble_mcp_seeking_speed_ind_t. */
#define BLE_MCP_PLAYING_ORDER_IND                       (BT_LE_AUDIO_MODULE_MCP | 0x000C)    /**< This event indicates the playing order changes with payload #ble_mcp_playing_order_ind_t. */
#define BLE_MCP_MEDIA_STATE_IND                         (BT_LE_AUDIO_MODULE_MCP | 0x000D)    /**< This event indicates the media state characteristic with payload #ble_mcp_media_state_ind_t. */
#define BLE_MCP_MEDIA_CONTROL_OPCODES_SUPPORTED_IND     (BT_LE_AUDIO_MODULE_MCP | 0x000F)    /**< This event indicates the media control opcodes supported changes with payload #ble_mcp_media_control_opcodes_supported_ind_t. */

#define BLE_MCP_READ_MEDIA_PLAYER_NAME_CNF                                  (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_READ_MEDIA_PLAYER_NAME)                /**< The result of reading media player name with payload #ble_mcp_read_media_player_name_cnf_t. */
#define BLE_MCP_READ_MEDIA_PLAYER_ICON_URL_CNF                              (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_READ_MEDIA_PLAYER_ICON_URL)            /**< The result of reading media player icon URL with payload #ble_mcp_read_media_player_icon_url_cnf_t. */
#define BLE_MCP_READ_TRACK_TITLE_CNF                                        (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_READ_TRACK_TITLE)                      /**< The result of reading track title with payload #ble_mcp_read_track_title_cnf_t. */
#define BLE_MCP_READ_TRACK_DURATION_CNF                                     (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_READ_TRACK_DURATION)                   /**< The result of reading track duration with payload #ble_mcp_read_track_duration_cnf_t. */
#define BLE_MCP_READ_TRACK_POSITION_CNF                                     (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_READ_TRACK_POSITION)                   /**< The result of reading track position with payload #ble_mcp_read_track_position_cnf_t. */
#define BLE_MCP_READ_PLAYBACK_SPEED_CNF                                     (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_READ_PLAYBACK_SPEED)                   /**< The result of reading playback speed with payload #ble_mcp_read_playback_speed_cnf_t. */
#define BLE_MCP_READ_SEEKING_SPEED_CNF                                      (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_READ_SEEKING_SPEED)                    /**< The result of reading seeking speed with payload #ble_mcp_read_seeking_speed_cnf_t. */
#define BLE_MCP_READ_PLAYING_ORDER_CNF                                      (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_READ_PLAYING_ORDER)                    /**< The result of reading playing order with payload #ble_mcp_read_playing_order_cnf_t. */
#define BLE_MCP_READ_PLAYING_ORDERS_SUPPORTED_CNF                           (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_READ_PLAYING_ORDERS_SUPPORTED)         /**< The result of reading playing orders supprted with payload #ble_mcp_read_playing_orders_supported_cnf_t. */
#define BLE_MCP_READ_MEDIA_STATE_CNF                                        (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_READ_MEDIA_STATE)                      /**< The result of reading media state with payload #ble_mcp_read_media_state_cnf_t. */
#define BLE_MCP_READ_MEDIA_CONTROL_OPCODES_SUPPORTED_CNF                    (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_READ_MEDIA_CONTROL_OPCODES_SUPPORTED)  /**< The result of reading media control opcodes supported with payload #ble_mcp_read_media_control_opcodes_supported_cnf_t. */
#define BLE_MCP_READ_CONTENT_CONTROL_ID_CNF                                 (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_READ_CONTENT_CONTROL_ID)               /**< The result of reading content control ID with payload #ble_mcp_read_content_control_id_cnf_t. */
#define BLE_MCP_READ_CURRENT_TRACK_OBJECT_INFORMATION_CNF                   (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_READ_CURRENT_TRACK_OBJECT_INFORMATION) /**< The result of reading current track object information #ble_mcp_read_current_track_information_cnf_t. */

#define BLE_MCP_SET_MEDIA_PLAYER_NAME_NOTIFICATION_CNF                   (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_SET_MEDIA_PLAYER_NAME_NOTIFICATION)               /**< The result of setting notification of the media player name characteristic with payload #ble_mcp_event_parameter_t. */
#define BLE_MCP_SET_TRACK_CHANGED_NOTIFICATION_CNF                       (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_SET_TRACK_CHANGED_NOTIFICATION)                   /**< The result of setting notification of the track changed characteristic with payload #ble_mcp_event_parameter_t. */
#define BLE_MCP_SET_TRACK_TITLE_NOTIFICATION_CNF                         (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_SET_TRACK_TITLE_NOTIFICATION)                     /**< The result of setting notification of the track title characteristic with payload #ble_mcp_event_parameter_t. */
#define BLE_MCP_SET_TRACK_DURATION_NOTIFICATION_CNF                      (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_SET_TRACK_DURATION_NOTIFICATION)                  /**< The result of setting notification of the track duration characteristic with payload #ble_mcp_event_parameter_t. */
#define BLE_MCP_SET_TRACK_POSITION_NOTIFICATION_CNF                      (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_SET_TRACK_POSITION_NOTIFICATION)                  /**< The result of setting notification of the track position characteristic with payload #ble_mcp_event_parameter_t. */
#define BLE_MCP_SET_PLAYBACK_SPEED_NOTIFICATION_CNF                      (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_SET_PLAYBACK_SPEED_NOTIFICATION)                  /**< The result of setting notification of the playback speed characteristic with payload #ble_mcp_event_parameter_t. */
#define BLE_MCP_SET_SEEKING_SPEED_NOTIFICATION_CNF                       (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_SET_SEEKING_SPEED_NOTIFICATION)                   /**< The result of setting notification of the seeking speed characteristic with payload #ble_mcp_event_parameter_t. */
#define BLE_MCP_SET_PLAYING_ORDER_NOTIFICATION_CNF                       (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_SET_PLAYING_ORDER_NOTIFICATION)                   /**< The result of setting notification of the playing order characteristic with payload #ble_mcp_event_parameter_t. */
#define BLE_MCP_SET_MEDIA_STATE_NOTIFICATION_CNF                         (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_SET_MEDIA_STATE_NOTIFICATION)                     /**< The result of setting notification of the media state characteristic with payload #ble_mcp_event_parameter_t. */
#define BLE_MCP_SET_MEDIA_CONTROL_OPCODES_SUPPORTED_NOTIFICATION_CNF     (BT_LE_AUDIO_MODULE_MCP | BLE_MCP_ACTION_SET_MEDIA_CONTROL_OPCODES_SUPPORTED_NOTIFICATION) /**< The result of setting notification of the media control opcodes supported characteristic with payload #ble_mcp_event_parameter_t. */

#define BLE_MCP_OBJECT_DATA_NOTIFY                                       (BT_LE_AUDIO_MODULE_MCP | 0xffff)
typedef uint32_t ble_mcp_event_t;                                           /**< The type of MCP events.*/

/**
 * @brief The GMCS service index.
 */
#define BLE_MCP_SERVICE_INDEX_GMCS              (0xFF)  /**< The service index of GMCS. */
#define BLE_MCP_GMCS_INDEX                       (0xFF)

/**
 * @}
 */

/**
 * @defgroup Bluetoothble_MCP_struct Struct
 * @{
 * This section defines basic data structures for the MCP.
 */

/**
 *  @brief This structure defines the parameter data type of set notification action.
 */
typedef struct {
    bool enable;    /**< Enable notification.*/
} ble_mcp_action_set_notification_t;

/**
 *  @brief This structure defines the parameter data type for event #BLE_MCP_MEDIA_PLAYER_NAME_IND.
 */
typedef struct {
    bt_handle_t handle;                 /**< Connection handle. */
    uint8_t service_idx;                /**< Service index. */
    uint8_t media_player_name_length;   /**< Media player name length. */
    uint8_t *p_media_player_name;       /**< Media player name. */
} ble_mcp_media_player_name_ind_t;

/**
 *  @brief This structure defines the parameter data type for event #BLE_MCP_TRACK_CHANGED_IND.
 */
typedef struct {
    bt_handle_t handle;                 /**< Connection handle. */
    uint8_t service_idx;                /**< Service index. */
} ble_mcp_track_changed_ind_t;

/**
 *  @brief This structure defines the parameter data type for event #BLE_MCP_TRACK_TITLE_IND.
 */
typedef struct {
    bt_handle_t handle;                 /**< Connection handle. */
    uint8_t service_idx;                /**< Service index. */
    uint8_t track_title_length;         /**< Track title length. */
    uint8_t *p_track_title;             /**< Track title. */
} ble_mcp_track_title_ind_t;

/**
 *  @brief This structure defines the parameter data type for event #BLE_MCP_TRACK_DURATION_IND.
 */
typedef struct {
    bt_handle_t handle;                 /**< Connection handle. */
    uint8_t service_idx;                /**< Service index. */
    int32_t track_duration;             /**< Track duration. */
} ble_mcp_track_duration_ind_t;

/**
 *  @brief This structure defines the parameter data type for event #BLE_MCP_TRACK_POSITION_IND.
 */
typedef struct {
    bt_handle_t handle;                 /**< Connection handle. */
    uint8_t service_idx;                /**< Service index. */
    int32_t track_position;             /**< Track position. */
} ble_mcp_track_position_ind_t;

/**
 *  @brief This structure defines the parameter data type for event #BLE_MCP_PLAYBACK_SPEED_IND.
 */
typedef struct {
    bt_handle_t handle;                 /**< Connection handle. */
    uint8_t service_idx;                /**< Service index. */
    int8_t playback_speed;              /**< Playback speed. */
} ble_mcp_playback_speed_ind_t;

/**
 *  @brief This structure defines the parameter data type for event #BLE_MCP_SEEKING_SPEED_IND.
 */
typedef struct {
    bt_handle_t handle;                 /**< Connection handle. */
    uint8_t service_idx;                /**< Service index. */
    int8_t seeking_speed;               /**< Seeking speed. */
} ble_mcp_seeking_speed_ind_t;

/**
 *  @brief This structure defines the parameter data type for event #BLE_MCP_PLAYING_ORDER_IND.
 */
typedef struct {
    bt_handle_t handle;                     /**< Connection handle. */
    uint8_t service_idx;                    /**< Service index. */
    ble_mcs_playing_order_t playing_order;  /**< Playing order. */
} ble_mcp_playing_order_ind_t;

/**
 *  @brief This structure defines the parameter data type for event #BLE_MCP_MEDIA_STATE_IND.
 */
typedef struct {
    bt_handle_t handle;                 /**< Connection handle. */
    uint8_t service_idx;                /**< Service index. */
    ble_mcs_media_state_t media_state;  /**< Media state. */
} ble_mcp_media_state_ind_t;

/**
 *  @brief This structure defines the parameter data type for event #BLE_MCP_MEDIA_CONTROL_OPCODES_SUPPORTED_IND.
 */
typedef struct {
    bt_handle_t handle;                 /**< Connection handle. */
    uint8_t service_idx;                /**< Service index. */
    ble_mcs_media_control_opcodes_supported_t media_control_opcodes_supported;  /**< The supported media control opcodes. */
} ble_mcp_media_control_opcodes_supported_ind_t;

/**
 *  @brief This structure defines the parameter data type for event #BLE_MCP_READ_MEDIA_PLAYER_NAME_CNF.
 */
typedef struct {
    bt_handle_t handle;                 /**< Connection handle. */
    uint8_t service_idx;                /**< Service index. */
    uint8_t media_player_name_length;   /**< Media player name length. */
    uint8_t *p_media_player_name;       /**< Media player name. */
} ble_mcp_read_media_player_name_cnf_t;

/**
 *  @brief This structure defines the parameter data type for event #BLE_MCP_READ_MEDIA_PLAYER_ICON_URL_CNF.
 */
typedef struct {
    bt_handle_t handle;                     /**< Connection handle. */
    uint8_t service_idx;                    /**< Service index. */
    uint8_t media_player_icon_url_length;   /**< Media player icon URL length. */
    uint8_t *p_media_player_icon_url;       /**< Media player icon URL. */
} ble_mcp_read_media_player_icon_url_cnf_t;

/**
 *  @brief This structure defines the parameter data type for event #BLE_MCP_READ_TRACK_TITLE_CNF.
 */
typedef struct {
    bt_handle_t handle;                 /**< Connection handle. */
    uint8_t service_idx;                /**< Service index. */
    uint8_t track_title_length;         /**< Track title length. */
    uint8_t *p_track_title;             /**< Track name. */
} ble_mcp_read_track_title_cnf_t;

/**
 *  @brief This structure defines the parameter data type for event #BLE_MCP_READ_TRACK_DURATION_CNF.
 */
typedef struct {
    bt_handle_t handle;                 /**< Connection handle. */
    uint8_t service_idx;                /**< Service index. */
    int32_t track_duration;             /**< Track duration. */
} ble_mcp_read_track_duration_cnf_t;

/**
 *  @brief This structure defines the parameter data type for event #BLE_MCP_READ_TRACK_POSITION_CNF.
 */
typedef struct {
    bt_handle_t handle;                 /**< Connection handle. */
    uint8_t service_idx;                /**< Service index. */
    int32_t track_position;             /**< Track position. */
} ble_mcp_read_track_position_cnf_t;

/**
 *  @brief This structure defines the parameter data type for event #BLE_MCP_READ_PLAYBACK_SPEED_CNF.
 */
typedef struct {
    bt_handle_t handle;                 /**< Connection handle. */
    uint8_t service_idx;                /**< Service index. */
    int8_t playback_speed;              /**< Playback speed. */
} ble_mcp_read_playback_speed_cnf_t;

/**
 *  @brief This structure defines the parameter data type for event #BLE_MCP_READ_SEEKING_SPEED_CNF.
 */
typedef struct {
    bt_handle_t handle;                 /**< Connection handle. */
    uint8_t service_idx;                /**< Service index. */
    int8_t seeking_speed;               /**< Seeking speed. */
} ble_mcp_read_seeking_speed_cnf_t;

/**
 *  @brief This structure defines the parameter data type for event #BLE_MCP_READ_PLAYING_ORDER_CNF.
 */
typedef struct {
    bt_handle_t handle;                     /**< Connection handle. */
    uint8_t service_idx;                    /**< Service index. */
    ble_mcs_playing_order_t playing_order;  /**< Playing order. */
} ble_mcp_read_playing_order_cnf_t;

/**
 *  @brief This structure defines the parameter data type for event #BLE_MCP_READ_PLAYING_ORDERS_SUPPORTED_CNF.
 */
typedef struct {
    bt_handle_t handle;                 /**< Connection handle. */
    uint8_t service_idx;                /**< Service index. */
    ble_mcs_playing_order_supported_t playing_order_supported;   /**< The supported playing order. */
} ble_mcp_read_playing_orders_supported_cnf_t;

/**
 *  @brief This structure defines the parameter data type for event #BLE_MCP_READ_MEDIA_STATE_CNF.
 */
typedef struct {
    bt_handle_t handle;                 /**< Connection handle. */
    uint8_t service_idx;                /**< Service index. */
    ble_mcs_media_state_t media_state;  /**< Media state. */
} ble_mcp_read_media_state_cnf_t;

/**
 *  @brief This structure defines the parameter data type for event #BLE_MCP_READ_MEDIA_CONTROL_OPCODES_SUPPORTED_CNF.
 */
typedef struct {
    bt_handle_t handle;                 /**< Connection handle. */
    uint8_t service_idx;                /**< Service index. */
    ble_mcs_media_control_opcodes_supported_t media_control_opcodes_supported;  /**< The supported media control opcodes. */
} ble_mcp_read_media_control_opcodes_supported_cnf_t;

/**
 *  @brief This structure defines the parameter data type for event #BLE_MCP_READ_CONTENT_CONTROL_ID_CNF.
 */
typedef struct {
    bt_handle_t handle;                 /**< Connection handle. */
    uint8_t service_idx;                /**< Service index. */
    uint8_t content_control_id;         /**< Content control ID. */
} ble_mcp_read_content_control_id_cnf_t;

/**
 *  @brief This structure defines the parameter data type for event #BLE_MCP_READ_CURRENT_TRACK_OBJECT_INFORMATION_CNF.
 */
typedef struct {
    bt_handle_t handle;                 /**< Connection handle. */
    uint8_t service_idx;                /**< Service index. */
} ble_mcp_read_current_track_information_cnf_t;

/**
 *  @brief This structure defines the parameter for MCP events.
 */
typedef struct {
    bt_handle_t handle;                 /**< Connection handle. */
    uint8_t service_idx;                /**< Service index. */
} ble_mcp_event_parameter_t;

/**
 *  @brief This structure defines the parameter for event #BLE_MCP_OBJECT_DATA_NOTIFY.
 */
typedef struct {
    bt_handle_t handle;                 /**< Connection handle. */
    uint8_t service_idx;              /**< Service index. */
    uint32_t offset;                    /**< Object data offset. */
    uint32_t data_size;                 /**< Object size. */
    uint8_t *p_data;                    /**< Object data. */
} ble_mcp_object_data_notify_t;
/**
 * @}
 */

/**
 * @brief                       This function triggers media action.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] action            is the action to be triggered.
 * @param[in] params            is the parameter of the related action.
 * @param[in] service_idx       is the service idx.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_UNSUPPORTED, the operation is unsupported.
 */
bt_status_t ble_mcp_send_action(bt_handle_t handle, ble_mcp_action_t action, void *params, uint8_t service_idx);

/**
 * @brief                       This function gets ID3 frame.
 * @param[in] id3_tag           is the ID3 tag got by current track object id.
 * @param[in] id3_frame_id      is the ID3 frame id, which is four characters.
 * @param[in] id3_frame_size    is the size of the id3 frame.
 * @return                      ID3 frame without frame header, if it is contained in ID3 tag.
 *                              NULL, if it is not contained in ID3 tag.
 */
uint8_t *ble_mcp_otp_get_id3_frame(uint8_t *id3_tag, uint8_t *id3_frame_id, uint32_t *id3_frame_size);

/**
 * @brief                       This function set media player play current track.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_play_current_track_req(bt_handle_t handle, uint8_t service_idx);

/**
 * @brief                       This function set media player play current track.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_play_current_track_cmd(bt_handle_t handle, uint8_t service_idx);

/**
 * @brief                       This function set media player pause current track.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_pause_current_track_req(bt_handle_t handle, uint8_t service_idx);

/**
 * @brief                       This function set media player move to previous track.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_move_to_previous_track_req(bt_handle_t handle, uint8_t service_idx);

/**
 * @brief                       This function set media player move to next track.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_move_to_next_track_req(bt_handle_t handle, uint8_t service_idx);

/**
 * @brief                       This function read media player icon object id.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_read_media_player_icon_object_id_req(bt_handle_t handle, uint8_t service_idx);

/**
 * @brief                       This function read track duration.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_read_track_duration_req(bt_handle_t handle, uint8_t service_idx);

/**
 * @brief                       This function read track position.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_read_track_position_req(bt_handle_t handle, uint8_t service_idx);

/**
 * @brief                       This function set track position.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @param[in] value             is the track position.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_write_track_position_cmd(bt_handle_t handle, uint8_t service_idx, uint8_t value);

/**
 * @brief                       This function set track position.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @param[in] value             is the track position.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_write_track_position_req(bt_handle_t handle, uint8_t service_idx, uint8_t value);

/**
 * @brief                       This function read media playback speed.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_read_playback_speed_req(bt_handle_t handle, uint8_t service_idx);

/**
 * @brief                       This function write media playback speed.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @param[in] value             is the playback speed.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_write_playback_speed_cmd(bt_handle_t handle, uint8_t service_idx, uint8_t value);

/**
 * @brief                       This function write media playback speed.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @param[in] value             is the playback speed.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_write_playback_speed_req(bt_handle_t handle, uint8_t service_idx, uint8_t value);

/**
 * @brief                       This function read media seeking speed.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_read_seeking_speed_req(bt_handle_t handle, uint8_t service_idx);

/**
 * @brief                       This function write media current track object id.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @param[in] value             is the object id.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_write_current_track_object_id_cmd(bt_handle_t handle, uint8_t service_idx, uint8_t value[BLE_MCS_OBJECT_ID_SIZE]);

/**
 * @brief                       This function write media current track object id.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @param[in] value             is the object id.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_write_current_track_object_id_req(bt_handle_t handle, uint8_t service_idx, uint8_t value[BLE_MCS_OBJECT_ID_SIZE]);

/**
 * @brief                       This function read media next track object id.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_read_next_track_object_id_req(bt_handle_t handle, uint8_t service_idx);

/**
 * @brief                       This function write media next track object id.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @param[in] value             is the next track object id.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_write_next_track_object_id_cmd(bt_handle_t handle, uint8_t service_idx, uint8_t value[BLE_MCS_OBJECT_ID_SIZE]);

/**
 * @brief                       This function write media next track object id.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @param[in] value             is the next track object id.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_write_next_track_object_id_req(bt_handle_t handle, uint8_t service_idx, uint8_t value[BLE_MCS_OBJECT_ID_SIZE]);

/**
 * @brief                       This function read media parent group object id.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_read_parent_group_object_id_req(bt_handle_t handle, uint8_t service_idx);

/**
 * @brief                       This function read media current group object id.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_read_current_group_object_id_req(bt_handle_t handle, uint8_t service_idx);

/**
 * @brief                       This function write media current group object id.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @param[in] value             is the current group object id.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_write_current_group_object_id_cmd(bt_handle_t handle, uint8_t service_idx, uint8_t value[BLE_MCS_OBJECT_ID_SIZE]);

/**
 * @brief                       This function write media current group object id.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @param[in] value             is the current group object id.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_write_current_group_object_id_req(bt_handle_t handle, uint8_t service_idx, uint8_t value[BLE_MCS_OBJECT_ID_SIZE]);

/**
 * @brief                       This function read media playing order.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_read_playing_order_req(bt_handle_t handle, uint8_t service_idx);

/**
 * @brief                       This function write media playing order.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @param[in] playing_order     is the playing order.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_write_playing_order_cmd(bt_handle_t handle, uint8_t service_idx, ble_mcs_playing_order_t playing_order);

/**
 * @brief                       This function write media playing order.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @param[in] playing_order     is the playing order.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_write_playing_order_req(bt_handle_t handle, uint8_t service_idx, ble_mcs_playing_order_t playing_order);

/**
 * @brief                       This function read media state.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_read_media_state_req(bt_handle_t handle, uint8_t service_idx);

/**
 * @brief                       This function set media player fast rewind.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_fast_rewind_req(bt_handle_t handle, uint8_t service_idx);

/**
 * @brief                       This function set media player fast forward.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_fast_forward_req(bt_handle_t handle, uint8_t service_idx);

/**
 * @brief                       This function set media player stop current track.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_stop_current_track_req(bt_handle_t handle, uint8_t service_idx);

/**
 * @brief                       This function set media player relative track position.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @param[in] offset            is the offset.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_set_relative_track_position_req(bt_handle_t handle, uint8_t service_idx, int32_t offset);

/**
 * @brief                       This function set media player move to previous segment.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_move_to_previous_segment_req(bt_handle_t handle, uint8_t service_idx);

/**
 * @brief                       This function set media player move to next segment.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_move_to_next_segment_req(bt_handle_t handle, uint8_t service_idx);

/**
 * @brief                       This function set media player move to first segment.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_move_to_first_segment_req(bt_handle_t handle, uint8_t service_idx);

/**
 * @brief                       This function set media player move to last segment.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_move_to_last_segment_req(bt_handle_t handle, uint8_t service_idx);

/**
 * @brief                       This function set media player move to the segment by segment number.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @param[in] segment_number    is the segment number.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_move_to_segment_number_req(bt_handle_t handle, uint8_t service_idx, int32_t seg_number);

/**
 * @brief                       This function set media player move to first track.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_move_to_first_track_req(bt_handle_t handle, uint8_t service_idx);

/**
 * @brief                       This function set media player move to last track.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_move_to_last_track_req(bt_handle_t handle, uint8_t service_idx);

/**
 * @brief                       This function set media player move track byt the track number.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @param[in] track_number      is the track number.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_move_to_track_number_req(bt_handle_t handle, uint8_t service_idx, int32_t track_number);

/**
 * @brief                       This function set media player move to the previous group.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_move_to_previous_group_req(bt_handle_t handle, uint8_t service_idx);

/**
 * @brief                       This function set media player move to the next group.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_move_to_next_group_req(bt_handle_t handle, uint8_t service_idx);

/**
 * @brief                       This function set media player move to the first group.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_move_to_first_group_req(bt_handle_t handle, uint8_t service_idx);

/**
 * @brief                       This function set media player move to the last group.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_move_to_last_group_req(bt_handle_t handle, uint8_t service_idx);

/**
 * @brief                       This function set media player move to the group by group number.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @param[in] group_number      is the group number.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_move_to_group_number_req(bt_handle_t handle, uint8_t service_idx, int32_t group_number);

/**
 * @brief                       This function read media control opcodes supported.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_read_media_control_opcodes_supported_req(bt_handle_t handle, uint8_t service_idx);

/**
 * @brief                       This function read media player icon url.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_read_media_player_icon_url_req(bt_handle_t handle, uint8_t service_idx);

/**
 * @brief                       This function read media player name.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_read_media_player_name_req(bt_handle_t handle, uint8_t service_idx);

/**
 * @brief                       This function read track title.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_read_track_title_req(bt_handle_t handle, uint8_t service_idx);

/**
 * @brief                       This function read playing order supported.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_read_playing_order_supported_req(bt_handle_t handle, uint8_t service_idx);

/**
 * @brief                       This function set media player absolute track position.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @param[in] track_position    is the absolute track position.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_set_absolute_track_position_req(bt_handle_t handle, uint8_t service_idx, int32_t track_position);

/**
 * @brief                       This function set media player absolute track position.
 * @param[in] handle            is the connection handle of the Bluetooth link.
 * @param[in] service_idx       is the service idx.
 * @param[in] track_position    is the absolute track position.
 * @return                      #BT_STATUS_SUCCESS, the operation completed successfully.
 *                              #BT_STATUS_FAIL, the operation has failed.
 *                              #BT_STATUS_CONNECTION_NOT_FOUND, link not exist.
 *                              #BT_STATUS_CONNECTION_IN_USE, other command in process.
 */
bt_status_t ble_mcp_set_absolute_track_position_cmd(bt_handle_t handle, uint8_t service_idx, int32_t track_position);

/* Unsupported*/
bt_status_t ble_mcp_set_xxx_req(bt_handle_t handle, uint8_t service_idx);


/**
 * @}
 * @}
 * @}
 */

#endif

