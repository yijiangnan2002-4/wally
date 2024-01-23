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
#include "bt_source_srv_utils.h"
#include "bt_source_srv_music_psd_manager.h"
#include "bt_timer_external.h"

/* waiting list API */
static void bt_source_srv_music_psd_add_waiting_list(bt_source_srv_music_psd_audio_source_t type, bt_source_srv_music_pseduo_dev_t *device);
static void bt_source_srv_music_psd_del_waiting_list(bt_source_srv_music_psd_audio_source_t type, bt_source_srv_music_pseduo_dev_t *device);
/* audio resource event handle API */
static void bt_source_srv_music_psd_audio_src_take_success_handle(bt_source_srv_music_pseduo_dev_t *device);
static void bt_source_srv_music_psd_audio_src_take_reject_handle(bt_source_srv_music_pseduo_dev_t *device);
static void bt_source_srv_music_psd_audio_src_give_success_handle(bt_source_srv_music_pseduo_dev_t *device);
static void bt_source_srv_music_psd_audio_src_suspend_handle(bt_source_srv_music_pseduo_dev_t *device);
/* state machine handle API */
static bt_status_t bt_source_srv_music_psd_state_none_handle(bt_source_srv_music_pseduo_dev_t *device, bt_source_srv_music_psd_event_t event, void *parameter);
static bt_status_t bt_source_srv_music_psd_state_ready_handle(bt_source_srv_music_pseduo_dev_t *device, bt_source_srv_music_psd_event_t event, void *parameter);
static bt_status_t bt_source_srv_music_psd_state_play_handle(bt_source_srv_music_pseduo_dev_t *device, bt_source_srv_music_psd_event_t event, void *parameter);
//static bt_status_t bt_source_srv_music_psd_state_give_audio_src_handle(bt_source_srv_music_pseduo_dev_t *device, bt_source_srv_music_psd_event_t event, void *parameter);
/* state machine run next state API */
static void bt_source_srv_music_psd_run_next_state_with_state_ready(bt_source_srv_music_pseduo_dev_t *device);
static void bt_source_srv_music_psd_run_next_state_with_codec_stoping_handle(bt_source_srv_music_pseduo_dev_t *device);
static void bt_source_srv_music_psd_run_next_state_with_codec_starting_handle(bt_source_srv_music_pseduo_dev_t *device);

static bt_status_t bt_source_srv_music_psd_play(bt_source_srv_music_pseduo_dev_t * device);
static bt_status_t bt_source_srv_music_psd_replay(bt_source_srv_music_pseduo_dev_t*device,bt_source_srv_port_t port);

static void bt_source_srv_music_psd_deinit_all_audio(bt_source_srv_music_pseduo_dev_t *device);

static bt_source_srv_music_pseduo_dev_t *bt_source_srv_music_psd_find_device_by_handle(bt_source_srv_music_psd_audio_source_t type, void *handle);
extern bt_source_srv_music_pseduo_dev_t g_source_srv_music_pseduo_dev[BT_SOURCE_SRV_MUSIC_PSEDUO_DEV_NUM];

static bool g_bt_a2dp_suspend_flag;


typedef void (*bt_source_srv_audio_src_event_handle_t)(bt_source_srv_music_pseduo_dev_t *device);
static bt_source_srv_audio_src_event_handle_t g_audio_src_event_handle[] = {
    bt_source_srv_music_psd_audio_src_take_success_handle,
    bt_source_srv_music_psd_audio_src_take_reject_handle,
    bt_source_srv_music_psd_audio_src_give_success_handle,
    bt_source_srv_music_psd_audio_src_suspend_handle
};

typedef bt_status_t (*bt_source_srv_state_machine_handle_t)(bt_source_srv_music_pseduo_dev_t *device, bt_source_srv_music_psd_event_t event, void *parameter);
static const bt_source_srv_state_machine_handle_t g_music_state_machine_handle[] = {
    bt_source_srv_music_psd_state_none_handle,
    bt_source_srv_music_psd_state_ready_handle,
    bt_source_srv_music_psd_state_play_handle,
};

static void bt_source_srv_music_psd_update_state(bt_source_srv_music_pseduo_dev_t *device, bt_source_srv_music_psd_state_t state)
{
    bt_source_srv_assert(device && "update state device is NULL");
    device->state = state;
}

static void bt_source_srv_music_psd_update_sub_state(bt_source_srv_music_pseduo_dev_t *device, bt_source_srv_music_psd_sub_state_t sub_state)
{
    bt_source_srv_assert(device && "update sub state device is NULL");
    LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR] device = %02x,sub_state %2x", 2, device, sub_state);
    device->sub_state = sub_state;
}

static void bt_source_srv_music_psd_update_next_state(bt_source_srv_music_pseduo_dev_t *device, bt_source_srv_music_psd_next_state_t next_state)
{
    bt_source_srv_assert(device && "update next state device is NULL");
    LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR] device = %02x update next state = %2x", 2, device, next_state);
    device->next_state = next_state;
}

static void bt_source_srv_music_psd_take_audio_src_complete(bt_source_srv_music_pseduo_dev_t *device)
{
    bt_source_srv_assert(device && "take audio source complete device is NULL");
    bt_source_srv_music_psd_update_state(device, BT_SOURCE_SRV_MUSIC_PSD_STATE_PLAY);
    bt_source_srv_music_psd_update_sub_state(device, BT_SOURCE_SRV_MUSIC_PSD_SUB_STATE_PLAY_IDLE);
    bt_source_srv_music_psd_update_next_state(device, BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_INIT);

    bt_source_srv_music_device_t *context = bt_source_srv_music_get_device(BT_SRV_MUSIC_DEVICE_AUDIO_RESOURCE_DEVICE, device);
    bool need_resume_special_mode = false;
    if (context && context->detect_flag) {
        need_resume_special_mode = true;
    }
    LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR] take_audio_src_complete:need resume = 0x%0x, flags:%x", 2, need_resume_special_mode, device->flags);
    if (!BT_SOURCE_SRV_MUSIC_PSD_FLAG_IS_SET(device, BT_SOURCE_SRV_MUSIC_PSD_FLAG_SPEAKER_ADD_WL)) {
        if (!need_resume_special_mode && !(BT_SOURCE_SRV_MUSIC_PSD_FLAG_IS_SET(device,BT_SOURCE_SRV_MUSIC_PSD_FLAG_ENTER_SPECIAL_MODE))) {
            bt_source_srv_music_psd_event_notify(device, BT_SOURCE_SRV_MUSIC_PSD_EVENT_MUSIC_START, NULL);
        }
    } else {
        BT_SOURCE_SRV_MUSIC_PSD_REMOVE_FLAG(device, BT_SOURCE_SRV_MUSIC_PSD_FLAG_SPEAKER_ADD_WL);
        if (device->user_musicback) {
            device->user_musicback(BT_SOURCE_SRV_MUSIC_PSD_USER_EVENT_RESUME, (void *)device, NULL);
        }
    }
}

static void bt_source_srv_music_psd_give_audio_src_complete(bt_source_srv_music_pseduo_dev_t *device)
{
    bt_source_srv_music_psd_update_state(device, BT_SOURCE_SRV_MUSIC_PSD_STATE_READY);
    bt_source_srv_music_psd_update_sub_state(device, BT_SOURCE_SRV_MUSIC_PSD_SUB_STATE_NONE);
    /* run next state in ready state */
    bt_source_srv_music_psd_run_next_state_with_state_ready(device);
}

static bt_status_t bt_source_srv_music_psd_take_audio_source(bt_source_srv_music_pseduo_dev_t *device)
{
    bt_source_srv_assert(device && "take audio source device is NULL");
    LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR] take audio source, device:0x%x, flag = 0x%0x,", 2, device, device->flags);
    if (!BT_SOURCE_SRV_MUSIC_PSD_FLAG_IS_SET(device, BT_SOURCE_SRV_MUSIC_PSD_FLAG_MIC_TAKED)) {
        if (device->sub_state == BT_SOURCE_SRV_MUSIC_PSD_SUB_STATE_NONE) {
            bt_source_srv_music_psd_update_next_state(device, BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_PLAY);
            bt_source_srv_music_psd_update_sub_state(device, BT_SOURCE_SRV_MUSIC_PSD_STATE_TAKE_AUDIO_SRC);
            audio_src_srv_resource_manager_take(device->speaker_audio_src);
        }

        return BT_STATUS_PENDING;
    }

    LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR] take device = %02x all audio source success", 1, device);
    bt_source_srv_music_psd_take_audio_src_complete(device);
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_music_psd_give_audio_source(bt_source_srv_music_pseduo_dev_t *device)
{
    bt_source_srv_assert(device && "give audio source device is NULL");
    LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR] give audio source device = 0x%x, flag = 0x%x", 2,device, device->flags);

    if (BT_SOURCE_SRV_MUSIC_PSD_FLAG_IS_SET(device, BT_SOURCE_SRV_MUSIC_PSD_FLAG_MIC_TAKED)) {
        bt_source_srv_music_psd_update_sub_state(device, BT_SOURCE_SRV_MUSIC_PSD_STATE_GIVE_AUDIO_SRC);
        //bt_source_srv_music_psd_update_next_state(device,BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_READY);
        audio_src_srv_resource_manager_give(device->speaker_audio_src);
        if (BT_SOURCE_SRV_MUSIC_PSD_FLAG_IS_SET(device, BT_SOURCE_SRV_MUSIC_PSD_FLAG_SUSPEND)) {
            bt_source_srv_music_psd_add_waiting_list(BT_SOURCE_SRV_MUSIC_PSD_AUDIO_SRC_TYPE_SPEAKER, device);
            BT_SOURCE_SRV_MUSIC_PSD_REMOVE_FLAG(device, BT_SOURCE_SRV_MUSIC_PSD_FLAG_SUSPEND);
        }
        return BT_STATUS_PENDING;
    }

    /* all audio source give success, update state */
    LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR] give device = %02x all audio source success", 1, device);
    bt_source_srv_music_psd_give_audio_src_complete(device);
    return BT_STATUS_SUCCESS;
}



static bt_source_srv_music_pseduo_dev_t *bt_source_srv_music_psd_find_device_by_handle(bt_source_srv_music_psd_audio_source_t type, void *handle)
{
    uint32_t i = 0;
    for (i = 0; i < BT_SOURCE_SRV_MUSIC_PSEDUO_DEV_NUM; i++) {
        bt_source_srv_music_pseduo_dev_t *device = &g_source_srv_music_pseduo_dev[i];
        if (((type == BT_SOURCE_SRV_MUSIC_PSD_AUDIO_SRC_TYPE_SPEAKER) && (device->speaker_audio_src == (audio_src_srv_resource_manager_handle_t *) handle))) {
            return device;
        }
    }
    return NULL;
}

static bt_status_t bt_source_srv_music_psd_run_next_state_with_prepare_starting(bt_source_srv_music_pseduo_dev_t *device)
{
   LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR] bt_source_srv_music_psd_run_next_state_with_prepare_starting,next_state = 0x%x", 1, device->next_state);

   switch (device->next_state) {
        //play
        case BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_PLAY: {
            bt_source_srv_music_psd_take_audio_source(device);
        }
        break;
        //disconnect
        case BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_NONE: {
            bt_source_srv_music_psd_give_audio_source(device);
            bt_source_srv_music_psd_update_next_state(device, BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_INIT);
        }
        break;
        //suspend/end
        case BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_READY:{
            bt_source_srv_music_psd_give_audio_source(device);
        }
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}


static void bt_source_srv_music_psd_audio_src_take_success_handle(bt_source_srv_music_pseduo_dev_t *device)
{
    BT_SOURCE_SRV_MUSIC_PSD_SET_FLAG(device, BT_SOURCE_SRV_MUSIC_PSD_FLAG_MIC_TAKED);
    g_bt_a2dp_suspend_flag = false;
    LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR] take_success_handle:flags = 0x%0x", 1, device->flags);
    if (BT_SOURCE_SRV_MUSIC_PSD_FLAG_IS_SET(device, BT_SOURCE_SRV_MUSIC_PSD_FLAG_SPEAKER_ADD_WL)) {
        bt_source_srv_music_psd_update_next_state(device, BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_PLAY);
    }
    bt_source_srv_music_psd_run_next_state_with_prepare_starting(device);

}

static void bt_source_srv_music_psd_audio_src_take_reject_handle(bt_source_srv_music_pseduo_dev_t *device)
{
    BT_SOURCE_SRV_MUSIC_PSD_SET_FLAG(device, BT_SOURCE_SRV_MUSIC_PSD_FLAG_REJECT);
    LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR] Reject handle: device = %02x, flags = 0x%x", 2, device, device->flags);
    bt_source_srv_music_psd_add_waiting_list(BT_SOURCE_SRV_MUSIC_PSD_AUDIO_SRC_TYPE_SPEAKER, device);

    bt_source_srv_music_psd_update_next_state(device, BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_INIT);
    bt_source_srv_music_psd_update_sub_state(device, BT_SOURCE_SRV_MUSIC_PSD_SUB_STATE_NONE);
    if (device->user_musicback) {
        device->user_musicback(BT_SOURCE_SRV_MUSIC_PSD_USER_EVENT_REJECT, (void *)device, NULL);
    }
}

static void bt_source_srv_music_psd_audio_src_give_success_handle(bt_source_srv_music_pseduo_dev_t *device)
{
    BT_SOURCE_SRV_MUSIC_PSD_REMOVE_FLAG(device, BT_SOURCE_SRV_MUSIC_PSD_FLAG_MIC_TAKED);
    bt_source_srv_music_psd_give_audio_source(device);
}

static void bt_source_srv_music_psd_audio_src_suspend_handle(bt_source_srv_music_pseduo_dev_t *device)
{
    LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR] suspend_handle: device = %02x, flags = 0x%x", 2, device, device->flags);
    BT_SOURCE_SRV_MUSIC_PSD_SET_FLAG(device, BT_SOURCE_SRV_MUSIC_PSD_FLAG_SUSPEND);
    if (device->user_musicback) {
        device->user_musicback(BT_SOURCE_SRV_MUSIC_PSD_USER_EVENT_SUSPEND, (void *)device, NULL);
    }
}

static void bt_source_srv_music_psd_audio_resource_callback(audio_src_srv_resource_manager_handle_t *handle,
        audio_src_srv_resource_manager_event_t event)
{
    bt_source_srv_assert(handle && "audio resource musicback handle is NULL");
    bt_source_srv_music_pseduo_dev_t *device = bt_source_srv_music_psd_find_device_by_handle(BT_SOURCE_SRV_MUSIC_PSD_AUDIO_SRC_TYPE_SPEAKER, (void *)handle);
    if (device == NULL) {
        LOG_MSGID_W(source_srv, "[MUSIC][PSD][MGR] resource event handle = %02x not find device", 1, handle);
        return;
    }

    LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR] psd_audio_resource_callback: device = %02x, event = %02x", 2, device, event);
    if ((sizeof(g_audio_src_event_handle) / sizeof(bt_source_srv_audio_src_event_handle_t)) > event) {
        g_audio_src_event_handle[event](device);
    }
}

static void bt_source_srv_music_psd_add_waiting_list(bt_source_srv_music_psd_audio_source_t type, bt_source_srv_music_pseduo_dev_t *device)
{
    bt_source_srv_assert(device && "add waiting list device is NULL");
    LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR] add_waiting_list: device = %02x, flags = 0x%x", 2, device, device->flags);

    if (!BT_SOURCE_SRV_MUSIC_PSD_FLAG_IS_SET(device, BT_SOURCE_SRV_MUSIC_PSD_FLAG_SPEAKER_ADD_WL)) {
        BT_SOURCE_SRV_MUSIC_PSD_SET_FLAG(device, BT_SOURCE_SRV_MUSIC_PSD_FLAG_SPEAKER_ADD_WL);
        audio_src_srv_resource_manager_add_waiting_list(device->speaker_audio_src);
    }
}


static void bt_source_srv_music_psd_del_waiting_list(bt_source_srv_music_psd_audio_source_t type, bt_source_srv_music_pseduo_dev_t *device)
{
    bt_source_srv_assert(device && "add waiting list device is NULL");
    LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR] del_waiting_list: device = %02x, flags = 0x%x", 2, device, device->flags);
    if (BT_SOURCE_SRV_MUSIC_PSD_FLAG_IS_SET(device, BT_SOURCE_SRV_MUSIC_PSD_FLAG_SPEAKER_ADD_WL)) {
        BT_SOURCE_SRV_MUSIC_PSD_REMOVE_FLAG(device, BT_SOURCE_SRV_MUSIC_PSD_FLAG_SPEAKER_ADD_WL);
        audio_src_srv_resource_manager_delete_waiting_list(device->speaker_audio_src);
    }
}

static bt_status_t bt_source_srv_music_pse_close(bt_source_srv_music_pseduo_dev_t *device, bt_source_srv_port_t port)
{
    bt_status_t ret = BT_STATUS_FAIL;

    LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR] music_pse_close:port:%x, audio_id:%x", 2, port, device->audio_id[port]);

    if (device->audio_id[port] != BT_SOURCE_SRV_MUSIC_AUDIO_INVALID_ID) {
        device->port = port;
        ret = bt_source_srv_music_audio_stop(device->audio_id[port]);
    }

    return ret;

}

static bt_status_t bt_source_srv_music_psd_audio_stop(bt_source_srv_music_pseduo_dev_t *device, bt_source_srv_port_t port)
{
    bt_source_srv_assert(device && "give codec device is NULL");
    bt_source_srv_music_psd_update_sub_state(device, BT_SOURCE_SRV_MUSIC_PSD_SUB_STATE_CODEC_STOPPING);
    bt_status_t ret = BT_STATUS_FAIL;
    if (port == BT_SOURCE_SRV_PORT_NONE) {
        ret = bt_source_srv_music_pse_close(device, BT_SOURCE_SRV_PORT_CHAT_SPEAKER);
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
        ret = bt_source_srv_music_pse_close(device, BT_SOURCE_SRV_PORT_LINE_IN);
        ret = bt_source_srv_music_pse_close(device, BT_SOURCE_SRV_PORT_I2S_IN);
        ret = bt_source_srv_music_pse_close(device, BT_SOURCE_SRV_PORT_I2S_IN_1);
#endif
    } else {
        ret = bt_source_srv_music_pse_close(device, port);
    }

    LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR] audio_stop:port:%x, ret :%x", 2, port, ret);
    if (port == BT_SOURCE_SRV_PORT_NONE) {
        bt_source_srv_assert(device && "give codec fail");
        bt_source_srv_music_device_t *context = bt_source_srv_music_get_device(BT_SRV_MUSIC_DEVICE_AUDIO_RESOURCE_DEVICE, device);
        bt_source_srv_common_switch_sniff_mode(&(context->dev_addr), true);
        bt_source_srv_set_music_enable(&(context->dev_addr), false);
    }
    return ret;
}

static void bt_source_srv_music_psd_run_next_state_with_state_ready(bt_source_srv_music_pseduo_dev_t *device)
{
    LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR] next_state_with_state_ready:state = %02x", 1, device->next_state);

    switch (device->next_state) {
        case BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_PLAY: {
            bt_source_srv_music_psd_take_audio_source(device);
        }
        break;
        case BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_NONE: {
            bt_source_srv_music_psd_update_state(device, BT_SOURCE_SRV_MUSIC_PSD_STATE_NONE);
            bt_source_srv_music_psd_update_next_state(device, BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_INIT);
        }
        break;
        case BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_READY: {
            bt_source_srv_music_psd_update_next_state(device, BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_INIT);
        }
        break;
        default:
            break;
    }
}

static bt_status_t bt_source_srv_music_psd_sub_state_none_handle(bt_source_srv_music_pseduo_dev_t *device, bt_source_srv_music_psd_event_t event, void *parameter)
{
    LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR]sub_state_none_handle event:%02x give codec type = %02x", 1,event, device->flags);

    switch (device->state) {
        case BT_SOURCE_SRV_MUSIC_PSD_STATE_READY: {
            switch (event) {
                case BT_SOURCE_SRV_MUSIC_PSD_EVENT_LINK_DISCONNECTED: {
                    /* delete waiting list device. */
                    bt_source_srv_music_psd_del_waiting_list(BT_SOURCE_SRV_MUSIC_PSD_AUDIO_SRC_TYPE_SPEAKER, device);
                    bt_source_srv_music_psd_update_state(device, BT_SOURCE_SRV_MUSIC_PSD_STATE_NONE);
                }
                break;
                case BT_SOURCE_SRV_MUSIC_PSD_EVENT_MUSIC_START: {
                    /* take audio source(speaker and mic) */
                    bt_source_srv_music_psd_take_audio_source(device);
                }
                break;
                case BT_SOURCE_SRV_MUSIC_PSD_EVENT_SUSPEND_REQ: {
                    /* delete waiting list device. */
                    bt_source_srv_music_psd_del_waiting_list(BT_SOURCE_SRV_MUSIC_PSD_AUDIO_SRC_TYPE_SPEAKER, device);
                }
                break;
                case BT_SOURCE_SRV_MUSIC_PSD_EVENT_AUDIO_STOP_IND: {

                    /*only for is inttrrupt by call when music is muted, so not clear flags*/
                    if (BT_SOURCE_SRV_MUSIC_PSD_FLAG_IS_SET(device, BT_SOURCE_SRV_MUSIC_PSD_FLAG_ENTER_SPECIAL_MODE)) {
                        bt_source_srv_music_audio_deinit(device->audio_id[device->port]);
                        BT_SOURCE_SRV_MUSIC_PSD_REMOVE_FLAG(device, BT_SOURCE_SRV_MUSIC_PSD_FLAG_ENTER_SPECIAL_MODE);
                    }
                }
                break;
                case BT_SOURCE_SRV_MUSIC_PSD_EVENT_AUDIO_PLAY_IND: {
                    if (BT_SOURCE_SRV_MUSIC_PSD_FLAG_IS_SET(device, BT_SOURCE_SRV_MUSIC_PSD_FLAG_ENTER_SPECIAL_MODE)) {
                        bt_source_srv_music_exit_with_out_bt_mode(device->port,device);
                    }
                }
                break;
                default:
                    break;
            }
        }
        break;
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}


static bt_status_t bt_source_srv_music_psd_state_none_handle(bt_source_srv_music_pseduo_dev_t *device, bt_source_srv_music_psd_event_t event, void *parameter)
{
    LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR] state_none_handle:event = %02x,flag:%0x",2, event, g_bt_a2dp_suspend_flag);
    switch (event) {
        case BT_SOURCE_SRV_MUSIC_PSD_EVENT_LINK_CONNECTED: {
            bt_source_srv_music_psd_update_state(device, BT_SOURCE_SRV_MUSIC_PSD_STATE_READY);
            bt_source_srv_music_psd_update_sub_state(device, BT_SOURCE_SRV_MUSIC_PSD_SUB_STATE_NONE);
            if (g_bt_a2dp_suspend_flag) {
                if (bt_source_srv_common_audio_port_is_valid(BT_SOURCE_SRV_PORT_CHAT_SPEAKER) &&
                    bt_source_srv_common_audio_port_is_valid(BT_SOURCE_SRV_PORT_MIC)) {
                    bt_source_srv_music_psd_add_waiting_list(BT_SOURCE_SRV_MUSIC_PSD_AUDIO_SRC_TYPE_SPEAKER,device);
                }
            }
        }
        break;
        case BT_SOURCE_SRV_MUSIC_PSD_EVENT_LINK_DISCONNECTED: {
            bt_source_srv_music_psd_update_state(device, BT_SOURCE_SRV_MUSIC_PSD_STATE_NONE);
            bt_source_srv_music_psd_update_sub_state(device, BT_SOURCE_SRV_MUSIC_PSD_SUB_STATE_NONE);
        }
        break;
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_music_psd_sub_state_taking_audio_handle(bt_source_srv_music_pseduo_dev_t *device, bt_source_srv_music_psd_event_t event, void *parameter)
{
    bt_status_t status = BT_STATUS_FAIL;
    switch (event) {
        case BT_SOURCE_SRV_MUSIC_PSD_EVENT_LINK_DISCONNECTED: {
            /* update state */
            bt_source_srv_music_psd_del_waiting_list(BT_SOURCE_SRV_MUSIC_PSD_AUDIO_SRC_TYPE_SPEAKER, device);
            bt_source_srv_music_psd_update_next_state(device, BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_NONE);
        }
        break;
        case BT_SOURCE_SRV_MUSIC_PSD_EVENT_SUSPEND_REQ: {
            bt_source_srv_music_psd_del_waiting_list(BT_SOURCE_SRV_MUSIC_PSD_AUDIO_SRC_TYPE_SPEAKER, device);
            bt_source_srv_music_psd_update_next_state(device, BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_READY);
        }
        break;
        default:
            break;
    }
    return status;
}

static bt_status_t bt_source_srv_music_psd_sub_state_giving_audio_handle(bt_source_srv_music_pseduo_dev_t *device, bt_source_srv_music_psd_event_t event, void *parameter)
{
    bt_status_t status = BT_STATUS_FAIL;
    switch (event) {
        //disconn
        case BT_SOURCE_SRV_MUSIC_PSD_EVENT_LINK_DISCONNECTED: {
            bt_source_srv_music_psd_update_next_state(device, BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_NONE);
        }
        break;
        //start
        case BT_SOURCE_SRV_MUSIC_PSD_EVENT_MUSIC_START: {
            bt_source_srv_music_psd_update_next_state(device, BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_PLAY);
        }
        break;
        default:
            break;
    }
    return status;

}


static bt_status_t bt_source_srv_music_psd_state_ready_handle(bt_source_srv_music_pseduo_dev_t *device, bt_source_srv_music_psd_event_t event, void *parameter)
{
    bt_status_t status = BT_STATUS_FAIL;
    switch (device->sub_state) {
        case BT_SOURCE_SRV_MUSIC_PSD_SUB_STATE_NONE: {
            status = bt_source_srv_music_psd_sub_state_none_handle(device, event, parameter);
        }
        break;
        case BT_SOURCE_SRV_MUSIC_PSD_STATE_TAKE_AUDIO_SRC: {
            status = bt_source_srv_music_psd_sub_state_taking_audio_handle(device, event, parameter);
        }
        break;
        case BT_SOURCE_SRV_MUSIC_PSD_STATE_GIVE_AUDIO_SRC: {
            status = bt_source_srv_music_psd_sub_state_giving_audio_handle(device, event, parameter);
          break;
        }
        default:
            break;
    }
    return status;
}

static void bt_source_srv_music_psd_deinit_all_audio(bt_source_srv_music_pseduo_dev_t *device)
{
    bt_source_srv_music_audio_deinit(device->audio_id[BT_SOURCE_SRV_PORT_CHAT_SPEAKER]);
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
    bt_source_srv_music_audio_deinit(device->audio_id[BT_SOURCE_SRV_PORT_LINE_IN]);
    bt_source_srv_music_audio_deinit(device->audio_id[BT_SOURCE_SRV_PORT_I2S_IN]);
    bt_source_srv_music_audio_deinit(device->audio_id[BT_SOURCE_SRV_PORT_I2S_IN_1]);
#endif
}

static bt_status_t bt_source_srv_music_psd_sub_state_play_idle_handle(bt_source_srv_music_pseduo_dev_t *device, bt_source_srv_music_psd_event_t event, void *parameter)
{
    LOG_MSGID_I(source_srv, "[MUSIC][PSD][TEST]state_play_idle_handle,event:0x%x,device :0x%x, flags:0x%02x", 3, event,device, device->flags);

    switch (event) {
        case BT_SOURCE_SRV_MUSIC_PSD_EVENT_LINK_DISCONNECTED: {
            if (BT_SOURCE_SRV_MUSIC_PSD_FLAG_IS_SET(device, BT_SOURCE_SRV_MUSIC_PSD_FLAG_SUSPEND)) {
                BT_SOURCE_SRV_MUSIC_PSD_REMOVE_FLAG(device, BT_SOURCE_SRV_MUSIC_PSD_FLAG_SUSPEND);
            }
            if (BT_SOURCE_SRV_MUSIC_PSD_FLAG_IS_SET(device,BT_SOURCE_SRV_MUSIC_PSD_FLAG_ENTER_SPECIAL_MODE)) {
                bt_source_srv_music_exit_with_out_bt_mode(BT_SOURCE_SRV_PORT_CHAT_SPEAKER, device);
            }
            bt_source_srv_music_psd_give_audio_source(device);
            bt_source_srv_music_psd_update_next_state(device, BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_NONE);
        }
        break;
        case BT_SOURCE_SRV_MUSIC_PSD_EVENT_AUDIO_STOP_IND:
        {
            if (BT_SOURCE_SRV_MUSIC_PSD_FLAG_IS_SET(device, BT_SOURCE_SRV_MUSIC_PSD_FLAG_ENTER_SPECIAL_MODE)) {
                bt_source_srv_music_audio_deinit(device->audio_id[device->port]);
                BT_SOURCE_SRV_MUSIC_PSD_REMOVE_FLAG(device,BT_SOURCE_SRV_MUSIC_PSD_FLAG_ENTER_SPECIAL_MODE);

            } else {
                bt_source_srv_music_psd_update_next_state(device, BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_READY);

                LOG_MSGID_I(source_srv,"[MUSIC][PSD][TEST]state_play_idle_handle,event:0x%x,num:0x%0x,flags:0x%x",3,device->audio_play_num, device, device->flags);
                if (!device->audio_play_num) {
                    bt_source_srv_music_psd_deinit_all_audio(device);
                    bt_source_srv_music_psd_give_audio_source(device);
                }
            }
        }
        break;
        case BT_SOURCE_SRV_MUSIC_PSD_USER_EVENT_SUSPEND_BY_PRIORITY: {
            LOG_MSGID_I(source_srv,"[MUSIC][PSD][TEST]SUSPEND_BY_PRIORITY,num:0x%0x", 1, device->audio_play_num);
            bt_source_srv_music_psd_update_next_state(device, BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_READY);
            bt_source_srv_music_psd_give_audio_source(device);
        }
        break;
        case BT_SOURCE_SRV_MUSIC_PSD_EVENT_SUSPEND_REQ: {
            /* give mic resource */
            bt_source_srv_music_psd_update_next_state(device, BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_READY);
            LOG_MSGID_I(source_srv,"[MUSIC][PSD][TEST]state_play_idle_handle,event:0x%x,num:0x%0x", 1, device->audio_play_num, device, device->flags);
            if (!device->audio_play_num) {
                bt_source_srv_music_psd_give_audio_source(device);
            }
        }
        break;
        case BT_SOURCE_SRV_MUSIC_PSD_EVENT_MUSIC_START:
        case BT_SOURCE_SRV_MUSIC_PSD_EVENT_MUSIC_REPLAY:{
            bt_status_t status = BT_STATUS_FAIL;
            if (event == BT_SOURCE_SRV_MUSIC_PSD_EVENT_MUSIC_REPLAY) {
                bt_source_srv_music_psd_audio_replay_req_t *req = (bt_source_srv_music_psd_audio_replay_req_t*)parameter;
                status = bt_source_srv_music_psd_replay(device, req->port);
            } else {
               status = bt_source_srv_music_psd_play(device);
            }
            if (status == BT_STATUS_SUCCESS) {
                bt_source_srv_music_psd_update_sub_state(device, BT_SOURCE_SRV_MUSIC_PSD_SUB_STATE_CODEC_STARTING);
                bt_source_srv_music_psd_update_next_state(device, BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_PLAY);
            } else if (status == BT_STATUS_FAIL){
                /* give mic resource */
                bt_source_srv_music_psd_update_next_state(device, BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_READY);
                bt_source_srv_music_psd_give_audio_source(device);
                if (device->user_musicback) {
                    device->user_musicback(BT_SOURCE_SRV_MUSIC_PSD_USER_EVENT_AUDIO_START_FAIL, (void *)device, NULL);
                }

            } else {
                /**/
            }
            LOG_MSGID_I(source_srv, "[MUSIC][PSD][TEST] device :0x%x, flags:0x%02x, status :0x%x", 3, device, device->flags, status);
        }
            break;
#if (defined AIR_BT_AUDIO_DONGLE_ENABLE) ||  (defined AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE)
        case BT_SOURCE_SRV_MUSIC_PSD_EVENT_AUDIO_PLAY_IND: {
            if (BT_SOURCE_SRV_MUSIC_PSD_FLAG_IS_SET(device, BT_SOURCE_SRV_MUSIC_PSD_FLAG_ENTER_SPECIAL_MODE)) {
                bt_source_srv_music_psd_start_detect_media_data(device->audio_id[device->port]);
            }
            break;
        }
#endif
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}


static bt_status_t bt_source_srv_music_psd_sub_state_playing_handle(bt_source_srv_music_pseduo_dev_t *device, bt_source_srv_music_psd_event_t event, void *parameter)
{
    LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR]sub_state_playing_handle:event,0x%0x, flag:0x%0x",2,event,device->flags);

    switch (event) {
        case BT_SOURCE_SRV_MUSIC_PSD_EVENT_LINK_DISCONNECTED: {
            bt_source_srv_music_psd_update_next_state(device, BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_NONE);
            bt_source_srv_music_psd_audio_stop(device, BT_SOURCE_SRV_PORT_NONE);
        }
        break;
        case BT_SOURCE_SRV_MUSIC_PSD_EVENT_SUSPEND_REQ: {
            bt_source_srv_music_psd_audio_replay_req_t *audio_replay = (bt_source_srv_music_psd_audio_replay_req_t *)parameter;

            if (audio_replay == NULL) {
                bt_source_srv_music_psd_update_next_state(device, BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_READY);
                bt_source_srv_music_psd_audio_stop(device, BT_SOURCE_SRV_PORT_NONE);
            } else {
                LOG_MSGID_I(source_srv,"[MUSIC][PSD][MGR]sub_state_playing_handle:0x%0x, port:0x%x", 2, audio_replay, audio_replay->port);
                bt_source_srv_music_psd_update_next_state(device, BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_INIT);
                bt_source_srv_music_psd_audio_stop(device, audio_replay->port);
            }
        }
        break;
        case BT_SOURCE_SRV_MUSIC_PSD_EVENT_AUDIO_STOP_IND: {
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
            bt_source_srv_music_psd_deinit_all_audio(device);
#else
            bt_source_srv_music_audio_deinit(device->audio_id[device->port]);
#endif

            bt_source_srv_music_psd_update_sub_state(device, BT_SOURCE_SRV_MUSIC_PSD_SUB_STATE_PLAY_IDLE);
            /* give audio source */
            bt_source_srv_music_psd_give_audio_source(device);
            break;
        }
        case BT_SOURCE_SRV_MUSIC_PSD_EVENT_AUDIO_PLAY_IND: {
            bt_source_srv_music_psd_update_next_state(device, BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_INIT);
#if (defined AIR_BT_AUDIO_DONGLE_ENABLE) ||  (defined AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE)
            if (device->port == BT_SOURCE_SRV_PORT_CHAT_SPEAKER) {
                bt_source_srv_music_psd_start_detect_media_data(device->audio_id[device->port]);
            }
#endif
            if (!BT_SOURCE_SRV_MUSIC_PSD_FLAG_IS_SET(device,BT_SOURCE_SRV_MUSIC_PSD_FLAG_ENTER_SPECIAL_MODE) && device->user_musicback) {
                device->user_musicback(BT_SOURCE_SRV_MUSIC_PSD_USER_EVENT_AUDIO_START_SUCCESS, (void *)device, NULL);
            }
            break;
        }

        case BT_SOURCE_SRV_MUSIC_PSD_EVENT_MUSIC_REPLAY: {
            bt_source_srv_music_psd_audio_replay_req_t *audio_replay = (bt_source_srv_music_psd_audio_replay_req_t *)parameter;
            LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR] REPLAY_Action,0x%02x,port:0x%02x", 2, audio_replay->action, audio_replay->port);
            if (audio_replay->action == BT_SOURCE_SRV_MUSIC_PORT_ACTION_OPEN) {
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
                bt_source_srv_music_psd_replay(device, audio_replay->port);
#endif
            } else if (audio_replay->action == BT_SOURCE_SRV_MUSIC_PORT_ACTION_UPDATE) {
                BT_SOURCE_SRV_MUSIC_PSD_SET_FLAG(device, BT_SOURCE_SRV_MUSIC_PSD_FLAG_RESTART);
                bt_source_srv_music_psd_update_next_state(device, BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_PLAY);
                bt_source_srv_music_psd_audio_stop(device, audio_replay->port);
            }
            break;
        }
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}

static void bt_source_srv_music_psd_sub_state_codec_starting_handle(bt_source_srv_music_pseduo_dev_t *device, bt_source_srv_music_psd_event_t event, void *parameter)
{
    LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR] codec_starting_handle:event,0x%02x", 1,event);
    switch (event) {
        case BT_SOURCE_SRV_MUSIC_PSD_EVENT_AUDIO_PLAY_IND: {
            bt_source_srv_music_psd_update_sub_state(device, BT_SOURCE_SRV_MUSIC_PSD_SUB_STATE_PLAYING);
            bt_source_srv_music_psd_run_next_state_with_codec_starting_handle(device);
        }
        break;
        case BT_SOURCE_SRV_MUSIC_PSD_EVENT_AUDIO_STOP_IND: {
            bt_source_srv_music_psd_audio_stop(device, BT_SOURCE_SRV_PORT_NONE);
            bt_source_srv_music_psd_update_sub_state(device, BT_SOURCE_SRV_MUSIC_PSD_STATE_READY);
        }
        break;
        case BT_SOURCE_SRV_MUSIC_PSD_EVENT_LINK_DISCONNECTED: {
            bt_source_srv_music_psd_update_next_state(device, BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_NONE);
        }
        break;
        case BT_SOURCE_SRV_MUSIC_PSD_EVENT_SUSPEND_REQ: {
            bt_source_srv_music_psd_update_next_state(device, BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_READY);
        }
        break;
    }
}


static void bt_source_srv_music_psd_sub_state_codec_stoping_handle(bt_source_srv_music_pseduo_dev_t *device, bt_source_srv_music_psd_event_t event, void *parameter)
{
    LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR]codec_starting_handle:event,0x%02x,port:0x%x",2,event, device->port);
    switch (event) {
        case BT_SOURCE_SRV_MUSIC_PSD_EVENT_MUSIC_START:
        case BT_SOURCE_SRV_MUSIC_PSD_EVENT_AUDIO_PLAY_IND: {
            bt_source_srv_music_psd_update_next_state(device, BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_PLAY);
        }
        break;
        case BT_SOURCE_SRV_MUSIC_PSD_EVENT_LINK_DISCONNECTED: {
            bt_source_srv_music_psd_update_next_state(device, BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_NONE);

        }
        break;
        case BT_SOURCE_SRV_MUSIC_PSD_EVENT_SUSPEND_REQ: {
            bt_source_srv_music_psd_update_next_state(device, BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_READY);
        }
        break;
        case BT_SOURCE_SRV_MUSIC_PSD_EVENT_AUDIO_STOP_IND: {
            LOG_MSGID_I(source_srv,"[MUSIC][PSD][MGR]codec_stoping_handle:num:0x%x", 1, device->audio_play_num);
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)

            if (!device->audio_play_num) {
                bt_source_srv_music_psd_update_sub_state(device, BT_SOURCE_SRV_MUSIC_PSD_SUB_STATE_PLAY_IDLE);
            }
            else {
                bt_source_srv_music_psd_update_sub_state(device, BT_SOURCE_SRV_MUSIC_PSD_SUB_STATE_PLAYING);
            }
#else
            bt_source_srv_music_psd_update_sub_state(device, BT_SOURCE_SRV_MUSIC_PSD_SUB_STATE_PLAY_IDLE);

#endif
            bt_source_srv_music_device_t *context = bt_source_srv_music_get_device(BT_SRV_MUSIC_DEVICE_AUDIO_RESOURCE_DEVICE, device);

            if (context && context->detect_flag) {
                LOG_MSGID_I(source_srv,"[MUSIC][PSD][MGR]codec_stoping_handle:flag,0x%02x",1,context->detect_flag);
                break;
            }
            bt_source_srv_music_psd_run_next_state_with_codec_stoping_handle(device);
        }
        break;
        default:
            break;
    }

}

static bt_status_t bt_source_srv_music_psd_state_play_handle(bt_source_srv_music_pseduo_dev_t *device, bt_source_srv_music_psd_event_t event, void *parameter)
{
    bt_status_t status = BT_STATUS_FAIL;
    switch (device->sub_state) {
        case BT_SOURCE_SRV_MUSIC_PSD_SUB_STATE_PLAY_IDLE: {
            status = bt_source_srv_music_psd_sub_state_play_idle_handle(device, event, parameter);
        }
        break;

        case BT_SOURCE_SRV_MUSIC_PSD_SUB_STATE_PLAYING: {
            status = bt_source_srv_music_psd_sub_state_playing_handle(device, event, parameter);
        }
        break;

        case BT_SOURCE_SRV_MUSIC_PSD_SUB_STATE_CODEC_STOPPING: {
            bt_source_srv_music_psd_sub_state_codec_stoping_handle(device, event, parameter);
        }
        break;
        case BT_SOURCE_SRV_MUSIC_PSD_SUB_STATE_CODEC_STARTING: {
            bt_source_srv_music_psd_sub_state_codec_starting_handle(device, event, parameter);
        }
        break;
        default:
            break;
    }
    return status;
}

void bt_source_srv_music_psd_convert_devid_to_btaddr(uint64_t dev_id, bt_bd_addr_t *bd_addr)
{
    uint32_t i = 0;
    bt_source_srv_assert(dev_id && "convert devid to address dev_id is NULL");
    for (i = 0; i < BT_BD_ADDR_LEN; ++i) {
        (*bd_addr)[BT_BD_ADDR_LEN - 1 - i] = dev_id & 0xff;
        dev_id = (dev_id >> 8);
    }
    LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR] convert devid_to address:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x", 6,
                (*bd_addr)[5], (*bd_addr)[4], (*bd_addr)[3],
                (*bd_addr)[2], (*bd_addr)[1], (*bd_addr)[0]);
}


bt_status_t bt_source_srv_music_psd_alloc_audio_src(bt_source_srv_music_pseduo_dev_t *device)
{
    bt_source_srv_assert(device && "alloc audio src device is NULL");

    audio_src_srv_resource_manager_handle_t *handle = audio_src_srv_resource_manager_construct_handle(AUDIO_SRC_SRV_RESOURCE_TYPE_BT_SOURCE, AUDIO_SRC_SRV_RESOURCE_TYPE_BT_SOURCE_USER_A2DP);// TO_DO
    if (handle == NULL) {
        LOG_MSGID_E(source_srv, "[MUSIC][PSD][MGR] construct audio source(transmitter) handle fail", 0);
        return BT_STATUS_FAIL;
    }

    /* set audio source musicback. */
    device->speaker_audio_src = handle;

    /* set audio source priority. */
    device->speaker_audio_src->priority = AUDIO_SRC_SRV_RESOURCE_TYPE_BT_SOURCE_USER_A2DP_PRIORIRTY;

    /* set audio resource musicback. */
    device->speaker_audio_src->callback_func = bt_source_srv_music_psd_audio_resource_callback;

    return BT_STATUS_SUCCESS;
}


bt_status_t bt_source_srv_music_psd_free_audio_src(bt_source_srv_music_pseduo_dev_t *device)
{
    bt_source_srv_assert(device && "free audio src device is NULL");

    audio_src_srv_resource_manager_destruct_handle(device->speaker_audio_src);

    return BT_STATUS_SUCCESS;
}

bt_status_t bt_source_srv_music_psd_state_machine(bt_source_srv_music_pseduo_dev_t *device,
        bt_source_srv_music_psd_event_t event, void *parameter)
{
    bt_source_srv_assert(device && "psd device is NULL");
    bt_status_t status = BT_STATUS_FAIL;
    LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR] state machine in  device:%02x, state:%02x, sub_state:%02x, event:%02x", 4,
                device, device->state, device->sub_state, event);
    if (device->state == BT_SOURCE_SRV_MUSIC_PSD_STATE_TAKE_AUDIO_SRC || device->state == BT_SOURCE_SRV_MUSIC_PSD_STATE_GIVE_AUDIO_SRC) {
        LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR] state machine on going device:%02x, state:%02x, sub_state:%02x, event:%02x", 4,
                    device, device->state, device->sub_state, event);
        return status;
    }
    status = g_music_state_machine_handle[device->state](device, event, parameter);

    LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR] state machine out device:%02x, state:%02x, sub_state:%02x, event:%02x", 4,
                device, device->state, device->sub_state, event);
    if ((device->state == BT_SOURCE_SRV_MUSIC_PSD_STATE_NONE) && (device->sub_state == BT_SOURCE_SRV_MUSIC_PSD_SUB_STATE_NONE)) {
        bt_source_srv_music_psd_del_waiting_list(BT_SOURCE_SRV_MUSIC_PSD_AUDIO_SRC_TYPE_SPEAKER, device);
        if (device->user_musicback) {
            device->user_musicback(BT_SOURCE_SRV_MUSIC_PSD_USER_EVENT_DEINIT, (void *)device, NULL);
        }
    }
    return status;
}

static void bt_source_srv_music_psd_run_next_state_with_codec_starting_handle(bt_source_srv_music_pseduo_dev_t *device)
{
    LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR] bt_source_srv_music_psd_run_next_state_with_codec_starting_handle, next_state = %02x", 1, device->next_state);


    switch(device->next_state) {
        case BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_NONE: {
            bt_source_srv_music_psd_event_notify(device, BT_SOURCE_SRV_MUSIC_PSD_EVENT_LINK_DISCONNECTED, NULL);
        }
        break;
        case BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_READY: {
            bt_source_srv_music_psd_event_notify(device, BT_SOURCE_SRV_MUSIC_PSD_EVENT_SUSPEND_REQ, NULL);
        }
        break;
        case BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_PLAY: {
            bt_source_srv_music_psd_event_notify(device, BT_SOURCE_SRV_MUSIC_PSD_EVENT_AUDIO_PLAY_IND, NULL);
        }
        break;
    }
}


static void bt_source_srv_music_psd_run_next_state_with_codec_stoping_handle(bt_source_srv_music_pseduo_dev_t *device)
{
    LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR] bt_source_srv_music_psd_run_next_state_with_codec_stoping_handle, next_state = %02x, flag:0x%x", 2, device->next_state, device->flags);

    switch(device->next_state) {
        case BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_NONE: {
            bt_source_srv_music_psd_event_notify(device, BT_SOURCE_SRV_MUSIC_PSD_EVENT_LINK_DISCONNECTED, NULL);
        }
        break;
        case BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_READY: {
            bt_source_srv_music_psd_event_notify(device, BT_SOURCE_SRV_MUSIC_PSD_EVENT_AUDIO_STOP_IND, NULL);
        }
        break;
        case BT_SOURCE_SRV_MUSIC_PSD_NEXT_STATE_PLAY: {
            if (BT_SOURCE_SRV_MUSIC_PSD_FLAG_IS_SET(device, BT_SOURCE_SRV_MUSIC_PSD_FLAG_RESTART)) {
                bt_source_srv_music_psd_audio_replay_req_t audio_replay_req = {0};
                audio_replay_req.port = device->port;
                bt_source_srv_music_psd_event_notify(device, BT_SOURCE_SRV_MUSIC_PSD_EVENT_MUSIC_REPLAY, &audio_replay_req);
            } else {
                bt_source_srv_music_psd_event_notify(device, BT_SOURCE_SRV_MUSIC_PSD_EVENT_AUDIO_PLAY_IND, NULL);
            }
        }
        break;
    }
}

void bt_source_srv_music_psd_audio_transmitter_callback(audio_transmitter_event_t event, void *data, void *user_data)
{
    bt_source_srv_music_pseduo_dev_t *device = (bt_source_srv_music_pseduo_dev_t *)user_data;
    LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR] audio transmitter callback event = %02x, device:%x,flags:0x%x, audio_play_num:0x%x",4, event, device, device->flags, device->audio_play_num);

    switch (event) {
        case AUDIO_TRANSMITTER_EVENT_START_SUCCESS: {
            device->audio_play_num++;
            if (device->audio_play_num == 1){
                bt_source_srv_music_psd_event_notify(device, BT_SOURCE_SRV_MUSIC_PSD_EVENT_AUDIO_PLAY_IND, NULL);
            }
        }
        break;
        case AUDIO_TRANSMITTER_EVENT_STOP_SUCCESS: {
            device->audio_play_num--;
            LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR] audio transmittercallback:port = %0x, audio_id:%x", 2, device->port, device->audio_id[device->port]);
            bt_status_t ret = bt_source_srv_music_audio_deinit(device->audio_id[device->port]);
            if (ret == BT_STATUS_SUCCESS) {
                device->audio_id[device->port] = BT_SOURCE_SRV_MUSIC_AUDIO_INVALID_ID;
            }
            if (BT_SOURCE_SRV_MUSIC_PSD_FLAG_IS_SET(device, BT_SOURCE_SRV_MUSIC_PSD_FLAG_ENTER_SPECIAL_MODE)) {
                BT_SOURCE_SRV_MUSIC_PSD_REMOVE_FLAG(device,BT_SOURCE_SRV_MUSIC_PSD_FLAG_ENTER_SPECIAL_MODE);
                break;
            }

            bt_source_srv_music_psd_event_notify(device, BT_SOURCE_SRV_MUSIC_PSD_EVENT_AUDIO_STOP_IND, NULL);
        }
        break;
        default:
            break;
    }
}

static bt_status_t bt_source_srv_music_psd_start(bt_source_srv_music_pseduo_dev_t *device,bt_source_srv_port_t port)
{
    bt_status_t ret = BT_STATUS_SUCCESS;

    bt_source_srv_music_audio_config_t config = {0};
    bt_source_srv_music_device_t *context = bt_source_srv_music_get_device(BT_SRV_MUSIC_DEVICE_AUDIO_RESOURCE_DEVICE, device);
    config.type = port;

    ret = bt_source_srv_music_audio_config_init(&config, context, (void*)device, bt_source_srv_music_psd_audio_transmitter_callback);

    if (ret == BT_STATUS_SUCCESS) {
        device->audio_id[port] = bt_source_srv_music_audio_start(&config);
        if (device->audio_id[port] == BT_SOURCE_SRV_MUSIC_AUDIO_INVALID_ID) {
            ret = BT_STATUS_FAIL;
        }
        device->port = port;
    }
    LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR]music_psd_play:device= %x,port:0x%x, id=0x%x,",3, device, port, device->audio_id[port]);
    return ret;
    }

static bt_status_t bt_source_srv_music_psd_replay(bt_source_srv_music_pseduo_dev_t*device,bt_source_srv_port_t port)
{
    bt_status_t ret = bt_source_srv_music_psd_start(device, port);
    return ret;
}


static bt_status_t bt_source_srv_music_psd_play(bt_source_srv_music_pseduo_dev_t * device)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    bt_source_srv_music_device_t *context = bt_source_srv_music_get_device(BT_SRV_MUSIC_DEVICE_AUDIO_RESOURCE_DEVICE, device);

    bt_source_srv_common_switch_sniff_mode(&(context->dev_addr), false);

    ret = bt_source_srv_music_psd_start(device, BT_SOURCE_SRV_PORT_CHAT_SPEAKER);


#ifdef AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE
    ret = ret & bt_source_srv_music_psd_start(device, BT_SOURCE_SRV_PORT_LINE_IN);

#endif

#ifdef AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE
    ret = ret & bt_source_srv_music_psd_start(device, BT_SOURCE_SRV_PORT_I2S_IN);
    ret = ret & bt_source_srv_music_psd_start(device, BT_SOURCE_SRV_PORT_I2S_IN_1);
#endif
    if (ret == BT_STATUS_SUCCESS) {
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
    if (!bt_source_srv_music_psd_is_playing(device))
#endif
        {
            bt_source_srv_set_music_enable(&(context->dev_addr), true);
        }
    }
    LOG_MSGID_I(source_srv, "[MUSIC][PSD][MGR]music_psd_play:device=0x%x,id=0x%x, flag=0x%x",3,device, device->audio_id[device->port], context->flag);
    return ret;
}

void bt_source_srv_music_psd_event_notify(void *device, bt_source_srv_music_psd_event_t event, void *parameter)
{
    bt_source_srv_mutex_lock();
    bt_source_srv_music_psd_state_machine((bt_source_srv_music_pseduo_dev_t *)device, event, parameter);
    bt_source_srv_mutex_unlock();
}

bt_status_t bt_source_srv_music_enter_with_out_bt_mode(bt_source_srv_port_t audio_port, bt_source_srv_music_device_t *device)
{
    bt_source_srv_music_audio_config_t config = {0};
    config.type = audio_port;
    bt_source_srv_music_pseduo_dev_t *audio_dev = (bt_source_srv_music_pseduo_dev_t *)device->audio_dev;
    bt_status_t ret = bt_source_srv_music_audio_config_init(&config, device, (void*)(device->audio_dev), bt_source_srv_music_psd_audio_transmitter_callback);
    if (ret == BT_STATUS_SUCCESS) {
        audio_dev->audio_id[audio_port] = bt_source_srv_music_audio_start(&config);
        if (audio_dev->audio_id[audio_port] == BT_SOURCE_SRV_MUSIC_AUDIO_INVALID_ID) {
            ret = BT_STATUS_FAIL;
        } else {
            BT_SOURCE_SRV_MUSIC_PSD_SET_FLAG(audio_dev, BT_SOURCE_SRV_MUSIC_PSD_FLAG_ENTER_SPECIAL_MODE);
        }
    }
    LOG_MSGID_I(source_srv,"[MUSIC][PSD][MGR]enter_with_out_bt_mode:device=%x,port:0x%x,aud_id=0x%x,ret:0x%x",4,device, audio_port,audio_dev->audio_id[audio_port], ret);
    return ret;
}


bt_status_t bt_source_srv_music_exit_with_out_bt_mode(bt_source_srv_port_t audio_port,bt_source_srv_music_pseduo_dev_t *device)
{
    bt_status_t ret = BT_STATUS_FAIL;

    if (device && device->audio_id[audio_port] != BT_SOURCE_SRV_MUSIC_AUDIO_INVALID_ID) {
        LOG_MSGID_I(source_srv,"[MUSIC][PSD][MGR]exit_with_out_bt_mode:device=%x,aud_id=0x%x,", 2, device, device->audio_id[audio_port]);
        ret = bt_source_srv_music_audio_stop(device->audio_id[audio_port]);
    } else {
        ret = BT_STATUS_SUCCESS;
    }
    LOG_MSGID_I(source_srv,"[MUSIC][PSD][MGR]exit_with_out_bt_mode:port:0x%x,ret=0x%x", 2, audio_port, ret);

    return ret;
}

void bt_source_srv_music_record_waitting_list(bt_source_srv_music_pseduo_dev_t *device)
{
    LOG_MSGID_I(source_srv,"[MUSIC][PSD][MGR]is_in_waiting_list:flags:0x%x,state:0x%x",2, device->flags, device->state);

    if (BT_SOURCE_SRV_MUSIC_PSD_FLAG_IS_SET(device,BT_SOURCE_SRV_MUSIC_PSD_FLAG_SPEAKER_ADD_WL) || device->state == BT_SOURCE_SRV_MUSIC_PSD_STATE_PLAY) {
        g_bt_a2dp_suspend_flag = true;
    }
}
