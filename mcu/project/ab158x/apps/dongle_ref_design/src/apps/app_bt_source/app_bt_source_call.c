
/* Copyright Statement:
 *
 * (C) 2019  Airoha Technology Corp. All rights reserved.
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
 * File: app_bt_source_call.c
 *
 * Description: This file provides API for BT Source Call.
 *
 */

#ifdef AIR_BT_SOURCE_ENABLE

#include "app_bt_source_call.h"

#include "app_bt_source_conn_mgr.h"

#include "apps_debug.h"
#include "apps_events_event_group.h"
#include "bt_source_srv.h"
#include "usbaudio_drv.h"
#include "usb_hid_srv.h"
#include "ui_shell_manager.h"



#define LOG_TAG             "[BT_SRC][CALL]"

typedef enum {
    APP_BT_SOURCE_NEW_CALL_TYPE_INCOMING,
    APP_BT_SOURCE_NEW_CALL_TYPE_DIALING,
    APP_BT_SOURCE_NEW_CALL_TYPE_ACTIVATE,
} app_bt_source_new_call_type_t;

typedef struct {
    bt_source_srv_call_index_t      index;              // 1~0xFF, invalid 0
    bt_source_srv_call_state_t      state;
    bt_addr_t                       peer_addr;
    bool                            current;
} PACKED app_bt_source_call_info_t;

typedef struct {
   bool                             incoming_flag;
   bool                             dialing_flag;
   app_bt_source_call_info_t        call_info[APP_BT_SOURCE_CALL_MAX_NUM];
} PACKED app_bt_source_call_context_t;

static app_bt_source_call_context_t                app_bt_source_call_ctx = {0};

static bool                                        app_bt_source_delay_hold_flag = FALSE;



/**================================================================================*/
/**                                   Internal API                                 */
/**================================================================================*/
extern bool app_bt_source_send_action(bt_source_srv_action_t action, void *parameter, uint32_t length);

static uint8_t app_bt_source_call_get_num(void)
{
    uint8_t num = 0;
    for (int i = 0; i < APP_BT_SOURCE_CALL_MAX_NUM; i++) {
        app_bt_source_call_info_t *call_info = &app_bt_source_call_ctx.call_info[i];
        if (call_info->index != BT_SOURCE_SRV_CALL_INVALID_INDEX) {
            num++;
        }
    }
    return num;
}

static void app_bt_source_call_set_current_flag(bt_source_srv_call_index_t index)
{
    if (index == BT_SOURCE_SRV_CALL_INVALID_INDEX) {
        return;
    }

    for (int i = 0; i < APP_BT_SOURCE_CALL_MAX_NUM; i++) {
        app_bt_source_call_info_t *call_info = &app_bt_source_call_ctx.call_info[i];
        if (call_info->index == index) {
            call_info->current = TRUE;
        } else {
            call_info->current = FALSE;
        }
    }
}

static void app_bt_source_call_update_current_flag(void)
{
    bool set_current = FALSE;
    for (int i = 0; i < APP_BT_SOURCE_CALL_MAX_NUM; i++) {
        app_bt_source_call_info_t *call_info = &app_bt_source_call_ctx.call_info[i];
        if (call_info->index != BT_SOURCE_SRV_CALL_INVALID_INDEX && !set_current) {
            call_info->current = TRUE;
            set_current = TRUE;
        } else {
            call_info->current = FALSE;
        }
    }
}

static app_bt_source_call_info_t* app_bt_source_call_get_current_call_info(void)
{
    int call_info_index = -1;
    app_bt_source_call_info_t *result = NULL;
    for (int i = 0; i < APP_BT_SOURCE_CALL_MAX_NUM; i++) {
        app_bt_source_call_info_t *call_info = &app_bt_source_call_ctx.call_info[i];
        if (call_info->index != BT_SOURCE_SRV_CALL_INVALID_INDEX && call_info->current) {
            result = call_info;
            call_info_index = i;
            break;
        }
    }

    if (result != NULL) {
        APPS_LOG_MSGID_I(LOG_TAG" get_current_call_info, [%d] call_index=%d state=%d",
                         3, call_info_index, result->index, result->state);
    } else {
        APPS_LOG_MSGID_E(LOG_TAG" get_current_call_info, fail", 0);
    }

    return result;
}

static app_bt_source_call_info_t* app_bt_source_call_get_call_info_by_state(bt_source_srv_call_state_t state)
{
    app_bt_source_call_info_t *result = NULL;
    for (int i = 0; i < APP_BT_SOURCE_CALL_MAX_NUM; i++) {
        app_bt_source_call_info_t *call_info = &app_bt_source_call_ctx.call_info[i];
        if (call_info->index != BT_SOURCE_SRV_CALL_INVALID_INDEX && call_info->state == state) {
            result = call_info;
            break;
        }
    }
    return result;
}

static app_bt_source_call_info_t* app_bt_source_call_get_call_info_by_index(bt_source_srv_call_index_t index)
{
    if (index == BT_SOURCE_SRV_CALL_INVALID_INDEX) {
        return NULL;
    }

    app_bt_source_call_info_t *result = NULL;
    for (int i = 0; i < APP_BT_SOURCE_CALL_MAX_NUM; i++) {
        app_bt_source_call_info_t *call_info = &app_bt_source_call_ctx.call_info[i];
        if (call_info->index == index) {
            result = call_info;
            break;
        }
    }
    return result;
}

static bool app_bt_source_call_is_exist_active_call(void)
{
    app_bt_source_call_info_t *active_call_info = app_bt_source_call_get_call_info_by_state(BT_SOURCE_SRV_CALL_STATE_ACTIVE);
    return (active_call_info != NULL);
}

static bool app_bt_source_call_is_exist_current_waiting_call(void)
{
    app_bt_source_call_info_t *active_call_info = app_bt_source_call_get_call_info_by_state(BT_SOURCE_SRV_CALL_STATE_WAITING);
    return (active_call_info != NULL && active_call_info->current);
}

static bool app_bt_source_call_notify_call_state(bt_source_srv_call_index_t index, bt_source_srv_call_state_t state)
{
//    if (!app_bt_source_conn_mgr_is_connected()) {
//        APPS_LOG_MSGID_E(LOG_TAG" notify_call_state, no connected", 0);
//        return FALSE;
//    }

    bt_source_srv_call_state_change_t state_change = {0};
    state_change.type = BT_SOURCE_SRV_TYPE_HFP;
    state_change.hfp_call_change.index = index;
    state_change.hfp_call_change.state = state;
    state_change.hfp_call_change.mpty = BT_SOURCE_SRV_HFP_CALL_MPTY_INACTIVE;
    bt_status_t bt_status = bt_source_srv_send_action(BT_SOURCE_SRV_ACTION_CALL_STATE_CHANGE,
                                                      &state_change, sizeof(bt_source_srv_call_state_change_t));
    APPS_LOG_MSGID_I(LOG_TAG" notify_call_state, index=%d state=%d bt_status=0x%08X",
                     3, index, state, bt_status);
    return (bt_status == BT_STATUS_SUCCESS);
}

static void app_bt_source_call_terminate_and_notify_previous_end(void)
{
    // Note: when USB_HID_SRV activate/hold/unhold current, it will end previous call, so need to notify previous END state
    for (int i = 0; i < APP_BT_SOURCE_CALL_MAX_NUM; i++) {
        app_bt_source_call_info_t *call_info = &app_bt_source_call_ctx.call_info[i];
        if (call_info->index != BT_SOURCE_SRV_CALL_INVALID_INDEX && !call_info->current) {
            bt_source_srv_call_index_t index = call_info->index;
            APPS_LOG_MSGID_I(LOG_TAG" terminate_and_notify_previous_end, [%d] index=%d state=%d",
                             3, i, index, call_info->state);
            memset(call_info, 0, sizeof(app_bt_source_call_info_t));
            app_bt_source_call_notify_call_state(index, BT_SOURCE_SRV_CALL_STATE_NONE);
        }
    }
}

static void app_bt_source_call_hold_old_and_active_waiting(void)
{
    for (int i = 0; i < APP_BT_SOURCE_CALL_MAX_NUM; i++) {
        app_bt_source_call_info_t *call_info = &app_bt_source_call_ctx.call_info[i];
        bt_source_srv_call_index_t index = call_info->index;
        if (index != BT_SOURCE_SRV_CALL_INVALID_INDEX) {
            if (call_info->state == BT_SOURCE_SRV_CALL_STATE_WAITING) {
                APPS_LOG_MSGID_I(LOG_TAG" hold_old_and_active_waiting, active [%d] index=%d waiting->activate",
                                 2, i, index);
                call_info->state = BT_SOURCE_SRV_CALL_STATE_ACTIVE;
                app_bt_source_call_notify_call_state(index, BT_SOURCE_SRV_CALL_STATE_ACTIVE);
            } else if (call_info->state == BT_SOURCE_SRV_CALL_STATE_ACTIVE) {
                APPS_LOG_MSGID_I(LOG_TAG" hold_old_and_active_waiting, hold [%d] index=%d activate->hold",
                                 2, i, index);
                call_info->state = BT_SOURCE_SRV_CALL_STATE_LOCAL_HELD;
                app_bt_source_call_notify_call_state(index, BT_SOURCE_SRV_CALL_STATE_LOCAL_HELD);
            } else if (!call_info->current) {
                APPS_LOG_MSGID_I(LOG_TAG" hold_old_and_active_waiting, terminate [%d] index=%d state=%d",
                                 3, i, index, call_info->state);
                memset(call_info, 0, sizeof(app_bt_source_call_info_t));
                app_bt_source_call_notify_call_state(index, BT_SOURCE_SRV_CALL_STATE_NONE);
            }
        }
    }
}

static void app_bt_source_call_update_new_index(bt_source_srv_call_index_t call_index)
{
    if (call_index == BT_SOURCE_SRV_CALL_INVALID_INDEX) {
        return;
    }

    bool is_exist_active_call = app_bt_source_call_is_exist_active_call();
    for (int i = 0; i < APP_BT_SOURCE_CALL_MAX_NUM; i++) {
        app_bt_source_call_info_t *call_info = &app_bt_source_call_ctx.call_info[i];
        if (call_info->index == BT_SOURCE_SRV_CALL_INVALID_INDEX) {
            call_info->index = call_index;
            call_info->state = BT_SOURCE_SRV_CALL_STATE_NONE;
            call_info->current = TRUE;

            APPS_LOG_MSGID_I(LOG_TAG" update_new_index, new call_info[%d] current=%d index=%d state=%d is_active_call=%d incoming_flag=%d dialing_flag=%d",
                             7, i, call_info->current, call_index, call_info->state, is_exist_active_call,
                             app_bt_source_call_ctx.incoming_flag, app_bt_source_call_ctx.dialing_flag);

            if (app_bt_source_call_ctx.incoming_flag) {
                if (is_exist_active_call) {
                    call_info->state = BT_SOURCE_SRV_CALL_STATE_WAITING;
                } else {
                    call_info->state = BT_SOURCE_SRV_CALL_STATE_INCOMING;
                }
                app_bt_source_call_ctx.incoming_flag = FALSE;
            } else if (app_bt_source_call_ctx.dialing_flag) {
                // Note: not notify dialing from USB_HID_SRV now.
                call_info->state = BT_SOURCE_SRV_CALL_STATE_DIALING;
                app_bt_source_call_ctx.dialing_flag = FALSE;
            } else {
                // Note: Dongle must notify dialing->alerting->active to Headset when USB_HID_SRV active
                // app_bt_source_call_notify_call_state(call_info->index, BT_SOURCE_SRV_CALL_STATE_DIALING);
                app_bt_source_call_notify_call_state(call_info->index, BT_SOURCE_SRV_CALL_STATE_ALERTING);
                call_info->state = BT_SOURCE_SRV_CALL_STATE_ACTIVE;
                if (app_bt_source_delay_hold_flag) {
                    APPS_LOG_MSGID_E(LOG_TAG" update_index, delay_hold", 0);
                    app_bt_source_delay_hold_flag = FALSE;
                    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_DELAY_HOLD);
                    ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                                        EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_DELAY_HOLD,
                                        NULL, 0, NULL, 0);
                }
            }

            app_bt_source_call_notify_call_state(call_info->index, call_info->state);
            break;
        } else {
            call_info->current = FALSE;
            APPS_LOG_MSGID_I(LOG_TAG" update_index, call_info[%d] current=%d index=%d state=%d",
                             4, i, call_info->current, call_info->index, call_info->state);
        }
    }
}

static bool app_bt_source_call_notify_new_call(app_bt_source_new_call_type_t type)
{
    const char *phone_number = "1234567";
    bt_source_srv_new_call_t new_call = {0};
    new_call.type = BT_SOURCE_SRV_TYPE_HFP;
    new_call.hfp_new_call.state = BT_SOURCE_SRV_CALL_STATE_ACTIVE;
    if (type == APP_BT_SOURCE_NEW_CALL_TYPE_INCOMING) {
        if (app_bt_source_call_is_exist_active_call()) {
            APPS_LOG_MSGID_I(LOG_TAG" notify_new_call, incoming->waiting", 0);
            new_call.hfp_new_call.state = BT_SOURCE_SRV_CALL_STATE_WAITING;
        } else {
            new_call.hfp_new_call.state = BT_SOURCE_SRV_CALL_STATE_INCOMING;
        }
    } else if (type == APP_BT_SOURCE_NEW_CALL_TYPE_DIALING) {
        new_call.hfp_new_call.state = BT_SOURCE_SRV_CALL_STATE_DIALING;
    } else {
        new_call.hfp_new_call.state = BT_SOURCE_SRV_CALL_STATE_DIALING;
    }
    new_call.hfp_new_call.mode = BT_SOURCE_SRV_HFP_CALL_MODE_VOICE;
    new_call.hfp_new_call.mpty = BT_SOURCE_SRV_HFP_CALL_MPTY_INACTIVE;
    new_call.hfp_new_call.number_length = strlen(phone_number);
    new_call.hfp_new_call.number = (uint8_t *)phone_number;
    new_call.hfp_new_call.iac = BT_SOURCE_SRV_HFP_CALL_IAC_WITHOUT;
    bt_status_t bt_status = bt_source_srv_send_action(BT_SOURCE_SRV_ACTION_NEW_CALL,
                                                      &new_call, sizeof(bt_source_srv_new_call_t));
    APPS_LOG_MSGID_I(LOG_TAG" notify_new_call, new_call_type=%d state=%d bt_status=0x%08X",
                     3, type, new_call.hfp_new_call.state, bt_status);

    if (bt_status == BT_STATUS_SUCCESS) {
        if (type == APP_BT_SOURCE_NEW_CALL_TYPE_INCOMING) {
            app_bt_source_call_ctx.incoming_flag = TRUE;
        } else if (type == APP_BT_SOURCE_NEW_CALL_TYPE_DIALING) {
            app_bt_source_call_ctx.dialing_flag = TRUE;
        }
    }
    return (bt_status == BT_STATUS_SUCCESS);
}



/**================================================================================*/
/**                               UI Shell Event Handler                           */
/**================================================================================*/
static void app_bt_source_call_proc_bt_source_group(uint32_t event_id,
                                                    void *extra_data,
                                                    size_t data_len)
{
    switch (event_id) {
        case BT_SOURCE_SRV_EVENT_CALL_INDEX_IND: {
            bt_source_srv_call_index_ind_t *call_index = (bt_source_srv_call_index_ind_t *)extra_data;
            APPS_LOG_MSGID_I(LOG_TAG" BT_SRC CALL_INDEX_IND event, new call index=%d incoming_flag=%d dialing_flag=%d",
                             3, call_index->index, app_bt_source_call_ctx.incoming_flag,
                             app_bt_source_call_ctx.dialing_flag);
            app_bt_source_call_update_new_index(call_index->index);
            break;
        }

        case BT_SOURCE_SRV_EVENT_ACCEPT_CALL: {
            bt_source_srv_accept_call_t *accept_ind = (bt_source_srv_accept_call_t *)extra_data;
            bt_source_srv_call_index_t index = accept_ind->index;
            uint8_t *addr = accept_ind->peer_address.addr;

            app_bt_source_call_info_t *call_info = app_bt_source_call_get_call_info_by_index(index);
            if (call_info == NULL) {
                APPS_LOG_MSGID_E(LOG_TAG" BT_SRC ACCEPT_CALL event, index=%d invalid call_info",
                                 1, accept_ind->index);
                break;
            }

            bt_source_srv_call_state_t state = call_info->state;
            APPS_LOG_MSGID_I(LOG_TAG" BT_SRC ACCEPT_CALL event, index=%d state=%d addr=%02X:%02X:%02X:%02X:%02X:%02X",
                             8, index, state, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
            if (state != BT_SOURCE_SRV_CALL_STATE_INCOMING) {
                APPS_LOG_MSGID_E(LOG_TAG" BT_SRC ACCEPT_CALL event, index=%d not incoming state",
                                 1, index);
                break;
            }

            bt_status_t bt_status = usb_hid_srv_send_action(USB_HID_SRV_ACTION_ACCEPT_CALL, NULL);
            APPS_LOG_MSGID_I(LOG_TAG" BT_SRC ACCEPT_CALL event, index=%d accept bt_status=0x%08X",
                             2, index, bt_status);
            break;
        }

        case BT_SOURCE_SRV_EVENT_REJECT_CALL: {
            bt_source_srv_reject_call_t *reject_ind = (bt_source_srv_reject_call_t *)extra_data;
            bt_source_srv_call_index_t index = reject_ind->index;
            uint8_t *addr = reject_ind->peer_address.addr;

            app_bt_source_call_info_t *call_info = app_bt_source_call_get_call_info_by_index(index);
            if (call_info == NULL) {
                APPS_LOG_MSGID_E(LOG_TAG" BT_SRC REJECT_CALL event, index=%d invalid call_info",
                                 1, reject_ind->index);
                break;
            }

            bt_source_srv_call_state_t state = call_info->state;
            APPS_LOG_MSGID_I(LOG_TAG" BT_SRC REJECT_CALL event, index=%d state=%d addr=%02X:%02X:%02X:%02X:%02X:%02X",
                             8, index, state, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
            if (state != BT_SOURCE_SRV_CALL_STATE_INCOMING && state != BT_SOURCE_SRV_CALL_STATE_WAITING) {
                APPS_LOG_MSGID_E(LOG_TAG" BT_SRC REJECT_CALL event, index=%d not incoming/waiting state",
                                 1, index);
                break;
            }

            bt_status_t bt_status = usb_hid_srv_send_action(USB_HID_SRV_ACTION_REJECT_CALL, NULL);
            APPS_LOG_MSGID_I(LOG_TAG" BT_SRC REJECT_CALL event, index=%d reject bt_status=0x%08X",
                             2, index, bt_status);
            if (bt_status == BT_STATUS_SUCCESS) {
                memset(call_info, 0, sizeof(app_bt_source_call_info_t));
                app_bt_source_call_notify_call_state(index, BT_SOURCE_SRV_CALL_STATE_NONE);
                app_bt_source_call_update_current_flag();
            }
            break;
        }

        case BT_SOURCE_SRV_EVENT_TERMINATE_CALL: {
            bt_source_srv_terminate_call_t *terminate_ind = (bt_source_srv_terminate_call_t *)extra_data;
            bt_source_srv_call_index_t index = terminate_ind->index;
            uint8_t *addr = terminate_ind->peer_address.addr;

            app_bt_source_call_info_t *call_info = app_bt_source_call_get_call_info_by_index(index);
            if (call_info == NULL) {
                APPS_LOG_MSGID_E(LOG_TAG" BT_SRC TERMINATE_CALL event, index=%d invalid call_info",
                                 1, terminate_ind->index);
                break;
            }

            bt_source_srv_call_state_t state = call_info->state;
            APPS_LOG_MSGID_I(LOG_TAG" BT_SRC TERMINATE_CALL event, index=%d state=%d addr=%02X:%02X:%02X:%02X:%02X:%02X",
                             8, index, state, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);

            // Note: Could terminate for any call state
            bt_status_t bt_status = usb_hid_srv_send_action(USB_HID_SRV_ACTION_TERMINATE_CALL, NULL);
            APPS_LOG_MSGID_I(LOG_TAG" BT_SRC TERMINATE_CALL event, index=%d terminate bt_status=0x%08X",
                             2, index, bt_status);
            break;
        }

        case BT_SOURCE_SRV_EVENT_DIAL_NUMBER: {
            APPS_LOG_MSGID_W(LOG_TAG" BT_SRC DIAL_NUMBER event", 0);
            break;
        }

        case BT_SOURCE_SRV_EVENT_DIAL_MEMORY: {
            APPS_LOG_MSGID_W(LOG_TAG" BT_SRC DIAL_MEMORY event", 0);
            break;
        }

        case BT_SOURCE_SRV_EVENT_DIAL_LAST_NUMBER: {
            APPS_LOG_MSGID_W(LOG_TAG" BT_SRC DIAL_LAST_NUMBER event", 0);
            // Call USB HIF create dialing call function, if successfully
            // app_bt_source_call_notify_new_call(APP_BT_SOURCE_NEW_CALL_TYPE_DIALING);
            break;
        }

        case BT_SOURCE_SRV_EVENT_VOICE_RECOGNITION_ACTIVATION: {
            APPS_LOG_MSGID_W(LOG_TAG" BT_SRC RECOGNITION_ACTIVATION event", 0);
            break;
        }

        case BT_SOURCE_SRV_EVENT_DTMF: {
            APPS_LOG_MSGID_W(LOG_TAG" BT_SRC DTMF event", 0);
            break;
        }

        case BT_SOURCE_SRV_EVENT_UNHOLD: {
            bt_source_srv_unhold_t *unhold = (bt_source_srv_unhold_t *)extra_data;
            APPS_LOG_MSGID_I(LOG_TAG" BT_SRC UNHOLD event, call_index=%d", 1, unhold->index);

            app_bt_source_call_info_t *call_info = app_bt_source_call_get_call_info_by_index(unhold->index);
            if (call_info != NULL) {
                APPS_LOG_MSGID_I(LOG_TAG" BT_SRC UNHOLD event, state=%d", 1, call_info->state);
                app_bt_source_call_set_current_flag(call_info->index);
                if (call_info->state == BT_SOURCE_SRV_CALL_STATE_LOCAL_HELD) {
                    bt_status_t bt_status = usb_hid_srv_send_action(USB_HID_SRV_ACTION_UNHOLD_CALL, NULL);
                    APPS_LOG_MSGID_I(LOG_TAG" BT_SRC UNHOLD event, bt_status=%d", 1, bt_status);
                }
            }
            break;
        }

        case BT_SOURCE_SRV_EVENT_HOLD: {
            bt_source_srv_hold_t *hold = (bt_source_srv_hold_t *)extra_data;
            APPS_LOG_MSGID_I(LOG_TAG" BT_SRC HOLD event, call_index=%d", 1, hold->index);

            app_bt_source_call_info_t *call_info = app_bt_source_call_get_call_info_by_index(hold->index);
            if (call_info != NULL) {
                APPS_LOG_MSGID_I(LOG_TAG" BT_SRC HOLD event, state=%d", 1, call_info->state);
                app_bt_source_call_set_current_flag(call_info->index);
                if (call_info->state == BT_SOURCE_SRV_CALL_STATE_ACTIVE) {
                    bt_status_t bt_status = usb_hid_srv_send_action(USB_HID_SRV_ACTION_HOLD_CALL, NULL);
                    APPS_LOG_MSGID_I(LOG_TAG" BT_SRC HOLD event, bt_status=%d", 1, bt_status);
                }
            }
            break;
        }

        default:
            break;
    }
}

static void app_bt_source_call_proc_usb_hid_call_group(uint32_t event_id,
                                                       void *extra_data,
                                                       size_t data_len)
{
    switch (event_id) {
        case USB_HID_SRV_EVENT_CALL_INCOMING: {
            APPS_LOG_MSGID_I(LOG_TAG" USB_HID_CALL INCOMING event", 0);
            app_bt_source_call_notify_new_call(APP_BT_SOURCE_NEW_CALL_TYPE_INCOMING);
            break;
        }

        case USB_HID_SRV_EVENT_CALL_ACTIVATE: {
            APPS_LOG_MSGID_I(LOG_TAG" USB_HID_CALL ACTIVATE event", 0);
            if (app_bt_source_call_get_num() == 1 && app_bt_source_call_is_exist_active_call()) {
                APPS_LOG_MSGID_W(LOG_TAG" USB_HID_CALL ACTIVATE event, exist unique active call", 0);
                break;
            }

            // Note: notify not_current ENDED (USB_HID_SRV will end other call when new call activate)
            app_bt_source_call_terminate_and_notify_previous_end();

            app_bt_source_call_info_t *incoming_call_info = app_bt_source_call_get_call_info_by_state(BT_SOURCE_SRV_CALL_STATE_INCOMING);
            app_bt_source_call_info_t *dialing_call_info = app_bt_source_call_get_call_info_by_state(BT_SOURCE_SRV_CALL_STATE_DIALING);
            if (incoming_call_info != NULL) {
                incoming_call_info->state = BT_SOURCE_SRV_CALL_STATE_ACTIVE;
                APPS_LOG_MSGID_I(LOG_TAG" USB_HID_CALL ACTIVATE event, index=%d incoming->activate",
                                 1, incoming_call_info->index);
                app_bt_source_call_notify_call_state(incoming_call_info->index, BT_SOURCE_SRV_CALL_STATE_ACTIVE);
                app_bt_source_call_set_current_flag(incoming_call_info->index);
            } else if (dialing_call_info != NULL) {
                dialing_call_info->state = BT_SOURCE_SRV_CALL_STATE_ACTIVE;
                APPS_LOG_MSGID_I(LOG_TAG" USB_HID_CALL ACTIVATE event, index=%d dialing->activate",
                                 1, dialing_call_info->index);
                app_bt_source_call_notify_call_state(dialing_call_info->index, BT_SOURCE_SRV_CALL_STATE_ACTIVE);
                app_bt_source_call_set_current_flag(dialing_call_info->index);
            } else {
                APPS_LOG_MSGID_I(LOG_TAG" USB_HID_CALL ACTIVATE event, activate", 0);
                app_bt_source_call_notify_new_call(APP_BT_SOURCE_NEW_CALL_TYPE_ACTIVATE);
            }
            break;
        }

        case USB_HID_SRV_EVENT_CALL_REMOTE_HOLD: {
            // Note: LE Audio doesn't handle REMOVE HOLD case
            APPS_LOG_MSGID_W(LOG_TAG" USB_HID_CALL REMOTE_HOLD event", 0);
            break;
        }

        case USB_HID_SRV_EVENT_CALL_HOLD: {
            if (app_bt_source_call_is_exist_active_call() && app_bt_source_call_is_exist_current_waiting_call()) {
                APPS_LOG_MSGID_W(LOG_TAG" USB_HID_CALL HOLD event, hold_old_and_active_waiting", 0);
                app_bt_source_call_hold_old_and_active_waiting();
            } else {
                APPS_LOG_MSGID_I(LOG_TAG" USB_HID_CALL HOLD event", 0);
                // Note: notify not_current ENDED (USB_HID_SRV will end other call when new call hold)
                app_bt_source_call_terminate_and_notify_previous_end();

                app_bt_source_call_info_t *call_info = app_bt_source_call_get_current_call_info();
                if (call_info != NULL && (call_info->state == BT_SOURCE_SRV_CALL_STATE_INCOMING
                                          || call_info->state == BT_SOURCE_SRV_CALL_STATE_ACTIVE)) {
                    call_info->state = BT_SOURCE_SRV_CALL_STATE_LOCAL_HELD;
                    app_bt_source_call_notify_call_state(call_info->index, BT_SOURCE_SRV_CALL_STATE_LOCAL_HELD);
                } else {
                    APPS_LOG_MSGID_E(LOG_TAG" USB_HID_CALL HOLD event, delay_hold", 0);
                    app_bt_source_delay_hold_flag = TRUE;
                }
            }
            break;
        }

        case USB_HID_SRV_EVENT_CALL_UNHOLD: {
            APPS_LOG_MSGID_I(LOG_TAG" USB_HID_CALL UNHOLD event", 0);
            // Note: notify not_current ENDED (USB_HID_SRV will end other call when new call unhold)
            app_bt_source_call_terminate_and_notify_previous_end();

            app_bt_source_call_info_t *call_info = app_bt_source_call_get_current_call_info();
            if (call_info != NULL && call_info->state == BT_SOURCE_SRV_CALL_STATE_LOCAL_HELD) {
                call_info->state = BT_SOURCE_SRV_CALL_STATE_ACTIVE;
                app_bt_source_call_notify_call_state(call_info->index, BT_SOURCE_SRV_CALL_STATE_ACTIVE);
            }
            break;
        }

        case USB_HID_SRV_EVENT_CALL_END: {
            APPS_LOG_MSGID_I(LOG_TAG" USB_HID_CALL ENDED event", 0);
            // Note: notify not_current ENDED (USB_HID_SRV will end other call when new call end)
            app_bt_source_call_terminate_and_notify_previous_end();

            app_bt_source_call_info_t *call_info = app_bt_source_call_get_current_call_info();
            if (call_info != NULL) {
                bt_source_srv_call_index_t index = call_info->index;
                memset(call_info, 0, sizeof(app_bt_source_call_info_t));
                app_bt_source_call_notify_call_state(index, BT_SOURCE_SRV_CALL_STATE_NONE);
                APPS_LOG_MSGID_I(LOG_TAG" USB_HID_CALL ENDED event, index=%d", 1, index);
            }
            break;
        }

        case USB_HID_SRV_EVENT_CALL_MIC_MUTE: {
            APPS_LOG_MSGID_I(LOG_TAG" USB_HID_CALL MIC_MUTE event", 0);
            bt_source_srv_audio_mute_t mute_param = {.port = BT_SOURCE_SRV_PORT_MIC};
            app_bt_source_send_action(BT_SOURCE_SRV_ACTION_MUTE, &mute_param, sizeof(bt_source_srv_audio_mute_t));
            break;
        }

        case USB_HID_SRV_EVENT_CALL_MIC_UNMUTE: {
            APPS_LOG_MSGID_I(LOG_TAG" USB_HID_CALL MIC_UNMUTE event", 0);
            bt_source_srv_audio_mute_t mute_param = {.port = BT_SOURCE_SRV_PORT_MIC};
            app_bt_source_send_action(BT_SOURCE_SRV_ACTION_UNMUTE, &mute_param, sizeof(bt_source_srv_audio_mute_t));
            break;
        }

        default:
            break;
    }
}

static void app_bt_source_call_proc_bt_source_app_group(uint32_t event_id,
                                                    void *extra_data,
                                                    size_t data_len)
{
    if (event_id == APP_BT_SOURCE_EVENT_DELAY_HOLD) {
        app_bt_source_call_terminate_and_notify_previous_end();

        app_bt_source_call_info_t *call_info = app_bt_source_call_get_current_call_info();
        APPS_LOG_MSGID_I(LOG_TAG" BT Source APP event, delay_hold call_info=%d", 1, (call_info != NULL));
        if (call_info != NULL && (call_info->state == BT_SOURCE_SRV_CALL_STATE_INCOMING || call_info->state == BT_SOURCE_SRV_CALL_STATE_ACTIVE)) {
            call_info->state = BT_SOURCE_SRV_CALL_STATE_LOCAL_HELD;
            app_bt_source_call_notify_call_state(call_info->index, BT_SOURCE_SRV_CALL_STATE_LOCAL_HELD);
        }
    }
}



/**================================================================================*/
/**                                     Public API                                 */
/**================================================================================*/
bool app_bt_source_call_exist_call(void)
{
    bool is_exist = FALSE;
    for (int i = 0; i < APP_BT_SOURCE_CALL_MAX_NUM; i++) {
        app_bt_source_call_info_t *call_info = &app_bt_source_call_ctx.call_info[i];
        if (call_info->state >= BT_SOURCE_SRV_CALL_STATE_INCOMING) {
            is_exist = TRUE;
            break;
        }
    }

    return is_exist;
}

bool app_bt_source_call_is_activate(void)
{
    bool is_activate = FALSE;
    for (int i = 0; i < APP_BT_SOURCE_CALL_MAX_NUM; i++) {
        app_bt_source_call_info_t *call_info = &app_bt_source_call_ctx.call_info[i];
        if (call_info->state >= BT_SOURCE_SRV_CALL_STATE_ACTIVE) {
            is_activate = TRUE;
            break;
        }
    }

    return is_activate;
}

void app_bt_source_call_switch_audio_path(bool to_hf)
{
    bt_source_srv_switch_audio_path_t param = {0};
    param.audio_transfer = (to_hf ? BT_SOURCE_SRV_AUDIO_TRANSFER_TO_HF : BT_SOURCE_SRV_AUDIO_TRANSFER_TO_AG);
    bool ret = app_bt_source_send_action(BT_SOURCE_SRV_ACTION_SWITCH_AUDIO_PATH, &param, sizeof(bt_source_srv_switch_audio_path_t));
    APPS_LOG_MSGID_W(LOG_TAG" switch_audio_path, to_hf=%d ret=%d", 2, to_hf, ret);
}

void app_bt_source_call_proc_ui_shell_event(uint32_t event_group,
                                            uint32_t event_id,
                                            void *extra_data,
                                            size_t data_len)
{
    switch (event_group) {
        case EVENT_GROUP_UI_SHELL_BT_SOURCE: {
            app_bt_source_call_proc_bt_source_group(event_id, extra_data, data_len);
            break;
        }

        case EVENT_GROUP_UI_SHELL_USB_HID_CALL: {
            app_bt_source_call_proc_usb_hid_call_group(event_id, extra_data, data_len);
            break;
        }

        case EVENT_GROUP_UI_SHELL_BT_SOURCE_APP: {
            app_bt_source_call_proc_bt_source_app_group(event_id, extra_data, data_len);
            break;
        }

        default:
            break;
    }
}

void app_bt_source_call_init(void)
{
    memset(&app_bt_source_call_ctx, 0, sizeof(app_bt_source_call_context_t));
}

#endif /* AIR_BT_SOURCE_ENABLE */
