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


#include "bt_sink_srv_le.h"
#include "bt_sink_srv_le_cap.h"
#include "bt_sink_srv_le_cap_stream.h"
#include "bt_sink_srv_music.h"
#include "bt_le_audio_sink.h"
#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
#include "bt_sink_srv_state_manager.h"
#endif
#ifdef AIR_BT_CODEC_BLE_ENABLED
#include "bt_gap_le_audio.h"
#include "atci.h"
#include "bt_sink_srv_state_notify.h"
#include "bt_utils.h"
#include "bt_device_manager_le.h"

extern bt_aws_mce_role_t bt_device_manager_aws_local_info_get_role(void);
extern bt_device_manager_le_bonded_info_t *bt_device_manager_le_get_bonding_info_by_addr_ext(bt_bd_addr_t *remote_addr);

static void le_sink_srv_event_callback(uint16_t event_id, void *p_msg);
bt_sink_srv_state_t g_le_sink_srv_state;

#ifdef AIR_LE_AUDIO_CIS_ENABLE



static void le_sink_srv_mcp_state_change_notify(bt_handle_t handle, ble_mcs_media_state_t state);
static void le_sink_srv_state_change_notify(bt_sink_srv_state_t previous, bt_sink_srv_state_t now);
static bt_sink_srv_state_t le_sink_srv_state_transfer_to_bt_state(uint16_t module, bt_sink_srv_cap_state le_state);
le_sink_srv_context_t g_le_sink_srv_cntx[CAP_UNICAST_DEVICE_NUM];
bt_le_sink_srv_music_active_handle g_music_active_handle;
bt_le_sink_srv_call_active_handle g_call_active_handle;
static uint8_t g_le_sink_srv_sync_pa_retry_count = 0;
#ifdef AIR_ADAPTIVE_EQ_ENABLE
extern int32_t bt_sink_srv_stop_am_notify(audio_src_srv_pseudo_device_t device_type);
#endif

void bt_sink_srv_le_media_state_change_callback_default(uint16_t event_id, bt_handle_t handle, bool is_resume)
{
}

le_sink_srv_context_t *le_sink_srv_get_context(bt_handle_t handle)
{
    uint8_t index = bt_sink_srv_cap_get_link_index(handle);
    bt_sink_srv_report_id("[Sink]debug 2, index:%d", 1, index);
    if (index < CAP_UNICAST_DEVICE_NUM) {
        return &g_le_sink_srv_cntx[index];
    }
    return NULL;
}

const bt_sink_srv_action_callback_table_t le_sink_srv_action_callback_table[] = {
    {
        SINK_MODULE_MASK_COMMON | SINK_MODULE_MASK_HFP | SINK_MODULE_MASK_HSP,
        bt_sink_srv_le_call_action_handler
    },
    {
        SINK_MODULE_MASK_COMMON | SINK_MODULE_MASK_A2DP | SINK_MODULE_MASK_AVRCP,
        bt_sink_srv_le_music_action_handler
    },
#if 0
#ifdef MTK_AWS_MCE_ENABLE
    {
        SINK_MODULE_MASK_COMMON | SINK_MODULE_MASK_A2DP | SINK_MODULE_MASK_AVRCP,
        bt_sink_srv_music_aws_a2dp_action_handler
    },
#endif/*MTK_AWS_MCE_ENABLE*/
    {
        SINK_MODULE_MASK_COMMON | SINK_MODULE_MASK_AVRCP,
        bt_sink_srv_music_avrcp_action_handler
    },
    {
        SINK_MODULE_MASK_COMMON | SINK_MODULE_MASK_PBAPC,
        bt_sink_srv_pbapc_action_handler
    },
#ifdef MTK_AWS_MCE_ENABLE
#ifndef MTK_BT_CM_SUPPORT
    {
        SINK_MODULE_MASK_COMMON | SINK_MODULE_MASK_AWS_MCE,
        bt_sink_srv_aws_mce_action_handler
    },
#endif
#endif/*MTK_AWS_MCE_ENABLE*/
#endif
};
//typedef

static void le_sink_srv_update_active_handle(uint16_t event_id, void *p_msg)
{
    switch (event_id) {
        case BT_LE_AUDIO_SINK_EVENT_DISCONNECTED: {

            /*If active handle is equal to disconnected handle, switch to another link*/
            bt_le_audio_sink_event_disconnected_t *noti = (bt_le_audio_sink_event_disconnected_t *)p_msg;
            bt_handle_t check_handle = bt_sink_srv_cap_get_another_connected_link_handle(noti->handle);
            bt_sink_srv_report_id("[Sink][SRV] update active handle 2", 0);
            g_music_active_handle.handle = (g_music_active_handle.handle == noti->handle ? check_handle : g_music_active_handle.handle);
            break;
        }
        case BT_LE_AUDIO_SINK_EVENT_MEDIA_SERVICE_READY: {
            bt_le_audio_sink_event_service_ready_t *noti = (bt_le_audio_sink_event_service_ready_t *)p_msg;
            bt_handle_t check_handle = bt_sink_srv_cap_get_another_connected_link_handle(noti->handle);
            if (check_handle == BT_HANDLE_INVALID ||
                (check_handle != BT_HANDLE_INVALID && check_handle != bt_sink_srv_cap_get_ble_link_by_streaming_mode(bt_sink_srv_cap_am_get_current_mode()))) {
                //check another link is playing or not
                g_music_active_handle.handle = noti->handle;
            }
            break;
        }
        case BT_SINK_SRV_CAP_EVENT_ASE_STATE: {
            bt_sink_srv_cap_event_ase_state_t *noti = (bt_sink_srv_cap_event_ase_state_t *)p_msg;
            bt_sink_srv_report_id("[Sink][SRV] update active handle 6", 0);
            if (BT_SINK_SRV_CAP_STATE_ASE_STREAMING_MUSIC == noti->current_state ||
                (BT_SINK_SRV_CAP_STATE_ASE_STREAMING_MUSIC == noti->pre_state && BT_SINK_SRV_CAP_STATE_CONNECTED == noti->current_state)) {
                g_music_active_handle.handle = noti->connect_handle;
            }
            break;
        }

        case BT_LE_AUDIO_SINK_EVENT_MEDIA_STATE: {
            bt_le_audio_sink_event_media_state_t *noti = (bt_le_audio_sink_event_media_state_t*)p_msg;
            g_music_active_handle.handle = noti->handle;
            break;
        }

        default:
            break;
    }
    bt_sink_srv_report_id("[Sink] update active handle music:%d, accept:%d, reject:%d, dial:%d", 1, g_music_active_handle.handle);
}

static bt_sink_srv_state_t le_sink_srv_state_transfer_to_bt_state(uint16_t module, bt_sink_srv_cap_state le_state)
{
    bt_sink_srv_state_t state = BT_SINK_SRV_STATE_NONE;

    if (module == BT_SINK_SRV_CAP_EVENT_ASE_STATE) {
        switch (le_state) {
            case BT_SINK_SRV_CAP_STATE_IDLE: {
                state = BT_SINK_SRV_STATE_POWER_ON;
                break;
            }
            case BT_SINK_SRV_CAP_STATE_CONNECTED: {
                state = BT_SINK_SRV_STATE_CONNECTED;
                break;
            }
            case BT_SINK_SRV_CAP_STATE_ASE_STREAMING_MUSIC: {
                state = BT_SINK_SRV_STATE_STREAMING;
                break;
            }
            case BT_SINK_SRV_CAP_STATE_ASE_STREAMING_CALL: {
                state = BT_SINK_SRV_STATE_ACTIVE;
                break;
            }
            default: {
                break;
            }
        }
    } else if (module == BT_LE_AUDIO_SINK_EVENT_CALL_STATE) {
        state = bt_sink_srv_le_remap_call_state(le_state, BLE_TBS_STATE_INVALID);
    } else if (module == BT_LE_AUDIO_SINK_EVENT_MEDIA_STATE) {
        switch (le_state) {
            case BLE_MCS_MEDIA_STATE_PLAYING: {
                state = BT_SINK_SRV_STATE_STREAMING;
                break;
            }
            case BLE_MCS_MEDIA_STATE_PAUSED:
            case BLE_MCS_MEDIA_STATE_STOPED: {
                state = BT_SINK_SRV_STATE_CONNECTED;
                break;
            }
            default: {
                break;
            }
        }
    }
    return state;
}

bt_status_t le_sink_srv_send_action(uint32_t action, void *params)
{
    bt_status_t result = BT_STATUS_FAIL;
    uint32_t index;
    uint32_t action_module = (action & 0xF8F00000);
    bt_sink_srv_report_id("[Sink]le_sink_srv_send_action, action: 0x%x", 1, action);

    if (BT_SINK_SRV_ACTION_PROFILE_INIT == action) {
        // return bt_sink_srv_cap_init(le_sink_srv_event_callback, 1);
    }

    if (BT_MODULE_CUSTOM_SINK == action_module) {
        bt_sink_module_mask_t module_mask = SINK_MODULE_MASK_OFFSET(action);
        bt_sink_srv_report_id("[Sink]le_sink_srv_send_action, module mask: 0x%x", 1, module_mask);
        for (index = 0; index < sizeof(le_sink_srv_action_callback_table) / sizeof(bt_sink_srv_action_callback_table_t); index++) {
            if ((le_sink_srv_action_callback_table[index].module & module_mask)
                && le_sink_srv_action_callback_table[index].callback) {
                result = le_sink_srv_action_callback_table[index].callback(action, params);
                bt_sink_srv_report_id("[Sink]le_sink_srv_send_action, result: 0x%x", 1, result);
                if (result == BT_STATUS_SUCCESS) {
                    return result;
                }
            }
        }
    }
    return result;
}

static void le_sink_srv_profile_status_notify(bt_sink_srv_profile_type_t profile_type, bt_handle_t handle,
                                              bt_sink_srv_profile_connection_state_t state, bt_status_t status)
{
    bt_addr_t dev_addr = bt_sink_srv_cap_get_peer_bdaddr(bt_sink_srv_cap_get_link_index(handle));
    le_sink_srv_context_t *le_sink_cntx = (le_sink_srv_context_t *)le_sink_srv_get_context(handle);
    //le_sink_srv_music_device_t *dev = le_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_ADDR_A2DP, &dev_addr);
    bt_sink_srv_report_id("[Sink]srv_profile_status_notify, profile_type: 0x%x, state: %x", 2, profile_type, state);
    if (le_sink_cntx == NULL) {
        return;
    }
    switch (state) {
        case BT_SINK_SRV_PROFILE_CONNECTION_STATE_CONNECTED: {
            //if (le_sink_cntx->state == BT_BLE_LINK_CONNECTED) {
            // bt_sink_srv_report_id("[Sink]status_notify, wrong state", 0);
            //bt_utils_assert(0);
            //} else {


            le_sink_cntx->dev_addr = &dev_addr;

            if (profile_type == BT_SINK_SRV_PROFILE_A2DP_SINK) {
                le_sink_cntx->music_state = BT_SINK_SRV_STATE_CONNECTED;
            }
            //}
            bt_le_link_state_t old_state = le_sink_cntx->state;
            le_sink_cntx->state = BT_BLE_LINK_CONNECTED;
            bt_le_service_mask_t pre_connected_service = le_sink_cntx->conn_mask;
            bt_sink_srv_report_id("[Sink]srv_profile_status_notify, profile_type: 0x%x, conn_mask: %x", 2, profile_type, le_sink_cntx->conn_mask);
            le_sink_cntx->conn_mask |= profile_type;

            bt_le_sink_srv_event_remote_info_update_t update_ind;
            memcpy(&(update_ind.address), &(dev_addr), sizeof(bt_addr_t));
            update_ind.reason = BT_STATUS_SUCCESS;
            update_ind.pre_state = old_state;
            update_ind.state = BT_BLE_LINK_CONNECTED;
            update_ind.pre_connected_service = pre_connected_service;
            update_ind.connected_service = le_sink_cntx->conn_mask;
            bt_sink_srv_event_callback(LE_SINK_SRV_EVENT_REMOTE_INFO_UPDATE, &update_ind, sizeof(bt_le_sink_srv_event_remote_info_update_t));
            break;
        }
        case BT_SINK_SRV_PROFILE_CONNECTION_STATE_DISCONNECTED: {
            bt_le_link_state_t old_state = le_sink_cntx->state;
            bt_sink_srv_report_id("[Sink]old_state:%x", 1, old_state);
            le_sink_cntx->state = BT_BLE_LINK_DISCONNECTED;
            bt_le_service_mask_t pre_connected_service = le_sink_cntx->conn_mask;
            le_sink_cntx->conn_mask &= (~profile_type);

            bt_le_sink_srv_event_remote_info_update_t update_ind;
            memcpy(&(update_ind.address), &(dev_addr), sizeof(bt_addr_t));
            update_ind.reason = BT_STATUS_SUCCESS;
            update_ind.pre_state = old_state;
            update_ind.state = BT_BLE_LINK_DISCONNECTED;
            update_ind.pre_connected_service = pre_connected_service;
            update_ind.connected_service = le_sink_cntx->conn_mask;
            bt_sink_srv_event_callback(LE_SINK_SRV_EVENT_REMOTE_INFO_UPDATE, &update_ind, sizeof(bt_le_sink_srv_event_remote_info_update_t));
            break;
        }
        default: {
            break;
        }
    }
}

static void le_sink_srv_mcp_state_change_notify(bt_handle_t handle, ble_mcs_media_state_t state)
{
    bt_avrcp_status_t avrcp_statre = BT_AVRCP_STATUS_PLAY_ERROR;
    bt_addr_t dev_addr;// = bt_sink_srv_cap_get_peer_bdaddr(0xFF);
    bt_sink_srv_memset(&dev_addr, 0, sizeof(bt_addr_t));
    bt_gap_le_srv_conn_info_t *conn_info = bt_gap_le_srv_get_conn_info(handle);
    if (conn_info != NULL) {
        bt_sink_srv_memcpy(&dev_addr, &conn_info->peer_addr, sizeof(bt_addr_t));
    }

    switch (state) {
        case BLE_MCS_MEDIA_STATE_STOPED:
            avrcp_statre = BT_AVRCP_STATUS_PLAY_STOPPED;
            break;
        case BLE_MCS_MEDIA_STATE_PLAYING:
            avrcp_statre = BT_AVRCP_STATUS_PLAY_PLAYING;
            break;
        case BLE_MCS_MEDIA_STATE_PAUSED:
            avrcp_statre = BT_AVRCP_STATUS_PLAY_PAUSED;
            break;
        default:
            return;
    }
    bt_sink_srv_music_avrcp_status_change_notify(&dev_addr.addr, avrcp_statre);
}

static void le_sink_srv_state_change_notify(bt_sink_srv_state_t previous, bt_sink_srv_state_t now)
{
    g_le_sink_srv_state = now;
#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
    bt_sink_srv_state_manager_notify_state_change(BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_LE, now);
#else
    bt_sink_srv_map_new_state_notify(now, BT_SINK_SRV_STATE_LE_AUDIO_TYPE);
#endif
}
#endif

static void le_sink_srv_event_callback(uint16_t event_id, void *p_msg)
{
    bt_sink_srv_report_id("[Sink] le_sink_srv_event_callback, event_id: 0x%x", 1, event_id);

    le_sink_srv_update_active_handle(event_id, p_msg);
    le_sink_srv_le_call_event_callback(event_id, p_msg);

    switch (event_id) {
#ifdef AIR_LE_AUDIO_CIS_ENABLE
        case BT_LE_AUDIO_SINK_EVENT_CONNECTED: {
            bt_le_audio_sink_event_connected_t *noti = (bt_le_audio_sink_event_connected_t *)p_msg;
            bt_hci_le_set_periodic_advrtising_sync_transfer_params_t params = {0};
            params.handle = noti->handle;
            params.past_params.mode = 0x02;
            params.past_params.sync_timeout = 360;
            bt_gap_le_set_periodic_advertising_sync_transfer_parameters(&params);
            bt_sink_srv_cap_set_link(noti->handle);
            break;
        }
        case BT_LE_AUDIO_SINK_EVENT_DISCONNECTED: {
            bt_le_audio_sink_event_disconnected_t *noti = (bt_le_audio_sink_event_disconnected_t *)p_msg;
            bt_sink_srv_state_t sink_state = bt_sink_srv_get_state();
            bt_sink_srv_state_t pre_state = g_le_sink_srv_state;
            bool state_change = true;

            /* check current device is call incoming */
            if (BLE_TBS_INVALID_CALL_INDEX != bt_le_audio_sink_call_check_state(noti->handle, BLE_CCP_GTBS_INDEX, BLE_TBS_STATE_INCOMING)) {
                state_change = false;
            }

            /* check another device is call incoming/active */
            if (sink_state >= BT_SINK_SRV_STATE_STREAMING && sink_state <= BT_SINK_SRV_STATE_MULTIPARTY) {
                /* EDR music is playing or Call is doing*/
                state_change = false;
            }

            bt_handle_t check_handle = bt_sink_srv_cap_get_another_connected_link_handle(noti->handle);
            if (BLE_TBS_INVALID_CALL_INDEX != bt_le_audio_sink_call_check_state(check_handle, BLE_CCP_GTBS_INDEX, BLE_TBS_STATE_INCOMING) || bt_le_audio_sink_call_check_state(check_handle, 0xFF, BLE_TBS_STATE_ACTIVE)) {
                state_change = false;
            }

            if (state_change) {
                le_sink_srv_state_change_notify(pre_state, BT_SINK_SRV_STATE_POWER_ON);
            }
            le_sink_srv_profile_status_notify((BT_SINK_SRV_PROFILE_A2DP_SINK | BT_SINK_SRV_PROFILE_HFP), noti->handle, BT_SINK_SRV_PROFILE_CONNECTION_STATE_DISCONNECTED, BT_STATUS_SUCCESS);

            bt_sink_srv_cap_clear_link(noti->handle);
        #ifdef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
            bt_sink_srv_state_manager_notify_state_change(BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_LE, BT_SINK_SRV_STATE_NONE);
        #endif
            break;
        }
        case BT_SINK_SRV_CAP_EVENT_ASE_STATE: {
            bt_sink_srv_cap_event_ase_state_t *noti = (bt_sink_srv_cap_event_ase_state_t *)p_msg;
            if (noti) {
                le_sink_srv_context_t *le_sink_cntx = (le_sink_srv_context_t *)le_sink_srv_get_context(noti->connect_handle);
                if (le_sink_cntx == NULL) {
                    break;
                }
                uint16_t le_call_state = 0;
                (void)le_call_state;
                le_sink_cntx->music_state = le_sink_srv_state_transfer_to_bt_state(BT_SINK_SRV_CAP_EVENT_ASE_STATE, noti->current_state);
                bt_sink_srv_state_t pre_state = le_sink_srv_state_transfer_to_bt_state(BT_SINK_SRV_CAP_EVENT_ASE_STATE, noti->pre_state);
                bt_sink_srv_state_t cur_state = le_sink_srv_state_transfer_to_bt_state(BT_SINK_SRV_CAP_EVENT_ASE_STATE, noti->current_state);
                bt_sink_srv_report_id("[Sink] streaming state, pre_state:%x, cur_state:%x", 2, pre_state, cur_state);
                //bt_sink_srv_state_t sink_state = bt_sink_srv_get_state();
                if (BLE_TBS_INVALID_CALL_INDEX != bt_le_audio_sink_call_check_state(noti->connect_handle, BLE_CCP_GTBS_INDEX, BLE_TBS_STATE_INCOMING)) {
                    le_call_state = BT_SINK_SRV_STATE_INCOMING;
                }
                //bt_sink_srv_report_id("[Sink] le_bt_sink_srv_get_state: state:%x, le_state:%x", 2, sink_state, le_call_state);

                if (pre_state == BT_SINK_SRV_STATE_POWER_ON && cur_state == BT_SINK_SRV_STATE_CONNECTED) {
                    break;/* LE conn no need notify */
                }

            }
            break;
        }
        case BT_LE_AUDIO_SINK_EVENT_MEDIA_SERVICE_READY: {
            bt_le_audio_sink_event_service_ready_t *noti = (bt_le_audio_sink_event_service_ready_t *)p_msg;
            if (BT_STATUS_SUCCESS == noti->status) {
                le_sink_srv_profile_status_notify(BT_SINK_SRV_PROFILE_A2DP_SINK, noti->handle, BT_SINK_SRV_PROFILE_CONNECTION_STATE_CONNECTED, BT_STATUS_SUCCESS);
            }
            bt_sink_srv_report_id("Media config complete! handle:0x%04x", 1, noti->handle);
            atci_response_t *response = NULL;
            response = (atci_response_t*)bt_sink_srv_memory_alloc(sizeof(atci_response_t));
            if (NULL != response) {
                memset(response, 0, sizeof(atci_response_t));
                snprintf ((char *)response->response_buf, sizeof(response->response_buf), "Media config complete! handle:0x%04x\r\n",noti->handle);
                response->response_len = strlen((char *)response->response_buf);
                response->response_flag = 0 | ATCI_RESPONSE_FLAG_URC_FORMAT;
                atci_send_response(response);
                bt_sink_srv_memory_free(response);
            }
            break;
        }
        case BT_LE_AUDIO_SINK_EVENT_CALL_SERVICE_READY: {
            bt_le_audio_sink_event_service_ready_t *noti = (bt_le_audio_sink_event_service_ready_t *)p_msg;
            if (BT_STATUS_SUCCESS == noti->status) {
                le_sink_srv_profile_status_notify(BT_SINK_SRV_PROFILE_HFP, noti->handle, BT_SINK_SRV_PROFILE_CONNECTION_STATE_CONNECTED, BT_STATUS_SUCCESS);
            }
            bt_sink_srv_report_id("Call config complete! handle:0x%04x", 1, noti->handle);
            atci_response_t *response = NULL;
            response = (atci_response_t*)bt_sink_srv_memory_alloc(sizeof(atci_response_t));
            if (NULL != response) {
                memset(response, 0, sizeof(atci_response_t));
                snprintf ((char *)response->response_buf, sizeof(response->response_buf), "Call config complete! handle:0x%04x\r\n",noti->handle);
                response->response_len = strlen((char *)response->response_buf);
                response->response_flag = 0 | ATCI_RESPONSE_FLAG_URC_FORMAT;
                atci_send_response(response);
                bt_sink_srv_memory_free(response);
            }
            break;
        }
        case BT_LE_AUDIO_SINK_EVENT_MEDIA_STATE: {
            bt_le_audio_sink_event_media_state_t *noti = (bt_le_audio_sink_event_media_state_t *)p_msg;
            bt_sink_srv_report_id("[Sink] mcp state %d", 1, noti->state);

            if (noti->state == BLE_MCS_MEDIA_STATE_PLAYING) {
                if (!bt_sink_srv_cap_stream_is_source_ase_only(noti->handle, false)) {
                    bt_sink_srv_cap_stream_start_unicast_streaming(noti->handle);
                }
            }
#if 0 //inform streaming state from AM play/stop
            bt_sink_srv_state_t sink_state = bt_sink_srv_get_state();
            uint16_t le_call_state = 0;
            if (BLE_TBS_INVALID_CALL_INDEX != bt_le_audio_sink_call_check_state(noti->handle, BLE_CCP_GTBS_INDEX, BLE_TBS_STATE_INCOMING)) {
                 le_call_state = BT_SINK_SRV_STATE_INCOMING;
            } else if (BLE_TBS_INVALID_CALL_INDEX != bt_le_audio_sink_call_check_state(noti->handle, BLE_CCP_GTBS_INDEX, BLE_TBS_STATE_ACTIVE)) {
                le_call_state = BT_SINK_SRV_STATE_ACTIVE;
            }

            bt_sink_srv_report_id("[Sink] le_bt_sink_srv_get_state: state:%x, le_state:%x", 2, sink_state, le_call_state);
#ifndef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE

            if (sink_state >= BT_SINK_SRV_STATE_STREAMING && sink_state <= BT_SINK_SRV_STATE_MULTIPARTY) {
                break; /* EDR music is playing or Call is doing*/
            }

            if (le_call_state) {
                break; /* LE Music is interrupted by LE Call */
            }
#endif
            bt_sink_srv_state_t cur_state = le_sink_srv_state_transfer_to_bt_state(BT_LE_AUDIO_SINK_EVENT_MEDIA_STATE, noti->state);
            bt_sink_srv_state_t pre_state = BT_SINK_SRV_STATE_NONE;

            if (BLE_MCS_MEDIA_STATE_PLAYING == noti->state) {
                pre_state = BT_SINK_SRV_STATE_CONNECTED;
            } else {
                pre_state = BT_SINK_SRV_STATE_STREAMING;
            }
            bt_sink_srv_report_id("[Sink] mcp state change: pre_state:%x, cur_state:%x", 2, pre_state, cur_state);
            le_sink_srv_state_change_notify(pre_state, cur_state);
#endif
            le_sink_srv_mcp_state_change_notify(noti->handle, noti->state);
            break;
        }
        case BT_LE_AUDIO_SINK_EVENT_CALL_STATE: {
            bt_le_audio_sink_event_call_state_t *call_noti = (bt_le_audio_sink_event_call_state_t *)p_msg;
            if (call_noti) {
                bt_sink_srv_state_t pre_state = le_sink_srv_state_transfer_to_bt_state(BT_LE_AUDIO_SINK_EVENT_CALL_STATE, call_noti->prev_state);
                bt_sink_srv_state_t cur_state = le_sink_srv_state_transfer_to_bt_state(BT_LE_AUDIO_SINK_EVENT_CALL_STATE, call_noti->cur_state);
                bt_sink_srv_report_id("[Sink][LE][Call] call state, call pre_state: %x, cur_state:%x", 2, pre_state, cur_state);
                    le_sink_srv_state_change_notify(pre_state, cur_state);
                if (BT_SINK_SRV_STATE_ACTIVE == cur_state || BT_SINK_SRV_STATE_INCOMING == cur_state) {
                    bt_sink_srv_cap_stream_start_unicast_streaming(call_noti->handle);
                }
            }
            break;
        }

        case BT_LE_AUDIO_SINK_EVENT_MEDIA_RESUME:
        case BT_LE_AUDIO_SINK_EVENT_MEDIA_SUSPEND:
        {
            /*bt_sink_srv_cap_event_media_change_state_t *info = (bt_sink_srv_cap_event_media_change_state_t *)p_msg;
            if (info) {
                bt_sink_srv_le_media_state_change_callback(event_id, info->connect_handle, info->resume);
            }*/
            break;
        }

        case BT_LE_AUDIO_SINK_EVENT_CALL_INCOMING_CALL: {
        #ifdef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
            bt_gap_le_srv_conn_info_t *conn_info = NULL;
            bt_le_audio_sink_event_call_incoming_call_t *incoming_call = (bt_le_audio_sink_event_call_incoming_call_t *)p_msg;
            if (NULL != incoming_call) {
                conn_info = bt_gap_le_srv_get_conn_info(incoming_call->handle);
            }
            if (NULL != conn_info) {
                bt_sink_srv_state_manager_notify_ring_ind(BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_LE, &conn_info->peer_addr.addr, true);
            }
        #endif
        }
#endif
        /*Broadcast*/
        case BT_SINK_SRV_CAP_EVENT_BASE_BROADCAST_AUDIO_ANNOUNCEMENTS: {
            bt_sink_srv_report_id("[Sink][BIS] Broadcast audio announcements", 0);
            break;
        }

        case BT_SINK_SRV_CAP_EVENT_BASE_PERIODIC_ADV_SYNC_ESTABLISHED: {
            ble_bap_periodic_adv_sync_established_notify_t *noti = (ble_bap_periodic_adv_sync_established_notify_t *)p_msg;

            if (BT_STATUS_SUCCESS == noti->status) {
                g_le_sink_srv_sync_pa_retry_count = 0;
                bt_sink_srv_report_id("[Sink][BIS] PA sync established", 0);
            } else {
                g_le_sink_srv_sync_pa_retry_count ++;
                if (g_le_sink_srv_sync_pa_retry_count <= MAX_PA_SYNC_RETRY_NUM) {
                    bt_sink_srv_cap_stream_broadcast_sync_periodic_advretising(noti->advertiser_addr, noti->advertising_sid);
                } else {
                    g_le_sink_srv_sync_pa_retry_count = 0;
                    bt_sink_srv_cap_stream_stop_scanning_broadcast_source();
                    bt_sink_srv_report_id("[Sink][BIS] Fail to sync PA", 0);
                }
            }
            break;
        }
        case BT_SINK_SRV_CAP_EVENT_BASE_BASIC_AUDIO_ANNOUNCEMENTS: {
            bt_sink_srv_report_id("[Sink][BIS] Basic audio announcements", 0);
            //bt_sink_srv_cap_event_base_basic_audio_announcements_t *data = (bt_sink_srv_cap_event_base_basic_audio_announcements_t *)p_msg;
            break;
        }

        case BT_SINK_SRV_CAP_EVENT_BASE_BIGINFO_ADV_REPORT: {
            /*bt_sink_srv_report_id("[Sink][BIS] BIG info received", 0);
            bt_sink_srv_cap_event_base_biginfo_adv_report_t *data = (bt_sink_srv_cap_event_base_biginfo_adv_report_t *)p_msg;
            uint8_t bis_indices = 0xFF;
            bt_sink_srv_cap_stream_start_broadcast_reception(data->sync_handle, 1, 1, &bis_indices);*/
            break;
        }

        case BT_SINK_SRV_CAP_EVENT_BASE_BIG_SYNC_IND: {
            bt_sink_srv_cap_event_base_big_sync_ind_t *data = (bt_sink_srv_cap_event_base_big_sync_ind_t *)p_msg;
            bt_sink_srv_cap_stream_bmr_scan_info_ex_t *scan_info = bt_sink_srv_cap_stream_get_bmr_scan_info_ex();
            uint8_t bis_subgroup_idx = bt_sink_srv_cap_stream_get_bis_subgroup_idx();
            bt_sink_srv_report_id("[Sink] sync_handle:0x%4X, num_bis:%d, bis_indices[0]:%d, sync_policy:%d, subgroup idx:%d", 5,
                                  data->sync_handle, data->num_bis, data->bis_indices[0], scan_info->sync_policy, bis_subgroup_idx);
            if (le_audio_get_device_type() == LE_AUDIO_DEVICE_TYPE_HEADSET && BT_SINK_SRV_CAP_STREAM_SYNC_POLICY_SELECT_BIS != scan_info->sync_policy) {
                if (data->bis_indices[0] == 0) {
                    uint8_t bis_indices[2] = {BIS_INDEX_INVALID, BIS_INDEX_INVALID};
                    /*Checking 1 stereo BIS*/
                    bis_indices[0] = bt_sink_srv_cap_stream_get_bis_index_by_audio_location(AUDIO_LOCATION_FRONT_LEFT | AUDIO_LOCATION_FRONT_RIGHT, bis_subgroup_idx);
                    bt_sink_srv_report_id("[Sink] check stereo BIS index: 0x%x", 1, bis_indices[0]);
                    if (BIS_INDEX_INVALID == bis_indices[0]) {
                        /*Checking 2 mono BIS*/
                        bis_indices[0] = bt_sink_srv_cap_stream_get_bis_index_by_audio_location(AUDIO_LOCATION_FRONT_LEFT, bis_subgroup_idx);
                        bis_indices[1] = bt_sink_srv_cap_stream_get_bis_index_by_audio_location(AUDIO_LOCATION_FRONT_RIGHT, bis_subgroup_idx);
                        bt_sink_srv_report_id("[Sink] check mono BIS index: 0x%x, 0x%x", 2, bis_indices[0], bis_indices[1]);
                        if (BIS_INDEX_INVALID != bis_indices[0] && BIS_INDEX_INVALID != bis_indices[1]) {
                            bt_sink_srv_cap_stream_start_broadcast_reception(data->sync_handle, 1, 2, bis_indices);
                        } else {
                            bt_sink_srv_cap_stream_start_broadcast_reception(data->sync_handle, 1, 1, bis_indices);
                        }
                    } else {
                        bt_sink_srv_cap_stream_start_broadcast_reception(data->sync_handle, 1, 1, bis_indices);
                    }
                } else {
                    /*BSA select BIS index*/
                    bt_sink_srv_cap_stream_start_broadcast_reception(data->sync_handle, 1, data->num_bis, &data->bis_indices[0]);
                }

            } else {
                if (data->num_bis == 1 && data->bis_indices[0] == 0) {
                    uint8_t bis_indices = BIS_INDEX_INVALID;
                    /*No prefered BIS index*/
                    if (BT_SINK_SRV_CAP_STREAM_SYNC_POLICY_SELECT_BIS != scan_info->sync_policy) {
                        bt_sink_srv_cap_stream_start_broadcast_reception(data->sync_handle, 1, 1, &bis_indices);
                    }
                } else {
                    /*BSA select BIS index*/
                    bt_sink_srv_cap_stream_start_broadcast_reception(data->sync_handle, 1, data->num_bis, &data->bis_indices[0]);
                }
            }
            break;
        }

        case BT_SINK_SRV_CAP_EVENT_BASE_BIG_SYNC_ESTABLISHED: {
            bt_sink_srv_report_id("[Sink][BIS] BIG sync established", 0);
#if 0 /*No need to report sink srv state*/
            bt_sink_srv_state_t pre_state = BT_SINK_SRV_STATE_POWER_ON;
            bt_sink_srv_state_t cur_state = BT_SINK_SRV_STATE_STREAMING;
            le_sink_srv_state_change_notify(pre_state, cur_state);
#endif
            break;
        }
        case BT_SINK_SRV_CAP_EVENT_BASE_BIG_TERMINATE_IND: {
            bt_sink_srv_report_id("[Sink][BIS] BIG sync terminate ind", 0);
            break;
        }
        case BT_SINK_SRV_CAP_EVENT_BASE_BIG_TERMINATE_CFM: {
            bt_sink_srv_report_id("[Sink][BIS] BIG sync terminate cfm", 0);
#if 0 /*No need to report sink srv state*/
            bt_sink_srv_state_t sink_state = bt_sink_srv_get_state();
            bt_sink_srv_report_id("[Sink]EDR state:%X", sink_state);

            if (sink_state >= BT_SINK_SRV_STATE_STREAMING && sink_state <= BT_SINK_SRV_STATE_MULTIPARTY) {
                break; /* EDR music is playing or Call is doing*/
            }

            for (uint8_t i = 0; i < CAP_UNICAST_DEVICE_NUM; i++) {
                if (BLE_TBS_INVALID_CALL_INDEX != bt_le_audio_sink_call_check_state(bt_sink_srv_cap_get_link_handle(i), 0xFF, BLE_TBS_STATE_INCOMING) ||
                    BLE_TBS_INVALID_CALL_INDEX != bt_le_audio_sink_call_check_state(bt_sink_srv_cap_get_link_handle(i), 0xFF, BLE_TBS_STATE_ACTIVE)) {
                    return;  /* LE call is incoming/active */
                }
            }

            bt_sink_srv_state_t pre_state = BT_SINK_SRV_STATE_STREAMING;
            bt_sink_srv_state_t cur_state = BT_SINK_SRV_STATE_POWER_ON;
            le_sink_srv_state_change_notify(pre_state, cur_state);
#endif
            break;
        }
        default:
            break;
    }
}

void le_sink_srv_init(uint8_t max_link_num)
{
    bt_sink_srv_cap_init(le_sink_srv_event_callback, max_link_num);
    bt_sink_srv_le_call_init();
    g_call_active_handle.accept_handle = 0xFF; // just avoid build error
}

void le_sink_srv_set_streaming_state(bool is_streaming)
{
    bt_sink_srv_report_id("[Sink] le_sink_srv_set_streaming_state: is_streaming:%d, le_state:%x, bt_state:%x", 3, is_streaming, g_le_sink_srv_state, bt_sink_srv_get_state());
#ifdef AIR_ADAPTIVE_EQ_ENABLE
    if (!is_streaming) {
        bt_sink_srv_stop_am_notify(AUDIO_SRC_SRV_PSEUDO_DEVICE_BLE);
    }
#endif

#ifndef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
    bt_sink_srv_state_t sink_state = bt_sink_srv_get_state();
    if (sink_state >= BT_SINK_SRV_STATE_STREAMING && sink_state <= BT_SINK_SRV_STATE_MULTIPARTY) {
        return; /* EDR music is playing or Call is doing*/
    }

    if (g_le_sink_srv_state >= BT_SINK_SRV_STATE_INCOMING && g_le_sink_srv_state <= BT_SINK_SRV_STATE_MULTIPARTY) {
        return; /* LE calling*/
    }
#endif
    bt_sink_srv_state_t pre_state = (is_streaming ? BT_SINK_SRV_STATE_CONNECTED : BT_SINK_SRV_STATE_STREAMING);
    bt_sink_srv_state_t cur_state = (is_streaming ? BT_SINK_SRV_STATE_STREAMING : BT_SINK_SRV_STATE_CONNECTED);

    bt_sink_srv_report_id("[Sink] streaming state change: pre_state:%x, cur_state:%x", 2, pre_state, cur_state);
    le_sink_srv_state_change_notify(pre_state, cur_state);
    //le_sink_srv_mcp_state_change_notify(noti->state);
}

bt_sink_srv_state_t le_sink_srv_get_state(void)
{
    bt_sink_srv_report_id("[Sink][State] get_state:0x%x", 1, g_le_sink_srv_state);

    return g_le_sink_srv_state;
}

bt_status_t bt_sink_srv_state_manager_le_callback(bt_sink_srv_state_manager_event_t event, bt_bd_addr_t *address, void *parameter)
{
    bt_sink_srv_report_id("[Sink] bt_sink_srv_state_manager_le_callback:%d", 1, event);
    bt_utils_assert(address && "please check input address");
    bt_status_t ret = BT_STATUS_FAIL;
#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
    bt_handle_t handle = bt_gap_le_srv_get_conn_handle_by_address(address);
    switch(event) {
        case BT_SINK_SRV_STATE_MANAGER_EVENT_GET_STATE: {
            bt_sink_srv_device_state_t *dev_state = (bt_sink_srv_device_state_t*)parameter;
            dev_state->sco_state = BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED;
            if (handle != BT_HANDLE_INVALID) {
                //ble_bap_ase_id_list_t ase_list = bt_sink_srv_cap_stream_find_processing_and_streaming_ase_id_list(handle);
                bt_le_audio_direction_t direction = bt_sink_srv_cap_stream_find_streaming_ase_direction(handle, true);
                bool is_playing = bt_sink_srv_cap_am_is_psedev_streaming(CAP_AM_UNICAST_MUSIC_MODE_START + bt_sink_srv_cap_get_link_index(handle));

                if (direction == AUDIO_DIRECTION_SOURCE) {
                    dev_state->sco_state = BT_SINK_SRV_SCO_CONNECTION_STATE_CONNECTED;
                }
                dev_state->call_state = bt_sink_srv_le_get_call_state_by_handle(handle);

                if (direction == AUDIO_DIRECTION_SINK && is_playing) {
                    dev_state->music_state = BT_SINK_SRV_STATE_STREAMING;
                } else {
                    dev_state->music_state = BT_SINK_SRV_STATE_CONNECTED;
                }
                bt_sink_srv_report_id("[Sink] bt_le_audio_sink_media_get_state, state:%d", 1, dev_state->music_state);
                ret = BT_STATUS_SUCCESS;
            }

            break;
        }
        case BT_SINK_SRV_STATE_MANAGER_EVENT_GET_MEDIA_DEVICE: {
            audio_src_srv_handle_t **pseudo_dev = (audio_src_srv_handle_t**)parameter;
            if (handle != BT_HANDLE_INVALID) {
                *pseudo_dev = bt_sink_srv_cap_am_get_audio_handle(bt_sink_srv_cap_get_link_index(handle) + CAP_AM_UNICAST_MUSIC_MODE_START);
                ret = BT_STATUS_SUCCESS;
            }
            break;

        }
        case BT_SINK_SRV_STATE_MANAGER_EVENT_GET_CALL_DEVICE: {
            audio_src_srv_handle_t **pseudo_dev = (audio_src_srv_handle_t**)parameter;
            if (handle != BT_HANDLE_INVALID) {
                *pseudo_dev = bt_sink_srv_cap_am_get_audio_handle(bt_sink_srv_cap_get_link_index(handle) + CAP_AM_UNICAST_CALL_MODE_START);
                ret = BT_STATUS_SUCCESS;
            }
            break;

        }
        case BT_SINK_SRV_STATE_MANAGER_EVENT_GET_INBAND_SUPPORT: {
            bool *supported = (bool*)parameter;
            if (handle != BT_HANDLE_INVALID) {
                ble_ccp_status_flags_t status_flags = bt_le_audio_sink_call_get_status_flags(handle, BLE_CCP_GTBS_INDEX);
                *supported = (status_flags.in_band_ringtone ? true : false);
                ret = BT_STATUS_SUCCESS;
            }
            break;

        }
        default:
            break;
    }
#endif
    uint8_t *addr  = (uint8_t*)address;
    (void)addr;
    bt_sink_srv_report_id("[Sink][music] state_manager_le_callback: event:0x%x, ret:0x%x, addr--0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x",
        8, event, ret, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
    return ret;
}

bt_handle_t bt_sink_srv_le_action_parse_addr(uint32_t action, void *param)
{
    bt_handle_t active_handle = BT_HANDLE_INVALID;
    bt_bd_addr_t *bt_addr = NULL;
    bt_bd_addr_t *temp_addr = NULL;
    uint8_t *addr = NULL;
    switch (action) {
        case BT_SINK_SRV_ACTION_PLAY_PAUSE:
        case BT_SINK_SRV_ACTION_PLAY: {
            bt_sink_srv_action_play_t *play_para = (bt_sink_srv_action_play_t *)param;
            if (NULL != play_para && BT_SINK_SRV_DEVICE_LE == play_para->type) {
                bt_addr = &(play_para->address);
                addr  = (uint8_t*)bt_addr;
                active_handle = bt_gap_le_srv_get_conn_handle_by_address(bt_addr);
                if (!active_handle) {
                    bt_sink_srv_report_id("[sink] le_action_parse_addr, cannot get active handle.", 0);
                    bt_device_manager_le_bonded_info_t *le_bond_info = bt_device_manager_le_get_bonding_info_by_addr_ext(bt_addr);
                    if (le_bond_info != NULL) {
                        temp_addr = &(le_bond_info->bt_addr.addr);
                        addr  = (uint8_t*)temp_addr;
                        active_handle = bt_gap_le_srv_get_conn_handle_by_address(temp_addr);
                    }
                }
            } else {
                bt_sink_srv_report_id("[sink] le_action_parse_addr, invalid param", 0);
            }
            break;
        }
        case BT_SINK_SRV_ACTION_VOLUME_UP:
        case BT_SINK_SRV_ACTION_VOLUME_DOWN: {
            bt_sink_srv_action_volume_t *vol_para = (bt_sink_srv_action_volume_t *)param;
            if (NULL != vol_para && BT_SINK_SRV_DEVICE_LE == vol_para->type) {
                bt_addr = &(vol_para->address);
                addr  = (uint8_t*)bt_addr;
                active_handle = bt_gap_le_srv_get_conn_handle_by_address(bt_addr);
            }
            break;
        }

        default:
            break;

    }

    (void)addr;
    if (addr != NULL) {
        bt_sink_srv_report_id("[sink] le_action_parse_addr, action:%x addr--0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x, handle:0x%x",
             8, action, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], active_handle);
    }
    return active_handle;
}

#endif


