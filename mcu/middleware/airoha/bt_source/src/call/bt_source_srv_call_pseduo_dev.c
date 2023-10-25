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

#include "bt_source_srv_call_pseduo_dev.h"
#include "bt_source_srv_utils.h"
#include "bt_source_srv_call_psd_manager.h"
#include "bt_source_srv_utils.h"

bt_source_srv_call_pseduo_dev_t g_source_srv_call_pseduo_dev[BT_SOURCE_SRV_CALL_PSEDUO_DEV_NUM] = {{0}};

static void bt_source_srv_call_psd_reset(bt_source_srv_call_pseduo_dev_t *device)
{
    bt_source_srv_assert(device && "reset pseduo device is NULL");

    audio_src_srv_resource_manager_handle_t *transmitter_handle = device->transmitter_audio_src;
    bt_source_srv_memset(device, 0, sizeof(bt_source_srv_call_pseduo_dev_t));
    uint32_t i = 0;
    for (i = 0; i < BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_MAX; i++) {
        device->audio_id[i] = BT_SOURCE_SRV_CALL_AUDIO_INVALID_ID;
    }
    device->transmitter_audio_src = transmitter_handle;
}

static bt_source_srv_call_pseduo_dev_t *bt_source_srv_call_psd_get_playing_device(void)
{
    for (uint32_t i = 0; i < BT_SOURCE_SRV_CALL_PSEDUO_DEV_NUM; i++) {
        bt_source_srv_call_pseduo_dev_t *call_device = (bt_source_srv_call_pseduo_dev_t *)&g_source_srv_call_pseduo_dev[i];
        if ((call_device->state == BT_SOURCE_SRV_CALL_PSD_STATE_PLAY) &&
                ((call_device->sub_state == BT_SOURCE_SRV_CALL_PSD_SUB_STATE_PLAY_STARTING) ||
                 (call_device->sub_state == BT_SOURCE_SRV_CALL_PSD_SUB_STATE_PLAYING) ||
                 ((call_device->sub_state == BT_SOURCE_SRV_CALL_PSD_SUB_STATE_PLAY_STOPPING) && (BT_SOURCE_SRV_CALL_PSD_FLAG_IS_SET(call_device, BT_SOURCE_SRV_CALL_PSD_FLAG_AUDIO_REPLAYING))))) {
            return call_device;
        }
    }
    LOG_MSGID_W(source_srv, "[AG][PSD] get playing device fail", 0);
    return NULL;
}

static void bt_source_srv_call_psd_slience_detection_callback(bt_audio_notify_event_t event)
{
    LOG_MSGID_I(source_srv, "[AG][PSD] slience detection event = %02x", 1, event);

    bt_source_srv_call_pseduo_dev_t *device = bt_source_srv_call_psd_get_playing_device();
    if (NULL == device) {
        return;
    }

    switch (event) {
        case AUDIO_BT_DONGLE_USB_DATA_SUSPEND: {
            device->user_callback((void *)device, BT_SOURCE_SRV_CALL_PSD_USER_EVENT_SLIENCE_DETECTION_SUSPEND, NULL);
        }
        break;
        case AUDIO_BT_DONGLE_USB_DATA_RESUME: {
            device->user_callback((void *)device, BT_SOURCE_SRV_CALL_PSD_USER_EVENT_SLIENCE_DETECTION_RESUME, NULL);
        }
        break;
    }
}

void bt_source_srv_call_psd_audio_transmitter_callback(audio_transmitter_event_t event, void *data, void *user_data)
{
    LOG_MSGID_I(source_srv, "[AG][PSD] audio transmitter callback event = %02x", 1, event);
    bt_source_srv_call_pseduo_dev_t *device = (bt_source_srv_call_pseduo_dev_t *)user_data;
    static uint32_t play_number = 0;
    switch (event) {
        case AUDIO_TRANSMITTER_EVENT_START_SUCCESS: {
            play_number++;
            if (play_number == device->audio_play_number) {
                if ((device->next_state == BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_INIT) && (!BT_SOURCE_SRV_CALL_PSD_FLAG_IS_SET(device, BT_SOURCE_SRV_CALL_PSD_FLAG_AUDIO_REPLAYING))) {
                    bt_source_srv_call_psd_remote_address_t psd_remote_address = {0};
                    device->user_callback((void *)device, BT_SOURCE_SRV_CALL_PSD_USER_EVENT_GET_REMOTE_ADDRESS, &psd_remote_address);
                    uint32_t gap_handle = bt_cm_get_gap_handle(psd_remote_address.remote_address);
                    bt_source_srv_call_audio_trigger_play(gap_handle);
                    bt_source_srv_call_audio_slience_detection_start(device->audio_id[BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_DL], bt_source_srv_call_psd_slience_detection_callback);
                } else {
                    LOG_MSGID_W(source_srv, "[AG][PSD] don't need trigger play next state = %02x, flag = %02x", 2, device->next_state, device->flags);
                }
                bt_source_srv_call_psd_event_notify(user_data, BT_SOURCE_SRV_CALL_PSD_EVENT_AUDIO_PLAY_IND, NULL);
            }
        }
        break;
        case AUDIO_TRANSMITTER_EVENT_STOP_SUCCESS: {
            play_number--;
            if ((play_number == 0) || ((play_number == device->audio_play_number) && (BT_SOURCE_SRV_CALL_PSD_FLAG_IS_SET(device, BT_SOURCE_SRV_CALL_PSD_FLAG_AUDIO_REPLAYING)))) {
                bt_source_srv_call_psd_event_notify(user_data, BT_SOURCE_SRV_CALL_PSD_EVENT_AUDIO_STOP_IND, NULL);
            }

        }
        break;
        default:
            break;
    }
}

bool bt_source_srv_call_psd_is_ready(void *device)
{
    bt_source_srv_assert(device && "get is ready state device is NULL");
    bt_source_srv_call_pseduo_dev_t *call_device = (bt_source_srv_call_pseduo_dev_t *)device;
    if ((call_device->state != BT_SOURCE_SRV_CALL_PSD_STATE_NONE) &&
            (call_device->sub_state != BT_SOURCE_SRV_CALL_PSD_SUB_STATE_DISCONNECTING) &&
            (call_device->next_state != BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_NONE)) {
        return true;
    }
    return false;
}

bool bt_source_srv_call_psd_is_playing(void *device)
{
    bt_source_srv_assert(device && "get is playing state device is NULL");
    bt_source_srv_call_pseduo_dev_t *call_device = (bt_source_srv_call_pseduo_dev_t *)device;
    if ((call_device->state == BT_SOURCE_SRV_CALL_PSD_STATE_PLAY) &&
            ((call_device->sub_state == BT_SOURCE_SRV_CALL_PSD_SUB_STATE_PLAY_STARTING) ||
             (call_device->sub_state == BT_SOURCE_SRV_CALL_PSD_SUB_STATE_PLAYING) ||
             ((call_device->sub_state == BT_SOURCE_SRV_CALL_PSD_SUB_STATE_PLAY_STOPPING) && (BT_SOURCE_SRV_CALL_PSD_FLAG_IS_SET(call_device, BT_SOURCE_SRV_CALL_PSD_FLAG_AUDIO_REPLAYING))))) {
        return true;
    }
    return false;
}

bool bt_source_srv_call_psd_is_connecting(void *device)
{
    bt_source_srv_assert(device && "get is connecting state device is NULL");
    bt_source_srv_call_pseduo_dev_t *call_device = (bt_source_srv_call_pseduo_dev_t *)device;

    if ((call_device->state == BT_SOURCE_SRV_CALL_PSD_STATE_NONE) &&
            (call_device->sub_state == BT_SOURCE_SRV_CALL_PSD_SUB_STATE_CONNECTING)) {
        return true;
    }
    return false;
}


bt_status_t bt_source_srv_call_psd_init(void)
{
    uint32_t i = 0;
    for (i = 0; i < BT_SOURCE_SRV_CALL_PSEDUO_DEV_NUM; i++) {
        bt_source_srv_call_pseduo_dev_t *device = &g_source_srv_call_pseduo_dev[i];
        bt_source_srv_call_psd_reset(device);
        if (bt_source_srv_call_psd_alloc_audio_src(device) != BT_STATUS_SUCCESS) {
            bt_source_srv_call_psd_reset(device);
            LOG_MSGID_E(source_srv, "[AG][PSD] alloc audio src fail index = %02x", 1, i);
            return BT_STATUS_FAIL;
        }
    }
    return BT_STATUS_SUCCESS;
}

bt_status_t bt_source_srv_call_psd_deinit(void)
{
    uint32_t i = 0;
    for (i = 0; i < BT_SOURCE_SRV_CALL_PSEDUO_DEV_NUM; i++) {
        bt_source_srv_call_pseduo_dev_t *device = &g_source_srv_call_pseduo_dev[i];
        bt_source_srv_call_psd_free_audio_src(device);
        bt_source_srv_call_psd_reset(device);
    }
    return BT_STATUS_SUCCESS;
}

void *bt_source_srv_call_psd_alloc_device(bt_bd_addr_t *bd_address, bt_source_srv_call_psd_user_callback callback)
{
    uint32_t i = 0;
    for (i = 0; i < BT_SOURCE_SRV_CALL_PSEDUO_DEV_NUM; i++) {
        bt_source_srv_call_pseduo_dev_t *device = &g_source_srv_call_pseduo_dev[i];
        uint8_t *p_bd_address = (uint8_t *)bd_address;
        if (device ->user_callback == callback) {
            LOG_MSGID_W(source_srv, "[AG][PSD] alloc device address = %02x:%02x:%02x:%02x:%02x:%02x had exist", 6, p_bd_address[0], p_bd_address[1],
                        p_bd_address[2], p_bd_address[3], p_bd_address[4], p_bd_address[5]);
            return device;
        }

        if (device ->user_callback == NULL) {
            device->user_callback = callback;
            device->speaker_volume = BT_SOURCE_SRV_CALL_AUDIO_DEFAULT_VOLUME;
            LOG_MSGID_I(source_srv, "[AG][PSD] alloc device %02x success by address = %02x:%02x:%02x:%02x:%02x:%02x", 7, device, p_bd_address[0], p_bd_address[1],
                        p_bd_address[2], p_bd_address[3], p_bd_address[4], p_bd_address[5]);
            return device;
        }
        return device;
    }
    return NULL;
}

bt_status_t bt_source_srv_call_psd_free_device(void *device)
{
    bt_source_srv_assert(device && "free device is NULL");

    bt_source_srv_call_psd_reset((bt_source_srv_call_pseduo_dev_t *)device);
    return BT_STATUS_SUCCESS;
}

void bt_source_srv_call_psd_set_codec_type(void *device, bt_source_srv_call_audio_codec_type_t codec_type)
{
    bt_source_srv_assert(device && "set codec type device is NULL");
    bt_source_srv_call_pseduo_dev_t *call_device = (bt_source_srv_call_pseduo_dev_t *)device;
    LOG_MSGID_I(source_srv, "[AG][PSD] set device:%02x codec type = %02x", 2, device, codec_type);
    call_device->codec_type = codec_type;
}

bt_source_srv_call_audio_codec_type_t bt_source_srv_call_psd_get_codec_type(void *device)
{
    bt_source_srv_assert(device && "get codec type device is NULL");
    bt_source_srv_call_pseduo_dev_t *call_device = (bt_source_srv_call_pseduo_dev_t *)device;
    LOG_MSGID_I(source_srv, "[AG][PSD] get device:%02x codec type = %02x", 2, device, call_device->codec_type);
    return call_device->codec_type;
}

void bt_source_srv_call_psd_set_speaker_volume(void *device, bt_source_srv_call_audio_volume_t volume)
{
    bt_source_srv_assert(device && "set speaker volume device is NULL");

    bt_source_srv_call_pseduo_dev_t *call_device = (bt_source_srv_call_pseduo_dev_t *)device;
    call_device->speaker_volume = volume;
    LOG_MSGID_I(source_srv, "[AG][PSD] set device = %02x speaker volume = %02x", 2, device, volume);
}

bt_source_srv_call_audio_volume_t bt_source_srv_call_psd_get_speaker_volume(void *device)
{
    bt_source_srv_assert(device && "set speaker volume device is NULL");

    bt_source_srv_call_pseduo_dev_t *call_device = (bt_source_srv_call_pseduo_dev_t *)device;
    return call_device->speaker_volume;
}

bt_status_t bt_source_srv_call_psd_audio_mute(void *device, bt_source_srv_call_audio_play_t play_type)
{
    bt_source_srv_assert(device && "mute audio device is NULL");
    bt_source_srv_call_pseduo_dev_t *call_device = (bt_source_srv_call_pseduo_dev_t *)device;
    return bt_source_src_call_audio_mute(call_device->audio_id[play_type]);
}

bt_status_t bt_source_srv_call_psd_audio_unmute(void *device, bt_source_srv_call_audio_play_t play_type)
{
    bt_source_srv_assert(device && "unmute audio device is NULL");
    bt_source_srv_call_pseduo_dev_t *call_device = (bt_source_srv_call_pseduo_dev_t *)device;
    return bt_source_src_call_audio_unmute(call_device->audio_id[play_type]);
}

void bt_source_srv_call_psd_event_notify(void *device, bt_source_srv_call_psd_event_t event, void *parameter)
{
    bt_source_srv_mutex_lock();
    bt_source_srv_call_psd_state_machine((bt_source_srv_call_pseduo_dev_t *)device, event, parameter);
    bt_source_srv_mutex_unlock();
}