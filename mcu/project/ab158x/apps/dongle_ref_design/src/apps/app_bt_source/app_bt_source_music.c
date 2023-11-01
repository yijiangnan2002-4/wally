
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
 * File: app_bt_source_music.c
 *
 * Description: This file provides API for BT Source Music.
 *
 */

#ifdef AIR_BT_SOURCE_ENABLE

#include "app_bt_source_music.h"

#include "app_bt_source_call.h"
#include "app_bt_source_conn_mgr.h"

#include "apps_debug.h"
#include "apps_events_event_group.h"
#include "bt_source_srv.h"
#include "usbaudio_drv.h"
#include "ui_shell_manager.h"



#define LOG_TAG             "[BT_SRC][MUSIC]"



#define APP_BT_SOURCE_MUSIC_SUSPEND_TIMER       (3 * 1000)

typedef struct {
    bt_addr_t                           peer_address;
    bool                                is_streaming;
} PACKED app_bt_source_music_context_t;

static app_bt_source_music_context_t                app_bt_source_music_ctx = {0};

/**================================================================================*/
/**                                   Internal API                                 */
/**================================================================================*/
extern bool app_bt_source_send_action(bt_source_srv_action_t action, void *parameter, uint32_t length);

static bool app_bt_source_music_start_streaming_imp(bool start)
{
    bool success = FALSE;
    bt_source_srv_music_action_t music_action = {0};
    music_action.peer_address.type = BT_ADDR_PUBLIC;
    uint8_t *addr = app_bt_source_conn_mgr_get_active_device();
    memcpy(music_action.peer_address.addr, addr, BT_BD_ADDR_LEN);

    if (start) {
        success = app_bt_source_send_action(BT_SOURCE_SRV_ACTION_START_STREAM, &music_action, sizeof(music_action));
    } else {
        success = app_bt_source_send_action(BT_SOURCE_SRV_ACTION_SUSPEND_STREAM, &music_action, sizeof(music_action));
    }
    APPS_LOG_MSGID_I(LOG_TAG" music_streaming, start=%d success=%d", 2, start, success);
    return success;
}

static bool app_bt_source_music_remote_control(bt_addr_t *addr, uint32_t operation, uint8_t *data, uint8_t len)
{
    bool accept = FALSE;
    USB_HID_Status_t status = USB_HID_IS_NOT_READY;
    bt_source_srv_music_action_t music_action = {0};
    if (app_bt_source_call_is_activate()) {
        APPS_LOG_MSGID_E(LOG_TAG" remote_control, call activate", 0);
        goto exit;
    }

    switch (operation) {
        case BT_SOURCE_SRV_ACTION_PLAY: {
            status = USB_HID_PlayPause();
            APPS_LOG_MSGID_I(LOG_TAG" remote_control, PLAY streaming=%d hid_status=%d",
                             2, app_bt_source_music_ctx.is_streaming, status);
            break;
        }

        case BT_SOURCE_SRV_ACTION_PAUSE: {
            status = USB_HID_PlayPause();
            APPS_LOG_MSGID_I(LOG_TAG" remote_control, PAUSE streaming=%d hid_status=%d",
                             2, app_bt_source_music_ctx.is_streaming, status);
            break;
        }

        case BT_SOURCE_SRV_ACTION_NEXT: {
            status = USB_HID_ScanNextTrack();
            APPS_LOG_MSGID_I(LOG_TAG" remote_control, NEXT hid_status=%d", 1, status);
            break;
        }

        case BT_SOURCE_SRV_ACTION_PREVIOUS: {
            status = USB_HID_ScanPreviousTrack();
            APPS_LOG_MSGID_I(LOG_TAG" remote_control, PREVIOUS hid_status=%d", 1, status);
            break;
        }
    }

    accept = (status == USB_HID_STATUS_OK);

exit:
    // Send response to remote device
    memcpy(&music_action.peer_address, addr, sizeof(bt_addr_t));
    music_action.data = (void *)&accept;
    bt_status_t bt_status = bt_source_srv_send_action(operation, &music_action, sizeof(music_action));
    if (bt_status != BT_STATUS_SUCCESS) {
        APPS_LOG_MSGID_E(LOG_TAG" remote_control, error operation=0x%08X bt_status=0x%08X",
                         2, operation, bt_status);
    }

    return (accept && bt_status == BT_STATUS_SUCCESS);
}



/**================================================================================*/
/**                               UI Shell Event Handler                           */
/**================================================================================*/
static void app_bt_source_music_proc_bt_source_group(uint32_t event_id,
                                                     void *extra_data,
                                                     size_t data_len)
{
    switch (event_id) {
        case BT_SOURCE_SRV_EVENT_MUSIC_STREAM_OPERATION_CNF: {
            bt_source_srv_music_stream_operation_cnf_t *stream_cnf = (bt_source_srv_music_stream_operation_cnf_t *)extra_data;
            if (stream_cnf == NULL) {
                break;
            }

            bt_addr_t *peer_addr = &stream_cnf->peer_address;
            uint8_t *addr = (uint8_t *)peer_addr->addr;
            bt_status_t bt_status = stream_cnf->status;
            bt_srv_music_operation_t operation = stream_cnf->operation;

            if (bt_status == BT_STATUS_SUCCESS) {
                if (operation == BT_SOURCE_SRV_ACTION_START_STREAM) {
                    app_bt_source_music_ctx.is_streaming = TRUE;
                } else if (operation == BT_SOURCE_SRV_ACTION_SUSPEND_STREAM) {
                    app_bt_source_music_ctx.is_streaming = FALSE;
                }
            }
            APPS_LOG_MSGID_I(LOG_TAG" STREAM_OPERATION_CNF, status=0x%08X addr=%02X:%02X:%02X:%02X:%02X:XX operation=0x%08X is_streaming=%d",
                             8, bt_status, addr[5], addr[4], addr[3], addr[2], addr[1], operation, app_bt_source_music_ctx.is_streaming);
            break;
        }

        case BT_SOURCE_SRV_EVENT_MUSIC_CONTROL_OPERATION_IND: {
            bt_source_srv_music_control_operation_ind_t *control_ind = (bt_source_srv_music_control_operation_ind_t *)extra_data;
            if (control_ind == NULL) {
                break;
            }

            app_bt_source_music_remote_control(&control_ind->peer_address, control_ind->operation,
                                               control_ind->data, control_ind->length);
            break;
        }

        case BT_SOURCE_SRV_EVENT_MUSIC_STREAM_OPERATION_IND: {
            bt_source_srv_music_stream_operation_ind_t *stream_ind = (bt_source_srv_music_stream_operation_ind_t *)extra_data;
            if (stream_ind == NULL) {
                break;
            }

            bt_addr_t *peer_addr = &stream_ind->peer_address;
            uint8_t *addr = (uint8_t *)peer_addr->addr;
            bt_srv_music_operation_t operation = stream_ind->operation;
            bool accept = !app_bt_source_call_is_activate();
            APPS_LOG_MSGID_I(LOG_TAG" STREAM_OPERAION_IND, addr=%02X:%02X:%02X:%02X:%02X:%02X start_stream=%d accept=%d",
                             8, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0],
                             (operation == BT_SOURCE_SRV_ACTION_START_STREAM), accept);

            if (operation == BT_SOURCE_SRV_ACTION_START_STREAM || operation == BT_SOURCE_SRV_ACTION_SUSPEND_STREAM) {
                memcpy(&app_bt_source_music_ctx.peer_address, peer_addr, sizeof(bt_addr_t));

                bt_source_srv_music_action_t music_action = {0};
                memcpy(&music_action.peer_address, peer_addr, sizeof(bt_addr_t));
                music_action.data = (void *)&accept;
                bt_status_t bt_status = bt_source_srv_send_action((operation == BT_SOURCE_SRV_ACTION_START_STREAM ? BT_SOURCE_SRV_ACTION_START_STREAM_RESPONSE : BT_SOURCE_SRV_ACTION_SUSPEND_STREAM_RESPONSE),
                                                                  &music_action, sizeof(music_action));
                APPS_LOG_MSGID_I(LOG_TAG" STREAM_OPERAION_IND, accept=%d bt_status=0x%08X",
                                 2, accept, bt_status);
            }
            break;
        }

        case BT_SOURCE_SRV_EVENT_MUSIC_DETECT_MEDIA_DATA_IND: {
            bt_status_t bt_status = BT_STATUS_FAIL;
            bt_source_srv_music_detect_media_data_ind_t *detect_ind = (bt_source_srv_music_detect_media_data_ind_t *)extra_data;
            bt_addr_t *peer_addr = &detect_ind->peer_address;
            uint8_t *addr = (uint8_t *)peer_addr->addr;
            uint8_t event = detect_ind->event;
            bt_source_srv_music_data_detect_t data_detect = 1;
            APPS_LOG_MSGID_I(LOG_TAG" DETECT_MEDIA_DATA_IND, addr=%02X:%02X:%02X:%02X:%02X:%02X event=%d",
                             7, addr[5], addr[4], addr[3], addr[2], addr[1], addr[0], event);

            if (event == BT_SOURCE_SRV_MUSIC_DATA_SUSPEND || event == BT_SOURCE_SRV_MUSIC_DATA_RESUME) {
                memcpy(&app_bt_source_music_ctx.peer_address, peer_addr, sizeof(bt_addr_t));

                bt_source_srv_music_action_t music_action = {0};
                if (event == BT_SOURCE_SRV_MUSIC_DATA_SUSPEND) {
                    music_action.data = &data_detect;
                }
                memcpy(&music_action.peer_address, peer_addr, sizeof(bt_addr_t));
                bt_status = bt_source_srv_send_action((event == BT_SOURCE_SRV_MUSIC_DATA_SUSPEND ? BT_SOURCE_SRV_ACTION_SUSPEND_STREAM : BT_SOURCE_SRV_ACTION_START_STREAM),
                                                      &music_action, sizeof(music_action));
            }
            APPS_LOG_MSGID_I(LOG_TAG" DETECT_MEDIA_DATA_IND, event=%d bt_status=0x%08X", 2, event, bt_status);
            break;
        }

        default:
            break;
    }
}

static void app_bt_source_music_proc_bt_source_app_group(uint32_t event_id,
                                                         void *extra_data,
                                                         size_t data_len)
{
    if (event_id == APP_BT_SOURCE_EVENT_MUSIC_SUSPEND_STREAM_TIMER) {
        app_bt_source_music_start_streaming_imp(FALSE);
    } else if (event_id == APP_BT_SOURCE_EVENT_NOTIFY_CONN_DISCONNECTED) {
        app_bt_source_music_ctx.is_streaming = FALSE;
    }
}



/**================================================================================*/
/**                                     Public API                                 */
/**================================================================================*/
bool app_bt_source_music_start_streaming(bool start, bool need_delay)
{
    bool success = FALSE;
    if (start) {
        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_MUSIC_SUSPEND_STREAM_TIMER);
        success = app_bt_source_music_start_streaming_imp(TRUE);
    } else {
        if (need_delay) {
            ui_shell_remove_event(EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_MUSIC_SUSPEND_STREAM_TIMER);
            ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                                EVENT_GROUP_UI_SHELL_BT_SOURCE_APP, APP_BT_SOURCE_EVENT_MUSIC_SUSPEND_STREAM_TIMER,
                                NULL, 0, NULL, APP_BT_SOURCE_MUSIC_SUSPEND_TIMER);
            success = TRUE;
            APPS_LOG_MSGID_I(LOG_TAG" music_streaming, suspend event delay=%d", 1, APP_BT_SOURCE_MUSIC_SUSPEND_TIMER);
        } else {
            success = app_bt_source_music_start_streaming_imp(FALSE);
        }
    }
    return success;
}

void app_bt_source_music_proc_ui_shell_event(uint32_t event_group,
                                             uint32_t event_id,
                                             void *extra_data,
                                             size_t data_len)
{
    switch (event_group) {
        case EVENT_GROUP_UI_SHELL_BT_SOURCE: {
            app_bt_source_music_proc_bt_source_group(event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_BT_SOURCE_APP: {
            app_bt_source_music_proc_bt_source_app_group(event_id, extra_data, data_len);
            break;
        }
        default:
            break;
    }
}

void app_bt_source_music_init(void)
{
    memset(&app_bt_source_music_ctx, 0, sizeof(app_bt_source_music_context_t));
}

#endif /* AIR_BT_SOURCE_ENABLE */
