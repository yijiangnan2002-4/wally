/* Copyright Statement:
 *
 * (C) 2021  Airoha Technology Corp. All rights reserved.
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

#ifdef AIR_LE_AUDIO_ENABLE

#include "app_le_audio_ucst.h"
#include "app_le_audio_aird.h"
#include "usbaudio_drv.h"
#include "usb_hid_srv.h"

#include "bt_gap_le.h"
#include "bt_gatts.h"

#include "bt_le_audio_msglog.h"
#include "app_le_audio_utillity.h"
#include "app_le_audio_csip_set_coordinator.h"

#ifdef AIR_MS_TEAMS_ENABLE
#include "app_ms_teams_utils.h"
#include "ms_teams.h"
#endif

#define APP_LE_AUDIO_AIRD_DEVICE_BUSY_LOCK_STREAM   APP_LE_AUDIO_UCST_LCOK_STREAM_UNIDIRECTIONAL

/* GATT releated */
#define APP_LE_AUDIO_AIRD_CCCD_VALUE_LEN    2
#define APP_LE_AUDIO_AIRD_NOTI_HDR_LEN      5
#define APP_LE_AUDIO_AIRD_NOTI_DATA_HDR_LEN 3


/************************************************
*   Attribute handle
*************************************************/
#define APP_LE_AUDIO_AIRD_START_HANDLE      (0xA401)
#define APP_LE_AUDIO_AIRD_VALUE_HANDLE_RX   (0xA403)    /**< Attribute Value Handle of Rx Characteristic. */
#define APP_LE_AUDIO_AIRD_VALUE_HANDLE_TX   (0xA405)    /**< Attribute Value Handle of Tx Characteristic. */
#define APP_LE_AUDIO_AIRD_END_HANDLE        (0xA406)

/************************************************
*   UUID
*************************************************/
#define APP_LE_AUDIO_AIRD_SERVICE_UUID   \
    {{0x41, 0x45, 0x4C, 0x61, 0x68, 0x6F, 0x72, 0x69, 0x41, 0x01, 0xAB, 0x2D, 0x4D, 0x49, 0x52, 0x50}}

#define APP_LE_AUDIO_AIRD_CHARC_UUID_RX  \
    {{0x41, 0x45, 0x4C, 0x61, 0x68, 0x6F, 0x72, 0x69, 0x41, 0x30, 0xAB, 0x2D, 0x52, 0x41, 0x48, 0x43}}

#define APP_LE_AUDIO_AIRD_CHARC_UUID_TX  \
    {{0x41, 0x45, 0x4C, 0x61, 0x68, 0x6F, 0x72, 0x69, 0x41, 0x31, 0xAB, 0x2D, 0x52, 0x41, 0x48, 0x43}}

static const bt_uuid_t g_le_audio_aird_charc_uuid_rx = APP_LE_AUDIO_AIRD_CHARC_UUID_RX;
static const bt_uuid_t g_le_audio_aird_charc_uuid_tx = APP_LE_AUDIO_AIRD_CHARC_UUID_TX;

/************************************************
*   CALLBACK
*************************************************/
static uint32_t app_le_audio_aird_charc_value_rx_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);
static uint32_t app_le_audio_aird_charc_client_config_tx_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);

/************************************************
*   SERVICE TABLE
*************************************************/
BT_GATTS_NEW_PRIMARY_SERVICE_128(g_le_audio_aird_primary_service, APP_LE_AUDIO_AIRD_SERVICE_UUID);

BT_GATTS_NEW_CHARC_128(g_le_audio_aird_char4_rx,
                       BT_GATT_CHARC_PROP_WRITE,
                       APP_LE_AUDIO_AIRD_VALUE_HANDLE_RX,
                       APP_LE_AUDIO_AIRD_CHARC_UUID_RX);
BT_GATTS_NEW_CHARC_VALUE_CALLBACK(g_le_audio_aird_charc_value_rx,
                                  g_le_audio_aird_charc_uuid_rx,
                                  BT_GATTS_REC_PERM_WRITABLE_ENCRYPTION,
                                  app_le_audio_aird_charc_value_rx_callback);

BT_GATTS_NEW_CHARC_128(g_le_audio_aird_char4_tx,
                       BT_GATT_CHARC_PROP_NOTIFY,
                       APP_LE_AUDIO_AIRD_VALUE_HANDLE_TX,
                       APP_LE_AUDIO_AIRD_CHARC_UUID_TX);
BT_GATTS_NEW_CHARC_VALUE_CALLBACK(g_le_audio_aird_charc_value_tx,
                                  g_le_audio_aird_charc_uuid_tx,
                                  0,
                                  NULL);
BT_GATTS_NEW_CLIENT_CHARC_CONFIG(g_le_audio_aird_charc_client_config_tx,
                                 BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE_ENCRYPTION,
                                 app_le_audio_aird_charc_client_config_tx_callback);

static const bt_gatts_service_rec_t *g_le_audio_aird_service_rec[] = {
    (const bt_gatts_service_rec_t *) &g_le_audio_aird_primary_service,
    (const bt_gatts_service_rec_t *) &g_le_audio_aird_char4_rx,
    (const bt_gatts_service_rec_t *) &g_le_audio_aird_charc_value_rx,
    (const bt_gatts_service_rec_t *) &g_le_audio_aird_char4_tx,
    (const bt_gatts_service_rec_t *) &g_le_audio_aird_charc_value_tx,
    (const bt_gatts_service_rec_t *) &g_le_audio_aird_charc_client_config_tx,
};

const bt_gatts_service_t ble_aird_service = {
    .starting_handle = APP_LE_AUDIO_AIRD_START_HANDLE,    /* 0xA401 */
    .ending_handle = APP_LE_AUDIO_AIRD_END_HANDLE,        /* 0xA406 */
    .required_encryption_key_size = 0,
    .records = g_le_audio_aird_service_rec,
};

typedef struct {
    uint16_t cccd_tx;
} app_le_audio_aird_info_t;

static app_le_audio_aird_info_t g_le_audio_aird_info[APP_LE_AUDIO_UCST_LINK_MAX_NUM];
static app_le_audio_aird_mode_t g_le_audio_aird_mode = APP_LE_AUDIO_AIRD_MODE_NORMAL;
static bool g_le_audio_aird_mute = false;

/**************************************************************************************************
* Prototype
**************************************************************************************************/
extern void ble_tbs_switch_notification(bt_handle_t handle,bool disable);
extern void app_le_audio_ucst_notify_mic_mute(bool mic_mute);


/************************************************
*   STATIC FUNCTIONS
*************************************************/
static app_le_audio_aird_info_t *app_le_audio_aird_get_info(bt_handle_t handle)
{
    uint8_t link_idx;

    if (APP_LE_AUDIO_UCST_LINK_IDX_INVALID == (link_idx = app_le_audio_ucst_get_link_idx(handle))) {
        return NULL;
    }

    return &g_le_audio_aird_info[link_idx];
}

static bt_status_t le_audio_send_notification(bt_handle_t handle, uint16_t att_handle, void *data, uint8_t data_len)
{
    bt_gattc_charc_value_notification_indication_t *noti;
    uint8_t *buf;
    bt_status_t ret;

    if (NULL == data) {
        //LE_AUDIO_MSGLOG_I("[APP][AIRD] send_notification, invalid data", 0);
        return BT_STATUS_FAIL;
    }
    if ((BT_HANDLE_INVALID == handle) || (BT_ATT_HANDLE_INVALID == att_handle)) {
        //LE_AUDIO_MSGLOG_I("[APP][AIRD] send_notification, invalid handle:%x att_handle:%x", 2, handle, att_handle);
        return BT_STATUS_FAIL;
    }

    if (NULL == (buf = pvPortMalloc(data_len + APP_LE_AUDIO_AIRD_NOTI_HDR_LEN))) {
        LE_AUDIO_MSGLOG_I("[APP][AIRD] send_notification, malloc fail data_len:%x", 1, data_len);
        return BT_STATUS_OUT_OF_MEMORY;
    }

    noti = (bt_gattc_charc_value_notification_indication_t *)buf;
    noti->att_req.opcode = BT_ATT_OPCODE_HANDLE_VALUE_NOTIFICATION;
    noti->att_req.handle = att_handle;
    noti->attribute_value_length = APP_LE_AUDIO_AIRD_NOTI_DATA_HDR_LEN;

    if (0 != data_len) {
        memcpy((void *)(&noti->att_req.attribute_value[0]), data, data_len);
        noti->attribute_value_length += data_len;
    }

    ret = bt_gatts_send_charc_value_notification_indication(handle, noti);

    vPortFree(buf);

    return ret;
}

static bt_status_t le_audio_notify_mode(bt_handle_t handle)
{
    app_le_audio_aird_info_t *p_info;
    app_le_audio_aird_event_mode_info_ind_t ind;

    if (NULL == (p_info = app_le_audio_aird_get_info(handle))) {
        return BT_STATUS_FAIL;
    }

    if (!(BT_ATT_CLIENT_CHARC_CONFIG_NOTIFICATION & p_info->cccd_tx)) {
        return BT_STATUS_UNSUPPORTED;
    }

    ind.evt = APP_LE_AUDIO_AIRD_EVENT_MODE_INFO;
    ind.mode = g_le_audio_aird_mode;
    return le_audio_send_notification(handle,
                                      APP_LE_AUDIO_AIRD_VALUE_HANDLE_TX,
                                      &ind,
                                      sizeof(app_le_audio_aird_event_mode_info_ind_t));
}

bt_status_t le_audio_notify_mic_mute(bt_handle_t handle, bool mic_mute)
{
    app_le_audio_aird_info_t *p_info;
    app_le_audio_aird_event_mic_mute_ind_t ind;

    g_le_audio_aird_mute = mic_mute;

    if (NULL == (p_info = app_le_audio_aird_get_info(handle))) {
        return BT_STATUS_FAIL;
    }

    if (!(BT_ATT_CLIENT_CHARC_CONFIG_NOTIFICATION & p_info->cccd_tx)) {
        return BT_STATUS_UNSUPPORTED;
    }

    ind.evt = APP_LE_AUDIO_AIRD_EVENT_MIC_MUTE;
    ind.mic_mute = mic_mute;
    return le_audio_send_notification(handle,
                                      APP_LE_AUDIO_AIRD_VALUE_HANDLE_TX,
                                      &ind,
                                      sizeof(app_le_audio_aird_event_mic_mute_ind_t));
}

static uint32_t app_le_audio_aird_charc_value_rx_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    bool is_active_device = true;
    app_le_audio_aird_info_t *p_info = NULL;
    app_le_audio_ucst_link_info_t *p_link_info = NULL;
    uint8_t *ptr;

    if ((rw != BT_GATTS_CALLBACK_WRITE) || (NULL == data) || (0 == size)) {
        return 0;
    }

    if (NULL == (p_info = app_le_audio_aird_get_info(handle))) {
        return 0;
    }

    ptr = (uint8_t *)data;

    if (APP_LE_AUDIO_AIRD_ACTION_MAX <= ptr[0]) {
        //LE_AUDIO_MSGLOG_I("[APP][AIRD] rx, invalid action:%x", 1, ptr[0]);
        return 0;
    }
    p_link_info = app_le_audio_ucst_get_link_info(handle);
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
    is_active_device = app_le_audio_ucst_is_active_group_by_handle(handle);
#endif
    LE_AUDIO_MSGLOG_I("[APP][AIRD] action:%x handle:%x is_active_device:%d", 3, ptr[0], handle, is_active_device);

    switch (ptr[0]) {
        case APP_LE_AUDIO_AIRD_ACTION_SET_STREAMING_VOLUME_UP: {
            LE_AUDIO_MSGLOG_I("[APP][AIRD] VOLUME_UP, interface:%x port:%x channel:%x volume:%x", 4, ptr[1], ptr[2], ptr[3], ptr[4]);
            if ((is_active_device == true)&&(APP_LE_AUDIO_AIRD_STREAMING_INTERFACE_SPEAKER == ptr[1])) {
                USB_HID_VolumeUp(ptr[4]);
            }
            break;
        }
        case APP_LE_AUDIO_AIRD_ACTION_SET_STREAMING_VOLUME_DOWN: {
            LE_AUDIO_MSGLOG_I("[APP][AIRD] VOLUME_DOWN, interface:%x port:%x channel:%x volume:%x", 4, ptr[1], ptr[2], ptr[3], ptr[4]);
            if ((is_active_device == true)&&(APP_LE_AUDIO_AIRD_STREAMING_INTERFACE_SPEAKER == ptr[1])) {
                USB_HID_VolumeDown(ptr[4]);
            }
            break;
        }
        case APP_LE_AUDIO_AIRD_ACTION_BLOCK_STREAM: {
            if (NULL == p_link_info) {
                //LE_AUDIO_MSGLOG_I("[APP][AIRD] BLOCK_STREAM, link not exist(handle:%x)", 1, handle);
                break;
            }

            LE_AUDIO_MSGLOG_I("[APP][AIRD] BLOCK_STREAM, handle:%x lock:%x param:%x", 3,
                              handle,
                              p_link_info->lock_stream,
                              ptr[1]);

            switch (ptr[1]) {
                case APP_LE_AUDIO_AIRD_BLOCK_STREAM_NONE: {
                    ble_tbs_switch_notification(handle, false);
                    p_link_info->lock_stream &= ~APP_LE_AUDIO_UCST_LCOK_STREAM_EARPHONE_FOTA;
                    app_le_audio_ucst_sync_lock_stream_flag(p_link_info, APP_LE_AUDIO_UCST_LCOK_STREAM_EARPHONE_FOTA, false);

                    if(is_active_device == true){
                        app_le_audio_ucst_resume();
                    }
                    break;
                }
                case APP_LE_AUDIO_AIRD_BLOCK_STREAM_ALL: {
                    /* Note: earphone send block stream when fota */
                    ble_tbs_switch_notification(handle, true);
                    p_link_info->lock_stream |= APP_LE_AUDIO_UCST_LCOK_STREAM_EARPHONE_FOTA;
                    app_le_audio_ucst_sync_lock_stream_flag(p_link_info, APP_LE_AUDIO_UCST_LCOK_STREAM_EARPHONE_FOTA, true);

                    if(is_active_device == true){
                        app_le_audio_ucst_pause();
                    }
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case APP_LE_AUDIO_AIRD_ACTION_SET_DEVICE_BUSY: {
            app_le_audio_ucst_link_info_t *p_link_info = NULL;

            if (NULL == (p_link_info = app_le_audio_ucst_get_link_info(handle))) {
                LE_AUDIO_MSGLOG_I("[APP][AIRD] DEVICE_BUSY, link not exist(handle:%x)", 1, handle);
                break;
            }

            LE_AUDIO_MSGLOG_I("[APP][AIRD] DEVICE_BUSY, handle:%x lock:%x", 2,
                              handle,
                              p_link_info->lock_stream);

            p_link_info->lock_stream |= APP_LE_AUDIO_AIRD_DEVICE_BUSY_LOCK_STREAM;
            app_le_audio_ucst_sync_lock_stream_flag(p_link_info, APP_LE_AUDIO_AIRD_DEVICE_BUSY_LOCK_STREAM, true);

            if(is_active_device == true){
                app_le_audio_ucst_pause();
            }
            break;
        }
        case APP_LE_AUDIO_AIRD_ACTION_RESET_DEVICE_BUSY: {
            app_le_audio_ucst_link_info_t *p_link_info = NULL;

            if (NULL == (p_link_info = app_le_audio_ucst_get_link_info(handle))) {
                LE_AUDIO_MSGLOG_I("[APP][AIRD] RESET_DEVICE_BUSY, link not exist(handle:%x)", 1, handle);
                break;
            }

            LE_AUDIO_MSGLOG_I("[APP][AIRD] RESET_DEVICE_BUSY, handle:%x lock:%x", 2,
                              handle,
                              p_link_info->lock_stream);

            p_link_info->lock_stream &= ~APP_LE_AUDIO_AIRD_DEVICE_BUSY_LOCK_STREAM;
            app_le_audio_ucst_sync_lock_stream_flag(p_link_info, APP_LE_AUDIO_AIRD_DEVICE_BUSY_LOCK_STREAM, false);

            if(is_active_device == true){
                app_le_audio_ucst_resume();
            }
            break;
        }
        case APP_LE_AUDIO_AIRD_ACTION_MUTE_MIC: {
            LE_AUDIO_MSGLOG_I("[APP][AIRD] MUTE_MIC", 0);
            if(is_active_device == true){
                if (size > 1) {
                    usb_hid_srv_send_action(USB_HID_SRV_ACTION_TOGGLE_MIC_MUTE, (void *)&ptr[1]);
                }
                else {
                    usb_hid_srv_send_action(USB_HID_SRV_ACTION_TOGGLE_MIC_MUTE, NULL);
                }
            }
            break;
        }
        case APP_LE_AUDIO_AIRD_ACTION_TOGGLE_PLAY: {
            LE_AUDIO_MSGLOG_I("[APP][AIRD] TOGGLE_PLAY, handle:%x", 1, handle);
            if(is_active_device == true){
                USB_HID_PlayPause();
            }
            break;
        }
        case APP_LE_AUDIO_AIRD_ACTION_PREVIOUS_TRACK: {
            LE_AUDIO_MSGLOG_I("[APP][AIRD] PREVIOUS_TRACK, handle:%x", 1, handle);
            if(is_active_device == true){
                USB_HID_ScanPreviousTrack();
            }
            break;
        }
        case APP_LE_AUDIO_AIRD_ACTION_NEXT_TRACK: {
            LE_AUDIO_MSGLOG_I("[APP][AIRD] NEXT_TRACK, handle:%x", 1, handle);
            if(is_active_device == true){
                USB_HID_ScanNextTrack();
            }
            break;
        }
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
        case APP_LE_AUDIO_AIRD_ACTION_SWITCH_DEVICE: {
            LE_AUDIO_MSGLOG_I("[APP][AIRD] SWITCH_DEVICE, cmd:%x", 1, ptr[1]);
            if (ptr[1]) {
                if (NULL == p_link_info) {
                    break;
                }
                LE_AUDIO_MSGLOG_I("[APP][AIRD] SWITCH_DEVICE, handle:%x group:%x", 2, handle, p_link_info->group_id);
                if (APP_LE_AUDIO_UCST_GROUP_ID_MAX > p_link_info->group_id) {
                    app_le_audio_ucst_set_active_group(p_link_info->group_id);
                }
            }
            break;
        }
#endif
#ifdef AIR_SILENCE_DETECTION_ENABLE
        case APP_LE_AUDIO_AIRD_ACTION_UPDATE_BREDR_CONNECTION_STATUS: {
            bool bredr_connected = FALSE;

            LE_AUDIO_MSGLOG_I("[APP][AIRD] UPDATE_BREDR_CONNECTION_STATUS, cmd:%x", 1, ptr[1]);

            if (ptr[1] == APP_LE_AUDIO_AIRD_BREDR_CONNECTION_STATUS_CONNECTED) {
                bredr_connected = TRUE;
            }

            app_le_audio_ucst_set_remote_device_bredr_connection_status(bredr_connected, handle);
            if (is_active_device) {
                app_le_audio_silence_detection_handle_event(APP_LE_AUDIO_SILENCE_DETECTION_EVENT_REMOTE_DEVICE_BREDR_STATUS_UPDATE, NULL);
            }
            break;
        }
#endif
    }

    return 0;
}

static uint32_t app_le_audio_aird_charc_client_config_tx_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    app_le_audio_aird_info_t *p_info = NULL;
    uint32_t ret = 0;

    if (BT_HANDLE_INVALID == handle) {
        return 0;
    }

    if (NULL == (p_info = app_le_audio_aird_get_info(handle))) {
        return 0;
    }

    switch (rw) {
        case BT_GATTS_CALLBACK_READ: {
            if (data != NULL) {
                uint16_t *buf = (uint16_t *)data;
                *buf = p_info->cccd_tx;
            }
            ret = sizeof(uint16_t);
            break;
        }

        case BT_GATTS_CALLBACK_WRITE: {
            if (size != sizeof(uint16_t)) {
                return 0;
            }

            p_info->cccd_tx = *(uint16_t *)data & BT_ATT_CLIENT_CHARC_CONFIG_NOTIFICATION;
            le_audio_notify_mode(handle);
            le_audio_notify_mic_mute(handle, g_le_audio_aird_mute);
            ret = sizeof(uint16_t);
            break;
        }
    }

    return ret;
}

void app_le_audio_aird_notify_volume_change(app_le_audio_aird_streaming_interface_t streaming_interface,
                                            app_le_audio_aird_streaming_port_t streaming_port,
                                            app_le_audio_aird_volume_t volume)
{
    app_le_audio_ucst_link_info_t *p_info = NULL;
    uint8_t link_idx;
    app_le_audio_aird_event_volume_change_ind_t ind = {
        .evt = APP_LE_AUDIO_AIRD_EVENT_VOLUME_CHANGE,
        .streaming_interface = streaming_interface,
        .streaming_port = streaming_port,
        .volume = volume,
    };

    for (link_idx = 0; link_idx < APP_LE_AUDIO_UCST_LINK_MAX_NUM; link_idx++) {
        if (NULL == (p_info = app_le_audio_ucst_get_link_info_by_idx(link_idx))) {
            continue;
        }
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
        if (!app_le_audio_ucst_is_active_group(p_info->group_id)) {
            continue;
        }
#endif
        le_audio_send_notification(p_info->handle,
                                   APP_LE_AUDIO_AIRD_VALUE_HANDLE_TX,
                                   &ind,
                                   sizeof(app_le_audio_aird_event_volume_change_ind_t));
    }
}

#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
bool ble_tbs_is_active_group_by_handle(bt_handle_t handle)
{
    return app_le_audio_ucst_is_active_group_by_handle(handle);
}
#endif

void app_le_audio_aird_init(void)
{
    memset(g_le_audio_aird_info, 0, sizeof(app_le_audio_aird_info_t)*APP_LE_AUDIO_UCST_LINK_MAX_NUM);

#ifdef AIR_MS_TEAMS_ENABLE
    g_le_audio_aird_mode = APP_LE_AUDIO_AIRD_MODE_SUPPORT_HID_CALL;
#else
    g_le_audio_aird_mode = APP_LE_AUDIO_AIRD_MODE_NORMAL;
#endif
}

bool app_le_audio_aird_is_connected(bt_handle_t handle)
{
    app_le_audio_aird_info_t *p_info = NULL;

    if (BT_HANDLE_INVALID == handle) {
        return false;
    }
    p_info = app_le_audio_aird_get_info(handle);

    if ((NULL != p_info) && (BT_ATT_CLIENT_CHARC_CONFIG_NOTIFICATION == p_info->cccd_tx)) {
        return true;
    }
    return false;

}
#endif  /* AIR_LE_AUDIO_ENABLE */

