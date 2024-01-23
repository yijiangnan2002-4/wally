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
#include "bt_avrcp.h"
#include "bt_a2dp.h"
#include "bt_type.h"
#include "bt_gap.h"
#include "syslog.h"
#include "stdbool.h"
#include "FreeRTOS.h"
#include "portable.h"
#include "stdlib.h"
#include "atci.h"
#include "bt_sink_srv_utils.h"
#include "bt_utils.h"

#define BT_SINK_SRV_AVRCP_BQB_STR_CMP(_cmd) (strncmp((_cmd), cmd, strlen(_cmd)) == 0)

typedef enum {
    BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_NULL,
    BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_PASS_TH,
    BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_LIST_SETTING,
    BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_SETTING,
    BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_SET_SETTING,
    BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_REG,
    BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_PLAY_STATUS,
    BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_ELEMENT,
    BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_CONTINE,
    BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_ABORT,
    BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_SET_ABSOLUTE_VOL,
    BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_FOLDER_ITEMS,
    BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_CAPABILITY
} bt_sink_srv_avrcp_bqb_exe_command_t;

typedef enum {
    BT_SINK_SRV_AVRCP_BQB_ITEM_CT_MCN_CB_BV_INVALID,
    BT_SINK_SRV_AVRCP_BQB_ITEM_CT_MCN_CB_BV_01_I,
    BT_SINK_SRV_AVRCP_BQB_ITEM_CT_MCN_CB_BV_02_I,
    BT_SINK_SRV_AVRCP_BQB_ITEM_CT_MCN_CB_BV_03_I,
    BT_SINK_SRV_AVRCP_BQB_ITEM_CT_MCN_CB_BV_04_I,
    BT_SINK_SRV_AVRCP_BQB_ITEM_CT_MCN_CB_BV_05_I,
    BT_SINK_SRV_AVRCP_BQB_ITEM_CT_MCN_CB_BV_06_I,
    BT_SINK_SRV_AVRCP_BQB_ITEM_CT_MCN_CB_BV_07_I,
    BT_SINK_SRV_AVRCP_BQB_ITEM_CT_MCN_CB_BV_08_I,
    BT_SINK_SRV_AVRCP_BQB_ITEM_CT_MCN_CB_BV_09_I,
    BT_SINK_SRV_AVRCP_BQB_ITEM_CT_MCN_NP_BV_01_I,
    BT_SINK_SRV_AVRCP_BQB_ITEM_CT_MCN_NP_BV_02_I,
    BT_SINK_SRV_AVRCP_BQB_ITEM_CT_MCN_NP_BV_03_I,
    BT_SINK_SRV_AVRCP_BQB_ITEM_CT_MCN_NP_BV_04_I,
    BT_SINK_SRV_AVRCP_BQB_ITEM_CT_MCN_NP_BV_05_I,
    BT_SINK_SRV_AVRCP_BQB_ITEM_CT_MCN_NP_BV_06_I,
    BT_SINK_SRV_AVRCP_BQB_ITEM_CT_MCN_NP_BV_07_I,
} bt_sink_srv_avrcp_bqb_item_id;

typedef struct {
    uint16_t uid_counter;
    uint64_t folder_uid;
    uint64_t item_uid;
    bt_avrcp_scope_t scope;
    bool is_set_player;
    bt_avrcp_direction_t direction;
    uint16_t player_id;
    uint8_t current_depth;
    bool is_playing;
    bool toggle;
    uint8_t folder_idx;
    bool found_item;
    uint8_t cmd_count;
    uint8_t last_folder_uid;
    bool empty_folder;
}bt_sink_srv_avrcp_bqb_browse_param_t;

typedef struct {
    bt_gap_connection_handle_t gap_conn_hd;
    uint32_t avrcp_hd;
    uint32_t a2dp_hd;
    bt_bd_addr_t pts_addr;
    bt_sink_srv_avrcp_bqb_exe_command_t exe_command;
    bt_sink_srv_avrcp_bqb_exe_command_t sub_exe_command;
    bt_avrcp_capability_types_t capability_type;
    bt_sink_srv_avrcp_bqb_item_id item_id;
    uint8_t register_event_id;
    uint8_t pth_op_id;
    bool is_packet_abort;
    bool is_browse_connect;
    bool is_avrcp_bqb_in_progress;
    bt_sink_srv_avrcp_bqb_browse_param_t browse_para;
} bt_sink_srv_avrcp_bqb_t;

static bt_sink_srv_avrcp_bqb_t bt_sink_srv_avrcp_bqb_data = {0};

static void bt_sink_srv_avrcp_bqb_copy_str_to_addr(uint8_t *addr, const char *str)
{
    unsigned int i, value;
    int using_long_format = 0;
    int using_hex_sign = 0;

    if (str[2] == ':' || str[2] == '-') {
        using_long_format = 1;
    }

    if (str[1] == 'x') {
        using_hex_sign = 2;
    }

    for (i=0; i<6; i++) {
        sscanf(str+using_hex_sign+i*(2+using_long_format), "%02x", &value);
        addr[5-i] = (uint8_t) value;
    }
}

static int32_t bt_sink_srv_avrcp_bqb_get_folder_items(uint32_t avrcp_hd, bt_avrcp_scope_t scope, uint32_t end_item)
{
    int32_t ret = 0;
    bt_avrcp_browse_get_folder_items_t param = {0};

    param.scope = scope;
    param.end_item = end_item;
    ret = bt_avrcp_browse_get_folder_items(avrcp_hd, &param);
    bt_sink_srv_report_id("[sink][music][avrcp][BQB] get_folder_items-avrcp_hd:0x%x, scope:0x%x, ret:0x%x", 3, avrcp_hd, scope, ret);
    return ret;
}

static int32_t bt_sink_srv_avrcp_bqb_change_path(uint32_t avrcp_hd, uint16_t uid_counter, uint64_t folder_uid, bt_avrcp_direction_t direction)
{
    int32_t ret = 0;
    bt_avrcp_browse_change_path_t param = {0};

    param.uid_counter = uid_counter;
    param.folder_uid = folder_uid;
    param.direction = direction;
    ret = bt_avrcp_browse_change_path(avrcp_hd, &param);
    bt_sink_srv_report_id("[sink][music][avrcp][BQB] change_path-avrcp_hd:0x%x, scope:0x%x, ret:0x%x",
        4, uid_counter, folder_uid, direction, ret);
    return ret;
}

static void bt_sink_srv_avrcp_bqb_command_execute(void)
{
    int32_t ret = 0;

    switch(bt_sink_srv_avrcp_bqb_data.exe_command) {
        case BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_PASS_TH:
        {
            if(bt_sink_srv_avrcp_bqb_data.pth_op_id != BT_AVRCP_OPERATION_ID_RESERVED)
            {
                bt_avrcp_send_pass_through_command(bt_sink_srv_avrcp_bqb_data.avrcp_hd,
                                                   bt_sink_srv_avrcp_bqb_data.pth_op_id,
                                                   BT_AVRCP_OPERATION_STATE_PUSH);
            }
            break;
        }
        case BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_LIST_SETTING:
        {
            bt_avrcp_list_app_setting_attributes(bt_sink_srv_avrcp_bqb_data.avrcp_hd);
            break;
        }
        case BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_SETTING:
        {
            bt_avrcp_get_app_setting_value_t setting = {0};
            setting.attribute_id = 0x01;
            bt_avrcp_get_app_setting_value(bt_sink_srv_avrcp_bqb_data.avrcp_hd, 1, &setting);
            break;
        }
        case BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_SET_SETTING:
        {
            bt_avrcp_app_setting_value_t  setting = {0};
            setting.attribute_id = 0x01;
            setting.value_id = 0x02;
            bt_avrcp_set_app_setting_value(bt_sink_srv_avrcp_bqb_data.avrcp_hd, 2, &setting);
            break;
        }
        case BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_REG:
        {
            bt_avrcp_register_notification(bt_sink_srv_avrcp_bqb_data.avrcp_hd, bt_sink_srv_avrcp_bqb_data.register_event_id, 0);
            break;
        }
        case BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_PLAY_STATUS:
        {
            bt_avrcp_get_play_status(bt_sink_srv_avrcp_bqb_data.avrcp_hd);
            break;
        }
        case BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_ELEMENT:
        {
            bt_avrcp_get_element_attributes_t element = {0};
            element.attribute_id = 0x01;
            bt_avrcp_get_element_attributes(bt_sink_srv_avrcp_bqb_data.avrcp_hd, 4, &element);
            break;
        }
        case BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_SET_ABSOLUTE_VOL:
        {
            bt_avrcp_set_absolute_volume(bt_sink_srv_avrcp_bqb_data.avrcp_hd, 0x70);
            break;
        }
        case BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_FOLDER_ITEMS:
        {
            bt_avrcp_scope_t scope = bt_sink_srv_avrcp_bqb_data.browse_para.scope;
            if (bt_sink_srv_avrcp_bqb_data.browse_para.is_set_player) {
                scope = BT_AVRCP_SCOPE_MEDIA_PLAYER_LIST;
                bt_sink_srv_avrcp_bqb_data.browse_para.is_set_player = false;

            }
            ret = bt_sink_srv_avrcp_bqb_get_folder_items(bt_sink_srv_avrcp_bqb_data.avrcp_hd, scope, 100);
            break;
        }
        case BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_CAPABILITY:
        {
            ret = bt_avrcp_get_capability(bt_sink_srv_avrcp_bqb_data.avrcp_hd, bt_sink_srv_avrcp_bqb_data.capability_type);
        }
        default:
            break;
    }
    
    bt_sink_srv_report_id("[sink][music][avrcp][BQB] command_execute-exe_command:0x%x, ret:0x%x", 2, bt_sink_srv_avrcp_bqb_data.exe_command, ret);
}

static int32_t bt_sink_srv_avrcp_bqb_event_notification_ind(bt_avrcp_event_notification_t *noti_ind, bt_status_t status)
{
    int32_t ret = 0;

    bt_sink_srv_report_id("[sink][music][avrcp][BQB] event_notification_ind", 0);
    if (noti_ind && noti_ind->event_id == BT_AVRCP_EVENT_UIDS_CHANGED) {
        bt_sink_srv_avrcp_bqb_data.browse_para.uid_counter = noti_ind->uid_counter;

#if 0
        if (bt_sink_srv_avrcp_bqb_data.exe_command == BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_REG &&
            bt_sink_srv_avrcp_bqb_data.sub_exe_command == BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_FOLDER_ITEMS)
        {
            bt_sink_srv_avrcp_bqb_data.exe_command = bt_sink_srv_avrcp_bqb_data.sub_exe_command;
        }
#endif
        bt_sink_srv_avrcp_bqb_command_execute();
#if 0
        if (bt_sink_srv_avrcp_bqb_data.exe_command == BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_FOLDER_ITEMS) {
            ret = bt_sink_srv_avrcp_bqb_get_folder_items(noti_ind->handle, bt_sink_srv_avrcp_bqb_data.browse_para.scope, 100);
        }
#endif
    }
    return ret;
}

static int32_t bt_sink_srv_avrcp_bqb_browse_connect_ind(bt_avrcp_browse_connect_ind_t *cnf, bt_status_t status)
{
    int32_t ret = 0;
    ret = bt_avrcp_browse_connect_response(cnf->handle, true);
    bt_sink_srv_report_id("[sink][music][avrcp][BQB] browse_connect_ind-ret:0x%x", 1, ret);
    return ret;
}

static int32_t bt_sink_srv_avrcp_bqb_browse_connect_cnf(bt_avrcp_browse_connect_cnf_t *cnf, bt_status_t status)
{
    int32_t ret = 0;

    if (status == BT_STATUS_SUCCESS) {
        bt_sink_srv_avrcp_bqb_command_execute(); 
    }
#if 0
    if (bt_sink_srv_avrcp_bqb_data.action == BT_SINK_SRV_ACTION_CT_CON_BV_03_C || bt_sink_srv_avrcp_bqb_data.action == BT_SINK_SRV_ACTION_TG_CON_BV_04_C) {
        ret = bt_gap_disconnect(bt_sink_srv_avrcp_bqb_data.conn_hd);
    } else if (bt_sink_srv_avrcp_bqb_data.action == BT_SINK_SRV_ACTION_CT_MCN_CB_BV_05_I) {
        ret = bt_avrcp_register_notification(bt_sink_srv_avrcp_bqb_data.avrcp_hd, BT_AVRCP_EVENT_UIDS_CHANGED, 0);
    } else if ((bt_sink_srv_avrcp_bqb_data.action >= BT_SINK_SRV_ACTION_CT_MCN_CB_BV_01_C && bt_sink_srv_avrcp_bqb_data.action <=  BT_SINK_SRV_ACTION_CT_NFY_BV_01_C) || 
                bt_sink_srv_avrcp_bqb_data.action == BT_SINK_SRV_ACTION_CONNECT_TO_IPHONE || bt_sink_srv_avrcp_bqb_data.action == BT_SINK_SRV_ACTION_DISC_N_BROWSE){
        ret = bt_sink_srv_avrcp_bqb_get_folder_items(cnf->handle, BT_AVRCP_SCOPE_MEDIA_PLAYER_LIST, 100);
    } else if (bt_sink_srv_avrcp_bqb_data.action == BT_SINK_SRV_ACTION_CONNECT_TO_ANDROID) {
        ret = bt_sink_srv_avrcp_bqb_get_folder_items(cnf->handle, BT_AVRCP_SCOPE_MEDIA_PLAYER_VIRTUAL_FILESYSTEM, 100);
    }
#endif
    bt_sink_srv_report_id("[sink][music][avrcp][BQB] browse_connect_cnf-ret:0x%x", 1, ret);
    return ret;
}
#if 0
static int32_t bt_sink_srv_avrcp_bqb_browse_disconnect_ind(bt_avrcp_browse_disconnect_ind_t *ind, bt_status_t status)
{
    int32_t ret = 0;
    if (bt_sink_srv_avrcp_bqb_data.action == BT_SINK_SRV_ACTION_CT_CON_BV_03_C) {
        ret = bt_avrcp_disconnect(bt_sink_srv_avrcp_bqb_data.avrcp_hd);
    }
    bt_sink_srv_report_id("[sink][avrcp][BQB]browse_disconnect_ind-ret:0x%x", 1, ret);
    return ret;
}
#endif

static int32_t bt_sink_srv_avrcp_bqb_get_folder_items_cnf(bt_avrcp_browse_get_folder_items_cnf_t *cnf, bt_status_t status)
{
    int32_t ret = 0;
    bool change_path = 0;
    bool get_item_attr = 0;
    uint32_t length = 0;
    uint8_t *name = NULL;
    bt_avrcp_get_folder_items_response_value_t *item = NULL;
    uint32_t num_items = 0;
    uint8_t folder_idx = 0;

    if (status != BT_STATUS_SUCCESS) {
        return ret;
    }

    num_items = cnf->number_of_items;    
    bt_sink_srv_avrcp_bqb_data.browse_para.uid_counter = cnf->uid_counter;
    item = cnf->items_list;
    if (item->item_type == BT_AVRCP_ITEM_TYPE_MEDIA_PLAYER_ITEM) {
        if (num_items == 0) {
            ret = bt_sink_srv_avrcp_bqb_get_folder_items(cnf->handle, BT_AVRCP_SCOPE_MEDIA_PLAYER_VIRTUAL_FILESYSTEM, 100);
        } else {
            while(num_items--) {
                bt_sink_srv_report_id("[sink][music][avrcp][BQB] type:%x len:0x%04x id:0x%04x player type:%x status:%x display_len:0x%04x", 6, 
                    num_items, item->item_type, item->media_player_item.item_length, item->media_player_item.player_id, item->media_player_item.major_player_type, item->media_player_item.play_status, item->media_player_item.displayable_name_length);
                if (item->media_player_item.major_player_type & BT_AVRCP_MAJOR_PLAYER_TYPE_AUDIO || item->media_player_item.major_player_type & BT_AVRCP_MAJOR_PLAYER_TYPE_BROADCASTING_AUDIO) {
                    bt_sink_srv_avrcp_bqb_data.browse_para.player_id = item->media_player_item.player_id;
                    ret = bt_avrcp_browse_set_browsed_player(cnf->handle, item->media_player_item.player_id);
                    return ret;
                }
                item = (bt_avrcp_get_folder_items_response_value_t *)((uint8_t *)item + offsetof(bt_avrcp_media_player_item_t, player_id) + item->media_player_item.item_length);
            }
        }
    } else {
        while(num_items--) {
            if (item->item_type == BT_AVRCP_ITEM_TYPE_FOLDER_ITEM) {           
                bt_sink_srv_report_id("[sink][music][avrcp][BQB] get_folder_items_cnf-item type:%x name_len:0x%04x set:0x%04x", 3, num_items, item->folder_item.displayable_name_length, item->folder_item.character_set_id);
                length = item->folder_item.displayable_name_length;
                name = item->folder_item.displayable_name;
                while(length--) {
                    name++;
                }

                if ((bt_sink_srv_avrcp_bqb_data.browse_para.folder_idx == 0 || folder_idx == bt_sink_srv_avrcp_bqb_data.browse_para.folder_idx) && !change_path) {
                    ret = bt_sink_srv_avrcp_bqb_change_path(cnf->handle, cnf->uid_counter, item->folder_item.folder_uid, bt_sink_srv_avrcp_bqb_data.browse_para.direction);
                    change_path = true;
                    bt_sink_srv_avrcp_bqb_data.browse_para.folder_uid = item->folder_item.folder_uid;
                }
#if 0
                if (bt_sink_srv_avrcp_bqb_data.action == BT_SINK_SRV_ACTION_GET_FOLDER_ITEM) {
                    if (folder_idx == bt_sink_srv_avrcp_bqb_data.browse_para.folder_idx && !change_path) {
                        ret = bt_sink_srv_avrcp_bqb_change_path(cnf->handle, cnf->uid_counter, item->folder_item.folder_uid, bt_sink_srv_avrcp_bqb_data.browse_para.direction);
                        change_path = true;
      
                        bt_sink_srv_avrcp_bqb_data.browse_para.folder_uid = item->folder_item.folder_uid;
                    }
                } else {
                    if (!change_path) {
                        ret = bt_sink_srv_avrcp_bqb_change_path(cnf->handle, cnf->uid_counter, item->folder_item.folder_uid, bt_sink_srv_avrcp_bqb_data.browse_para.direction);
                        change_path = true;
      
                        bt_sink_srv_avrcp_bqb_data.browse_para.folder_uid = item->folder_item.folder_uid;
                    }
                }
#endif
                folder_idx++;

                if (change_path && get_item_attr) {
                    return ret;
                }
                item = (bt_avrcp_get_folder_items_response_value_t *)((uint8_t *)item + offsetof(bt_avrcp_folder_item_t, folder_uid) + item->folder_item.item_length);
            } else {
                uint8_t num_attr;
                bt_avrcp_attribute_value_entry_t *attr;
                uint32_t value_len;
                uint8_t *value;
                bt_sink_srv_report_id("[sink][music][avrcp][BQB] get_folder_items_cnf-num_items:0x%x name_len 0x%04x", 2, num_items, item->media_element_item.displayable_name_length);
                length = item->media_element_item.displayable_name_length;
                name = item->media_element_item.displayable_name;
                if (length < 50) {
                    while(length--) {
                        bt_sink_srv_report_id("[sink][music][avrcp][BQB] get_folder_items_cnf-name:0x%x", 1, *name);
                        name++;
                    }
                }
                if (!get_item_attr) {
                    bt_sink_srv_avrcp_bqb_data.browse_para.found_item = true;
                    bt_sink_srv_avrcp_bqb_data.browse_para.item_uid = item->media_element_item.media_element_uid;
                    if (bt_sink_srv_avrcp_bqb_data.item_id == BT_SINK_SRV_AVRCP_BQB_ITEM_CT_MCN_CB_BV_04_I) {
                        bt_avrcp_play_item_t param = {0};
                        param.scope = bt_sink_srv_avrcp_bqb_data.browse_para.scope;
                        param.uid = bt_sink_srv_avrcp_bqb_data.browse_para.item_uid;
                        param.uid_counter = bt_sink_srv_avrcp_bqb_data.browse_para.uid_counter;
                        ret = bt_avrcp_play_item(bt_sink_srv_avrcp_bqb_data.avrcp_hd, &param);
                    } else {
                        bt_avrcp_browse_get_item_attributes_t param;
                        param.scope = bt_sink_srv_avrcp_bqb_data.browse_para.scope;
                        param.uid = bt_sink_srv_avrcp_bqb_data.browse_para.item_uid;
                        param.uid_counter = bt_sink_srv_avrcp_bqb_data.browse_para.uid_counter;
                        param.num_of_attributes = 0;
                        ret = bt_avrcp_browse_get_item_attributes(cnf->handle, &param);
                        bt_sink_srv_report_id("[sink][music][avrcp][BQB] get_folder_items_cnf-get item attr %x", 1, ret);
                    }
                    get_item_attr = true;                    
                }                    

                attr = (bt_avrcp_attribute_value_entry_t *)((uint8_t *)item + offsetof(bt_avrcp_media_element_item_t, displayable_name) + item->media_element_item.displayable_name_length);
                num_attr = *(uint8_t *)attr;
                attr = (bt_avrcp_attribute_value_entry_t *)((uint8_t *)attr + 1);

                bt_sink_srv_report_id("[sink][music][avrcp][BQB] get_folder_items_cnf-num attr %x", 1, num_attr);
                while(num_attr--) {
                    bt_sink_srv_report_id("[sink][music][avrcp][BQB] get_folder_items_cnf-attr id:0x%08x value_len:0x%04x",
                        2, attr->id, attr->value_length);
                    value_len = attr->value_length;
                    value = attr->value;
                    if (value_len < 50) {
                        while(value_len--) {
                            bt_sink_srv_report_id("[sink][music][avrcp][BQB] get_folder_items_cnf-attr value: 0x%x",
                                1, *value);
                            value++;
                        }
                    }
                    attr = (bt_avrcp_attribute_value_entry_t *)((uint8_t *)attr + offsetof(bt_avrcp_attribute_value_entry_t, value) + attr->value_length);
                }
                
                if (change_path && get_item_attr) {
                    return ret;
                }
                item = (bt_avrcp_get_folder_items_response_value_t *)((uint8_t *)item + offsetof(bt_avrcp_media_element_item_t, media_element_uid) + item->media_element_item.item_length);
            }
        }
        bt_sink_srv_report_id("[sink][music][avrcp][BQB] get_folder_items_cnf-no folder item!!!", 0);
    }
        
    bt_sink_srv_report_id("[sink][music][avrcp][BQB] get_folder_items_cnf-uid_counter:0x%x, number_of_items:0x%x, size_of_item_list:0x%x, ret:0x%x", 4,
        cnf->uid_counter, cnf->number_of_items, cnf->size_of_item_list, ret);
    return ret;
}

static int32_t bt_sink_srv_avrcp_bqb_set_browsed_player_cnf(bt_avrcp_browse_set_browsed_player_cnf_t *cnf, bt_status_t status)
{
    int32_t ret = 0;
    bt_avrcp_folder_name_pair_t *folder_path = NULL;
    uint32_t folder_depth = 0;
    uint32_t length = 0;
    uint8_t *name = NULL;

    if (status != BT_STATUS_SUCCESS) {
        return ret;
    }

    folder_path = cnf->folder_path;
    folder_depth = cnf->folder_depth;
    bt_sink_srv_avrcp_bqb_data.browse_para.uid_counter = cnf->uid_counter;

    while(folder_depth--) {
        length = folder_path->length;
        name = &folder_path->name[0];
        while(length--) {
            bt_sink_srv_report_id("[sink][music][avrcp][BQB] set_browsed_player_cnf-name:0x%04x, length:0x%x", 1, *name, length);
            name++;
        }
        folder_path = (bt_avrcp_folder_name_pair_t *)((uint8_t *)folder_path + length);
    }
    ret = bt_sink_srv_avrcp_bqb_get_folder_items(cnf->handle, bt_sink_srv_avrcp_bqb_data.browse_para.scope, 100);

    bt_sink_srv_report_id("[sink][music][avrcp][BQB] browse_connect_cnf-uid_counter:0x%x, number_of_items:0x%x, size_of_item_list:0x%x, ret:0x%x", 4,
        cnf->uid_counter, cnf->number_of_items, cnf->folder_depth, ret);

    return ret;
}

static int32_t bt_sink_srv_avrcp_bqb_change_path_cnf(bt_avrcp_browse_change_path_cnf_t *cnf, bt_status_t status)
{
    int32_t ret = 0;

#if 0 /* Not sure which case need repeat mode */
    if (bt_sink_srv_avrcp_bqb_data.action == BT_SINK_SRV_ACTION_REPEAT_CMD) {
        if (bt_sink_srv_avrcp_bqb_data.browse_para.cmd_count) {
            ret = bt_sink_srv_avrcp_bqb_change_path(bt_sink_srv_avrcp_bqb_data.avrcp_hd, bt_sink_srv_avrcp_bqb_data.browse_para.uid_counter, 0xffffffffffffffff, BT_AVRCP_DIRECTION_UP);
            bt_sink_srv_avrcp_bqb_data.browse_para.cmd_count--;
        }
        return ret;
    }
#endif
    
    if (status != BT_STATUS_SUCCESS) {
        return ret;
    }

#if 0
    if (bt_sink_srv_avrcp_bqb_data.action == 0) {
        return ret;
    }
#endif

    if (bt_sink_srv_avrcp_bqb_data.browse_para.direction == BT_AVRCP_DIRECTION_UP) {
        return ret;
    }
    
    bt_sink_srv_report_id("[sink][music][avrcp][BQB] browse_connect_cnf-number_of_items:0x%x", 1,cnf->number_of_items);

    if (cnf->number_of_items) {
        ret = bt_sink_srv_avrcp_bqb_get_folder_items(cnf->handle,bt_sink_srv_avrcp_bqb_data.browse_para.scope,100);
    } else {
#if 0
        if (bt_sink_srv_avrcp_bqb_data.action >= BT_SINK_SRV_ACTION_CONNECT_TO_IPHONE) {
            if (bt_sink_srv_avrcp_bqb_data.browse_para.found_item == true) {
                ret = bt_avrcp_disconnect(cnf->handle);
            } else {/*
                ret = bt_sink_srv_avrcp_bqb_change_path(cnf->handle, bt_sink_srv_avrcp_bqb_data.browse_para.uid_counter, 0xffffffffffffffff, BT_AVRCP_DIRECTION_UP);
                bt_sink_srv_avrcp_bqb_data.browse_para.folder_idx++;*/
            }
        }
#endif
    }
    return ret;
}

static int32_t bt_sink_srv_avrcp_bqb_get_item_attributes_cnf(bt_avrcp_browse_get_item_attributes_cnf_t *cnf, bt_status_t status)
{
    int32_t ret = 0;
    uint32_t num = 0;
    bt_avrcp_attribute_value_entry_t *attr;
    uint32_t i = 0;
    uint8_t *value;

    if (status != BT_STATUS_SUCCESS) {
        return ret;
    }
    
    num = cnf->number_of_attributes;
    attr = cnf->attribute_value_entry_list;

    while(num--) {
        bt_sink_srv_report_id("[sink][music][avrcp][BQB] get_item_attributes_cnf-0x%08x char_set_id:0x%x len:0x%x",
            3, attr->id, attr->character_set_id, attr->value_length);
        i = attr->value_length;
        value = &attr->value[0];
        while(i--) {
            bt_sink_srv_report_id("[sink][music][avrcp][BQB] get_item_attributes_cnf-value:0x%x", 1, *value);
            value++;
        }
        attr = (bt_avrcp_attribute_value_entry_t *)((uint8_t *)attr + offsetof(bt_avrcp_attribute_value_entry_t, value) + attr->value_length);
    }

    if (BT_SINK_SRV_AVRCP_BQB_ITEM_CT_MCN_NP_BV_01_I == bt_sink_srv_avrcp_bqb_data.item_id) {
        bt_avrcp_play_item_t param = {0};
        param.scope = bt_sink_srv_avrcp_bqb_data.browse_para.scope;
        param.uid = bt_sink_srv_avrcp_bqb_data.browse_para.item_uid;
        param.uid_counter = bt_sink_srv_avrcp_bqb_data.browse_para.uid_counter;
        ret = bt_avrcp_play_item(bt_sink_srv_avrcp_bqb_data.avrcp_hd, &param);
    } else if (BT_SINK_SRV_AVRCP_BQB_ITEM_CT_MCN_CB_BV_02_I == bt_sink_srv_avrcp_bqb_data.item_id) {
        ret = bt_sink_srv_avrcp_bqb_change_path(cnf->handle,
            bt_sink_srv_avrcp_bqb_data.browse_para.uid_counter,
            bt_sink_srv_avrcp_bqb_data.browse_para.folder_uid,
            BT_AVRCP_DIRECTION_UP);
        bt_sink_srv_avrcp_bqb_data.item_id = BT_SINK_SRV_AVRCP_BQB_ITEM_CT_MCN_CB_BV_INVALID;
    }

    bt_sink_srv_report_id("[sink][music][avrcp][BQB] browse_connect_cnf-number_of_attributes:0x%x, item_id:0x%x, ret:0x%x", 3,
        cnf->number_of_attributes, bt_sink_srv_avrcp_bqb_data.item_id, ret);
    return ret;
}


static int32_t bt_sink_srv_avrcp_bqb_play_item_cnf(void)
{
    int32_t ret = 0;
    bt_sink_srv_avrcp_bqb_data.item_id = BT_SINK_SRV_AVRCP_BQB_ITEM_CT_MCN_CB_BV_INVALID;
    ret = bt_sink_srv_avrcp_bqb_get_folder_items(bt_sink_srv_avrcp_bqb_data.avrcp_hd, BT_AVRCP_SCOPE_NOW_PLAYING, 100);
    bt_sink_srv_report_id("[sink][music][avrcp][BQB] play_item_cnf-ret:0x%x", 1, ret);
    return ret;
}


static int32_t bt_sink_srv_avrcp_bqb_set_browsed_player_ind(bt_avrcp_browse_set_browsed_player_ind_t *ind, bt_status_t status)
{
    int32_t ret = 0;
    bt_avrcp_browse_set_browsed_player_response_t param = {0};
    
    if (ind->player_id == 0xffff) {
        param.status = BT_AVRCP_ERR_CODE_INVALID_PLAYER_ID;
    } else {
        param.status = BT_AVRCP_ERR_CODE_OPERATION_COMPLETED_WITHOUT_ERROR;
    }
    param.uid_counter = bt_sink_srv_avrcp_bqb_data.browse_para.uid_counter;
    param.number_of_items = 3;
    param.character_set_id = 0x006A;
    param.folder_depth = 0;
    param.size_of_folder_path = 0;
    ret = bt_avrcp_browse_set_browsed_player_response(ind->handle, &param);
    bt_sink_srv_report_id("[sink][music][avrcp][BQB] set_browsed_player_ind-ret:0x%x", 1, ret);
    return ret;
}

const uint8_t attr_1_value[8] = {'T','o','m','o','r','r','o','w'};
const uint8_t attr_1_value_new[8] = {'U','p','d','a','t','e','!','!'};

const bt_avrcp_attribute_value_entry_t attr_1 = {
    0x01,
    0x006A,
    0x0008,
    {0}
};
const uint8_t attr_2_value[7] = {'F','o','o',' ','B','a','r'};
const bt_avrcp_attribute_value_entry_t attr_2 = {
    0x02,
    0x006A,
    0x0007,
    {0}
};
const uint8_t attr_3_value[1] = {'1'};
const bt_avrcp_attribute_value_entry_t attr_3 = {
    0x04,
    0x006A,
    0x0001,
    {0}
};
const uint8_t item_name[] = {'T','o','m','o','r','r','o','w'};
const bt_avrcp_media_element_item_t item_1 = {
    BT_AVRCP_ITEM_TYPE_MEDIA_ELEMENT_ITEM,
    0x0035,
    0x07,
    BT_AVRCP_MEDIA_TYPE_AUDIO,
    0x006A,
    0x0008,
    {0},
};
const bt_avrcp_media_element_item_t item_2 = {
    BT_AVRCP_ITEM_TYPE_MEDIA_ELEMENT_ITEM,
    0x0035,
    0x08,
    BT_AVRCP_MEDIA_TYPE_AUDIO,
    0x006A,
    0x0008,
    {0},
};

static int32_t bt_sink_srv_avrcp_bqb_get_folder_items_ind(bt_avrcp_browse_get_folder_items_ind_t *ind, bt_status_t status)
{
    int32_t ret = 0;
    bt_avrcp_browse_get_folder_items_response_t param = {0};    
    uint8_t *dataPtr = NULL;
    uint8_t *dataPtrBackup = NULL;
    
    
    if (status != BT_STATUS_SUCCESS) {
        return ret;
    }

    param.status = BT_AVRCP_ERR_CODE_OPERATION_COMPLETED_WITHOUT_ERROR;
    param.uid_counter = bt_sink_srv_avrcp_bqb_data.browse_para.uid_counter;

    bt_sink_srv_report_id("[sink][music][avrcp][BQB] get_folder_items_ind-scope:0x%x", 1, ind->scope);
    switch (ind->scope) {
        case BT_AVRCP_SCOPE_MEDIA_PLAYER_LIST: {
            uint8_t name_1[] = {'m','i','n','1'};
            uint8_t name_2[] = {'p','l','a','y','e','r'};
            bt_avrcp_media_player_item_t player_1 = {
                BT_AVRCP_ITEM_TYPE_MEDIA_PLAYER_ITEM,
                0x0020,
                0x0001,
                BT_AVRCP_MAJOR_PLAYER_TYPE_AUDIO,
                BT_AVRCP_PLAYER_SUB_TYPE_AUDIO_BOOK,
                BT_AVRCP_PLAY_STATUS_STOPPED,
                {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x7f,
                0,0,0,0,0,0,0,0},
                0x006A,
                0x0004,
                {0},
                //name_1
            };
            bt_avrcp_media_player_item_t player_2 = {
                BT_AVRCP_ITEM_TYPE_MEDIA_PLAYER_ITEM,
                0x0022,
                0x0002,
                BT_AVRCP_MAJOR_PLAYER_TYPE_AUDIO,
                BT_AVRCP_PLAYER_SUB_TYPE_AUDIO_BOOK,
                BT_AVRCP_PLAY_STATUS_PAUSED,
                {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x77,
                0,0,0,0,0,0,0,0},
                0x006A,
                0x0006,
                {0},
                //name_2
            };
            if (ind->start_item >= 2) {
                param.status = BT_AVRCP_ERR_CODE_RANGE_OUT_OF_BOUNDS;
                param.number_of_items = 0;
                param.size_of_item_list = 0;
                ret = bt_avrcp_browse_get_folder_items_response(ind->handle, &param);
            } else {
                param.number_of_items = 2;
                param.size_of_item_list = offsetof(bt_avrcp_media_player_item_t, displayable_name) * 2 + 10;
                bt_sink_srv_report_id("[sink][music][avrcp][BQB] get_folder_items_ind-mem size 0x%x", param.size_of_item_list);
                dataPtr = bt_sink_srv_memory_alloc(param.size_of_item_list);
                if (dataPtr) {
                    dataPtrBackup = dataPtr;
                    param.item_list = (bt_avrcp_get_folder_items_response_value_t *)dataPtr;
                    bt_sink_srv_report_id("[sink][music][avrcp][BQB] get_folder_items_ind-cp 1 0x%x 0x%08x", 2,
                        offsetof(bt_avrcp_media_player_item_t, displayable_name), dataPtr);
                    memcpy((void *)dataPtr, (void *)&player_1, offsetof(bt_avrcp_media_player_item_t, displayable_name));
                    dataPtr += offsetof(bt_avrcp_media_player_item_t, displayable_name);
                    bt_sink_srv_report_id("[sink][music][avrcp][BQB] get_folder_items_ind-cp 2 0x%08x", 1, dataPtr);
                    memcpy((void *)dataPtr, (void *)name_1, 4);
                    dataPtr += 4;
                    bt_sink_srv_report_id("[sink][music][avrcp][BQB] get_folder_items_ind-cp 3 0x%08x", 1, dataPtr);
                    memcpy((void *)dataPtr, (void *)&player_2, offsetof(bt_avrcp_media_player_item_t, displayable_name));
                    dataPtr += offsetof(bt_avrcp_media_player_item_t, displayable_name);
                    bt_sink_srv_report_id("[sink][music][avrcp][BQB] get_folder_items_ind-cp 4 0x%08x", 1, dataPtr);
                    memcpy((void *)dataPtr, (void *)name_2, 6);
                    
                    ret = bt_avrcp_browse_get_folder_items_response(ind->handle, &param);
                } else {
                    bt_sink_srv_report_id("[sink][music][avrcp][BQB] get_folder_items_ind-null dataPtr", 0);
                }
            }
            break;
        }
        case BT_AVRCP_SCOPE_MEDIA_PLAYER_VIRTUAL_FILESYSTEM: {
            uint8_t folder_name[] = {'s','o','n','g','l','i','s','t','s'};
            bt_avrcp_folder_item_t folder_1 = {
                BT_AVRCP_ITEM_TYPE_FOLDER_ITEM,
                0x17,
                0x05,
                BT_AVRCP_FOLDER_TYPE_PLAYLISTS,
                BT_AVRCP_IS_PLAYABLE,
                0x006A,
                0x0009,
                {0},
            };            
            
            uint8_t folder_2_name[] = {'s','o','n','g','l','i','s','2','2'};
            bt_avrcp_folder_item_t folder_2 = {
                BT_AVRCP_ITEM_TYPE_FOLDER_ITEM,
                0x17,
                0x0A,
                BT_AVRCP_FOLDER_TYPE_PLAYLISTS,
                BT_AVRCP_IS_PLAYABLE,
                0x006A,
                0x0009,
                {0},
            };
            if (ind->start_item > ind->end_item || ind->start_item >= 3 || (bt_sink_srv_avrcp_bqb_data.browse_para.current_depth == 1 && (ind->start_item > 0 || bt_sink_srv_avrcp_bqb_data.browse_para.folder_uid == 0x0a))) {
                param.status = BT_AVRCP_ERR_CODE_RANGE_OUT_OF_BOUNDS;
                param.number_of_items = 0;
                param.size_of_item_list = 0;
            } else if (bt_sink_srv_avrcp_bqb_data.browse_para.current_depth == 1) {
                param.number_of_items = 1;
                param.size_of_item_list = offsetof(bt_avrcp_media_element_item_t, displayable_name) + item_1.displayable_name_length + 1 + offsetof(bt_avrcp_attribute_value_entry_t, value) * 2 + attr_1.value_length + attr_2.value_length;
                dataPtr = bt_sink_srv_memory_alloc(param.size_of_item_list);
                if (dataPtr) {
                    dataPtrBackup = dataPtr;
                    param.item_list = (bt_avrcp_get_folder_items_response_value_t *)dataPtr;
                    /* media_element_item */
                    memcpy(dataPtr, &item_1, offsetof(bt_avrcp_media_element_item_t, displayable_name));
                    dataPtr += offsetof(bt_avrcp_media_element_item_t, displayable_name);
                    memcpy(dataPtr, &item_name[0], item_1.displayable_name_length);
                    dataPtr += item_1.displayable_name_length;
                    /* number_of_attributes */
                    dataPtr[0] = 2;
                    dataPtr++;
                    /* attr_1 */
                    memcpy(dataPtr, &attr_1, offsetof(bt_avrcp_attribute_value_entry_t, value));
                    dataPtr += offsetof(bt_avrcp_attribute_value_entry_t, value);
                    memcpy(dataPtr, &attr_1_value[0], attr_1.value_length);
                    dataPtr += attr_1.value_length;
                    /* attr_2 */
                    memcpy(dataPtr, &attr_2, offsetof(bt_avrcp_attribute_value_entry_t, value));
                    dataPtr += offsetof(bt_avrcp_attribute_value_entry_t, value);
                    memcpy(dataPtr, &attr_2_value[0], attr_2.value_length);
                    dataPtr += attr_2.value_length;
                } else {
                    bt_sink_srv_report_id("[sink][music][avrcp][BQB] get_folder_items_ind-null dataPtr", 0);
                }
            } else {
                param.number_of_items = 3;
                param.size_of_item_list = offsetof(bt_avrcp_folder_item_t, displayable_name) * 2 + folder_1.displayable_name_length + offsetof(bt_avrcp_media_element_item_t, displayable_name) + item_1.displayable_name_length + 1 +
                    offsetof(bt_avrcp_attribute_value_entry_t, value) * 2 + attr_1.value_length + attr_2.value_length + folder_2.displayable_name_length;
                bt_sink_srv_report_id("[sink][music][avrcp][BQB] get_folder_items_ind-mem size 0x%x", 1, param.size_of_item_list);
                dataPtr = bt_sink_srv_memory_alloc(param.size_of_item_list);
                if (dataPtr) {
                    dataPtrBackup = dataPtr;
                    param.item_list = (bt_avrcp_get_folder_items_response_value_t *)dataPtr;
                    /* folder_item */
                    memcpy(dataPtr, &folder_1, offsetof(bt_avrcp_folder_item_t, displayable_name));
                    dataPtr += offsetof(bt_avrcp_folder_item_t, displayable_name);
                    memcpy(dataPtr, &folder_name[0], folder_1.displayable_name_length);
                    dataPtr += folder_1.displayable_name_length;
                    /* media_element_item */
                    memcpy(dataPtr, &item_1, offsetof(bt_avrcp_media_element_item_t, displayable_name));
                    dataPtr += offsetof(bt_avrcp_media_element_item_t, displayable_name);
                    memcpy(dataPtr, &item_name[0], item_1.displayable_name_length);
                    dataPtr += item_1.displayable_name_length;
                    /* number_of_attributes */
                    dataPtr[0] = 2;
                    dataPtr++;
                    /* attr_1 */
                    memcpy(dataPtr, &attr_1, offsetof(bt_avrcp_attribute_value_entry_t, value));
                    dataPtr += offsetof(bt_avrcp_attribute_value_entry_t, value);
                    memcpy(dataPtr, &attr_1_value[0], attr_1.value_length);
                    dataPtr += attr_1.value_length;
                    /* attr_2 */
                    memcpy(dataPtr, &attr_2, offsetof(bt_avrcp_attribute_value_entry_t, value));
                    dataPtr += offsetof(bt_avrcp_attribute_value_entry_t, value);
                    memcpy(dataPtr, &attr_2_value[0], attr_2.value_length);
                    dataPtr += attr_2.value_length;
                    /* folder_item */
                    memcpy(dataPtr, &folder_2, offsetof(bt_avrcp_folder_item_t, displayable_name));
                    dataPtr += offsetof(bt_avrcp_folder_item_t, displayable_name);
                    memcpy(dataPtr, &folder_2_name[0], folder_2.displayable_name_length);
                } else {
                    bt_sink_srv_report_id("[sink][music][avrcp][BQB] get_folder_items_ind-null dataPtr", 0);
                }
            }
            ret = bt_avrcp_browse_get_folder_items_response(ind->handle, &param);
            break;
        }
        case BT_AVRCP_SCOPE_NOW_PLAYING: {
            if (bt_sink_srv_avrcp_bqb_data.browse_para.is_playing && ind->start_item == 0) {
                param.number_of_items = 1;
                param.size_of_item_list = offsetof(bt_avrcp_media_element_item_t, displayable_name) + item_1.displayable_name_length + 1 + offsetof(bt_avrcp_attribute_value_entry_t, value)*2 + attr_1.value_length + attr_2.value_length;
                dataPtr = bt_sink_srv_memory_alloc(param.size_of_item_list);
                if (dataPtr) {
                    dataPtrBackup = dataPtr;
                    param.item_list = (bt_avrcp_get_folder_items_response_value_t *)dataPtr;
                    /* media_element_item */
                    memcpy(dataPtr, &item_2, offsetof(bt_avrcp_media_element_item_t, displayable_name));
                    dataPtr += offsetof(bt_avrcp_media_element_item_t, displayable_name);
                    memcpy(dataPtr, &item_name[0], item_1.displayable_name_length);
                    dataPtr += item_1.displayable_name_length;
                    /* number_of_attributes */
                    dataPtr[0] = 2;
                    dataPtr++;
                    /* attr_1 */
                    memcpy(dataPtr, &attr_1, offsetof(bt_avrcp_attribute_value_entry_t, value));
                    dataPtr += offsetof(bt_avrcp_attribute_value_entry_t, value);
                    memcpy(dataPtr, &attr_1_value[0], attr_1.value_length);
                    dataPtr += attr_1.value_length;
                    /* attr_2 */
                    memcpy(dataPtr, &attr_2, offsetof(bt_avrcp_attribute_value_entry_t, value));
                    dataPtr += offsetof(bt_avrcp_attribute_value_entry_t, value);
                    memcpy(dataPtr, &attr_2_value[0], attr_2.value_length);
                } else {
                    bt_sink_srv_report_id("[sink][music][avrcp][BQB] get_folder_items_ind-null dataPtr", 0);
                }
            } else {
                param.status = BT_AVRCP_ERR_CODE_RANGE_OUT_OF_BOUNDS;
                param.number_of_items = 0;
                param.size_of_item_list = 0;
            }
            ret = bt_avrcp_browse_get_folder_items_response(ind->handle, &param);
            break;
        }
        default:
            break;
    }      
    
    bt_sink_srv_memory_free(dataPtrBackup);
    return ret;
}

static int32_t bt_sink_srv_avrcp_bqb_change_path_ind(bt_avrcp_browse_change_path_ind_t *ind, bt_status_t status)
{
    int32_t ret = 0;
    bt_avrcp_browse_change_path_response_t param;

    if (status != BT_STATUS_SUCCESS) {
        return ret;
    }

    bt_sink_srv_report_id("[sink][music][avrcp][BQB] change_path_ind-direction:0x%x", 1, ind->direction);
    param.status = BT_AVRCP_ERR_CODE_OPERATION_COMPLETED_WITHOUT_ERROR;
    param.number_of_items = 0x03;
    if (ind->direction == BT_AVRCP_DIRECTION_DOWN) {
        bt_sink_srv_avrcp_bqb_data.browse_para.folder_uid = ind->folder_uid;
        if (bt_sink_srv_avrcp_bqb_data.browse_para.current_depth >= 3) {
            param.status = BT_AVRCP_ERR_CODE_DOSE_NOT_EXIST;
            param.number_of_items = 0;
            return bt_avrcp_browse_change_path_response(ind->handle, &param);
        }
        if (ind->folder_uid == 0xFFFFFFFFFFFFFFFF) {
            param.status = BT_AVRCP_ERR_CODE_NOT_A_DIRECTION;
            param.number_of_items = 0;
            return bt_avrcp_browse_change_path_response(ind->handle, &param);
        } else {
            bt_sink_srv_avrcp_bqb_data.browse_para.current_depth++;
            if (bt_sink_srv_avrcp_bqb_data.browse_para.current_depth == 1) {
                if (bt_sink_srv_avrcp_bqb_data.browse_para.empty_folder == true && ind->folder_uid == 0x0a) {
                    param.number_of_items = 0;
                } else {
                    param.number_of_items = 1;
                }
            }
            return bt_avrcp_browse_change_path_response(ind->handle, &param);
        }
    } else {
        if (bt_sink_srv_avrcp_bqb_data.browse_para.current_depth == 0) {
            param.status = BT_AVRCP_ERR_CODE_DOSE_NOT_EXIST;
            param.number_of_items = 0;
            return bt_avrcp_browse_change_path_response(ind->handle, &param);
        } else {
            bt_sink_srv_avrcp_bqb_data.browse_para.current_depth--;
            return bt_avrcp_browse_change_path_response(ind->handle, &param);
        }
    }
}


static int32_t bt_sink_srv_avrcp_bqb_pass_through_command_ind(bt_avrcp_pass_through_command_ind_t *ind, bt_status_t status)
{
    return bt_avrcp_send_pass_through_response(ind->handle, BT_AVRCP_RESPONSE_ACCEPTED, ind->op_id, ind->op_state);
}

static int32_t bt_sink_srv_avrcp_bqb_play_item_ind(bt_avrcp_play_item_command_t *ind, bt_status_t status)
{
    if (ind->uid == 0xFFFFFFFFFFFFFFFF) {
        return bt_avrcp_send_play_item_response(ind->handle, BT_AVRCP_ERR_CODE_DOSE_NOT_EXIST);
    } else {
        bt_sink_srv_avrcp_bqb_data.browse_para.is_playing = true;
        return bt_avrcp_send_play_item_response(ind->handle, BT_AVRCP_ERR_CODE_OPERATION_COMPLETED_WITHOUT_ERROR);
    }
}

static int32_t bt_sink_srv_avrcp_bqb_register_notification_ind(bt_avrcp_register_notification_commant_t *ind, bt_status_t status)
{
    int32_t ret = 0;
    bt_avrcp_send_register_notification_response_t rsp;

    bt_sink_srv_report_id("[sink][music][avrcp][BQB] register_notification_ind-null dataPtr", 1, ind->event_id);
    if (ind->event_id == BT_AVRCP_EVENT_UIDS_CHANGED) {
        bt_sink_srv_avrcp_bqb_data.browse_para.uid_counter = 0;
        rsp.response_type = BT_AVRCP_RESPONSE_INTERIM;
        rsp.parameter_length = 3;
        rsp.event_id = ind->event_id;
        rsp.uid_counter = bt_sink_srv_avrcp_bqb_data.browse_para.uid_counter;
        ret = bt_avrcp_send_register_notification_response(ind->handle, &rsp);
    } else if (ind->event_id == BT_AVRCP_EVENT_NOW_PLAYING_CONTENT_CHANGED) {
        rsp.response_type = BT_AVRCP_RESPONSE_INTERIM;
        rsp.parameter_length = 1;
        rsp.event_id = ind->event_id;
        ret = bt_avrcp_send_register_notification_response(ind->handle, &rsp);

        rsp.response_type = BT_AVRCP_RESPONSE_CHANGED;
        ret = bt_avrcp_send_register_notification_response(ind->handle, &rsp);

        bt_sink_srv_avrcp_bqb_data.browse_para.is_playing = true;
    } else if (ind->event_id == BT_AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED) {
        rsp.response_type = BT_AVRCP_RESPONSE_INTERIM;
        rsp.parameter_length = 5;
        rsp.event_id = ind->event_id;
        rsp.addressed_player.player_id = 1;
        rsp.addressed_player.uid_counter = 0;
        ret = bt_avrcp_send_register_notification_response(ind->handle, &rsp);
    } else if (ind->event_id == BT_AVRCP_EVENT_VOLUME_CHANGED) {
        rsp.response_type = BT_AVRCP_RESPONSE_INTERIM;
        rsp.parameter_length = 0x02; /* event id(1) + volume(1) */
        rsp.event_id = ind->event_id;
        rsp.volume = 0x20;
        bt_avrcp_send_register_notification_response(ind->handle, &rsp);

    } else if (ind->event_id == BT_AVRCP_EVENT_TRACK_CHANGED) {

        rsp.response_type = BT_AVRCP_RESPONSE_INTERIM;
        rsp.parameter_length = 9;
        rsp.event_id = ind->event_id;
        rsp.id = item_2.media_element_uid;
        ret = bt_avrcp_send_register_notification_response(ind->handle, &rsp);
    }
    
    return ret;
}

static int32_t bt_sink_srv_avrcp_bqb_get_item_attributes_ind(bt_avrcp_browse_get_item_attributes_ind_t *ind, bt_status_t status)
{
    int32_t ret = 0;
    bt_avrcp_browse_get_item_attributes_response_t param;
    uint32_t *attr = ind->attribute_list;
    uint8_t *dataPtr = NULL;
    uint8_t *dataPtrBackup = NULL;
    uint32_t num_attr = ind->num_of_attributes;

    if (status != BT_STATUS_SUCCESS) {
        return ret;
    }

    bt_sink_srv_report_id("[sink][music][avrcp][BQB] get_item_attributes_ind-uid_counter:0x%04x, uid_counter: 0x%04x, num_attr:0x%x", 
        2, ind->uid_counter, bt_sink_srv_avrcp_bqb_data.browse_para.uid_counter, num_attr);
    if (ind->uid_counter != bt_sink_srv_avrcp_bqb_data.browse_para.uid_counter) {
        param.status = BT_AVRCP_ERR_CODE_UID_CHANGED;
    } else {
        param.status = BT_AVRCP_ERR_CODE_OPERATION_COMPLETED_WITHOUT_ERROR;
        param.number_of_attributes = 0;
        param.size_of_attribute_list = 0;
        if (num_attr) {
            while (num_attr--) {
                if (*attr == 0x01) {
                    param.number_of_attributes++;
                    param.size_of_attribute_list += (offsetof(bt_avrcp_attribute_value_entry_t, value) + attr_1.value_length);
                } else if (*attr == 0x02) {
                    param.number_of_attributes++;
                    param.size_of_attribute_list += (offsetof(bt_avrcp_attribute_value_entry_t, value) + attr_2.value_length);
                } else if (*attr == 0x04) {
                    param.number_of_attributes++;
                    param.size_of_attribute_list += (offsetof(bt_avrcp_attribute_value_entry_t, value) + attr_3.value_length);
                }
                attr++;
            }

            dataPtr = bt_sink_srv_memory_alloc(param.size_of_attribute_list);
            bt_utils_assert(dataPtr);
            if (dataPtr) {
                dataPtrBackup = dataPtr;
                param.attribute_value_entry_list = (bt_avrcp_attribute_value_entry_t *)dataPtr;
                num_attr = param.number_of_attributes;
                attr = ind->attribute_list;
                while(num_attr--) {
                    if (*attr == 0x01) {
                        memcpy(dataPtr, &attr_1, offsetof(bt_avrcp_attribute_value_entry_t, value));
                        dataPtr += offsetof(bt_avrcp_attribute_value_entry_t, value);
                        if (bt_sink_srv_avrcp_bqb_data.browse_para.toggle) {
                            bt_sink_srv_avrcp_bqb_data.browse_para.toggle = false;
                            memcpy(dataPtr, &attr_1_value[0], attr_1.value_length);
                        } else {
                            bt_sink_srv_avrcp_bqb_data.browse_para.toggle = true;
                            memcpy(dataPtr, &attr_1_value_new[0], attr_1.value_length);
                        }
                        dataPtr += attr_1.value_length;
                    } else if (*attr == 0x02) {
                        memcpy(dataPtr, &attr_2, offsetof(bt_avrcp_attribute_value_entry_t, value));
                        dataPtr += offsetof(bt_avrcp_attribute_value_entry_t, value);
                        memcpy(dataPtr, &attr_2_value[0], attr_2.value_length);
                        dataPtr += attr_2.value_length;
                    } else if (*attr == 0x04) {
                        memcpy(dataPtr, &attr_3, offsetof(bt_avrcp_attribute_value_entry_t, value));
                        dataPtr += offsetof(bt_avrcp_attribute_value_entry_t, value);
                        memcpy(dataPtr, &attr_3_value[0], attr_3.value_length);
                        dataPtr += attr_3.value_length;
                    }
                    attr++;
                }
            } 
        } else {
            param.size_of_attribute_list = offsetof(bt_avrcp_attribute_value_entry_t, value) * 2 + attr_1.value_length + attr_2.value_length;
            param.number_of_attributes = 2;
            dataPtr = bt_sink_srv_memory_alloc(param.size_of_attribute_list);
            bt_utils_assert(dataPtr);
            if (dataPtr) {
                dataPtrBackup = dataPtr;
                param.attribute_value_entry_list = (bt_avrcp_attribute_value_entry_t *)dataPtr;
                memcpy(dataPtr, &attr_1, offsetof(bt_avrcp_attribute_value_entry_t, value));
                dataPtr += offsetof(bt_avrcp_attribute_value_entry_t, value);
                memcpy(dataPtr, &attr_1_value[0], attr_1.value_length);
                dataPtr += attr_1.value_length;
                memcpy(dataPtr, &attr_2, offsetof(bt_avrcp_attribute_value_entry_t, value));
                dataPtr += offsetof(bt_avrcp_attribute_value_entry_t, value);
                memcpy(dataPtr, &attr_2_value[0], attr_2.value_length);
                dataPtr += attr_2.value_length;
            }
        }
    }
    
    ret = bt_avrcp_browse_get_item_attributes_response(ind->handle, &param);
    bt_sink_srv_memory_free(dataPtrBackup);
    return ret;
}

static int32_t bt_sink_srv_avrcp_bqb_element_metadata_ind(bt_avrcp_get_element_attribute_t *ind, bt_status_t status)
{
    int32_t ret = 0;
    bt_avrcp_get_element_attributes_response_t rsp = {0};
    uint8_t *dataPtr = NULL;
    uint8_t *dataPtrBackup = NULL;

    rsp.handle = ind->handle;
    rsp.packet_type = BT_AVRCP_METADATA_PACKET_TYPE_NON_FRAGMENT;
    rsp.length = offsetof(bt_avrcp_get_element_attributes_response_value_t, attribute_value) + attr_1.value_length + 1;
    rsp.number = 1;
    dataPtr = bt_sink_srv_memory_alloc(offsetof(bt_avrcp_get_element_attributes_response_value_t, attribute_value) + attr_1.value_length);
    if (dataPtr) {
        dataPtrBackup = dataPtr;
        rsp.attribute_list = (bt_avrcp_get_element_attributes_response_value_t *)dataPtr;
        memcpy(dataPtr, &attr_1, offsetof(bt_avrcp_get_element_attributes_response_value_t, attribute_value));
        dataPtr += offsetof(bt_avrcp_get_element_attributes_response_value_t, attribute_value);
        if (!bt_sink_srv_avrcp_bqb_data.browse_para.toggle) {
            memcpy(dataPtr, &attr_1_value[0], attr_1.value_length);
        } else {
            memcpy(dataPtr, &attr_1_value_new[0], attr_1.value_length);
        }
    }
    ret = bt_avrcp_element_metadata_attributes_response(ind->handle, &rsp);
    bt_sink_srv_memory_free(dataPtrBackup);
    bt_sink_srv_report_id("[sink][music][avrcp][BQB] element_metadata_ind-ret", 1, ret);
    return ret;
}

static int32_t bt_sink_srv_avrcp_bqb_get_total_number_of_items_ind(bt_avrcp_browse_get_total_number_of_items_ind_t *ind, bt_status_t status)
{
    int32_t ret = 0;
    bt_avrcp_browse_get_total_number_of_items_response_t param;

    param.status = BT_AVRCP_ERR_CODE_OPERATION_COMPLETED_WITHOUT_ERROR;
    param.uid_counter = bt_sink_srv_avrcp_bqb_data.browse_para.uid_counter;
    param.number_of_items = 3;
    ret = bt_avrcp_browse_get_total_number_of_items_response(ind->handle, &param);
    return ret;
}

static int32_t bt_sink_srv_avrcp_bqb_set_addressed_player_ind(bt_avrcp_set_addressed_player_command_t *ind, bt_status_t status)
{
    if (ind->player_id == 0xffff) {
        return bt_avrcp_send_set_addressed_player_response(ind->handle, BT_AVRCP_ERR_CODE_INVALID_PLAYER_ID);
    }
    return bt_avrcp_send_set_addressed_player_response(ind->handle, BT_AVRCP_ERR_CODE_OPERATION_COMPLETED_WITHOUT_ERROR);
}

bool bt_sink_srv_avrcp_bqb_in_progress(void)
{
    return bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress;
}

bt_status_t bt_sink_srv_avrcp_bqb_common_cb(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    bool is_case_end = false;
    int32_t ret = 0;
    bt_sink_srv_report_id("[sink][music][avrcp][BQB] bqb_common_cb-msg (0x%x), status(0x%08x)", 2, msg, status);

    switch (msg) {
        case BT_AVRCP_CONNECT_CNF:
        {
            bt_avrcp_connect_cnf_t *cnf = (bt_avrcp_connect_cnf_t *)buff;

            bt_sink_srv_report_id("[sink][music][avrcp][BQB] bqb_common_cb-result(0x%x), handle(0x%08x)", 2, status, cnf->handle);
            if (status != BT_STATUS_SUCCESS) {
                is_case_end = true;
                break;
            } else {
                if (bt_sink_srv_avrcp_bqb_data.avrcp_hd) {
                   if (cnf->handle != bt_sink_srv_avrcp_bqb_data.avrcp_hd) {
                        bt_sink_srv_report_id("[sink][music][avrcp][BQB] bqb_common_cb-wrongly device connected", 0);
                   }
                } else {
                    bt_sink_srv_avrcp_bqb_data.avrcp_hd = cnf->handle;
                }
            }
            if (bt_sink_srv_avrcp_bqb_data.is_browse_connect) {
                ret = bt_avrcp_browse_connect(&bt_sink_srv_avrcp_bqb_data.avrcp_hd, (const bt_bd_addr_t *)&bt_sink_srv_avrcp_bqb_data.pts_addr);
                bt_sink_srv_report_id("[sink][music][avrcp][BQB] bqb_common_cb-browse_connect ret:0x%08x", 1, ret);
            } else {
                bt_sink_srv_avrcp_bqb_command_execute();
            }
            break;
        }
        case BT_AVRCP_CONNECT_IND:
        {
            bt_avrcp_connect_ind_t *ind = (bt_avrcp_connect_ind_t *)buff;

#if 0
            if (bt_sink_srv_avrcp_bqb_data.avrcp_hd) {
                LOG_E(common, "[BQB_AVRCP]more connection happened");
                bt_avrcp_connect_response(ind->handle, false);
                is_case_end = true;
                break;
            } else {
                bt_sink_srv_avrcp_bqb_data.avrcp_hd = ind->handle;
            }
#endif
            bt_sink_srv_report_id("[sink][music][avrcp][BQB] bqb_common_cb-result(0x%x), handle(0x%08x)", 2, status, ind->handle);

            bt_avrcp_connect_response(ind->handle, true);
            memcpy((uint8_t *)&bt_sink_srv_avrcp_bqb_data.pts_addr, ind->address, sizeof(bt_bd_addr_t));
            break;
        }
        case BT_AVRCP_DISCONNECT_IND:
        {
            //bt_avrcp_disconnect_ind_t *ind = (bt_avrcp_disconnect_ind_t *)buff;
            is_case_end = true;

            bt_sink_srv_report_id("[sink][music][avrcp][BQB] bqb_common_cb-disconnect", 0);
            break;
        }
        case BT_AVRCP_PASS_THROUGH_CNF:
        {
            bt_avrcp_pass_through_cnf_t *cnf = (bt_avrcp_pass_through_cnf_t *)buff;
            bt_sink_srv_report_id("[sink][music][avrcp][BQB] bqb_common_cb-op_id(%x),state(%x)", 2, cnf->op_id, cnf->op_state);
            if (status != BT_STATUS_SUCCESS) {
                break;
            }

            if (cnf->op_state == BT_AVRCP_OPERATION_STATE_PUSH) {
                bt_avrcp_send_pass_through_command(cnf->handle,
                                                   cnf->op_id,
                                                   BT_AVRCP_OPERATION_STATE_RELEASED);
            }



#if 0
            if (cnf->op_id == BT_AVRCP_OPERATION_ID_PLAY) {
                if (cnf->op_state == BT_AVRCP_OPERATION_STATE_PUSH) {
                    bt_avrcp_send_pass_through_command(cnf->handle, 
                                                       BT_AVRCP_OPERATION_ID_PLAY,
                                                       BT_AVRCP_OPERATION_STATE_RELEASED);
                }
            } else if (cnf->op_id == BT_AVRCP_OPERATION_ID_PAUSE) {

                if (cnf->op_state == BT_AVRCP_OPERATION_STATE_PUSH) {
                    bt_avrcp_send_pass_through_command(cnf->handle, 
                                                       BT_AVRCP_OPERATION_ID_PAUSE,
                                                       BT_AVRCP_OPERATION_STATE_RELEASED);
                }
               
            } else if (cnf->op_id == BT_AVRCP_OPERATION_ID_STOP) {
                if (cnf->op_state == BT_AVRCP_OPERATION_STATE_PUSH) {
                    bt_avrcp_send_pass_through_command(bt_sink_srv_avrcp_bqb_data.avrcp_hd, 
                                                       BT_AVRCP_OPERATION_ID_STOP,
                                                       BT_AVRCP_OPERATION_STATE_RELEASED);
                } 

            } else if (cnf->op_id == BT_AVRCP_OPERATION_ID_VOLUME_UP) {
                if (cnf->op_state == BT_AVRCP_OPERATION_STATE_PUSH) {
                    bt_avrcp_send_pass_through_command(bt_sink_srv_avrcp_bqb_data.avrcp_hd,
                                                       BT_AVRCP_OPERATION_ID_VOLUME_UP,
                                                       BT_AVRCP_OPERATION_STATE_RELEASED);
                }
            } else if (cnf->op_id == BT_AVRCP_OPERATION_ID_VOLUME_DOWN) {
                if (cnf->op_state == BT_AVRCP_OPERATION_STATE_PUSH) {
                    bt_avrcp_send_pass_through_command(bt_sink_srv_avrcp_bqb_data.avrcp_hd,
                                                       BT_AVRCP_OPERATION_ID_VOLUME_DOWN,
                                                       BT_AVRCP_OPERATION_STATE_RELEASED);
                }
            } else if (cnf->op_id == BT_AVRCP_OPERATION_ID_0){
                if (cnf->op_state == BT_AVRCP_OPERATION_STATE_PUSH) {
                ret = bt_avrcp_send_pass_through_command(bt_sink_srv_avrcp_bqb_data.avrcp_hd, BT_AVRCP_OPERATION_ID_0, BT_AVRCP_OPERATION_STATE_RELEASED);
                } else {
                    ret = bt_avrcp_send_pass_through_command(bt_sink_srv_avrcp_bqb_data.avrcp_hd, BT_AVRCP_OPERATION_ID_6, BT_AVRCP_OPERATION_STATE_PUSH);
                }
            } else if (cnf->op_id == BT_AVRCP_OPERATION_ID_6) {
                if (cnf->op_state == BT_AVRCP_OPERATION_STATE_PUSH) {
                    ret = bt_avrcp_send_pass_through_command(bt_sink_srv_avrcp_bqb_data.avrcp_hd, BT_AVRCP_OPERATION_ID_6, BT_AVRCP_OPERATION_STATE_RELEASED);
                }
            }
#endif

            break;
        }
        case BT_AVRCP_GET_CAPABILITY_CNF:
        {
            bt_avrcp_get_capability_response_t *cnf = (bt_avrcp_get_capability_response_t *)buff;

            for (uint32_t i = 0; i < cnf->capability_count; i++) {
                if (cnf->type == BT_AVRCP_CAPABILITY_COMPANY_ID) {
                    bt_sink_srv_report_id("[sink][music][avrcp][BQB] bqb_common_cb-company_id, {%x, %x, %x}", 3, cnf->company_id[i][0], cnf->company_id[i][1], cnf->company_id[i][2]);
                } else if (cnf->type == BT_AVRCP_CAPABILITY_EVENTS_SUPPORTED) {
                    bt_sink_srv_report_id("[sink][music][avrcp][BQB] bqb_common_cb-event[%d] %d", 2, i, cnf->event[i]);
                }
            }
            break;


        }
        case BT_AVRCP_GET_ELEMENT_ATTRIBUTES_CNF:
        {
            bt_avrcp_get_element_attributes_response_t *cnf = (bt_avrcp_get_element_attributes_response_t *)buff;
            static uint32_t count = 0;

            bt_sink_srv_report_id("[sink][music][avrcp][BQB] bqb_common_cb-get elemnt pack type(%d), count(%d)", 2, cnf->packet_type, count);
            if (status != BT_STATUS_SUCCESS) {
                break;
            }

            if (cnf->packet_type == BT_AVRCP_METADATA_PACKET_TYPE_START ||
                cnf->packet_type == BT_AVRCP_METADATA_PACKET_TYPE_CONTINUE) {

                if (bt_sink_srv_avrcp_bqb_data.is_packet_abort == true) {
                    if (count > 4) {
                        bt_avrcp_abort_continuing_response(cnf->handle, BT_AVRCP_PDU_ID_GET_ELEMENT_ATTRIBUTES);
                        count = 0;
                        bt_sink_srv_avrcp_bqb_data.is_packet_abort = false;
                        break;
                    }
                } 

                bt_avrcp_request_continuing_response(cnf->handle, BT_AVRCP_PDU_ID_GET_ELEMENT_ATTRIBUTES);
                count++;
            }

            break;
        }
        case BT_AVRCP_ABORT_CONTINUING_CNF:
        {
            /* Just for protect  */
            bt_sink_srv_avrcp_bqb_data.is_packet_abort = false;
            break;
        }
        case BT_GAP_LINK_STATUS_UPDATED_IND:
        {
            bt_gap_link_status_updated_ind_t * update_ind = (bt_gap_link_status_updated_ind_t *)buff;
            if (update_ind->link_status >= BT_GAP_LINK_STATUS_CONNECTED_0) {
                bt_sink_srv_avrcp_bqb_data.gap_conn_hd = update_ind->handle;
            }
            break;
        }
        case BT_AVRCP_SET_ABSOLUTE_VOLUME_CNF:
        {
            bt_avrcp_set_absolute_volume_response_t *cnf = (bt_avrcp_set_absolute_volume_response_t *)buff;
            bt_sink_srv_report_id("[sink][music][avrcp][BQB] bqb_common_cb-Set Absolute Volume Response: handle(0x%08x) volume(0x%x)", 2, cnf->handle, cnf->volume);
            break;
        }
#if 0
        case BT_AVRCP_PASS_THROUGH_COMMAND_IND:
        {
            bt_avrcp_pass_through_command_ind_t *pass_cmd =
                                                (bt_avrcp_pass_through_command_ind_t *)buff;
            bt_sink_srv_report_id("[sink][avrcp][BQB]bqb_common_cb: result(0x%x), handle(0x%08x), op_id(0x%x), op_state(0x%x)", 4, status, pass_cmd->handle, pass_cmd->op_id, pass_cmd->op_state);

            if (pass_cmd->op_id == BT_AVRCP_OPERATION_ID_VOLUME_UP)
            {
                if (pass_cmd->op_state == BT_AVRCP_OPERATION_STATE_PUSH) {
                    bt_avrcp_send_pass_through_response(pass_cmd->handle, BT_AVRCP_RESPONSE_ACCEPTED, pass_cmd->op_id, pass_cmd->op_state);
                } else {
                    bt_avrcp_send_pass_through_response(pass_cmd->handle, BT_AVRCP_RESPONSE_ACCEPTED, pass_cmd->op_id, pass_cmd->op_state);
                }
            } else if (pass_cmd->op_id == BT_AVRCP_OPERATION_ID_VOLUME_DOWN) {
                if (pass_cmd->op_state == BT_AVRCP_OPERATION_STATE_PUSH) {
                    bt_avrcp_send_pass_through_response(pass_cmd->handle, BT_AVRCP_RESPONSE_ACCEPTED, pass_cmd->op_id, pass_cmd->op_state);
                } else {
                    bt_avrcp_send_pass_through_response(pass_cmd->handle, BT_AVRCP_RESPONSE_ACCEPTED, pass_cmd->op_id, pass_cmd->op_state);
                }
            } else {
                // Not expected
            }
            break;
        }
#endif
        case BT_AVRCP_SET_ABSOLUTE_VOLUME_COMMAND_IND:
        {
            bt_avrcp_set_absolute_volume_event_t *set_vol = (bt_avrcp_set_absolute_volume_event_t *)buff;
            bt_sink_srv_report_id("[sink][music][avrcp][BQB] bqb_common_cb-set absolute volume command: result(0x%x), handle(0x%08x), volume(0x%x)", 
                3, status, set_vol->handle, set_vol->volume);

            /* To do: Adjust system volume here */
            bt_avrcp_send_set_absolute_volume_response(set_vol->handle, set_vol->volume);

            break;
        }
#if 0
        case BT_AVRCP_REGISTER_NOTIFICATION_IND:
        {
            bt_avrcp_send_register_notification_response_t reg_noti_rsp;
            bt_avrcp_register_notification_commant_t *reg_noti_cmd =
                                                     (bt_avrcp_register_notification_commant_t *)buff;
            bt_sink_srv_report_id("[sink][avrcp][BQB]Receive register notification command: result(0x%x), handle(0x%08x), event_id(0x%x), parameter_length(%x)", 4, status, reg_noti_cmd->handle, reg_noti_cmd->event_id, reg_noti_cmd->parameter_length);

            if (reg_noti_cmd->event_id == BT_AVRCP_EVENT_VOLUME_CHANGED) {
                reg_noti_rsp.response_type = BT_AVRCP_RESPONSE_INTERIM;
                reg_noti_rsp.parameter_length = 0x02; /* event id(1) + volume(1) */
                reg_noti_rsp.event_id = reg_noti_cmd->event_id;
                reg_noti_rsp.volume = 0x20;
                bt_avrcp_send_register_notification_response(reg_noti_cmd->handle, &reg_noti_rsp);
            } else {
                // Not expected.
            }
            break;
        }
#endif
        case BT_AVRCP_EVENT_NOTIFICATION_IND: {
            ret = bt_sink_srv_avrcp_bqb_event_notification_ind((bt_avrcp_event_notification_t *)buff, status);
            break;
        }
        case BT_AVRCP_BROWSE_CONNECT_IND:
        {
            ret = bt_sink_srv_avrcp_bqb_browse_connect_ind((bt_avrcp_browse_connect_ind_t *)buff, status);
            break;
        }
        case BT_AVRCP_BROWSE_CONNECT_CNF: {
            ret = bt_sink_srv_avrcp_bqb_browse_connect_cnf((bt_avrcp_browse_connect_cnf_t *)buff, status);
            break;
        }

        case BT_AVRCP_BROWSE_DISCONNECT_IND: {
            //ret = bt_sink_srv_avrcp_bqb_browse_disconnect_ind((bt_avrcp_browse_disconnect_ind_t *)buff, status);
            break;
        }

        case BT_AVRCP_BROWSE_GET_FOLDER_ITEMS_CNF: {
            ret = bt_sink_srv_avrcp_bqb_get_folder_items_cnf((bt_avrcp_browse_get_folder_items_cnf_t *)buff, status);
            break;
        }
            
        case BT_AVRCP_BROWSE_SET_BROWSED_PLAYER_CNF: {
            ret = bt_sink_srv_avrcp_bqb_set_browsed_player_cnf((bt_avrcp_browse_set_browsed_player_cnf_t *)buff, status);
            break;
        }

        case BT_AVRCP_BROWSE_CHANGE_PATH_CNF: {
            ret = bt_sink_srv_avrcp_bqb_change_path_cnf((bt_avrcp_browse_change_path_cnf_t *)buff, status);
            break;
        }

        case BT_AVRCP_BROWSE_GET_ITEM_ATTRIBUTES_CNF: {
            ret = bt_sink_srv_avrcp_bqb_get_item_attributes_cnf((bt_avrcp_browse_get_item_attributes_cnf_t *)buff, status);
            break;
        }

        case BT_AVRCP_PLAY_ITEM_CNF: {
            ret = bt_sink_srv_avrcp_bqb_play_item_cnf();
            break;
        }

        case BT_AVRCP_BROWSE_SET_BROWSED_PLAYER_IND: {
            ret = bt_sink_srv_avrcp_bqb_set_browsed_player_ind((bt_avrcp_browse_set_browsed_player_ind_t *)buff, status);
            break;
        }

        case BT_AVRCP_BROWSE_GET_FOLDER_ITEMS_IND: {
            ret = bt_sink_srv_avrcp_bqb_get_folder_items_ind((bt_avrcp_browse_get_folder_items_ind_t *)buff, status);
            break;
        }

        case BT_AVRCP_BROWSE_CHANGE_PATH_IND: {
            ret = bt_sink_srv_avrcp_bqb_change_path_ind((bt_avrcp_browse_change_path_ind_t *)buff, status);
            break;
        }

        case BT_AVRCP_BROWSE_GET_ITEM_ATTRIBUTES_IND: {
            ret = bt_sink_srv_avrcp_bqb_get_item_attributes_ind((bt_avrcp_browse_get_item_attributes_ind_t *)buff, status);
            break;
        }

        case BT_AVRCP_REGISTER_NOTIFICATION_IND: {
            ret = bt_sink_srv_avrcp_bqb_register_notification_ind((bt_avrcp_register_notification_commant_t *)buff, status);
            break;
        }

        case BT_AVRCP_ELEMENT_METADATA_IND: {
            ret = bt_sink_srv_avrcp_bqb_element_metadata_ind((bt_avrcp_get_element_attribute_t *)buff, status);
            break;
        }

        case BT_AVRCP_BROWSE_GET_TOTAL_NUMBER_OF_ITEMS_IND: {
            ret = bt_sink_srv_avrcp_bqb_get_total_number_of_items_ind((bt_avrcp_browse_get_total_number_of_items_ind_t *)buff, status);
            break;
        }

        case BT_AVRCP_PLAY_ITEM_COMMAND_IND: {
            ret = bt_sink_srv_avrcp_bqb_play_item_ind((bt_avrcp_play_item_command_t *)buff, status);
            break;
        }

#if 0
        case BT_AVRCP_PASS_THROUGH_CNF: {
            ret = bt_sink_srv_avrcp_bqb_pass_through_cnf((bt_avrcp_pass_through_cnf_t *)buff, status);
            break;
        }
#endif

        case BT_AVRCP_SET_ADDRESSED_PLAYER_COMMAND_IND: {
            ret = bt_sink_srv_avrcp_bqb_set_addressed_player_ind((bt_avrcp_set_addressed_player_command_t *)buff, status);
            bt_sink_srv_report_id("[sink][music][avrcp][BQB] bqb_common_cb-bt_avrcp_send_set_addressed_player_response 0x%08x", 1, ret);
            break;
        }

        case BT_AVRCP_PASS_THROUGH_COMMAND_IND: {
            ret= bt_sink_srv_avrcp_bqb_pass_through_command_ind((bt_avrcp_pass_through_command_ind_t *)buff, status);
            bt_sink_srv_report_id("[sink][music][avrcp][BQB] bqb_common_cb-bt_avrcp_send_pass_through_response 0x%08x", 1, ret);
            break;
        }

        case BT_AVRCP_GET_PLAY_STATUS_CNF: {
            bt_avrcp_media_play_status_response_t *play_status = (bt_avrcp_media_play_status_response_t *)buff;
            if (status != BT_STATUS_SUCCESS) {
                bt_sink_srv_report_id("[sink][music][avrcp][BQB] bqb_common_cb-get play status err 0x%08x", 1, status);
                break;
            }
            bt_sink_srv_report_id("[sink][music][avrcp][BQB] bqb_common_cb-BT_AVRCP_GET_PLAY_STATUS_CNF song_length %d, song_position %d, status %d, count:%d", 4, play_status->song_length, play_status->song_position, play_status->status, bt_sink_srv_avrcp_bqb_data.browse_para.cmd_count);
            if (bt_sink_srv_avrcp_bqb_data.browse_para.cmd_count) {
                bt_sink_srv_avrcp_bqb_data.browse_para.cmd_count--;
                ret = bt_avrcp_get_play_status(bt_sink_srv_avrcp_bqb_data.avrcp_hd);
                bt_sink_srv_report_id("[sink][music][avrcp][BQB] bqb_common_cb-get play status 0x%08x", 1, ret);
            }
                
            break;
        }

        default:
            break;
    }

    if (is_case_end) {
        /* clean callback  & handle*/
        bt_sink_srv_report_id("[sink][music][avrcp][BQB] bqb_common_cb-case end 0x%08x", 1, ret);
        memset(&bt_sink_srv_avrcp_bqb_data, 0, sizeof(bt_sink_srv_avrcp_bqb_data));
        bt_sink_srv_avrcp_bqb_data.pth_op_id = BT_AVRCP_OPERATION_ID_RESERVED;
    }
    return BT_STATUS_SUCCESS;

}

extern bt_a2dp_codec_capability_t g_bt_sink_srv_a2dp_codec_list;
static void BT_BQB_A2DP_SOURCE_MAKE_SBC_CODEC(bt_a2dp_codec_capability_t *codec,
                            bt_a2dp_role_t role,
                            uint8_t min_bit_pool, uint8_t max_bit_pool,
                            uint8_t block_length, uint8_t subband_num,
                            uint8_t alloc_method, uint8_t sample_rate,
                            uint8_t channel_mode)
{
    do {
        codec->type = BT_A2DP_CODEC_SBC;
        codec->sep_type = role;
        codec->length = sizeof(bt_a2dp_sbc_codec_t);
        codec->codec.sbc.channel_mode = (channel_mode & 0x0F);
        codec->codec.sbc.sample_freq = (sample_rate & 0x0F);
        codec->codec.sbc.alloc_method = (alloc_method & 0x03);
        codec->codec.sbc.subbands = (subband_num & 0x03);
        codec->codec.sbc.block_len = (block_length & 0x0F);
        codec->codec.sbc.min_bitpool = (min_bit_pool & 0xFF);
        codec->codec.sbc.max_bitpool = (max_bit_pool & 0xFF);
        codec->delay_report = 0;
        codec->sec_type = 0;
    } while (0);
}

bt_status_t bt_sink_srv_avrcp_bqb_a2dp_get_init_params(bt_a2dp_init_params_t *param)
{
    int32_t ret = BT_STATUS_FAIL;

    if (param) {
        bt_codec_sbc_t avrcp_bqb_sbc_cap[1] = {
            {
                25,  // min_bit_pool
                75,  // max_bit_pool
                0xf, // block_len: all
                0xf, // subband_num: all
                0x3, // both snr/loudness
                0x1, // sample_rate: all
                0xf  // channel_mode: all
            }
        };

        /* fill init params */
        param->codec_number = 1;
        BT_BQB_A2DP_SOURCE_MAKE_SBC_CODEC(&g_bt_sink_srv_a2dp_codec_list, BT_A2DP_SOURCE,
                               avrcp_bqb_sbc_cap[0].min_bit_pool, avrcp_bqb_sbc_cap[0].max_bit_pool,
                               avrcp_bqb_sbc_cap[0].block_length, avrcp_bqb_sbc_cap[0].subband_num,
                               avrcp_bqb_sbc_cap[0].alloc_method, avrcp_bqb_sbc_cap[0].sample_rate,
                               avrcp_bqb_sbc_cap[0].channel_mode);
        param->codec_list = &g_bt_sink_srv_a2dp_codec_list;
        param->sink_feature = 0x00;
        param->source_feature = 0x0F;
        //param->sink_delay = 1500;

        ret = BT_STATUS_SUCCESS;
    }
    return ret;
}

atci_status_t bt_sink_srv_avrcp_bqb_atci_callback(atci_parse_cmd_param_t *parse_cmd)
{
    const char *cmd = parse_cmd->string_ptr+strlen(BT_SINK_SRV_AVRCP_BQB_HEADER);
    uint32_t handle = 0;
    int32_t ret = 0;

    //printf("[sink][avrcp][BQB]txy avrcp_atci_callback: %s", cmd);
    bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
#ifdef BT_SINK_SRV_AVRCP_BQB_RESERVED_CASE
    if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_NFR_BV_04_C")) {
        /* make sure scan enable */
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_sink_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_LIST_SETTING;
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_CEC_BV_01_I_A2DP")) {
        bt_sink_srv_avrcp_bqb_copy_str_to_addr((uint8_t *)bt_sink_srv_avrcp_bqb_data.pts_addr, cmd + strlen("bt avrcp TC_CT_CEC_BV_01_I_A2DP "));
        bt_a2dp_connect(&handle, (const bt_bd_addr_t *)&bt_sink_srv_avrcp_bqb_data.pts_addr, BT_A2DP_SINK);
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_CEC_BV_01_I_AVRCP")) {
        bt_sink_srv_avrcp_bqb_copy_str_to_addr((uint8_t *)&bt_sink_srv_avrcp_bqb_data.pts_addr, cmd + strlen("bt avrcp TC_CT_CEC_BV_01_I_AVRCP "));
        bt_avrcp_connect(&handle, (const bt_bd_addr_t *)&bt_sink_srv_avrcp_bqb_data.pts_addr);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_CEC_BV_02_I")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_CFG_BV_01_I")) {
        bt_sink_srv_avrcp_bqb_data.capability_type = (bt_avrcp_capability_types_t)strtoul(cmd + strlen("bt avrcp TC_CT_CFG_BV-01_I"), NULL, 10);
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        bt_sink_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_CAPABILITY;
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_CRC_BV_01_I")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_CRC_BV_01_DIS")) {
        bt_gap_disconnect(bt_sink_srv_avrcp_bqb_data.gap_conn_hd);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_CRC_BV_02_I")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_NFY_BV_01_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_sink_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_REG;
        bt_sink_srv_avrcp_bqb_data.register_event_id = BT_AVRCP_EVENT_PLAYBACK_STATUS_CHANGED;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_PAS_BV_01_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_sink_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_LIST_SETTING;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_PAS_BV_09_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_sink_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_SETTING;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_PAS_BV_11_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_sink_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_SET_SETTING;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_MDI_BV_01_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_sink_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_PLAY_STATUS;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_MDI_BV_03_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_sink_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_ELEMENT;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_PTH_BV_01_C_PLAY")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_sink_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_PASS_TH;
        bt_sink_srv_avrcp_bqb_data.pth_op_id = BT_AVRCP_OPERATION_ID_PLAY;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_PTH_BV_01_C_PAUSE")) {
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_avrcp_send_pass_through_command(bt_sink_srv_avrcp_bqb_data.avrcp_hd, 
                                           BT_AVRCP_OPERATION_ID_PAUSE, 
                                           BT_AVRCP_OPERATION_STATE_PUSH);
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_PTH_BV_01_C_STOP")) {
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_avrcp_send_pass_through_command(bt_sink_srv_avrcp_bqb_data.avrcp_hd, 
                                           BT_AVRCP_OPERATION_ID_STOP, 
                                           BT_AVRCP_OPERATION_STATE_PUSH);
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_PTH_BV_01_C_REWIND")) {
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_avrcp_send_pass_through_command(bt_sink_srv_avrcp_bqb_data.avrcp_hd, 
                                           BT_AVRCP_OPERATION_ID_REWIND, 
                                           BT_AVRCP_OPERATION_STATE_PUSH);
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_PTH_BV_01_C_FF")) {
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_avrcp_send_pass_through_command(bt_sink_srv_avrcp_bqb_data.avrcp_hd, 
                                           BT_AVRCP_OPERATION_ID_FAST_FORWARD, 
                                           BT_AVRCP_OPERATION_STATE_PUSH);
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_PTH_BV_01_C_FORWARD")) {
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_avrcp_send_pass_through_command(bt_sink_srv_avrcp_bqb_data.avrcp_hd, 
                                           BT_AVRCP_OPERATION_ID_FORWARD, 
                                           BT_AVRCP_OPERATION_STATE_PUSH);
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_PTH_BV_01_C_BACKWARD")) {
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_avrcp_send_pass_through_command(bt_sink_srv_avrcp_bqb_data.avrcp_hd, 
                                           BT_AVRCP_OPERATION_ID_BACKWARD, 
                                           BT_AVRCP_OPERATION_STATE_PUSH);
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_PTH_BV_01_C_VOLUP")) {
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_avrcp_send_pass_through_command(bt_sink_srv_avrcp_bqb_data.avrcp_hd,
                                           BT_AVRCP_OPERATION_ID_VOLUME_UP,
                                           BT_AVRCP_OPERATION_STATE_PUSH);
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_PTH_BV_01_C_VOLDOWN")) {
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_avrcp_send_pass_through_command(bt_sink_srv_avrcp_bqb_data.avrcp_hd,
                                           BT_AVRCP_OPERATION_ID_VOLUME_DOWN,
                                           BT_AVRCP_OPERATION_STATE_PUSH);
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_PTT_BV_01_C_PLAY")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_sink_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_PASS_TH;
        bt_sink_srv_avrcp_bqb_data.pth_op_id = BT_AVRCP_OPERATION_ID_PLAY;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_PTT_BV_01_C_PAUSE")) {
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_avrcp_send_pass_through_command(bt_sink_srv_avrcp_bqb_data.avrcp_hd, 
                                           BT_AVRCP_OPERATION_ID_PAUSE, 
                                           BT_AVRCP_OPERATION_STATE_PUSH);
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_PTT_BV_01_C_STOP")) {
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_avrcp_send_pass_through_command(bt_sink_srv_avrcp_bqb_data.avrcp_hd, 
                                           BT_AVRCP_OPERATION_ID_STOP, 
                                           BT_AVRCP_OPERATION_STATE_PUSH);
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_PTT_BV_02_I_VOLUP")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_sink_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_PASS_TH;
        bt_sink_srv_avrcp_bqb_data.pth_op_id = BT_AVRCP_OPERATION_ID_VOLUME_UP;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_PTT_BV_02_I_VOLDOWN")) {
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_avrcp_send_pass_through_command(bt_sink_srv_avrcp_bqb_data.avrcp_hd,
                                           BT_AVRCP_OPERATION_ID_VOLUME_DOWN,
                                           BT_AVRCP_OPERATION_STATE_PUSH);
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_RCR_BV_01_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_sink_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_ELEMENT;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_RCR_BV_03_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_sink_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_ELEMENT;
        bt_sink_srv_avrcp_bqb_data.is_packet_abort = true;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_VLH_BI_03_C_REG_NOTIFY")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_sink_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_REG;
        bt_sink_srv_avrcp_bqb_data.register_event_id = BT_AVRCP_EVENT_VOLUME_CHANGED;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_VLH_BI_03_C_SET_VOL")) {
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_avrcp_set_absolute_volume(bt_sink_srv_avrcp_bqb_data.avrcp_hd, 0x70);
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_VLH_BI_04_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_sink_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_REG;
        bt_sink_srv_avrcp_bqb_data.register_event_id = BT_AVRCP_EVENT_VOLUME_CHANGED;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_VLH_BV_01_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_sink_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_SET_ABSOLUTE_VOL;
        bt_sink_srv_avrcp_bqb_data.register_event_id = BT_AVRCP_EVENT_VOLUME_CHANGED;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_VLH_BV_03_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_sink_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_REG;
        bt_sink_srv_avrcp_bqb_data.register_event_id = BT_AVRCP_EVENT_VOLUME_CHANGED;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_VLH_BV_01_I")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_sink_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_REG;
        bt_sink_srv_avrcp_bqb_data.register_event_id = BT_AVRCP_EVENT_VOLUME_CHANGED;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_VLH_BV_02_I")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_sink_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_SET_ABSOLUTE_VOL;
        bt_sink_srv_avrcp_bqb_data.register_event_id = BT_AVRCP_EVENT_VOLUME_CHANGED;
    /* AVCTP/AVRCP TG cases */
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_NFR_BI_01_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_NFR_BV_03_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_CEC_BV_01_I")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else 
#endif
    if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_CEC_BV_02_I_CON_A2DP")) {
        bt_a2dp_init_params_t init_param = {0};
        bt_sink_srv_avrcp_bqb_a2dp_get_init_params(&init_param);
        bt_a2dp_update_sep(&init_param);
        bt_sink_srv_avrcp_bqb_copy_str_to_addr((uint8_t *)&bt_sink_srv_avrcp_bqb_data.pts_addr, cmd + strlen("bt avrcp TC_TG_CEC_BV_02_I_CON_A2DP "));
        ret = bt_a2dp_connect(&handle, (const bt_bd_addr_t *)&bt_sink_srv_avrcp_bqb_data.pts_addr, BT_A2DP_SOURCE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_CEC_BV_02_I_CON_AVRCP")) {
        bt_sink_srv_avrcp_bqb_copy_str_to_addr((uint8_t *)&bt_sink_srv_avrcp_bqb_data.pts_addr, cmd + strlen("bt avrcp TC_TG_CEC_BV_02_I_CON_AVRCP "));
        ret = bt_avrcp_connect(&handle, (const bt_bd_addr_t *)&bt_sink_srv_avrcp_bqb_data.pts_addr);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
#ifdef BT_SINK_SRV_AVRCP_BQB_RESERVED_CASE
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_CON_BV_02_C_A2DP")) {
        bt_sink_srv_avrcp_bqb_copy_str_to_addr((uint8_t *)&bt_sink_srv_avrcp_bqb_data.pts_addr, cmd + strlen("bt avrcp TC_TG_CON_BV_02_C_A2DP "));
        bt_a2dp_connect(&handle, (const bt_bd_addr_t *)&bt_sink_srv_avrcp_bqb_data.pts_addr, BT_A2DP_SOURCE);
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_CON_BV_02_C_BROWSE")) {
        bt_avrcp_browse_connect(&handle, (const bt_bd_addr_t *)&bt_sink_srv_avrcp_bqb_data.pts_addr);
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_CON_BV_04_C")) {
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_CON_BV_04_DIS")) {
        bt_avrcp_disconnect(bt_sink_srv_avrcp_bqb_data.avrcp_hd);
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_CRC_BV_01_I")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_CRC_BV_02_I")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_CRC_BV_02_DIS")) {
        bt_gap_disconnect(bt_sink_srv_avrcp_bqb_data.gap_conn_hd);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_ICC_BV_01_I")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_ICC_BV_02_I")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_NFY_BI_01_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_NFY_BV_06_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_sink_srv_avrcp_bqb_data.browse_para.is_playing = true;

    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_NFY_BV_06_RES")) {
        bt_avrcp_send_register_notification_response_t rsp_data;
        rsp_data.response_type = BT_AVRCP_RESPONSE_CHANGED;
        rsp_data.parameter_length = 9;
        rsp_data.event_id = BT_AVRCP_EVENT_TRACK_CHANGED;
        rsp_data.id = (uint64_t)0x09;
        bt_avrcp_send_register_notification_response(bt_sink_srv_avrcp_bqb_data.avrcp_hd, &rsp_data);
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_PTT_BV_01_I")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_PTT_BV_02_I")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_VLH_BI_01_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_VLH_BI_02_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_VLH_BV_02_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_VLH_BV_04_C_ENTER_SCAN")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_VLH_BV_04_C_SEND_NOTIFY")) {
        bt_avrcp_send_register_notification_response_t rsp_data;
        rsp_data.response_type = BT_AVRCP_RESPONSE_CHANGED;
        rsp_data.parameter_length = 0x0002;
        rsp_data.event_id = BT_AVRCP_EVENT_VOLUME_CHANGED;
        rsp_data.volume = 0x60;
        bt_avrcp_send_register_notification_response(bt_sink_srv_avrcp_bqb_data.avrcp_hd, &rsp_data);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_VLH_BV_01_I_ENTER_SCAN")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_VLH_BV_01_I_SEND_NOTIFY")) {
        bt_avrcp_send_register_notification_response_t rsp_data;
        rsp_data.response_type = BT_AVRCP_RESPONSE_CHANGED;
        rsp_data.parameter_length = 0x0002;
        rsp_data.event_id = BT_AVRCP_EVENT_VOLUME_CHANGED;
        rsp_data.volume = 0x60;
        bt_avrcp_send_register_notification_response(bt_sink_srv_avrcp_bqb_data.avrcp_hd, &rsp_data);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if(BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_VLH_BV_02_I")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
/* Browse TG */
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_INV_BI_02_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_MCN_CB_BV_02_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_MCN_CB_BV_05_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_MCN_CB_BV_06_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_MCN_CB_BV_08_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_MCN_CB_BV_09_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_MCN_CB_BV_13_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
#endif
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_MCN_CB_BV_01_I")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_MCN_CB_BV_02_I")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_MCN_CB_BV_03_I")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_MCN_CB_BV_04_I")) {
        /* Next TC_MCN_PLAYING */
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_MCN_CB_BV_06_I")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
#ifdef BT_SINK_SRV_AVRCP_BQB_RESERVED_CASE
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_MCN_CB_BI_01_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_MCN_CB_BI_02_C")) {
        /* To be continue */
        bt_sink_srv_avrcp_bqb_data.browse_para.empty_folder = true;
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;

    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_MCN_CB_BI_03_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_MCN_CB_BI_04_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_MCN_CB_BI_05_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_MCN_NP_BI_01_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_MCN_NP_BV_02_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_MCN_NP_BV_06_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_MCN_NP_BV_07_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_MCN_NP_BV_09_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_MCN_NP_BV_11_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
#endif
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_MCN_NP_BV_01_I")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_MCN_NP_BV_04_I")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_MCN_NP_BV_05_I")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_MCN_NP_BV_06_I")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
#ifdef BT_SINK_SRV_AVRCP_BQB_RESERVED_CASE
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_MPS_BI_01_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_MPS_BI_02_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_MPS_BV_02_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_MPS_BV_04_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_MPS_BV_06_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_MPS_BV_09_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_MPS_BV_12_C")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_MPS_BV_01_I")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_MPS_BV_02_I")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_TG_MPS_BV_03_I")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_CON_BV_01_C_A2DP")) {
        bt_sink_srv_avrcp_bqb_copy_str_to_addr((uint8_t *)&bt_sink_srv_avrcp_bqb_data.pts_addr, cmd + strlen("bt avrcp TC_CT_CON_BV_01_C_A2DP "));
        bt_a2dp_connect(&handle, (const bt_bd_addr_t *)&bt_sink_srv_avrcp_bqb_data.pts_addr, BT_A2DP_SINK);
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_CON_BV_01_C_BROWSE")) {
        //bt_sink_srv_avrcp_bqb_copy_str_to_addr((uint8_t *)&bt_sink_srv_avrcp_bqb_data.pts_addr, cmd + strlen("bt avrcp TC_CT_CON_BV_01_C_BROWSE"));
        bt_avrcp_browse_connect(&handle, (const bt_bd_addr_t *)&bt_sink_srv_avrcp_bqb_data.pts_addr);
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_CON_BV_03_C")) {
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_CON_BV_03_DIS")) {
        bt_avrcp_disconnect(bt_sink_srv_avrcp_bqb_data.avrcp_hd);
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_MCN_CB_BV_01_C")) {
        bt_sink_srv_avrcp_bqb_data.browse_para.scope = BT_AVRCP_SCOPE_MEDIA_PLAYER_VIRTUAL_FILESYSTEM;
        bt_sink_srv_avrcp_bqb_data.browse_para.direction = BT_AVRCP_DIRECTION_DOWN;
        bt_sink_srv_avrcp_bqb_data.is_browse_connect = true;
        bt_sink_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_FOLDER_ITEMS;
        bt_sink_srv_avrcp_bqb_data.browse_para.is_set_player = true;
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_MCN_CB_BV_04_C")) {
        bt_sink_srv_avrcp_bqb_data.browse_para.scope = BT_AVRCP_SCOPE_MEDIA_PLAYER_VIRTUAL_FILESYSTEM;
        bt_sink_srv_avrcp_bqb_data.browse_para.direction = BT_AVRCP_DIRECTION_DOWN;
        bt_sink_srv_avrcp_bqb_data.is_browse_connect = true;
        bt_sink_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_FOLDER_ITEMS;
        bt_sink_srv_avrcp_bqb_data.browse_para.is_set_player = true;
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_MCN_CB_BV_07_C")) {
        bt_sink_srv_avrcp_bqb_data.browse_para.scope = BT_AVRCP_SCOPE_MEDIA_PLAYER_VIRTUAL_FILESYSTEM;
        bt_sink_srv_avrcp_bqb_data.browse_para.direction = BT_AVRCP_DIRECTION_DOWN;
        bt_sink_srv_avrcp_bqb_data.is_browse_connect = true;
        bt_sink_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_FOLDER_ITEMS;
        bt_sink_srv_avrcp_bqb_data.browse_para.is_set_player = true;
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
#endif
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_MCN_CB_BV_01_I")
        || BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_MPS_BV_01_I")) {
        bt_sink_srv_avrcp_bqb_data.browse_para.scope = BT_AVRCP_SCOPE_MEDIA_PLAYER_VIRTUAL_FILESYSTEM;
        bt_sink_srv_avrcp_bqb_data.browse_para.direction = BT_AVRCP_DIRECTION_DOWN;
        bt_sink_srv_avrcp_bqb_data.is_browse_connect = true;
        bt_sink_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_FOLDER_ITEMS;
        bt_sink_srv_avrcp_bqb_data.browse_para.is_set_player = true;
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_MCN_CB_BV_02_I")) {
        bt_sink_srv_avrcp_bqb_data.browse_para.scope = BT_AVRCP_SCOPE_MEDIA_PLAYER_VIRTUAL_FILESYSTEM;
        bt_sink_srv_avrcp_bqb_data.browse_para.direction = BT_AVRCP_DIRECTION_DOWN;
        bt_sink_srv_avrcp_bqb_data.is_browse_connect = true;
        bt_sink_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_FOLDER_ITEMS;
        bt_sink_srv_avrcp_bqb_data.item_id = BT_SINK_SRV_AVRCP_BQB_ITEM_CT_MCN_CB_BV_02_I;
        bt_sink_srv_avrcp_bqb_data.browse_para.is_set_player = true;
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_MCN_CB_BV_02_UP")) {
        ret = bt_sink_srv_avrcp_bqb_change_path(bt_sink_srv_avrcp_bqb_data.avrcp_hd, 
            bt_sink_srv_avrcp_bqb_data.browse_para.uid_counter,
            bt_sink_srv_avrcp_bqb_data.browse_para.folder_uid,
            BT_AVRCP_DIRECTION_UP);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_MCN_CB_BV_03_I")) {
        bt_sink_srv_avrcp_bqb_data.browse_para.scope = BT_AVRCP_SCOPE_MEDIA_PLAYER_VIRTUAL_FILESYSTEM;
        bt_sink_srv_avrcp_bqb_data.browse_para.direction = BT_AVRCP_DIRECTION_DOWN;
        bt_sink_srv_avrcp_bqb_data.is_browse_connect = true;
        bt_sink_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_FOLDER_ITEMS;
        bt_sink_srv_avrcp_bqb_data.browse_para.is_set_player = true;
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_MCN_CB_BV_04_I")) {
        bt_sink_srv_avrcp_bqb_data.browse_para.scope = BT_AVRCP_SCOPE_MEDIA_PLAYER_VIRTUAL_FILESYSTEM;
        bt_sink_srv_avrcp_bqb_data.browse_para.direction = BT_AVRCP_DIRECTION_DOWN;
        bt_sink_srv_avrcp_bqb_data.is_browse_connect = true;
        bt_sink_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_FOLDER_ITEMS;
        bt_sink_srv_avrcp_bqb_data.item_id = BT_SINK_SRV_AVRCP_BQB_ITEM_CT_MCN_CB_BV_04_I;
        bt_sink_srv_avrcp_bqb_data.browse_para.is_set_player = true;
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_MCN_CB_BV_05_REG")) {
        bt_sink_srv_avrcp_bqb_data.browse_para.scope = BT_AVRCP_SCOPE_MEDIA_PLAYER_VIRTUAL_FILESYSTEM;
        bt_sink_srv_avrcp_bqb_data.browse_para.direction = BT_AVRCP_DIRECTION_DOWN;
        bt_sink_srv_avrcp_bqb_data.is_browse_connect = true;
        bt_sink_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_FOLDER_ITEMS;
        bt_sink_srv_avrcp_bqb_data.browse_para.is_set_player = true;
        bt_sink_srv_avrcp_bqb_data.register_event_id = BT_AVRCP_EVENT_UIDS_CHANGED;
        bt_avrcp_register_notification(bt_sink_srv_avrcp_bqb_data.avrcp_hd, bt_sink_srv_avrcp_bqb_data.register_event_id, 0);
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_MCN_CB_BV_05_GET")) {
        ret = bt_sink_srv_avrcp_bqb_get_folder_items(bt_sink_srv_avrcp_bqb_data.avrcp_hd, bt_sink_srv_avrcp_bqb_data.browse_para.scope, 100);
        bt_sink_srv_report_id("[sink][music][avrcp][BQB] atci_callback-TC_CT_MCN_CB_BV_05_GET", 0);
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_MCN_CB_BV_06_I")) {
        bt_sink_srv_avrcp_bqb_data.browse_para.scope = BT_AVRCP_SCOPE_MEDIA_PLAYER_VIRTUAL_FILESYSTEM;
        bt_sink_srv_avrcp_bqb_data.browse_para.direction = BT_AVRCP_DIRECTION_DOWN;
        bt_sink_srv_avrcp_bqb_data.is_browse_connect = true;
        bt_sink_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_FOLDER_ITEMS;
        bt_sink_srv_avrcp_bqb_data.browse_para.is_set_player = true;
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_MCN_CB_BV_09_I")) {
        bt_sink_srv_avrcp_bqb_data.browse_para.scope = BT_AVRCP_SCOPE_MEDIA_PLAYER_VIRTUAL_FILESYSTEM;
        bt_sink_srv_avrcp_bqb_data.browse_para.direction = BT_AVRCP_DIRECTION_DOWN;
        bt_sink_srv_avrcp_bqb_data.is_browse_connect = true;
        bt_sink_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_FOLDER_ITEMS;
        bt_sink_srv_avrcp_bqb_data.browse_para.is_set_player = true;
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
#ifdef BT_SINK_SRV_AVRCP_BQB_RESERVED_CASE
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_MCN_NP_BV_01_C")) {
        /* Call TC_MCN_PLAYING */
        bt_sink_srv_avrcp_bqb_data.browse_para.scope = BT_AVRCP_SCOPE_NOW_PLAYING;
        bt_sink_srv_avrcp_bqb_data.is_browse_connect = true;
        bt_sink_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_FOLDER_ITEMS;
        bt_sink_srv_avrcp_bqb_data.browse_para.is_set_player = true;
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
            
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_MCN_NP_BV_05_C")) {
        bt_sink_srv_avrcp_bqb_data.browse_para.scope = BT_AVRCP_SCOPE_NOW_PLAYING;
        bt_sink_srv_avrcp_bqb_data.is_browse_connect = true;
        bt_sink_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_FOLDER_ITEMS;
        bt_sink_srv_avrcp_bqb_data.browse_para.is_set_player = true;
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_MCN_NP_BV_08_C")) {
        bt_sink_srv_avrcp_bqb_data.browse_para.scope = BT_AVRCP_SCOPE_NOW_PLAYING;
        bt_sink_srv_avrcp_bqb_data.is_browse_connect = true;
        bt_sink_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_FOLDER_ITEMS;
        bt_sink_srv_avrcp_bqb_data.browse_para.is_set_player = true;
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
#endif
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_MCN_NP_BV_01_I")) {
        bt_sink_srv_avrcp_bqb_data.browse_para.scope = BT_AVRCP_SCOPE_NOW_PLAYING;
        bt_sink_srv_avrcp_bqb_data.is_browse_connect = true;
        bt_sink_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_FOLDER_ITEMS;
        bt_sink_srv_avrcp_bqb_data.item_id = BT_SINK_SRV_AVRCP_BQB_ITEM_CT_MCN_NP_BV_01_I;
        bt_sink_srv_avrcp_bqb_data.browse_para.is_set_player = true;
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);

    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_MCN_NP_BV_05_I")) {
        bt_sink_srv_avrcp_bqb_data.browse_para.scope = BT_AVRCP_SCOPE_NOW_PLAYING;
        bt_sink_srv_avrcp_bqb_data.is_browse_connect = true;
        bt_sink_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_FOLDER_ITEMS;
        bt_sink_srv_avrcp_bqb_data.browse_para.is_set_player = true;
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_MCN_NP_BV_06_I")) {
        bt_sink_srv_avrcp_bqb_data.browse_para.scope = BT_AVRCP_SCOPE_NOW_PLAYING;
        bt_sink_srv_avrcp_bqb_data.is_browse_connect = true;
        bt_sink_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_FOLDER_ITEMS;
        bt_sink_srv_avrcp_bqb_data.browse_para.is_set_player = true;
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
#ifdef BT_SINK_SRV_AVRCP_BQB_RESERVED_CASE
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_MCN_PLAYING")) {
        bt_avrcp_play_item_t param;
        param.scope = bt_sink_srv_avrcp_bqb_data.browse_para.scope;
        param.uid = bt_sink_srv_avrcp_bqb_data.browse_para.item_uid;
        param.uid_counter = bt_sink_srv_avrcp_bqb_data.browse_para.uid_counter;
        ret = bt_avrcp_play_item(bt_sink_srv_avrcp_bqb_data.avrcp_hd, &param);
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_MPS_BV_03_C")) {
        bt_sink_srv_avrcp_bqb_data.browse_para.scope = BT_AVRCP_SCOPE_MEDIA_PLAYER_VIRTUAL_FILESYSTEM;
        bt_sink_srv_avrcp_bqb_data.browse_para.direction = BT_AVRCP_DIRECTION_DOWN;
        bt_sink_srv_avrcp_bqb_data.is_browse_connect = true;
        bt_sink_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_FOLDER_ITEMS;
        bt_sink_srv_avrcp_bqb_data.browse_para.is_set_player = true;
        //bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TC_CT_SFC_BV_03_I")) {
        /* To be continue */
#endif        
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp CT_bqb_init")) {
        bt_avrcp_init_t init;
        init.role = BT_AVRCP_ROLE_CT;
        init.support_browse = true;
        bt_avrcp_init(&init);

        bt_sink_srv_avrcp_bqb_data.browse_para.current_depth = 0;
        bt_sink_srv_avrcp_bqb_data.browse_para.uid_counter = 0x2468;
        bt_sink_srv_avrcp_bqb_data.browse_para.is_playing = false;
        bt_sink_srv_avrcp_bqb_data.browse_para.toggle = true;
    } else if (BT_SINK_SRV_AVRCP_BQB_STR_CMP("bt avrcp TG_bqb_init")) {
        bt_avrcp_init_t init;
        init.role = BT_AVRCP_ROLE_TG;
        init.support_browse = true;
        bt_avrcp_init(&init);
    }
    bt_sink_srv_report_id("[sink][music][avrcp][BQB] atci_callback-in_progress:0x%08x, exe_command:0x%x, ret:0x%x", 
        3, bt_sink_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress, bt_sink_srv_avrcp_bqb_data.exe_command, ret);

    return BT_STATUS_SUCCESS;
} 

