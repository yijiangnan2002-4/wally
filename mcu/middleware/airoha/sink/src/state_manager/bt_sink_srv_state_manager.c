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

static bt_sink_srv_state_manager_context_t g_bt_sink_srv_state_manager_context;

static void bt_sink_srv_state_manager_add_played_device(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_manager_device_t *device);

bool bt_sink_srv_state_manager_remove_played_device(
    bt_sink_srv_state_manager_device_type_t type,
    bt_bd_addr_t *address);

static void bt_sink_srv_state_manager_set_played_device_mask(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_manager_played_device_t *played_device,
    bt_sink_srv_state_manager_device_mask_t mask);

static void bt_sink_srv_state_manager_sync_play_count(
    bt_sink_srv_state_manager_context_t *context);

void bt_sink_srv_state_manager_initialize(void)
{
    bt_sink_srv_state_manager_context_t *context = bt_sink_srv_state_manager_get_context();

    context->play_count = BT_SINK_SRV_STATE_MANAGER_PLAY_COUNT_MIN;

#ifdef MTK_AWS_MCE_ENABLE
    bt_aws_mce_report_register_callback(
        BT_AWS_MCE_REPORT_MODULE_SINK_STAMGR,
        bt_sink_srv_state_manager_aws_mce_report_callback);
#endif
#ifdef SUPPORT_ROLE_HANDOVER_SERVICE
    bt_role_handover_callbacks_t callbacks = {
        .allowed_cb = NULL,
        .get_len_cb = bt_sink_srv_state_manager_rho_get_len,
        .get_data_cb = bt_sink_srv_state_manager_rho_get_data,
        .update_cb = bt_sink_srv_state_manager_rho_update,
        .status_cb = bt_sink_srv_state_manager_rho_status
    };
    bt_role_handover_register_callbacks(BT_ROLE_HANDOVER_MODULE_SINK_STAMGR, &callbacks);
#endif
#ifdef MTK_BT_CM_SUPPORT
    bt_cm_register_event_callback(bt_sink_srv_state_manager_cm_callback);
#endif
#ifdef AIR_LE_AUDIO_ENABLE
    bt_gap_le_srv_register_event_callback(bt_sink_srv_state_manager_gap_le_srv_callback);
#endif
}

bt_sink_srv_state_manager_play_count_t bt_sink_srv_state_manager_get_play_count(void)
{
    bt_sink_srv_state_manager_context_t *context = bt_sink_srv_state_manager_get_context();
    bt_sink_srv_report_id("[Sink][StaMgr]play count: %d", 1, context->play_count);
    bt_sink_srv_state_manager_sync_play_count(context);

    return context->play_count++;
}

bt_sink_srv_state_manager_context_t *bt_sink_srv_state_manager_get_context(void)
{
    return &g_bt_sink_srv_state_manager_context;
}

bt_sink_srv_state_manager_device_t *bt_sink_srv_state_manager_add_device(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_manager_device_t *device)
{
    bt_sink_srv_state_manager_device_t *free_device = NULL;

    /* 1. Get a free device. */
    for (uint32_t i = 0; i < BT_SINK_SRV_STATE_MANAGER_MAX_DEVICE_NUM; i++) {
        if (context->devices[i].type == BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_INVALID) {
            free_device = &context->devices[i];
            break;
        }
    }

    /* 2. Add device. */
    bt_utils_assert(free_device && "Cannot alloc a free device!");
    bt_sink_srv_memcpy(free_device, device, sizeof(bt_sink_srv_state_manager_device_t));

    bt_sink_srv_report_id("[Sink][StaMgr]add device, device 0x%x(type 0x%x, call_device 0x%x, media_device 0x%x)", 4,
                          free_device, device->type, device->call_device, device->media_device);
    bt_sink_srv_report_id("[Sink][StaMgr]add device, device 0x%x(call_state 0x%x, call_audio_state 0x%x, media_state 0x%x)", 4,
                          free_device, device->call_state, device->call_audio_state, device->media_state);

    /* 3. Set focus device. */
    if (BT_SINK_SRV_STATE_MANAGER_IS_CALL_STATE(device->call_state) ||
        BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED != device->call_audio_state ||
        BT_SINK_SRV_STATE_MANAGER_IS_MEDIA_STATE(device->media_state)) {
        if (context->focus_device == NULL) {
            bt_sink_srv_state_manager_set_focus_device(context, free_device, true);
        }
    }

    /* 4. Update played device. */
    bt_sink_srv_state_manager_add_played_device(context, free_device);

    return free_device;
}

void bt_sink_srv_state_manager_remove_device(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_manager_device_t *device)
{
    /* 1. Reset focus device. */
    if (context->focus_device == device) {
        bt_sink_srv_state_manager_set_focus_device(context, device, false);
    }

    /* 2. Reset device. */
    bt_sink_srv_memset(device, 0, sizeof(bt_sink_srv_state_manager_device_t));
    bt_sink_srv_report_id("[Sink][StaMgr]remove device, 0x%x", 1, device);
}

bt_sink_srv_state_manager_device_t *bt_sink_srv_state_manager_get_device(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_manager_device_type_t type,
    bt_bd_addr_t *address)
{
    for (uint32_t i = 0; i < BT_SINK_SRV_STATE_MANAGER_MAX_DEVICE_NUM; i++) {
        if (context->devices[i].type == type &&
            !bt_sink_srv_memcmp(&context->devices[i].address, address, sizeof(bt_bd_addr_t))) {
            return &context->devices[i];
        }
    }

    return NULL;
}

bt_sink_srv_state_manager_device_t *bt_sink_srv_state_manager_get_device_by_call_state(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_t call_state,
    bt_sink_srv_state_manager_device_t *skip_device)
{
    for (uint32_t i = 0; i < BT_SINK_SRV_STATE_MANAGER_MAX_DEVICE_NUM; i++) {
        if ((&context->devices[i] != skip_device) &&
            (context->devices[i].call_state & call_state)) {
            return &context->devices[i];
        }
    }

    return NULL;
}

#ifdef AIR_BT_SINK_SRV_CUSTOMIZED_ENABLE
bt_sink_srv_state_manager_device_t *bt_sink_srv_state_manager_get_device_by_call_state_ext(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_t call_state,
    bt_bd_addr_t *skip_address)
{
    for (uint32_t i = 0; i < BT_SINK_SRV_STATE_MANAGER_MAX_DEVICE_NUM; i++) {
        if (0 != (context->devices[i].call_state & call_state) &&
            0 != bt_sink_srv_memcmp(&context->devices[i].address, skip_address, sizeof(bt_bd_addr_t))) {
            return &context->devices[i];
        }
    }

    return NULL;
}
#endif

bt_sink_srv_state_manager_device_t *bt_sink_srv_state_manager_get_device_by_call_audio_state(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_sco_connection_state_t call_audio_state,
    bt_sink_srv_state_manager_device_t *skip_device)
{
    for (uint32_t i = 0; i < BT_SINK_SRV_STATE_MANAGER_MAX_DEVICE_NUM; i++) {
        if (&context->devices[i] != skip_device &&
            context->devices[i].call_audio_state == call_audio_state) {
            return &context->devices[i];
        }
    }

    return NULL;
}

bt_sink_srv_state_manager_device_t *bt_sink_srv_state_manager_get_device_by_media_state(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_t media_state,
    bt_sink_srv_state_manager_device_t *skip_device)
{
    for (uint32_t i = 0; i < BT_SINK_SRV_STATE_MANAGER_MAX_DEVICE_NUM; i++) {
        if ((&context->devices[i] != skip_device) &&
            (context->devices[i].media_state & media_state)) {
            return &context->devices[i];
        }
    }

    return NULL;
}

bt_sink_srv_state_manager_device_t *bt_sink_srv_state_manager_get_device_by_psedev(
    bt_sink_srv_state_manager_context_t *context,
    audio_src_srv_handle_t *psedev)
{
    for (uint32_t i = 0; i < BT_SINK_SRV_STATE_MANAGER_MAX_DEVICE_NUM; i++) {
        if (psedev == context->devices[i].call_device ||
            psedev == context->devices[i].media_device
#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_DUMMY_DEIVCE_ENABLE
            || psedev == context->devices[i].dummy_device
#endif
           ) {
            return &context->devices[i];
        }
    }

    return NULL;
}

bt_sink_srv_state_manager_device_t *bt_sink_srv_state_manager_get_device_by_flag(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_manager_device_flag_t flag)
{
    for (uint32_t i = 0; i < BT_SINK_SRV_STATE_MANAGER_MAX_DEVICE_NUM; i++) {
        if (flag == (context->devices[i].flag & flag)) {
            return &context->devices[i];
        }
    }

    return NULL;
}

void bt_sink_srv_state_manager_set_focus_device(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_manager_device_t *device,
    bool is_add)
{
    if (is_add) {
        context->focus_device = device;
    } else {
        context->focus_device = NULL;
    }

    bt_sink_srv_report_id("[Sink][StaMgr]set focus device, 0x%x", 1, context->focus_device);
}


static void bt_sink_srv_state_manager_add_played_device(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_manager_device_t *device)
{
    bt_sink_srv_state_manager_played_device_t *played_device = NULL;
    bt_sink_srv_state_manager_device_mask_t played_device_mask = BT_SINK_SRV_STATE_MANAGER_DEVICE_MASK_NONE;

#ifdef MTK_AWS_MCE_ENABLE
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    if (BT_AWS_MCE_ROLE_AGENT != role) {
        return;
    }
#endif

    /* 1. Remove existed played device. */
    for (uint32_t i = 0; i < BT_SINK_SRV_STATE_MANAGER_MAX_PLAYED_DEVICE_NUM; i++) {
        if (device->type == context->played_devices[i].type &&
            0 == bt_sink_srv_memcmp(&device->address, &context->played_devices[i].address, sizeof(bt_bd_addr_t))) {
            played_device = &context->played_devices[i];
            played_device_mask = context->played_devices[i].mask;
        }
    }

    if (NULL != played_device) {
        bt_sink_srv_state_manager_remove_played_device(played_device->type, &played_device->address);
    }

    /* 2. Add a new played device. */
    memmove(&context->played_devices[1],
            &context->played_devices[0],
            (BT_SINK_SRV_STATE_MANAGER_MAX_PLAYED_DEVICE_NUM - 1) * sizeof(bt_sink_srv_state_manager_played_device_t));

    /* 3. Update new played device. */
    played_device = &context->played_devices[0];

    played_device->type = device->type;
    played_device->mask = played_device_mask;
    bt_sink_srv_memcpy(&played_device->address, &device->address, sizeof(bt_bd_addr_t));

    if (BT_SINK_SRV_STATE_MANAGER_IS_CALL_STATE(device->call_state) ||
        BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED != device->call_audio_state) {
        bt_sink_srv_state_manager_set_played_device_mask(
            context,
            played_device,
            BT_SINK_SRV_STATE_MANAGER_DEVICE_MASK_CALL);
    }

    if (BT_SINK_SRV_STATE_MANAGER_IS_MEDIA_STATE(device->media_state)) {
        bt_sink_srv_state_manager_set_played_device_mask(
            context,
            played_device,
            BT_SINK_SRV_STATE_MANAGER_DEVICE_MASK_MEDIA);
    }

    bt_sink_srv_report_id("[Sink][StaMgr]add played device, mask 0x%x", 1, played_device->mask);
    bt_sink_srv_state_manager_sync_played_devices(context);
}

bool bt_sink_srv_state_manager_remove_played_device(
    bt_sink_srv_state_manager_device_type_t type,
    bt_bd_addr_t *address)
{
    uint32_t list_number = 0;
    int32_t delete_index = -1;
    bt_sink_srv_state_manager_context_t *context = bt_sink_srv_state_manager_get_context();

#if 0
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    if (BT_AWS_MCE_ROLE_AGENT != role) {
        return false;
    }
#endif

    if (BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_INVALID == type) {
        return false;
    }

    for (uint32_t i = 0; i < BT_SINK_SRV_STATE_MANAGER_MAX_PLAYED_DEVICE_NUM; i++) {
        if (BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_INVALID != context->played_devices[i].type) {
            list_number++;
        }
        if (type == context->played_devices[i].type &&
            (NULL == address || 0 == bt_sink_srv_memcmp(address, &context->played_devices[i].address, sizeof(bt_bd_addr_t)))) {
            delete_index = i;
            break;
        }
    }

    if (-1 != delete_index) {
        uint32_t move_number = BT_SINK_SRV_STATE_MANAGER_MAX_PLAYED_DEVICE_NUM - delete_index - 1;

        bt_sink_srv_report_id("[Sink][StaMgr]delete played device, index %d(mask 0x%x, total %d)", 3,
                              delete_index, context->played_devices[delete_index].mask, list_number);

        if (0 != move_number && BT_SINK_SRV_STATE_MANAGER_MAX_PLAYED_DEVICE_NUM - 1 != delete_index) {
            memmove(&context->played_devices[delete_index],
                    &context->played_devices[delete_index + 1],
                    move_number * sizeof(bt_sink_srv_state_manager_played_device_t));
        }

        if (0 < list_number) {
            bt_sink_srv_memset(&context->played_devices[list_number - 1],
                               0,
                               sizeof(bt_sink_srv_state_manager_played_device_t));
        }
    }

    bt_sink_srv_report_id("[Sink][StaMgr]delete played device, list_number: %d delete_index: %d", 2,
                          list_number, delete_index);

    bt_sink_srv_state_manager_sync_played_devices(context);
    return (-1 != delete_index);
}

static void bt_sink_srv_state_manager_set_played_device_mask(
    bt_sink_srv_state_manager_context_t *context,
    bt_sink_srv_state_manager_played_device_t *played_device,
    bt_sink_srv_state_manager_device_mask_t mask)
{
    for (uint32_t i = 0; i < BT_SINK_SRV_STATE_MANAGER_MAX_PLAYED_DEVICE_NUM; i++) {
        if (played_device == &context->played_devices[i]) {
            context->played_devices[i].mask |= mask;
        } else {
            context->played_devices[i].mask &= ~mask;
        }
    }
}

void bt_sink_srv_state_manager_sync_played_devices(bt_sink_srv_state_manager_context_t *context)
{
#ifdef MTK_AWS_MCE_ENABLE
    bt_status_t status = BT_STATUS_FAIL;
    bt_aws_mce_report_info_t report_info = {0};
    uint8_t buffer[sizeof(bt_sink_srv_state_manager_sync_played_t)] = {0};
    bt_sink_srv_state_manager_sync_played_t *sync_played = (bt_sink_srv_state_manager_sync_played_t *)&buffer;


    /* 1. Fill data. */
    sync_played->header.type = BT_SINK_SRV_STATE_MANAGER_SYNC_TYPE_PLAYED;
    sync_played->header.length = sizeof(bt_sink_srv_state_manager_sync_played_t);
    sync_played->header.direction = BT_SINK_SRV_STATE_MANAGER_SYNC_DIRECTION_PARTNER;
    bt_sink_srv_memcpy(&sync_played->played_devices, &context->played_devices, sizeof(sync_played->played_devices));

    /* 2. Fill report info. */
    report_info.module_id = BT_AWS_MCE_REPORT_MODULE_SINK_STAMGR;
    report_info.param = (void *)&buffer;
    report_info.param_len = sizeof(buffer);

    status = bt_aws_mce_report_send_urgent_event(&report_info);
    (void)status;
    bt_sink_srv_report_id("[Sink][StaMgr]sync played, status 0x%x", 1, status);
#endif
}

static void bt_sink_srv_state_manager_sync_play_count(
    bt_sink_srv_state_manager_context_t *context)
{
#if defined(MTK_AWS_MCE_ENABLE)
    bt_status_t status = BT_STATUS_FAIL;
    bt_aws_mce_report_info_t report_info = {0};
    bt_sink_srv_state_manager_sync_play_count_t sync_count = {{0}};

    if (BT_AWS_MCE_ROLE_AGENT != bt_connection_manager_device_local_info_get_aws_role()) {
        return;
    }

    sync_count.header.type = BT_SINK_SRV_STATE_MANAGER_SYNC_TYPE_PLAY_COUNT;
    sync_count.header.length = sizeof(bt_sink_srv_state_manager_sync_play_count_t);
    sync_count.header.direction = BT_SINK_SRV_STATE_MANAGER_SYNC_DIRECTION_PARTNER;

    report_info.module_id = BT_AWS_MCE_REPORT_MODULE_SINK_STAMGR;
    report_info.param = (void *)&sync_count;
    report_info.param_len = sizeof(bt_sink_srv_state_manager_sync_play_count_t);

    status = bt_aws_mce_report_send_urgent_event(&report_info);
    (void)status;
    bt_sink_srv_report_id("[Sink][StaMgr]sync play count, status 0x%x", 1, status);
#endif
}

uint32_t bt_sink_srv_state_manager_get_played_device_list(
    bt_sink_srv_state_manager_played_device_t *list,
    uint32_t list_number)
{
    uint32_t number = 0;
    uint32_t copy_number = 0;
    bt_sink_srv_state_manager_context_t *context = bt_sink_srv_state_manager_get_context();

    if (NULL == list || 0 == list_number) {
        return 0;
    }

    for (uint32_t i = 0; i < BT_SINK_SRV_STATE_MANAGER_MAX_PLAYED_DEVICE_NUM; i++) {
        if (BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_INVALID != context->played_devices[i].type) {
            number++;
        }
    }

    copy_number = (number > list_number) ? list_number : number;
    bt_sink_srv_memcpy(list, context->played_devices, copy_number * sizeof(bt_sink_srv_state_manager_played_device_t));

    return copy_number;
}

#ifdef MTK_BT_CM_SUPPORT
bt_status_t bt_sink_srv_state_manager_cm_callback(
    bt_cm_event_t event,
    void *parameter,
    uint32_t parameter_length)
{
    switch (event) {
        case BT_CM_EVENT_REMOTE_INFO_UPDATE: {
            bt_cm_remote_info_update_ind_t *update_ind = (bt_cm_remote_info_update_ind_t *)parameter;
            bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();

            /* 1.1. Check parameter. */
            if (NULL == update_ind || sizeof(bt_cm_remote_info_update_ind_t) != parameter_length) {
                return BT_STATUS_FAIL;
            }

            /* 1.2. Check if remove played device. */
            bt_sink_srv_report_id("[Sink][StaMgr]cm callback, connected 0x%x(pre connected 0x%x)", 2,
                                  update_ind->connected_service, update_ind->pre_connected_service);

            if ((BT_AWS_MCE_ROLE_NONE == role || BT_AWS_MCE_ROLE_AGENT == role) &&
                (update_ind->pre_connected_service & BT_SINK_SRV_STATE_MANAGER_PROFILE_MASK) &&
                !(update_ind->connected_service & BT_SINK_SRV_STATE_MANAGER_PROFILE_MASK)) {
                bt_sink_srv_state_manager_remove_played_device(BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_EDR,
                                                               &update_ind->address);
            }

#ifdef MTK_AWS_MCE_ENABLE
            if ((BT_AWS_MCE_ROLE_PARTNER == role || BT_AWS_MCE_ROLE_CLIENT == role) &&
                (update_ind->pre_connected_service & BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS)) &&
                !(update_ind->connected_service & BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS))) {
                bt_sink_srv_report_id("[Sink][StaMgr]cm callback, clear played devices", 0);
                while (bt_sink_srv_state_manager_remove_played_device(BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_EDR, NULL)) {
                    ;
                }
            }
#endif

            break;
        }

        default: {
            break;
        }
    }

    return BT_STATUS_SUCCESS;
}
#endif

#ifdef AIR_LE_AUDIO_ENABLE
void bt_sink_srv_state_manager_gap_le_srv_callback(bt_gap_le_srv_event_t event, void *parameter)
{
    bt_sink_srv_state_manager_device_t *dev = NULL;
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bt_sink_srv_state_manager_context_t *context = bt_sink_srv_state_manager_get_context();
    bt_sink_srv_report_id("[Sink][StaMgr]LE Service callback, event 0x%x", 1, event);

    switch (event) {
        case BT_GAP_LE_SRV_EVENT_DISCONNECT_IND: {
            bt_gap_le_srv_disconnect_ind_t *disconnect_ind = (bt_gap_le_srv_disconnect_ind_t *)parameter;

            /* 1.1. Check parameter. */
            if (NULL == disconnect_ind) {
                return;
            }

            /* 1.2. Remove played device. */

            if (BT_AWS_MCE_ROLE_NONE == role || BT_AWS_MCE_ROLE_AGENT == role) {
                bt_sink_srv_state_manager_remove_played_device(
                    BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_LE,
                    &disconnect_ind->peer_address.addr);
            }

            dev = bt_sink_srv_state_manager_get_device(
                      context,
                      BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_LE,
                      &disconnect_ind->peer_address.addr);

            if (NULL != dev) {
#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_DUMMY_DEIVCE_ENABLE
                bt_sink_srv_state_manager_free_psedev(context, dev);
                if (NULL == dev->dummy_device) {
#endif
                    bt_sink_srv_state_manager_remove_device(context, dev);
#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_DUMMY_DEIVCE_ENABLE
                } else {
                    bt_sink_srv_report_id("[Sink][staMgr]LE Service callback, context not free!", 0);
                }
#endif
            }
            break;
        }

        default: {
            break;
        }
    }
}
#endif

#if defined(MTK_AWS_MCE_ENABLE)
bt_status_t bt_sink_srv_aws_mce_state_manager_callback(bt_msg_type_t msg, bt_status_t status, void *buffer)
{
    bt_sink_srv_state_manager_context_t *context = bt_sink_srv_state_manager_get_context();

    switch (msg) {
        case BT_AWS_MCE_STATE_CHANGED_IND: {
            bt_aws_mce_state_change_ind_t *state_change = (bt_aws_mce_state_change_ind_t *)buffer;

            if (NULL == state_change || 0 == (state_change->state & BT_AWS_MCE_AGENT_STATE_ATTACHED)) {
                break;
            }

            if (BT_AWS_MCE_ROLE_AGENT == bt_connection_manager_device_local_info_get_aws_role()) {
                if (!BT_SINK_SRV_STATE_MANAGER_IS_MEDIA_STATE(context->previous_state)) {
                    bt_sink_srv_state_manager_sync_state_change(context, context->previous_state);
                }

                // bt_sink_srv_state_manager_sync_played_devices(context);
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

#if defined(MTK_AWS_MCE_ENABLE)
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
            bt_sink_srv_state_manager_device_t *device = NULL;

            bt_sink_srv_state_manager_sync_focus_t *sync_focus
                = (bt_sink_srv_state_manager_sync_focus_t *)info->param;

            if (BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_LE == sync_focus->device_type) {
                device = bt_sink_srv_state_manager_get_device(
                             context, sync_focus->device_type, &sync_focus->address);
            } else {
                bt_bd_addr_t address = {0};

                bt_cm_get_connected_devices(
                    BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS), &address, 1);

                device = bt_sink_srv_state_manager_get_device(
                             context, BT_SINK_SRV_STATE_MANAGER_DEVICE_TYPE_EDR, &address);
            }

            if (NULL != device) {
                bt_sink_srv_state_manager_set_focus_device(context, device, true);
            } else {
                bt_sink_srv_report_id("[Sink][StaMgr]aws report callback, cannot find device!", 0);
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

        case BT_SINK_SRV_STATE_MANAGER_SYNC_TYPE_PLAY_COUNT: {
            bt_sink_srv_state_manager_sync_play_count_t *sync_count
                = (bt_sink_srv_state_manager_sync_play_count_t *)info->param;

            bt_sink_srv_report_id("[Sink][StaMgr]aws report callback, play_count: %d", 1,
                                  sync_count->play_count);

            context->play_count = sync_count->play_count;

            break;
        }

        case BT_SINK_SRV_STATE_MANAGER_SYNC_TYPE_REQUEST_STATE: {
            if (BT_AWS_MCE_ROLE_AGENT == bt_connection_manager_device_local_info_get_aws_role() &&
                BT_AWS_MCE_SRV_LINK_NORMAL == bt_aws_mce_srv_get_link_type() &&
                BT_SINK_SRV_STATE_MANAGER_IS_CALL_STATE(context->previous_state)) {
                bt_sink_srv_state_manager_sync_state_change(context, context->previous_state);
            }

            break;
        }

        default: {
            break;
        }
    }
}
#endif
