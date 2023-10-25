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
#include "bt_utils.h"
#include "bt_source_srv_utils.h"
#include "bt_connection_manager.h"
#include "bt_source_srv_a2dp.h"
#include "bt_l2cap.h"


#ifdef AIR_SOURCE_SRV_AVRCP_CT_BQB_ENABLE

#define BT_SRV_AVRCP_BQB_STR_CMP(_cmd) (strncmp((_cmd), cmd, strlen(_cmd)) == 0)

atci_status_t bt_avrcp_bqb_atci_callback(const char *string);

#if 0
static atci_cmd_hdlr_item_t bt_avrcp_bqb_atci_cmd[] = {
    {
        .command_head = "AT+AVRCPBQB",               /* INTERNAL USE, IT TEST */
        .command_hdlr = bt_avrcp_bqb_atci_callback,
        .hash_value1 = 0,
        .hash_value2 = 0,
    }
};


bt_status_t bt_srv_avrcp_bqb_init(void)
{
    atci_status_t ret;
    ret = atci_register_handler(bt_avrcp_bqb_atci_cmd, sizeof(bt_avrcp_bqb_atci_cmd) / sizeof(atci_cmd_hdlr_item_t));
    if (ret != ATCI_STATUS_OK) {
        LOG_MSGID_E(source_srv, "[SOURCE][SRV][AT] register handler fail status = %02x", 1, ret);
    }
    return ret;
}
#endif

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
    BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_CAPABILITY,
    BT_SRV_AVRCP_DON_NOT_CLEAN_FLAG
} bt_sink_srv_avrcp_bqb_exe_command_t;
#if 0
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
#endif

typedef struct {
    bt_gap_connection_handle_t gap_conn_hd;
    uint32_t avrcp_hd;
    uint32_t a2dp_hd;
    bt_bd_addr_t pts_addr;
    bt_sink_srv_avrcp_bqb_exe_command_t exe_command;
    bt_avrcp_capability_types_t capability_type;
    //bt_sink_srv_avrcp_bqb_item_id item_id;
    uint8_t pth_op_id;
    uint8_t register_event_id;
    bool is_avrcp_bqb_in_progress;
} bt_srv_avrcp_bqb_t;

static bt_srv_avrcp_bqb_t bt_srv_avrcp_bqb_data = {0};


static void bt_srv_avrcp_bqb_command_execute(void)
{
    int32_t ret = 0;

    switch(bt_srv_avrcp_bqb_data.exe_command) {
        case BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_PASS_TH:
        {
            if(bt_srv_avrcp_bqb_data.pth_op_id != BT_AVRCP_OPERATION_ID_RESERVED)
            {
                bt_avrcp_send_pass_through_command(bt_srv_avrcp_bqb_data.avrcp_hd,
                                                   bt_srv_avrcp_bqb_data.pth_op_id,
                                                   BT_AVRCP_OPERATION_STATE_PUSH);
            }
            break;
        }
        case BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_LIST_SETTING:
        {
            bt_avrcp_list_app_setting_attributes(bt_srv_avrcp_bqb_data.avrcp_hd);
            break;
        }
        case BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_REG:
        {
            bt_avrcp_register_notification(bt_srv_avrcp_bqb_data.avrcp_hd, bt_srv_avrcp_bqb_data.register_event_id, 0);
            break;
        }
        case BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_PLAY_STATUS:
        {
            bt_avrcp_get_play_status(bt_srv_avrcp_bqb_data.avrcp_hd);
            break;
        }
        case BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_SET_ABSOLUTE_VOL:
        {
            bt_avrcp_set_absolute_volume(bt_srv_avrcp_bqb_data.avrcp_hd, 0x70);
            break;
        }

        case BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_GET_CAPABILITY:
        {
            ret = bt_avrcp_get_capability(bt_srv_avrcp_bqb_data.avrcp_hd, bt_srv_avrcp_bqb_data.capability_type);
        }
        default:
            break;
    }

    LOG_MSGID_I(source_srv,"[avrcp][BQB] command_execute-exe_command:0x%x, ret:0x%x", 2, bt_srv_avrcp_bqb_data.exe_command, ret);
}

static int32_t bt_srv_avrcp_bqb_event_notification_ind(bt_avrcp_event_notification_t *noti_ind, bt_status_t status)
{
    int32_t ret = 0;

    LOG_MSGID_I(source_srv,"[avrcp][BQB] event_notification_ind", 0);
    if (noti_ind && noti_ind->event_id == BT_AVRCP_EVENT_VOLUME_CHANGED) {
        bt_srv_avrcp_bqb_command_execute();
    }
    return ret;
}

static int32_t bt_srv_avrcp_bqb_register_notification_ind(bt_avrcp_register_notification_commant_t *ind, bt_status_t status)
{
    int32_t ret = 0;
    bt_avrcp_send_register_notification_response_t rsp;

    LOG_MSGID_I(source_srv,"[avrcp][BQB] register_notification_ind-null dataPtr: %x", 1, ind->event_id);
    if (ind->event_id == BT_AVRCP_EVENT_VOLUME_CHANGED) {
        rsp.response_type = BT_AVRCP_RESPONSE_INTERIM;
        rsp.parameter_length = 0x02; /* event id(1) + volume(1) */
        rsp.event_id = ind->event_id;
        rsp.volume = 0x20;
        bt_avrcp_send_register_notification_response(ind->handle, &rsp);
    }

    return ret;
}


bool bt_srv_avrcp_bqb_in_progress(void)
{
    LOG_MSGID_I(source_srv,"[avrcp][BQB] bt_srv_avrcp_bqb_in_progress-inbqb(0x%0x)", 1, bt_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress);

    return bt_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress;
}

bt_status_t bt_srv_avrcp_bqb_common_cb(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    bool is_case_end = false;
    int32_t ret = 0;
    LOG_MSGID_I(source_srv,"[avrcp][BQB] bqb_common_cb-msg (0x%x), status(0x%08x)", 2, msg, status);

    switch (msg) {
        case BT_A2DP_CONNECT_CNF:{
            bt_a2dp_connect_cnf_t *conn_cnf = (bt_a2dp_connect_cnf_t *)buff;
            bt_srv_avrcp_bqb_data.a2dp_hd = conn_cnf->handle;
            LOG_MSGID_I(source_srv,"[avrcp][BQB] bqb_common_cb-A2DP_conn:handle:%x", 2, bt_srv_avrcp_bqb_data.a2dp_hd);

            break;
        }

        case BT_A2DP_DISCONNECT_IND: {
            bt_srv_avrcp_bqb_data.a2dp_hd = 0;

           if (bt_srv_avrcp_bqb_data.exe_command == BT_SRV_AVRCP_DON_NOT_CLEAN_FLAG) {
                is_case_end = true;
           }
           break;
        }
        case BT_AVRCP_CONNECT_CNF:
        {
            bt_avrcp_connect_cnf_t *cnf = (bt_avrcp_connect_cnf_t *)buff;

            LOG_MSGID_I(source_srv,"[avrcp][BQB] bqb_common_cb-result(0x%x), handle(0x%08x)", 2, status, cnf->handle);
            if (status != BT_STATUS_SUCCESS) {
                is_case_end = true;
                break;
            } else {
                if (bt_srv_avrcp_bqb_data.avrcp_hd) {
                   if (cnf->handle != bt_srv_avrcp_bqb_data.avrcp_hd) {
                        LOG_MSGID_I(source_srv,"[avrcp][BQB] bqb_common_cb-wrongly device connected", 0);
                   }
                } else {
                    bt_srv_avrcp_bqb_data.avrcp_hd = cnf->handle;
                }

                bt_srv_avrcp_bqb_command_execute();
            }
            break;
        }
        case BT_AVRCP_CONNECT_IND:
        {
            bt_avrcp_connect_ind_t *ind = (bt_avrcp_connect_ind_t *)buff;

            LOG_MSGID_I(source_srv,"[avrcp][BQB] bqb_common_cb-result(0x%x), handle(0x%08x)", 2, status, ind->handle);

            bt_avrcp_connect_response(ind->handle, true);
            memcpy((uint8_t *)&bt_srv_avrcp_bqb_data.pts_addr, ind->address, sizeof(bt_bd_addr_t));
            break;
        }
        case BT_AVRCP_DISCONNECT_IND:
        {
            //bt_avrcp_disconnect_ind_t *ind = (bt_avrcp_disconnect_ind_t *)buff;
            if (!bt_srv_avrcp_bqb_data.a2dp_hd) {
            is_case_end = true;
            } else {
                bt_srv_avrcp_bqb_data.exe_command = BT_SRV_AVRCP_DON_NOT_CLEAN_FLAG;
            }
            LOG_MSGID_I(source_srv,"[avrcp][BQB] bqb_common_cb-disconnect:handle:%x", 1, bt_srv_avrcp_bqb_data.a2dp_hd);
            break;
        }
        case BT_AVRCP_PASS_THROUGH_CNF:
        {
            bt_avrcp_pass_through_cnf_t *cnf = (bt_avrcp_pass_through_cnf_t *)buff;
            LOG_MSGID_I(source_srv,"[avrcp][BQB] bqb_common_cb-op_id(%x),state(%x)", 2, cnf->op_id, cnf->op_state);
            if (status != BT_STATUS_SUCCESS) {
                break;
            }

            if (cnf->op_state == BT_AVRCP_OPERATION_STATE_PUSH) {
                bt_avrcp_send_pass_through_command(cnf->handle,
                                                   cnf->op_id,
                                                   BT_AVRCP_OPERATION_STATE_RELEASED);
            }
            break;
        }

        case BT_GAP_LINK_STATUS_UPDATED_IND:
        {
            bt_gap_link_status_updated_ind_t * update_ind = (bt_gap_link_status_updated_ind_t *)buff;
            if (update_ind->link_status >= BT_GAP_LINK_STATUS_CONNECTED_0) {
                bt_srv_avrcp_bqb_data.gap_conn_hd = update_ind->handle;
            }
            break;
        }
        case BT_AVRCP_SET_ABSOLUTE_VOLUME_CNF:
        {
            bt_avrcp_set_absolute_volume_response_t *cnf = (bt_avrcp_set_absolute_volume_response_t *)buff;
            LOG_MSGID_I(source_srv,"[avrcp][BQB] bqb_common_cb-Set Absolute Volume Response: handle(0x%08x) volume(0x%x)", 2, cnf->handle, cnf->volume);
            break;
        }
        case BT_AVRCP_SET_ABSOLUTE_VOLUME_COMMAND_IND:
        {
            bt_avrcp_set_absolute_volume_event_t *set_vol = (bt_avrcp_set_absolute_volume_event_t *)buff;
            LOG_MSGID_I(source_srv,"[avrcp][BQB] bqb_common_cb-set absolute volume command: result(0x%x), handle(0x%08x), volume(0x%x)",
                3, status, set_vol->handle, set_vol->volume);

            /* To do: Adjust system volume here */
            bt_avrcp_send_set_absoulte_volume_response(set_vol->handle, set_vol->volume);

            break;
        }
        case BT_AVRCP_EVENT_NOTIFICATION_IND: {
            ret = bt_srv_avrcp_bqb_event_notification_ind((bt_avrcp_event_notification_t *)buff, status);
            break;
        }

        case BT_AVRCP_REGISTER_NOTIFICATION_IND: {
            ret = bt_srv_avrcp_bqb_register_notification_ind((bt_avrcp_register_notification_commant_t *)buff, status);
            break;
        }

        case BT_AVRCP_PASS_THROUGH_COMMAND_IND: {
            bt_avrcp_pass_through_command_ind_t * ind = (bt_avrcp_pass_through_command_ind_t *)buff;
            bt_avrcp_send_pass_through_response(ind->handle, BT_AVRCP_RESPONSE_ACCEPTED, ind->op_id, ind->op_state);
            LOG_MSGID_I(source_srv,"[avrcp][BQB] bqb_common_cb-bt_avrcp_send_pass_through_response 0x%08x", 1, ret);
            break;
        }

        default:
            break;
    }

    if (is_case_end) {
        /* clean callback  & handle*/
        LOG_MSGID_I(source_srv,"[sink][music][avrcp][BQB] bqb_common_cb-case end 0x%08x", 1, ret);
        memset(&bt_srv_avrcp_bqb_data, 0, sizeof(bt_srv_avrcp_bqb_data));
        bt_srv_avrcp_bqb_data.pth_op_id = BT_AVRCP_OPERATION_ID_RESERVED;
    }
    return BT_STATUS_SUCCESS;

}

static void bt_srv_cmd_copy_str_to_addr(uint8_t *addr, const char *str)
{
    unsigned int i, value;
    int using_long_format = 0;
    int using_hex_sign = 0;
    int result = 0;
    if (str[2] == ':' || str[2] == '-') {
        using_long_format = 1;
    }
    if (str[1] == 'x') {
        using_hex_sign = 2;
    }
    for (i = 0; i < 6; i++) {
        result = sscanf(str + using_hex_sign + i * (2 + using_long_format), "%02x", &value);
        if (result <= 0) {
            LOG_MSGID_E(source_srv, "[SOURCE][SRV][AT] addr convert fail", 0);
        }
        addr[5 - i] = (uint8_t) value;
    }
}

#define AVRCP_CMD_PARAM(s) s, bt_source_srv_strlen(s)

atci_status_t bt_avrcp_bqb_atci_callback(const char *string)
{
    uint32_t handle = 0;
    int32_t ret = 0;
    const char *cmd =string;


    LOG_I(source_srv, "[AVRCP][SRV][BQB] cmd string = %s", cmd);

    if (0 == bt_source_srv_memcmp(string, AVRCP_CMD_PARAM("TC_CT_CEC_BV_01_I,"))) {
        bt_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_avrcp_init_t avrcp_param = {0};
        avrcp_param.role = BT_AVRCP_ROLE_CT;
        avrcp_param.support_browse = false;
        bt_avrcp_init(&avrcp_param);
        char *address_str = (char *)string + strlen("TC_CT_CEC_BV_01_I,");
        bt_srv_cmd_copy_str_to_addr((uint8_t *)&bt_srv_avrcp_bqb_data.pts_addr, address_str);
        uint32_t a2dp_handle = 0;
        int32_t a2dp_ret = 0;
        a2dp_ret = bt_a2dp_connect(&a2dp_handle, (const bt_bd_addr_t *)&bt_srv_avrcp_bqb_data.pts_addr, BT_A2DP_SINK);
        ret = bt_avrcp_connect(&handle, (const bt_bd_addr_t *)&bt_srv_avrcp_bqb_data.pts_addr);
        LOG_MSGID_I(source_srv, "[AVRCP][SRV][BQB] conn_ret = %x, handle:%x, a2dp_ret ", 3, ret, handle, a2dp_ret);
        bt_srv_avrcp_bqb_data.exe_command = BT_SRV_AVRCP_DON_NOT_CLEAN_FLAG;

    } else if (BT_SRV_AVRCP_BQB_STR_CMP("TC_CT_BQB_MODE")) {
        bt_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress = true;
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);

    } else if (BT_SRV_AVRCP_BQB_STR_CMP("TC_CT_CEC_BV_02_I")) {

        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);

    } else if (BT_SRV_AVRCP_BQB_STR_CMP("TC_CT_CRC_BV_01_I")) {

        ret = bt_avrcp_disconnect(bt_srv_avrcp_bqb_data.avrcp_hd);
        uint32_t a2dp_handle = bt_l2cap_get_channel_handle(&bt_srv_avrcp_bqb_data.pts_addr, 0x0019);

        int a2dp_ret  = bt_a2dp_disconnect(a2dp_handle);

        LOG_MSGID_I(source_srv, "[AVRCP][SRV][BQB] discon=%x, avrcp_hd=%x,a2dp_hd=%x", 4, ret, bt_srv_avrcp_bqb_data.avrcp_hd, a2dp_ret, a2dp_handle);

    } else if (BT_SRV_AVRCP_BQB_STR_CMP("TC_CT_CRC_BV_02_I")) {
        bt_gap_set_scan_mode(BT_GAP_SCAN_MODE_GENERAL_ACCESSIBLE);

    } else if (0 == bt_source_srv_memcmp(string, AVRCP_CMD_PARAM("TC_CT_PHT_BV_01_I"))) {
        char *action = (char *)string + strlen("TC_CT_PTT_BV_02_I,");

        if (0 == bt_source_srv_memcmp(action, AVRCP_CMD_PARAM("play"))) {
            bt_avrcp_send_pass_through_command(bt_srv_avrcp_bqb_data.avrcp_hd, BT_AVRCP_OPERATION_ID_PLAY,BT_AVRCP_OPERATION_STATE_PUSH);

        } else if (0 == bt_source_srv_memcmp(action, AVRCP_CMD_PARAM("pause"))) {
            bt_avrcp_send_pass_through_command(bt_srv_avrcp_bqb_data.avrcp_hd, BT_AVRCP_OPERATION_ID_PAUSE,BT_AVRCP_OPERATION_STATE_PUSH);
        }
    } else if (0 == bt_source_srv_memcmp(string, AVRCP_CMD_PARAM("TC_CT_PTT_BV_02_I"))) {
        char *action = (char *)string + strlen("TC_CT_PTT_BV_02_I,");
        LOG_I(source_srv, "[AVRCP][SRV][BQB] PPT action cmd = %s",action);

        if (0 == bt_source_srv_memcmp(action, AVRCP_CMD_PARAM("up"))) {
            ret = bt_avrcp_send_pass_through_command(bt_srv_avrcp_bqb_data.avrcp_hd, BT_AVRCP_OPERATION_ID_VOLUME_UP,BT_AVRCP_OPERATION_STATE_PUSH);
        } else if((0 == bt_source_srv_memcmp(action, AVRCP_CMD_PARAM("down")))){
            ret = bt_avrcp_send_pass_through_command(bt_srv_avrcp_bqb_data.avrcp_hd, BT_AVRCP_OPERATION_ID_VOLUME_DOWN,BT_AVRCP_OPERATION_STATE_PUSH);
        }

    } else if (0 == bt_source_srv_memcmp(string, AVRCP_CMD_PARAM("TC_CT_VLH_BI_03_C"))) {

        uint8_t volume = (uint8_t)strtoul(string + strlen("TC_CT_VLH_BI_03_C,"), NULL, 16);
        ret  = bt_avrcp_set_absolute_volume(bt_srv_avrcp_bqb_data.avrcp_hd,volume);
        LOG_MSGID_I(source_srv,"[AVRCP][SRV][BQB] TC_CT_VLH_BI_03_C: ret=%x, volume=%x, handle:%x", 2, ret, bt_srv_avrcp_bqb_data.avrcp_hd);

    } else if (BT_SRV_AVRCP_BQB_STR_CMP("TC_CT_VLH_BI_04_C")) {

    } else if (BT_SRV_AVRCP_BQB_STR_CMP("TC_CT_VLH_BV_01_C")) {

    } else if (BT_SRV_AVRCP_BQB_STR_CMP("TC_CT_VLH_BV_03_C")) {
#if 0
        if (bt_srv_avrcp_bqb_data.register_event_id == BT_AVRCP_EVENT_VOLUME_CHANGED) {
            bt_avrcp_send_register_notification_response_t rsp = {0};
            rsp.response_type = BT_AVRCP_RESPONSE_INTERIM;
            rsp.parameter_length = 0x02; /* event id(1) + volume(1) */
            rsp.event_id = BT_AVRCP_EVENT_VOLUME_CHANGED;
            rsp.volume = 0x40;
            bt_avrcp_send_register_notification_response(bt_srv_avrcp_bqb_data.avrcp_hd, &rsp);
        }
#endif
        bt_srv_avrcp_bqb_data.exe_command = BT_SINK_SRV_AVRCP_BQB_EXE_COMMAND_REG;
        bt_srv_avrcp_bqb_data.register_event_id = BT_AVRCP_EVENT_VOLUME_CHANGED;

    } else if (BT_SRV_AVRCP_BQB_STR_CMP("TC_CT_VLH_BV_01_I")) {

        ret = bt_avrcp_register_notification(bt_srv_avrcp_bqb_data.avrcp_hd, BT_AVRCP_EVENT_VOLUME_CHANGED, 0);
        LOG_MSGID_I(source_srv,"[AVRCP][SRV][BQB] TC_CT_VLH_BV_01_I: ret=%x, volume=%x, handle:%x", 2, ret, bt_srv_avrcp_bqb_data.avrcp_hd);

    } else if (BT_SRV_AVRCP_BQB_STR_CMP("TC_CT_VLH_BV_02_I")) {

        uint8_t volume = (uint8_t)strtoul(string + strlen("TC_CT_VLH_BV_02_I,"), NULL, 16);
        ret  = bt_avrcp_set_absolute_volume(bt_srv_avrcp_bqb_data.avrcp_hd,volume);
        LOG_MSGID_I(source_srv,"[AVRCP][SRV][BQB] TC_CT_VLH_BI_03_C: ret=%x, volume=%x, handle:%x", 2, ret, bt_srv_avrcp_bqb_data.avrcp_hd);

    } else if (BT_SRV_AVRCP_BQB_STR_CMP("TC_CT_CHANGE_A2DP_SINK")) {
        bt_a2dp_init_params_t init_params = {0};
        bt_source_srv_a2dp_get_init_params(&init_params);
        bt_a2dp_update_sep(&init_params);
    }
    LOG_MSGID_I(source_srv,"[avrcp][BQB] atci_callback-in_progress:0x%08x, exe_command:0x%x, ret:0x%x",
        3, bt_srv_avrcp_bqb_data.is_avrcp_bqb_in_progress, bt_srv_avrcp_bqb_data.exe_command, ret);

    return BT_STATUS_SUCCESS;
}

#endif

