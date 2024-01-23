/* Copyright Statement:
 *
 * (C) 2023  Airoha Technology Corp. All rights reserved.
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

#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
#include "bt_type.h"
#include "bt_ull_service.h"
#include "bt_ull_le_service.h"
#include "bt_ull_utility.h"
#include "bt_ull_le_utility.h"
#include "bt_ull_le_call_service.h"
#include "bt_ull_le_audio_manager.h"

/**<  Invalid call index. */

typedef struct
{
    bt_ull_le_srv_call_index_t call_idx;
    bt_ull_le_srv_call_status_t call_state;
    bt_ull_le_srv_call_flag_t   call_flag;
    //ble_tbs_call_flag_t call_flags;
    //uint8_t join_flag;//Alerting/Dialing should use the flag to join after the call is answered by remote.
    //uint8_t uri_len;
    //uint8_t call_friendly_name_len;
    //uint8_t *uri;
    //uint8_t *call_friendly_name;
} bt_ull_le_srv_call_record_t;

typedef struct
{
    bt_ull_le_srv_call_status_t call_state;
} bt_ull_le_srv_call_state_notify_t;

typedef struct
{
    bt_ull_le_srv_call_action_t call_action;
} bt_ull_le_srv_call_action_notify_t;

typedef struct
{
    bt_ull_le_srv_call_record_t call_record_list[BT_ULL_LE_SRV_DEFAULT_MAX_CALL_COUNT];
    bt_ull_le_srv_call_index_t curr_call_index;
    bt_ull_le_srv_call_index_t pre_call_index;
} bt_ull_le_srv_call_state_incoming_info_t;

typedef struct
{
    bt_ull_le_srv_call_record_t call_record_list[BT_ULL_LE_SRV_DEFAULT_MAX_CALL_COUNT];
} bt_ull_le_srv_call_state_active_info_t;


typedef uint8_t bt_ull_le_call_event_t;
static bt_ull_le_srv_call_record_t g_bt_ull_le_srv_call_record_list[BT_ULL_LE_SRV_DEFAULT_MAX_CALL_COUNT];
static uint8_t g_bt_ull_le_max_call_count;
bt_ull_le_srv_call_index_t g_bt_ull_le_curr_call_index;
bt_ull_le_srv_call_index_t g_bt_ull_le_pre_call_index;

bt_status_t bt_ull_le_call_srv_init();
//static bt_ull_le_srv_call_record_t *bt_ull_le_call_srv_find_call_record(bt_ull_le_srv_call_index_t call_idx);
//static bt_ull_le_srv_call_record_t *bt_ull_le_call_srv_get_call_record(void);
static bt_status_t bt_ull_le_call_srv_handle_call_incoming(void);
static bt_status_t bt_ull_le_call_srv_handle_call_active(void);
static bt_status_t bt_ull_le_call_srv_handle_call_end();
static bt_status_t bt_ull_le_call_srv_accept_call(void);
static bt_status_t bt_ull_le_call_srv_reject_call(void);
static bt_status_t bt_ull_le_call_srv_terminate_call(void);
static bt_status_t bt_ull_le_call_srv_handle_mic_mute(bool is_mute);
static bt_status_t bt_ull_le_call_srv_notify_client_call_incoming(void);
static bt_status_t bt_ull_le_call_srv_notify_client_call_active(void);
static bt_status_t bt_ull_le_call_srv_notify_client_call_end(void);

//static uint8_t bt_ull_le_call_srv_get_call_record_num(void);

bt_status_t bt_ull_le_call_srv_send_event(bt_ull_le_srv_call_event_t call_event, void *extra_data, size_t data_len)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    ull_report("[ULL][LE][CALL] bt_ull_le_srv_call_send_event, call_event: 0x%x", 1, call_event);
    BT_ULL_MUTEX_LOCK();
    switch(call_event) {
        case BT_ULL_LE_SRV_CALL_EVENT_INCOMING: {
            status = bt_ull_le_call_srv_handle_call_incoming();
            break;
        }
        case BT_ULL_LE_SRV_CALL_EVENT_ACTIVE: {
            status = bt_ull_le_call_srv_handle_call_active();
            break;
        }
        case BT_ULL_LE_SRV_CALL_EVENT_REMOTE_HOLD: {
            break;
        }
        case BT_ULL_LE_SRV_CALL_EVENT_HOLD: {
            break;
        }
        case BT_ULL_LE_SRV_CALL_EVENT_UNHOLD: {
            break;
        }
        case BT_ULL_LE_SRV_CALL_EVENT_END: {
            status = bt_ull_le_call_srv_handle_call_end();
            break;
        }
        case BT_ULL_LE_SRV_CALL_EVENT_REMOTE_MIC_MUTE: {
            status = bt_ull_le_call_srv_handle_mic_mute(true);
            break;
        }
        case BT_ULL_LE_SRV_CALL_EVENT_REMOTE_MIC_UNMUTE: {
            status = bt_ull_le_call_srv_handle_mic_mute(false);
            break;
        }
        default:
            status = BT_STATUS_SUCCESS;
            break;
    }
    BT_ULL_MUTEX_UNLOCK();
    return status;
}

static bt_status_t bt_ull_le_call_srv_accept_call(void)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_srv_context_t* ctx = bt_ull_le_srv_get_context();
    if (BT_ULL_ROLE_CLIENT == ctx->role) {
        /*client notify server call action*/
        uint8_t i = BT_ULL_LE_CLIENT_LINK_MAX_NUM;
        while (0 != i) {
            i--;
            if ((BT_HANDLE_INVALID != bt_ull_le_srv_get_connection_handle_by_index(i)) &&
                (BT_ULL_LE_LINK_STATE_READY <= bt_ull_le_srv_get_link_state_by_index(i))) {
                uint8_t data_len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_le_srv_call_action_t);
                uint8_t *data = (uint8_t *)bt_ull_le_srv_memory_alloc(data_len);
                if (NULL != data) {
                    data[0] = BT_ULL_EVENT_CALL_ACTION;
                    data[1] = BT_ULL_LE_SRV_CALL_ACTION_ANSWER;
                    status = bt_ull_le_srv_send_data(bt_ull_le_srv_get_connection_handle_by_index(i), (uint8_t*)data,data_len);
                    bt_ull_le_srv_memory_free(data);
                }
            }
        }        
    }
    return status;
}

static bt_status_t bt_ull_le_call_srv_reject_call(void)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_srv_context_t* ctx = bt_ull_le_srv_get_context();
    if (BT_ULL_ROLE_CLIENT == ctx->role) {
        /*client notify server call action*/
        uint8_t i = BT_ULL_LE_CLIENT_LINK_MAX_NUM;
        while (0 != i) {
            i--;
            if ((BT_HANDLE_INVALID != bt_ull_le_srv_get_connection_handle_by_index(i)) &&
                (BT_ULL_LE_LINK_STATE_READY <= bt_ull_le_srv_get_link_state_by_index(i))) {
                uint8_t data_len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_le_srv_call_action_t);
                uint8_t *data = (uint8_t *)bt_ull_le_srv_memory_alloc(data_len);
                if (NULL != data) {
                    data[0] = BT_ULL_EVENT_CALL_ACTION;
                    data[1] = BT_ULL_LE_SRV_CALL_ACTION_REJECT;
                    status = bt_ull_le_srv_send_data(bt_ull_le_srv_get_connection_handle_by_index(i), (uint8_t*)data,data_len);
                    bt_ull_le_srv_memory_free(data);
                }
            }
        }        
    }
    return status;
}

static bt_status_t bt_ull_le_call_srv_terminate_call(void)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_srv_context_t* ctx = bt_ull_le_srv_get_context();
    if (BT_ULL_ROLE_CLIENT == ctx->role) {
        /*client notify server call action*/
        uint8_t i = BT_ULL_LE_CLIENT_LINK_MAX_NUM;
        while (0 != i) {
            i--;
            if ((BT_HANDLE_INVALID != bt_ull_le_srv_get_connection_handle_by_index(i)) &&
                (BT_ULL_LE_LINK_STATE_READY <= bt_ull_le_srv_get_link_state_by_index(i))) {
                uint8_t data_len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_le_srv_call_action_t);
                uint8_t *data = (uint8_t *)bt_ull_le_srv_memory_alloc(data_len);
                if (NULL != data) {
                    data[0] = BT_ULL_EVENT_CALL_ACTION;
                    data[1] = BT_ULL_LE_SRV_CALL_ACTION_TERMINATE;
                    status = bt_ull_le_srv_send_data(bt_ull_le_srv_get_connection_handle_by_index(i), (uint8_t*)data,data_len);
                    bt_ull_le_srv_memory_free(data);
                }
            }
        }        
    }
    return status;
}

bt_status_t bt_ull_le_call_srv_send_action(bt_ull_le_srv_call_action_t  call_action)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    ull_report("[ULL][LE][CALL] bt_ull_le_call_srv_send_action, call_action: 0x%x", 1, call_action);
    switch(call_action) {
        case BT_ULL_LE_SRV_CALL_ACTION_ANSWER: {
            status = bt_ull_le_call_srv_accept_call();
            break;
        }
        case BT_ULL_LE_SRV_CALL_ACTION_REJECT: {
            status = bt_ull_le_call_srv_reject_call();
            break;
        }
        case BT_ULL_LE_SRV_CALL_ACTION_TERMINATE: {
            status = bt_ull_le_call_srv_terminate_call();
            break;
        }
        case BT_ULL_LE_SRV_CALL_ACTION_MUTE: {
            bt_ull_le_call_srv_handle_mic_mute(true);
            break;
        }
        case BT_ULL_LE_SRV_CALL_ACTION_UNMUTE: {
            bt_ull_le_call_srv_handle_mic_mute(false);
            break;
        }
        default:
            break;
    }
    return status;
}

bt_status_t bt_ull_le_call_srv_client_handle_call_state(bt_ull_le_srv_call_status_t call_state, void *data)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_srv_stream_context_t* stream_ctx = bt_ull_le_srv_get_stream_context();
    ull_report("[ULL][LE][CALL] bt_ull_le_call_srv_client_handle_call_state, call_state: 0x%x", 1, call_state);
    switch(call_state) {
        case BT_ULL_LE_SRV_CALL_STATE_INCOMING: {
            //bt_ull_le_srv_call_state_incoming_info_t *incoming_call_info = (bt_ull_le_srv_call_state_incoming_info_t*)data;
            //uint8_t i;
            /*update call info*/
            /*for (i = 0; i < BT_ULL_LE_SRV_DEFAULT_MAX_CALL_COUNT; i++) {
                g_bt_ull_le_srv_call_record_list[i].call_idx = incoming_call_info->call_record_list[i].call_idx;
                g_bt_ull_le_srv_call_record_list[i].call_state = incoming_call_info->call_record_list[i].call_state;
                g_bt_ull_le_srv_call_record_list[i].call_flag = incoming_call_info->call_record_list[i].call_flag;
            }*/
            //g_bt_ull_le_curr_call_index = incoming_call_info->curr_call_index;
            //g_bt_ull_le_pre_call_index = incoming_call_info->pre_call_index;

            /*update app call state change*/
            bt_ull_le_srv_call_state_notify_t call_state_notify;
            call_state_notify.call_state = BT_ULL_LE_SRV_CALL_STATE_INCOMING;
            bt_ull_le_srv_event_callback(BT_ULL_EVENT_LE_CALL_STATE, (void *)&call_state_notify, sizeof(bt_ull_le_srv_call_state_notify_t));
            break;
        }
        case BT_ULL_LE_SRV_CALL_STATE_ACTIVE: {
            //bt_ull_le_srv_call_state_active_info_t *active_call_info = (bt_ull_le_srv_call_state_active_info_t*)data;
            //uint8_t i;
            /*update call info*/
            /*for (i = 0; i < BT_ULL_LE_SRV_DEFAULT_MAX_CALL_COUNT; i++) {
                g_bt_ull_le_srv_call_record_list[i].call_idx = active_call_info->call_record_list[i].call_idx;
                g_bt_ull_le_srv_call_record_list[i].call_state = active_call_info->call_record_list[i].call_state;
                g_bt_ull_le_srv_call_record_list[i].call_flag = active_call_info->call_record_list[i].call_flag;
            }*/
            /*update app call state change*/
            bt_ull_le_srv_call_state_notify_t call_state_notify;
            call_state_notify.call_state = BT_ULL_LE_SRV_CALL_STATE_ACTIVE;
            bt_ull_le_srv_event_callback(BT_ULL_EVENT_LE_CALL_STATE, (void *)&call_state_notify, sizeof(bt_ull_le_srv_call_state_notify_t));

            break;
        }
        case BT_ULL_LE_SRV_CALL_STATE_STATE_MIC_MUTE: {
            /*update app call state change*/
            bt_ull_le_srv_call_state_notify_t call_state_notify;
            call_state_notify.call_state = BT_ULL_LE_SRV_CALL_STATE_STATE_MIC_MUTE;
            bt_ull_le_srv_event_callback(BT_ULL_EVENT_LE_CALL_STATE, (void *)&call_state_notify, sizeof(bt_ull_le_srv_call_state_notify_t));

            stream_ctx->client.ul.is_mute = true;
            bt_ull_le_am_set_mute(BT_ULL_LE_AM_UL_MODE, true);
            break;
        }
        case BT_ULL_LE_SRV_CALL_STATE_STATE_MIC_UNMUTE: {
            /*update app call state change*/
            bt_ull_le_srv_call_state_notify_t call_state_notify;
            call_state_notify.call_state = BT_ULL_LE_SRV_CALL_STATE_STATE_MIC_UNMUTE;
            bt_ull_le_srv_event_callback(BT_ULL_EVENT_LE_CALL_STATE, (void *)&call_state_notify, sizeof(bt_ull_le_srv_call_state_notify_t));

            stream_ctx->client.ul.is_mute = false;
            bt_ull_le_am_set_mute(BT_ULL_LE_AM_UL_MODE, false);
            break;
        }        
        case BT_ULL_LE_SRV_CALL_STATE_STATE_IDLE: {
            //bt_ull_le_srv_call_state_incoming_info_t *incoming_call_info = (bt_ull_le_srv_call_state_incoming_info_t*)data;
            //uint8_t i;
            /*update call info*/
            /*for (i = 0; i < BT_ULL_LE_SRV_DEFAULT_MAX_CALL_COUNT; i++) {
                g_bt_ull_le_srv_call_record_list[i].call_idx = incoming_call_info->call_record_list[i].call_idx;
                g_bt_ull_le_srv_call_record_list[i].call_state = incoming_call_info->call_record_list[i].call_state;
                g_bt_ull_le_srv_call_record_list[i].call_flag = incoming_call_info->call_record_list[i].call_flag;
            }*/
            //g_bt_ull_le_curr_call_index = incoming_call_info->curr_call_index;
            //g_bt_ull_le_pre_call_index = incoming_call_info->pre_call_index;

            /*update app call state change*/
            bt_ull_le_srv_call_state_notify_t call_state_notify;
            call_state_notify.call_state = BT_ULL_LE_SRV_CALL_STATE_STATE_IDLE;
            bt_ull_le_srv_event_callback(BT_ULL_EVENT_LE_CALL_STATE, (void *)&call_state_notify, sizeof(bt_ull_le_srv_call_state_notify_t));
            break;
        }
        default:
            break;
    }
    return  status;
}

bt_status_t bt_ull_le_call_srv_client_handle_call_action(bt_ull_le_srv_call_action_t call_action, void *data)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    //bt_ull_le_srv_stream_context_t* stream_ctx = bt_ull_le_srv_get_stream_context();
    ull_report("[ULL][LE][CALL] bt_ull_le_call_srv_client_handle_call_action, call_action: 0x%x", 1, call_action);
    switch(call_action) {
        case BT_ULL_LE_SRV_CALL_ACTION_ANSWER: {
            /*notify app call action*/
            bt_ull_le_srv_call_action_notify_t call_action_notify;
            call_action_notify.call_action = BT_ULL_LE_SRV_CALL_ACTION_ANSWER;
            bt_ull_le_srv_event_callback(BT_ULL_EVENT_LE_CALL_ACTION, (void *)&call_action_notify, sizeof(bt_ull_le_srv_call_action_notify_t));            
            break;
        }
        case BT_ULL_LE_SRV_CALL_ACTION_REJECT: {
            /*notify app call action*/
            bt_ull_le_srv_call_action_notify_t call_action_notify;
            call_action_notify.call_action = BT_ULL_LE_SRV_CALL_ACTION_REJECT;
            bt_ull_le_srv_event_callback(BT_ULL_EVENT_LE_CALL_ACTION, (void *)&call_action_notify, sizeof(bt_ull_le_srv_call_action_notify_t)); 
            break;
        }
        case BT_ULL_LE_SRV_CALL_ACTION_TERMINATE: {
            /*notify app call action*/
            bt_ull_le_srv_call_action_notify_t call_action_notify;
            call_action_notify.call_action = BT_ULL_LE_SRV_CALL_ACTION_TERMINATE;
            bt_ull_le_srv_event_callback(BT_ULL_EVENT_LE_CALL_ACTION, (void *)&call_action_notify, sizeof(bt_ull_le_srv_call_action_notify_t)); 
            break;
        }
        case BT_ULL_LE_SRV_CALL_ACTION_MUTE: {
            /*notify app call action*/
            bt_ull_le_srv_call_action_notify_t call_action_notify;
            call_action_notify.call_action = BT_ULL_LE_SRV_CALL_ACTION_MUTE;
            bt_ull_le_srv_event_callback(BT_ULL_EVENT_LE_CALL_ACTION, (void *)&call_action_notify, sizeof(bt_ull_le_srv_call_action_notify_t)); 
            break;

        }
        case BT_ULL_LE_SRV_CALL_ACTION_UNMUTE: {
            /*notify app call action*/
            bt_ull_le_srv_call_action_notify_t call_action_notify;
            call_action_notify.call_action = BT_ULL_LE_SRV_CALL_ACTION_UNMUTE;
            bt_ull_le_srv_event_callback(BT_ULL_EVENT_LE_CALL_ACTION, (void *)&call_action_notify, sizeof(bt_ull_le_srv_call_action_notify_t)); 
            break;
        }
        default:
            break;
    }
    return status;
}

bt_status_t bt_ull_le_call_srv_init(void)
{
    g_bt_ull_le_max_call_count = BT_ULL_LE_SRV_DEFAULT_MAX_CALL_COUNT;
    g_bt_ull_le_curr_call_index = BT_ULL_LE_SRV_INVALID_CALL_INDEX;
    g_bt_ull_le_pre_call_index = BT_ULL_LE_SRV_INVALID_CALL_INDEX;
    bt_ull_le_srv_memset(g_bt_ull_le_srv_call_record_list, 0, sizeof(bt_ull_le_srv_call_record_t)*BT_ULL_LE_SRV_DEFAULT_MAX_CALL_COUNT);
    return BT_STATUS_SUCCESS;
}
#if 0
static bt_ull_le_srv_call_record_t *bt_ull_le_call_srv_get_call_record(void)
{
    uint8_t i = 0;
    //uint16_t call_list_idx = 0;
    if (NULL == g_bt_ull_le_srv_call_record_list) {
        return NULL;
    }
    for(i = 0; i < g_bt_ull_le_max_call_count; i++) {
        if (BT_ULL_LE_SRV_INVALID_CALL_INDEX == g_bt_ull_le_srv_call_record_list[i].call_idx) {
            g_bt_ull_le_srv_call_record_list[i].call_idx = i+1;
            g_bt_ull_le_srv_call_record_list[i].call_state = BT_ULL_LE_SRV_CALL_STATE_STATE_INVALID;
            return &g_bt_ull_le_srv_call_record_list[i];
        }
    }
    return NULL;
}

static bt_ull_le_srv_call_record_t *bt_ull_le_call_srv_find_call_record(bt_ull_le_srv_call_index_t call_idx)
{
    if ((NULL == g_bt_ull_le_srv_call_record_list) || (BT_ULL_LE_SRV_INVALID_CALL_INDEX == call_idx)) {
        return NULL;
    }
    return &g_bt_ull_le_srv_call_record_list[call_idx];
}
#endif
static bt_status_t bt_ull_le_call_srv_handle_call_incoming(void)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    //bt_ull_le_srv_call_index_t call_idx;
    //bt_ull_le_srv_call_record_t *record = NULL;
    /*if (NULL == (record = bt_ull_le_call_srv_get_call_record())) {
        ull_report("[ULL][LE][CALL][DEBUG_0] ", 0);
        return BT_STATUS_FAIL;
    }*/
    //record->call_state = BT_ULL_LE_SRV_CALL_STATE_INCOMING;
    //record->call_flag = BT_ULL_LE_SRV_CALL_FLAG_INCOMING_CALL;
    /*update call index*/
    /*if (BT_ULL_LE_SRV_INVALID_CALL_INDEX != g_bt_ull_le_curr_call_index) {
        g_bt_ull_le_pre_call_index = g_bt_ull_le_curr_call_index;
    } else {
        g_bt_ull_le_curr_call_index = record->call_idx;
    }*/
    status = bt_ull_le_call_srv_notify_client_call_incoming();
    //ull_report("[ULL][LE][CALL] bt_ull_le_call_srv_handle_call_incoming, new_call_index: 0x%x, curr_call", 2, record->call_idx, g_bt_ull_le_curr_call_index);
    /*if (BT_ULL_LE_SRV_INVALID_CALL_INDEX == record->call_idx) {
        return BT_STATUS_FAIL;
    }*/
    return status;
}

/*static bt_status_t bt_ull_le_call_srv_handle_call_outgoing(void)
{
	ull_report("[ULL][LE][CALL] bt_ull_le_call_srv_handle_call_outgoing", 0);
    return BT_STATUS_SUCCESS;
}*/

static bt_status_t bt_ull_le_call_srv_handle_call_active(void)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    //bt_ull_le_srv_call_index_t call_idx;
    //bt_ull_le_srv_call_record_t *record = NULL;

    //record = bt_ull_le_call_srv_find_call_record(g_bt_ull_le_curr_call_index);
    //record->call_state = BT_ULL_LE_SRV_CALL_STATE_ACTIVE;
    status = bt_ull_le_call_srv_notify_client_call_active();

    return status;
}

static bt_status_t bt_ull_le_call_srv_handle_call_end(void)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    //bt_ull_le_srv_call_index_t call_idx;
    //bt_ull_le_srv_call_record_t *record = NULL;
    //bt_ull_le_srv_call_status_t pre_call_state = BT_ULL_LE_SRV_CALL_STATE_STATE_INVALID;

    //record = bt_ull_le_call_srv_find_call_record(g_bt_ull_le_curr_call_index);
    //record->call_state = BT_ULL_LE_SRV_CALL_STATE_STATE_IDLE;
    //g_bt_ull_le_curr_call_index = BT_ULL_LE_SRV_INVALID_CALL_INDEX;
    status = bt_ull_le_call_srv_notify_client_call_end();

    return status;
}

static bt_status_t bt_ull_le_call_srv_handle_mic_mute(bool is_mute)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_srv_context_t* ctx = bt_ull_le_srv_get_context();
    bt_ull_le_srv_stream_context_t* stream_ctx = bt_ull_le_srv_get_stream_context();

    ull_report("[ULL][LE][CALL] bt_ull_le_call_srv_handle_mic_mute, is_mute: %d", 1, is_mute);
        if (BT_ULL_ROLE_CLIENT == ctx->role) {
            //stream_ctx->client.ul.is_mute = is_mute;
            //bt_ull_le_am_set_mute(BT_ULL_LE_AM_UL_MODE, is_mute);
            /* sync mute operation to server */
            uint8_t i = BT_ULL_LE_CLIENT_LINK_MAX_NUM;
            while (0 != i) {
                i--;
                if ((BT_HANDLE_INVALID != bt_ull_le_srv_get_connection_handle_by_index(i)) &&
                    (BT_ULL_LE_LINK_STATE_READY <= bt_ull_le_srv_get_link_state_by_index(i))) {
                    uint8_t data_len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_le_srv_call_status_t);
                    uint8_t *data = (uint8_t *)bt_ull_le_srv_memory_alloc(data_len);

                    if (NULL != data) {
                        data[0] = BT_ULL_EVENT_CALL_ACTION;
                        data[1] = is_mute ? BT_ULL_LE_SRV_CALL_ACTION_MUTE : BT_ULL_LE_SRV_CALL_ACTION_UNMUTE;
                        status = bt_ull_le_srv_send_data(bt_ull_le_srv_get_connection_handle_by_index(i), (uint8_t*)data, data_len);
                        bt_ull_le_srv_memory_free(data);
                    }
                }
            }

        } else if (BT_ULL_ROLE_SERVER == ctx->role) {
            stream_ctx->server.stream[BT_ULL_MIC_TRANSMITTER].is_mute = is_mute;
            bt_ull_le_at_set_mute(BT_ULL_MIC_TRANSMITTER, is_mute);
            /* sync mute operation to server */
            uint8_t i = BT_ULL_LE_MAX_LINK_NUM;
            while (0 != i) {
                i--;
                if ((BT_HANDLE_INVALID != bt_ull_le_srv_get_connection_handle_by_index(i)) &&
                    (BT_ULL_LE_LINK_STATE_READY <= bt_ull_le_srv_get_link_state_by_index(i))) {
                      uint8_t data_len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_le_srv_call_status_t);
                      uint8_t *data = (uint8_t *)bt_ull_le_srv_memory_alloc(data_len);

                        if (NULL != data) {
                            data[0] = BT_ULL_EVENT_CALL_STATE;
                            data[1] = is_mute ? BT_ULL_LE_SRV_CALL_STATE_STATE_MIC_MUTE : BT_ULL_LE_SRV_CALL_STATE_STATE_MIC_UNMUTE;
                            status = bt_ull_le_srv_send_data(bt_ull_le_srv_get_connection_handle_by_index(i), (uint8_t*)data, data_len);
                            bt_ull_le_srv_memory_free(data);
                        }
                    }
                }
        } else {
            ull_assert(0 && "[ULL][LE] unknown role");
        }
        return status;
}


static bt_status_t bt_ull_le_call_srv_notify_client_call_incoming(void)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_srv_context_t* ctx = bt_ull_le_srv_get_context();    
    ull_report("[ULL][LE][CALL] bt_ull_le_call_srv_notify_client_call_incoming, call_state: 0x%x", 1, BT_ULL_LE_SRV_CALL_STATE_INCOMING);
    if (bt_ull_le_service_is_connected() && (BT_ULL_ROLE_SERVER == ctx->role)) {
        uint8_t i = BT_ULL_LE_MAX_LINK_NUM;
        while(0 != i) {
            i--;
            if ((BT_HANDLE_INVALID != bt_ull_le_srv_get_connection_handle_by_index(i)) && (BT_ULL_LE_LINK_STATE_READY <= bt_ull_le_srv_get_link_state_by_index(i))) {
                uint8_t data_len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_le_srv_call_status_t);
                uint8_t *data = (uint8_t *)bt_ull_le_srv_memory_alloc(data_len);
                if (NULL != data) {
                    data[0] = BT_ULL_EVENT_CALL_STATE;
                    data[1] = BT_ULL_LE_SRV_CALL_STATE_INCOMING;
                    status = bt_ull_le_srv_send_data(bt_ull_le_srv_get_connection_handle_by_index(i), (uint8_t*)data, data_len);
                    bt_ull_le_srv_memory_free(data);
                }
            }
        }
    }
    return status;
}


bt_status_t bt_ull_le_call_srv_notify_client_call_active(void)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_srv_context_t* ctx = bt_ull_le_srv_get_context();    
    ull_report("[ULL][LE][CALL] bt_ull_le_call_srv_notify_client_call_active", 0);
    if (bt_ull_le_service_is_connected() && (BT_ULL_ROLE_SERVER == ctx->role)) {
        uint8_t i = BT_ULL_LE_MAX_LINK_NUM;
        while(0 != i) {
            i--;
            if ((BT_HANDLE_INVALID != bt_ull_le_srv_get_connection_handle_by_index(i)) && (BT_ULL_LE_LINK_STATE_READY <= bt_ull_le_srv_get_link_state_by_index(i))) {
                uint8_t data_len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_le_srv_call_status_t);
                uint8_t *data = (uint8_t *)bt_ull_le_srv_memory_alloc(data_len);
                if (NULL != data) {
                    data[0] = BT_ULL_EVENT_CALL_STATE;
                    data[1] = BT_ULL_LE_SRV_CALL_STATE_ACTIVE;
                    status = bt_ull_le_srv_send_data(bt_ull_le_srv_get_connection_handle_by_index(i), (uint8_t*)data, data_len);
                    bt_ull_le_srv_memory_free(data);
                }
            }
        }
    }
    return status;
}


static bt_status_t bt_ull_le_call_srv_notify_client_call_end(void)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_le_srv_context_t* ctx = bt_ull_le_srv_get_context();    
    ull_report("[ULL][LE][CALL] bt_ull_le_call_srv_notify_client_call_end, call_state: 0x%x", 1, BT_ULL_LE_SRV_CALL_STATE_STATE_IDLE);
    if (bt_ull_le_service_is_connected() && (BT_ULL_ROLE_SERVER == ctx->role)) {
        uint8_t i = BT_ULL_LE_MAX_LINK_NUM;
        while(0 != i) {
            i--;
            if ((BT_HANDLE_INVALID != bt_ull_le_srv_get_connection_handle_by_index(i)) && (BT_ULL_LE_LINK_STATE_READY <= bt_ull_le_srv_get_link_state_by_index(i))) {
                uint8_t data_len = sizeof(bt_ull_req_event_t) + sizeof(bt_ull_le_srv_call_status_t);
                uint8_t *data = (uint8_t *)bt_ull_le_srv_memory_alloc(data_len);
                if (NULL != data) {
                    data[0] = BT_ULL_EVENT_CALL_STATE;
                    data[1] = BT_ULL_LE_SRV_CALL_STATE_STATE_IDLE;
                    status = bt_ull_le_srv_send_data(bt_ull_le_srv_get_connection_handle_by_index(i), (uint8_t*)data, data_len);
                    bt_ull_le_srv_memory_free(data);
                }
            }
        }
    }
    return status;
}
#if 0
static uint8_t bt_ull_le_call_srv_get_call_record_num(void)
{
    uint8_t i;
    uint8_t num;
    if(NULL == g_bt_ull_le_srv_call_record_list) {
        return 0;
    }
    for (i = 0; i < BT_ULL_LE_SRV_DEFAULT_MAX_CALL_COUNT; i++) {
        if (BLE_TBS_INVALID_CALL_INDEX != g_bt_ull_le_srv_call_record_list[i].call_idx
            && BT_ULL_LE_SRV_CALL_STATE_STATE_INVALID != g_bt_ull_le_srv_call_record_list[i].call_state
            && BT_ULL_LE_SRV_CALL_STATE_STATE_IDLE != g_bt_ull_le_srv_call_record_list[i].call_state) {
            num++;
        }
    }
    return num;
}
#endif

#endif



