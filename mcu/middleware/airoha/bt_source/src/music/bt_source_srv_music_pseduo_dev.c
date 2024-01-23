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

#include "bt_source_srv_music_pseduo_dev.h"
#include "bt_source_srv_music_psd_manager.h"
#include "bt_source_srv_utils.h"
#include "bt_source_srv_a2dp.h"

bt_source_srv_music_pseduo_dev_t g_source_srv_music_pseduo_dev[BT_SOURCE_SRV_MUSIC_PSEDUO_DEV_NUM] = {{0}};
//static void bt_source_srv_music_psd_am_music_back(bt_source_srv_music_audio_id_t audio_id, bt_sink_srv_am_cb_msg_class_t msg_id,
        //bt_sink_srv_am_cb_sub_msg_t sub_msg, void *parameter);
static void bt_source_srv_music_psd_reset(bt_source_srv_music_pseduo_dev_t *device)
{
    bt_source_srv_assert(device && "reset pseduo device is NULL");

    audio_src_srv_resource_manager_handle_t *speaker_handle = device->speaker_audio_src;
    bt_source_srv_memset(device, 0, sizeof(bt_source_srv_music_pseduo_dev_t));
    uint32_t i = 0;
    for (i = 0; i < BT_SOURCE_SRV_MUSIC_AUDIO_PLAY_TYPE_MAX; i++) {
        device->audio_id[i] = BT_SOURCE_SRV_MUSIC_AUDIO_INVALID_ID;
    }
    device->speaker_audio_src = speaker_handle;
}

#if 0
static uint64_t bt_source_srv_music_psd_convert_btaddr_to_devid(bt_bd_addr_t *bd_addr)
{
    uint64_t dev_id = 0;
    uint32_t hdev = 0, ldev = 0;
    int32_t i = 0;
    uint8_t addr[16] = {0};

    bt_source_srv_assert(bd_addr);
    bt_source_srv_memcpy(addr, bd_addr, sizeof(bt_bd_addr_t));
    for (i = 0; i < BT_BD_ADDR_LEN; ++i) {
        dev_id = ((dev_id << 8) | (addr[i]));
    }

    hdev = (dev_id >> 32 & 0xffffffff);
    ldev = (dev_id & 0xffffffff);
    LOG_MSGID_I(source_srv, "[AG][PSD] convert_btaddr_to_devid--0x%x, ldev: 0x%x", 2, hdev, ldev);
    return dev_id;
}


static bt_source_srv_music_pseduo_dev_t *bt_source_srv_music_psd_find_device_by_audio_id(bt_source_srv_music_audio_id_t audio_id)
{
    uint32_t i = 0;
    for (i = 0; i < BT_SOURCE_SRV_MUSIC_PSEDUO_DEV_NUM; i++) {
        bt_source_srv_music_pseduo_dev_t *device = &g_source_srv_music_pseduo_dev[i];
        if (device->audio_id == audio_id) {
            return device;
        }
    }
    LOG_MSGID_W(source_srv, "[AG][PSD] find device by audio id = %02x fail", 1, audio_id);
    return NULL;
}


static void bt_source_srv_music_psd_am_music_back(bt_source_srv_music_audio_id_t audio_id, bt_sink_srv_am_cb_msg_class_t msg_id,
        bt_sink_srv_am_cb_sub_msg_t sub_msg, void *parameter)
{
    LOG_MSGID_I(source_srv, "[AG][PSD] am musicback audio_id = %02x, msg = %02x, sub_msg = %02x", 3, audio_id, msg_id, sub_msg);
    if (msg_id == AUD_SELF_CMD_REQ) {
        bt_source_srv_music_pseduo_dev_t *device = bt_source_srv_music_psd_find_device_by_audio_id(audio_id);
        if (device == NULL) {
            return;
        }

        switch (sub_msg) {
            case AUD_HFP_PLAY_OK: {
                bt_bd_addr_t address = {0};
                bt_source_srv_music_psd_convert_devid_to_btaddr(device->speaker_audio_src->dev_id, &address);
                uint32_t gap_handle = bt_cm_get_gap_handle(address);
                LOG_MSGID_I(source_srv, "[AG][PSD] play ok get gap handle = %02x", 1, audio_id);
                if (device->next_state == BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_INIT) {
                    bt_source_srv_music_audio_trigger_play(gap_handle);
                } else {
                    LOG_MSGID_I(source_srv, "[AG][PSD] don't need trigger play next state = %02x", 1, device->next_state);
                }
                bt_source_srv_music_psd_event_notify(device, BT_SOURCE_SRV_CALL_PSD_EVENT_PLAY_CODEC_IND, NULL);
            }
            break;
            case AUD_CMD_COMPLETE: {
                if (device->sub_state == BT_SOURCE_SRV_CALL_PSD_SUB_STATE_CODEC_STARTING) {
                    bt_source_srv_music_psd_event_notify(device, BT_SOURCE_SRV_CALL_PSD_EVENT_PLAY_CODEC_IND, NULL);
                } else if (device->sub_state == BT_SOURCE_SRV_CALL_PSD_SUB_STATE_CODEC_STOPPING) {
                    bt_source_srv_music_psd_event_notify(device, BT_SOURCE_SRV_CALL_PSD_EVENT_STOP_CODEC_IND, NULL);
                }
            }
            break;
            default:
                break;
        }
    }
}


bool bt_source_srv_music_psd_is_ready(void *device)
{
    bt_source_srv_assert(device && "get is ready state device is NULL");
    bt_source_srv_music_pseduo_dev_t *music_device = device;
    if ((music_device->state != BT_SOURCE_SRV_MUSIC_PSD_STATE_NONE) &&
            (music_device->sub_state != BT_SOURCE_SRV_MUSIC_PSD_SUB_STATE_DISCONNECTING) &&
            (music_device->next_state != BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_NONE)) {
        return true;
    }
    return false;
}

/* this function is workaround for sink & source audio source compare */
bool bt_source_srv_music_psd_is_exist_by_audio_handle(audio_src_srv_handle_t *handle)
{
    bt_source_srv_assert(handle && "get is exist handle is NULL");
    uint32_t i = 0;
    for (i = 0; i < BT_SOURCE_SRV_MUSIC_PSEDUO_DEV_NUM; i++) {
        bt_source_srv_music_pseduo_dev_t *device = &g_source_srv_music_pseduo_dev[i];
        if (device->speaker_audio_src == handle) {
            return true;
        }
    }
    return false;
}
#endif

bt_status_t bt_source_srv_music_psd_init(void)
{
    uint32_t i = 0;
    for (i = 0; i < BT_SOURCE_SRV_MUSIC_PSEDUO_DEV_NUM; i++) {
        bt_source_srv_music_pseduo_dev_t *device = &g_source_srv_music_pseduo_dev[i];
        bt_source_srv_music_psd_reset(device);
        if (bt_source_srv_music_psd_alloc_audio_src(device) != BT_STATUS_SUCCESS) {
            bt_source_srv_music_psd_reset(device);
            LOG_MSGID_E(source_srv, "[AG][PSD] alloc audio src fail index = %02x", 1, i);
            return BT_STATUS_FAIL;
        }
    }
    return BT_STATUS_SUCCESS;
}

bt_status_t bt_source_srv_music_psd_deinit(void)
{
    uint32_t i = 0;
    for (i = 0; i < BT_SOURCE_SRV_MUSIC_PSEDUO_DEV_NUM; i++) {
        bt_source_srv_music_pseduo_dev_t *device = &g_source_srv_music_pseduo_dev[i];
        bt_source_srv_music_psd_free_audio_src(device);
        bt_source_srv_music_psd_reset(device);
    }
    return BT_STATUS_SUCCESS;
}

void *bt_source_srv_music_psd_alloc_device(bt_bd_addr_t *bd_address, bt_source_srv_music_psd_user_musicback musicback)
{
    uint32_t i = 0;
    for (i = 0; i < BT_SOURCE_SRV_MUSIC_PSEDUO_DEV_NUM; i++) {
        bt_source_srv_music_pseduo_dev_t *device = &g_source_srv_music_pseduo_dev[i];
        uint8_t *p_bd_address = (uint8_t *)bd_address;
        if (device->user_musicback == musicback) {
            LOG_MSGID_W(source_srv, "[MUSIC][PSD] alloc device address = %02x:%02x:%02x:%02x:%02x:%02x had exist", 6, p_bd_address[0], p_bd_address[1],
                        p_bd_address[2], p_bd_address[3], p_bd_address[4], p_bd_address[5]);
            return device;
        }

        if (device ->user_musicback == NULL) {
            device->user_musicback = musicback;
            device->speaker_volume = BT_SOURCE_SRV_MUSIC_AUDIO_DEFAULT_VOLUME;
            LOG_MSGID_I(source_srv, "[MUSIC][PSD] alloc device %02x success by address = %02x:%02x:%02x:%02x:%02x:%02x", 7, device, p_bd_address[0], p_bd_address[1],
                        p_bd_address[2], p_bd_address[3], p_bd_address[4], p_bd_address[5]);
            return device;
        }
    }

    return NULL;
}

bt_status_t bt_source_srv_music_psd_free_device(void *device)
{
    LOG_MSGID_E(source_srv, "[MUSIC][PSD] free device:0x%x", 1, device);
    if (device) {
        bt_source_srv_music_psd_reset((bt_source_srv_music_pseduo_dev_t *)device);

    }

    return BT_STATUS_SUCCESS;
}

void bt_source_srv_music_psd_set_codec_type(void *device, bt_source_srv_music_audio_codec_type_t codec_type)
{
    bt_source_srv_assert(device && "set codec type device is NULL");
    bt_source_srv_music_pseduo_dev_t *music_device = (bt_source_srv_music_pseduo_dev_t *)device;
    LOG_MSGID_I(source_srv, "[MUSIC][PSD] set device:%02x codec type = %02x", 2, device, codec_type);
    music_device->codec_type = codec_type;
}

void bt_source_srv_music_psd_set_speaker_volume(void *device, bt_source_srv_music_audio_volume_t volume)
{
    bt_source_srv_assert(device && "set speaker volume device is NULL");

    bt_source_srv_music_pseduo_dev_t *music_device = (bt_source_srv_music_pseduo_dev_t *)device;
    music_device->speaker_volume = volume;
    LOG_MSGID_I(source_srv, "[MUSIC][PSD] set device = %02x speaker volume = %02x, port = %02x", 3, device, volume, music_device->port);
    if (music_device->codec_type != BT_HFP_CODEC_TYPE_NONE) {
        bt_sink_srv_ami_audio_set_volume(music_device->audio_id[music_device->port], music_device->speaker_volume, STREAM_OUT);
    }
}

bt_source_srv_music_audio_volume_t bt_source_srv_music_psd_get_speaker_volume(void *device)
{
    bt_source_srv_assert(device && "set speaker volume device is NULL");

    bt_source_srv_music_pseduo_dev_t *music_device = (bt_source_srv_music_pseduo_dev_t *)device;
    return music_device->speaker_volume;
}

bt_status_t bt_source_srv_music_audio_port_update(bt_source_srv_port_t audio_port, bt_source_srv_music_port_action_t action)
{
    bt_status_t status = BT_STATUS_FAIL;
    if (audio_port == BT_SOURCE_SRV_PORT_MIC || audio_port == BT_SOURCE_SRV_PORT_GAMING_SPEAKER) {
        LOG_MSGID_E(source_srv, "[MUSIC][PSD]  bt_source_srv_music_audio_port_update: invaild = %x", 1, audio_port);
        return BT_STATUS_FAIL;
    }

    status = bt_source_srv_a2dp_audio_port_update(audio_port, action);
    return status;
}

bool bt_source_srv_music_psd_is_playing(void *device)
{
    bt_source_srv_assert(device && "get is playing state device is NULL");
    bt_source_srv_music_pseduo_dev_t *music_device = (bt_source_srv_music_pseduo_dev_t *)device;
    LOG_MSGID_I(source_srv, "[MUSIC][PSD]music_psd_is_playing, state = %02x, sub_state = %02x", 2, music_device->state, music_device->sub_state);
    if ((music_device->state == BT_SOURCE_SRV_MUSIC_PSD_STATE_PLAY) &&
    (music_device->sub_state ==BT_SOURCE_SRV_MUSIC_PSD_SUB_STATE_CODEC_STARTING || music_device->sub_state == BT_SOURCE_SRV_MUSIC_PSD_SUB_STATE_PLAYING)) {
        return true;
    }
    return false;
}


void bt_source_srv_music_psd_update_port(void *device, bt_source_srv_port_t port)
{
    bt_source_srv_assert(device && "get is playing state device is NULL");

    bt_source_srv_music_pseduo_dev_t *music_device = (bt_source_srv_music_pseduo_dev_t *)device;
    if (music_device) {
        music_device->port = port;
    }
}


bool bt_source_srv_music_psd_is_prepare_play(void *device)
{
    bt_source_srv_assert(device && "get is playing state device is NULL");
    bt_source_srv_music_pseduo_dev_t *music_device = (bt_source_srv_music_pseduo_dev_t *)device;
    LOG_MSGID_I(source_srv, "[MUSIC][PSD]music_psd_is_playing, state = %02x, sub_state = %02x", 2, music_device->state, music_device->sub_state);
    if ((music_device->state == BT_SOURCE_SRV_MUSIC_PSD_STATE_READY) && music_device->sub_state == BT_SOURCE_SRV_MUSIC_PSD_STATE_TAKE_AUDIO_SRC) {
        return true;
    }
    return false;
}

bool bt_source_srv_music_psd_is_idle(void *device)
{
    bt_source_srv_assert(device && "get is playing state device is NULL");
    bt_source_srv_music_pseduo_dev_t *music_device = (bt_source_srv_music_pseduo_dev_t *)device;
    LOG_MSGID_I(source_srv, "[MUSIC][PSD]music_psd_is_playing, state = %02x, sub_state = %02x", 2, music_device->state, music_device->sub_state);
    if ((music_device->state == BT_SOURCE_SRV_MUSIC_PSD_STATE_READY) && music_device->sub_state == BT_SOURCE_SRV_MUSIC_PSD_SUB_STATE_NONE) {
        return true;
    }
    return false;
}


uint32_t bt_source_srv_music_get_audio_number(void *device)
{
    uint32_t num = 0;
    bt_source_srv_assert(device && "get is playing state device is NULL");
    bt_source_srv_music_pseduo_dev_t *music_device = (bt_source_srv_music_pseduo_dev_t *)device;

    if (music_device) {
        num = music_device->audio_play_num;
    }
    LOG_MSGID_I(source_srv, "[MUSIC][PSD]bt_source_srv_music_get_audio_number,number =%x",1, num);
    return num;
}
