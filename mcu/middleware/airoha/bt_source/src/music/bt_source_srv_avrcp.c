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


#include "bt_source_srv_avrcp.h"
#include "bt_source_srv_utils.h"
#include <stdlib.h>
#include "bt_iot_device_white_list.h"


#define BT_SOURCE_AVRCP_CAPABILITY_EVENT_SUPPORTED_LEN 7 // we support 7 total capability of the application
//log_create_module(avrcp_srv, PRINT_LEVEL_INFO);
static uint32_t g_local_volume = 0;
static bt_source_srv_volume_step_t bt_source_srv_avrcp_write_volume(bt_bd_addr_t *remote_addr, void *data, uint8_t *vol_step);

static bt_status_t bt_source_srv_avrcp_read_volume(bt_bd_addr_t *bd_addr, bt_source_srv_music_stored_data_t *stored_data);
static void bt_source_srv_avrcp_notify_volume_ind_to_upper_layer(bt_source_srv_music_device_t *dev, uint8_t volume);
static uint32_t bt_source_srv_avrcp_get_map_volume(uint32_t volume);
static bool bt_source_srv_avrcp_set_nvdm_data(bt_bd_addr_t *bt_addr, bt_source_srv_music_stored_data_t *data_p);
static uint32_t bt_source_srv_avrcp_get_volume_step(uint8_t avrcp_volume);

static bt_srv_music_operation_t bt_source_avrcp_transfer_operation(bt_avrcp_operation_id_t op_id)
{
    bt_srv_music_operation_t operation = 0;
    switch(op_id)
    {
        case BT_AVRCP_OPERATION_ID_VOLUME_UP: {
            operation = BT_SOURCE_SRV_ACTION_VOLUME_UP;

            break;
        }
        case BT_AVRCP_OPERATION_ID_VOLUME_DOWN: {
            operation = BT_SOURCE_SRV_ACTION_VOLUME_DOWN;
            break;
        }

        case BT_AVRCP_OPERATION_ID_PLAY: {
            operation = BT_SOURCE_SRV_ACTION_PLAY;
            break;
        }
        case BT_AVRCP_OPERATION_ID_STOP: {
            operation = BT_SOURCE_SRV_ACTION_PAUSE;
            break;
        }
        case BT_AVRCP_OPERATION_ID_PAUSE: {
            operation = BT_SOURCE_SRV_ACTION_PAUSE;
            break;
        }
        case BT_AVRCP_OPERATION_ID_FORWARD: {
            operation = BT_SOURCE_SRV_ACTION_NEXT;
            break;
        }
        case BT_AVRCP_OPERATION_ID_BACKWARD: {
            operation = BT_SOURCE_SRV_ACTION_PREVIOUS;
            break;
        }
        default: {
            break;
        }

    }
    LOG_MSGID_I(source_srv, "[AVRCP_SRV]avrcp_transfer_operation: operaion:%x", 1, operation);


    return operation;
}


static bt_status_t bt_source_srv_avrcp_pass_through_command_handler(bt_avrcp_pass_through_command_ind_t *ind)
{
    bt_status_t ret = BT_STATUS_FAIL;

    bt_source_srv_music_device_t *dev = bt_source_srv_music_get_device(BT_SRV_MUSIC_DEVICE_AVRCP_HD, (void *)&(ind->handle));
    LOG_MSGID_I(source_srv, "[AVRCP_SRV]pass_through_command_handler: dev = 0x%x, handle = %x", 2, dev, ind->handle);

    if (dev == NULL) {
        return ret;
    }
    if (ind->op_state == BT_AVRCP_OPERATION_STATE_PUSH) {
        bt_source_srv_music_control_operation_ind_t avrcp_ind;
        bt_utils_memcpy(&(avrcp_ind.peer_address.addr), &(dev->dev_addr), sizeof(bt_bd_addr_t));

        avrcp_ind.operation = bt_source_avrcp_transfer_operation(ind->op_id);
        avrcp_ind.data = NULL;
        avrcp_ind.length = 0;
        bt_source_srv_event_callback(BT_SOURCE_SRV_EVENT_MUSIC_CONTROL_OPERATION_IND, &avrcp_ind, sizeof(bt_source_srv_music_control_operation_ind_t));
    }
    ret = bt_avrcp_send_pass_through_response(dev->avrcp_hd,BT_AVRCP_RESPONSE_ACCEPTED, ind->op_id, ind->op_state);

    LOG_MSGID_I(source_srv,"[AVRCP_SRV]pass_through_command_handler: ret = 0x%x, op_id = %x, op_state:%x", 3, ret, ind->op_id, ind->op_state);
    return ret;
}


static bt_status_t bt_source_srv_avrcp_connct_cnf_handler(bt_avrcp_connect_cnf_t *cnf, bt_status_t status)
{
    bt_source_srv_music_device_t *dev = bt_source_srv_music_get_device(BT_SRV_MUSIC_DEVICE_AVRCP_HD, (void*)&(cnf->handle));

    LOG_MSGID_I(source_srv,"[AVRCP_SRV]avrcp_connct_cnf_handler: dev = 0x%x, handle = %x", 2, dev,cnf->handle);

    bt_status_t ret = BT_STATUS_FAIL;
    if (dev) {
        bt_audio_srv_context_t *cntx = bt_source_srv_music_get_context();
        LOG_MSGID_I(source_srv,"[AVRCP_SRV]avrcp_connct_cnf_handler: a2dp_state = 0x%x, bqb_flag = %x", 2, dev->a2dp_state,cntx->bqb_flag);
        if (dev->a2dp_state < BT_SOURCE_SRV_STATE_READY && !cntx->bqb_flag) {
            bt_source_srv_avrcp_disconnect(&dev->dev_addr);
            return ret;
        } else if (dev->a2dp_state == BT_SOURCE_SRV_STATE_STREAMING) {
            dev->avrcp_play_status = BT_AVRCP_EVENT_MEDIA_PLAYING;
        }
        if (status == BT_STATUS_SUCCESS) {
            dev->avrcp_state = BT_SOURCE_SRV_STATE_READY;
#ifdef BT_SOURCE_PC_VOLUME_PRO_HIGH
        ret = bt_avrcp_set_absolute_volume(cnf->handle, g_local_volume);
        dev->absolute_support = true;
        dev->bt_volume = g_local_volume;
        dev->flag = dev->flag | BT_SOURCE_SRV_A2DP_REGISTER_FLAG;//busy need retry
        LOG_MSGID_I(source_srv,"[AVRCP_SRV]avrcp_connct_cnf_handler:ret = 0x%x, local_values:%x, nv_vol:0x%x", 3, ret, g_local_volume,dev->bt_volume);
#else

            ret = bt_avrcp_register_notification(cnf->handle, BT_AVRCP_EVENT_VOLUME_CHANGED, 0);
#endif
        } else {
            dev->avrcp_state = BT_SOURCE_SRV_STATE_IDLE;//May be need
        }

        bt_source_srv_music_stored_data_t dev_db = {0};

#ifdef BT_SOURCE_PC_VOLUME_PRO_HIGH
        dev_db.music_volume = g_local_volume | BT_SOURCE_SRV_MUSIC_VOLUME_SET_FLAG;
        bt_source_srv_avrcp_set_nvdm_data(&dev->dev_addr, &dev_db);
#else
        ret = bt_source_srv_avrcp_read_volume(&dev->dev_addr, &dev_db);
        uint32_t vol_flag = (dev_db.music_volume & BT_SOURCE_SRV_MUSIC_VOLUME_SET_FLAG);
        LOG_MSGID_I(source_srv,"[AVRCP_SRV]avrcp_connct_cnf_handler:vol_flag: %x", 1, vol_flag);
        if (vol_flag != BT_SOURCE_SRV_MUSIC_VOLUME_SET_FLAG) {
            dev_db.music_volume = g_local_volume | BT_SOURCE_SRV_MUSIC_VOLUME_SET_FLAG;
            bt_source_srv_avrcp_set_nvdm_data(&dev->dev_addr, &dev_db);
        }
        dev->bt_volume = dev_db.music_volume & 0x7F;
#endif
    LOG_MSGID_I(source_srv,"[AVRCP_SRV]avrcp_connct_cnf_handler:ret = 0x%x, local_values:%x, nv_vol:0x%x", 3, ret, g_local_volume,dev_db.music_volume);

#ifdef MTK_BT_CM_SUPPORT
        bt_cm_profile_service_status_notify(BT_CM_PROFILE_SERVICE_AVRCP, dev->dev_addr, BT_CM_PROFILE_SERVICE_STATE_CONNECTED, status);
#endif

        LOG_MSGID_I(source_srv,"[AVRCP_SRV]avrcp_connct_cnf_handler: ret = 0x%x, handle = %x, cnf_handle: %x", 3, ret, dev->avrcp_hd, cnf->handle);
    }
    return ret;
}


static bt_status_t bt_source_srv_avrcp_connect_ind_handler(bt_avrcp_connect_ind_t *ind)
{
    bt_status_t ret = BT_STATUS_FAIL;
    bt_source_srv_music_device_t *dev = bt_source_srv_music_get_device(BT_SRV_MUSIC_DEVICE_AVRCP_HD, (void*)(&(ind->handle)));

    LOG_MSGID_I(source_srv,"[AVRCP_SRV]avrcp_connct_ind_handler: dev = 0x%x, handle = %x", 2, dev, ind->handle);

    if (dev) { /*dev has connected before*/
        if (BT_SOURCE_SRV_STATE_READY == dev->avrcp_state ||BT_SOURCE_SRV_STATE_CONNECTING == dev->avrcp_state) {
            ret = bt_avrcp_connect_response(ind->handle, false);
        } else {
            /*there may be some error*/
        }

    } else {
        dev = bt_source_srv_music_get_device(BT_SRV_MUSIC_DEVICE_ADDR_A2DP, (void*)(ind->address));
        if (dev) {
            if ((dev->a2dp_hd != BT_SOURCE_SRV_INVAILD_HANDLE) && (dev->avrcp_hd == BT_SOURCE_SRV_INVAILD_HANDLE)) {
                ret = bt_avrcp_connect_response(ind->handle, true);
            }
        } else {
            dev = bt_source_srv_music_get_device(BT_SRV_MUSIC_DEVICE_UNUSED, ind->address);

            LOG_MSGID_I(source_srv,"[AVRCP_SRV]avrcp_connct_ind_handler: unused_dev = 0x%x", 1, dev);

            if (dev) {/*a2dp has connected*/
                ret = bt_avrcp_connect_response(ind->handle, true);
            } else {
                LOG_MSGID_I(source_srv,"[AVRCP_SRV]avrcp_connct_ind_handler: Device max = 0x%x", 0);
                ret = bt_avrcp_connect_response(ind->handle, false);
            }
        }

        if (BT_STATUS_SUCCESS == ret) {
            dev->avrcp_hd = ind->handle;
            dev->avrcp_role = BT_AVRCP_ROLE_TG;
            dev->avrcp_state = BT_SOURCE_SRV_STATE_CONNECTING;
        }

    }

    LOG_MSGID_I(source_srv,"[AVRCP_SRV]avrcp_connct_ind_handler: ret = %x", 1, ret);

    return ret;
}


static void bt_source_srv_set_absolute_volume_ind_handler(bt_avrcp_set_absolute_volume_response_t * response)
{
    bt_source_srv_music_device_t *dev = bt_source_srv_music_get_device(BT_SRV_MUSIC_DEVICE_AVRCP_HD, (void *)(&(response->handle)));
    if (dev) {
        if (response) {
            LOG_MSGID_I(source_srv,"[AVRCP_SRV]set_absolute_volume_handler: dev = 0x%x, ind = %x", 2, dev, response->handle);
            bt_avrcp_send_set_absolute_volume_response(response->handle, response->volume);
            bt_source_srv_avrcp_notify_volume_ind_to_upper_layer(dev, response->volume);
        }
    }
}

static void bt_source_srv_set_absolute_volume_cnf_handler(bt_avrcp_set_absolute_volume_response_t * response)
{
    bt_source_srv_music_device_t *dev = bt_source_srv_music_get_device(BT_SRV_MUSIC_DEVICE_AVRCP_HD, (void *)(&(response->handle)));
    LOG_MSGID_I(source_srv,"[AVRCP_SRV]set_absolute_volume_cnf:dev = 0x%x,flag:0x%x,handle:0x%x", 3, dev, dev->flag, response->handle);

    if (dev && (dev->flag & BT_SOURCE_SRV_A2DP_REGISTER_FLAG)) {
        bt_avrcp_register_notification(response->handle, BT_AVRCP_EVENT_VOLUME_CHANGED, 0);
        dev->flag = dev->flag & 0xFE;
    }
}

static bt_status_t bt_source_srv_avrcp_register_notification_ind_handler(bt_avrcp_register_notification_commant_t *register_ind)
{
    int32_t ret = 0;
    bt_source_srv_music_device_t *dev = bt_source_srv_music_get_device(BT_SRV_MUSIC_DEVICE_AVRCP_HD, (void *)(&(register_ind->handle)));

    LOG_MSGID_I(source_srv,"[AVRCP_SRV]notification_ind_handler: dev = 0x%x, event:0x%x,ind:0x%x", 3, dev, register_ind->event_id, register_ind->handle);

    if (dev == NULL) {
        return ret;
    }
    bt_avrcp_send_register_notification_response_t rsp;
    bt_utils_memset(&rsp, 0, sizeof(bt_avrcp_send_register_notification_response_t));
    switch (register_ind->event_id) {
        case BT_AVRCP_EVENT_PLAYBACK_STATUS_CHANGED: {
            bool is_iot = bt_iot_device_white_list_check_iot_case(&(dev->dev_addr), BT_IOT_MUSIC_NO_PRE_NEXT_PASS_THROUGH_CMD);
            if (is_iot) {
                rsp.parameter_length = 1;
                rsp.response_type = BT_AVRCP_RESPONSE_REJECTED;
                rsp.event_id = register_ind->event_id;
            } else {
                rsp.parameter_length = 2;
                rsp.response_type = BT_AVRCP_RESPONSE_INTERIM;
                rsp.event_id = BT_AVRCP_EVENT_PLAYBACK_STATUS_CHANGED;
                rsp.status = dev->avrcp_play_status;
            }
            break;
        }
        case BT_AVRCP_EVENT_TRACK_CHANGED: {
            rsp.parameter_length = 9;
            rsp.response_type = BT_AVRCP_RESPONSE_INTERIM;
            rsp.event_id = BT_AVRCP_EVENT_TRACK_CHANGED;
            rsp.id = 0xFFFFFFFFFFFFFFFF;
            break;
        }
        case BT_AVRCP_EVENT_VOLUME_CHANGED: {
            rsp.parameter_length = 2;
            rsp.response_type = BT_AVRCP_RESPONSE_INTERIM;
            rsp.event_id = BT_AVRCP_EVENT_VOLUME_CHANGED;
            rsp.volume = dev->bt_volume;
            dev->absolute_support = false;
            break;
        }
        default: {
            rsp.parameter_length = 1;
            rsp.response_type = BT_AVRCP_RESPONSE_NOT_IMPLEMENTED;
            rsp.event_id = register_ind->event_id;
            break;
        }
    }

    ret = bt_avrcp_send_register_notification_response(register_ind->handle, &rsp);

    LOG_MSGID_I(source_srv,"[AVRCP_SRV]notification_ind_handler:response, ret = %x,state = %x,bt_vol:0x%x", 3,
    ret, dev->avrcp_play_status, dev->bt_volume);

    return ret;
}

static bool bt_source_srv_avrcp_set_nvdm_data(bt_bd_addr_t *bt_addr, bt_source_srv_music_stored_data_t *data_p)
{
    bool result = false;
#ifdef BT_SOURCE_SRV_TG_STORAGE_SIZE
    // Warnning: Due to the task stack limite, record should not increase more than 100 bytes
    bt_device_manager_db_remote_profile_info_t record;
    bt_utils_memset(&record, 0, sizeof(bt_device_manager_db_remote_profile_info_t));
    if (NULL != bt_addr && NULL != data_p) {
        bt_device_manager_remote_find_profile_info((*bt_addr), &record);
        bt_utils_memcpy(record.tg_info, data_p, sizeof(bt_source_srv_music_stored_data_t));
        result = bt_device_manager_remote_update_profile_info((*bt_addr), &record);
        bt_device_manager_db_flush_all(BT_DEVICE_MANAGER_DB_FLUSH_NON_BLOCK);
    }
#endif
    LOG_MSGID_I(source_srv,"[AVRCP_SRV][VOL]bt_source_srv_avrcp_set_nvdm_data:result:0x%x,", 1, result);

    return result;
}


static bt_status_t bt_source_srv_avrcp_read_volume(bt_bd_addr_t *bd_addr, bt_source_srv_music_stored_data_t *stored_data)
{
    bt_status_t status = BT_STATUS_FAIL;
#ifdef BT_SOURCE_SRV_TG_STORAGE_SIZE
    bt_device_manager_db_remote_profile_info_t record = {{0}};
    status = bt_device_manager_remote_find_profile_info((*bd_addr), &record);
    LOG_MSGID_I(source_srv,"[AVRCP_SRV][VOL] tg_info,0x%x", 1, record.tg_info[0]);
    if (status != BT_STATUS_SUCCESS) {
        return status;
    }
    bt_utils_memcpy(stored_data, record.tg_info, sizeof(bt_source_srv_music_stored_data_t));
#endif
    return status;
}


static bt_source_srv_volume_step_t bt_source_srv_avrcp_write_volume(bt_bd_addr_t *remote_addr, void *data, uint8_t *vol_step)
{
    bt_source_srv_volume_step_t type = BT_SOURCE_SRV_VOLUME_STEP_TYPE_UP;
    uint8_t local_vol = 0;
    uint8_t *new_vol = (uint8_t*)(data);
    bt_source_srv_music_stored_data_t dev_db;
    bt_source_srv_memset(&dev_db, 0, sizeof(bt_source_srv_music_stored_data_t));
    bt_status_t ret = bt_source_srv_avrcp_read_volume(remote_addr, &dev_db);
    if (ret == BT_STATUS_SUCCESS) {
        local_vol = (dev_db.music_volume & 0x7F);
        LOG_MSGID_I(source_srv,"[AVRCP_SRV][VOL]bt_source_srv_avrcp_write_volume:local_volume:0x%x, new_volume = 0x%x", 2, local_vol, (*new_vol));
        if ((*new_vol) > local_vol) {
            *vol_step = ((*new_vol)) - local_vol;
        } else if (local_vol > ((*new_vol))){
            *vol_step = local_vol - ((*new_vol));
            type = BT_SOURCE_SRV_VOLUME_STEP_TYPE_DOWN;
        } else {
            type = BT_SOURCE_SRV_MUSIC_VOLUME_CHANGE_INVAILD;
        }
        dev_db.music_volume = (((*new_vol)) | BT_SOURCE_SRV_MUSIC_VOLUME_SET_FLAG);
        LOG_MSGID_I(source_srv,"[AVRCP_SRV][VOL]bt_source_srv_avrcp_write_volume:update:0x%x",1, dev_db.music_volume);
        bt_source_srv_avrcp_set_nvdm_data(remote_addr, &dev_db);
    }
    LOG_MSGID_I(source_srv,"[AVRCP_SRV][VOL]bt_source_srv_avrcp_write_volume:vol_step:0x%x, vol_type:0x%x", 2, *vol_step, type);
    return type;
}


static void bt_source_srv_avrcp_notify_volume_ind_to_upper_layer(bt_source_srv_music_device_t *dev, uint8_t volume)
{
    bt_source_srv_common_audio_port_context_t port_ind = {0};
    bt_source_srv_volume_change_ind_t vol_ind = {0};
    bt_status_t ret = BT_STATUS_FAIL;
    uint8_t vol_step = 0;

    vol_ind.step_type = bt_source_srv_avrcp_write_volume(&(dev->dev_addr),&volume, &vol_step);
    bt_utils_memcpy(&vol_ind.peer_address.addr, &(dev->dev_addr), sizeof(bt_bd_addr_t));

    if (vol_ind.step_type != BT_SOURCE_SRV_MUSIC_VOLUME_CHANGE_INVAILD) {
        ret = bt_source_srv_common_audio_find_port_context(BT_SOURCE_SRV_PORT_CHAT_SPEAKER, &port_ind);
        if (ret == BT_STATUS_SUCCESS) {
            vol_ind.port = port_ind.port;
            vol_ind.volume_step = bt_source_srv_avrcp_get_volume_step(vol_step);
            vol_ind.mute_state = BT_SOURCE_SRV_MUTE_STATE_NONE;
            LOG_MSGID_I(source_srv,"[AVRCP_SRV][VOL]volume_ind_to_upper_layer:response, volume = 0x%x,volume_percent = 0x%x, type:0x%x", 3, volume, vol_ind.volume_step, vol_ind.step_type);
            bt_source_srv_event_callback(BT_SOURCE_SRV_EVENT_VOLUME_CHANGE, &vol_ind, sizeof(bt_source_srv_volume_change_ind_t));
        }
    }
}


static bt_status_t bt_source_srv_avrcp_notification_ind_handler(bt_avrcp_event_notification_t *noti_ind, bt_status_t status)
{
    bt_status_t ret = BT_STATUS_FAIL;
    bt_source_srv_music_device_t *dev = bt_source_srv_music_get_device(BT_SRV_MUSIC_DEVICE_AVRCP_HD, (void *)(&(noti_ind->handle)));
    LOG_MSGID_I(source_srv,"[AVRCP_SRV]avrcp_notification_ind_handler: status = 0x%x, event_id: %x, handle:%x", 3, status, noti_ind->event_id, noti_ind->handle);

    if (dev == NULL) {
        LOG_MSGID_E(source_srv,"[AVRCP_SRV]avrcp_notification_ind_handler: dev is not in ", 0);
        return ret;
    }
    switch (noti_ind->event_id) {
        case BT_AVRCP_EVENT_VOLUME_CHANGED: {
            LOG_MSGID_E(source_srv,"[AVRCP_SRV][VOL][NOTIFY]VOL changged ", 0);
            if (status == BT_STATUS_AVRCP_INTERIM) { /*register -> interim*/
                dev->absolute_support = true;
                bt_source_srv_avrcp_notify_volume_ind_to_upper_layer(dev, noti_ind->volume);
            } else if (status == BT_STATUS_SUCCESS) {
                bt_source_srv_avrcp_notify_volume_ind_to_upper_layer(dev, noti_ind->volume);
                ret = bt_avrcp_register_notification(noti_ind->handle, BT_AVRCP_EVENT_VOLUME_CHANGED, 0);
                if(ret == BT_STATUS_AVRCP_BUSY) {
                    dev->flag = dev->flag | BT_SOURCE_SRV_A2DP_REGISTER_FLAG;//busy need retry
                    LOG_MSGID_E(source_srv,"[AVRCP_SRV][VOL][NOTIFY]register volume changged fail, flag:0x%x ", 1, dev->flag);
                }
            } else {
                /*error handle*/
                dev->absolute_support = true;
            }
            break;
        }
    }

    return ret;
}


static void bt_source_srv_avrcp_disconn_ind_handler(bt_avrcp_disconnect_ind_t *ind, bt_status_t status)
{
    bt_source_srv_music_device_t *dev = bt_source_srv_music_get_device(BT_SRV_MUSIC_DEVICE_AVRCP_HD, (void*)(&ind->handle));
    if (dev) {
#ifdef MTK_BT_CM_SUPPORT
       bt_cm_profile_service_status_notify(BT_CM_PROFILE_SERVICE_AVRCP, dev->dev_addr, BT_CM_PROFILE_SERVICE_STATE_DISCONNECTED, BT_STATUS_SUCCESS);
#endif
        bt_source_srv_music_clean_avrcp_conn_info(dev);
    }
}


static bt_status_t bt_source_srv_avrcp_get_capability_ind_handler(uint32_t handle)
{
    bt_status_t ret;
    bt_avrcp_capability_attributes_response_t rsp = {0};
    bt_source_srv_music_device_t *dev = bt_source_srv_music_get_device(BT_SRV_MUSIC_DEVICE_AVRCP_HD, (void *)(&handle));

    bt_avrcp_event_t capability_attr_list[2] = {
        BT_AVRCP_EVENT_VOLUME_CHANGED,
        BT_AVRCP_EVENT_PLAYBACK_STATUS_CHANGED
    };

    rsp.handle = handle;
    rsp.length = 4;
    rsp.number = 2;
    if (dev) {
        bool is_iot = bt_iot_device_white_list_check_iot_case(&(dev->dev_addr), BT_IOT_MUSIC_NO_PRE_NEXT_PASS_THROUGH_CMD);
        if (is_iot) {
            rsp.length = 3;
            rsp.number = 1;
        }
    }

    rsp.attribute_list = capability_attr_list;
    ret = bt_avrcp_send_capability_response(handle, &rsp, BT_AVRCP_CAPABILITY_EVENTS_SUPPORTED);
    return ret;
}


static bt_status_t bt_source_avrcp_get_play_status_notification_ind_hanlder(bt_avrcp_get_play_status_commant_t *noti_ind)
{
    bt_status_t ret;
    bt_avrcp_media_play_status_notification_t rsp = {0};
    bt_source_srv_music_device_t *dev = bt_source_srv_music_get_device(BT_SRV_MUSIC_DEVICE_AVRCP_HD, (void *)(&(noti_ind->handle)));
    LOG_MSGID_I(source_srv,"[AVRCP_SRV]status_notification_ind_hanlder: dev = 0x%x", 2, dev, noti_ind->handle);
    if (dev) {

        LOG_MSGID_I(source_srv,"[AVRCP_SRV]status_notification_ind_hanlder: status: 0x%x", 1, dev->avrcp_play_status);

        rsp.status = dev->avrcp_play_status; //current media state
        if (BT_AVRCP_EVENT_MEDIA_PLAY_STOPPED != rsp.status) {
            rsp.song_length = 2;
            rsp.song_position = 1;//need extend
        }
    }
    ret = bt_avrcp_media_send_play_status_response(noti_ind->handle, &rsp);
    return ret;
}


static void bt_source_avrcp_get_element_attribute_notification_ind_handler(bt_avrcp_get_element_attribute_t *ind)
{
    bt_avrcp_get_element_attributes_response_t rsp = {ind->handle,
    BT_AVRCP_METADATA_PACKET_TYPE_NON_FRAGMENT,
    0};

    bt_source_srv_music_device_t *source_dev = bt_source_srv_music_get_device(BT_SRV_MUSIC_DEVICE_AVRCP_HD, (void *)&(ind->handle));

    LOG_MSGID_I(source_srv,"[AVRCP_SRV]notification_ind_handler:number: %x, handle : %x, source_dev:%x", 3, ind->number, ind->handle, source_dev);

    if (source_dev == NULL) {
        LOG_MSGID_I(source_srv,"[AVRCP_SRV]notification_ind_handler:source is not in", 0);
        return;
    }

    bt_avrcp_element_metadata_attributes_response(ind->handle, &rsp);

#if 0
    bt_source_srv_music_control_operation_ind_t avrcp_ind;

    bt_utils_memset(&avrcp_ind, 0, sizeof(bt_source_srv_music_control_operation_ind_t));
    bt_utils_memcpy(avrcp_ind.peer_address.addr, &(source_dev->dev_addr), sizeof(bt_bd_addr_t));

    for (i = 0; i < ind->number; i ++) {
        avrcp_ind.data = temp_data;
        //LOG_MSGID_I(source_srv,"[AVRCP_SRV]notification_ind_handler:data[0] : %x, data[1]:%x, data[2]:%x, data[3]:%x", 4, ind->data[0], ind->data[1], ind->data[2], ind->data[3]);
        bt_utils_memcpy(temp_data + avrcp_ind.length, &(ind->data), 4);//attrite id is 4 bytes
        avrcp_ind.length += 4;
    }

    avrcp_ind.operation = BT_SOURCE_SRV_ACTION_GET_ELEMENT;
    bt_source_srv_event_callback(BT_SOURCE_SRV_EVENT_MUSIC_CONTROL_OPERATION_IND, &ind, sizeof(bt_source_srv_music_control_operation_ind_t));
#endif
}


uint32_t bt_source_srv_get_role_by_handle(uint32_t handle)
{
    uint32_t role = BT_AVRCP_ROLE_UNDEF;

    bt_source_srv_music_device_t * source_dev = bt_source_srv_music_get_device(BT_SRV_MUSIC_DEVICE_AVRCP_HD, (void *)&handle);
    if (source_dev) {
        /*bt_bd_addr_t *addr = NULL;
        addr = &source_dev->dev_addr;*/
        role =  BT_AVRCP_ROLE_TG;
    }
    LOG_MSGID_I(source_srv,"[AVRCP_SRV]get_role_by_handle: source_dev:%x, handle:%x, role:%x", 3, source_dev, handle, role);
    return role;
}


bt_status_t bt_source_srv_avrcp_callback(bt_msg_type_t msg, bt_status_t status, void *buffer)
{
    bt_status_t ret = BT_STATUS_FAIL;
    LOG_MSGID_I(source_srv,"[AVRCP_SRV]srv_avrcp_callback: msg: 0x%x, status:%x", 2, msg, status);
    switch (msg) {
        case BT_AVRCP_CONNECT_CNF: {
            bt_avrcp_connect_cnf_t *cnf = (bt_avrcp_connect_cnf_t *)buffer;
            if (status == BT_STATUS_SUCCESS) {
                bt_source_srv_avrcp_connct_cnf_handler(cnf, status);
            }
            break;
        }
        case BT_AVRCP_CONNECT_IND: {
            bt_avrcp_connect_ind_t *ind = (bt_avrcp_connect_ind_t *)buffer;
            ret = bt_source_srv_avrcp_connect_ind_handler(ind);
            break;
        }
        case BT_AVRCP_DISCONNECT_IND: {
            bt_avrcp_disconnect_ind_t *ind = (bt_avrcp_disconnect_ind_t *)buffer;
            if (ind) {
                bt_source_srv_avrcp_disconn_ind_handler(ind, status);
            }
            break;
        }
        case BT_AVRCP_PASS_THROUGH_COMMAND_IND: {
            bt_avrcp_pass_through_command_ind_t *ind = (bt_avrcp_pass_through_command_ind_t *)buffer;
            bt_source_srv_avrcp_pass_through_command_handler(ind);
            break;
        }
        case BT_AVRCP_SET_ABSOLUTE_VOLUME_CNF: {
            bt_avrcp_set_absolute_volume_response_t *response = (bt_avrcp_set_absolute_volume_response_t *)buffer;
            bt_source_srv_set_absolute_volume_cnf_handler(response);
            break;
        }
        case BT_AVRCP_SET_ABSOLUTE_VOLUME_COMMAND_IND: {
            bt_avrcp_set_absolute_volume_response_t *response = (bt_avrcp_set_absolute_volume_response_t *)buffer;
            bt_source_srv_set_absolute_volume_ind_handler(response);
            break;
        }
        case BT_AVRCP_REGISTER_NOTIFICATION_IND: {
            bt_avrcp_register_notification_commant_t *register_ind = (bt_avrcp_register_notification_commant_t *)buffer;
            LOG_MSGID_I(source_srv,"[AVRCP_SRV]srv_avrcp_callback: reg = 0x%x", 1, register_ind);

            if (register_ind && BT_STATUS_SUCCESS == status) {
                bt_source_srv_avrcp_register_notification_ind_handler(register_ind);
            }
            break;
        }
        case BT_AVRCP_EVENT_NOTIFICATION_IND: {
            bt_avrcp_event_notification_t *notify = (bt_avrcp_event_notification_t *)buffer;
            if (notify) {
                bt_source_srv_avrcp_notification_ind_handler(notify, status);
            }
            break;
        }
        case BT_AVRCP_GET_CAPABILITY_IND: {
            uint32_t conn_handle = (uint32_t)buffer;
            bt_source_srv_avrcp_get_capability_ind_handler(conn_handle);
            break;
        }
        case BT_AVRCP_GET_PLAY_STATUS_NOTIFICATION_IND: {
            bt_avrcp_get_play_status_commant_t *noti_ind = (bt_avrcp_get_play_status_commant_t *)buffer;
            bt_source_avrcp_get_play_status_notification_ind_hanlder(noti_ind);
            break;
        }
        case BT_AVRCP_ELEMENT_METADATA_IND: {
            bt_avrcp_get_element_attribute_t *media_ind = (bt_avrcp_get_element_attribute_t *)buffer;
            if (media_ind) {
                bt_source_avrcp_get_element_attribute_notification_ind_handler(media_ind);
            }
            break;
        }
    }
    return ret;
}


bt_status_t bt_source_srv_notify_avrcp_event_change(const bt_bd_addr_t *address, bt_avrcp_event_t event, uint32_t data)
{
    bt_status_t ret = BT_STATUS_FAIL;
    uint8_t state = (uint8_t)data;
    bt_avrcp_send_register_notification_response_t rsp = {0, };
    bt_source_srv_music_device_t *dev = bt_source_srv_music_get_device(BT_SRV_MUSIC_DEVICE_ADDR_AVRCP, (void *)(address));
    LOG_MSGID_I(source_srv,"[AVRCP_SRV]notify_avrcp_event_change:state = 0x%x, event:%x, data:%x ", 3, state, event, data);

    if (dev && (dev->avrcp_state == BT_SOURCE_SRV_STATE_READY)) {
        rsp.parameter_length = 2;
        rsp.response_type = BT_AVRCP_RESPONSE_CHANGED;
        rsp.event_id = event;
        if (BT_AVRCP_EVENT_PLAYBACK_STATUS_CHANGED == event) {
            bool is_iot = bt_iot_device_white_list_check_iot_case(&(dev->dev_addr), BT_IOT_MUSIC_NO_PRE_NEXT_PASS_THROUGH_CMD);
            if (is_iot) {
                return ret;
            }
            if ((state) != dev->avrcp_play_status) {
                rsp.status = state;
                dev->avrcp_play_status = state;
                LOG_MSGID_I(source_srv,"[AVRCP_SRV]notify_avrcp_event_change: status = 0x%x", 1, rsp.status);
            } else {
                return ret;
            }
        } else if (BT_AVRCP_EVENT_VOLUME_CHANGED == event) {
            rsp.volume = state;
            dev->bt_volume = state;
        }
        ret = bt_avrcp_send_register_notification_response(dev->avrcp_hd, &rsp);

    } else {
        LOG_MSGID_E(source_srv,"[AVRCP_SRV]notify_avrcp_event_change: avrcp connect has error",  0);
    }

    return ret;
}


void bt_source_srv_avrcp_init(void *param)
{
    bt_avrcp_init_t *data = (bt_avrcp_init_t *)param;
    LOG_MSGID_I(source_srv,"[AVRCP_SRV]srv_avrcp_init: role:0x%x, br: %x", 2, data->role, data->support_browse);
    bt_avrcp_init(data);
}


bt_status_t bt_source_srv_avrcp_connect(const bt_bd_addr_t *address)
{
    uint32_t handle;
    bt_status_t ret = BT_STATUS_FAIL;
    bt_source_srv_music_device_t *dev = bt_source_srv_music_get_device(BT_SRV_MUSIC_DEVICE_ADDR_A2DP, (void *)(address));
    LOG_MSGID_I(source_srv,"[AVRCP_SRV]avrcp_connect:dev: 0x%x, addr0: %x, addr1:%x", 3, dev, (*address[0]), (*address[1]));
    if (dev) {
       LOG_MSGID_I(source_srv,"[AVRCP_SRV]avrcp_connect:a2dp ready,a2dp_handle: 0x%x, avrcp_handle:%x", 2, dev->a2dp_hd, dev->avrcp_hd);
        if ((dev->a2dp_hd != BT_SOURCE_SRV_INVAILD_HANDLE) && (dev->avrcp_hd == BT_SOURCE_SRV_INVAILD_HANDLE)) {
            ret = bt_avrcp_connect(&handle, address);
        } else {
            LOG_MSGID_E(source_srv,"[AVRCP_SRV]avrcp error ", 0);
        }
    } else {
        dev = bt_source_srv_music_get_device(BT_SRV_MUSIC_DEVICE_UNUSED, NULL);
        if (dev) {
            LOG_MSGID_I(source_srv,"[AVRCP_SRV]avrcp_connect:a2dp not ready, avrcp_handle:%x", 1, dev->avrcp_hd);
            if (BT_SOURCE_SRV_INVAILD_HANDLE == dev->avrcp_hd) {
                ret = bt_avrcp_connect(&handle, address);
            }
        }
    }

    LOG_MSGID_I(source_srv,"[AVRCP_SRV]avrcp_connect: ret = %x, handle = %x", 2, ret, handle);

    if (BT_STATUS_SUCCESS == ret) {
        dev->avrcp_hd = handle;
        dev->avrcp_role = BT_AVRCP_ROLE_TG;
        dev->avrcp_state = BT_SOURCE_SRV_STATE_CONNECTING;
        dev->avrcp_play_status = BT_AVRCP_EVENT_MEDIA_PLAY_STOPPED;
    }

    return ret;
}

bt_status_t bt_source_srv_avrcp_disconnect(const bt_bd_addr_t *address)
{
    bt_status_t ret = BT_STATUS_FAIL;
    bt_source_srv_music_device_t *dev = bt_source_srv_music_get_device(BT_SRV_MUSIC_DEVICE_ADDR_AVRCP, (void *)(address));

    if (dev && dev->avrcp_hd != BT_SOURCE_SRV_INVAILD_HANDLE) {

        ret = bt_avrcp_disconnect(dev->avrcp_hd);

        LOG_MSGID_I(source_srv,"[AVRCP_SRV][VOL]bt_source_srv_avrcp_disconnect: dev:%x, ret:%x,avrcp_state:%x",3, dev->avrcp_hd,ret, dev->avrcp_state);
        if (ret == BT_STATUS_SUCCESS) {
            dev->avrcp_state = BT_SOURCE_SRV_STATE_DISCONNECTING;
        }
    }
    return ret;
}

static uint32_t bt_source_srv_avrcp_get_map_volume(uint32_t volume)
{
    bt_sink_srv_am_volume_level_t max_level = bt_sink_srv_ami_get_a2dp_max_volume_level();
    uint32_t bt_volume = (uint8_t)((1.0f * volume * BT_AVRCP_VOL_IN_LEVEL_VALUE_MAX) / max_level + 0.5f);
    LOG_MSGID_I(source_srv,"[AVRCP_SRV][VOL]bt_source_srv_avrcp_get_map_volume: volume:%x, bt_volume:%x,max_level:%x", 3, volume, bt_volume, max_level);
    return bt_volume;
}

static uint32_t bt_source_srv_avrcp_get_volume_step(uint8_t avrcp_volume)
{
    uint16_t host_volume_scale = 0;
    bt_source_srv_common_feature_config_t common_feature_config = {0};
    float avrcp_level_f = avrcp_volume;
    if (bt_source_srv_get_feature_config(BT_SOURCE_SRV_TYPE_NONE, &common_feature_config) == BT_STATUS_SUCCESS) {
        host_volume_scale = common_feature_config.host_volume_scale;
    }
    float avrcp_step_f = (avrcp_level_f *host_volume_scale) / BT_AVRCP_VOL_IN_LEVEL_VALUE_MAX + 0.5f;
    LOG_MSGID_I(source_srv,"[AVRCP_SRV][VOL]avrcp_get_volume_step-avrcp_vol: %x, vol_step: %x, scale:%x", 3, avrcp_volume, (uint32_t)avrcp_step_f, host_volume_scale);
    return (uint32_t)avrcp_step_f;
}

static bool bt_source_srv_avrcp_is_speaker_port(bt_source_srv_port_t port)
{
    bool is_speaker = false;
    if (port == BT_SOURCE_SRV_PORT_CHAT_SPEAKER
        || port == BT_SOURCE_SRV_PORT_GAMING_SPEAKER
        || port == BT_SOURCE_SRV_PORT_LINE_IN
        || port == BT_SOURCE_SRV_PORT_I2S_IN
        || port == BT_SOURCE_SRV_PORT_I2S_IN_1) {
        is_speaker = true;
    }
    return is_speaker;
}

bt_status_t bt_source_srv_avrcp_common_action_handler(uint32_t action, void* param,uint32_t length)
{
    bt_status_t ret = BT_STATUS_FAIL;
    bt_source_srv_volume_change_t *data = (bt_source_srv_volume_change_t*)param;
    bt_source_srv_music_device_t *dev = bt_source_srv_music_get_device(BT_SRV_MUSIC_DEVICE_HIGHLIGHT, NULL);
    if (data == NULL) {
        return ret;
    }
    bool is_speaker = bt_source_srv_avrcp_is_speaker_port(data->port);
    LOG_MSGID_I(source_srv,"[AVRCP_SRV][VOL]avrcp_common_handler: action:%x, port = %x, volume:%x, is_speaker:%x", 4, action, data->port, data->volume_value, is_speaker);
    if (!is_speaker){
        return ret;
    }

    uint32_t bt_volume = bt_source_srv_avrcp_get_map_volume(data->volume_value);

    if (dev == NULL) {
        dev = bt_source_srv_music_get_device(BT_SRV_MUSIC_DEVICE_USED, NULL);
    }
    if (dev == NULL) {
        dev = bt_source_srv_music_get_device(BT_SRV_MUSIC_DEVICE_VAILD_DEVICE, NULL);
    }


    if (dev == NULL) {
        g_local_volume = bt_volume;
        LOG_MSGID_I(source_srv,"[AVRCP_SRV][VOL]avrcp_common_handler: initation , volume:0x%x, local_vol:0x%x", 2, bt_volume, g_local_volume);
        return ret;
    }

    switch (action) {
        case BT_SOURCE_SRV_ACTION_VOLUME_CHANGE:{

            if (dev && dev->avrcp_state == BT_SOURCE_SRV_STATE_READY) {
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
                if (!data->is_local) {
                    bt_source_srv_music_set_audio_volume_by_port(dev, data->port, data->volume_value);
                } else
#endif
                {
                bt_source_srv_music_stored_data_t music_data = {0};
                bt_source_srv_avrcp_read_volume(&dev->dev_addr, &music_data);
                music_data.music_volume = (music_data.music_volume & 0x7F);
                LOG_MSGID_I(source_srv,"[AVRCP_SRV][VOL]avrcp_common_handler:bt_vol:0x%x, music_vol:0x%x,absolte:%x", 3, bt_volume, music_data.music_volume,dev->absolute_support, dev->absolute_support);
                if (bt_volume !=  music_data.music_volume) {
                    bt_source_srv_music_stored_data_t dev_db;
                    bt_source_srv_memset(&dev_db, 0, sizeof(bt_source_srv_music_stored_data_t));
                    dev_db.music_volume = (uint8_t)(bt_volume | BT_SOURCE_SRV_MUSIC_VOLUME_SET_FLAG);
                    if (dev->absolute_support) {
                        ret = bt_avrcp_set_absolute_volume(dev->avrcp_hd, (uint8_t)bt_volume);
                    } else {
                        bt_source_srv_notify_avrcp_event_change(&dev->dev_addr, BT_AVRCP_EVENT_VOLUME_CHANGED, bt_volume);
                    }
                    if (ret == BT_STATUS_SUCCESS) {
                        bt_source_srv_avrcp_set_nvdm_data(&dev->dev_addr, &dev_db);
                    }
                }
                LOG_MSGID_I(source_srv,"[AVRCP_SRV][VOL]avrcp_common_handler:ret:0x%x, local_vol:0x%x", 2, ret, g_local_volume);
                }
            } else {
                LOG_MSGID_I(source_srv,"[AVRCP_SRV][VOL]avrcp_common_handler: AVRCP NOT CONNECTED , volume:0x%x, local_vol:0x%x", 2,bt_volume, g_local_volume);
            }
            g_local_volume = bt_volume;
            ret = BT_STATUS_SUCCESS;
            break;
        }
        default:
            break;
    }
    return ret;
}

bt_status_t bt_source_srv_avrcp_action_handler(uint32_t action, void* param,uint32_t length)
{
    bt_source_srv_music_device_t *dev = NULL;
    bt_source_srv_music_action_t *srv_action = NULL;
    uint32_t avrcp_status = BT_AVRCP_EVENT_MEDIA_PLAY_STOPPED;
    uint32_t handle = BT_SOURCE_SRV_INVAILD_HANDLE;
    bt_status_t ret = BT_STATUS_SUCCESS;

    srv_action = (bt_source_srv_music_action_t *)param;
    if (srv_action == NULL) {
        return ret;
    }
    dev = bt_source_srv_music_get_device(BT_SRV_MUSIC_DEVICE_ADDR_AVRCP, &(srv_action->peer_address.addr));
    if (dev == NULL) {
        return ret;
    }
    handle = dev->avrcp_hd;
    LOG_MSGID_I(source_srv,"[AVRCP_SRV]avrcp_action_handler: dev:%x, handle:%x", 3, dev, handle,action);

    switch(action) {
        case BT_SOURCE_SRV_ACTION_PLAY:
        {
            avrcp_status = BT_AVRCP_EVENT_MEDIA_PLAYING;
            ret = bt_source_srv_notify_avrcp_event_change(&srv_action->peer_address.addr, BT_AVRCP_EVENT_PLAYBACK_STATUS_CHANGED, avrcp_status);
            break;
        }
        case BT_SOURCE_SRV_ACTION_PAUSE:{
            avrcp_status = BT_AVRCP_EVENT_MEDIA_PAUSED;
            ret = bt_source_srv_notify_avrcp_event_change(&srv_action->peer_address.addr, BT_AVRCP_EVENT_PLAYBACK_STATUS_CHANGED, avrcp_status);
            break;
        }
        case BT_SOURCE_SRV_ACTION_NEXT:{
            break;
        }
        case BT_SOURCE_SRV_ACTION_PREVIOUS:{
            break;
        }
        case BT_SOURCE_SRV_ACTION_GET_ELEMENT:{
            uint32_t i = 0;
            uint8_t *p_data = NULL;
            uint16_t temp_length = 0;
            bt_avrcp_get_element_attributes_response_t rsp = {0};
            if (length > 512){
                for (;  i < 5;i++) {
                    rsp.packet_type = BT_AVRCP_METADATA_PACKET_TYPE_START;
                    rsp.length = temp_length;
                    temp_length = length - i * rsp.length;
                }
                // to do
            } else {
                rsp.packet_type = BT_AVRCP_METADATA_PACKET_TYPE_NON_FRAGMENT;
                rsp.length = length; //(high_len << 8) | low_len;
            }
            p_data = (uint8_t*)bt_utils_memory_alloc(rsp.length);
            if (p_data == NULL) {
                LOG_MSGID_E(source_srv,"[AVRCP_SRV]set_element:alloc momery fail", 0);
                break;
            }

            rsp.data = p_data;
            LOG_MSGID_I(source_srv,"[AVRCP_SRV]set_element:p_data:%x, tmp_len: %x", 2, p_data, temp_length);
            bt_utils_memcpy(p_data, srv_action->data, rsp.length);
            ret = bt_avrcp_element_metadata_attributes_response(handle, &rsp);
            LOG_MSGID_I(source_srv,"[AVRCP_SRV]set_element:ret:%x, type:%x, len: %x", 3, ret, length, rsp.data);
            bt_utils_memory_free(p_data);
            break;
        }
        default:
            ret = BT_STATUS_FAIL;
            break;
    }

    return ret;
}


bt_status_t bt_source_srv_avrcp_BQB_notify_volume_change(bool is_notify,uint8_t volume)
{
    bt_source_srv_music_device_t *dev = bt_source_srv_music_get_device(BT_SRV_MUSIC_DEVICE_USED, NULL);
    bt_status_t ret = BT_STATUS_SUCCESS;

    if (dev == NULL) {
        dev = bt_source_srv_music_get_device(BT_SRV_MUSIC_DEVICE_VAILD_DEVICE, NULL);
    }
    if (is_notify) {
        ret = bt_source_srv_notify_avrcp_event_change(&dev->dev_addr, BT_AVRCP_EVENT_VOLUME_CHANGED, volume);
    } else {
        ret = bt_avrcp_set_absolute_volume(dev->avrcp_hd, volume);
    }
    LOG_MSGID_E(source_srv,"[AVRCP_SRV]BQB_notify_volume_change:is_notify:%x,volume:%x, ret:%x",3,is_notify,volume, ret);

    return ret;
}
