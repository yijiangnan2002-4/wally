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

#include "bt_sink_srv_state_manager.h"
#include "bt_sink_srv_state_manager_internal.h"

static void bt_sink_srv_state_manager_suspend_callback(
    audio_src_srv_handle_t *handle,
    audio_src_srv_handle_t *interrupt);
static void bt_sink_srv_state_manager_exception_callback(
    audio_src_srv_handle_t *handle,
    int32_t event,
    void *parameter);
static void bt_sink_srv_state_manager_set_next_state(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_manager_device_t *device,
    bt_sink_srv_state_manager_next_state_t next_state);
static void bt_sink_srv_state_manager_run_next_state(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_manager_device_t *device);

static void bt_sink_srv_state_manager_play_callback(audio_src_srv_handle_t *handle);
static void bt_sink_srv_state_manager_stop_callback(audio_src_srv_handle_t *handle);
static void bt_sink_srv_state_manager_reject_callback(audio_src_srv_handle_t *handle);

void bt_sink_srv_state_manager_alloc_psedev(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_manager_device_t *device)
{
    device->dummy_device = audio_src_srv_construct_handle(AUDIO_SRC_SRV_PSEUDO_DEVICE_DUMMY);
    bt_utils_assert(device->dummy_device && "Cannot construct a psedev");

    device->dummy_device->priority = AUDIO_SRC_SRV_PRIORITY_HIGH;
    device->dummy_device->play = bt_sink_srv_state_manager_play_callback;
    device->dummy_device->stop = bt_sink_srv_state_manager_stop_callback;
    device->dummy_device->suspend = bt_sink_srv_state_manager_suspend_callback;
    device->dummy_device->reject = bt_sink_srv_state_manager_reject_callback;
    device->dummy_device->exception_handle = bt_sink_srv_state_manager_exception_callback;

    audio_src_srv_update_state(device->dummy_device, AUDIO_SRC_SRV_EVT_READY);
}

void bt_sink_srv_state_manager_free_psedev(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_manager_device_t *device)
{
    bt_sink_srv_report_id("[Sink][StaMgr][PseDev]free, next_state: 0x%x", 1, device->next_state);

    if (NULL != device->dummy_device &&
        BT_SINK_SRV_STATE_MANAGER_NEXT_STATE_INVALID == device->next_state) {
        audio_src_srv_del_waiting_list(device->dummy_device);
        audio_src_srv_destruct_handle(device->dummy_device);
        device->dummy_device = NULL;
    }
}

void bt_sink_srv_state_manager_play_psedev(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_manager_device_t *device)
{
    audio_src_srv_state_t pre_state = device->dummy_device->state;

    if (AUDIO_SRC_SRV_STATE_READY == device->dummy_device->state) {
        bt_sink_srv_state_manager_set_next_state(context, device, BT_SINK_SRV_STATE_MANAGER_NEXT_STATE_PLAYING);
        audio_src_srv_update_state(device->dummy_device, AUDIO_SRC_SRV_EVT_PREPARE_PLAY);
    }

    (void)pre_state;
    bt_sink_srv_report_id("[Sink][StaMgr][PseDev]play psedev, OUT: 0x%x(0x%x -> 0x%x)", 3,
                          device->dummy_device, pre_state, device->dummy_device->state);
}

void bt_sink_srv_state_manager_stop_psedev(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_manager_device_t *device)
{
    audio_src_srv_state_t pre_state = device->dummy_device->state;

    if (AUDIO_SRC_SRV_STATE_PLAYING == device->dummy_device->state) {
        bt_sink_srv_state_manager_set_next_state(context, device, BT_SINK_SRV_STATE_MANAGER_NEXT_STATE_READY);
        audio_src_srv_update_state(device->dummy_device, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
    } else if (AUDIO_SRC_SRV_STATE_READY == device->dummy_device->state) {
        bt_sink_srv_state_manager_set_next_state(context, device, BT_SINK_SRV_STATE_MANAGER_NEXT_STATE_INVALID);
        audio_src_srv_del_waiting_list(device->dummy_device);
    } else if (AUDIO_SRC_SRV_STATE_PREPARE_PLAY == device->dummy_device->state) {
        bt_sink_srv_state_manager_set_next_state(context, device, BT_SINK_SRV_STATE_MANAGER_NEXT_STATE_READY);
    }

    (void)pre_state;
    bt_sink_srv_report_id("[Sink][StaMgr][PseDev]stop psedev, OUT: 0x%x(0x%x -> 0x%x)", 3,
                          device->dummy_device, pre_state, device->dummy_device->state);
}

static void bt_sink_srv_state_manager_play_callback(audio_src_srv_handle_t *handle)
{
    bt_sink_srv_state_manager_context_t *context = bt_sink_srv_state_manager_get_context();
    bt_sink_srv_state_manager_device_t *device = bt_sink_srv_state_manager_get_device_by_psedev(context, handle);
    audio_src_srv_state_t pre_state = handle->state;

    audio_src_srv_update_state(handle, AUDIO_SRC_SRV_EVT_PLAYING);
    bt_sink_srv_state_manager_run_next_state(context, device);

    (void)pre_state;
    bt_sink_srv_report_id("[Sink][StaMgr][PseDev]play callback, OUT: 0x%x(0x%x -> 0x%x)", 3,
                          handle, pre_state, handle->state);
}

static void bt_sink_srv_state_manager_stop_callback(audio_src_srv_handle_t *handle)
{
    bt_sink_srv_state_manager_context_t *context = bt_sink_srv_state_manager_get_context();
    bt_sink_srv_state_manager_device_t *device = bt_sink_srv_state_manager_get_device_by_psedev(context, handle);
    audio_src_srv_state_t pre_state = handle->state;

    audio_src_srv_update_state(handle, AUDIO_SRC_SRV_EVT_READY);
    bt_sink_srv_state_manager_run_next_state(context, device);

    (void)pre_state;
    bt_sink_srv_report_id("[Sink][StaMgr][PseDev]stop callback, OUT: 0x%x(0x%x -> 0x%x)", 3,
                          handle, pre_state, handle->state);
}

static void bt_sink_srv_state_manager_suspend_callback(
    audio_src_srv_handle_t *handle,
    audio_src_srv_handle_t *interrupt)
{
    bt_sink_srv_state_manager_context_t *context = bt_sink_srv_state_manager_get_context();
    bt_sink_srv_state_manager_device_t *device = bt_sink_srv_state_manager_get_device_by_psedev(context, handle);
    audio_src_srv_state_t pre_state = handle->state;

    audio_src_srv_update_state(handle, AUDIO_SRC_SRV_EVT_READY);

    if (NULL != device && interrupt != device->call_device) {
        audio_src_srv_add_waiting_list(handle);
    }

    (void)pre_state;
    bt_sink_srv_report_id("[Sink][StaMgr][PseDev]suspend callback, OUT: 0x%x(0x%x -> 0x%x)", 3,
                          handle, pre_state, handle->state);
}

static void bt_sink_srv_state_manager_reject_callback(audio_src_srv_handle_t *handle)
{
    bt_sink_srv_state_manager_context_t *context = bt_sink_srv_state_manager_get_context();
    bt_sink_srv_state_manager_device_t *device = bt_sink_srv_state_manager_get_device_by_psedev(context, handle);
    audio_src_srv_state_t pre_state = handle->state;

    audio_src_srv_add_waiting_list(handle);
    bt_sink_srv_state_manager_set_next_state(context, device, BT_SINK_SRV_STATE_MANAGER_NEXT_STATE_INVALID);

    (void)pre_state;
    bt_sink_srv_report_id("[Sink][StaMgr][PseDev]reject callback, OUT: 0x%x(0x%x -> 0x%x)", 3,
                          handle, pre_state, handle->state);
}

static void bt_sink_srv_state_manager_exception_callback(
    audio_src_srv_handle_t *handle,
    int32_t event, void *parameter)
{
    return;
}

static void bt_sink_srv_state_manager_set_next_state(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_manager_device_t *device,
    bt_sink_srv_state_manager_next_state_t next_state)
{
    bt_sink_srv_report_id(
        "[Sink][StaMgr][PseDev]set next state, 0x%x(0x%x -> 0x%x)",
        3,
        device->dummy_device,
        device->next_state,
        next_state);

    device->next_state = next_state;
}

static void bt_sink_srv_state_manager_run_next_state(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_manager_device_t *device)
{
    bt_sink_srv_report_id(
        "[Sink][StaMgr][PseDev]run next state, 0x%x(0x%x)",
        2,
        device->dummy_device,
        device->next_state);

    if (NULL == device->dummy_device) {
        return ;
    }

    switch (device->dummy_device->state) {
        case AUDIO_SRC_SRV_STATE_READY: {
            if (BT_SINK_SRV_STATE_MANAGER_NEXT_STATE_READY == device->next_state) {
                bt_sink_srv_state_manager_set_next_state(
                    context,
                    device,
                    BT_SINK_SRV_STATE_MANAGER_NEXT_STATE_INVALID);
            }
            break;
        }

        case AUDIO_SRC_SRV_STATE_PREPARE_PLAY: {
            break;
        }

        case AUDIO_SRC_SRV_STATE_PLAYING: {
            if (BT_SINK_SRV_STATE_MANAGER_NEXT_STATE_PLAYING == device->next_state) {
                bt_sink_srv_state_manager_set_next_state(
                    context,
                    device,
                    BT_SINK_SRV_STATE_MANAGER_NEXT_STATE_INVALID);
            } else if (BT_SINK_SRV_STATE_MANAGER_NEXT_STATE_READY == device->next_state) {
                audio_src_srv_update_state(device->dummy_device, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
            }
            break;
        }

        case AUDIO_SRC_SRV_STATE_PREPARE_STOP: {
            break;
        }

        default: {
            break;
        }
    }
}
