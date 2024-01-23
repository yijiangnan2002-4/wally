/* Copyright Statement:
 *
 * (C) 2017  Airoha Technology Corp. All rights reserved.
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

#include "bt_sink_srv_avrcp.h"
#include "bt_sink_srv_music.h"
#include "bt_sink_srv_utils.h"
#include "bt_sink_srv_state_notify.h"
#include "bt_sink_srv.h"
#include "audio_src_srv.h"
#include "audio_src_srv_internal.h"
#include "bt_sink_srv_a2dp.h"
#include "bt_device_manager_internal.h"
#include "bt_iot_device_white_list.h"
#include "bt_utils.h"
#ifdef __BT_AWS_MCE_A2DP_SUPPORT__
#include "bt_role_handover.h"
#endif

#define AUD_VOL_IN_LEVEL_VALUE_MAX (127)
/* Function declare */
static int32_t bt_sink_srv_avrcp_set_absolute_volume(bt_avrcp_set_absolute_volume_event_t *vol_ind, bt_status_t status);
static void bt_sink_srv_avrcp_as_tg_callback(uint32_t timer_id, uint32_t data);
static void bt_sink_srv_avrcp_handle_play_status_notification(uint32_t handle, uint8_t avrcp_status, bt_status_t status);
static void bt_sink_srv_avrcp_handle_last_wear_action(bt_sink_srv_music_device_t *dev);
#ifdef MTK_BT_AVRCP_BROWSE_ENABLE
static bt_status_t bt_sink_srv_avrcp_get_folder_item(bt_sink_srv_music_device_t *dev, bt_sink_srv_avrcp_get_folder_items_parameter_t *parameter);
#endif

static uint8_t bt_sink_srv_avrcp_get_volume_value(uint8_t local_volume);

uint8_t bt_sink_srv_avrcp_get_volume_level(uint8_t avrcp_volume)
{
    float avrcp_level_f = avrcp_volume;
    float local_level_f = (avrcp_level_f * BT_SINK_SRV_A2DP_MAX_VOL_LEV) / AUD_VOL_IN_LEVEL_VALUE_MAX + 0.5f;
    bt_sink_srv_report_id("[sink][music][avrcp] get_volume_level-avrcp_vol: %d, local_lev: %d", 2, avrcp_volume, (uint8_t)local_level_f);
    return (uint8_t)local_level_f;
}

static uint8_t bt_sink_srv_avrcp_get_volume_value(uint8_t local_volume)
{
    float local_level_f = local_volume;
    float avrcp_level_f = (local_level_f * AUD_VOL_IN_LEVEL_VALUE_MAX) / BT_SINK_SRV_A2DP_MAX_VOL_LEV + 0.5f;
    bt_sink_srv_report_id("[sink][music][avrcp] get_volume_value-local_vol: %d, avrcp_vol, %d", 2, local_volume, (uint8_t)avrcp_level_f);
    return (uint8_t)avrcp_level_f;
}

bt_status_t bt_sink_srv_avrcp_volume_notification(uint32_t handle, uint8_t
                                                  vol_level, uint8_t type)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    uint8_t volume = 0;
    bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_AVRCP_HD, (void *)(&handle));

    if (dev && dev->volume_change_status) {
        bt_avrcp_send_register_notification_response_t rsp;
        volume = bt_sink_srv_avrcp_get_volume_value(vol_level);
        rsp.event_id = BT_AVRCP_EVENT_VOLUME_CHANGED;
        rsp.parameter_length = 2;
        rsp.response_type = BT_AVRCP_RESPONSE_CHANGED;
        rsp.volume = volume;
        dev->volume_change_status = false;
        ret = bt_avrcp_send_register_notification_response(handle, &rsp);
#ifdef BT_SINK_SRV_AVRCP_CATEGORY_2
    } else {
        if (type == VOLUME_UP) {
            bt_avrcp_send_pass_through_command(handle,
                                               BT_AVRCP_OPERATION_ID_VOLUME_UP, BT_AVRCP_OPERATION_STATE_PUSH);
        } else {
            bt_avrcp_send_pass_through_command(handle,
                                               BT_AVRCP_OPERATION_ID_VOLUME_DOWN, BT_AVRCP_OPERATION_STATE_PUSH);
        }
        bt_sink_srv_report_id("[sink][music][avrcp] It`s category 2 product", 0);
#endif
    }

    bt_sink_srv_report_id("[sink][music][avrcp] vol_noti[e]-vol_level:%d, type:%d, volume_value: %d, ret: %d", 4, vol_level, type, volume, ret);

    return ret;
}

static int32_t bt_sink_srv_avrcp_handle_connect_cnf(bt_avrcp_connect_cnf_t *conn_cnf, bt_status_t status)
{
    bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_AVRCP_HD, (void *)(&(conn_cnf->handle)));

    bt_timer_ext_stop(BT_SINK_SRV_AVRCP_CONNECTION_TIMER_ID);
    if (dev) {
        bt_sink_srv_report_id("[sink][music][avrcp] connect_cnf-avrcp_hd: 0x%x, status:0x%x, dev:0x%x, flag:0x%x, op:0x%x, avrcp_role:0x%x, conn_bit:0x%x",
            6, conn_cnf->handle, status, dev, dev->flag, dev->op, dev->avrcp_role, dev->conn_bit);
        bool clear_avrcp_hd = false;
        dev->last_play_pause_action = BT_SINK_SRV_INVALID_LAST_PLAY_PAUSE_ACTION;
        dev->avrcp_status = BT_SINK_SRV_MUSIC_AVRCP_INVALID_STATUS;
        dev->operation_state = BT_SINK_SRV_MUSIC_AVRCP_NO_STATE;
        dev->operation_action = BT_SINK_SRV_MUSIC_AVRCP_NO_OPERATION;
        if (BT_STATUS_SUCCESS == status) {
            BT_SINK_SRV_SET_FLAG(dev->conn_bit, BT_SINK_SRV_MUSIC_AVRCP_CONN_BIT);
            if ((dev->flag & BT_SINK_SRV_MUSIC_STOP_MUSIC_PENDING)) {
                bt_sink_srv_music_pause_remote_music(dev);
                BT_SINK_SRV_REMOVE_FLAG(dev->flag, BT_SINK_SRV_MUSIC_STOP_MUSIC_PENDING);
            }

            bt_avrcp_get_capability(conn_cnf->handle, BT_AVRCP_CAPABILITY_EVENTS_SUPPORTED);
            if ((dev->op & BT_SINK_SRV_MUSIC_INIT_DISCONNNECT_AVRCP_TIMER_FLAG) && (!(dev->conn_bit & BT_SINK_SRV_MUSIC_A2DP_CONN_BIT))) {
                if (dev->avrcp_flag & BT_SINK_SRV_INIT_AVRCP_BY_REMOTE_DEVICE) {
                    uint32_t data = (uint32_t)dev;
                    bt_timer_ext_start(BT_SINK_SRV_AVRCP_DISCONNECT_TIMER_ID, data,
                                       BT_SINK_SRV_AVRCP_DISCONNECT_TIMER_DUR, bt_sink_srv_a2dp_disconnect_avrcp_timer);
                } else {
                    bt_avrcp_disconnect(dev->avrcp_hd);
                }
            }
            bt_sink_srv_cm_profile_status_notify(&(dev->dev_addr), BT_SINK_SRV_PROFILE_AVRCP, BT_SINK_SRV_PROFILE_CONNECTION_STATE_CONNECTED, status);
        } else if (status == BT_STATUS_AVRCP_SDP_QUERY_NUMER_ERROR) {
            if (dev->avrcp_role == BT_AVRCP_ROLE_CT) {
                if (!(dev->conn_bit & BT_SINK_SRV_MUSIC_AVRCP_CONN_BIT)) {
                    uint32_t data = (uint32_t)dev;
                    bt_timer_ext_start(BT_SINK_SRV_AVRCP_CONN_AS_TG_TIMER_ID, (uint32_t)data,
                                        BT_SINK_SRV_AVRCP_CONNECTION_AS_TG_TIMER_DUR, bt_sink_srv_avrcp_as_tg_callback);

                }
            } else if (dev->avrcp_role == BT_AVRCP_ROLE_TG) {
                bt_timer_ext_stop(BT_SINK_SRV_AVRCP_CONN_AS_TG_TIMER_ID);
                clear_avrcp_hd = true;
            }
        } else {
            bt_sink_srv_cm_profile_status_notify(&(dev->dev_addr), BT_SINK_SRV_PROFILE_AVRCP, BT_SINK_SRV_PROFILE_CONNECTION_STATE_DISCONNECTED, status);
            clear_avrcp_hd = true;
        }

        if (clear_avrcp_hd) {
            dev->avrcp_hd = BT_SINK_SRV_MUSIC_INVALID_HD;
            if (!(dev->conn_bit) && dev->a2dp_hd == BT_SINK_SRV_MUSIC_INVALID_HD
                && dev->handle->substate != BT_SINK_SRV_MUSIC_TRANSIENT_STATE_CLEAR_CODEC) {
                bt_sink_srv_a2dp_free_pseudo_handle(dev->handle);
                dev->handle = NULL;
                bt_sink_srv_music_reset_device(dev);
            }
        }

        BT_SINK_SRV_REMOVE_FLAG(dev->op, BT_SINK_SRV_MUSIC_INIT_DISCONNNECT_AVRCP_TIMER_FLAG);
        BT_SINK_SRV_REMOVE_FLAG(dev->flag, BT_SINK_SRV_MUSIC_FLAG_WAIT_AVRCP_CONN_RESULT);
    } else {
        if (status == BT_STATUS_SUCCESS) {
            bt_avrcp_disconnect(conn_cnf->handle);
        }
    }

    return BT_STATUS_SUCCESS;
}


static int32_t bt_sink_srv_avrcp_handle_connect_ind(bt_avrcp_connect_ind_t *conn_ind, bt_status_t status)
{
    bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_ADDR_A2DP, conn_ind->address);
    int32_t ret = 0;
    uint8_t *addr = (uint8_t *)(conn_ind->address);
    (void)addr;

    if (!dev) {
        dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_ADDR_AVRCP, conn_ind->address);
    }

    bt_sink_srv_report_id("[sink][music][avrcp] connect_ind-avrcp_hd: 0x%x, dev:0x%x, addr--0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x", 8,
        conn_ind->handle, dev, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
    if (!dev) {
        dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_UNUSED, (void *)AUDIO_SRC_SRV_PSEUDO_DEVICE_A2DP);
        if (dev) {
            bt_sink_srv_memcpy(&(dev->dev_addr), conn_ind->address, sizeof(bt_bd_addr_t));

            dev->handle = bt_sink_srv_a2dp_alloc_pseudo_handle();
            dev->handle->type = AUDIO_SRC_SRV_PSEUDO_DEVICE_A2DP;
            dev->handle->dev_id = bt_sink_srv_music_convert_btaddr_to_devid(conn_ind->address);
            dev->handle->priority = AUDIO_SRC_SRV_PRIORITY_NORMAL;
        }
    }
    bt_utils_assert(dev);
    dev->avrcp_hd = conn_ind->handle;
    ret = bt_avrcp_connect_response(conn_ind->handle, true);
    BT_SINK_SRV_SET_FLAG(dev->flag, BT_SINK_SRV_MUSIC_FLAG_WAIT_AVRCP_CONN_RESULT);
    BT_SINK_SRV_SET_FLAG(dev->avrcp_flag, BT_SINK_SRV_INIT_AVRCP_BY_REMOTE_DEVICE);

    return ret;
}


static int32_t bt_sink_srv_avrcp_handle_disconnect_ind(bt_avrcp_disconnect_ind_t *disconn_ind, bt_status_t status)
{
    bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_AVRCP_HD, (void *)(&(disconn_ind->handle)));
    bt_bd_addr_t dev_addr = {0};

    bt_timer_ext_stop(BT_SINK_SRV_AVRCP_CONNECTION_TIMER_ID);
    bt_timer_ext_stop(BT_SINK_SRV_AVRCP_DISCONNECT_TIMER_ID);
    bt_timer_ext_stop(BT_SINK_SRV_AVRCP_CONN_AS_TG_TIMER_ID);
    bt_sink_srv_report_id("[sink][music][avrcp] disconnect_ind-avrcp_hd: 0x%x, dev:0x%x", 2, disconn_ind->handle, dev);
    if (dev) {
        memcpy(&dev_addr, &(dev->dev_addr), sizeof(bt_bd_addr_t));
        dev->last_play_pause_action = BT_SINK_SRV_INVALID_LAST_PLAY_PAUSE_ACTION;
        dev->last_wear_action = BT_SINK_SRV_INVALID_LAST_PLAY_PAUSE_ACTION;
        dev->volume_change_status = false;
        bt_timer_ext_stop(dev->avrcp_cmd_timer_id);
        dev->avrcp_cmd_timer_id = 0;
        bt_timer_ext_stop(dev->avrcp_push_release_timer_id);
        dev->avrcp_push_release_timer_id = 0;

        BT_SINK_SRV_REMOVE_FLAG(dev->conn_bit, BT_SINK_SRV_MUSIC_AVRCP_CONN_BIT);
        BT_SINK_SRV_REMOVE_FLAG(dev->avrcp_flag, BT_SINK_SRV_INIT_AVRCP_BY_REMOTE_DEVICE);
        dev->avrcp_hd = BT_SINK_SRV_MUSIC_INVALID_HD;
        if (!(dev->conn_bit) && dev->a2dp_hd == BT_SINK_SRV_MUSIC_INVALID_HD
            && dev->handle->substate != BT_SINK_SRV_MUSIC_TRANSIENT_STATE_CLEAR_CODEC) {
            bt_sink_srv_a2dp_free_pseudo_handle(dev->handle);
            dev->handle = NULL;
            bt_sink_srv_music_reset_device(dev);
        }
        bt_sink_srv_cm_profile_status_notify(&dev_addr, BT_SINK_SRV_PROFILE_AVRCP, BT_SINK_SRV_PROFILE_CONNECTION_STATE_DISCONNECTED, status);
    }

    return 0;
}


static int32_t bt_sink_srv_avrcp_handle_pass_through_cnf(bt_avrcp_pass_through_cnf_t *through_cnf, bt_status_t status)
{
    int32_t ret = 0;

    bt_sink_srv_report_id("[sink][music][avrcp] pass_through_cnf-handle: 0x%x, op_id:0x%x, status:0x%x", 3,
        through_cnf->handle, through_cnf->op_id, status);
    if ((through_cnf->op_id == BT_AVRCP_OPERATION_ID_FAST_FORWARD)
        || (through_cnf->op_id == BT_AVRCP_OPERATION_ID_REWIND)
        || (through_cnf->op_id == BT_AVRCP_OPERATION_ID_PAUSE)
        || (through_cnf->op_id == BT_AVRCP_OPERATION_ID_PLAY)) {

        return ret;
    }

    if (status == BT_STATUS_SUCCESS) {
        if (through_cnf->op_state == 0) {
            ret = bt_avrcp_send_pass_through_command(through_cnf->handle, through_cnf->op_id, BT_AVRCP_OPERATION_STATE_RELEASED);
        }
    }

    return ret;
}

static int32_t bt_sink_srv_avrcp_handle_pass_through_ind(bt_avrcp_pass_through_command_ind_t *through_ind, bt_status_t status)
{
    bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_AVRCP_HD, (void *)(&(through_ind->handle)));
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
    volume_change_type_t type = VOLUME_UP;
    int32_t ret = 0;

    if (BT_AVRCP_OPERATION_ID_VOLUME_UP == through_ind->op_id
        || BT_AVRCP_OPERATION_ID_VOLUME_DOWN == through_ind->op_id) {

        if (BT_AVRCP_OPERATION_ID_VOLUME_DOWN == through_ind->op_id) {
            type = VOLUME_DOWN;
        }

        if (BT_AVRCP_OPERATION_STATE_RELEASED == through_ind->op_state && ctx->run_dev == dev && dev) {
            bt_sink_srv_a2dp_change_volume(type, 1, 0, dev);
        }
        ret = bt_avrcp_send_pass_through_response(through_ind->handle,
                                                  BT_AVRCP_RESPONSE_ACCEPTED, through_ind->op_id, through_ind->op_state);
    } else {
        ret = bt_avrcp_send_pass_through_response(through_ind->handle,
                                                  BT_AVRCP_RESPONSE_REJECTED, through_ind->op_id, through_ind->op_state);
    }


    bt_sink_srv_report_id("[sink][music][avrcp] pass_through_ind-handle:0x%08x, op_id:0x%02x, op_state:0x%02x", 3,
                          through_ind->handle, through_ind->op_id, through_ind->op_state);

    return ret;
}

static int32_t bt_sink_srv_avrcp_handle_event_notification_ind(bt_avrcp_event_notification_t *noti_ind, bt_status_t status)
{
    int32_t ret = BT_STATUS_FAIL;

    if (noti_ind) {
        bt_sink_srv_report_id("[sink][music][avrcp] event_notification_ind(changed)--evt_id: %d, status: 0x%x, AVRCP status: %d",
            3, noti_ind->event_id, noti_ind->status, status);
        if (BT_AVRCP_EVENT_PLAYBACK_STATUS_CHANGED == noti_ind->event_id) {
            if (BT_STATUS_SUCCESS == status) {
                ret = bt_avrcp_register_notification(noti_ind->handle, BT_AVRCP_EVENT_PLAYBACK_STATUS_CHANGED, 0);
            }
            bt_sink_srv_avrcp_handle_play_status_notification(noti_ind->handle, noti_ind->status, status);
        }
    }
    return ret;
}

static int32_t bt_sink_srv_avrcp_handle_register_notification_ind(bt_avrcp_register_notification_commant_t *register_ind, bt_status_t status)
{
    int32_t ret = 0;

    if (BT_STATUS_SUCCESS == status) {
        if (BT_AVRCP_EVENT_VOLUME_CHANGED == register_ind->event_id) {

            bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_AVRCP_HD, (void *)(&(register_ind->handle)));
            bt_avrcp_send_register_notification_response_t rsp = {0};
            uint8_t vol_lev = 0;
            bt_utils_assert(dev);

            dev->volume_change_status = true;
            bt_sink_srv_music_get_nvdm_data(&(dev->dev_addr), BT_SINK_SRV_MUSIC_DATA_VOLUME, &vol_lev);

            rsp.event_id = BT_AVRCP_EVENT_VOLUME_CHANGED;
            rsp.parameter_length = 2;
            rsp.response_type = BT_AVRCP_RESPONSE_INTERIM;
            rsp.volume = bt_sink_srv_avrcp_get_volume_value(vol_lev);

            ret = bt_avrcp_send_register_notification_response(register_ind->handle, &rsp);
            bt_sink_srv_report_id("[sink][music][avrcp] register_notificatin_ind[s]-volume_value: %d, ret:0x%x", 2, rsp.volume, ret);
        }
    }

    return ret;
}

static int32_t bt_sink_srv_avrcp_set_absolute_volume(bt_avrcp_set_absolute_volume_event_t *vol_ind, bt_status_t status)
{
    if (BT_STATUS_SUCCESS == status) {
        uint8_t volume = vol_ind->volume;
        bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_AVRCP_HD, (void *)(&(vol_ind->handle)));
        if (dev != NULL) {
#ifdef MTK_AP_DEFAULT_TYPE
            if (bt_iot_device_white_list_check_iot_case((bt_bd_addr_t *)&dev->dev_addr, BT_IOT_MUSIC_IGNORE_ABSOLUTE_VOLUME)) {
                volume = 0x7F;
            }
#endif
            bt_sink_srv_a2dp_set_volume(volume, dev);
        }

        bt_sink_srv_report_id("[sink][music][avrcp] absolute_volume_ind-vol_ind->volume:%d, volume_value: %d", 2, vol_ind->volume, volume);
    }
    return 0;
}

bt_status_t bt_sink_srv_avrcp_handle_set_absolute_volume_ind(bt_avrcp_set_absolute_volume_event_t *absolute_volume_event, uint32_t status)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    if (BT_STATUS_SUCCESS == status) {
        bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_AVRCP_HD, (void *) & (absolute_volume_event->handle));
        bt_avrcp_send_set_absolute_volume_response(absolute_volume_event->handle, absolute_volume_event->volume);
        bt_utils_assert(dev && "NULL dev");

        ret = bt_sink_srv_avrcp_set_absolute_volume(absolute_volume_event, status);
        bt_sink_srv_report_id("[sink][music][avrcp] absolute_volume_ind-volume:0x%02x, ret:0x%08x", 2, absolute_volume_event->volume, ret);
    }
    return ret;
}

bt_sink_srv_music_device_t *bt_sink_srv_avrcp_get_device(void *param, bt_sink_srv_action_t action)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
    bt_sink_srv_music_device_t *dev = ctx->run_dev;
    bt_bd_addr_t *bt_addr = param;

    if (action == BT_SINK_SRV_ACTION_GET_CAPABILITY) {
        bt_sink_srv_avrcp_get_capability_parameter_t *cap_para = (bt_sink_srv_avrcp_get_capability_parameter_t *)param;
        bt_addr = cap_para->address;
    } else if (action == BT_SINK_SRV_ACTION_GET_ELEMENT_ATTRIBUTE) {
        bt_sink_srv_avrcp_get_element_attributes_parameter_t *attribute_parameter = (bt_sink_srv_avrcp_get_element_attributes_parameter_t *)param;
        bt_addr = attribute_parameter->address;
    } else if (action == BT_SINK_SRV_ACTION_GET_FOLDER_ITEM) {
        bt_sink_srv_avrcp_get_folder_items_parameter_t *folder_parameter = (bt_sink_srv_avrcp_get_folder_items_parameter_t *)param;
        bt_addr = folder_parameter->address;
    } else {
        bt_addr = param;
    }

    if (role == BT_AWS_MCE_ROLE_NONE || role == BT_AWS_MCE_ROLE_AGENT) {
        if (bt_addr) {
            dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_ADDR_AVRCP, bt_addr);
        } else if (!dev) {
            bt_sink_srv_music_device_list_t *device_list = bt_sink_srv_music_get_played_device_list(true);
            if (device_list && device_list->number) {
                dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_ADDR_A2DP, &(device_list->device_list[0]));
            } else {
                dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_SP, NULL);
            }
        }
    }
    /* check dev avrcp profile connection state */
    if (dev && !(dev->conn_bit & BT_SINK_SRV_MUSIC_AVRCP_CONN_BIT)) {
        dev = NULL;
    }
    bt_sink_srv_report_id("[sink][music][avrcp] bt_sink_srv_avrcp_get_device, dev:0x%08x, action:0x%08x", 2, dev, action);
    return dev;
}

static void bt_sink_srv_avrcp_send_get_play_status_cnf(bt_avrcp_media_play_status_response_t *cnf, bt_status_t status, bt_bd_addr_t *address)
{
    bt_sink_srv_avrcp_get_play_status_cnf_t params = {0};

    params.status = status;
    bt_sink_srv_memcpy(&params.address, address, sizeof(bt_bd_addr_t));

    if (BT_STATUS_SUCCESS == status) {
        params.song_length = cnf->song_length;
        params.song_position = cnf->song_position;
        params.play_status = cnf->status;
    }

    bt_sink_srv_event_callback(BT_SINK_SRV_EVENT_AVRCP_GET_PLAY_STATUS_CNF, &params, sizeof(bt_sink_srv_avrcp_get_play_status_cnf_t));

    extern void bt_sink_srv_atci_get_play_status(bt_sink_srv_avrcp_get_play_status_cnf_t *params);
    bt_sink_srv_atci_get_play_status(&params);
}


static int32_t bt_sink_srv_avrcp_handle_get_play_status_cnf(bt_avrcp_media_play_status_response_t *cnf, bt_status_t status)
{
    bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_AVRCP_HD, (void *)(&(cnf->handle)));
    bt_utils_assert(dev && "Null dev");

    bt_sink_srv_avrcp_send_get_play_status_cnf(cnf, status, &dev->dev_addr);

    return 0;
}

static void bt_sink_srv_avrcp_send_get_capability_cnf(bt_avrcp_get_capability_response_t *cnf, bt_status_t status, bt_bd_addr_t *address)
{
    bt_sink_srv_avrcp_get_capability_cnf_t params = {0};

    if (cnf->type != BT_AVRCP_CAPABILITY_EVENTS_SUPPORTED) {
        return;
    }

    bt_sink_srv_memcpy(&(params.address), address, sizeof(bt_bd_addr_t));
    params.status = status;

    if (BT_STATUS_SUCCESS == status) {
        
        params.number = cnf->capability_count;

        uint32_t copy_num = cnf->capability_count;
        if (copy_num > BT_SINK_SRV_MAX_CAPABILITY_VALUE_LENGTH) {
            copy_num = BT_SINK_SRV_MAX_CAPABILITY_VALUE_LENGTH;
        }
        bt_sink_srv_memcpy(&(params.capability_value[0]), cnf->event, copy_num);
    }

    bt_sink_srv_event_callback(BT_SINK_SRV_EVENT_AVRCP_GET_CAPABILITY_CNF, &params, sizeof(bt_sink_srv_avrcp_get_capability_cnf_t));
}


static int32_t bt_sink_srv_avrcp_handle_get_capability_cnf(bt_avrcp_get_capability_response_t *cnf, bt_status_t status)
{
    bt_sink_srv_music_device_t *dev = NULL;

    dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_AVRCP_HD, (void *)(&(cnf->handle)));
    bt_utils_assert(dev && "Null dev");
    if (cnf->type == BT_AVRCP_CAPABILITY_EVENTS_SUPPORTED) {
        
        uint32_t cap_num = cnf->capability_count;
        if (cap_num > BT_SINK_SRV_MAX_CAPABILITY_VALUE_LENGTH) {
            cap_num = BT_SINK_SRV_MAX_CAPABILITY_VALUE_LENGTH;
        }
        for(int i = 0; i < cap_num; i++) {
            if((cnf->event)[i] == BT_AVRCP_EVENT_PLAYBACK_STATUS_CHANGED){
                bt_avrcp_register_notification(cnf->handle, BT_AVRCP_EVENT_PLAYBACK_STATUS_CHANGED, 0);
                break;
            }
        }
    }

    bt_sink_srv_avrcp_send_get_capability_cnf(cnf, status, &(dev->dev_addr));

    return 0;
}

static void bt_sink_srv_avrcp_send_get_element_attributes_cnf(bt_avrcp_get_element_attributes_response_t *cnf,
    bt_status_t status, bt_bd_addr_t *address, bool save_left_data)
{
    bt_sink_srv_avrcp_get_element_attributes_cnf_t params = {0};
    static uint8_t *left_attribute_data = NULL;
    static uint32_t left_length = 0;
    static uint8_t left_number = 0;
    uint32_t parsered_length = 0;
    uint32_t parsered_num = 0;
    uint8_t *allocate_attribute = NULL;
    bt_sink_srv_report_id("[sink][music][avrcp] get_element_attributes_cnf(s):packet_type:%d,length:%d,number:0x%x,data:0x%x,left_length:%d,left_number:%d,left_attribute_data:0x%x,save_left_data:%d",
        8, cnf->packet_type, cnf->length, cnf->number, cnf->data, left_length, left_number, left_attribute_data, save_left_data);

    bt_sink_srv_memcpy(&params.address, address, sizeof(bt_bd_addr_t));
    params.status = status;
    if (BT_STATUS_SUCCESS == status) {
        params.packet_type = cnf->packet_type;
        params.length = cnf->length;
        if (params.packet_type == BT_AVRCP_METADATA_PACKET_TYPE_CONTINUE || params.packet_type == BT_AVRCP_METADATA_PACKET_TYPE_END) {
            params.number = left_number;
            params.attribute_list = (bt_avrcp_get_element_attributes_response_value_t*)(cnf->data);
            // alloc new space to combine the left data of pre-packet and this packet
            if (left_length && left_attribute_data && cnf->data) {
                allocate_attribute = (uint8_t*)bt_sink_srv_memory_alloc(left_length+params.length);
                bt_sink_srv_report_id("[sink][music][avrcp] get_element_attributes_cnf-alloc allocate_attribute:0x%x", 1, allocate_attribute);
                bt_sink_srv_memcpy(allocate_attribute, left_attribute_data, left_length);
                bt_sink_srv_memcpy(allocate_attribute+left_length, cnf->data, params.length);
                params.length += left_length;
                params.attribute_list = (bt_avrcp_get_element_attributes_response_value_t*)allocate_attribute;
            }
            uint32_t i = 0;
            uint32_t sum_len = 0;
            bt_avrcp_get_element_attributes_response_value_t *p = params.attribute_list;

            if (left_length >= 8) {
                // header is swapped in hummingbird if left data is more than header length
                i = 1;
                sum_len += (8 + p->attribute_value_length);/*4: attribute id , 2: set id , 2 value len*/
                p = (bt_avrcp_get_element_attributes_response_value_t *)((uint8_t *)p
                                                                         + sizeof(bt_avrcp_get_element_attributes_response_value_t)
                                                                         + p->attribute_value_length - 1);
            }
            for (; i < left_number && sum_len < params.length; i++) {
                bt_sink_srv_util_endian_order_swap((uint8_t *)(&p->attribute_id),
                                     (uint8_t *)(&p->attribute_id), 4);
                bt_sink_srv_util_endian_order_swap((uint8_t *)(&p->character_set_id),
                                     (uint8_t *)(&p->character_set_id), 2);
                bt_sink_srv_util_endian_order_swap((uint8_t *)(&p->attribute_value_length),
                                     (uint8_t *)(&p->attribute_value_length),
                                     2);
                sum_len += (8 + p->attribute_value_length);/*4: attribute id , 2: set id , 2 value len*/
                p = (bt_avrcp_get_element_attributes_response_value_t *)((uint8_t *)p
                                                                         + sizeof(bt_avrcp_get_element_attributes_response_value_t)
                                                                         + p->attribute_value_length - 1);
            }
        } else {
            params.number = cnf->number;
            params.attribute_list = cnf->attribute_list;
        }
    }

    left_length = 0;
    left_number = 0;
    if (left_attribute_data) {
        bt_sink_srv_report_id("[sink][music][avrcp] get_element_attributes_cnf-free left_attribute_data:0x%x", 1, left_attribute_data);
        bt_sink_srv_memory_free(left_attribute_data);
        left_attribute_data = NULL;
    }

    bt_sink_srv_event_callback(BT_SINK_SRV_EVENT_AVRCP_GET_ELEMENT_ATTRIBUTES_CNF, &params, sizeof(bt_sink_srv_avrcp_get_element_attributes_cnf_t));

    extern uint32_t bt_sink_srv_atci_get_element_attributes(bt_sink_srv_avrcp_get_element_attributes_cnf_t *params, uint32_t *parsered_num);
    parsered_length = bt_sink_srv_atci_get_element_attributes(&params, &parsered_num);
    if (save_left_data) {
        if (params.number >= parsered_num) {
            left_number = params.number - parsered_num;
        }
        if (parsered_length) {
            if (params.length > parsered_length) {
                left_length = params.length - parsered_length;
                left_attribute_data = (uint8_t*)bt_sink_srv_memory_alloc(left_length);
                bt_sink_srv_memcpy(left_attribute_data, (uint8_t*)(params.attribute_list)+parsered_length, left_length);
            }
        }
    }
    if (allocate_attribute) {
        bt_sink_srv_memory_free(allocate_attribute);
        allocate_attribute = NULL;
    }
    bt_sink_srv_report_id("[sink][music][avrcp] get_element_attributes_cnf(e): left_attribute_data:0x%x, left_length:%d, left_number:%d, allocate_attribute:0x%x",
        4, left_attribute_data, left_length, left_number, allocate_attribute);
}

static int32_t bt_sink_srv_avrcp_handle_get_element_attributes_cnf(bt_avrcp_get_element_attributes_response_t *cnf, bt_status_t status)
{
    bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_AVRCP_HD, (void *)(&(cnf->handle)));
    bt_utils_assert(dev && "Null dev");
    bool save_left_data = false;

    if ((dev) && (dev->conn_bit & BT_SINK_SRV_MUSIC_AVRCP_CONN_BIT)) {
        if (cnf->packet_type == BT_AVRCP_METADATA_PACKET_TYPE_START && !(dev->avrcp_flag & BT_SINK_SRV_AVRCP_GET_ELEMENT_ATTRIBUTE_FRAGMENT_SUPPORT)) {
            bt_avrcp_abort_continuing_response(cnf->handle, BT_AVRCP_PDU_ID_GET_ELEMENT_ATTRIBUTES);
        }

        if (!(dev->avrcp_flag & BT_SINK_SRV_AVRCP_GET_ELEMENT_ATTRIBUTE_FRAGMENT_SUPPORT)
            && ((cnf->packet_type == BT_AVRCP_METADATA_PACKET_TYPE_CONTINUE) || (cnf->packet_type == BT_AVRCP_METADATA_PACKET_TYPE_END))) {
            return 0;
        }

        if ((dev->avrcp_flag & BT_SINK_SRV_AVRCP_GET_ELEMENT_ATTRIBUTE_FRAGMENT_SUPPORT) &&
            (cnf->packet_type == BT_AVRCP_METADATA_PACKET_TYPE_START ||
             cnf->packet_type == BT_AVRCP_METADATA_PACKET_TYPE_CONTINUE)) {
            bt_avrcp_request_continuing_response(cnf->handle, BT_AVRCP_PDU_ID_REQUEST_CONTINUING_RSP);
            save_left_data = true;
        }

        bt_sink_srv_avrcp_send_get_element_attributes_cnf(cnf, status, &dev->dev_addr, save_left_data);
    }

    return 0;
}


int32_t bt_sink_srv_avrcp_send_pass_through(uint32_t handle, bt_avrcp_operation_id_t op_id, bt_avrcp_operation_state_t op_state)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    bt_sink_srv_music_device_t *dev = NULL;

    dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_AVRCP_HD, (void *)(&handle));
    if ((dev) && (dev->conn_bit & BT_SINK_SRV_MUSIC_AVRCP_CONN_BIT)) {
        ret = bt_avrcp_send_pass_through_command(handle, op_id, op_state);
    }
    bt_sink_srv_report_id("[sink][music][avrcp] send_pass_through-hd: 0x%x, ret: %d", 2, handle, ret);
    return ret;
}

bt_status_t bt_sink_srv_avrcp_action_handler(bt_sink_srv_action_t action, void *param)
{
    bt_sink_srv_mutex_lock();
    bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_context()->run_dev;
    bt_status_t ret = 0;
    (void)ret;

    if (BT_SINK_SRV_ACTION_PLAY == action && param) {
        bt_sink_srv_action_play_t *play_para = param;
        if (BT_SINK_SRV_DEVICE_EDR == play_para->type) {  
            bt_bd_addr_t *bt_addr = &(play_para->address);
            dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_ADDR_AVRCP, (void *)bt_addr);
        }
    } else if (!dev) {
        dev = bt_sink_srv_avrcp_get_device(param, action);
    }

    if(!dev || (0 == (dev->conn_bit & BT_SINK_SRV_MUSIC_AVRCP_CONN_BIT))) {
        bt_sink_srv_mutex_unlock();
        return BT_STATUS_FAIL;
    }

    switch (action) {
        case BT_SINK_SRV_ACTION_PLAY: {
            ret = bt_sink_srv_avrcp_play_music(dev);
            break;
        }
        case BT_SINK_SRV_ACTION_PAUSE: {
            ret = bt_sink_srv_music_pause_remote_music(dev);
            break;
        }
        case BT_SINK_SRV_ACTION_PLAY_PAUSE: {
            bt_sink_srv_music_decide_play_pause_action(dev);
            break;
        }

        case BT_SINK_SRV_ACTION_NEXT_TRACK: {
            ret = bt_sink_srv_avrcp_change_track(dev->avrcp_hd, false);
            break;
        }

        case BT_SINK_SRV_ACTION_PREV_TRACK: {
            ret = bt_sink_srv_avrcp_change_track(dev->avrcp_hd, true);
            break;
        }
        case BT_SINK_SRV_ACTION_FAST_FORWARD: {
            bt_sink_srv_avrcp_operation_state_t *operation_state = (bt_sink_srv_avrcp_operation_state_t *)param;
            if (*operation_state == BT_SINK_SRV_AVRCP_OPERATION_PRESS) {
                ret = bt_sink_srv_avrcp_send_pass_through(dev->avrcp_hd, BT_AVRCP_OPERATION_ID_FAST_FORWARD, BT_AVRCP_OPERATION_STATE_PUSH);
            } else {
                ret = bt_sink_srv_avrcp_send_pass_through(dev->avrcp_hd, BT_AVRCP_OPERATION_ID_FAST_FORWARD, BT_AVRCP_OPERATION_STATE_RELEASED);
            }
            break;
        }

        case BT_SINK_SRV_ACTION_REWIND: {
            bt_sink_srv_avrcp_operation_state_t *operation_state = (bt_sink_srv_avrcp_operation_state_t *)param;
            if (*operation_state == BT_SINK_SRV_AVRCP_OPERATION_PRESS) {
                ret = bt_sink_srv_avrcp_send_pass_through(dev->avrcp_hd, BT_AVRCP_OPERATION_ID_REWIND, BT_AVRCP_OPERATION_STATE_PUSH);
            } else {
                ret = bt_sink_srv_avrcp_send_pass_through(dev->avrcp_hd, BT_AVRCP_OPERATION_ID_REWIND, BT_AVRCP_OPERATION_STATE_RELEASED);
            }
            break;
        }

        case BT_SINK_SRV_ACTION_GET_PLAY_STATUS: {
            ret = bt_avrcp_get_play_status(dev->avrcp_hd);
            break;
        }

        case BT_SINK_SRV_ACTION_GET_CAPABILITY: {
            bt_sink_srv_avrcp_get_capability_parameter_t *capability_parameter = (bt_sink_srv_avrcp_get_capability_parameter_t *)param;
            ret = bt_avrcp_get_capability(dev->avrcp_hd, capability_parameter->type);
            break;
        }

        case BT_SINK_SRV_ACTION_GET_ELEMENT_ATTRIBUTE: {
            bt_sink_srv_avrcp_get_element_attributes_parameter_t *attribute_parameter = (bt_sink_srv_avrcp_get_element_attributes_parameter_t *)param;
            bt_utils_assert(attribute_parameter && "Null parameter");
            if (attribute_parameter->accept_fragment) {
                BT_SINK_SRV_SET_FLAG(dev->avrcp_flag, BT_SINK_SRV_AVRCP_GET_ELEMENT_ATTRIBUTE_FRAGMENT_SUPPORT);
            } else {
                BT_SINK_SRV_REMOVE_FLAG(dev->avrcp_flag, BT_SINK_SRV_AVRCP_GET_ELEMENT_ATTRIBUTE_FRAGMENT_SUPPORT);
            }
            ret = bt_avrcp_get_element_attributes(dev->avrcp_hd, attribute_parameter->attribute_size, attribute_parameter->attribute_list);
            break;
        }

#ifdef MTK_BT_AVRCP_BROWSE_ENABLE
        case BT_SINK_SRV_ACTION_GET_FOLDER_ITEM: {
            bt_sink_srv_avrcp_get_folder_items_parameter_t *folder_parameter = (bt_sink_srv_avrcp_get_folder_items_parameter_t *)param;
            ret = bt_sink_srv_avrcp_get_folder_item(dev, folder_parameter);
            break;
        }
#endif
    }

    bt_sink_srv_report_id("[sink][music][avrcp] process_avrcp_action-action:0x%x, ret: %d", 2, action, ret);
    bt_sink_srv_mutex_unlock();

    return BT_STATUS_SUCCESS;
}

#ifdef MTK_BT_AVRCP_BROWSE_ENABLE
static bt_status_t bt_sink_srv_avrcp_handle_browse_connect_cnf(bt_avrcp_browse_connect_cnf_t *conn_cnf, bt_status_t status)
{
    bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_AVRCP_HD, (void *)(&(conn_cnf->handle)));
    bt_status_t ret = BT_STATUS_SUCCESS;
    bt_utils_assert(dev && "browse cnf null dev");

    if (status == BT_STATUS_SUCCESS) {
        BT_SINK_SRV_SET_FLAG(dev->conn_bit, BT_SINK_SRV_MUSIC_AVRCP_BROWSING_CONN_BIT);
        if (dev && (dev->avrcp_flag & BT_SINK_SRV_AVRCP_GET_FOLDER_ITEM)) {
            BT_SINK_SRV_REMOVE_FLAG(dev->avrcp_flag, BT_SINK_SRV_AVRCP_GET_FOLDER_ITEM)

            bt_avrcp_browse_get_folder_items_t folder_item = {0};

            folder_item.scope = dev->folder_parameter.scope;
            folder_item.start_item = dev->folder_parameter.start_item;
            folder_item.end_item = dev->folder_parameter.end_item;
            folder_item.attribute_count = 0;
            folder_item.attribute_list = NULL;
            ret = bt_avrcp_browse_get_folder_items(dev->avrcp_browse_hd, &folder_item);
            bt_sink_srv_report_id("[sink][music][avrcp] scope:0x%02x, start_item:0x%08x, end:0x%08x, count:0x%02x", 4, folder_item.scope, folder_item.start_item,
                                  folder_item.end_item, folder_item.attribute_count);
        }
    }
    bt_sink_srv_report_id("[sink][music][avrcp] get_music_player, ret: %x, status:0x%08x", 2, ret, status);

    return ret;
}

static bt_status_t bt_sink_srv_avrcp_handle_browse_disconnect_ind(bt_avrcp_browse_disconnect_ind_t *conn_cnf, bt_status_t status)
{
    bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_AVRCP_HD, (void *)(&(conn_cnf->handle)));
    bt_utils_assert(dev && "folder item cnf null dev");
    if (dev) {
        BT_SINK_SRV_REMOVE_FLAG(dev->avrcp_flag, BT_SINK_SRV_AVRCP_GET_FOLDER_ITEM);
        BT_SINK_SRV_REMOVE_FLAG(dev->conn_bit, BT_SINK_SRV_MUSIC_AVRCP_BROWSING_CONN_BIT);
        bt_sink_srv_init_role(BT_AVRCP_ROLE_CT);
    }

    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_sink_srv_avrcp_handle_get_folder_item_cnf(bt_avrcp_browse_get_folder_items_cnf_t *folder_item, bt_status_t status)
{
    bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_AVRCP_HD, (void *)(&(folder_item->handle)));
    bt_utils_assert(dev && "folder item cnf null dev");
    if (dev) {
        bt_sink_srv_avrcp_get_folder_items_cnf_t param = {0};
        param.status = status;
        if (status == BT_STATUS_SUCCESS) {
            bt_sink_srv_memcpy(&(param.address), &(dev->dev_addr), sizeof(bt_bd_addr_t));
            param.uid_counter = folder_item->uid_counter;
            param.number_of_items = folder_item->number_of_items;
            param.size_of_item_list = folder_item->size_of_item_list;
            param.items_list = folder_item->items_list;
        }
        bt_sink_srv_event_callback(BT_SINK_SRV_EVENT_AVRCP_GET_FOLDER_ITEM_CNF, &param, sizeof(bt_sink_srv_avrcp_get_folder_items_cnf_t));
    }

    return BT_STATUS_SUCCESS;
}
#endif

bt_status_t bt_sink_srv_avrcp_common_callback(bt_msg_type_t msg, bt_status_t status, void *buffer)
{
    int32_t ret = 0;
#ifdef AIR_FEATURE_SINK_AVRCP_BQB
    if (bt_sink_srv_avrcp_bqb_in_progress()) {
        return bt_sink_srv_avrcp_bqb_common_cb(msg, status, buffer);
    }
#endif

    switch (msg) {
        case BT_AVRCP_CONNECT_CNF: {
            bt_avrcp_connect_cnf_t *conn_cnf = (bt_avrcp_connect_cnf_t *)buffer;
            ret = bt_sink_srv_avrcp_handle_connect_cnf(conn_cnf, status);
            break;
        }

        case BT_AVRCP_CONNECT_IND: {
            bt_avrcp_connect_ind_t *conn_ind = (bt_avrcp_connect_ind_t *)buffer;
            ret = bt_sink_srv_avrcp_handle_connect_ind(conn_ind, status);
            break;
        }

        case BT_AVRCP_DISCONNECT_IND: {
            bt_avrcp_disconnect_ind_t *disconn_ind = (bt_avrcp_disconnect_ind_t *)buffer;
            ret = bt_sink_srv_avrcp_handle_disconnect_ind(disconn_ind, status);
            break;
        }

        case BT_AVRCP_PASS_THROUGH_CNF: {
            bt_avrcp_pass_through_cnf_t *through_cnf = (bt_avrcp_pass_through_cnf_t *)buffer;
            ret = bt_sink_srv_avrcp_handle_pass_through_cnf(through_cnf, status);
            break;
        }
        case BT_AVRCP_PASS_THROUGH_COMMAND_IND: {
            bt_avrcp_pass_through_command_ind_t *through_ind = (bt_avrcp_pass_through_command_ind_t *)buffer;
            bt_sink_srv_avrcp_handle_pass_through_ind(through_ind, status);
            break;
        }
        case BT_AVRCP_EVENT_NOTIFICATION_IND: {
            bt_avrcp_event_notification_t *noti_ind = (bt_avrcp_event_notification_t *)buffer;
            ret = bt_sink_srv_avrcp_handle_event_notification_ind(noti_ind, status);
            break;
        }

        case BT_AVRCP_REGISTER_NOTIFICATION_IND: {
            bt_avrcp_register_notification_commant_t *register_noti_ind = (bt_avrcp_register_notification_commant_t *)buffer;
            bt_sink_srv_avrcp_handle_register_notification_ind(register_noti_ind, status);
            break;
        }

        case BT_AVRCP_SET_ABSOLUTE_VOLUME_COMMAND_IND: {
            bt_avrcp_set_absolute_volume_event_t *set_vol_ind = (bt_avrcp_set_absolute_volume_event_t *)buffer;
            bt_sink_srv_avrcp_handle_set_absolute_volume_ind(set_vol_ind, status);
            break;
        }

        case BT_AVRCP_GET_CAPABILITY_IND: {
            uint32_t conn_handle = (uint32_t)buffer;
            bt_avrcp_capability_attributes_response_t rsp = {0};

            bt_avrcp_event_t capability_attr_list[1] = {
                BT_AVRCP_EVENT_VOLUME_CHANGED
            };

            rsp.handle = conn_handle;
            rsp.length = 3;
            rsp.number = 1;
            rsp.attribute_list = capability_attr_list;
            bt_avrcp_send_capability_response(conn_handle, &rsp, BT_AVRCP_CAPABILITY_EVENTS_SUPPORTED);
            break;
        }
#ifdef MTK_BT_AVRCP_BROWSE_ENABLE
        case BT_AVRCP_BROWSE_CONNECT_IND: {
            bt_avrcp_browse_connect_ind_t *conn_ind = (bt_avrcp_browse_connect_ind_t *)buffer;
            if (conn_ind) {
                bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_AVRCP_HD, (void *)(&(conn_ind->handle)));
                bt_utils_assert(dev && "browse cnf null dev");

                ret = bt_avrcp_browse_connect_response(conn_ind->handle, true);
                dev->avrcp_browse_hd = conn_ind->handle;
                bt_sink_srv_report_id("[sink][music][avrcp] BT_AVRCP_BROWSE_CONNECT_IND ret: %x, hd :%x, dev:%x", 3, ret, conn_ind->handle, dev);
            }
            break;
        }
        case BT_AVRCP_BROWSE_CONNECT_CNF: {
            bt_avrcp_browse_connect_cnf_t *conn_cnf = (bt_avrcp_browse_connect_cnf_t *)buffer;
            ret = bt_sink_srv_avrcp_handle_browse_connect_cnf(conn_cnf, status);
            break;
        }

        case BT_AVRCP_BROWSE_GET_FOLDER_ITEMS_CNF: {
            bt_avrcp_browse_get_folder_items_cnf_t *folder_item = (bt_avrcp_browse_get_folder_items_cnf_t *)buffer;
            bt_sink_srv_avrcp_handle_get_folder_item_cnf(folder_item, status);
            break;
        }
        case BT_AVRCP_BROWSE_DISCONNECT_IND: {
            bt_avrcp_browse_disconnect_ind_t *disconn_ind = (bt_avrcp_browse_disconnect_ind_t *)buffer;
            bt_sink_srv_avrcp_handle_browse_disconnect_ind(disconn_ind, status);
            break;
        }
#endif

        case BT_AVRCP_GET_PLAY_STATUS_CNF: {
            bt_avrcp_media_play_status_response_t *cnf = (bt_avrcp_media_play_status_response_t *)buffer;
            ret = bt_sink_srv_avrcp_handle_get_play_status_cnf(cnf, status);
            break;
        }

        case BT_AVRCP_GET_CAPABILITY_CNF: {
            bt_avrcp_get_capability_response_t *cnf = (bt_avrcp_get_capability_response_t *)buffer;
            ret = bt_sink_srv_avrcp_handle_get_capability_cnf(cnf, status);
            break;
        }

        case BT_AVRCP_GET_ELEMENT_ATTRIBUTES_CNF: {
            bt_avrcp_get_element_attributes_response_t *cnf = (bt_avrcp_get_element_attributes_response_t *)buffer;
            ret = bt_sink_srv_avrcp_handle_get_element_attributes_cnf(cnf, status);
            break;
        }
        default:
            break;
    }

    bt_sink_srv_report_id("[sink][music][avrcp] common_hdr-msg: 0x%x, status: %d, buffer:0x%x, ret:0x%x", 4, msg, status, buffer, ret);
    return ret;
}

void bt_sink_srv_avrcp_play_pause_command_timeout_handler(uint32_t timer_id, uint32_t data)
{
    bt_sink_srv_music_device_t *dev = (bt_sink_srv_music_device_t *)data;
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
    bt_sink_srv_report_id("[sink][music][avrcp] play_pause_command_timeout, last_play_pause_action:0x%x, avrcp_status:0x%x, avrcp_cmd_timer_id:0x%x",
                          3, dev->last_play_pause_action, dev->avrcp_status, dev->avrcp_cmd_timer_id);
    
    if (ctx->run_dev
        && ctx->run_dev != dev
        && (BT_AVRCP_OPERATION_ID_PAUSE == dev->last_play_pause_action)
        && (dev->avrcp_status == BT_AVRCP_STATUS_PLAY_PLAYING || dev->a2dp_status == BT_SINK_SRV_A2DP_STATUS_STREAMING)) {
        dev->last_play_pause_action = BT_SINK_SRV_INVALID_LAST_PLAY_PAUSE_ACTION;
        dev->avrcp_status = BT_SINK_SRV_MUSIC_AVRCP_INVALID_STATUS;
        dev->avrcp_cmd_timer_id = 0;
        bt_sink_srv_avrcp_force_pause_music(dev);
    } else {
        dev->last_wear_action = BT_SINK_SRV_INVALID_LAST_PLAY_PAUSE_ACTION;
        dev->avrcp_status = BT_SINK_SRV_MUSIC_AVRCP_INVALID_STATUS;
        dev->avrcp_cmd_timer_id = 0;
        bt_sink_srv_avrcp_handle_last_wear_action(dev);
    }
}

static uint32_t bt_sink_srv_avrcp_find_free_cmd_timer_id(uint32_t start, uint32_t end)
{
    uint32_t i = start;
    uint32_t timer_ret = 0;
    for (i = start; i <= end; i++) {
        bt_timer_ext_t *avrcp_timer = bt_timer_ext_find(i);
        if (!avrcp_timer) {
            timer_ret = i;
            break;
        }
    }
    bt_sink_srv_report_id("[sink][music][avrcp] find_free_cmd_timer_id, timer_id:0x%08x", 1, timer_ret);
    return timer_ret;
}
#ifdef __BT_AWS_MCE_A2DP_SUPPORT__
bool bt_sink_srv_avrcp_is_push_release_command_timer_active(void)
{
    bt_sink_srv_music_device_t *device_list = bt_sink_srv_music_get_context()->sink_dev;
    bt_sink_srv_music_device_t *sp_dev = NULL;
    uint32_t dev_index = 0;
    bool ret = false;
    for(dev_index = 0; dev_index < BT_SINK_SRV_MUISC_DEV_COUNT; dev_index++) {
        sp_dev = &(device_list[dev_index]);
        if(sp_dev->conn_bit & BT_SINK_SRV_MUSIC_AVRCP_CONN_BIT) {
            if (sp_dev->avrcp_push_release_timer_id) {
                ret = true;
                break;
            }
        }
    }
    bt_sink_srv_report_id("[sink][music][avrcp] is_push_release_command_timer_active, ret:%d, sp_dev:0x%x", 2,
        ret, sp_dev);
    return ret;
}
#endif

void bt_sink_srv_avrcp_push_release_command_timeout_handler(uint32_t timer_id, uint32_t data)
{
    bt_sink_srv_music_device_t *dev = (bt_sink_srv_music_device_t *)data;
    bt_status_t ret = BT_STATUS_FAIL;
    (void)ret;

    ret = bt_avrcp_send_pass_through_command(dev->avrcp_hd, dev->avrcp_cmd_action_id, BT_AVRCP_OPERATION_STATE_RELEASED);
    dev->avrcp_push_release_timer_id = 0;
    bt_sink_srv_report_id("[sink][music][avrcp] pass through button released, ret:0x%08x, action_id:0x%02x, dev:0x%08x, timer_id:0x%08x", 4,
        ret, dev->avrcp_cmd_action_id, dev, dev->avrcp_push_release_timer_id);
}

bt_status_t bt_sink_srv_avrcp_send_play_pause_command(bt_sink_srv_music_device_t *dev, bt_avrcp_operation_id_t action_id)
{
    bt_status_t ret = BT_STATUS_FAIL;
    if (dev && (dev->conn_bit & BT_SINK_SRV_MUSIC_AVRCP_CONN_BIT) && action_id != BT_SINK_SRV_INVALID_LAST_PLAY_PAUSE_ACTION) {
        ret = bt_avrcp_send_pass_through_command(dev->avrcp_hd, action_id, BT_AVRCP_OPERATION_STATE_PUSH);
        if (ret == BT_STATUS_SUCCESS) {
            uint32_t timer_id = bt_sink_srv_avrcp_find_free_cmd_timer_id(BT_SINK_SRV_AVRCP_PUSH_RELEASE_ACTION_TIMER_ID_START,
                                                                         BT_SINK_SRV_AVRCP_PUSH_RELEASE_ACTION_TIMER_ID_END);
            dev->avrcp_push_release_timer_id = timer_id;
            dev->avrcp_cmd_action_id = action_id;
            bt_timer_ext_start(timer_id, (uint32_t)dev, BT_SINK_SRV_AVRCP_PUSH_RELEASE_TIMER_DUR, bt_sink_srv_avrcp_push_release_command_timeout_handler);
            
            if (dev->avrcp_cmd_timer_id) {
                bt_timer_ext_stop(dev->avrcp_cmd_timer_id);
                dev->avrcp_cmd_timer_id = 0;
            }
            if ((BT_AVRCP_OPERATION_ID_PAUSE == dev->avrcp_cmd_action_id)
                && (BT_AVRCP_STATUS_PLAY_PLAYING != dev->avrcp_status)) {
                // Do not retry pause when avrcp status is not playing
            } else {
                uint32_t timer_id = bt_sink_srv_avrcp_find_free_cmd_timer_id(BT_SINK_SRV_AVRCP_PLAY_PAUSE_ACTION_TIMER_ID_START,
                                                                             BT_SINK_SRV_AVRCP_PLAY_PAUSE_ACTION_TIMER_ID_END);
                dev->avrcp_cmd_timer_id = timer_id;
                bt_timer_ext_start(timer_id, (uint32_t)dev,
                                   BT_SINK_SRV_AVRCP_PLAY_PAUSE_ACTION_TIMER_DUR, bt_sink_srv_avrcp_play_pause_command_timeout_handler);
           }
        }
    }
    bt_sink_srv_report_id("[sink][music][avrcp] send_play_pause_cmd, ret:0x%08x, action_id:0x%02x, dev:0x%08x", 3, ret, action_id, dev);
    return ret;
}

/*bt_sink_srv_avrcp_send_stop_command is only for BQB test*/
void bt_sink_srv_avrcp_send_stop_command(void *param)
{
    bt_sink_srv_mutex_lock();
    bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_context()->run_dev;
    if (!dev) {
        dev = bt_sink_srv_avrcp_get_device(param, BT_SINK_SRV_ACTION_NONE);
    }
    bt_sink_srv_avrcp_send_play_pause_command(dev, BT_AVRCP_OPERATION_ID_STOP);
    bt_sink_srv_mutex_unlock();
}

/*bt_sink_srv_avrcp_send_vup_vdn_command is only for BQB test*/
void bt_sink_srv_avrcp_send_vup_vdn_command(uint8_t type)
{
    bt_sink_srv_mutex_lock();
    //void *param = NULL;
    bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_context()->run_dev;
    if(!dev){
        dev = bt_sink_srv_avrcp_get_device(NULL, BT_SINK_SRV_ACTION_NONE);
    }
    if (type == VOLUME_UP) {
            bt_avrcp_send_pass_through_command(dev->avrcp_hd, BT_AVRCP_OPERATION_ID_VOLUME_UP, BT_AVRCP_OPERATION_STATE_PUSH);
        } else {
            bt_avrcp_send_pass_through_command(dev->avrcp_hd, BT_AVRCP_OPERATION_ID_VOLUME_DOWN, BT_AVRCP_OPERATION_STATE_PUSH);
        }
    bt_sink_srv_mutex_unlock();
}

bt_status_t bt_sink_srv_avrcp_force_pause_music(bt_sink_srv_music_device_t *dev)
{
    bt_status_t ret = bt_sink_srv_avrcp_send_play_pause_command(dev, BT_AVRCP_OPERATION_ID_PAUSE);

    if (ret == BT_STATUS_SUCCESS) {
        dev->last_play_pause_action = BT_AVRCP_OPERATION_ID_PAUSE;
        dev->last_wear_action = BT_SINK_SRV_INVALID_LAST_PLAY_PAUSE_ACTION;
    }

    bt_sink_srv_report_id("[sink][music][avrcp] force_pause_music, ret:0x%08x", 1, ret);
    return ret;
}

static void bt_sink_srv_avrcp_handle_last_wear_action(bt_sink_srv_music_device_t *dev)
{
    bt_status_t ret = BT_STATUS_FAIL;

    bt_utils_assert(dev && "last_wear_action, dev NULL");
    bt_avrcp_operation_id_t action_id = bt_sink_srv_music_get_play_pause_action(dev);

    if (dev->last_wear_action != BT_SINK_SRV_INVALID_LAST_PLAY_PAUSE_ACTION
        && dev->last_wear_action == action_id) {
        ret = bt_sink_srv_avrcp_send_play_pause_command(dev, action_id);
        if (BT_STATUS_SUCCESS == ret) {
            dev->last_play_pause_action = action_id;
            dev->last_wear_action = BT_SINK_SRV_INVALID_LAST_PLAY_PAUSE_ACTION;
        }
    } else if (dev->last_play_pause_action == BT_SINK_SRV_INVALID_LAST_PLAY_PAUSE_ACTION) {
        dev->last_wear_action = BT_SINK_SRV_INVALID_LAST_PLAY_PAUSE_ACTION;
    }
    bt_sink_srv_report_id("[sink][music][avrcp] handle_last_wear_action, action_id:0x%02x, dev:0x%08x, last_send_actio:0x%02x, last_wear_action:0x%02x",
                          4, action_id, dev,
                          dev->last_play_pause_action,
                          dev->last_wear_action);
}

int32_t bt_sink_srv_avrcp_play_music(bt_sink_srv_music_device_t *dev)
{
    if ((dev) && (dev->conn_bit & BT_SINK_SRV_MUSIC_AVRCP_CONN_BIT)) {
        dev->last_wear_action = BT_AVRCP_OPERATION_ID_PLAY;
        bt_sink_srv_avrcp_handle_last_wear_action(dev);
    }

    bt_sink_srv_report_id("[sink][music][avrcp] play_music-dev: 0x%08x", 1, dev);
    return BT_STATUS_SUCCESS;
}


int32_t bt_sink_srv_avrcp_stop_music(bt_sink_srv_music_device_t *dev)
{
    bt_utils_assert(dev && "stop_music, dev NULL");

    uint32_t handle = dev->avrcp_hd;
    if ((dev->conn_bit & BT_SINK_SRV_MUSIC_AVRCP_CONN_BIT) && handle) {
        dev->last_wear_action = BT_AVRCP_OPERATION_ID_PAUSE;
        bt_sink_srv_avrcp_handle_last_wear_action(dev);
    } else {
        BT_SINK_SRV_SET_FLAG(dev->flag, BT_SINK_SRV_MUSIC_STOP_MUSIC_PENDING);
    }

    bt_sink_srv_report_id("[sink][music][avrcp] stop_music-dev:0x%x, conn_bit:0x%x, avrcp_hd: 0x%08x, avrcp_last_play_pause_action:0x%02x", 4,
        dev, dev->conn_bit, handle, dev->last_play_pause_action);
    return BT_STATUS_SUCCESS;
}

int32_t bt_sink_srv_avrcp_change_track(uint32_t handle, bool is_ptrack)
{
    int32_t ret = BT_STATUS_SUCCESS;
    bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_AVRCP_HD, (void *)(&handle));

    if ((dev) && (dev->conn_bit & BT_SINK_SRV_MUSIC_AVRCP_CONN_BIT)) {
        ret = bt_avrcp_send_pass_through_command(handle, is_ptrack?BT_AVRCP_OPERATION_ID_BACKWARD : BT_AVRCP_OPERATION_ID_FORWARD,
            BT_AVRCP_OPERATION_STATE_PUSH);
    }

    bt_sink_srv_report_id("[sink][music][avrcp] change_track-is_ptrack:%d, hd:0x%x, dev:%d, ret:0x%x", 4, is_ptrack, handle, dev, ret);
    return ret;
}

//[TD-PLAYER]
void bt_sink_srv_init_role(bt_avrcp_role_t role)
{
    bt_avrcp_init_t init;

    bt_sink_srv_report_id("[sink][music][avrcp] init_role:0x%x", 1, role);
    init.role = role;
    init.support_browse = 0;
    bt_avrcp_init(&init);
}

static void bt_sink_srv_avrcp_as_tg_callback(uint32_t timer_id, uint32_t data)
{
    uint32_t hd = BT_SINK_SRV_MUSIC_INVALID_HD;//[TD-PLAYER]
    bt_sink_srv_music_device_t *dev = (bt_sink_srv_music_device_t *)data;

    bt_sink_srv_init_role(BT_AVRCP_ROLE_TG);
    bt_status_t ret = bt_avrcp_connect(&hd, (const bt_bd_addr_t *)(&(dev->dev_addr)));
    if (ret == BT_STATUS_SUCCESS) {
        dev->avrcp_hd = hd;
        dev->avrcp_role = BT_AVRCP_ROLE_TG;
    }
    bt_sink_srv_report_id("[sink][music][avrcp] avrcp_as_tg_callback, change role to connect again-ret:%x, hd:0x%x, dev:0x%x", 3, ret, hd, dev);
}

static void bt_sink_srv_avrcp_handle_play_status_notification(uint32_t handle, uint8_t avrcp_status, bt_status_t status)
{
    bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_AVRCP_HD, (void *)(&handle));

    bt_utils_assert(dev);
    if (status != BT_STATUS_AVRCP_INTERIM) {
        return;
    }
    uint32_t old_avrcp_status = dev->avrcp_status;
    dev->avrcp_status = avrcp_status;

    dev->last_play_pause_action = BT_SINK_SRV_INVALID_LAST_PLAY_PAUSE_ACTION;
    bt_timer_ext_stop(dev->avrcp_cmd_timer_id);
    dev->avrcp_cmd_timer_id = 0;
    if (dev->last_wear_action) {
        bt_sink_srv_avrcp_handle_last_wear_action(dev);
    }
#if (BT_A2DP_TOTAL_LINK_NUM>3)
    if (BT_AVRCP_STATUS_PLAY_PLAYING == avrcp_status) {
        dev->avrcp_play_count = bt_sink_srv_state_manager_get_play_count();
    }
#endif
    /* Notify change to appliation */
    bt_sink_srv_music_avrcp_status_change_notify(&dev->dev_addr, dev->avrcp_status);

    bt_sink_srv_music_state_change_handler(dev, dev->a2dp_status, old_avrcp_status);
}

#ifdef MTK_BT_AVRCP_BROWSE_ENABLE
static bt_status_t bt_sink_srv_avrcp_get_folder_item(bt_sink_srv_music_device_t *dev, bt_sink_srv_avrcp_get_folder_items_parameter_t *parameter)
{
    bt_utils_assert(dev && parameter && "NULL dev or parameter");
    bt_status_t ret = BT_STATUS_FAIL;

    if (!(dev->conn_bit & BT_SINK_SRV_MUSIC_AVRCP_BROWSING_CONN_BIT)) {
        bt_avrcp_init_t init;
        init.role = BT_AVRCP_ROLE_CT;
        init.support_browse = 1;
        bt_avrcp_init(&init);
        ret = bt_avrcp_browse_connect(&dev->avrcp_browse_hd, (const bt_bd_addr_t *)(&dev->dev_addr));
        if (ret == BT_STATUS_SUCCESS) {
            bt_sink_srv_memcpy(&dev->folder_parameter, parameter, sizeof(bt_sink_srv_avrcp_get_folder_items_parameter_t));
            BT_SINK_SRV_SET_FLAG(dev->avrcp_flag, BT_SINK_SRV_AVRCP_GET_FOLDER_ITEM);
        }
    } else {
        bt_avrcp_browse_get_folder_items_t folder_item = {0};

        folder_item.scope = parameter->scope;
        folder_item.start_item = parameter->start_item;
        folder_item.end_item = parameter->end_item;
        folder_item.attribute_count = 0;
        folder_item.attribute_list = NULL;
        ret = bt_avrcp_browse_get_folder_items(dev->avrcp_browse_hd, &folder_item);
    }

    bt_sink_srv_report_id("[sink][music][avrcp] get_folder_item, ret:0x%08x, conn_bit:0x%08x, handle:0x%08x, avrcp_flag:0x%x",
        3, ret, dev->conn_bit, dev->avrcp_browse_hd, dev->avrcp_flag);
    return ret;
}
#endif

bt_status_t  bt_sink_srv_avrcp_cm_callback_handler(bt_cm_profile_service_handle_t type, void *data)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    uint8_t *address = NULL;
    switch (type) {
        case BT_CM_PROFILE_SERVICE_HANDLE_DISCONNECT: {
            bt_sink_srv_mutex_lock();
            address = (uint8_t *)data;
            bt_utils_assert(address);
            bt_sink_srv_report_id("[sink][music][avrcp] disconnect addr :0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", 6, address[0], address[1], address[2],
                                  address[3], address[4], address[5]);
            bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_ADDR_AVRCP, (void *)address);
            if ((dev) && (dev->conn_bit & BT_SINK_SRV_MUSIC_AVRCP_CONN_BIT)) {
                bt_timer_ext_stop(BT_SINK_SRV_AVRCP_CONNECTION_TIMER_ID);
                status = bt_avrcp_disconnect(dev->avrcp_hd);
            }
            bt_sink_srv_mutex_unlock();
            break;
        }
        default:
            break;
    }
    return status;
}

bool bt_sink_srv_avrcp_is_playing(uint32_t avrcp_status)
{
    return (BT_AVRCP_STATUS_PLAY_PAUSED != avrcp_status && BT_AVRCP_STATUS_PLAY_STOPPED != avrcp_status);
}

