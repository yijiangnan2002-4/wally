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

#include "bt_source_srv_call_psd_manager.h"
#include "bt_source_srv_call_pseduo_dev.h"
#include "bt_source_srv_utils.h"

static bt_status_t bt_source_srv_call_psd_audio_play_int(bt_source_srv_call_pseduo_dev_t *device, bt_source_srv_call_audio_play_t type);
static bt_status_t bt_source_srv_call_psd_audio_stop_int(bt_source_srv_call_pseduo_dev_t *device, bt_source_srv_call_audio_play_t type);
/* waiting list API */
static void bt_source_srv_call_psd_add_waiting_list(bt_source_srv_call_pseduo_dev_t *device, bt_source_srv_call_psd_audio_source_t type);
static void bt_source_srv_call_psd_del_waiting_list(bt_source_srv_call_pseduo_dev_t *device, bt_source_srv_call_psd_audio_source_t type);
/* audio resource event handle API */
static void bt_source_srv_call_psd_handle_audio_src_take_success(bt_source_srv_call_pseduo_dev_t *device);
static void bt_source_srv_call_psd_handle_audio_src_take_reject(bt_source_srv_call_pseduo_dev_t *device);
static void bt_source_srv_call_psd_handle_audio_src_give_success(bt_source_srv_call_pseduo_dev_t *device);
static void bt_source_srv_call_psd_handle_audio_src_suspend(bt_source_srv_call_pseduo_dev_t *device);
/* state machine handle API */
static bt_status_t bt_source_srv_call_psd_handle_state_none(bt_source_srv_call_pseduo_dev_t *device, bt_source_srv_call_psd_event_t event, void *parameter);
static bt_status_t bt_source_srv_call_psd_handle_state_ready(bt_source_srv_call_pseduo_dev_t *device, bt_source_srv_call_psd_event_t event, void *parameter);
static bt_status_t bt_source_srv_call_psd_handle_state_take_audio_src(bt_source_srv_call_pseduo_dev_t *device, bt_source_srv_call_psd_event_t event, void *parameter);
static bt_status_t bt_source_srv_call_psd_handle_state_play(bt_source_srv_call_pseduo_dev_t *device, bt_source_srv_call_psd_event_t event, void *parameter);
static bt_status_t bt_source_srv_call_psd_handle_state_give_audio_src(bt_source_srv_call_pseduo_dev_t *device, bt_source_srv_call_psd_event_t event, void *parameter);
/* state machine run next state API */
static bt_status_t bt_source_srv_call_psd_run_next_state_with_state_ready(bt_source_srv_call_pseduo_dev_t *device);
static bt_status_t bt_source_srv_call_psd_run_next_state_with_sub_state_play_idle(bt_source_srv_call_pseduo_dev_t *device);

static void bt_source_srv_call_psd_audio_play(bt_source_srv_call_pseduo_dev_t *device);

extern bt_source_srv_call_pseduo_dev_t g_source_srv_call_pseduo_dev[BT_SOURCE_SRV_CALL_PSEDUO_DEV_NUM];

typedef void (*bt_source_srv_audio_src_event_handler_t)(bt_source_srv_call_pseduo_dev_t *device);
const static bt_source_srv_audio_src_event_handler_t g_audio_src_event_handler[] = {
    bt_source_srv_call_psd_handle_audio_src_take_success,
    bt_source_srv_call_psd_handle_audio_src_take_reject,
    bt_source_srv_call_psd_handle_audio_src_give_success,
    bt_source_srv_call_psd_handle_audio_src_suspend
};

typedef bt_status_t (*bt_source_srv_state_machine_handler_t)(bt_source_srv_call_pseduo_dev_t *device, bt_source_srv_call_psd_event_t event, void *parameter);
const static bt_source_srv_state_machine_handler_t g_state_machine_handler[] = {
    bt_source_srv_call_psd_handle_state_none,
    bt_source_srv_call_psd_handle_state_ready,
    bt_source_srv_call_psd_handle_state_take_audio_src,
    bt_source_srv_call_psd_handle_state_play,
    bt_source_srv_call_psd_handle_state_give_audio_src
};

const static bt_source_srv_call_psd_flag_t g_play_flag_mapping_with_type[] = {
    BT_SOURCE_SRV_CALL_PSD_FLAG_NONE,
    BT_SOURCE_SRV_CALL_PSD_FLAG_UL_PLAY_TRIGGER,
    BT_SOURCE_SRV_CALL_PSD_FLAG_DL_PLAY_TRIGGER,
    BT_SOURCE_SRV_CALL_PSD_FLAG_LINE_IN_PLAY_TRIGGER,
    BT_SOURCE_SRV_CALL_PSD_FLAG_I2S_IN_PLAY_TRIGGER
};

const static bt_source_srv_call_psd_flag_t g_next_play_flag_mapping_with_type[] = {
    BT_SOURCE_SRV_CALL_PSD_FLAG_NONE,
    BT_SOURCE_SRV_CALL_PSD_FLAG_UL_NEXT_REPLAY,
    BT_SOURCE_SRV_CALL_PSD_FLAG_DL_NEXT_REPLAY,
    BT_SOURCE_SRV_CALL_PSD_FLAG_LINE_IN_NEXT_REPLAY,
    BT_SOURCE_SRV_CALL_PSD_FLAG_I2S_IN_NEXT_REPLAY
};

static void bt_source_srv_call_psd_update_state(bt_source_srv_call_pseduo_dev_t *device, bt_source_srv_call_psd_state_t state)
{
    bt_source_srv_assert(device && "update state device is NULL");
    device->state = state;
}

static void bt_source_srv_call_psd_update_sub_state(bt_source_srv_call_pseduo_dev_t *device, bt_source_srv_call_psd_sub_state_t sub_state)
{
    bt_source_srv_assert(device && "update sub state device is NULL");
    device->sub_state = sub_state;
}

static void bt_source_srv_call_psd_update_next_state(bt_source_srv_call_pseduo_dev_t *device, bt_source_srv_call_psd_next_state_t next_state)
{
    bt_source_srv_assert(device && "update next state device is NULL");
    LOG_MSGID_I(source_srv, "[AG][PSD][MGR] device = %02x update next state = %2x", 2, device, next_state);
    device->next_state = next_state;
}

static void bt_source_srv_call_psd_take_audio_src_complete(bt_source_srv_call_pseduo_dev_t *device)
{
    bt_source_srv_assert(device && "take audio source complete device is NULL");
    if (device->codec_type == BT_HFP_CODEC_TYPE_NONE) {
        /* esco was not established, wait esco establish */
        bt_source_srv_call_psd_update_state(device, BT_SOURCE_SRV_CALL_PSD_STATE_PLAY);
        bt_source_srv_call_psd_update_sub_state(device, BT_SOURCE_SRV_CALL_PSD_SUB_STATE_PLAY_IDLE);
        /* run next state in play idle sub state */
        bt_source_srv_call_psd_run_next_state_with_sub_state_play_idle(device);
    } else {
        /* active esco. */
        bt_source_srv_call_audio_controller_config(device->codec_type);
        device->user_callback((void *)device, BT_SOURCE_SRV_CALL_PSD_USER_EVENT_ACTIVATE_SCO, NULL);
        bt_source_srv_call_psd_update_state(device, BT_SOURCE_SRV_CALL_PSD_STATE_PLAY);
        bt_source_srv_call_psd_update_sub_state(device, BT_SOURCE_SRV_CALL_PSD_SUB_STATE_SCO_ACTIVATING);
    }
}

static void bt_source_srv_call_psd_give_audio_src_complete(bt_source_srv_call_pseduo_dev_t *device)
{
    bt_source_srv_call_psd_update_state(device, BT_SOURCE_SRV_CALL_PSD_STATE_READY);
    bt_source_srv_call_psd_update_sub_state(device, BT_SOURCE_SRV_CALL_PSD_SUB_STATE_NONE);
    /* run next state in ready state */
    bt_source_srv_call_psd_run_next_state_with_state_ready(device);
}

static bt_status_t bt_source_srv_call_psd_take_audio_source(bt_source_srv_call_pseduo_dev_t *device)
{
    bt_source_srv_assert(device && "take audio source device is NULL");
    bt_source_srv_call_psd_audio_source_t type = BT_SOURCE_SRV_CALL_PSD_AUDIO_SRC_TYPE_TRANSMITTER;
    bt_source_srv_call_psd_flag_t flag = BT_SOURCE_SRV_CALL_PSD_FLAG_NONE;
    for (type = 0; type < BT_SOURCE_SRV_CALL_PSD_AUDIO_SRC_TYPE_MAX; type++) {
        flag = BT_SOURCE_SRV_CALL_PSD_FLAG_NONE;
        flag |= (bt_source_srv_call_psd_flag_t)(1 << type);
        if (!BT_SOURCE_SRV_CALL_PSD_FLAG_IS_SET(device, flag)) {
            LOG_MSGID_I(source_srv, "[AG][PSD][MGR] take audio source device = %02x, flag = %02x, audio type = %02x", 3,
                        device, device->flags, type);
            switch (type) {
                case BT_SOURCE_SRV_CALL_PSD_AUDIO_SRC_TYPE_TRANSMITTER: {
                    audio_src_srv_resource_manager_take(device->transmitter_audio_src);
                }
                break;
                default:
                    break;
            }
            return BT_STATUS_PENDING;
        }
    }
    /* all audio source take success, update state */
    LOG_MSGID_I(source_srv, "[AG][PSD][MGR] take device = %02x all audio source success", 1, device);
    bt_source_srv_call_psd_take_audio_src_complete(device);
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_call_psd_give_audio_source(bt_source_srv_call_pseduo_dev_t *device)
{
    bt_source_srv_assert(device && "give audio source device is NULL");
    bt_source_srv_call_psd_audio_source_t type = BT_SOURCE_SRV_CALL_PSD_AUDIO_SRC_TYPE_TRANSMITTER;
    bt_source_srv_call_psd_flag_t flag = BT_SOURCE_SRV_CALL_PSD_FLAG_NONE;
    for (type = 0; type < BT_SOURCE_SRV_CALL_PSD_AUDIO_SRC_TYPE_MAX; type++) {
        flag = BT_SOURCE_SRV_CALL_PSD_FLAG_NONE;
        flag |= (bt_source_srv_call_psd_flag_t)(1 << type);
        if (BT_SOURCE_SRV_CALL_PSD_FLAG_IS_SET(device, flag)) {
            LOG_MSGID_I(source_srv, "[AG][PSD][MGR] give audio source device = %02x, flag = %02x, audio type = %02x", 3,
                        device, device->flags, type);
            switch (type) {
                case BT_SOURCE_SRV_CALL_PSD_AUDIO_SRC_TYPE_TRANSMITTER: {
                    audio_src_srv_resource_manager_give(device->transmitter_audio_src);
                }
                break;
                default:
                    break;
            }
            return BT_STATUS_PENDING;
        }
    }
    /* all audio source give success, update state */
    LOG_MSGID_I(source_srv, "[AG][PSD][MGR] give device = %02x all audio source success", 1, device);
    bt_source_srv_call_psd_give_audio_src_complete(device);
    return BT_STATUS_SUCCESS;
}

static bt_source_srv_call_pseduo_dev_t *bt_source_srv_call_psd_find_device_by_handle(bt_source_srv_call_psd_audio_source_t type, void *handle)
{
    uint32_t i = 0;
    for (i = 0; i < BT_SOURCE_SRV_CALL_PSEDUO_DEV_NUM; i++) {
        bt_source_srv_call_pseduo_dev_t *device = &g_source_srv_call_pseduo_dev[i];
        if (((type == BT_SOURCE_SRV_CALL_PSD_AUDIO_SRC_TYPE_TRANSMITTER) && (device->transmitter_audio_src == (audio_src_srv_resource_manager_handle_t *)handle))) {
            return device;
        }
    }
    return NULL;
}

static void bt_source_srv_call_psd_handle_audio_src_take_success(bt_source_srv_call_pseduo_dev_t *device)
{
    BT_SOURCE_SRV_CALL_PSD_SET_FLAG(device, BT_SOURCE_SRV_CALL_PSD_FLAG_TRANSMITTER_TAKED);
    BT_SOURCE_SRV_CALL_PSD_REMOVE_FLAG(device, BT_SOURCE_SRV_CALL_PSD_FLAG_TRANSMITTER_ADD_WL);
    bt_source_srv_call_psd_update_state(device, BT_SOURCE_SRV_CALL_PSD_STATE_TAKE_AUDIO_SRC);
    bt_source_srv_call_psd_take_audio_source(device);
}

static void bt_source_srv_call_psd_handle_audio_src_take_reject(bt_source_srv_call_pseduo_dev_t *device)
{
    audio_src_srv_resource_manager_add_waiting_list(device->transmitter_audio_src);
    bt_source_srv_call_psd_add_waiting_list(device, BT_SOURCE_SRV_CALL_PSD_AUDIO_SRC_TYPE_TRANSMITTER);
    bt_source_srv_call_psd_give_audio_source(device);
}

static void bt_source_srv_call_psd_handle_audio_src_give_success(bt_source_srv_call_pseduo_dev_t *device)
{
    BT_SOURCE_SRV_CALL_PSD_REMOVE_FLAG(device, BT_SOURCE_SRV_CALL_PSD_FLAG_TRANSMITTER_TAKED);
    bt_source_srv_psd_get_esco_state_t esco_state = {0};
    bt_source_srv_psd_get_call_state_t call_state = {0};
    device->user_callback(device, BT_SOURCE_SRV_CALL_PSD_USER_EVENT_GET_CALL_STATE, &call_state);
    device->user_callback(device, BT_SOURCE_SRV_CALL_PSD_USER_EVENT_GET_ESCO_STATE, &esco_state);
    if ((esco_state.state == BT_SOURCE_SRV_CALL_PSD_ESCO_CONNECTED) || (call_state.state == BT_SOURCE_SRV_CALL_PSD_CALL_EXISTENCE)) {
        bt_source_srv_call_psd_add_waiting_list(device, BT_SOURCE_SRV_CALL_PSD_AUDIO_SRC_TYPE_TRANSMITTER);
    }
    bt_source_srv_call_psd_give_audio_source(device);
}

static void bt_source_srv_call_psd_handle_audio_src_suspend(bt_source_srv_call_pseduo_dev_t *device)
{
    device->user_callback((void *)device, BT_SOURCE_SRV_CALL_PSD_USER_EVENT_SUSPEND, NULL);
}

static void bt_source_srv_call_psd_audio_resource_callback(audio_src_srv_resource_manager_handle_t *handle,
        audio_src_srv_resource_manager_event_t event)
{
    bt_source_srv_assert(handle && "audio resource callback handle is NULL");
    bt_source_srv_call_pseduo_dev_t *device = bt_source_srv_call_psd_find_device_by_handle(BT_SOURCE_SRV_CALL_PSD_AUDIO_SRC_TYPE_TRANSMITTER, (void *)handle);
    if (device == NULL) {
        LOG_MSGID_W(source_srv, "[AG][PSD][MGR] audio resource device is NULL by handle = %02x", 1, handle);
        return;
    }

    LOG_MSGID_I(source_srv, "[AG][PSD][MGR] audio resource device = %02x, event = %02x", 2, device, event);
    if ((sizeof(g_audio_src_event_handler) / sizeof(bt_source_srv_audio_src_event_handler_t)) > event) {
        g_audio_src_event_handler[event](device);
    }
}

static void bt_source_srv_call_psd_handle_audio_replay(bt_source_srv_call_pseduo_dev_t *device, bt_source_srv_call_audio_play_t type)
{
    LOG_MSGID_I(source_srv, "[AG][PSD][MGR] audio replay device = %02x, type = %02x", 2, device, type);
    switch (type) {
        case BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_UL: {
            if (BT_SOURCE_SRV_CALL_PSD_FLAG_IS_SET(device, BT_SOURCE_SRV_CALL_PSD_FLAG_UL_PLAY_TRIGGER)) {
                bt_source_srv_call_psd_audio_stop_int(device, type);

                bt_source_srv_call_psd_audio_stop_int(device, BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_DL);
            } else {
                bt_source_srv_call_psd_audio_play_int(device, type);
            }
        }
        break;
        case BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_DL: {
            if (BT_SOURCE_SRV_CALL_PSD_FLAG_IS_SET(device, BT_SOURCE_SRV_CALL_PSD_FLAG_DL_PLAY_TRIGGER)) {
                /**
                 * type:DSP workaround
                 * root cause:because DSP UL variable rely on DL variable, so stop DL will influence UL flow.
                 * solution:stop DL synchronizes stop UL.
                 */
                bt_source_srv_call_psd_audio_stop_int(device, BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_UL);

                bt_source_srv_call_psd_audio_stop_int(device, type);
            }
        }
        break;
        case BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_LINE_IN: {
            /* audio play line in*/
            bt_source_srv_call_psd_audio_play_int(device, BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_LINE_IN);
        }
        break;
        case BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_I2S_IN: {
            /* audio play i2s in*/
            bt_source_srv_call_psd_audio_play_int(device, BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_I2S_IN);
        }
        break;
        default:
            break;
    }

    if ((device->sub_state == BT_SOURCE_SRV_CALL_PSD_SUB_STATE_PLAY_STARTING) ||
            (device->sub_state == BT_SOURCE_SRV_CALL_PSD_SUB_STATE_PLAY_STOPPING)) {
        BT_SOURCE_SRV_CALL_PSD_SET_FLAG(device, BT_SOURCE_SRV_CALL_PSD_FLAG_AUDIO_REPLAYING);
    }
}

static void bt_source_srv_call_psd_add_waiting_list(bt_source_srv_call_pseduo_dev_t *device, bt_source_srv_call_psd_audio_source_t type)
{
    bt_source_srv_assert(device && "add waiting list device is NULL");
    LOG_MSGID_I(source_srv, "[AG][PSD][MGR] add waiting list device = %02x, type = %02x", 2, device, type);
    switch (type) {
        case BT_SOURCE_SRV_CALL_PSD_AUDIO_SRC_TYPE_TRANSMITTER: {
            if (!BT_SOURCE_SRV_CALL_PSD_FLAG_IS_SET(device, BT_SOURCE_SRV_CALL_PSD_FLAG_TRANSMITTER_ADD_WL)) {
                BT_SOURCE_SRV_CALL_PSD_SET_FLAG(device, BT_SOURCE_SRV_CALL_PSD_FLAG_TRANSMITTER_ADD_WL);
                audio_src_srv_resource_manager_add_waiting_list(device->transmitter_audio_src);
            }
        }
        break;
        default:
            break;
    }

}

static void bt_source_srv_call_psd_del_waiting_list(bt_source_srv_call_pseduo_dev_t *device, bt_source_srv_call_psd_audio_source_t type)
{
    bt_source_srv_assert(device && "add waiting list device is NULL");
    LOG_MSGID_I(source_srv, "[AG][PSD][MGR] del waiting list device = %02x, type = %02x", 2, device, type);
    switch (type) {
        case BT_SOURCE_SRV_CALL_PSD_AUDIO_SRC_TYPE_TRANSMITTER: {
            if (BT_SOURCE_SRV_CALL_PSD_FLAG_IS_SET(device, BT_SOURCE_SRV_CALL_PSD_FLAG_TRANSMITTER_ADD_WL)) {
                BT_SOURCE_SRV_CALL_PSD_REMOVE_FLAG(device, BT_SOURCE_SRV_CALL_PSD_FLAG_TRANSMITTER_ADD_WL);
                audio_src_srv_resource_manager_delete_waiting_list(device->transmitter_audio_src);
            }
        }
        break;
        default:
            break;
    }

}

static bt_status_t bt_source_srv_call_psd_audio_play_int(bt_source_srv_call_pseduo_dev_t *device, bt_source_srv_call_audio_play_t type)
{
    bt_source_srv_call_audio_config_t config = {
        .type = type
    };

    if (BT_SOURCE_SRV_CALL_PSD_FLAG_IS_SET(device, g_play_flag_mapping_with_type[type])) {
        return BT_SOURCE_SRV_CALL_PSD_FLAG_IS_SET(device, BT_SOURCE_SRV_CALL_PSD_FLAG_AUDIO_REPLAYING) ? BT_STATUS_SUCCESS : BT_STATUS_FAIL;
    }

    device->audio_id[type] = bt_source_srv_call_audio_config_init(&config, device->codec_type, (void *)device, \
                             (void *)bt_source_srv_call_psd_audio_transmitter_callback);
    if (device->audio_id[type] == BT_SOURCE_SRV_CALL_AUDIO_INVALID_ID) {
        /* For DL audio config fail, switch state to ready, DL must play success. */
        if (BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_DL == type) {
            device->user_callback((void *)device, BT_SOURCE_SRV_CALL_PSD_USER_EVENT_DEACTIVATE_SCO, NULL);
            bt_source_srv_call_psd_update_sub_state(device, BT_SOURCE_SRV_CALL_PSD_SUB_STATE_SCO_DEACTIVATING);
            bt_source_srv_call_psd_update_next_state(device, BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_READY);
        }
        return BT_STATUS_FAIL;
    }

    switch (type) {
        case BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_UL: {
            bt_source_srv_call_psd_mic_location_t mic_location = {0};
            device->user_callback((void *)device, BT_SOURCE_SRV_CALL_PSD_USER_EVENT_GET_MIC_LOCATION, &mic_location);
            if ((mic_location.location != BT_SOURCE_SRV_CALL_PSD_LOCATION_LOCAL) &&
                    (bt_source_srv_call_audio_play(device->audio_id[type]) == BT_STATUS_SUCCESS)) {
                device->audio_play_number++;
                BT_SOURCE_SRV_CALL_PSD_SET_FLAG(device, BT_SOURCE_SRV_CALL_PSD_FLAG_UL_PLAY_TRIGGER);
                bt_source_srv_call_psd_update_sub_state(device, BT_SOURCE_SRV_CALL_PSD_SUB_STATE_PLAY_STARTING);
            } else {
                bt_source_srv_call_audio_config_deinit(device->audio_id[type]);
            }
        }
        break;
        case BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_DL: {
            if (bt_source_srv_call_audio_play(device->audio_id[type]) != BT_STATUS_SUCCESS) {
                /* DL play fail, switch state to ready, DL must play success. */
                device->user_callback((void *)device, BT_SOURCE_SRV_CALL_PSD_USER_EVENT_DEACTIVATE_SCO, NULL);
                bt_source_srv_call_psd_update_sub_state(device, BT_SOURCE_SRV_CALL_PSD_SUB_STATE_SCO_DEACTIVATING);
                bt_source_srv_call_psd_update_next_state(device, BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_READY);
                bt_source_srv_call_audio_config_deinit(device->audio_id[type]);
                return BT_STATUS_FAIL;
            } else {
                device->audio_play_number++;
                BT_SOURCE_SRV_CALL_PSD_SET_FLAG(device, BT_SOURCE_SRV_CALL_PSD_FLAG_DL_PLAY_TRIGGER);
                bt_source_srv_call_psd_update_sub_state(device, BT_SOURCE_SRV_CALL_PSD_SUB_STATE_PLAY_STARTING);
            }
        }
        break;
        case BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_LINE_IN:
        case BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_I2S_IN: {
            if (bt_source_srv_call_audio_play(device->audio_id[type]) == BT_STATUS_SUCCESS) {
                device->audio_play_number++;
                BT_SOURCE_SRV_CALL_PSD_SET_FLAG(device, g_play_flag_mapping_with_type[type]);
                bt_source_srv_call_psd_update_sub_state(device, BT_SOURCE_SRV_CALL_PSD_SUB_STATE_PLAY_STARTING);
            } else {
                bt_source_srv_call_audio_config_deinit(device->audio_id[type]);
            }
        }
        break;
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}

static void bt_source_srv_call_psd_audio_play(bt_source_srv_call_pseduo_dev_t *device)
{
    bt_source_srv_assert(device && "take codec device is NULL");
    LOG_MSGID_I(source_srv, "[AG][PSD][MGR] device:%02x audio play codec type = %02x", 2, device, device->codec_type);

    if (device->codec_type != BT_HFP_CODEC_TYPE_NONE) {
        if (bt_source_srv_call_psd_audio_play_int(device, BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_DL) != BT_STATUS_SUCCESS) {
            return;
        }

        bt_source_srv_call_psd_audio_play_int(device, BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_UL);
        bt_source_srv_call_psd_audio_play_int(device, BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_LINE_IN);
        bt_source_srv_call_psd_audio_play_int(device, BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_I2S_IN);
    } else {
        bt_source_srv_assert(device && "audio play codec type is none");
    }
}

static bt_status_t bt_source_srv_call_psd_audio_stop_int(bt_source_srv_call_pseduo_dev_t *device, bt_source_srv_call_audio_play_t type)
{
    bt_status_t status = BT_STATUS_FAIL;

    if (!BT_SOURCE_SRV_CALL_PSD_FLAG_IS_SET(device, g_play_flag_mapping_with_type[type])) {
        return BT_STATUS_FAIL;
    }

    switch (type) {
        case BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_UL:
        case BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_DL:
        case BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_LINE_IN:
        case BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_I2S_IN: {
            status = bt_source_srv_call_audio_stop(device->audio_id[type]);
            if (status == BT_STATUS_SUCCESS) {
                device->audio_play_number--;
                BT_SOURCE_SRV_CALL_PSD_REMOVE_FLAG(device, g_play_flag_mapping_with_type[type]);
                bt_source_srv_call_psd_update_sub_state(device, BT_SOURCE_SRV_CALL_PSD_SUB_STATE_PLAY_STOPPING);
            }
        }
        break;
        default:
            break;
    }
    return status;
}

static void bt_source_srv_call_psd_audio_stop(bt_source_srv_call_pseduo_dev_t *device)
{
    bt_source_srv_assert(device && "audio stop device is NULL");
    LOG_MSGID_I(source_srv, "[AG][PSD][MGR] device:%02x audio stop codec type = %02x", 2, device, device->codec_type);

    bt_source_srv_call_audio_slience_detection_stop(device->audio_id[BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_DL]);

    bt_source_srv_call_psd_audio_stop_int(device, BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_UL);
    bt_source_srv_call_psd_audio_stop_int(device, BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_DL);
    bt_source_srv_call_psd_audio_stop_int(device, BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_LINE_IN);
    bt_source_srv_call_psd_audio_stop_int(device, BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_I2S_IN);
}

static bt_status_t bt_source_srv_call_psd_run_next_state_with_state_ready(bt_source_srv_call_pseduo_dev_t *device)
{
    switch (device->next_state) {
        case BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_PLAY: {
            bt_source_srv_call_psd_update_state(device, BT_SOURCE_SRV_CALL_PSD_STATE_TAKE_AUDIO_SRC);
            bt_source_srv_call_psd_take_audio_source(device);
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_NONE: {
            bt_source_srv_call_psd_update_state(device, BT_SOURCE_SRV_CALL_PSD_STATE_NONE);
            bt_source_srv_call_psd_update_next_state(device, BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_INIT);
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_READY: {
            bt_source_srv_call_psd_update_next_state(device, BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_INIT);
        }
        break;
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_call_psd_handle_sub_state_none(bt_source_srv_call_pseduo_dev_t *device, bt_source_srv_call_psd_event_t event, void *parameter)
{
    switch (device->state) {
        case BT_SOURCE_SRV_CALL_PSD_STATE_NONE: {
            switch (event) {
                case BT_SOURCE_SRV_CALL_PSD_EVENT_LINK_CONNECT_REQ_IND:
                case BT_SOURCE_SRV_CALL_PSD_EVENT_LINK_CONNECT_REQ: {
                    bt_source_srv_call_psd_update_sub_state(device, BT_SOURCE_SRV_CALL_PSD_SUB_STATE_CONNECTING);
                }
                break;
                default:
                    break;
            }
        };
        break;
        case BT_SOURCE_SRV_CALL_PSD_STATE_READY: {
            switch (event) {
                case BT_SOURCE_SRV_CALL_PSD_EVENT_LINK_DISCONNECT_REQ: {
                    bt_source_srv_call_psd_update_sub_state(device, BT_SOURCE_SRV_CALL_PSD_SUB_STATE_DISCONNECTING);
                }
                break;
                case BT_SOURCE_SRV_CALL_PSD_EVENT_LINK_DISCONNECTED: {
                    /* delete waiting list device. */
                    bt_source_srv_call_psd_del_waiting_list(device, BT_SOURCE_SRV_CALL_PSD_AUDIO_SRC_TYPE_TRANSMITTER);
                    bt_source_srv_call_psd_update_state(device, BT_SOURCE_SRV_CALL_PSD_STATE_NONE);
                }
                break;
                case BT_SOURCE_SRV_CALL_PSD_EVENT_SCO_DISCONNECTED: {
                    /* delete waiting list device. */
                    bt_source_srv_call_psd_del_waiting_list(device, BT_SOURCE_SRV_CALL_PSD_AUDIO_SRC_TYPE_TRANSMITTER);
                    bt_source_srv_call_psd_update_next_state(device, BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_INIT);
                }
                break;
                case BT_SOURCE_SRV_CALL_PSD_EVENT_SCO_CONNECTED:
                case BT_SOURCE_SRV_CALL_PSD_EVENT_CALL_START: {
                    bt_source_srv_call_psd_update_state(device, BT_SOURCE_SRV_CALL_PSD_STATE_TAKE_AUDIO_SRC);
                    bt_source_srv_call_psd_take_audio_source(device);
                }
                break;
                case BT_SOURCE_SRV_CALL_PSD_EVENT_CALL_END: {
                    /* delete waiting list device. */
                    bt_source_srv_call_psd_del_waiting_list(device, BT_SOURCE_SRV_CALL_PSD_AUDIO_SRC_TYPE_TRANSMITTER);
                }
                break;
                case BT_SOURCE_SRV_CALL_PSD_EVENT_AUDIO_REPLAY_REQ: {
                    bt_source_srv_call_psd_update_state(device, BT_SOURCE_SRV_CALL_PSD_STATE_TAKE_AUDIO_SRC);
                    bt_source_srv_call_psd_take_audio_source(device);
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

static bt_status_t bt_source_srv_call_psd_handle_sub_state_connecting(bt_source_srv_call_pseduo_dev_t *device, bt_source_srv_call_psd_event_t event, void *parameter)
{
    switch (event) {
        case BT_SOURCE_SRV_CALL_PSD_EVENT_LINK_CONNECTED: {
            bt_source_srv_call_psd_update_state(device, BT_SOURCE_SRV_CALL_PSD_STATE_READY);
            bt_source_srv_call_psd_update_sub_state(device, BT_SOURCE_SRV_CALL_PSD_SUB_STATE_NONE);
            bt_source_srv_call_psd_run_next_state_with_state_ready(device);
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_EVENT_LINK_DISCONNECTED: {
            bt_source_srv_call_psd_update_state(device, BT_SOURCE_SRV_CALL_PSD_STATE_NONE);
            bt_source_srv_call_psd_update_sub_state(device, BT_SOURCE_SRV_CALL_PSD_SUB_STATE_NONE);
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_EVENT_CALL_START:
        case BT_SOURCE_SRV_CALL_PSD_EVENT_SCO_CONNECTED: {
            bt_source_srv_call_psd_update_next_state(device, BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_PLAY);
        }
        break;
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_call_psd_handle_state_none(bt_source_srv_call_pseduo_dev_t *device, bt_source_srv_call_psd_event_t event, void *parameter)
{
    bt_status_t status = BT_STATUS_FAIL;
    switch (device->sub_state) {
        case BT_SOURCE_SRV_CALL_PSD_SUB_STATE_NONE: {
            status = bt_source_srv_call_psd_handle_sub_state_none(device, event, parameter);
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_SUB_STATE_CONNECTING: {
            status = bt_source_srv_call_psd_handle_sub_state_connecting(device, event, parameter);
        }
        break;
        default:
            break;
    }
    return status;
}

static bt_status_t bt_source_srv_call_psd_handle_sub_state_disconnecting(bt_source_srv_call_pseduo_dev_t *device, bt_source_srv_call_psd_event_t event, void *parameter)
{
    switch (event) {
        case BT_SOURCE_SRV_CALL_PSD_EVENT_LINK_DISCONNECTED: {
            bt_source_srv_call_psd_update_state(device, BT_SOURCE_SRV_CALL_PSD_STATE_NONE);
            bt_source_srv_call_psd_update_sub_state(device, BT_SOURCE_SRV_CALL_PSD_SUB_STATE_NONE);
        }
        break;
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_call_psd_handle_state_ready(bt_source_srv_call_pseduo_dev_t *device, bt_source_srv_call_psd_event_t event, void *parameter)
{
    bt_status_t status = BT_STATUS_FAIL;
    switch (device->sub_state) {
        case BT_SOURCE_SRV_CALL_PSD_SUB_STATE_NONE: {
            status = bt_source_srv_call_psd_handle_sub_state_none(device, event, parameter);
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_SUB_STATE_DISCONNECTING: {
            status = bt_source_srv_call_psd_handle_sub_state_disconnecting(device, event, parameter);
        }
        break;
        default:
            break;
    }
    return status;
}

static bt_status_t bt_source_srv_call_psd_handle_state_take_audio_src(bt_source_srv_call_pseduo_dev_t *device, bt_source_srv_call_psd_event_t event, void *parameter)
{
    switch (event) {
        case BT_SOURCE_SRV_CALL_PSD_EVENT_LINK_DISCONNECT_REQ:
        case BT_SOURCE_SRV_CALL_PSD_EVENT_LINK_DISCONNECTED: {
            bt_source_srv_call_psd_update_next_state(device, BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_NONE);
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_EVENT_SCO_DISCONNECTED: {
            bt_source_srv_call_psd_update_next_state(device, BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_READY);
        }
        break;
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}


static bt_status_t bt_source_srv_call_psd_handle_state_give_audio_src(bt_source_srv_call_pseduo_dev_t *device, bt_source_srv_call_psd_event_t event, void *parameter)
{
    switch (event) {
        case BT_SOURCE_SRV_CALL_PSD_EVENT_LINK_CONNECTED: {
            bt_source_srv_call_psd_update_next_state(device, BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_READY);
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_EVENT_CALL_START:
        case BT_SOURCE_SRV_CALL_PSD_EVENT_SCO_CONNECTED: {
            bt_source_srv_call_psd_update_next_state(device, BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_PLAY);
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_EVENT_LINK_DISCONNECTED: {
            bt_source_srv_call_psd_update_next_state(device, BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_NONE);
        }
        break;
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_call_psd_run_next_state_with_sub_state_play_idle(bt_source_srv_call_pseduo_dev_t *device)
{
    switch (device->next_state) {
        case BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_NONE:
        case BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_READY: {
            bt_source_srv_call_psd_give_audio_source(device);
        }
        break;
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_call_psd_handle_sub_state_play_idle(bt_source_srv_call_pseduo_dev_t *device, bt_source_srv_call_psd_event_t event, void *parameter)
{
    switch (event) {
        case BT_SOURCE_SRV_CALL_PSD_EVENT_LINK_DISCONNECTED: {
            /* update state */
            bt_source_srv_call_psd_update_state(device, BT_SOURCE_SRV_CALL_PSD_STATE_GIVE_AUDIO_SRC);
            bt_source_srv_call_psd_update_sub_state(device, BT_SOURCE_SRV_CALL_PSD_SUB_STATE_NONE);
            bt_source_srv_call_psd_update_next_state(device, BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_NONE);
            /* give audio resource */
            bt_source_srv_call_psd_give_audio_source(device);
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_EVENT_SCO_CONNECTED: {
            /* activate sco */
            bt_source_srv_call_audio_controller_config(device->codec_type);
            device->user_callback((void *)device, BT_SOURCE_SRV_CALL_PSD_USER_EVENT_ACTIVATE_SCO, NULL);
            bt_source_srv_call_psd_update_sub_state(device, BT_SOURCE_SRV_CALL_PSD_SUB_STATE_SCO_ACTIVATING);
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_EVENT_CALL_END: {
            /* update state */
            bt_source_srv_call_psd_update_state(device, BT_SOURCE_SRV_CALL_PSD_STATE_GIVE_AUDIO_SRC);
            bt_source_srv_call_psd_update_sub_state(device, BT_SOURCE_SRV_CALL_PSD_SUB_STATE_NONE);
            bt_source_srv_call_psd_update_next_state(device, BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_READY);
            /* give audio resource */
            bt_source_srv_call_psd_give_audio_source(device);
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_EVENT_SUSPEND_REQ: {
            bt_source_srv_call_psd_update_state(device, BT_SOURCE_SRV_CALL_PSD_STATE_GIVE_AUDIO_SRC);
            bt_source_srv_call_psd_update_sub_state(device, BT_SOURCE_SRV_CALL_PSD_SUB_STATE_NONE);
            bt_source_srv_call_psd_update_next_state(device, BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_READY);
            /* suspend will give audio source */
            bt_source_srv_call_psd_give_audio_source(device);
        }
        break;
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_call_psd_handle_sub_state_sco_activating(bt_source_srv_call_pseduo_dev_t *device, bt_source_srv_call_psd_event_t event, void *parameter)
{
    switch (event) {
        case BT_SOURCE_SRV_CALL_PSD_EVENT_SCO_ACTIVATED: {
            /* sco activate success, take codec */
            bt_source_srv_call_psd_audio_play(device);
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_EVENT_SCO_DISCONNECTED: {
            /* give audio source */
            bt_source_srv_call_psd_give_audio_source(device);
        }
        break;
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_call_psd_run_next_state_with_sub_state_playing(bt_source_srv_call_pseduo_dev_t *device)
{
    switch (device->next_state) {
        case BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_NONE:
        case BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_READY: {
            bt_source_srv_call_psd_audio_stop(device);
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_PLAY: {
            bt_source_srv_call_psd_update_next_state(device, BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_INIT);
        }
        break;
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_call_psd_handle_sub_state_play_starting(bt_source_srv_call_pseduo_dev_t *device, bt_source_srv_call_psd_event_t event, void *parameter)
{
    switch (event) {
        case BT_SOURCE_SRV_CALL_PSD_EVENT_AUDIO_PLAY_IND: {
            bool is_replay_ind = false;
            if (BT_SOURCE_SRV_CALL_PSD_FLAG_IS_SET(device, BT_SOURCE_SRV_CALL_PSD_FLAG_AUDIO_REPLAYING)) {
                BT_SOURCE_SRV_CALL_PSD_REMOVE_FLAG(device, BT_SOURCE_SRV_CALL_PSD_FLAG_AUDIO_REPLAYING);
                is_replay_ind = true;
            }
            /* take codec success, update sub state*/
            bt_source_srv_call_psd_update_sub_state(device, BT_SOURCE_SRV_CALL_PSD_SUB_STATE_PLAYING);
            /* run next state */
            bt_source_srv_call_psd_run_next_state_with_sub_state_playing(device);

            /* replay audio for port parameter change. */
            if (device->sub_state == BT_SOURCE_SRV_CALL_PSD_SUB_STATE_PLAYING) {
                for (uint32_t i = 1; i < BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_MAX; i++) {
                    if (BT_SOURCE_SRV_CALL_PSD_FLAG_IS_SET(device, g_next_play_flag_mapping_with_type[i])) {
                        BT_SOURCE_SRV_CALL_PSD_REMOVE_FLAG(device, g_next_play_flag_mapping_with_type[i]);
                        bt_source_srv_call_psd_handle_audio_replay(device, i);
                    }
                }
            }

            if ((!is_replay_ind) && (device->sub_state == BT_SOURCE_SRV_CALL_PSD_SUB_STATE_PLAYING)) {
                device->user_callback((void *)device, BT_SOURCE_SRV_CALL_PSD_USER_EVENT_AUDIO_PLAY_COMPLETE, NULL);
            }
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_EVENT_LINK_DISCONNECTED: {
            bt_source_srv_call_psd_update_next_state(device, BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_NONE);
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_EVENT_SCO_DISCONNECTED: {
            bt_source_srv_call_psd_update_next_state(device, BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_READY);
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_EVENT_AUDIO_REPLAY_REQ: {
            bt_source_srv_call_psd_audio_replay_req_t *audio_replay_req = ( bt_source_srv_call_psd_audio_replay_req_t *)parameter;
            if ((audio_replay_req->port_action == BT_SOURCE_SRV_CALL_PSD_CALL_ACTION_UPDATE) ||
                    (audio_replay_req->port_action == BT_SOURCE_SRV_CALL_PSD_CALL_ACTION_OPEN)) {
                BT_SOURCE_SRV_CALL_PSD_SET_FLAG(device, g_next_play_flag_mapping_with_type[audio_replay_req->type]);
            }
        }
        break;
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_call_psd_handle_sub_state_playing(bt_source_srv_call_pseduo_dev_t *device, bt_source_srv_call_psd_event_t event, void *parameter)
{
    switch (event) {
        case BT_SOURCE_SRV_CALL_PSD_EVENT_LINK_DISCONNECTED: {
            bt_source_srv_call_psd_update_next_state(device, BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_NONE);
            bt_source_srv_call_psd_audio_stop(device);
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_EVENT_SCO_DISCONNECTED:
        case BT_SOURCE_SRV_CALL_PSD_EVENT_SUSPEND_REQ: {
            bt_source_srv_call_psd_audio_stop(device);
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_EVENT_CALL_END: {
            bt_source_srv_call_psd_call_end_t *call_end = (bt_source_srv_call_psd_call_end_t *)parameter;
            if (call_end->is_allow_audio_stop) {
                /* give codec */
                bt_source_srv_call_psd_audio_stop(device);
            } else {
                LOG_MSGID_W(source_srv, "[AG][PSD][MGR] device = %02x call end disallow audio stop", 1, device);
            }
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_EVENT_AUDIO_REPLAY_REQ: {
            bt_source_srv_call_psd_audio_replay_req_t *audio_replay_req = ( bt_source_srv_call_psd_audio_replay_req_t *)parameter;

            LOG_MSGID_I(source_srv, "[AG][PSD][MGR] audio playing repay type = %02x, action = %02x", 2, audio_replay_req->type, audio_replay_req->port_action);
            if ((audio_replay_req->port_action == BT_SOURCE_SRV_CALL_PSD_CALL_ACTION_OPEN) &&
                    (BT_SOURCE_SRV_CALL_PSD_FLAG_IS_SET(device, g_play_flag_mapping_with_type[audio_replay_req->type]))) {
                /* Ignore replay request when port open and audio had start. */
                break;
            }

            bt_source_srv_call_psd_handle_audio_replay(device, audio_replay_req->type);
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_EVENT_AUDIO_STOP_REQ: {
            bt_source_srv_call_psd_audio_stop_req_t *audio_stop_req = ( bt_source_srv_call_psd_audio_stop_req_t *)parameter;
            if ((BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_UL == audio_stop_req->type) || ((BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_DL == audio_stop_req->type))) {
                bt_source_srv_call_psd_update_next_state(device, BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_READY);
                bt_source_srv_call_psd_audio_stop(device);
            }
        }
        break;
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_call_psd_handle_sub_state_play_stopping(bt_source_srv_call_pseduo_dev_t *device, bt_source_srv_call_psd_event_t event, void *parameter)
{
    switch (event) {
        case BT_SOURCE_SRV_CALL_PSD_EVENT_AUDIO_STOP_IND: {
            /* Because audio id number is 16, so free audio id after stop audio success */
            if (!BT_SOURCE_SRV_CALL_PSD_FLAG_IS_SET(device, BT_SOURCE_SRV_CALL_PSD_FLAG_UL_PLAY_TRIGGER)) {
                bt_source_srv_call_audio_config_deinit(device->audio_id[BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_UL]);
            }

            if (!BT_SOURCE_SRV_CALL_PSD_FLAG_IS_SET(device, BT_SOURCE_SRV_CALL_PSD_FLAG_DL_PLAY_TRIGGER)) {
                bt_source_srv_call_audio_config_deinit(device->audio_id[BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_DL]);
            }

            if (!BT_SOURCE_SRV_CALL_PSD_FLAG_IS_SET(device, BT_SOURCE_SRV_CALL_PSD_FLAG_LINE_IN_PLAY_TRIGGER)) {
                bt_source_srv_call_audio_config_deinit(device->audio_id[BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_LINE_IN]);
            }

            if (!BT_SOURCE_SRV_CALL_PSD_FLAG_IS_SET(device, BT_SOURCE_SRV_CALL_PSD_FLAG_I2S_IN_PLAY_TRIGGER)) {
                bt_source_srv_call_audio_config_deinit(device->audio_id[BT_SOURCE_SRV_CALL_AUDIO_PLAY_TYPE_I2S_IN]);
            }

            /**
             * type:source issue
             * root cause:device is not replay when port parameter had changem,cause UL/DL no sound.
             * solution:replay audio(first stop, then play) when port parameter change.
             */
            if (BT_SOURCE_SRV_CALL_PSD_FLAG_IS_SET(device, BT_SOURCE_SRV_CALL_PSD_FLAG_AUDIO_REPLAYING)) {
                bt_source_srv_call_psd_audio_play(device);
                break;
            }

            device->user_callback((void *)device, BT_SOURCE_SRV_CALL_PSD_USER_EVENT_AUDIO_STOP_COMPLETE, NULL);

            if (device->codec_type != BT_HFP_CODEC_TYPE_NONE) {
                /* codec give success, deactivate sco */
                device->user_callback((void *)device, BT_SOURCE_SRV_CALL_PSD_USER_EVENT_DEACTIVATE_SCO, NULL);
                bt_source_srv_call_psd_update_sub_state(device, BT_SOURCE_SRV_CALL_PSD_SUB_STATE_SCO_DEACTIVATING);
            } else {
                bt_source_srv_call_psd_update_state(device, BT_SOURCE_SRV_CALL_PSD_STATE_GIVE_AUDIO_SRC);
                bt_source_srv_call_psd_update_sub_state(device, BT_SOURCE_SRV_CALL_PSD_SUB_STATE_NONE);
                /* give audio source */
                bt_source_srv_call_psd_give_audio_source(device);

            }
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_EVENT_LINK_CONNECTED: {
            bt_source_srv_call_psd_update_next_state(device, BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_READY);
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_EVENT_LINK_DISCONNECTED: {
            bt_source_srv_call_psd_update_next_state(device, BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_NONE);
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_EVENT_SCO_DISCONNECTED: {
            /**
             * type:source issue
             * root cause:device is audio stopping for replay when esco disconnect, the state machine will be blocked.
             * solution:clear audio replaying flag when device receive esco disconnect event in audio stopping state.
             */
            bt_source_srv_call_psd_update_next_state(device, BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_READY);
            if (BT_SOURCE_SRV_CALL_PSD_FLAG_IS_SET(device, BT_SOURCE_SRV_CALL_PSD_FLAG_AUDIO_REPLAYING)) {
                BT_SOURCE_SRV_CALL_PSD_REMOVE_FLAG(device, BT_SOURCE_SRV_CALL_PSD_FLAG_AUDIO_REPLAYING);
            }
        }
        break;
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_call_psd_handle_sub_state_sco_deactivating(bt_source_srv_call_pseduo_dev_t *device, bt_source_srv_call_psd_event_t event, void *parameter)
{
    switch (event) {
        case BT_SOURCE_SRV_CALL_PSD_EVENT_SCO_DEACTIVATED: {
            /* give audio source */
            bt_source_srv_call_psd_update_state(device, BT_SOURCE_SRV_CALL_PSD_STATE_GIVE_AUDIO_SRC);
            bt_source_srv_call_psd_update_sub_state(device, BT_SOURCE_SRV_CALL_PSD_SUB_STATE_NONE);
            bt_source_srv_call_psd_give_audio_source(device);
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_EVENT_SCO_DISCONNECTED: {
            /* because sco disonnected, so will cannot receive sco activated event. give audio source */
            bt_source_srv_call_psd_update_state(device, BT_SOURCE_SRV_CALL_PSD_STATE_GIVE_AUDIO_SRC);
            bt_source_srv_call_psd_update_sub_state(device, BT_SOURCE_SRV_CALL_PSD_SUB_STATE_NONE);
            bt_source_srv_call_psd_give_audio_source(device);
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_EVENT_LINK_CONNECTED: {
            bt_source_srv_call_psd_update_next_state(device, BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_READY);
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_EVENT_LINK_DISCONNECTED: {
            bt_source_srv_call_psd_update_next_state(device, BT_SOURCE_SRV_CALL_PSD_NEXT_STATE_NONE);
        }
        break;
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_source_srv_call_psd_handle_state_play(bt_source_srv_call_pseduo_dev_t *device, bt_source_srv_call_psd_event_t event, void *parameter)
{
    bt_status_t status = BT_STATUS_FAIL;
    switch (device->sub_state) {
        case BT_SOURCE_SRV_CALL_PSD_SUB_STATE_PLAY_IDLE: {
            status = bt_source_srv_call_psd_handle_sub_state_play_idle(device, event, parameter);
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_SUB_STATE_SCO_ACTIVATING: {
            status = bt_source_srv_call_psd_handle_sub_state_sco_activating(device, event, parameter);
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_SUB_STATE_PLAY_STARTING: {
            status = bt_source_srv_call_psd_handle_sub_state_play_starting(device, event, parameter);
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_SUB_STATE_PLAYING: {
            status = bt_source_srv_call_psd_handle_sub_state_playing(device, event, parameter);
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_SUB_STATE_PLAY_STOPPING: {
            status = bt_source_srv_call_psd_handle_sub_state_play_stopping(device, event, parameter);
        }
        break;
        case BT_SOURCE_SRV_CALL_PSD_SUB_STATE_SCO_DEACTIVATING: {
            status = bt_source_srv_call_psd_handle_sub_state_sco_deactivating(device, event, parameter);
        }
        break;
        default:
            break;
    }
    return status;
}

bt_status_t bt_source_srv_call_psd_alloc_audio_src(bt_source_srv_call_pseduo_dev_t *device)
{
    bt_source_srv_assert(device && "alloc audio src device is NULL");

    device->transmitter_audio_src = audio_src_srv_resource_manager_construct_handle(AUDIO_SRC_SRV_RESOURCE_TYPE_BT_SOURCE, AUDIO_SRC_SRV_RESOURCE_TYPE_BT_SOURCE_USER_HFP);
    if (device->transmitter_audio_src == NULL) {
        LOG_MSGID_E(source_srv, "[AG][PSD][MGR] construct audio source(transmitter) handle fail", 0);
        return BT_STATUS_FAIL;
    }

    /* set audio resource callback. */
    device->transmitter_audio_src->callback_func = bt_source_srv_call_psd_audio_resource_callback;
    /* set audio resource priority. */
    device->transmitter_audio_src->priority = AUDIO_SRC_SRV_RESOURCE_TYPE_BT_SOURCE_USER_HFP_PRIORIRTY;

    return BT_STATUS_SUCCESS;
}

bt_status_t bt_source_srv_call_psd_free_audio_src(bt_source_srv_call_pseduo_dev_t *device)
{
    bt_source_srv_assert(device && "free audio src device is NULL");

    audio_src_srv_resource_manager_destruct_handle(device->transmitter_audio_src);
    device->transmitter_audio_src = NULL;
    return BT_STATUS_SUCCESS;
}

bt_status_t bt_source_srv_call_psd_state_machine(bt_source_srv_call_pseduo_dev_t *device,
        bt_source_srv_call_psd_event_t event, void *parameter)
{
    bt_source_srv_assert(device && "psd device is NULL");
    bt_status_t status = BT_STATUS_FAIL;
    LOG_MSGID_I(source_srv, "[AG][PSD][MGR] state machine in  device:%02x, state:%02x, sub_state:%02x, event:%02x", 4,
                device, device->state, device->sub_state, event);

    status = g_state_machine_handler[device->state](device, event, parameter);

    LOG_MSGID_I(source_srv, "[AG][PSD][MGR] state machine out device:%02x, state:%02x, sub_state:%02x, event:%02x", 4,
                device, device->state, device->sub_state, event);

    if ((device->state == BT_SOURCE_SRV_CALL_PSD_STATE_NONE) && (device->sub_state == BT_SOURCE_SRV_CALL_PSD_SUB_STATE_NONE)) {
        bt_source_srv_call_psd_del_waiting_list(device, BT_SOURCE_SRV_CALL_PSD_AUDIO_SRC_TYPE_TRANSMITTER);
        device->user_callback((void *)device, BT_SOURCE_SRV_CALL_PSD_USER_EVENT_DEINIT, NULL);
    }
    return status;
}
