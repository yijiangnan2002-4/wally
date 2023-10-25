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

#define BT_SINK_SRV_STATE_MANAGER_CALL_REMAP_TABLE_SIZE \
    (sizeof(g_bt_sink_srv_state_manager_call_remap_table) / \
     sizeof(bt_sink_srv_state_manager_remap_table_t))

#define BT_SINK_SRV_STATE_MANAGER_SHOULD_REMOVE_DEVICE(device) \
    (BT_SINK_SRV_STATE_MANAGER_IS_NONE_STATE((device).call_state) &&                \
     BT_SINK_SRV_STATE_MANAGER_IS_NONE_STATE((device).media_state) &&               \
     (device).call_audio_state == BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED)

#define BT_SINK_SRV_STATE_MANAGER_SHOULD_ADD_CALL_DEVICE(device_state) \
    (BT_SINK_SRV_STATE_MANAGER_IS_CALL_STATE((device_state).call_state) || \
     (device_state).sco_state != BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED)

#define BT_SINK_SRV_STATE_MANAGER_SHOULD_SET_CALL_FOCUS_DEVICE(device, new_state) \
    ((BT_SINK_SRV_STATE_MANAGER_IS_NONE_STATE((device).call_state) &&               \
      BT_SINK_SRV_STATE_MANAGER_IS_CALL_STATE((new_state).call_state)) ||           \
     ((device).call_audio_state == BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED && \
      (new_state).sco_state != BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED))

#define BT_SINK_SRV_STATE_MANAGER_SHOULD_ADD_MEDIA_DEVICE(device_state) \
    (BT_SINK_SRV_STATE_MANAGER_IS_MEDIA_STATE((device_state).music_state))

#define BT_SINK_SRV_STATE_MANAGER_SHOULD_SET_MEDIA_FOCUS_DEVICE(device, new_state) \
    ((BT_SINK_SRV_STATE_MANAGER_IS_NONE_STATE((device).media_state) && \
      BT_SINK_SRV_STATE_MANAGER_IS_MEDIA_STATE((new_state).music_state)))

#define BT_SINK_SRV_STATE_MANAGER_SHOULD_ADD_LE_DEVICE(device_state) \
    (BT_SINK_SRV_STATE_MANAGER_IS_CALL_STATE((device_state).call_state) ||      \
     BT_SINK_SRV_STATE_MANAGER_IS_MEDIA_STATE((device_state).music_state) ||    \
     (device_state).sco_state != BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED)

static const bt_sink_srv_state_manager_remap_table_t g_bt_sink_srv_state_manager_call_remap_table[] = {
    {
        BT_SINK_SRV_STATE_INCOMING,
        BT_SINK_SRV_STATE_OUTGOING | BT_SINK_SRV_STATE_ACTIVE | BT_SINK_SRV_STATE_HELD_REMAINING,
        BT_SINK_SRV_STATE_TWC_INCOMING
    },
    {
        BT_SINK_SRV_STATE_OUTGOING,
        BT_SINK_SRV_STATE_INCOMING,
        BT_SINK_SRV_STATE_TWC_INCOMING
    },
    {
        BT_SINK_SRV_STATE_OUTGOING,
        BT_SINK_SRV_STATE_ACTIVE | BT_SINK_SRV_STATE_HELD_REMAINING,
        BT_SINK_SRV_STATE_TWC_OUTGOING
    },
    {
        BT_SINK_SRV_STATE_ACTIVE | BT_SINK_SRV_STATE_HELD_REMAINING,
        BT_SINK_SRV_STATE_INCOMING,
        BT_SINK_SRV_STATE_TWC_INCOMING
    },
    {
        BT_SINK_SRV_STATE_ACTIVE | BT_SINK_SRV_STATE_HELD_REMAINING,
        BT_SINK_SRV_STATE_OUTGOING,
        BT_SINK_SRV_STATE_TWC_OUTGOING
    },
    {
        BT_SINK_SRV_STATE_ACTIVE,
        BT_SINK_SRV_STATE_HELD_REMAINING,
        BT_SINK_SRV_STATE_HELD_ACTIVE
    },
    {
        BT_SINK_SRV_STATE_HELD_REMAINING,
        BT_SINK_SRV_STATE_ACTIVE,
        BT_SINK_SRV_STATE_HELD_ACTIVE
    }
};

#if defined(MTK_AWS_MCE_ENABLE)
static void bt_sink_srv_state_manager_sync_state_change(
        bt_sink_srv_state_manager_context_t *context,
        bt_sink_srv_state_t state);
#endif
static bt_status_t default_bt_sink_srv_state_manager_call_callback(
        bt_sink_srv_state_manager_event_t event,
        bt_bd_addr_t *address,
        void *parameter);
static bt_status_t default_bt_sink_srv_state_manager_music_callback(
        bt_sink_srv_state_manager_event_t event,
        bt_bd_addr_t *address,
        void *parameter);
static bt_status_t default_bt_sink_srv_state_manager_le_callback(
        bt_sink_srv_state_manager_event_t event,
        bt_bd_addr_t *address,
        void *parameter);

void bt_sink_srv_state_manager_update_edr_call_devices(bt_sink_srv_state_manager_context_t *context);
void bt_sink_srv_state_manager_update_edr_media_devices(bt_sink_srv_state_manager_context_t *context);
void bt_sink_srv_state_manager_update_le_devices(bt_sink_srv_state_manager_context_t *context);
static bt_sink_srv_state_t bt_sink_srv_state_manager_remap_call_state(bt_sink_srv_state_manager_context_t *context);
static bt_sink_srv_state_t bt_sink_srv_state_manager_remap_media_state(bt_sink_srv_state_manager_context_t *context);

#if _MSC_VER >= 1500
#pragma comment(linker, "/alternatename:_bt_sink_srv_state_manager_call_callback=_default_bt_sink_srv_state_manager_call_callback")
#pragma comment(linker, "/alternatename:_bt_sink_srv_state_manager_music_callback=_default_bt_sink_srv_state_manager_music_callback")
#pragma comment(linker, "/alternatename:_bt_sink_srv_state_manager_le_callback=_default_bt_sink_srv_state_manager_le_callback")
#elif defined(__GNUC__) || defined(__ICCARM__) || defined(__CC_ARM)
#pragma weak bt_sink_srv_state_manager_call_callback = default_bt_sink_srv_state_manager_call_callback
#pragma weak bt_sink_srv_state_manager_music_callback = default_bt_sink_srv_state_manager_music_callback
#pragma weak bt_sink_srv_state_manager_le_callback = default_bt_sink_srv_state_manager_le_callback
#else
#error "Unsupported platform"
#endif

void bt_sink_srv_state_manager_notify_state_change(
        bt_sink_srv_state_manager_device_type_t type,
        bt_sink_srv_state_t state)
{
    bt_sink_srv_state_manager_device_t *device = NULL;
    bt_sink_srv_state_manager_context_t *context = bt_sink_srv_state_manager_get_context();

    /* 1. Update devices state. */
    if (type == BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_EDR) {
        bt_sink_srv_state_manager_update_edr_devices(context);
    } else {
        bt_sink_srv_state_manager_update_le_devices(context);
    }

    /* 2. Remap call state & media state. */
    context->call_state = bt_sink_srv_state_manager_remap_call_state(context);
    context->media_state = bt_sink_srv_state_manager_remap_media_state(context);

    bt_sink_srv_report_id(
            "[Sink][StaMgr]notify state change, call_state: 0x%x, media_state: 0x%x",
            2,
            context->call_state,
            context->media_state);

    /* 3. Notify & sync state change. */
    const audio_src_srv_handle_t *psedev = audio_src_srv_get_runing_pseudo_device();

    if (NULL != psedev) {
        device = bt_sink_srv_state_manager_get_device_by_psedev(context, (audio_src_srv_handle_t *)psedev);
    }

    if (NULL != device) {
        bt_sink_srv_state_manager_update_state(context, device, (audio_src_srv_handle_t *)psedev, false);
    } else {
        if (BT_SINK_SRV_STATE_MANAGER_IS_NONE_STATE(context->previous_state)) {
            if (BT_SINK_SRV_STATE_MANAGER_IS_CALL_STATE(state)) {
                bt_sink_srv_state_manager_notify_state_change_internal(context, context->call_state, false);
            } else if (BT_SINK_SRV_STATE_MANAGER_IS_MEDIA_STATE(state)) {
                bt_sink_srv_state_manager_notify_state_change_internal(context, context->media_state, false);
            } else {
                ;
            }
        } else if (BT_SINK_SRV_STATE_MANAGER_IS_CALL_STATE(context->previous_state)) {
            if (BT_SINK_SRV_STATE_MANAGER_IS_CALL_STATE(state) ||
                BT_SINK_SRV_STATE_MANAGER_IS_NONE_STATE(state)) {
                bt_sink_srv_state_manager_notify_state_change_internal(context, context->call_state, false);
            }
        } else {
            if (BT_SINK_SRV_STATE_MANAGER_IS_MEDIA_STATE(state) ||
                BT_SINK_SRV_STATE_MANAGER_IS_NONE_STATE(state)) {
                bt_sink_srv_state_manager_notify_state_change_internal(context, context->media_state, false);
            }
        }
    }
}

void bt_sink_srv_state_manager_notify_call_audio_state(
        bt_sink_srv_state_manager_device_type_t type,
        bt_bd_addr_t *address,
        bt_sink_srv_sco_connection_state_t call_audio_state)
{
    bt_sink_srv_state_manager_notify_state_change(type, BT_SINK_SRV_STATE_NONE);
}

void bt_sink_srv_state_manager_update_edr_devices(bt_sink_srv_state_manager_context_t *context)
{
    bt_sink_srv_state_manager_update_edr_call_devices(context);
    bt_sink_srv_state_manager_update_edr_media_devices(context);
}

void bt_sink_srv_state_manager_update_state(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_manager_device_t *device,
    audio_src_srv_handle_t *running_device,
    bool set_focus_device)
{
    bool is_le_call_ongoing = false;
    if (NULL == context || NULL == device || NULL == running_device) {
        return ;
    }

    /*  BT_SINK_SRV_STATE_INCOMING  for incoming call ringtone
        BT_SINK_SRV_STATE_OUTGOING/BT_SINK_SRV_STATE_ACTIVE  for dongle outgoing call
    */
    if (BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_LE == device->type &&
        (BT_SINK_SRV_STATE_INCOMING <= device->call_state &&
        BT_SINK_SRV_STATE_HELD_REMAINING >= device->call_state)) {
        is_le_call_ongoing = true;
    }
    bt_sink_srv_report_id("[Sink][StaMgr][Debug] dev_type:%d, dev_state:0x%x, is le call:%d", 3,
        device->type, device->call_state, is_le_call_ongoing);

    if (is_le_call_ongoing
        || running_device == device->call_device
#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_DUMMY_DEIVCE_ENABLE
        || running_device == device->dummy_device
#endif
    ) {
        if (set_focus_device) {
            bt_sink_srv_state_manager_set_focus_call_device(context, device, true);
        }
        bt_sink_srv_state_manager_notify_state_change_internal(context, context->call_state, false);
    } else {
        if (set_focus_device) {
            bt_sink_srv_state_manager_set_focus_media_device(context, device, true);
        }
        bt_sink_srv_state_manager_notify_state_change_internal(context, context->media_state, false);
    }
}

void bt_sink_srv_state_manager_update_edr_call_devices(bt_sink_srv_state_manager_context_t *context)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_sink_srv_state_t previous_call_state = BT_SINK_SRV_STATE_NONE;
    bt_sink_srv_device_state_t device_state = BT_SINK_SRV_STATE_MANAGER_INVALID_DEVICE_STATE;

    uint32_t connected_num = 0;
    bt_bd_addr_t connected_address[BT_SINK_SRV_STATE_MANAGER_MAX_DEVICE_NUM] = {{0}};
    bt_sink_srv_state_manager_device_t device = BT_SINK_SRV_STATE_MANAGER_INVALID_DEVICE;
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();

    /* 1. Update exist devices & remove unused devices. */
    for (uint32_t i = 0; i < BT_SINK_SRV_STATE_MANAGER_MAX_DEVICE_NUM; i++) {
        if (BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_EDR != context->devices[i].type) {
            continue;
        }

        /* 1.1. Get device state. */
        bt_sink_srv_memset(&device_state, 0, sizeof(bt_sink_srv_device_state_t));

        bt_sink_srv_state_manager_call_callback(
                BT_SINK_SRV_STATE_MANAGER_EVENT_GET_STATE,
                &context->devices[i].address,
                &device_state);

        bt_sink_srv_state_manager_call_callback(
                BT_SINK_SRV_STATE_MANAGER_EVENT_GET_CALL_DEVICE,
                &context->devices[i].address,
                &context->devices[i].call_device);

        /* 1.2. Update exists device. */
        if (BT_SINK_SRV_STATE_MANAGER_SHOULD_SET_CALL_FOCUS_DEVICE(context->devices[i], device_state)) {
            if (NULL == context->focus_call_device) {
                bt_sink_srv_state_manager_set_focus_call_device(context, &context->devices[i], true);
            }
        }

        previous_call_state = context->devices[i].call_state;
        context->devices[i].call_state = device_state.call_state;
        context->devices[i].call_audio_state = device_state.sco_state;

        bt_sink_srv_report_id("[Sink][StaMgr]update EDR call devices, device: 0x%x(call_state: 0x%x, call_audio_state: 0x%x)",
            3, &context->devices[i], context->devices[i].call_state, context->devices[i].call_audio_state);

        if (BT_SINK_SRV_STATE_INCOMING == previous_call_state &&
            BT_SINK_SRV_STATE_INCOMING != device_state.call_state) {
            context->devices[i].flag &= ~BT_SINK_SRV_STATE_MANAGER_DEVICE_FLAG_RING;

            if (0 != (BT_SINK_SRV_STATE_MANAGER_DEVICE_FLAG_TWC_RING & context->devices[i].flag)) {
                bt_sink_srv_state_manager_device_t *ring_device
                    = bt_sink_srv_state_manager_get_device_by_flag(
                        context,
                        BT_SINK_SRV_STATE_MANAGER_DEVICE_FLAG_RING);

                context->devices[i].flag &= ~BT_SINK_SRV_STATE_MANAGER_DEVICE_FLAG_TWC_RING;

                bt_sink_srv_state_manager_notify_ring_ind(
                    context->devices[i].type,
                    &context->devices[i].address,
                    false);

                if (NULL != ring_device) {
                    bt_sink_srv_state_manager_notify_ring_ind(
                        ring_device->type,
                        &ring_device->address,
                        true);
                }
            }
        }

        /* 1.3. Check device should be removed or not. */
        if (BT_SINK_SRV_STATE_MANAGER_SHOULD_REMOVE_DEVICE(context->devices[i])) {
            bt_sink_srv_state_manager_remove_device(context, &context->devices[i]);
        }
    }

    /* 2. Check if need to add new devices. */
    if (BT_AWS_MCE_ROLE_NONE == role || BT_AWS_MCE_ROLE_AGENT == role) {
        connected_num = bt_cm_get_connected_devices(
                BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP),
                connected_address,
                BT_SINK_SRV_STATE_MANAGER_MAX_DEVICE_NUM);
    } else {
    #ifdef MTK_AWS_MCE_ENABLE
        connected_num = bt_cm_get_connected_devices(
                BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS),
                connected_address,
                BT_SINK_SRV_STATE_MANAGER_MAX_DEVICE_NUM);
    #endif
    }

    for (uint32_t i = 0; i < connected_num; i++) {
        /* 2.1. Check device exists. */
        if (NULL != bt_sink_srv_state_manager_get_device(
                context,
                BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_EDR,
                &connected_address[i])) {
            continue;
        }

        /* 2.2. Get device state. */
        status = bt_sink_srv_state_manager_call_callback(
                BT_SINK_SRV_STATE_MANAGER_EVENT_GET_STATE,
                &connected_address[i],
                &device_state);

        /* 2.3. Add new device. */
        if (status == BT_STATUS_SUCCESS &&
            BT_SINK_SRV_STATE_MANAGER_SHOULD_ADD_CALL_DEVICE(device_state)) {
            device.type = BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_EDR;
            device.call_state = device_state.call_state;
            device.call_audio_state = device_state.sco_state;
            bt_sink_srv_memcpy(&device.address, &connected_address[i], sizeof(bt_bd_addr_t));

            bt_sink_srv_state_manager_call_callback(
                    BT_SINK_SRV_STATE_MANAGER_EVENT_GET_CALL_DEVICE,
                    &connected_address[i],
                    &device.call_device);

            bt_sink_srv_state_manager_add_device(context, &device);
        }
    }
}

void bt_sink_srv_state_manager_update_edr_media_devices(bt_sink_srv_state_manager_context_t *context)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_sink_srv_device_state_t device_state = BT_SINK_SRV_STATE_MANAGER_INVALID_DEVICE_STATE;

    uint32_t connected_num = 0;
    bt_bd_addr_t connected_address[BT_SINK_SRV_STATE_MANAGER_MAX_DEVICE_NUM] = {{0}};
    bt_sink_srv_state_manager_device_t device = BT_SINK_SRV_STATE_MANAGER_INVALID_DEVICE;
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();

    /* 1. Update exist devices & remove unused devices. */
    for (uint32_t i = 0; i < BT_SINK_SRV_STATE_MANAGER_MAX_DEVICE_NUM; i++) {
        if (BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_EDR != context->devices[i].type) {
            continue;
        }

        /* 1.1. Get device state. */
        bt_sink_srv_memset(&device_state, 0, sizeof(bt_sink_srv_device_state_t));

        bt_sink_srv_state_manager_music_callback(
                BT_SINK_SRV_STATE_MANAGER_EVENT_GET_STATE,
                &context->devices[i].address,
                &device_state);

        bt_sink_srv_state_manager_music_callback(
                BT_SINK_SRV_STATE_MANAGER_EVENT_GET_MEDIA_DEVICE,
                &context->devices[i].address,
                &context->devices[i].media_device);

        /* 1.2. Update existed device. */
        if (BT_SINK_SRV_STATE_MANAGER_SHOULD_SET_MEDIA_FOCUS_DEVICE(context->devices[i], device_state)) {
            if (NULL == context->focus_media_device) {
                bt_sink_srv_state_manager_set_focus_media_device(context, &context->devices[i], true);
            }
        }

        context->devices[i].media_state = device_state.music_state;

        bt_sink_srv_report_id(
                "[Sink][StaMgr]update EDR media devices, device: 0x%x(media_state: 0x%x)",
                2,
                &context->devices[i],
                context->devices[i].media_state);

        /* 1.3. Check device should be removed or not. */
        if (BT_SINK_SRV_STATE_MANAGER_SHOULD_REMOVE_DEVICE(context->devices[i])) {
            bt_sink_srv_state_manager_remove_device(context, &context->devices[i]);
        }
    }

    /* 2. Check if need to add new devices. */
    if (BT_AWS_MCE_ROLE_NONE == role || BT_AWS_MCE_ROLE_AGENT == role) {
        connected_num = bt_cm_get_connected_devices(
                BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK),
                connected_address,
                BT_SINK_SRV_STATE_MANAGER_MAX_DEVICE_NUM);
    } else {
    #ifdef MTK_AWS_MCE_ENABLE
        connected_num = bt_cm_get_connected_devices(
                BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS),
                connected_address,
                BT_SINK_SRV_STATE_MANAGER_MAX_DEVICE_NUM);
    #endif
    }
    

    for (uint32_t i = 0; i < connected_num; i++) {
        /* 2.1. Check device exists. */
        if (NULL != bt_sink_srv_state_manager_get_device(
                context,
                BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_EDR,
                &connected_address[i])) {
            continue;
        }

        /* 2.2. Get device state. */
        status = bt_sink_srv_state_manager_music_callback(
                BT_SINK_SRV_STATE_MANAGER_EVENT_GET_STATE,
                &connected_address[i],
                &device_state);

        /* 2.3. Add new device. */
        if (BT_STATUS_SUCCESS == status &&
            BT_SINK_SRV_STATE_MANAGER_SHOULD_ADD_MEDIA_DEVICE(device_state)) {
            device.type = BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_EDR;
            device.media_state = device_state.music_state;
            bt_sink_srv_memcpy(device.address, connected_address[i], sizeof(bt_bd_addr_t));

            bt_sink_srv_state_manager_music_callback(
                    BT_SINK_SRV_STATE_MANAGER_EVENT_GET_MEDIA_DEVICE,
                    &connected_address[i],
                    &device.media_device);

            bt_sink_srv_state_manager_add_device(context, &device);
        }
    }
}

void bt_sink_srv_state_manager_update_le_devices(bt_sink_srv_state_manager_context_t *context)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_sink_srv_device_state_t device_state = BT_SINK_SRV_STATE_MANAGER_INVALID_DEVICE_STATE;

    uint32_t connected_num = 0;
    bt_addr_t connected_address[BT_SINK_SRV_STATE_MANAGER_MAX_DEVICE_NUM] = {{0}};
    bt_sink_srv_state_manager_device_t device = BT_SINK_SRV_STATE_MANAGER_INVALID_DEVICE;

    /* 1. Update exist devices & remove unused devices. */
    for (uint32_t i = 0; i < BT_SINK_SRV_STATE_MANAGER_MAX_DEVICE_NUM; i++) {
        if (BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_LE != context->devices[i].type) {
            continue;
        }

        /* 1.1. Get device state. */
        bt_sink_srv_memset(&device_state, 0, sizeof(bt_sink_srv_device_state_t));

        bt_sink_srv_state_manager_le_callback(
                BT_SINK_SRV_STATE_MANAGER_EVENT_GET_STATE,
                &context->devices[i].address,
                &device_state);

        bt_sink_srv_state_manager_le_callback(
                BT_SINK_SRV_STATE_MANAGER_EVENT_GET_CALL_DEVICE,
                &context->devices[i].address,
                &context->devices[i].call_device);

        bt_sink_srv_state_manager_le_callback(
                BT_SINK_SRV_STATE_MANAGER_EVENT_GET_MEDIA_DEVICE,
                &context->devices[i].address,
                &context->devices[i].media_device);

        /* 1.2. Update exists device. */
        if (BT_SINK_SRV_STATE_MANAGER_SHOULD_SET_CALL_FOCUS_DEVICE(context->devices[i], device_state)) {
            if (NULL == context->focus_call_device) {
                bt_sink_srv_state_manager_set_focus_call_device(context, &context->devices[i], true);
            }
        }

        if (BT_SINK_SRV_STATE_MANAGER_SHOULD_SET_MEDIA_FOCUS_DEVICE(context->devices[i], device_state)) {
            if (NULL == context->focus_media_device) {
                bt_sink_srv_state_manager_set_focus_media_device(context, &context->devices[i], true);
            }
        }

        bt_sink_srv_state_t pre = context->devices[i].call_state;
        context->devices[i].call_state = device_state.call_state;
        context->devices[i].call_audio_state = device_state.sco_state;
        context->devices[i].media_state = device_state.music_state;

#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_DUMMY_DEIVCE_ENABLE
        if (BT_SINK_SRV_STATE_INCOMING != pre &&
            BT_SINK_SRV_STATE_INCOMING == device_state.call_state) {
            bt_sink_srv_state_manager_play_psedev(context, &context->devices[i]);
        } else if (BT_SINK_SRV_STATE_INCOMING == pre &&
            BT_SINK_SRV_STATE_INCOMING != device_state.call_state) {
            context->devices[i].flag &= ~BT_SINK_SRV_STATE_MANAGER_DEVICE_FLAG_RING;

            if (context->focus_call_device != &context->devices[i]) {
                bt_sink_srv_state_manager_stop_psedev(context, &context->devices[i]);
            } else if (context->focus_call_device == &context->devices[i] &&
                BT_SINK_SRV_STATE_MANAGER_IS_NONE_STATE(device_state.call_state)) {
                bt_sink_srv_state_manager_stop_psedev(context, &context->devices[i]);
            } else {
                ;
            }

            if (0 != (BT_SINK_SRV_STATE_MANAGER_DEVICE_FLAG_TWC_RING & context->devices[i].flag)) {
                bt_sink_srv_state_manager_device_t *ring_device
                    = bt_sink_srv_state_manager_get_device_by_flag(
                        context,
                        BT_SINK_SRV_STATE_MANAGER_DEVICE_FLAG_RING);

                context->devices[i].flag &= ~BT_SINK_SRV_STATE_MANAGER_DEVICE_FLAG_TWC_RING;

                bt_sink_srv_state_manager_notify_ring_ind(
                    context->devices[i].type,
                    &context->devices[i].address,
                    false);

                if (NULL != ring_device) {
                    bt_sink_srv_state_manager_notify_ring_ind(
                        ring_device->type,
                        &ring_device->address,
                        true);
                }
            }
        } else if (BT_SINK_SRV_STATE_MANAGER_IS_CALL_STATE(pre) &&
            BT_SINK_SRV_STATE_MANAGER_IS_NONE_STATE(device_state.call_state)) {
        #ifdef MTK_AWS_MCE_ENABLE
            bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
            if (BT_AWS_MCE_ROLE_NONE == role || BT_AWS_MCE_ROLE_AGENT == role) {
        #endif
                bt_sink_srv_state_manager_device_t *other_device
                    = bt_sink_srv_state_manager_get_device_by_call_state(
                        context,
                        BT_SINK_SRV_STATE_HELD_REMAINING,
                        &context->devices[i]);

                if (&context->devices[i] == context->focus_call_device &&
                    NULL != other_device && BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_EDR == other_device->type) {
                    bt_sink_srv_state_manager_edr_action_handler(
                        context,
                        BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER,
                        NULL);
                }
        #ifdef MTK_AWS_MCE_ENABLE
            }
        #endif
        } else {
            ;
        }
#endif

        bt_sink_srv_report_id(
                "[Sink][StaMgr]update LE devices, device: 0x%x"
                "(call_state: 0x%x, call_audio_state: 0x%x, media_state: 0x%x)",
                4,
                &context->devices[i],
                context->devices[i].call_state,
                context->devices[i].call_audio_state,
                context->devices[i].media_state);

        /* 1.3. Check device should be remove or not. */
        if (BT_SINK_SRV_STATE_MANAGER_SHOULD_REMOVE_DEVICE(context->devices[i])) {
#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_DUMMY_DEIVCE_ENABLE
            bt_sink_srv_state_manager_free_psedev(context, &context->devices[i]);
            if (NULL == context->devices[i].dummy_device) {
#endif
                bt_sink_srv_state_manager_remove_device(context, &context->devices[i]);
#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_DUMMY_DEIVCE_ENABLE
            }
#endif
        }
    }

    /* 2. Check if need to add new devices. */
    connected_num = bt_gap_le_srv_get_address_by_link_type(
            connected_address,
            BT_SINK_SRV_STATE_MANAGER_MAX_DEVICE_NUM,
            false);

    for (uint32_t i = 0; i < connected_num; i++) {
        /* 2.1. Check device exists. */
        if (NULL != bt_sink_srv_state_manager_get_device(
                context,
                BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_LE,
                &connected_address[i].addr)) {
            continue;
        }

        /* 2.2. Get device state. */
        bt_sink_srv_memset(&device_state, 0, sizeof(bt_sink_srv_device_state_t));

        status = bt_sink_srv_state_manager_le_callback(
                BT_SINK_SRV_STATE_MANAGER_EVENT_GET_STATE,
                &connected_address[i].addr,
                &device_state);

        /* 2.3. Add new device. */
        if (BT_STATUS_SUCCESS == status &&
            BT_SINK_SRV_STATE_MANAGER_SHOULD_ADD_LE_DEVICE(device_state)) {
            device.type = BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_LE;
            device.call_state = device_state.call_state;
            device.call_audio_state = device_state.sco_state;
            device.media_state = device_state.music_state;
            bt_sink_srv_memcpy(&device.address, &connected_address[i].addr, sizeof(bt_bd_addr_t));

            bt_sink_srv_state_manager_le_callback(
                    BT_SINK_SRV_STATE_MANAGER_EVENT_GET_CALL_DEVICE,
                    &connected_address[i].addr,
                    &device.call_device);

            bt_sink_srv_state_manager_le_callback(
                    BT_SINK_SRV_STATE_MANAGER_EVENT_GET_MEDIA_DEVICE,
                    &connected_address[i].addr,
                    &device.media_device);

            bt_sink_srv_state_manager_device_t *new_device
                    = bt_sink_srv_state_manager_add_device(context, &device);

#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_DUMMY_DEIVCE_ENABLE
            bt_sink_srv_state_manager_alloc_psedev(context, new_device);
            if (BT_SINK_SRV_STATE_INCOMING == new_device->call_state) {
                bt_sink_srv_state_manager_play_psedev(context, new_device);
            }
#endif
        }
    }
}

static bt_sink_srv_state_t bt_sink_srv_state_manager_remap_call_state(bt_sink_srv_state_manager_context_t *context)
{
    bt_sink_srv_state_manager_device_t *other_device = NULL;
    bt_sink_srv_state_t focus_state = BT_SINK_SRV_STATE_NONE;
    bt_sink_srv_state_t other_state = BT_SINK_SRV_STATE_NONE;

    if (context->focus_call_device == NULL) {
        return BT_SINK_SRV_STATE_NONE;
    }

    /* 1. Get focus state & other state. */
    focus_state = context->focus_call_device->call_state;

#ifdef AIR_BT_SINK_SRV_CUSTOMIZED_ENABLE
    other_device = bt_sink_srv_state_manager_get_device_by_call_state_ext(
            context,
            BT_SINK_SRV_STATE_MANAGER_CALL_STATE,
            &context->focus_call_device->address);
#else
    other_device = bt_sink_srv_state_manager_get_device_by_call_state(
            context,
            BT_SINK_SRV_STATE_MANAGER_CALL_STATE,
            context->focus_call_device);
#endif

    if (other_device != NULL) {
        other_state = other_device->call_state;
    }

    /* 2. Remap call state. */
    for (uint32_t i = 0; i < BT_SINK_SRV_STATE_MANAGER_CALL_REMAP_TABLE_SIZE; i++) {
        if ((g_bt_sink_srv_state_manager_call_remap_table[i].focus_state & focus_state) &&
            (g_bt_sink_srv_state_manager_call_remap_table[i].other_state & other_state)) {
            return g_bt_sink_srv_state_manager_call_remap_table[i].result;
        }
    }

    /* 3. Not in state table, process focus device firstly. */
    return focus_state;
}

static bt_sink_srv_state_t bt_sink_srv_state_manager_remap_media_state(bt_sink_srv_state_manager_context_t *context)
{
    bt_sink_srv_state_manager_device_t *other_device = NULL;
    bt_sink_srv_state_t focus_state = BT_SINK_SRV_STATE_NONE;
    bt_sink_srv_state_t other_state = BT_SINK_SRV_STATE_NONE;

    if (context->focus_media_device == NULL) {
        return BT_SINK_SRV_STATE_NONE;
    }

    /* 1. Get focus state & other device. */
    focus_state = context->focus_media_device->media_state;

    other_device = bt_sink_srv_state_manager_get_device_by_media_state(
            context,
            BT_SINK_SRV_STATE_MANAGER_MEDIA_STATE,
            context->focus_media_device);

    if (other_device != NULL) {
        other_state = other_device->media_state;
    }

    /* 2. Remap media state. */
    if ((focus_state & BT_SINK_SRV_STATE_MANAGER_MEDIA_STATE) ||
        (other_state & BT_SINK_SRV_STATE_MANAGER_MEDIA_STATE)) {
        return BT_SINK_SRV_STATE_STREAMING;
    }

    /* 3. All device not in STREAMING. */
    return BT_SINK_SRV_STATE_NONE;
}

void bt_sink_srv_state_manager_notify_state_change_internal(
        bt_sink_srv_state_manager_context_t *context,
        bt_sink_srv_state_t state,
        bool is_sync)
{
    bt_sink_srv_state_change_t state_change = {0};
    bt_sink_srv_state_manager_device_t *device = NULL;

#if defined(MTK_AWS_MCE_ENABLE) && defined(AIR_MULTI_POINT_ENABLE)
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bt_aws_mce_srv_link_type_t link = bt_aws_mce_srv_get_link_type();
#endif

    if (state == context->previous_state) {
        return;
    }

#if defined(MTK_AWS_MCE_ENABLE) && defined(AIR_MULTI_POINT_ENABLE)
    if (BT_AWS_MCE_ROLE_PARTNER == role && BT_AWS_MCE_SRV_LINK_NORMAL == link) {
        if (!is_sync && BT_SINK_SRV_STATE_MANAGER_IS_CALL_STATE(state)) {
            bt_sink_srv_report_id("[Sink][StaMgr]notify state change, return. is_sync: 0x%x state: 0x%x",
                                  2, is_sync, state);
            return; /* Partner should wait for Agent sync state, except media state. */
        }
    }
#endif

    /* 1. Notify Sink state to App. */
    state_change.previous= context->previous_state;
    state_change.current = state;

    bt_sink_srv_event_callback(
            BT_SINK_SRV_EVENT_STATE_CHANGE,
            &state_change,
            sizeof(bt_sink_srv_state_change_t));

    bt_sink_srv_report_id(
            "[Sink][StaMgr]notify state change, 0x%x -> 0x%x",
            2,
            context->previous_state,
            state);

    context->previous_state = state;

    /* 2. Sync Sink state to Partner. */
#if defined(MTK_AWS_MCE_ENABLE) && defined(AIR_MULTI_POINT_ENABLE)
    if (BT_AWS_MCE_ROLE_AGENT == role &&
        BT_AWS_MCE_SRV_LINK_NORMAL == link &&
        BT_SINK_SRV_STATE_MANAGER_IS_CALL_STATE(state)) {
        bt_sink_srv_state_manager_sync_state_change(context, state);
    }
#endif

    /* 3. Notify RING indication to App. */
    if (BT_SINK_SRV_STATE_MANAGER_IS_CALL_STATE(state)) {
        device = bt_sink_srv_state_manager_get_device_by_flag(context, BT_SINK_SRV_STATE_MANAGER_DEVICE_FLAG_SEND_RING);

        if (NULL != device) {
            device->flag &= ~BT_SINK_SRV_STATE_MANAGER_DEVICE_FLAG_SEND_RING;
            bt_sink_srv_state_manager_notify_ring_ind(device->type, &device->address, true);
        }
    }
}

#if defined(MTK_AWS_MCE_ENABLE)
static void bt_sink_srv_state_manager_sync_state_change(
        bt_sink_srv_state_manager_context_t *context,
        bt_sink_srv_state_t state)
{
    bt_status_t status = BT_STATUS_FAIL;
    uint8_t buffer[sizeof(bt_sink_srv_state_manager_sync_state_t)] = {0};
    bt_sink_srv_state_manager_sync_state_t *sync_state = (bt_sink_srv_state_manager_sync_state_t *)&buffer;

    bt_aws_mce_report_info_t report_info = {0};
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bt_aws_mce_srv_link_type_t link = bt_aws_mce_srv_get_link_type();

    /* 1. Check parameter. */
    if (BT_AWS_MCE_ROLE_AGENT != role || BT_AWS_MCE_SRV_LINK_NORMAL != link) {
        return;
    }

    /* 2. Fill data. */
    sync_state->header.type = BT_SINK_SRV_STATE_MANAGER_SYNC_TYPE_STATE;
    sync_state->header.length = sizeof(buffer);
    sync_state->header.direction = BT_SINK_SRV_STATE_MANAGER_SYNC_DIRECTION_PARTNER;
    sync_state->state = state;

    /* 3. Fill report info. */
    report_info.module_id = BT_AWS_MCE_REPORT_MODULE_SINK_STAMGR;
    report_info.param = (void *)&buffer;
    report_info.param_len = sizeof(buffer);

    status = bt_aws_mce_report_send_urgent_event(&report_info);
    (void)status;
    bt_sink_srv_report_id("[Sink][StaMgr]sync state, status: 0x%x", 1, status);
}
#endif

#ifdef MTK_AWS_MCE_ENABLE
bt_status_t bt_sink_srv_aws_mce_state_manager_callback(bt_msg_type_t msg, bt_status_t status, void *buffer)
{
    bt_sink_srv_state_manager_context_t *context = bt_sink_srv_state_manager_get_context();

    switch (msg) {
        case BT_AWS_MCE_STATE_CHANGED_IND: {
            bt_aws_mce_state_change_ind_t *state_change = (bt_aws_mce_state_change_ind_t *)buffer;
            if (BT_AWS_MCE_ROLE_AGENT == bt_connection_manager_device_local_info_get_aws_role()) {
                if ((NULL != state_change) &&
                    (0 != (state_change->state & BT_AWS_MCE_AGENT_STATE_ATTACHED))) {
                    if (!BT_SINK_SRV_STATE_MANAGER_IS_MEDIA_STATE(context->previous_state)) {
                        bt_sink_srv_state_manager_sync_state_change(context, context->previous_state);
                    }

                    bt_sink_srv_state_manager_sync_played_devices(context);
                }
            }
            break;
        }

        default: {
            break;
        }
    }

    return BT_STATUS_SUCCESS;
}
#endif

#ifdef MTK_AWS_MCE_ENABLE
void bt_sink_srv_state_manager_aws_mce_report_callback(bt_aws_mce_report_info_t *info)
{
    bt_sink_srv_state_manager_sync_header_t *header = NULL;
    bt_sink_srv_state_manager_context_t *context = bt_sink_srv_state_manager_get_context();

    if (NULL == info || BT_AWS_MCE_REPORT_MODULE_SINK_STAMGR != info->module_id) {
        return;
    }

    /* 1. Get header. */
    header = (bt_sink_srv_state_manager_sync_header_t *)info->param;

    bt_sink_srv_report_id("[Sink][StaMgr]aws mce report callback, type: 0x%x direction: 0x%x",
                          2, header->type, header->direction);

    /* 2. Dispatch data. */
    switch (header->type) {
        case BT_SINK_SRV_STATE_MANAGER_SYNC_TYPE_STATE: {
            bt_sink_srv_state_manager_sync_state_t *sync_state
                    = (bt_sink_srv_state_manager_sync_state_t *)info->param;
            bt_sink_srv_state_manager_notify_state_change_internal(context, sync_state->state, true);
            break;
        }

        case BT_SINK_SRV_STATE_MANAGER_SYNC_TYPE_FOCUS: {
            bt_sink_srv_state_manager_sync_focus_t *sync_focus
                    = (bt_sink_srv_state_manager_sync_focus_t *)info->param;

            bt_sink_srv_state_manager_device_t *device = bt_sink_srv_state_manager_get_device(
                    context,
                    sync_focus->device_type,
                    &sync_focus->address);

            if (BT_SINK_SRV_STATE_MANAGER_FOCUS_DEVICE_CALL == sync_focus->focus_type) {
                bt_sink_srv_state_manager_set_focus_call_device(context, device, true);
            } else {
                bt_sink_srv_state_manager_set_focus_media_device(context, device, true);
            }
            break;
        }

        case BT_SINK_SRV_STATE_MANAGER_SYNC_TYPE_PLAYED: {
            bt_sink_srv_state_manager_sync_played_t *sync_played
                = (bt_sink_srv_state_manager_sync_played_t *)info->param;

            bt_sink_srv_memcpy(
                context->played_devices,
                sync_played->played_devices,
                BT_SINK_SRV_STATE_MANAGER_MAX_PLAYED_DEVICE_NUM * sizeof(bt_sink_srv_state_manager_played_device_t));

            break;
        }

        case BT_SINK_SRV_STATE_MANAGER_SYNC_TYPE_ACTION: {
            bt_sink_srv_state_manager_sync_action_t *sync_action
                = (bt_sink_srv_state_manager_sync_action_t *)info->param;

            bt_sink_srv_report_id("[Sink][StaMgr]aws report callback, parameter_length: %d", 1,
                                  sync_action->parameter_length);

            if (0 != sync_action->parameter_length) {
                bt_sink_srv_state_manager_action_handler(sync_action->action, sync_action->parameter);
            } else {
                bt_sink_srv_state_manager_action_handler(sync_action->action, NULL);
            }

            break;
        }

        default: {
            break;
        }
    }
}
#endif

static bt_status_t default_bt_sink_srv_state_manager_call_callback(
        bt_sink_srv_state_manager_event_t event,
        bt_bd_addr_t *address,
        void *parameter)
{
    return BT_STATUS_FAIL;
}

static bt_status_t default_bt_sink_srv_state_manager_music_callback(
        bt_sink_srv_state_manager_event_t event,
        bt_bd_addr_t *address,
        void *parameter)
{
    return BT_STATUS_FAIL;
}

static bt_status_t default_bt_sink_srv_state_manager_le_callback(
        bt_sink_srv_state_manager_event_t event,
        bt_bd_addr_t *address,
        void *parameter)
{
    return BT_STATUS_FAIL;
}

