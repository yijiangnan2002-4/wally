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

/* include */
#include "FreeRTOS.h"
#include <stdlib.h>
#include "bt_sink_srv_le_cap.h"
#include "atci.h"
#include "bt_le_audio_msglog.h"

/* mcp include */
#include "ble_mcp.h"

/* ccp include */
#include "ble_ccp.h"

/* pacs include */
#include "ble_pacs.h"

/* mics & aics include */
#include "ble_mics.h"
#include "ble_aics.h"

/* Prototype */
atci_status_t bt_le_audio_atci_cmd_handler(atci_parse_cmd_param_t *parse_cmd);

/* extern */
extern bt_status_t ble_csis_set_lock_state(uint8_t state);
extern void ble_csis_send_lock_notify_req(void);
extern void bt_device_manager_le_clear_all_bonded_info(void);
extern bt_status_t ble_csis_config_encrypted_sirk(bool encrypted_sirk);
extern void ble_csis_write_nvkey_sirk(bt_key_t *sirk);

#define BT_PTS_STATE_IDLE                    (0)
#define BT_PTS_STATE_WRITE                   (1)
#define BT_PTS_STATE_WRITE_WITHOUT_RESPONSE  (2)
#define BT_PTS_STATE_READ                    (3)


#define BT_PTS_CCP_URI_LEN                   (8)
#define BT_PTS_CCP_OPCODE_LEN                (1) /* opcode(1) */
#define BT_PTS_CCP_CP_DEFAULT_LEN            (2) /* opcode(1), default_param_len(1) */
#define BT_PTS_CCP_JOIN_CALL_NUM             (2)

const static uint8_t bt_app_pts_uri[BT_PTS_CCP_URI_LEN] = {'t', 'e', 'l', ':', '+', '1', '5', '0'};
const static uint8_t bt_app_pts_join_call_id[BT_PTS_CCP_JOIN_CALL_NUM] = {1, 2};

// for upper layer to check pts enable status
bool bt_le_audio_pts_test_enable = false;

static uint8_t bt_app_pts_state = BT_PTS_STATE_IDLE;

static atci_cmd_hdlr_item_t bt_le_audio_atci_cmd[] = {
    {
        .command_head = "AT+PTSLEA",    /* AT+PTSLEA=<cmd> */
        .command_hdlr = bt_le_audio_atci_cmd_handler,
        .hash_value1 = 0,
        .hash_value2 = 0,
    }
};

bt_status_t bt_le_audio_at_cmd_pts_mcp_hdl(char *pChar)
{
    bt_status_t result = BT_STATUS_UNSUPPORTED;
    ble_mcp_event_t event_id, write_command = 0;
    bool enable = false;
    bt_handle_t handle = bt_sink_srv_cap_get_link_handle(0xFF);
    uint8_t service_idx = BLE_MCP_SERVICE_INDEX_GMCS;
    uint8_t object_id[BLE_MCS_OBJECT_ID_SIZE] = {0};

    object_id[0] = 10;

    LOG_MSGID_I(BT_APP, "[MCP] bt_app_comm_at_cmd_mcp_hdl\r\n", 0);

    /* AT+PTSLEA=PTSMCP,<cmdType>,<cmdNum>
    * "R,xx" --> Read characteristic xx
    * "W,xx" --> Write characteristic xx
    * "N,xx" --> Write CCCD xx
    *
    * PTS at cmd format:
    * AT+PTSLEA=PTSMCP,MCP/CL/CGGIT/CHA/BV-06-C\0d\0a for PTS test case(MCP/CL/CGGIT/CHA/BV-06-C)
    */

    if (NULL != strstr(pChar, "PLAY")) {
        pChar = strchr(pChar, ',');

        if (pChar == NULL)
            result = ble_mcp_play_current_track_req(handle, service_idx);
        else {
            pChar++;
            write_command = atoi(pChar);
            if(write_command == 0)
                result = ble_mcp_play_current_track_req(handle, service_idx);
            else
                result = ble_mcp_play_current_track_cmd(handle, service_idx);
        }
    } else if (NULL != strstr(pChar, "PAUSE")) {
        result = ble_mcp_pause_current_track_req(handle, service_idx);
    } else if (NULL != strstr(pChar, "PREV")) {
        result = ble_mcp_move_to_previous_track_req(handle, service_idx);
    } else if (NULL != strstr(pChar, "NEXT")) {
        result = ble_mcp_move_to_next_track_req(handle, service_idx);
    /* Map to PTS test case */
    /* CGGIT */
    /* MCP/CL/CGGIT/SER/BV-01-C: do nothing */
    /* MCP/CL/CGGIT/SER/BV-02-C: do nothing */
    /* MCP/CL/CGGIT/SER/BV-03-C: do nothing */
    /* MCP/CL/CGGIT/CHA/BV-01-C: do nothing */
    /* MCP/CL/CGGIT/CHA/BV-23-C: do nothing */
    } else if (NULL != strstr(pChar, "MCP/CL/CGGIT/CHA/BV-02-C")) {
        result = ble_mcp_read_media_player_icon_object_id_req(handle, service_idx);
    /* MCP/CL/CGGIT/CHA/BV-03-C: do nothing */
    /* MCP/CL/CGGIT/CHA/BV-04-C: do nothing */
    /* MCP/CL/CGGIT/CHA/BV-05-C: do nothing */
    } else if (NULL != strstr(pChar, "MCP/CL/CGGIT/CHA/BV-06-C")) {
        result = ble_mcp_read_track_duration_req(handle, service_idx);
    } else if (NULL != strstr(pChar, "MCP/CL/CGGIT/CHA/BV-07-C")) {
        if (BT_PTS_STATE_IDLE == bt_app_pts_state) {
            bt_app_pts_state = BT_PTS_STATE_WRITE_WITHOUT_RESPONSE;
            result = ble_mcp_read_track_position_req(handle, service_idx);
        } else if (BT_PTS_STATE_WRITE_WITHOUT_RESPONSE == bt_app_pts_state) {
            /* write cmd without rsp */
            bt_app_pts_state = BT_PTS_STATE_WRITE;
            result = ble_mcp_write_track_position_cmd(handle, service_idx, 0);
        } else {
            /* write cmd with rsp */
            bt_app_pts_state = BT_PTS_STATE_IDLE;
            result = ble_mcp_write_track_position_req(handle, service_idx, 0);
        }
    } else if (NULL != strstr(pChar, "MCP/CL/CGGIT/CHA/BV-08-C")) {
        if (BT_PTS_STATE_IDLE == bt_app_pts_state) {
            bt_app_pts_state = BT_PTS_STATE_WRITE_WITHOUT_RESPONSE;
            result = ble_mcp_read_playback_speed_req(handle, service_idx);
        } else if (BT_PTS_STATE_WRITE_WITHOUT_RESPONSE == bt_app_pts_state) {
            /* write cmd without rsp */
            bt_app_pts_state = BT_PTS_STATE_WRITE;
            result = ble_mcp_write_playback_speed_cmd(handle, service_idx, 0);
        } else {
            /* write cmd with rsp */
            bt_app_pts_state = BT_PTS_STATE_IDLE;
            result = ble_mcp_write_playback_speed_req(handle, service_idx, 0);
        }
    } else if (NULL != strstr(pChar, "MCP/CL/CGGIT/CHA/BV-09-C")) {
        result = ble_mcp_read_seeking_speed_req(handle, service_idx);
    } else if (NULL != strstr(pChar, "MCP/CL/CGGIT/CHA/BV-10-C")) {
        result = ble_mcp_send_action(handle, BLE_MCP_ACTION_READ_CURRENT_TRACK_SEGMENTS_OBJECT_ID, NULL, service_idx);
    } else if (NULL != strstr(pChar, "MCP/CL/CGGIT/CHA/BV-11-C")) {
        if (BT_PTS_STATE_IDLE == bt_app_pts_state) {
            bt_app_pts_state = BT_PTS_STATE_WRITE_WITHOUT_RESPONSE;
            result = ble_mcp_send_action(handle, BLE_MCP_ACTION_READ_CURRENT_TRACK_OBJECT_ID, NULL, service_idx);
        } else if (BT_PTS_STATE_WRITE_WITHOUT_RESPONSE == bt_app_pts_state) {
            /* write cmd without rsp */
            uint8_t id[BLE_MCS_OBJECT_ID_SIZE] = {20, 0, 0, 0, 0, 0};
            bt_app_pts_state = BT_PTS_STATE_WRITE;
            result = ble_mcp_write_current_track_object_id_cmd(handle, service_idx, id);
        } else {
            /* write cmd with rsp */
            uint8_t id[BLE_MCS_OBJECT_ID_SIZE] = {20, 0, 0, 0, 0, 0};
            bt_app_pts_state = BT_PTS_STATE_IDLE;
            result = ble_mcp_write_current_track_object_id_req(handle, service_idx, id);
        }
    } else if (NULL != strstr(pChar, "MCP/CL/CGGIT/CHA/BV-12-C")) {
        if (BT_PTS_STATE_IDLE == bt_app_pts_state) {
            bt_app_pts_state = BT_PTS_STATE_WRITE_WITHOUT_RESPONSE;
            result = ble_mcp_read_next_track_object_id_req(handle, service_idx);
        } else if (BT_PTS_STATE_WRITE_WITHOUT_RESPONSE == bt_app_pts_state) {
            /* write cmd without rsp */
            bt_app_pts_state = BT_PTS_STATE_WRITE;
            result = ble_mcp_write_next_track_object_id_cmd(handle, service_idx, object_id);
        } else {
            /* write cmd with rsp */
            bt_app_pts_state = BT_PTS_STATE_IDLE;
            result = ble_mcp_write_next_track_object_id_req(handle, service_idx, object_id);
        }
    } else if (NULL != strstr(pChar, "MCP/CL/CGGIT/CHA/BV-13-C")) {
        result = ble_mcp_read_parent_group_object_id_req(handle, service_idx);
    } else if (NULL != strstr(pChar, "MCP/CL/CGGIT/CHA/BV-14-C")) {
        if (BT_PTS_STATE_IDLE == bt_app_pts_state) {
            bt_app_pts_state = BT_PTS_STATE_WRITE_WITHOUT_RESPONSE;
            result = ble_mcp_read_current_group_object_id_req(handle, service_idx);
        } else if (BT_PTS_STATE_WRITE_WITHOUT_RESPONSE == bt_app_pts_state) {
            /* write cmd without rsp */
            bt_app_pts_state = BT_PTS_STATE_WRITE;
            result = ble_mcp_write_current_group_object_id_cmd(handle, service_idx, object_id);
        } else {
            /* write cmd with rsp */
            bt_app_pts_state = BT_PTS_STATE_IDLE;
            result = ble_mcp_write_current_group_object_id_req(handle, service_idx, object_id);
        }
    } else if (NULL != strstr(pChar, "MCP/CL/CGGIT/CHA/BV-15-C")) {
        if (BT_PTS_STATE_IDLE == bt_app_pts_state) {
            bt_app_pts_state = BT_PTS_STATE_WRITE_WITHOUT_RESPONSE;
            result = ble_mcp_read_playing_order_req(handle, service_idx);
        } else if (BT_PTS_STATE_WRITE_WITHOUT_RESPONSE == bt_app_pts_state) {
            /* write cmd without rsp */
            bt_app_pts_state = BT_PTS_STATE_WRITE;
            result = ble_mcp_write_playing_order_cmd(handle, service_idx, BLE_MCS_PLAYING_ORDER_SINGLE_ONCE);
        } else {
            /* write cmd with rsp */
            bt_app_pts_state = BT_PTS_STATE_IDLE;
            result = ble_mcp_write_playing_order_req(handle, service_idx, BLE_MCS_PLAYING_ORDER_SINGLE_ONCE);
        }
    /* MCP/CL/CGGIT/CHA/BV-16-C: do nothing */
    } else if (NULL != strstr(pChar, "MCP/CL/CGGIT/CHA/BV-17-C")) {
        result = ble_mcp_read_media_state_req(handle, service_idx);
    /* MCP/CL/CGGIT/CHA/BV-18-C: do nothing */
    /* MCP/CL/CGGIT/CHA/BV-19-C: do nothing */
    } else if (NULL != strstr(pChar, "MCP/CL/CGGIT/CHA/BV-22-C")) {
        result = ble_mcp_send_action(handle, BLE_MCP_ACTION_READ_CONTENT_CONTROL_ID, NULL, service_idx);
    /* MCCP */
    } else if (NULL != strstr(pChar, "MCP/CL/MCCP/BV-01-C")) {
        result = ble_mcp_play_current_track_req(handle, service_idx);
    } else if (NULL != strstr(pChar, "MCP/CL/MCCP/BV-02-C")) {
        result = ble_mcp_pause_current_track_req(handle, service_idx);
    } else if (NULL != strstr(pChar, "MCP/CL/MCCP/BV-03-C")) {
        result = ble_mcp_fast_rewind_req(handle, service_idx);
    } else if (NULL != strstr(pChar, "MCP/CL/MCCP/BV-04-C")) {
        result = ble_mcp_fast_forward_req(handle, service_idx);
    } else if (NULL != strstr(pChar, "MCP/CL/MCCP/BV-05-C")) {
        result = ble_mcp_stop_current_track_req(handle, service_idx);
    } else if (NULL != strstr(pChar, "MCP/CL/MCCP/BV-06-C")) {
        result = ble_mcp_set_relative_track_position_req(handle, service_idx, 1);
    } else if (NULL != strstr(pChar, "MCP/CL/MCCP/BV-07-C")) {
        result = ble_mcp_move_to_previous_segment_req(handle, service_idx);
    } else if (NULL != strstr(pChar, "MCP/CL/MCCP/BV-08-C")) {
        result = ble_mcp_move_to_next_segment_req(handle, service_idx);
    } else if (NULL != strstr(pChar, "MCP/CL/MCCP/BV-09-C")) {
        result = ble_mcp_move_to_first_segment_req(handle, service_idx);
    } else if (NULL != strstr(pChar, "MCP/CL/MCCP/BV-10-C")) {
        result = ble_mcp_move_to_last_segment_req(handle, service_idx);
    } else if (NULL != strstr(pChar, "MCP/CL/MCCP/BV-11-C")) {
        result = ble_mcp_move_to_segment_number_req(handle, service_idx, 1);
    } else if (NULL != strstr(pChar, "MCP/CL/MCCP/BV-12-C")) {
        result = ble_mcp_move_to_previous_track_req(handle, service_idx);
    } else if (NULL != strstr(pChar, "MCP/CL/MCCP/BV-13-C")) {
        result = ble_mcp_move_to_next_track_req(handle, service_idx);
    } else if (NULL != strstr(pChar, "MCP/CL/MCCP/BV-14-C")) {
        result = ble_mcp_move_to_first_track_req(handle, service_idx);
    } else if (NULL != strstr(pChar, "MCP/CL/MCCP/BV-15-C")) {
        result = ble_mcp_move_to_last_track_req(handle, service_idx);
    } else if (NULL != strstr(pChar, "MCP/CL/MCCP/BV-16-C")) {
        result = ble_mcp_move_to_track_number_req(handle, service_idx, 1);
    } else if (NULL != strstr(pChar, "MCP/CL/MCCP/BV-17-C")) {
        result = ble_mcp_move_to_previous_group_req(handle, service_idx);
    } else if (NULL != strstr(pChar, "MCP/CL/MCCP/BV-18-C")) {
        result = ble_mcp_move_to_next_group_req(handle, service_idx);
    } else if (NULL != strstr(pChar, "MCP/CL/MCCP/BV-19-C")) {
        result = ble_mcp_move_to_first_group_req(handle, service_idx);
    } else if (NULL != strstr(pChar, "MCP/CL/MCCP/BV-20-C")) {
        result = ble_mcp_move_to_last_group_req(handle, service_idx);
    } else if (NULL != strstr(pChar, "MCP/CL/MCCP/BV-21-C")) {
        result = ble_mcp_move_to_group_number_req(handle, service_idx, 1);
    /* SPE */
    } else if (NULL != strstr(pChar, "MCP/CL/SPE/BI-01-C")) {
        result = ble_mcp_play_current_track_cmd(handle, service_idx);
    } else {
        event_id = atoi(pChar);

        LOG_MSGID_I(BT_APP, "[MCP] AT+MCP service[0x%x] handle[0x%04x] event_id[%02x]\r\n", 3, service_idx, handle, event_id);

        if (event_id >= BLE_MCP_SET_TRACK_CHANGED_NOTIFICATION_CNF && event_id <= BLE_MCP_SET_MEDIA_CONTROL_POINT_NOTIFICATION_CNF) {
            pChar++;
            pChar = strchr(pChar, ',');
            if (pChar == NULL){
                LOG_MSGID_I(BT_APP, "[MCP] AT+MCP cmd String error!!\r\n", 0);
            } else {
                if (strstr(pChar, "E") != NULL) {
                    enable = TRUE;
                } else {
                    enable = FALSE;
                }
            }
        }

        switch (event_id) {
            /* READ/WRITE */
            case BLE_MCP_READ_CONTENT_CONTROL_ID_CNF:
                result = ble_mcp_send_action(handle, BLE_MCP_ACTION_READ_CONTENT_CONTROL_ID, NULL, service_idx);
                break;
            case BLE_MCP_PLAY_CURRENT_TRACK_CNF:
                result = ble_mcp_play_current_track_req(handle, service_idx);
                break;
            case BLE_MCP_PAUSE_CURRENT_TRACK_CNF:
                result = ble_mcp_pause_current_track_req(handle, service_idx);
                break;
            case BLE_MCP_STOP_CURRENT_TRACK_CNF:
                result = ble_mcp_stop_current_track_req(handle, service_idx);
                break;
            case BLE_MCP_MOVE_TO_NEXT_TRACK_CNF:
                result = ble_mcp_move_to_next_track_req(handle, service_idx);
                break;
            case BLE_MCP_MOVE_TO_PREVIOUS_TRACK_CNF:
                result = ble_mcp_move_to_previous_track_req(handle, service_idx);
                break;
            case BLE_MCP_FAST_FORWARD_CNF:
                result = ble_mcp_fast_forward_req(handle, service_idx);
                break;
            case BLE_MCP_FAST_REWIND_CNF:
                result = ble_mcp_fast_rewind_req(handle, service_idx);
                break;
            case BLE_MCP_READ_PLAYBACK_SPEED_CNF:
                result = ble_mcp_read_playback_speed_req(handle, service_idx);
                break;
            case BLE_MCP_WRITE_PLAYBACK_SPEED_CNF:
                pChar = strchr(pChar, ',');
                if (pChar == NULL)
                    result = ble_mcp_write_playback_speed_req(handle, service_idx, 1);
                else {
                    pChar++;
                    write_command = atoi(pChar);
                    LOG_MSGID_I(common, "PLAYBACK_SPEED write_command=%d\n", 1, write_command);
                    if(write_command == 0)
                        result = ble_mcp_write_playback_speed_req(handle, service_idx, 1);
                    else
                        result = ble_mcp_write_playback_speed_cmd(handle, service_idx, 1);
                }

                break;
            case BLE_MCP_READ_PLAYING_ORDER_CNF:
                result = ble_mcp_read_playing_order_req(handle, service_idx);
                break;
            case BLE_MCP_WRITE_PLAYING_ORDER_CNF:
            {
                pChar = strchr(pChar, ',');
                if (pChar == NULL){
                    LOG_MSGID_I(BT_APP, "[MCP] AT+MCP cmd String error!!\r\n", 0);
                    return false;
                }

                pChar++;

                uint8_t playing_order = atoi(pChar);

                pChar = strchr(pChar, ',');
                if (pChar == NULL) {
                    LOG_MSGID_I(BT_APP, "[MCP] pChar == NULL\r\n", 0);
                    result = ble_mcp_write_playing_order_req(handle, service_idx, playing_order);
                }
                else {
                    pChar++;
                    write_command = atoi(pChar);

                    if(write_command == 0)
                        result = ble_mcp_write_playing_order_req(handle, service_idx, playing_order);
                    else
                        result = ble_mcp_write_playing_order_cmd(handle, service_idx, playing_order);
                }

                break;
            }
            case BLE_MCP_READ_MEDIA_CONTROL_OPCODES_SUPPORTED_CNF:
                result = ble_mcp_read_media_control_opcodes_supported_req(handle, service_idx);
                break;
            case BLE_MCP_READ_MEDIA_PLAYER_ICON_URL_CNF:
                result = ble_mcp_read_media_player_icon_url_req(handle, service_idx);
                break;
            case BLE_MCP_READ_MEDIA_PLAYER_NAME_CNF:
                result = ble_mcp_read_media_player_name_req(handle, service_idx);
                break;
            case BLE_MCP_READ_TRACK_TITLE_CNF:
                result = ble_mcp_read_track_title_req(handle, service_idx);
                break;
            case BLE_MCP_READ_TRACK_POSITION_CNF:
                result = ble_mcp_read_track_position_req(handle, service_idx);
                break;
            case BLE_MCP_READ_TRACK_DURATION_CNF:
                result = ble_mcp_read_track_duration_req(handle, service_idx);
                break;
            case BLE_MCP_READ_SEEKING_SPEED_CNF:
                result = ble_mcp_read_seeking_speed_req(handle, service_idx);
                break;
            case BLE_MCP_READ_MEDIA_STATE_CNF:
                result = ble_mcp_read_media_state_req(handle, service_idx);
                break;
            case BLE_MCP_READ_PLAYING_ORDERS_SUPPORTED_CNF:
                result = ble_mcp_read_playing_order_supported_req(handle, service_idx);
                break;
            case BLE_MCP_MOVE_TO_FIRST_TRACK_CNF:
                result = ble_mcp_move_to_first_track_req(handle, service_idx);
                break;
            case BLE_MCP_MOVE_TO_LAST_TRACK_CNF:
                result = ble_mcp_move_to_last_track_req(handle, service_idx);
                break;
            case BLE_MCP_MOVE_TO_TRACK_NUMBER_CNF: {
                pChar = strchr(pChar, ',');
                if (pChar == NULL){
                    LOG_MSGID_I(BT_APP, "[MCP] AT+MCP cmd String error!!\r\n", 0);
                    return false;
                }
                pChar++;
                result = ble_mcp_move_to_track_number_req(handle, service_idx, atoi(pChar));
                break;
            }
            case BLE_MCP_MOVE_TO_NEXT_GROUP_CNF:
                result = ble_mcp_move_to_next_group_req(handle, service_idx);
                break;
            case BLE_MCP_MOVE_TO_PREVIOUS_GROUP_CNF:
                result = ble_mcp_move_to_previous_group_req(handle, service_idx);
                break;
            case BLE_MCP_MOVE_TO_FIRST_GROUP_CNF:
                result = ble_mcp_move_to_first_group_req(handle, service_idx);
                break;
            case BLE_MCP_MOVE_TO_LAST_GROUP_CNF:
                result = ble_mcp_move_to_last_group_req(handle, service_idx);
                break;
            case BLE_MCP_MOVE_TO_GROUP_NUMBER_CNF:
                pChar = strchr(pChar, ',');
                if (pChar == NULL){
                    LOG_MSGID_I(BT_APP, "[MCP] AT+MCP cmd String error!!\r\n", 0);
                    return false;
                }
                pChar++;
                result = ble_mcp_move_to_group_number_req(handle, service_idx, atoi(pChar));
                break;
            case BLE_MCP_MOVE_TO_NEXT_SEGMENT_CNF:
                result = ble_mcp_move_to_next_segment_req(handle, service_idx);
                break;
            case BLE_MCP_MOVE_TO_PREVIOUS_SEGMENT_CNF:
                result = ble_mcp_move_to_previous_segment_req(handle, service_idx);
                break;
            case BLE_MCP_MOVE_TO_FIRST_SEGMENT_CNF:
                result = ble_mcp_move_to_first_segment_req(handle, service_idx);
                break;
            case BLE_MCP_MOVE_TO_LAST_SEGMENT_CNF:
                result = ble_mcp_move_to_last_segment_req(handle, service_idx);
                break;
            case BLE_MCP_MOVE_TO_SEGMENT_NUMBER_CNF:
                pChar = strchr(pChar, ',');
                if (pChar == NULL){
                    LOG_MSGID_I(BT_APP, "[MCP] AT+MCP cmd String error!!\r\n", 0);
                    return false;
                }
                pChar++;
                result = ble_mcp_move_to_segment_number_req(handle, service_idx, atoi(pChar));
                break;
            case BLE_MCP_SET_ABSOLUTE_TRACK_POSITION_CNF:
            {
                pChar = strchr(pChar, ',');
                if (pChar == NULL){
                    LOG_MSGID_I(BT_APP, "[MCP] AT+MCP cmd String error!!\r\n", 0);
                    return false;
                }
                pChar++;
                int32_t track_position = atoi(pChar);

                pChar = strchr(pChar, ',');
                if (pChar == NULL)
                    result = ble_mcp_set_absolute_track_position_req(handle, service_idx, track_position);
                else {
                    pChar++;
                    write_command = atoi(pChar);
                    if(write_command == 0)
                        result = ble_mcp_set_absolute_track_position_req(handle, service_idx, track_position);
                    else
                        result = ble_mcp_set_absolute_track_position_cmd(handle, service_idx, track_position);
                }

                break;
            }
            case BLE_MCP_SET_RELATIVE_TRACK_POSITION_CNF:
                pChar = strchr(pChar, ',');
                if (pChar == NULL){
                    LOG_MSGID_I(BT_APP, "[MCP] AT+MCP cmd String error!!\r\n", 0);
                    return false;
                }
                pChar++;
                result = ble_mcp_set_relative_track_position_req(handle, service_idx, atoi(pChar));
                break;
            case BLE_MCP_WRITE_MCP_UNSUPPORTED_OPCODE_CNF:
                result = ble_mcp_set_xxx_req(handle, service_idx);
                break;
            /* NOTIFY */
            case BLE_MCP_SET_TRACK_CHANGED_NOTIFICATION_CNF:
                result = ble_mcp_send_action(handle, BLE_MCP_ACTION_SET_TRACK_CHANGED_NOTIFICATION, (void *)&enable, service_idx);
                break;
            case BLE_MCP_SET_SEEKING_SPEED_NOTIFICATION_CNF:
                result = ble_mcp_send_action(handle, BLE_MCP_ACTION_SET_SEEKING_SPEED_NOTIFICATION, (void *)&enable, service_idx);
                break;
            case BLE_MCP_SET_MEDIA_STATE_NOTIFICATION_CNF:
                result = ble_mcp_send_action(handle, BLE_MCP_ACTION_SET_MEDIA_STATE_NOTIFICATION, (void *)&enable, BLE_MCP_SERVICE_INDEX_GMCS);
                break;
            case BLE_MCP_SET_MEDIA_PLAYER_NAME_NOTIFICATION_CNF:
                result = ble_mcp_send_action(handle, BLE_MCP_ACTION_SET_MEDIA_PLAYER_NAME_NOTIFICATION, (void *)&enable, BLE_MCP_SERVICE_INDEX_GMCS);
                break;
            case BLE_MCP_SET_TRACK_TITLE_NOTIFICATION_CNF:
                result = ble_mcp_send_action(handle, BLE_MCP_ACTION_SET_TRACK_TITLE_NOTIFICATION, (void *)&enable, service_idx);
                break;
            case BLE_MCP_SET_TRACK_DURATION_NOTIFICATION_CNF:
                result = ble_mcp_send_action(handle, BLE_MCP_ACTION_SET_TRACK_DURATION_NOTIFICATION, (void *)&enable, service_idx);
                break;
            case BLE_MCP_SET_TRACK_POSITION_NOTIFICATION_CNF:
                result = ble_mcp_send_action(handle, BLE_MCP_ACTION_SET_TRACK_DURATION_NOTIFICATION, (void *)&enable, service_idx);
                break;
            case BLE_MCP_SET_PARENT_GROUP_OBJECT_ID_NOTIFICATION_CNF:
                //result = ble_mcp_set_parent_group_object_id_notification_req(handle, service_idx, enable);
                break;
            case BLE_MCP_SET_PLAYBACK_SPEED_NOTIFICATION_CNF:
                result = ble_mcp_send_action(handle, BLE_MCP_ACTION_SET_PLAYBACK_SPEED_NOTIFICATION, (void *)&enable, service_idx);
                break;
            case BLE_MCP_SET_PLAYING_ORDER_NOTIFICATION_CNF:
                result = ble_mcp_send_action(handle, BLE_MCP_ACTION_SET_PLAYING_ORDER_NOTIFICATION, (void *)&enable, service_idx);
                break;
            case BLE_MCP_SET_MEDIA_CONTROL_OPCODES_SUPPORTED_NOTIFICATION_CNF:
                result = ble_mcp_send_action(handle, BLE_MCP_ACTION_SET_MEDIA_CONTROL_OPCODES_SUPPORTED_NOTIFICATION, (void *)&enable, service_idx);
                break;
            case BLE_MCP_SET_MEDIA_CONTROL_POINT_NOTIFICATION_CNF:
                //result = ble_mcp_set_media_control_point_notification_req(handle, service_idx, enable);
                break;
            default:
                result = BT_STATUS_UNSUPPORTED;
                break;
        }
    }

    LOG_MSGID_I(BT_APP, "[MCP] AT+MCP result:%08x \r\n", 1, result);

    return result;
}

#define BT_APP_PTS_CSIS_STATE_FLAG   (1)

bt_status_t bt_le_audio_at_cmd_pts_csis_hdl(char *pChar)
{
    bt_status_t result = BT_STATUS_UNSUPPORTED;
    // bt_handle_t handle = app_lea_conn_mgr_get_handle(0);

    /* Map to PTS test case */
    /* SGGIT */
    /* CSIS/SR/SGGIT/SER/BV-01-C: do nothing */
    /* CSIS/SR/SGGIT/CHA/BV-01-C: do nothing */
    /* CSIS/SR/SGGIT/CHA/BV-02-C: do nothing */
    /* CSIS/SR/SGGIT/CHA/BV-03-C: do nothing */
    /* CSIS/SR/SGGIT/CHA/BV-04-C: do nothing */
    /* SP */
    /* CSIS/SR/SP/BV-01-C: do nothing */
    /* CSIS/SR/SP/BV-02-C: do nothing */
    if (NULL != strstr(pChar, "CSIS/SR/SP/BV-03-C")) {
        result = ble_csis_set_lock_state(0x01);
        ble_csis_send_lock_notify_req();
    } else if (NULL != strstr(pChar, "CSIS/SR/SP/BV-05-C")) {
        /* Run CSIS/SR/SGGIT/SER/BV-01-C first, to create bonding with PTS */
        ble_csis_send_lock_notify_req();
        result = BT_STATUS_SUCCESS;
    } else if (NULL != strstr(pChar, "CSIS/SR/SP/BV-08-C")) {
        /* Tap this case, before run this case*/
        result = ble_csis_config_encrypted_sirk(true);
    /* SPE */
    } else if (NULL != strstr(pChar, "CSIS/SR/SPE/BI-01-C")) {
        if (BT_PTS_STATE_IDLE == bt_app_pts_state) {
            bt_app_pts_state = BT_APP_PTS_CSIS_STATE_FLAG;
            result = ble_csis_set_lock_state(0x02);
        } else {
            bt_app_pts_state = BT_PTS_STATE_IDLE;
            ble_csis_send_lock_notify_req();
        }
    } else if (NULL != strstr(pChar, "CSIS/SR/SPE/BI-02-C")) {
        result = ble_csis_set_lock_state(0x02);
    /* CSIS/SR/SPE/BI-03-C: do nothing */
    /* Special Test Case */
    /* CSIP Test Case, need to trigger by CSIS sink project */
    } else if (NULL != strstr(pChar, "CSIP/SR/SP/BV-01-C")) {
        /* Tap this case to write SIRK and reset, before run this case */
        bt_key_t sirk = {0xb8, 0x03, 0xea, 0xc6, 0xaf, 0xbb, 0x65, 0xa2, 0x5a, 0x41, 0xf1, 0x53, 0x05, 0x68, 0x8e, 0x83};
        ble_csis_write_nvkey_sirk(&sirk);
    } else if (NULL != strstr(pChar, "CSIP/SR/SP/BV-03-C")) {
        /* Must Run CSIP/SR/SP/BV-01-C first, to change SIRK, then run this case */
        result = ble_csis_config_encrypted_sirk(true);
    }

    LOG_MSGID_I(BT_APP, "[CSIS] AT+CSIS result:%08x \r\n", 1, result);

    if (BT_STATUS_SUCCESS != result) {
        return result;
    }

    return result;
}

bool bt_comm_at_cmd_le_audio_pts_ccp_hdl(char *pChar)
{
    bt_status_t result = BT_STATUS_UNSUPPORTED;
    ble_ccp_event_t event_id;
    bool enable, ret = true;
    bt_handle_t handle = bt_sink_srv_cap_get_link_handle(0xFF);
    uint8_t service_idx = BLE_CCP_SERVICE_INDEX_GTBS;
    LOG_MSGID_I(BT_APP, "[CCP] bt_app_comm_at_cmd_ccp_hdl\r\n", 0);

    /* AT+PTSLEA=PTSMCP,<cmdType>,<cmdNum>
    * "R,xx" --> Read characteristic xx
    * "W,xx" --> Write characteristic xx
    * "N,xx" --> Write CCCD xx
    *
    * PTS at cmd format:
    * AT+PTSLEA=PTSCCP,CCP/CL/CGGIT/CHA/BV-32-C\0d\0a for PTS test case(CCP/CL/CGGIT/CHA/BV-32-C)
    */

    event_id = atoi(pChar);

    ble_ccp_write_call_control_point_param_t param;
    ble_tbs_call_control_point_t tbs_param;

    tbs_param.opcode = BLE_TBS_CALL_CONTROL_OPCODE_TYPE_ACCEPT;
    tbs_param.params.call_index = 1;
    param.length = BT_PTS_CCP_CP_DEFAULT_LEN;
    param.call_control_point = &tbs_param;

    LOG_MSGID_I(BT_APP, "[CCP] AT+CCP service[0x%x] handle[0x%04x] event_id[0x%02x]\r\n", 3, service_idx, handle, event_id);

    /* CGGIT */
    /* CCP/CL/CGGIT/SER/BV-01-C: do nothing */
    if (NULL != strstr(pChar, "CCP/CL/CGGIT/CHA/BV-01-C")){
        result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_BEARER_PROVIDER_NAME, NULL, 0);
    } else if (NULL != strstr(pChar, "CCP/CL/CGGIT/CHA/BV-02-C")) {
        result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_BEARER_UCI, NULL, 0);
    } else if (NULL != strstr(pChar, "CCP/CL/CGGIT/CHA/BV-03-C")) {
        result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_BEARER_TECHNOLOGY, NULL, 0);
    } else if (NULL != strstr(pChar, "CCP/CL/CGGIT/CHA/BV-04-C")) {
        result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_BEARER_URI_SCHEMES_SUPPORTED_LIST, NULL, 0);
    } else if (NULL != strstr(pChar, "CCP/CL/CGGIT/CHA/BV-05-C")) {
        result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_BEARER_SIGNAL_STRENGTH, NULL, 0);
    } else if (NULL != strstr(pChar, "CCP/CL/CGGIT/CHA/BV-06-C")) {
        if (BT_PTS_STATE_IDLE == bt_app_pts_state) {
            bt_app_pts_state = BT_PTS_STATE_WRITE_WITHOUT_RESPONSE;
            result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_BEARER_SIGNAL_STRENGTH_REPORTING_INTERVAL, NULL, 0);
        } else if (BT_PTS_STATE_WRITE_WITHOUT_RESPONSE == bt_app_pts_state) {
            /* write cmd without rsp */
            bt_app_pts_state = BT_PTS_STATE_WRITE;
            result = ble_ccp_write_bearer_signal_strength_reporting_interval_cmd(handle, 0, 1);
        } else {
            /* write cmd with rsp */
            bt_app_pts_state = BT_PTS_STATE_IDLE;
            result = ble_ccp_write_bearer_signal_strength_reporting_interval_req(handle, 0, 1);
        }
    } else if (NULL != strstr(pChar, "CCP/CL/CGGIT/CHA/BV-07-C")) {
        result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_BEARER_LIST_CURRENT_CALLS, NULL, 0);
    } else if (NULL != strstr(pChar, "CCP/CL/CGGIT/CHA/BV-08-C")) {
        result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_CONTENT_CONTROL_ID, NULL, 0);
    } else if (NULL != strstr(pChar, "CCP/CL/CGGIT/CHA/BV-09-C")) {
        result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_INCOMING_CALL_TARGET_BEARER_URI, NULL, 0);
    } else if (NULL != strstr(pChar, "CCP/CL/CGGIT/CHA/BV-10-C")) {
        result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_STATUS_FLAGS, NULL, 0);
    } else if (NULL != strstr(pChar, "CCP/CL/CGGIT/CHA/BV-11-C")) {
        result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_CALL_STATE, NULL, 0);
    /* CCP/CL/CGGIT/CHA/BV-12-C: do nothing */
    } else if (NULL != strstr(pChar, "CCP/CL/CGGIT/CHA/BV-13-C")) {
        result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_CALL_CONTROL_POINT_OPTIONAL_OPCODES, NULL, 0);
    /* CCP/CL/CGGIT/CHA/BV-14-C: do nothing */
    } else if (NULL != strstr(pChar, "CCP/CL/CGGIT/CHA/BV-15-C")) {
        result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_INCOMING_CALL, NULL, 0);
    } else if (NULL != strstr(pChar, "CCP/CL/CGGIT/CHA/BV-16-C")) {
        result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_CALL_FRIENDLY_NAME, NULL, 0);
    /* CCP/CL/CGGIT/SER/BV-02-C: do nothing */
    } else if (NULL != strstr(pChar, "CCP/CL/CGGIT/CHA/BV-17-C")){
        result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_BEARER_PROVIDER_NAME, NULL, service_idx);
    } else if (NULL != strstr(pChar, "CCP/CL/CGGIT/CHA/BV-18-C")) {
        result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_BEARER_UCI, NULL, service_idx);
    } else if (NULL != strstr(pChar, "CCP/CL/CGGIT/CHA/BV-19-C")) {
        result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_BEARER_TECHNOLOGY, NULL, service_idx);
    } else if (NULL != strstr(pChar, "CCP/CL/CGGIT/CHA/BV-20-C")) {
        result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_BEARER_URI_SCHEMES_SUPPORTED_LIST, NULL, service_idx);
    } else if (NULL != strstr(pChar, "CCP/CL/CGGIT/CHA/BV-21-C")) {
        result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_BEARER_SIGNAL_STRENGTH, NULL, service_idx);
    } else if (NULL != strstr(pChar, "CCP/CL/CGGIT/CHA/BV-22-C")) {
        if (BT_PTS_STATE_IDLE == bt_app_pts_state) {
            bt_app_pts_state = BT_PTS_STATE_WRITE_WITHOUT_RESPONSE;
            result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_BEARER_SIGNAL_STRENGTH_REPORTING_INTERVAL, NULL, service_idx);
        } else if (BT_PTS_STATE_WRITE_WITHOUT_RESPONSE == bt_app_pts_state) {
            /* write cmd without rsp */
            bt_app_pts_state = BT_PTS_STATE_WRITE;
            result = ble_ccp_write_bearer_signal_strength_reporting_interval_cmd(handle, service_idx, 1);
        } else {
            /* write cmd with rsp */
            bt_app_pts_state = BT_PTS_STATE_IDLE;
            result = ble_ccp_write_bearer_signal_strength_reporting_interval_req(handle, service_idx, 1);
        }
    } else if (NULL != strstr(pChar, "CCP/CL/CGGIT/CHA/BV-23-C")) {
        result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_BEARER_LIST_CURRENT_CALLS, NULL, service_idx);
    } else if (NULL != strstr(pChar, "CCP/CL/CGGIT/CHA/BV-24-C")) {
        result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_CONTENT_CONTROL_ID, NULL, service_idx);
    } else if (NULL != strstr(pChar, "CCP/CL/CGGIT/CHA/BV-25-C")) {
        result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_INCOMING_CALL_TARGET_BEARER_URI, NULL, service_idx);
    } else if (NULL != strstr(pChar, "CCP/CL/CGGIT/CHA/BV-26-C")) {
        result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_STATUS_FLAGS, NULL, service_idx);
    } else if (NULL != strstr(pChar, "CCP/CL/CGGIT/CHA/BV-27-C")) {
        result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_CALL_STATE, NULL, service_idx);
    } else if (NULL != strstr(pChar, "CCP/CL/CGGIT/CHA/BV-28-C")) { // DO NOTHING
        result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_CALL_STATE, NULL, service_idx);
    } else if (NULL != strstr(pChar, "CCP/CL/CGGIT/CHA/BV-29-C")) {
    /* CCP/CL/CGGIT/CHA/BV-30-C: do nothing */
        result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_CALL_CONTROL_POINT_OPTIONAL_OPCODES, NULL, service_idx);
    } else if (NULL != strstr(pChar, "CCP/CL/CGGIT/CHA/BV-31-C")) {
        result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_INCOMING_CALL, NULL, service_idx);
    } else if (NULL != strstr(pChar, "CCP/CL/CGGIT/CHA/BV-32-C")) {
        result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_CALL_FRIENDLY_NAME, NULL, service_idx);
    /* CP */
    } else if (NULL != strstr(pChar, "CCP/CL/CP/BV-01-C")) {
        tbs_param.opcode = BLE_TBS_CALL_CONTROL_OPCODE_TYPE_ACCEPT;
        result = ble_ccp_write_call_control_point_req(handle, 0, &param);
    } else if (NULL != strstr(pChar, "CCP/CL/CP/BV-02-C")) {
        tbs_param.opcode = BLE_TBS_CALL_CONTROL_OPCODE_TYPE_TERMINATE;
        result = ble_ccp_write_call_control_point_req(handle, 0, &param);
    } else if (NULL != strstr(pChar, "CCP/CL/CP/BV-03-C")) {
        tbs_param.opcode = BLE_TBS_CALL_CONTROL_OPCODE_TYPE_LOCAL_HOLD;
        result = ble_ccp_write_call_control_point_req(handle, 0, &param);
    } else if (NULL != strstr(pChar, "CCP/CL/CP/BV-04-C")) {
        tbs_param.opcode = BLE_TBS_CALL_CONTROL_OPCODE_TYPE_LOCAL_RETRIEVE;
        result = ble_ccp_write_call_control_point_req(handle, 0, &param);
    } else if (NULL != strstr(pChar, "CCP/CL/CP/BV-05-C")) {
        tbs_param.opcode = BLE_TBS_CALL_CONTROL_OPCODE_TYPE_LOCAL_RETRIEVE;
        result = ble_ccp_write_call_control_point_req(handle, 0, &param);
    } else if (NULL != strstr(pChar, "CCP/CL/CP/BV-06-C")) {
        ble_tbs_call_control_point_t *tbs_ori_param = pvPortMalloc(sizeof(ble_tbs_call_control_opcode_t) + BT_PTS_CCP_URI_LEN);
        if (tbs_ori_param != NULL) {
            tbs_ori_param->opcode = BLE_TBS_CALL_CONTROL_OPCODE_TYPE_ORIGINATE;
            memcpy(tbs_ori_param->params.originate_call.uri, bt_app_pts_uri, BT_PTS_CCP_URI_LEN);
            param.length = BT_PTS_CCP_OPCODE_LEN + BT_PTS_CCP_URI_LEN;
            param.call_control_point = tbs_ori_param;
            result = ble_ccp_write_call_control_point_req(handle, 0, &param);
            vPortFree(tbs_ori_param);
        }
    } else if (NULL != strstr(pChar, "CCP/CL/CP/BV-07-C")) {
        ble_tbs_call_control_point_t *tbs_join_param = pvPortMalloc(sizeof(ble_tbs_call_control_opcode_t) + BT_PTS_CCP_JOIN_CALL_NUM);
        if (tbs_join_param != NULL) {
            tbs_join_param->opcode = BLE_TBS_CALL_CONTROL_OPCODE_TYPE_JOIN;
            memcpy(tbs_join_param->params.join_call.call_index_list, bt_app_pts_join_call_id, BT_PTS_CCP_JOIN_CALL_NUM);
            param.length = BT_PTS_CCP_OPCODE_LEN + BT_PTS_CCP_JOIN_CALL_NUM;
            param.call_control_point = tbs_join_param;
            result = ble_ccp_write_call_control_point_req(handle, 0, &param);
            vPortFree(tbs_join_param);
        }
    } else if (NULL != strstr(pChar, "CCP/CL/CP/BV-08-C")) {
        tbs_param.opcode = BLE_TBS_CALL_CONTROL_OPCODE_TYPE_ACCEPT;
        result = ble_ccp_write_call_control_point_req(handle, service_idx, &param);
    } else if (NULL != strstr(pChar, "CCP/CL/CP/BV-09-C")) {
        tbs_param.opcode = BLE_TBS_CALL_CONTROL_OPCODE_TYPE_TERMINATE;
        result = ble_ccp_write_call_control_point_req(handle, service_idx, &param);
    } else if (NULL != strstr(pChar, "CCP/CL/CP/BV-10-C")) {
        tbs_param.opcode = BLE_TBS_CALL_CONTROL_OPCODE_TYPE_LOCAL_HOLD;
        result = ble_ccp_write_call_control_point_req(handle, service_idx, &param);
    } else if (NULL != strstr(pChar, "CCP/CL/CP/BV-11-C")) {
        tbs_param.opcode = BLE_TBS_CALL_CONTROL_OPCODE_TYPE_LOCAL_RETRIEVE;
        result = ble_ccp_write_call_control_point_req(handle, service_idx, &param);
    } else if (NULL != strstr(pChar, "CCP/CL/CP/BV-12-C")) {
        tbs_param.opcode = BLE_TBS_CALL_CONTROL_OPCODE_TYPE_LOCAL_RETRIEVE;
        result = ble_ccp_write_call_control_point_req(handle, service_idx, &param);
    } else if (NULL != strstr(pChar, "CCP/CL/CP/BV-13-C")) {
        ble_tbs_call_control_point_t *tbs_ori_param = pvPortMalloc(sizeof(ble_tbs_call_control_opcode_t) + BT_PTS_CCP_URI_LEN);
        if (tbs_ori_param != NULL) {
            tbs_ori_param->opcode = BLE_TBS_CALL_CONTROL_OPCODE_TYPE_ORIGINATE;
            memcpy(tbs_ori_param->params.originate_call.uri, bt_app_pts_uri, BT_PTS_CCP_URI_LEN);
            param.length = BT_PTS_CCP_OPCODE_LEN + BT_PTS_CCP_URI_LEN;
            param.call_control_point = tbs_ori_param;
            result = ble_ccp_write_call_control_point_req(handle, service_idx, &param);
            vPortFree(tbs_ori_param);
        }
    } else if (NULL != strstr(pChar, "CCP/CL/CP/BV-14-C")) {
        ble_tbs_call_control_point_t *tbs_join_param = pvPortMalloc(sizeof(ble_tbs_call_control_point_t) + BT_PTS_CCP_JOIN_CALL_NUM);
        if (tbs_join_param != NULL) {
            tbs_join_param->opcode = BLE_TBS_CALL_CONTROL_OPCODE_TYPE_JOIN;
            memcpy(tbs_join_param->params.join_call.call_index_list, bt_app_pts_join_call_id, BT_PTS_CCP_JOIN_CALL_NUM);
            param.length = BT_PTS_CCP_OPCODE_LEN + BT_PTS_CCP_JOIN_CALL_NUM;
            param.call_control_point = tbs_join_param;
            result = ble_ccp_write_call_control_point_req(handle, service_idx, &param);
            vPortFree(tbs_join_param);
        }
    /* SPE */
    } else if (NULL != strstr(pChar, "CCP/CL/SPE/BI-01-C")) {
        if (BT_PTS_STATE_IDLE == bt_app_pts_state) {
            bt_app_pts_state = BT_PTS_STATE_READ;
            tbs_param.opcode = BLE_TBS_CALL_CONTROL_OPCODE_TYPE_ACCEPT;
            result = ble_ccp_write_call_control_point_req(handle, 0, &param);
        } else if (BT_PTS_STATE_READ == bt_app_pts_state) {
            bt_app_pts_state = BT_PTS_STATE_IDLE;
            result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_CALL_FRIENDLY_NAME, NULL, 0);
        }
    } else if (NULL != strstr(pChar, "CCP/CL/SPE/BI-02-C")) {
        if (BT_PTS_STATE_IDLE == bt_app_pts_state) {
            bt_app_pts_state = BT_PTS_STATE_READ;
            tbs_param.opcode = BLE_TBS_CALL_CONTROL_OPCODE_TYPE_ACCEPT;
            result = ble_ccp_write_call_control_point_req(handle, service_idx, &param);
        } else if (BT_PTS_STATE_READ == bt_app_pts_state) {
            bt_app_pts_state = BT_PTS_STATE_IDLE;
            result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_CALL_FRIENDLY_NAME, NULL, service_idx);
        }
    } else if (NULL != strstr(pChar, "CCP/CL/SPE/BI-03-C")) {
        if (BT_PTS_STATE_IDLE == bt_app_pts_state) {
            bt_app_pts_state = BT_PTS_STATE_READ;
            ble_tbs_call_control_point_t *tbs_join_param = pvPortMalloc(sizeof(ble_tbs_call_control_point_t) * BT_PTS_CCP_JOIN_CALL_NUM);
            if (tbs_join_param != NULL) {
                tbs_join_param->opcode = BLE_TBS_CALL_CONTROL_OPCODE_TYPE_JOIN;
                memcpy(tbs_join_param->params.join_call.call_index_list, bt_app_pts_join_call_id, BT_PTS_CCP_JOIN_CALL_NUM);
                param.length = BT_PTS_CCP_OPCODE_LEN + BT_PTS_CCP_JOIN_CALL_NUM;
                param.call_control_point = tbs_join_param;
                result = ble_ccp_write_call_control_point_req(handle, 0, &param);
                vPortFree(tbs_join_param);
            }
        } else if (BT_PTS_STATE_READ == bt_app_pts_state) {
            bt_app_pts_state = BT_PTS_STATE_IDLE;
            result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_CALL_FRIENDLY_NAME, NULL, 0);
        }
    } else if (NULL != strstr(pChar, "CCP/CL/SPE/BI-04-C")) {
        if (BT_PTS_STATE_IDLE == bt_app_pts_state) {
            bt_app_pts_state = BT_PTS_STATE_READ;
            ble_tbs_call_control_point_t *tbs_join_param = pvPortMalloc(sizeof(ble_tbs_call_control_point_t) * BT_PTS_CCP_JOIN_CALL_NUM);
            if (tbs_join_param != NULL) {
                tbs_join_param->opcode = BLE_TBS_CALL_CONTROL_OPCODE_TYPE_JOIN;
                memcpy(tbs_join_param->params.join_call.call_index_list, bt_app_pts_join_call_id, BT_PTS_CCP_JOIN_CALL_NUM);
                param.length = BT_PTS_CCP_OPCODE_LEN + BT_PTS_CCP_JOIN_CALL_NUM;
                param.call_control_point = tbs_join_param;
                result = ble_ccp_write_call_control_point_req(handle, service_idx, &param);
                vPortFree(tbs_join_param);
            }
        } else if (BT_PTS_STATE_READ == bt_app_pts_state) {
            bt_app_pts_state = BT_PTS_STATE_IDLE;
            result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_CALL_FRIENDLY_NAME, NULL, service_idx);
        }
    } else if (NULL != strstr(pChar, "CCP/CL/SPE/BI-05-C")) {
        if (BT_PTS_STATE_IDLE == bt_app_pts_state) {
            bt_app_pts_state = BT_PTS_STATE_READ;
            ble_tbs_call_control_point_t *tbs_ori_param = pvPortMalloc(sizeof(ble_tbs_call_control_point_t) + BT_PTS_CCP_URI_LEN);
            if (tbs_ori_param != NULL) {
                tbs_ori_param->opcode = BLE_TBS_CALL_CONTROL_OPCODE_TYPE_ORIGINATE;
                memcpy(tbs_ori_param->params.originate_call.uri, bt_app_pts_uri, BT_PTS_CCP_URI_LEN);
                param.length = BT_PTS_CCP_OPCODE_LEN + BT_PTS_CCP_URI_LEN;
                param.call_control_point = tbs_ori_param;
                result = ble_ccp_write_call_control_point_req(handle, 0, &param);
                vPortFree(tbs_ori_param);
            }
        } else if (BT_PTS_STATE_READ == bt_app_pts_state) {
            bt_app_pts_state = BT_PTS_STATE_IDLE;
            result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_CALL_FRIENDLY_NAME, NULL, 0);
        }
    } else if (NULL != strstr(pChar, "CCP/CL/SPE/BI-06-C")) {
        if (BT_PTS_STATE_IDLE == bt_app_pts_state) {
            bt_app_pts_state = BT_PTS_STATE_READ;
            ble_tbs_call_control_point_t *tbs_ori_param = pvPortMalloc(sizeof(ble_tbs_call_control_point_t) + BT_PTS_CCP_URI_LEN);
            if (tbs_ori_param != NULL) {
                tbs_ori_param->opcode = BLE_TBS_CALL_CONTROL_OPCODE_TYPE_ORIGINATE;
                memcpy(tbs_ori_param->params.originate_call.uri, bt_app_pts_uri, BT_PTS_CCP_URI_LEN);
                param.length = BT_PTS_CCP_OPCODE_LEN + BT_PTS_CCP_URI_LEN;
                param.call_control_point = tbs_ori_param;
                result = ble_ccp_write_call_control_point_req(handle, service_idx, &param);
                vPortFree(tbs_ori_param);
            }
        } else if (BT_PTS_STATE_READ == bt_app_pts_state) {
            bt_app_pts_state = BT_PTS_STATE_IDLE;
            result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_CALL_FRIENDLY_NAME, NULL, service_idx);
        }
    } else {
        if (event_id >= BLE_CCP_SET_BEARER_PROVIDER_NAME_NOTIFICATION_CNF && event_id <= BLE_CCP_SET_CALL_FRIENDLY_NAME_NOTIFICATION_CNF) {
            pChar++;
            pChar = strchr(pChar, ',');
            if (pChar == NULL){
                LOG_MSGID_I(BT_APP, "[MCP] AT+CCP cmd String error!!\r\n", 0);
            } else {
                if (strstr(pChar, "E") != NULL) {
                    enable = TRUE;
                } else {
                    enable = FALSE;
                }
            }
        }
        switch (event_id) {
            /* READ/WRITE */
            case BLE_CCP_READ_BEARER_PROVIDER_NAME_CNF:
                result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_BEARER_PROVIDER_NAME, NULL, service_idx);
                break;
            case BLE_CCP_READ_BEARER_URI_SCHEMES_SUPPORTED_LIST_CNF:
                result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_BEARER_URI_SCHEMES_SUPPORTED_LIST, NULL, service_idx);
                break;
            case BLE_CCP_READ_BEARER_LIST_CURRENT_CALLS_CNF:
                result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_BEARER_LIST_CURRENT_CALLS, NULL, service_idx);
                break;
            case BLE_CCP_READ_INCOMING_CALL_TARGET_BEARER_URI_CNF:
                result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_INCOMING_CALL_TARGET_BEARER_URI, NULL, service_idx);
                break;
            case BLE_CCP_READ_CALL_STATE_CNF:
                result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_CALL_STATE, NULL, service_idx);
                break;
            case BLE_CCP_READ_INCOMING_CALL_CNF:
                result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_INCOMING_CALL, NULL, service_idx);
                break;
            case BLE_CCP_READ_CALL_FRIENDLY_NAME_CNF:
                result = ble_ccp_send_action(handle, BLE_CCP_ACTION_READ_CALL_FRIENDLY_NAME, NULL, service_idx);
                break;

            /* NOTIFY */
            case BLE_CCP_SET_BEARER_PROVIDER_NAME_NOTIFICATION_CNF:
                result = ble_ccp_send_action(handle, BLE_CCP_ACTION_SET_BEARER_PROVIDER_NAME_NOTIFICATION, &enable, service_idx);
                break;
            case BLE_CCP_SET_BEARER_URI_SCHEMES_SUPPORTED_LIST_NOTIFICATION_CNF:
                result = ble_ccp_send_action(handle, BLE_CCP_ACTION_SET_BEARER_URI_SCHEMES_SUPPORTED_LIST_NOTIFICATION, &enable, service_idx);
                break;
            case BLE_CCP_SET_BEARER_LIST_CURRENT_CALLS_NOTIFICATION_CNF:
                result = ble_ccp_send_action(handle, BLE_CCP_ACTION_SET_BEARER_LIST_CURRENT_CALLS_NOTIFICATION, &enable, service_idx);
                break;
            case BLE_CCP_SET_INCOMING_CALL_TARGET_BEARER_URI_NOTIFICATION_CNF:
                result = ble_ccp_send_action(handle, BLE_CCP_ACTION_SET_INCOMING_CALL_TARGET_BEARER_URI_NOTIFICATION, &enable, service_idx);
                break;
            case BLE_CCP_SET_CALL_STATE_NOTIFICATION_CNF:
                result = ble_ccp_send_action(handle, BLE_CCP_ACTION_SET_CALL_STATE_NOTIFICATION, &enable, service_idx);
                break;
            case BLE_CCP_SET_INCOMING_CALL_NOTIFICATION_CNF:
                result = ble_ccp_send_action(handle, BLE_CCP_ACTION_SET_INCOMING_CALL_NOTIFICATION, &enable, service_idx);
                break;
            case BLE_CCP_SET_CALL_FRIENDLY_NAME_NOTIFICATION_CNF:
                result = ble_ccp_send_action(handle, BLE_CCP_ACTION_SET_CALL_FRIENDLY_NAME_NOTIFICATION, &enable, service_idx);
                break;
            default:
                ret = false;
                break;
        }
    }

    LOG_MSGID_I(BT_APP, "[CCP] AT+CCP result:%08x \r\n", 1, result);

    return ret;
}

bt_status_t bt_le_audio_at_cmd_pts_cap_hdl(char *pChar)
{
    bt_status_t result = BT_STATUS_FAIL;
    char *p = pChar, *ret = 0;
    uint16_t sink_val = 0, src_val = 0;

    while (*p && *p == ' ') p++;

    if (!*p) return result;

    if (0 == memcmp(pChar, "CTX", 3))
    {
        /* Map to PTS test case */
        /*
        CAP/ACC/ERR/BI-01-C
        CAP/ACC/ERR/BI-02-C
        CAP/ACC/ERR/BI-03-C
        CAP/ACC/ERR/BI-04-C
        */
        ret = strchr(p + 3, ',');

        if (!ret) return result;

        p = ret + 1;
        ret = 0;

        while (*p && *p == ' ') p++;

        if (!*p) return result;

        sink_val = (uint16_t)strtol(p, &ret, 16);

        if (!ret) return result;

        ret = strchr(ret, ',');

        if (!ret) return result;

        p = ret + 1;
        ret = 0;

        while (*p && *p == ' ') p++;

        if (!*p) return result;

        src_val = (uint16_t)strtol(p, &ret, 16);

        if (!ret) return result;

        ble_pacs_set_supported_audio_contexts(sink_val, src_val);
        ble_pacs_set_available_audio_contexts(sink_val, src_val);
        result = BT_STATUS_SUCCESS;
    }

    LOG_MSGID_I(BT_APP, "[CAP] AT+PTSCAP result:%08x sink_val = %x src_val = %x\r\n", 3, result, sink_val, src_val);

    return result;
}

static bt_status_t bt_le_audio_at_cmd_pts_mics_hdl(char *pChar)
{
    bt_status_t result = BT_STATUS_UNSUPPORTED;
    bt_handle_t handle = bt_sink_srv_cap_get_link_handle(0xFF);

    /* AT+PTSLEA=PTSMICS,<ACTION> */
    /* <ACTION>: SET_MUTE, SET_UNMUTE */

    if (0 == memcmp(pChar, "SET_MUTE", 8)) {
        result = ble_mics_set_mute(handle, BLE_MICS_MUTE_STATE_MUTE);

    } else if (0 == memcmp(pChar, "SET_UNMUTE", 10)) {
        result = ble_mics_set_mute(handle, BLE_MICS_MUTE_STATE_UNMUTE);

    } else if (0 == memcmp(pChar, "SET_DISABLE", 11)) {
        result = ble_mics_set_mute(handle, BLE_MICS_MUTE_STATE_DISABLE);

    } else if (0 == memcmp(pChar, "AICS_SET_AUTO", 13)) {
        result = ble_aics_set_audio_input_gain_mode(handle, 0, BLE_AICS_GAIN_MODE_AUTOMATIC);

    } else if (0 == memcmp(pChar, "AICS_SET_ONLY_AUTO", 18)) {
        result = ble_aics_set_audio_input_gain_mode(handle, 0, BLE_AICS_GAIN_MODE_AUTOMATIC_ONLY);

    } else if (0 == memcmp(pChar, "AICS_SET_MANUAL", 15)) {
        result = ble_aics_set_audio_input_gain_mode(handle, 0, BLE_AICS_GAIN_MODE_MANUAL);

    } else if (0 == memcmp(pChar, "AICS_SET_ONLY_MANUAL", 20)) {
        result = ble_aics_set_audio_input_gain_mode(handle, 0, BLE_AICS_GAIN_MODE_MANUAL_ONLY);

    }

    LOG_MSGID_I(BT_APP, "[MICS] AT+MICS result:%08x \r\n", 1, result);

    return result;
}

static bt_status_t bt_le_audio_at_cmd_pts_pacs_hdl(char *pChar)
{
    bt_status_t result = BT_STATUS_UNSUPPORTED;
    bt_handle_t handle = bt_sink_srv_cap_get_link_handle(0xFF);

    /* AT+PTSLEA=PTSPACS,<ACTION> */
    /* <ACTION>: UPDATE_AVAILABLE_AUDIO_CONTEXTS */

    if (0 == memcmp(pChar, "UPDATE_AVAILABLE_AUDIO_CONTEXTS", 31)) {
        bt_le_audio_content_type_t sink_type = AUDIO_CONTENT_TYPE_NOTIFICATION;
        bt_le_audio_content_type_t source_type = AUDIO_CONTENT_TYPE_NOTIFICATION;

        if (NULL != (pChar = strchr(pChar, ','))) {
            unsigned int value[4] = {0};
            pChar ++;
            /* AT+PTSLEA=PTSPACS,UPDATE_AVAILABLE_AUDIO_CONTEXTS,<sink_type>,<source_type> */
            /* <sink_type> & <source_type>: 0x1234 */
            /* ex: AT+PTSLEA=PTSPACS,UPDATE_AVAILABLE_AUDIO_CONTEXTS,1234,5678 */
            if (sscanf(pChar, "%2x%2x,%2x%2x", &value[0], &value[1], &value[2], &value[3]) > 0) {
                sink_type = ((uint16_t)value[0] << 8) | ((uint16_t)value[1]);
                source_type = ((uint16_t)value[2] << 8) | ((uint16_t)value[3]);
            }
        }

        ble_pacs_set_available_audio_contexts(sink_type, source_type);
        if (true == ble_pacs_send_available_audio_contexts_notify(handle)) {
            result = BT_STATUS_SUCCESS;
        }

    } else if (0 == memcmp(pChar, "SET_AVAILABLE_AUDIO_CONTEXTS", 28)) {
        result = ble_pacs_set_available_audio_contexts(AUDIO_CONTENT_TYPE_NOTIFICATION, AUDIO_CONTENT_TYPE_NOTIFICATION);
    }

    return result;
}

static bt_status_t bt_le_audio_at_cmd_pts_ascs_hdl(char *pChar)
{
    bt_status_t result = BT_STATUS_UNSUPPORTED;
    bt_handle_t handle = bt_sink_srv_cap_get_link_handle(0);
    uint8_t ase_id = 0xFF;
    unsigned int val = 0;

    /* AT+PTSLEA=PTSASCS,<ACTION1>,<ACTION2> */
    /* <ACTION1>: RELEASE */
    if (0 == memcmp(pChar, "RELEASE", 7)) {
        /* ASCS release ASE ID*/
        /* AT+PTSLEA=PTSASCS,RELEASE,<ase_id> */
        pChar = strchr(pChar, ',');
        pChar++;

        if (sscanf(pChar, "%2x", &val) > 0) {
            ase_id = (uint8_t)val;
            if (bt_sink_srv_cap_stream_release_autonomously(handle, ase_id, FALSE, 0)) {
                result = BT_STATUS_SUCCESS;
            }
        }
    }

    return result;
}

atci_status_t bt_le_audio_atci_cmd_handler(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t *response = (atci_response_t *)pvPortMalloc(sizeof(atci_response_t));
    bt_status_t result = BT_STATUS_FAIL;
    char *pChar = NULL;

    if (NULL == response) {
        LE_AUDIO_MSGLOG_I("[LEA][ATCI] malloc heap memory fail.", 0);
        return ATCI_STATUS_ERROR;
    }
    memset(response, 0, sizeof(atci_response_t));

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: /* rec: AT+PTSLEA=<ACTION, PARAMS> */
            pChar = parse_cmd->string_ptr + parse_cmd->name_len + 1;
            if (0 == memcmp(pChar, "PTSMCP", 6)) {
                pChar = strchr(pChar, ',');
                pChar++;
                result = bt_le_audio_at_cmd_pts_mcp_hdl(pChar);

            } else if (0 == memcmp(pChar, "PTSCSIS", 7)) {
                pChar = strchr(pChar, ',');
                pChar++;
                result = bt_le_audio_at_cmd_pts_csis_hdl(pChar);

            } else if (0 == memcmp(pChar, "PTSCCP", 6)) {
                pChar = strchr(pChar, ',');
                pChar++;
                if (bt_comm_at_cmd_le_audio_pts_ccp_hdl(pChar)) {
                    result = BT_STATUS_SUCCESS;
                }

            } else if (0 == memcmp(pChar, "PTSCAP", 6)) {
                pChar = strchr(pChar, ',');
                if (pChar)
                {
                    pChar++;
                    result = bt_le_audio_at_cmd_pts_cap_hdl(pChar);
                }

            } else if (0 == memcmp(pChar, "PTSMICS", 7)) {
                pChar = strchr(pChar, ',');
                pChar++;
                result = bt_le_audio_at_cmd_pts_mics_hdl(pChar);

            } else if (0 == memcmp(pChar, "PTSPACS", 7)) {
                pChar = strchr(pChar, ',');
                pChar++;
                result = bt_le_audio_at_cmd_pts_pacs_hdl(pChar);

            } else if (0 == memcmp(pChar, "PTSASCS", 7)) {
                pChar = strchr(pChar, ',');
                pChar++;
                result = bt_le_audio_at_cmd_pts_ascs_hdl(pChar);

            }

            if (BT_STATUS_SUCCESS == result) {
                response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            } else {
                response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            }

            response->response_len = strlen((char *)response->response_buf);
            atci_send_response(response);
            break;
        default:
            break;
    }

    vPortFree(response);
    return ATCI_STATUS_OK;
}

void bt_le_audio_atci_init(void)
{
    atci_status_t ret;

    ret = atci_register_handler(bt_le_audio_atci_cmd, sizeof(bt_le_audio_atci_cmd) / sizeof(atci_cmd_hdlr_item_t));

    if (ret != ATCI_STATUS_OK) {
        LE_AUDIO_MSGLOG_I("[LEA][ATCI] Register fail!", 0);
    }
}
