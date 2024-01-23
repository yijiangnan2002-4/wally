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
#include "app_le_audio_aird_client.h"

#include "app_lea_service_conn_mgr.h"
#include "app_lea_service_event.h"

#include "apps_debug.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"
#include "apps_config_vp_index_list.h"

#include "bt_connection_manager.h"
#include "bt_connection_manager_internal.h"
#include "bt_device_manager.h"
#include "bt_sink_srv_le_cap_stream.h"
#include "bt_sink_srv_le_volume.h"
#include "bt_sink_srv_a2dp.h"

#include "bt_gattc.h"
#include "bt_gattc_discovery.h"

#ifdef MTK_AWS_MCE_ENABLE
#include "app_rho_idle_activity.h"
#include "bt_aws_mce_srv.h"
#endif

#include "voice_prompt_api.h"

/**************************************************************************************************
* Define
**************************************************************************************************/
#define LOG_TAG     "[LEA][AIRD_CLIENT]"

#define APP_LE_AUDIO_DISABLE_NOTIFICATION       0
#define APP_LE_AUDIO_ENABLE_NOTIFICATION        1
#define APP_LE_AUDIO_CCCD_VALUE_LEN             2
#define APP_LE_AUDIO_ATT_HDR_SIZE               3   /* |opcode (1 bytes) | att_handle (2 bytes) | */
#define APP_LE_AUDIO_ATT_VALUE_IDX              3   /* byte_0: opcode, byte_1~2: att_handle, byte_3:att_value */

#define APP_LE_AIRD_MAX_CHARC_NUMBER            2

#define APP_LE_AUDIO_AIRD_ACTION_QUEUE_MAX_NUM  5

#define APP_LE_AUDIO_MAX_VOL_LEV            bt_sink_srv_ami_get_a2dp_max_volume_level() // LEA use A2DP volume table

typedef struct {
    bt_gattc_discovery_characteristic_t         charc[APP_LE_AIRD_MAX_CHARC_NUMBER];
    bt_gattc_discovery_descriptor_t             descrp[APP_LE_AIRD_MAX_CHARC_NUMBER];
} app_le_audio_aird_discovery_charc_t;

typedef struct {
    void                                       *next;
    bt_gattc_write_without_rsp_req_t            write_cmd;
    uint8_t                                     buf[1];
} app_le_audio_aird_action_node_t;

static const uint8_t LEA_AIRD_CHARC_UUID_RX[16] = {0x41, 0x45, 0x4C, 0x61, 0x68, 0x6F, 0x72, 0x69,
                                                   0x41, 0x30, 0xAB, 0x2D, 0x52, 0x41, 0x48, 0x43
                                                  };
static const uint8_t LEA_AIRD_CHARC_UUID_TX[16] = {0x41, 0x45, 0x4C, 0x61, 0x68, 0x6F, 0x72, 0x69,
                                                   0x41, 0x31, 0xAB, 0x2D, 0x52, 0x41, 0x48, 0x43
                                                  };

/**************************************************************************************************
 * Variable
**************************************************************************************************/
static app_le_audio_aird_discovery_charc_t      app_lea_aird_charc_discovery;
static bt_gattc_discovery_service_t             app_lea_aird_discovery;

static app_le_audio_aird_client_info_t          app_lea_aird_client_info[APP_LEA_MAX_CONN_NUM] = {0};
static app_le_audio_aird_client_callback_t      app_lea_aird_client_callback = NULL;

static bool                                     app_lea_aird_device_busy_flag = FALSE;
static bool                                     app_lea_aird_block_stream_flag = FALSE;
/**************************************************************************************************
 * Prototype
**************************************************************************************************/
static app_le_audio_aird_client_info_t *app_le_audio_aird_client_get_info(bt_handle_t handle);

/**************************************************************************************************
 * Static function
**************************************************************************************************/
static void app_le_audio_aird_client_proc_bt_group(uint32_t event_id, void *extra_data, size_t data_len)
{
    switch (event_id) {
        case BT_POWER_ON_CNF: {
            app_lea_aird_device_busy_flag = FALSE;
            app_lea_aird_block_stream_flag = FALSE;
            break;
        }
    }
}

static void app_le_audio_aird_client_proc_lea_group(uint32_t event_id, void *extra_data, size_t data_len)
{
    switch (event_id) {
        case EVENT_ID_LE_AUDIO_RESET_DONGLE_BUSY_EVENT: {
            APPS_LOG_MSGID_I(LOG_TAG" LE Audio event, RESET_DONGLE_BUSY_EVENT", 0);
            app_le_audio_aird_client_notify_dongle_media_state(FALSE, NULL, 0);
            break;
        }
    }
}

#if defined(MTK_AWS_MCE_ENABLE) && defined(AIR_INFORM_CONNECTION_STATUS_ENABLE)
static void app_le_audio_aird_client_proc_interaction_group(uint32_t event_id, void *extra_data, size_t data_len)
{
    if (event_id == APPS_EVENTS_INTERACTION_RHO_END || event_id == APPS_EVENTS_INTERACTION_PARTNER_SWITCH_TO_AGENT) {
        bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
        app_rho_result_t rho_ret = (app_rho_result_t)extra_data;
        if (APP_RHO_RESULT_SUCCESS == rho_ret) {
            uint32_t edr_num = bt_cm_get_acl_connected_device(NULL, 0);
            uint8_t lea_num = app_lea_conn_mgr_get_conn_num();
            bool is_sp_connected = (edr_num > 0 || lea_num > 1);
            APPS_LOG_MSGID_I(LOG_TAG"[Silence_Detection] [%02X] RHO end, edr_num=%d lea_num=%d",
                             3, role, edr_num, lea_num);
            app_le_audio_aird_client_infom_connection_status(is_sp_connected);
        }
    }
}
#endif

static bool app_le_audio_aird_client_add_action_node(app_le_audio_aird_client_info_t *info, app_le_audio_aird_action_node_t *new_node)
{
    if (NULL == info || NULL == new_node) {
        return FALSE;
    }

    if (NULL == info->action_queue) {
        /* add node to queue head */
        info->action_queue = (uint8_t *)new_node;
        //APPS_LOG_MSGID_I(LOG_TAG" add_action_node, head node=0x%08X", 1, new_node);
        return TRUE;
    }

    uint8_t i = 1;
    app_le_audio_aird_action_node_t *node = (app_le_audio_aird_action_node_t *)info->action_queue;
    while (NULL != node->next) {
        i++;
        node = (app_le_audio_aird_action_node_t *)node->next;
    }
    node->next = new_node;
    //APPS_LOG_MSGID_I(LOG_TAG" add_action_node, node_num=%d node=0x%08X", 2, i, new_node);

    if (APP_LE_AUDIO_AIRD_ACTION_QUEUE_MAX_NUM <= i) {
        node = (app_le_audio_aird_action_node_t *)info->action_queue;
        info->action_queue = (uint8_t *)node->next;
        //APPS_LOG_MSGID_I(LOG_TAG" add_action_node, free 1st", 0);
        vPortFree(node);
    }
    return TRUE;
}

static void app_le_audio_aird_cilent_do_resend_action(bt_handle_t handle, app_le_audio_aird_client_info_t *info)
{
    if (info == NULL || info->action_queue == NULL) {
        return;
    }

    app_le_audio_aird_action_node_t *node = (app_le_audio_aird_action_node_t *)info->action_queue;
    while (node != NULL) {
        bt_status_t bt_status = bt_gattc_write_without_rsp(handle, FALSE, &(node->write_cmd));
        APPS_LOG_MSGID_I(LOG_TAG" do_resend_action, handle=0x%04X action=%d bt_status=0x%08X",
                         3, handle, node->buf[APP_LE_AUDIO_ATT_VALUE_IDX], bt_status);

        info->action_queue = (uint8_t *)node->next;
        vPortFree(node);
        node = (app_le_audio_aird_action_node_t *)info->action_queue;
    }
}

static void app_le_audio_aird_client_resend_action(bt_handle_t handle)
{
    uint8_t index = app_lea_conn_mgr_get_index(handle);
    if (index >= APP_LEA_MAX_CONN_NUM) {
        //APPS_LOG_MSGID_E(LOG_TAG" resend_action, error link handle=0x%04X", 1, handle);
        return;
    }
    app_le_audio_aird_cilent_do_resend_action(handle, &app_lea_aird_client_info[index]);
}

static void app_le_audio_aird_client_handle_ready(bt_handle_t handle)
{
    if (app_lea_aird_block_stream_flag) {
        app_le_audio_aird_action_block_stream_t param = {0};
        param.type = APP_LE_AUDIO_AIRD_BLOCK_STREAM_ALL;
        app_le_audio_aird_client_send_action(handle, APP_LE_AUDIO_AIRD_ACTION_BLOCK_STREAM, &param, sizeof(app_le_audio_aird_action_block_stream_t));
    }
    if (app_lea_aird_device_busy_flag) {
        app_le_audio_aird_client_send_action(handle, APP_LE_AUDIO_AIRD_ACTION_SET_DEVICE_BUSY, NULL, 0);
    }

#ifdef AIR_INFORM_CONNECTION_STATUS_ENABLE
    uint32_t edr_num = bt_cm_get_acl_connected_device(NULL, 0);
    uint8_t lea_num = app_lea_conn_mgr_get_conn_num();
    bool is_sp_connected = (edr_num > 0 || lea_num > 1);
    if (is_sp_connected) {
        APPS_LOG_MSGID_I(LOG_TAG"[Silence_Detection] handle_ready, sp_connected edr_num=%d lea_num=%d",
                         2, edr_num, lea_num);
        app_le_audio_aird_client_infom_connection_status(is_sp_connected);
    }
#endif

    extern void app_lea_conn_mgr_notify_aird_ready(bt_handle_t handle);
    app_lea_conn_mgr_notify_aird_ready(handle);
}

static bt_status_t app_le_audio_aird_client_set_cccd(bt_handle_t handle, uint16_t att_handle, uint16_t cccd)
{
    uint8_t buf[5] = {0};
    BT_GATTC_NEW_WRITE_CHARC_REQ(req, buf, att_handle, (uint8_t *)&cccd, APP_LE_AUDIO_CCCD_VALUE_LEN);
    bt_status_t bt_status = bt_gattc_write_charc(handle, &req);
    APPS_LOG_MSGID_I(LOG_TAG" set_cccd, handle=0x%04X att_handle=%d bt_status=0x%08X",
                     3, handle, att_handle, bt_status);
    return bt_status;
}

static bool app_le_audio_aird_client_start_pre_action(bt_handle_t handle, app_le_audio_aird_client_info_t *info)
{
    bool ret = FALSE;
    if (BT_HANDLE_INVALID == handle || NULL == info) {
        //APPS_LOG_MSGID_E(LOG_TAG" start_pre_action, error handle=0x%04X info=0x%08X", 2, handle, info);
        return FALSE;
    } else if (APP_LE_AUDIO_AIRD_CLIENT_STATE_SRV_DISCOVERY_COMPLETE > info->state
               || APP_LE_AUDIO_AIRD_CLIENT_STATE_READY <= info->state) {
        //APPS_LOG_MSGID_E(LOG_TAG" start_pre_action, error state=%d", 1, info->state);
        return FALSE;
    }

    APPS_LOG_MSGID_I(LOG_TAG" start_pre_action, start handle=0x%04X state=%d att_handle_tx_cccd=0x%04X block_stream=%d device_busy=%d",
                     5, handle, info->state, info->att_handle_tx_cccd, app_lea_aird_block_stream_flag, app_lea_aird_device_busy_flag);

    switch (info->state) {
        case APP_LE_AUDIO_AIRD_CLIENT_STATE_SRV_DISCOVERY_COMPLETE: {
            if (BT_HANDLE_INVALID != info->att_handle_tx_cccd) {
                info->state = APP_LE_AUDIO_AIRD_CLIENT_STATE_SET_CCCD_TX;
                bt_status_t bt_status = app_le_audio_aird_client_set_cccd(handle,
                                                                          info->att_handle_tx_cccd,
                                                                          APP_LE_AUDIO_ENABLE_NOTIFICATION);
                APPS_LOG_MSGID_I(LOG_TAG" start_pre_action, state=SRV_DISCOVERY_COMPLETE bt_status=0x%08X", 1, bt_status);
                if (bt_status == BT_STATUS_SUCCESS) {
                    ret = TRUE;
                    break;
                }
            }
        }
        /*Pass through*/
        case APP_LE_AUDIO_AIRD_CLIENT_STATE_SET_CCCD_TX: {
            //APPS_LOG_MSGID_I(LOG_TAG" start_pre_action, state=SET_CCCD_TX block_stream=%d device_busy=%d", 2, app_lea_aird_block_stream_flag, app_lea_aird_device_busy_flag);
            info->state = APP_LE_AUDIO_AIRD_CLIENT_STATE_READY;
            app_le_audio_aird_client_handle_ready(handle);
            bt_gattc_discovery_continue(handle);
            ret = TRUE;
            break;
        }
    }

    APPS_LOG_MSGID_I(LOG_TAG" start_pre_action, end handle=0x%04X state=%d ret=%d",
                     3, handle, info->state, ret);
    return ret;
}

static void app_le_audio_aird_client_handle_write_rsp(bt_gattc_write_rsp_t *rsp)
{
    app_le_audio_aird_client_info_t *info = app_le_audio_aird_client_get_info(rsp->connection_handle);
    if (info == NULL) {
        //APPS_LOG_MSGID_E(LOG_TAG" handle_write_rsp, link not exist", 0);
        return;
    }

    app_le_audio_aird_client_start_pre_action(rsp->connection_handle, info);
}

static void app_le_audio_aird_client_handle_error_rsp(bt_status_t status, bt_gattc_error_rsp_t *rsp)
{
    app_le_audio_aird_client_info_t *info = app_le_audio_aird_client_get_info(rsp->connection_handle);
    if (info == NULL) {
        //APPS_LOG_MSGID_E(LOG_TAG" handle_error_rsp, link not exist", 0);
        return;
    }

    app_le_audio_aird_client_start_pre_action(rsp->connection_handle, info);
}

static void app_le_audio_aird_client_handle_notification(bt_gatt_handle_value_notification_t *notification)
{
    app_le_audio_aird_client_info_t *info = app_le_audio_aird_client_get_info(notification->connection_handle);
    bt_att_handle_value_notification_t *att_rsp = notification->att_rsp;
    uint16_t data_len = notification->length;

    if (info == NULL) {
        //APPS_LOG_MSGID_E(LOG_TAG" handle_notification, error conn handle", 0);
        return;
    } else if (att_rsp->handle != info->att_handle_tx) {
        //APPS_LOG_MSGID_E(LOG_TAG" handle_notification, invalid TX handle %d", 1, att_rsp->handle);
        return;
    } else if (data_len <= APP_LE_AUDIO_ATT_HDR_SIZE) {
        APPS_LOG_MSGID_E(LOG_TAG" handle_notification, invalid data_len=%d", 1, data_len);
        return;
    }

    data_len -= APP_LE_AUDIO_ATT_HDR_SIZE;
    uint8_t event = att_rsp->attribute_value[0];
    APPS_LOG_MSGID_I(LOG_TAG" handle_notification, handle=0x%04X data_len=%d event=%d",
                     3, notification->connection_handle, data_len, event);

    switch (event) {
        case APP_LE_AUDIO_AIRD_EVENT_MODE_INFO: {
            if (data_len == sizeof(app_le_audio_aird_event_mode_info_ind_t)) {
                app_le_audio_aird_event_mode_info_ind_t *ind = (app_le_audio_aird_event_mode_info_ind_t *)&att_rsp->attribute_value[0];
                info->mode = ind->mode;
                APPS_LOG_MSGID_I(LOG_TAG" handle_notification, mode_info=%d", 1, info->mode);
            }
            break;
        }
        case APP_LE_AUDIO_AIRD_EVENT_MIC_MUTE: {
            if (data_len == sizeof(app_le_audio_aird_event_mic_mute_ind_t)) {

                app_le_audio_aird_event_mic_mute_ind_t *ind = (app_le_audio_aird_event_mic_mute_ind_t *)&att_rsp->attribute_value[0];
                bt_status_t ret;
                bt_sink_srv_le_set_mute_state_and_volume_level_t param = {
                    .set_mute = true,
                    .mute = ind->mic_mute,
                    .set_volume_level = false,
                };

                info->mic_mute = ind->mic_mute;
                APPS_LOG_MSGID_I(LOG_TAG" handle_noti_mic_mute, mute:%x %x", 2, info->mic_mute, param.mute);

                ret = bt_sink_srv_le_volume_vcp_send_action(notification->connection_handle,
                                                            BT_SINK_SRV_LE_VCS_ACTION_SET_MIC_MUTE_STATE_AND_VOLUME_LEVEL,
                                                            &param);

                APPS_LOG_MSGID_I(LOG_TAG" handle_noti_mic_mute, ret:%x", 1, ret);
            }
            break;
        }
        case APP_LE_AUDIO_AIRD_EVENT_VOLUME_CHANGE: {
            if (data_len == sizeof(app_le_audio_aird_event_volume_change_ind_t)) {

                app_le_audio_aird_event_volume_change_ind_t *ind = (app_le_audio_aird_event_volume_change_ind_t *)&att_rsp->attribute_value[0];
                bt_sink_srv_le_vcp_action_t action;
                float vol = ind->volume;
                float vol_level = 0;
                bt_status_t ret = BT_STATUS_SUCCESS;

                if (APP_LE_AUDIO_AIRD_STREAMING_INTERFACE_SPEAKER == ind->streaming_interface) {
                    action = BT_SINK_SRV_LE_VCS_ACTION_SET_MUTE_STATE_AND_VOLUME_LEVEL;
                    vol_level = (vol * APP_LE_AUDIO_MAX_VOL_LEV) / 100 + 0.5f;

                    if (APP_LE_AUDIO_AIRD_VOLUME_MAX == ind->volume
                        || APP_LE_AUDIO_AIRD_VOLUME_MIN == ind->volume) {
                        /* SPK vol: Max/Min */
#if defined(MTK_AWS_MCE_ENABLE)
                        if (BT_AWS_MCE_SRV_LINK_NONE != bt_aws_mce_srv_get_link_type()
                            && BT_AWS_MCE_ROLE_AGENT == bt_device_manager_aws_local_info_get_role())
#endif
                        {
                            voice_prompt_play_sync_vp_succeed();
                        }
                    }
                } else if (APP_LE_AUDIO_AIRD_STREAMING_INTERFACE_MICROPHONE == ind->streaming_interface) {
                    action = BT_SINK_SRV_LE_VCS_ACTION_SET_MIC_MUTE_STATE_AND_VOLUME_LEVEL;
                    vol_level = (vol * 15) / 100 + 0.5f;

                } else {
                    break;
                }

                bt_sink_srv_le_set_mute_state_and_volume_level_t param = {
                    .set_mute = false,
                    .set_volume_level = true,
                    .volume_level = (uint8_t)vol_level,
                };

                APPS_LOG_MSGID_I(LOG_TAG" handle_noti_vol_change, action:%x ind(vol:%x) param(vol_level:%d set_vol_level:%x mute:%x set_mute:%x)", 6,
                                 action, ind->volume,
                                 param.volume_level, param.set_volume_level, param.mute, param.set_mute);

                ret = bt_sink_srv_le_volume_vcp_send_action(notification->connection_handle, action, &param);

                APPS_LOG_MSGID_I(LOG_TAG" handle_noti_vol_change, ret:%x", 1, ret);
            }
            break;
        }
        case APP_LE_AUDIO_AIRD_EVENT_EXTENDED_VOLUME_CHANGE: {
            if (data_len != sizeof(app_le_audio_aird_event_extended_volume_change_ind_t)) {
                break;
            }

            app_le_audio_aird_event_extended_volume_change_ind_t *ind = (app_le_audio_aird_event_extended_volume_change_ind_t *)&att_rsp->attribute_value[0];
            bt_sink_srv_le_vcp_action_t action;
            float vol = ind->volume;
            float vol_level = 0;
            bt_status_t ret;

            if (APP_LE_AUDIO_AIRD_STREAMING_INTERFACE_SPEAKER == ind->streaming_interface) {
                action = BT_SINK_SRV_LE_VCS_ACTION_SET_MUTE_STATE_AND_VOLUME_LEVEL;
                vol_level = (vol * APP_LE_AUDIO_MAX_VOL_LEV) / 100 + 0.5f;

            } else if (APP_LE_AUDIO_AIRD_STREAMING_INTERFACE_MICROPHONE == ind->streaming_interface){
                action = BT_SINK_SRV_LE_VCS_ACTION_SET_MIC_MUTE_STATE_AND_VOLUME_LEVEL;
                vol_level = (vol * 15) / 100 + 0.5f;

            } else {
                break;
            }

            bt_sink_srv_le_set_mute_state_and_volume_level_t param = {
                .set_mute = true,
                .mute = ind->mute,
                .set_volume_level = true,
                .volume_level = (uint8_t)vol_level,
            };

            APPS_LOG_MSGID_I(LOG_TAG" handle_noti_ext_vol_change, action:%x ind(vol:%x mute:%x) param(vol_level:%d set_vol_level:%x mute:%x set_mute:%x)", 7,
                             action, ind->volume, ind->mute,
                             param.volume_level, param.set_volume_level, param.mute, param.set_mute);

            ret = bt_sink_srv_le_volume_vcp_send_action(notification->connection_handle, action, &param);

            APPS_LOG_MSGID_I(LOG_TAG" handle_noti_ext_vol_change, ret:%x", 1, ret);
            break;
        }
    }
}

static void app_le_audio_aird_client_discovery_callback(bt_gattc_discovery_event_t *event)
{
    app_le_audio_aird_client_info_t *info = NULL;
    bt_gattc_discovery_characteristic_t *charc = NULL;
    uint8_t i = 0;
    if (event == NULL) {
        APPS_LOG_MSGID_E(LOG_TAG" discovery_callback, NULL event", 0);
        return;
    } else if ((event->event_type != BT_GATTC_DISCOVERY_EVENT_COMPLETE && !event->last_instance)
               || (NULL == (info = app_le_audio_aird_client_get_info(event->conn_handle)))) {
        if (app_lea_aird_client_callback != NULL) {
            app_le_audio_aird_client_event_srv_discovery_complete_t param = {0};
            param.handle = event->conn_handle;
            param.status = BT_STATUS_FAIL;
            app_lea_aird_client_callback(APP_LE_AUDIO_AIRD_CLIENT_EVENT_SRV_DISCOVERY_COMPLETE, &param);
        }

        bt_gattc_discovery_continue(event->conn_handle);
        APPS_LOG_MSGID_E(LOG_TAG" discovery_callback, error handle=0x%04X info=0x%08X charc_num=%d",
                         3, event->conn_handle, info, app_lea_aird_discovery.char_count_found);
        return;
    }

    APPS_LOG_MSGID_I(LOG_TAG" discovery_callback, handle=0x%04X event_type=%d charc_num=%d",
                     3, event->conn_handle, event->event_type, app_lea_aird_discovery.char_count_found);

    if (BT_GATTC_DISCOVERY_EVENT_COMPLETE == event->event_type) {
        if (0 != (i = app_lea_aird_discovery.char_count_found)) {
            while (i > 0) {
                i--;
                charc = &app_lea_aird_discovery.charateristics[i];

#if 0
                APPS_LOG_MSGID_I(LOG_TAG" discovery_callback, UUID=%02X:%02X:%02X:%02X %02X:%02X:%02X:%02X",
                                 8, charc->char_uuid.uuid.uuid[0], charc->char_uuid.uuid.uuid[1],
                                 charc->char_uuid.uuid.uuid[2], charc->char_uuid.uuid.uuid[3],
                                 charc->char_uuid.uuid.uuid[4], charc->char_uuid.uuid.uuid[5],
                                 charc->char_uuid.uuid.uuid[6], charc->char_uuid.uuid.uuid[7]);
                APPS_LOG_MSGID_I(LOG_TAG" discovery_callback, UUID=%02X:%02X:%02X:%02X %02X:%02X:%02X:%02X",
                                 8, charc->char_uuid.uuid.uuid[8], charc->char_uuid.uuid.uuid[9],
                                 charc->char_uuid.uuid.uuid[10], charc->char_uuid.uuid.uuid[11],
                                 charc->char_uuid.uuid.uuid[12], charc->char_uuid.uuid.uuid[13],
                                 charc->char_uuid.uuid.uuid[14], charc->char_uuid.uuid.uuid[15]);
#endif
                if (0 == memcmp(&charc->char_uuid.uuid.uuid[0], &LEA_AIRD_CHARC_UUID_RX[0], 16)) {
                    /* RX */
                    info->att_handle_rx = charc->value_handle;
                    //APPS_LOG_MSGID_I(LOG_TAG" discovery_callback, att_handle_rx=%d", 1, info->att_handle_rx);
                } else if (0 == memcmp(&charc->char_uuid.uuid.uuid[0], &LEA_AIRD_CHARC_UUID_TX[0], 16)) {
                    /* TX */
                    info->att_handle_tx = charc->value_handle;
                    info->att_handle_tx_cccd = charc->descriptor[0].handle;
                    //APPS_LOG_MSGID_I(LOG_TAG" discovery_callback, att_handle_tx=%d att_handle_tx_cccd=%d",
                    //                 2, info->att_handle_tx, info->att_handle_tx_cccd);
                }
            }

            info->state = APP_LE_AUDIO_AIRD_CLIENT_STATE_SRV_DISCOVERY_COMPLETE;
            app_le_audio_aird_client_start_pre_action(event->conn_handle, info);

            if (app_lea_aird_client_callback != NULL) {
                app_le_audio_aird_client_event_srv_discovery_complete_t param = {0};
                param.handle = event->conn_handle;
                param.status = BT_STATUS_SUCCESS;
                app_lea_aird_client_callback(APP_LE_AUDIO_AIRD_CLIENT_EVENT_SRV_DISCOVERY_COMPLETE, &param);
            }
            return;
        }
    }

    if (app_lea_aird_client_callback != NULL) {
        app_le_audio_aird_client_event_srv_discovery_complete_t param = {0};
        param.handle = event->conn_handle;
        param.status = BT_STATUS_FAIL;
        app_lea_aird_client_callback(APP_LE_AUDIO_AIRD_CLIENT_EVENT_SRV_DISCOVERY_COMPLETE, &param);
    }

    bt_gattc_discovery_continue(event->conn_handle);
}

static app_le_audio_aird_client_info_t *app_le_audio_aird_client_get_info(bt_handle_t handle)
{
    uint8_t index = app_lea_conn_mgr_get_index(handle);
    if (index >= APP_LEA_MAX_CONN_NUM) {
        APPS_LOG_MSGID_E(LOG_TAG" get_info, error handle=0x%04X index=%d", 2, handle, index);
        return NULL;
    }
    return &app_lea_aird_client_info[index];
}



/**************************************************************************************************
 * Public function
**************************************************************************************************/
bt_status_t app_le_audio_aird_client_register_callback(app_le_audio_aird_client_callback_t callback)
{
    if (NULL != app_lea_aird_client_callback) {
        return BT_STATUS_FAIL;
    }

    app_lea_aird_client_callback = callback;
    return BT_STATUS_SUCCESS;
}

void app_le_audio_aird_client_init(void)
{
    uint8_t i = 0;

    /* Clear AIRD Client Context */
    i = APP_LEA_MAX_CONN_NUM;
    while (i > 0) {
        i--;
        app_le_audio_aird_client_reset_info(i);
    }

    /* Register AIRD discovery */
    i = APP_LE_AIRD_MAX_CHARC_NUMBER;
    memset(&app_lea_aird_charc_discovery, 0, sizeof(app_le_audio_aird_discovery_charc_t));
    while (i > 0) {
        i--;
        app_lea_aird_charc_discovery.charc[i].descriptor_count = 1;
        app_lea_aird_charc_discovery.charc[i].descriptor = &app_lea_aird_charc_discovery.descrp[i];
    }

    ble_uuid_t aird_srv_uuid = {
        .type = BLE_UUID_TYPE_128BIT,
        .uuid.uuid = {0x41, 0x45, 0x4C, 0x61, 0x68, 0x6F, 0x72, 0x69, 0x41, 0x01, 0xAB, 0x2D, 0x4D, 0x49, 0x52, 0x50}
    };
    memset(&app_lea_aird_discovery, 0, sizeof(bt_gattc_discovery_service_t));
    app_lea_aird_discovery.charateristics = app_lea_aird_charc_discovery.charc;
    app_lea_aird_discovery.characteristic_count = APP_LE_AIRD_MAX_CHARC_NUMBER;

    bt_gattc_discovery_user_data_t discovery_data = {
        .uuid = aird_srv_uuid,
        .need_cache = TRUE,
        .srv_info = &app_lea_aird_discovery,
        .handler = app_le_audio_aird_client_discovery_callback
    };
    bt_gattc_discovery_register_service(BT_GATTC_DISCOVERY_USER_LE_AUDIO, &discovery_data);
}

void app_le_audio_aird_client_event_handler(bt_msg_type_t msg, bt_status_t status, void *buf)
{
    if (NULL == buf) {
        return;
    }

    switch (msg) {
        case BT_GATTC_DISCOVER_PRIMARY_SERVICE: {
            app_le_audio_aird_client_resend_action(((bt_gattc_read_by_group_type_rsp_t *)buf)->connection_handle);
            break;
        }
        case BT_GATTC_DISCOVER_PRIMARY_SERVICE_BY_UUID: {
            app_le_audio_aird_client_resend_action(((bt_gattc_find_by_type_value_rsp_t *)buf)->connection_handle);
            break;
        }
        case BT_GATTC_FIND_INCLUDED_SERVICES:
        case BT_GATTC_DISCOVER_CHARC:
        case BT_GATTC_READ_USING_CHARC_UUID: {
            app_le_audio_aird_client_resend_action(((bt_gattc_read_by_type_rsp_t *)buf)->connection_handle);
            break;
        }
        case BT_GATTC_DISCOVER_CHARC_DESCRIPTOR: {
            app_le_audio_aird_client_resend_action(((bt_gattc_find_info_rsp_t *)buf)->connection_handle);
            break;
        }
        case BT_GATTC_READ_CHARC:
        case BT_GATTC_READ_LONG_CHARC: {
            app_le_audio_aird_client_resend_action(((bt_gattc_read_rsp_t *)buf)->connection_handle);
            break;
        }
        case BT_GATTC_READ_MULTI_CHARC_VALUES: {
            app_le_audio_aird_client_resend_action(((bt_gattc_read_multiple_rsp_t *)buf)->connection_handle);
            break;
        }
        case BT_GATTC_WRITE_LONG_CHARC:
        case BT_GATTC_WRITE_CHARC: {
            if (status == BT_STATUS_SUCCESS) {
                app_le_audio_aird_client_handle_write_rsp((bt_gattc_write_rsp_t *)buf);
                app_le_audio_aird_client_resend_action(((bt_gattc_write_rsp_t *)buf)->connection_handle);
            } else {
                app_le_audio_aird_client_handle_error_rsp(status, (bt_gattc_error_rsp_t *)buf);
                app_le_audio_aird_client_resend_action(((bt_gattc_error_rsp_t *)buf)->connection_handle);
            }
            break;
        }
        case BT_GATTC_RELIABLE_WRITE_CHARC: {
            app_le_audio_aird_client_resend_action(((bt_gattc_execute_write_rsp_t *)buf)->connection_handle);
            break;
        }
        case BT_GATTC_CHARC_VALUE_NOTIFICATION: {
            app_le_audio_aird_client_handle_notification((bt_gatt_handle_value_notification_t *)buf);
            break;
        }
        default:
            break;
    }
}

void app_le_audio_aird_client_reset_info(uint8_t index)
{
    if (APP_LEA_MAX_CONN_NUM <= index) {
        APPS_LOG_MSGID_E(LOG_TAG" reset_info, error index=%d", 1, index);
        return;
    }

    app_lea_aird_client_info[index].att_handle_rx = BT_HANDLE_INVALID;
    app_lea_aird_client_info[index].att_handle_tx = BT_HANDLE_INVALID;
    app_lea_aird_client_info[index].att_handle_tx_cccd = BT_HANDLE_INVALID;

    app_lea_aird_client_info[index].state = APP_LE_AUDIO_AIRD_CLIENT_STATE_IDLE;
    app_lea_aird_client_info[index].mode = APP_LE_AUDIO_AIRD_MODE_NORMAL;

    app_le_audio_aird_action_node_t *node = (app_le_audio_aird_action_node_t *)app_lea_aird_client_info[index].action_queue;
    uint8_t free_count = 0;
    while (node != NULL) {
        app_le_audio_aird_action_node_t *temp = (app_le_audio_aird_action_node_t *)node->next;
        vPortFree(node);
        node = temp;
        free_count++;
    }
    if (free_count > 0) {
        APPS_LOG_MSGID_E(LOG_TAG" reset_info, index=%d free_count=%d", 2, index, free_count);
    }
    app_lea_aird_client_info[index].action_queue = NULL;
}

void app_le_audio_aird_client_notify_dongle_media_state(bool suspend, uint8_t *param, uint32_t param_len)
{
    app_lea_aird_device_busy_flag = suspend;

    APPS_LOG_MSGID_I(LOG_TAG" notify_dongle_media_state, suspend=%d", 1, suspend);

    for (int i = 0; i < APP_LEA_MAX_CONN_NUM; i++) {
        bt_handle_t handle = app_lea_conn_mgr_get_handle(i);
        if (handle != BT_HANDLE_INVALID
            && APP_LE_AUDIO_AIRD_CLIENT_STATE_READY == app_lea_aird_client_info[i].state) {
            if (suspend) {
                app_le_audio_aird_client_send_action(handle, APP_LE_AUDIO_AIRD_ACTION_SET_DEVICE_BUSY, NULL, 0);
            } else {
                app_le_audio_aird_client_send_action(handle, APP_LE_AUDIO_AIRD_ACTION_RESET_DEVICE_BUSY, NULL, 0);
            }
        }
    }
}

void app_le_audio_aird_client_notify_block_stream(app_le_audio_aird_block_stream_t type)
{
    APPS_LOG_MSGID_I(LOG_TAG" notify_block_stream, type=%d", 1, type);
    if (type > APP_LE_AUDIO_AIRD_BLOCK_STREAM_ALL) {
        return;
    }

    app_le_audio_aird_action_block_stream_t param = {0};

    switch (type) {
        case APP_LE_AUDIO_AIRD_BLOCK_STREAM_NONE: {
            app_lea_aird_block_stream_flag = FALSE;
            break;
        }
        case APP_LE_AUDIO_AIRD_BLOCK_STREAM_ALL: {
            app_lea_aird_block_stream_flag = TRUE;
            break;
        }
        default:
            return;
    }

    param.type = type;

    for (int i = 0; i < APP_LEA_MAX_CONN_NUM; i++) {
        bt_handle_t handle = app_lea_conn_mgr_get_handle(i);
        if (handle != BT_HANDLE_INVALID &&
            APP_LE_AUDIO_AIRD_CLIENT_STATE_READY == app_lea_aird_client_info[i].state) {
            app_le_audio_aird_client_send_action(handle, APP_LE_AUDIO_AIRD_ACTION_BLOCK_STREAM,
                                                 &param, sizeof(app_le_audio_aird_action_block_stream_t));
        }
    }
}

#ifdef AIR_INFORM_CONNECTION_STATUS_ENABLE
void app_le_audio_aird_client_infom_connection_status(bool connected)
{
    bool success = FALSE;
    app_le_audio_aird_multi_point_status_t param = APP_LE_AUDIO_AIRD_MULTI_POINT_STATUS_DISCONNECTED;
    if (connected) {
        param = APP_LE_AUDIO_AIRD_MULTI_POINT_STATUS_CONNECTED;
    } else {
        uint32_t edr_num = bt_cm_get_acl_connected_device(NULL, 0);
        uint8_t lea_num = app_lea_conn_mgr_get_conn_num();
        if (edr_num > 0 || lea_num > 1) {
            APPS_LOG_MSGID_W(LOG_TAG"[Silence_Detection] infom_connection_status, no need inform disconnect edr_num=%d lea_num=%d",
                             2, edr_num, lea_num);
            return;
        }
    }

    for (int i = 0; i < APP_LEA_MAX_CONN_NUM; i++) {
        bt_handle_t handle = app_lea_conn_mgr_get_handle(i);
        if (handle != BT_HANDLE_INVALID &&
            APP_LE_AUDIO_AIRD_CLIENT_STATE_READY == app_lea_aird_client_info[i].state) {
            success = app_le_audio_aird_client_send_action(handle, APP_LE_AUDIO_AIRD_ACTION_UPDATE_MULTI_POINT_STATUS,
                                                           &param, sizeof(app_le_audio_aird_multi_point_status_t));
        }
    }

    APPS_LOG_MSGID_I(LOG_TAG"[Silence_Detection] infom_connection_status, param=%d success=%d",
                     2, param, success);
}
#endif

bool app_le_audio_aird_client_switch_device(bt_handle_t handle, bool active)
{
    bool success = app_le_audio_aird_client_send_action(handle, APP_LE_AUDIO_AIRD_ACTION_SWITCH_DEVICE,
                                                        &active, 1);
    APPS_LOG_MSGID_I(LOG_TAG" switch_device, handle=0x%04X active=%d success=%d", 3, handle, active, success);
    return success;
}

static bool app_le_audio_aird_client_is_support_fun(bt_handle_t handle, bool check_hid_call)
{
    app_le_audio_aird_client_info_t *info = app_le_audio_aird_client_get_info(handle);

    bool ret = (info != NULL && (check_hid_call ? (info->mode == APP_LE_AUDIO_AIRD_MODE_SUPPORT_HID_CALL) : (info->state != APP_LE_AUDIO_AIRD_CLIENT_STATE_IDLE)));
    if (!ret) {
        if (info == NULL) {
            APPS_LOG_MSGID_I(LOG_TAG" is_support, NO check_hid_call=%d handle=0x%04X->NULL info", 2, check_hid_call, handle);
        } else {
            APPS_LOG_MSGID_I(LOG_TAG" is_support, NO check_hid_call=%d state=%d mode=%d", 3, check_hid_call, info->state, info->mode);
        }
    }
    return ret;
}

bool app_le_audio_aird_client_is_support_hid_call(bt_handle_t handle)
{
    return app_le_audio_aird_client_is_support_fun(handle, TRUE);
}

bool app_le_audio_aird_client_is_support(bt_handle_t handle)
{
    return app_le_audio_aird_client_is_support_fun(handle, FALSE);
}

bool app_le_audio_aird_client_send_action(bt_handle_t handle, app_le_audio_aird_action_t action,
                                          void *param, uint32_t param_len)
{
    bool success = FALSE;
    app_le_audio_aird_action_node_t *node = NULL;
    uint16_t att_value_len = 0;
    bt_status_t bt_status = BT_STATUS_SUCCESS;
    app_le_audio_aird_client_info_t *info = NULL;

    if (handle == BT_HANDLE_INVALID || APP_LE_AUDIO_AIRD_ACTION_MAX <= action) {
        APPS_LOG_MSGID_E(LOG_TAG" send_action, error handle=0x%04X action=%d", 2, handle, action);
        goto exit;
    }

    info = app_le_audio_aird_client_get_info(handle);
    if (info == NULL) {
        //APPS_LOG_MSGID_E(LOG_TAG" send_action, not AIRD link handle=0x%04X", 1, handle);
        goto exit;
    }

    att_value_len = (param_len + 1);    /* att_value: |action (1 byte) | param (param_len bytes) | */
    node = (app_le_audio_aird_action_node_t *)pvPortMalloc(sizeof(app_le_audio_aird_action_node_t) + APP_LE_AUDIO_ATT_HDR_SIZE + att_value_len);
    if (node == NULL) {
        APPS_LOG_MSGID_E(LOG_TAG" send_action, malloc fail", 0);
        goto exit;
    }

    node->next = NULL;
    node->write_cmd.attribute_value_length = att_value_len;
    node->write_cmd.att_req = (bt_att_write_command_t *)(node->buf);
    node->write_cmd.att_req->opcode = BT_ATT_OPCODE_WRITE_COMMAND;
    node->write_cmd.att_req->attribute_handle = info->att_handle_rx;
    node->buf[APP_LE_AUDIO_ATT_VALUE_IDX] = action;
    if (NULL != param && 0 != param_len) {
        memcpy(&node->buf[APP_LE_AUDIO_ATT_VALUE_IDX + 1], param, param_len);
    }

    app_le_audio_aird_cilent_do_resend_action(handle, info);

    bt_status = bt_gattc_write_without_rsp(handle, FALSE, &(node->write_cmd));
    if (BT_STATUS_SUCCESS == bt_status) {
        success = TRUE;
        vPortFree(node);
        goto exit;
    }

    if (!app_le_audio_aird_client_add_action_node(info, node)) {
        APPS_LOG_MSGID_E(LOG_TAG" send_action, add node fail", 0);
        vPortFree(node);
    }

exit:
    APPS_LOG_MSGID_I(LOG_TAG" send_action, handle=0x%04X action=%d success=%d", 3, handle, action, success);
    return success;
}

void app_le_audio_aird_client_proc_ui_shell_event(uint32_t event_group,
                                                  uint32_t event_id,
                                                  void *extra_data,
                                                  size_t data_len)
{
    switch (event_group) {
        case EVENT_GROUP_UI_SHELL_BT:
            app_le_audio_aird_client_proc_bt_group(event_id, extra_data, data_len);
            break;
        case EVENT_GROUP_UI_SHELL_LE_AUDIO:
            app_le_audio_aird_client_proc_lea_group(event_id, extra_data, data_len);
            break;
#if defined(MTK_AWS_MCE_ENABLE) && defined(AIR_INFORM_CONNECTION_STATUS_ENABLE)
        case EVENT_GROUP_UI_SHELL_APP_INTERACTION:
            app_le_audio_aird_client_proc_interaction_group(event_id, extra_data, data_len);
            break;
#endif
    }
}

#endif  /* AIR_LE_AUDIO_ENABLE */
