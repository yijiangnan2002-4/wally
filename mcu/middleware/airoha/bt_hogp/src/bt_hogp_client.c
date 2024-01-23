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

#include "bt_hogp.h"
#include "FreeRTOS.h"
#include "bt_hid_service.h"
#include "bt_hogp_internal.h"
#include "bt_gap_le.h"
#include "bt_gap_le_service.h"
#include "bt_callback_manager.h"
#include "bt_gattc_discovery.h"
#include "bt_device_manager_le.h"
#include "bt_hogp_client.h"
#include "hal_dvfs.h"

log_create_module(BT_HOGP_CLIENT, PRINT_LEVEL_INFO);

static bt_gattc_discovery_service_t g_hid_service_discovery;
static bt_gattc_discovery_characteristic_t g_hid_service_discovery_character[BT_HID_SERVICE_DISCOVERY_CHARACTER_MAX_NUMBER];
static bt_gattc_discovery_descriptor_t g_hid_service_discovery_descriptor[BT_HID_SERVICE_DISCOVERY_CHARACTER_MAX_NUMBER * BT_HID_SERVICE_DISCOVERY_DESCRIPTOR_MAX_NUMBER];
bt_hogp_client_event_callback hogp_client_callabck;

typedef struct{
    bool    bonded;
    bool    rediscovery;
    bool    bond_compelte;
    bt_hogp_client_state_t hogp_discovery_state;
    bt_handle_t  conn_handle;
    bt_addr_t   bt_addr;
    bt_hogp_client_report_char_t hogp_report_char;
    bt_hogp_client_handle_t hogp_charc_handle;
    uint8_t report_map_length;
    uint8_t *report_map_value;
    uint8_t report_cccd_handle[BT_HID_SERVICE_DISCOVERY_DESCRIPTOR_MAX_NUMBER];
}bt_hogp_client_context_t;

static bt_hogp_client_context_t g_hogp_client_ctx[BT_HOGP_CLIENT_MAX_LINK_NUM];

static bool bt_hogp_client_addr_is_bonded(bt_handle_t conn_handle){
    bt_bd_addr_t peer_addr;
    uint8_t i;
    bt_gap_le_srv_conn_info_t *conn_info = bt_gap_le_srv_get_conn_info(conn_handle);
    bt_gap_le_bonding_info_t *bonded_info = NULL;
    if(!conn_info){
        LOG_MSGID_E(BT_HOGP_CLIENT,"[BT_HOGP_CLIENT] no context", 0);
        return false;
    }
    memcpy(&peer_addr, conn_info->peer_addr.addr, sizeof(bt_bd_addr_t));
    bonded_info = (bt_gap_le_bonding_info_t *)bt_device_manager_le_get_bonding_info_by_addr((bt_bd_addr_t *)&peer_addr);
    for(i = 0; i < BT_HOGP_CLIENT_MAX_LINK_NUM; i++){
        if(memcmp(&(conn_info->peer_addr.addr[0]), &(g_hogp_client_ctx[i].bt_addr.addr[0]), sizeof(bt_bd_addr_t)) == 0){
            g_hogp_client_ctx[i].conn_handle = conn_handle;
            LOG_MSGID_I(BT_HOGP_CLIENT,"[BT_HOGP_CLIENT] find addr!, conn_handle = 0x%x", 1, conn_handle);
            if ((bonded_info != NULL) && (bonded_info->key_security_mode & BT_GAP_LE_SECURITY_BONDED_MASK) && (bonded_info->key_security_mode & BT_GAP_LE_SECURITY_ENCRYPTION_MASK)){
                LOG_MSGID_I(BT_HOGP_CLIENT,"[BT_HOGP_CLIENT] HAVE BONDED,bonded_info->key_security_mode = %x", 1, bonded_info->key_security_mode);
                g_hogp_client_ctx[i].bonded = 1;
            } else {
                LOG_MSGID_I(BT_HOGP_CLIENT,"[BT_HOGP_CLIENT] NOT BONDED", 0);
            }
            return true;
        }
    }
    return false;
}

void bt_hogp_client_set_rediscovery_flag(bt_handle_t conn_handle, bool flag)
{
    uint8_t i;
    for(i = 0; i < BT_HOGP_CLIENT_MAX_LINK_NUM; i++){
        if(g_hogp_client_ctx[i].conn_handle == conn_handle){
            g_hogp_client_ctx[i].rediscovery = flag;
            break;
        }
        if(i == (BT_HOGP_CLIENT_MAX_LINK_NUM - 1)){
            LOG_MSGID_I(BT_HOGP_CLIENT, "[BT_HOGP_CLIENT] bt_hogp_client_set_rediscovery_flag max!", 1, conn_handle);
        }
    }
    LOG_MSGID_I(BT_HOGP_CLIENT,"[BT_HOGP_CLIENT] bt_hogp_client_set_rediscovery_flag, conn_handle = 0x%x, rediscovery = %d", 2, conn_handle, g_hogp_client_ctx[i].rediscovery);
}
static void bt_hogp_clear_context(void)
{
    uint8_t i;
    
    for(i = 0; i < BT_HOGP_CLIENT_MAX_LINK_NUM; i++){
        if(g_hogp_client_ctx[i].conn_handle == 0){
            memset(&g_hogp_client_ctx[i], 0, sizeof(bt_hogp_client_context_t));
            g_hogp_client_ctx[i].hogp_discovery_state = HOGP_DISCOVEY_SERVICE_IDLE;
        }
    }
    LOG_MSGID_I(BT_HOGP_CLIENT, "[BT_HOGP_CLIENT] bt_hogp_clear_context!", 0);
}

static bt_status_t bt_hogp_client_new_conn(bt_handle_t conn_handle)
{
    uint8_t i;
    for(i = 0; i < BT_HOGP_CLIENT_MAX_LINK_NUM; i++){
        if((g_hogp_client_ctx[i].conn_handle == 0) && (g_hogp_client_ctx[i].bt_addr.addr[5] == 0)){
            g_hogp_client_ctx[i].conn_handle = conn_handle;
            g_hogp_client_ctx[i].bonded = 0;
            bt_gap_le_srv_conn_info_t *conn_info = bt_gap_le_srv_get_conn_info(conn_handle);
            memcpy(&(g_hogp_client_ctx[i].bt_addr.addr), &(conn_info->peer_addr.addr), sizeof(bt_bd_addr_t));
            g_hogp_client_ctx[i].bt_addr.type = conn_info->peer_addr.type;
            LOG_MSGID_I(BT_HOGP_CLIENT, "[BT_HOGP_CLIENT] link_idx = %x, new_conn: type=%x,addr= %x %x %x %x %x %x", 8, i, g_hogp_client_ctx[i].bt_addr.type
                                                                                                       , g_hogp_client_ctx[i].bt_addr.addr[5]
                                                                                                       , g_hogp_client_ctx[i].bt_addr.addr[4]
                                                                                                       , g_hogp_client_ctx[i].bt_addr.addr[3]
                                                                                                       , g_hogp_client_ctx[i].bt_addr.addr[2]
                                                                                                       , g_hogp_client_ctx[i].bt_addr.addr[1]
                                                                                                       , g_hogp_client_ctx[i].bt_addr.addr[0]);
            break;
        }
        if(i == (BT_HOGP_CLIENT_MAX_LINK_NUM - 1)){
            LOG_MSGID_I(BT_HOGP_CLIENT, "[BT_HOGP_CLIENT] bt_hogp_client_new_conn max!", 1, conn_handle);
            bt_hogp_clear_context();
            bt_hogp_client_new_conn(conn_handle);
        }
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_hogp_client_free_conn(bt_handle_t conn_handle)
{
    uint8_t i;
    LOG_MSGID_I(BT_HOGP_CLIENT,"[BT_HOGP_CLIENT] bt_hogp_client_free_conn, conn_handle = 0x%x", 1, conn_handle);
    for(i = 0; i < BT_HOGP_CLIENT_MAX_LINK_NUM; i++){
        if(g_hogp_client_ctx[i].conn_handle == conn_handle){
            g_hogp_client_ctx[i].conn_handle = 0;
            g_hogp_client_ctx[i].bonded = 0;
            g_hogp_client_ctx[i].report_map_length = 0;
            g_hogp_client_ctx[i].bond_compelte = 0;
            break;
        }
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_hogp_client_update_conn(bt_handle_t conn_handle)
{
    uint8_t i;
    LOG_MSGID_I(BT_HOGP_CLIENT,"[BT_HOGP_CLIENT] bt_hogp_client_update_conn, conn_handle = 0x%x", 1, conn_handle);
    bt_gap_le_srv_conn_info_t *conn_info = bt_gap_le_srv_get_conn_info(conn_handle);
    LOG_MSGID_I(BT_HOGP_CLIENT, "[BT_HOGP_CLIENT] bt_hogp_client_update_conn: type=%x,addr= %x %x %x %x %x %x", 7, conn_info->peer_addr.type
                                                                                                       , conn_info->peer_addr.addr[5]
                                                                                                       , conn_info->peer_addr.addr[4]
                                                                                                       , conn_info->peer_addr.addr[3]
                                                                                                       , conn_info->peer_addr.addr[2]
                                                                                                       , conn_info->peer_addr.addr[1]
                                                                                                       , conn_info->peer_addr.addr[0]);
    for(i = 0; i < BT_HOGP_CLIENT_MAX_LINK_NUM; i++){
        LOG_MSGID_I(BT_HOGP_CLIENT, "[BT_HOGP_CLIENT] bt_hogp_client_update_conn: type=%x,addr= %x %x %x %x %x %x", 7, g_hogp_client_ctx[i].bt_addr.type
                                                                                                       , g_hogp_client_ctx[i].bt_addr.addr[5]
                                                                                                       , g_hogp_client_ctx[i].bt_addr.addr[4]
                                                                                                       , g_hogp_client_ctx[i].bt_addr.addr[3]
                                                                                                       , g_hogp_client_ctx[i].bt_addr.addr[2]
                                                                                                       , g_hogp_client_ctx[i].bt_addr.addr[1]
                                                                                                       , g_hogp_client_ctx[i].bt_addr.addr[0]);
        if(memcmp(&(conn_info->peer_addr.addr[0]), &(g_hogp_client_ctx[i].bt_addr.addr[0]), sizeof(bt_bd_addr_t)) == 0){
            g_hogp_client_ctx[i].conn_handle = conn_handle;
            LOG_MSGID_I(BT_HOGP_CLIENT,"[BT_HOGP_CLIENT] find addr!, conn_handle = 0x%x", 1, conn_handle);
            break;
        }
    }
    if(i == BT_HOGP_CLIENT_MAX_LINK_NUM){
        bt_hogp_client_new_conn(conn_handle);
    }
    return BT_STATUS_SUCCESS;    
}

static uint8_t bt_hogp_client_find_context_by_conn_handle(bt_handle_t conn_handle)
{
    uint8_t i;
    for(i = 0; i < BT_HOGP_CLIENT_MAX_LINK_NUM; i++){
        if(g_hogp_client_ctx[i].conn_handle == conn_handle){
            LOG_MSGID_I(BT_HOGP_CLIENT, "[BT_HOGP_CLIENT]find_context handle= 0x%x index = %d", 2, conn_handle, i);
            break;
        }
        if(i == (BT_HOGP_CLIENT_MAX_LINK_NUM - 1)){
            LOG_MSGID_I(BT_HOGP_CLIENT, "[BT_HOGP_CLIENT] bt_hogp_client_find_context_by_conn_handle max, conn_handle = %x!", 1, conn_handle);
        }
    }
    return i;
}

static bt_status_t bt_hogp_client_read(bt_handle_t handle, uint16_t att_handle)
{
    bt_status_t ret;

    BT_GATTC_NEW_READ_CHARC_REQ(req, att_handle);
    if (BT_STATUS_SUCCESS != (ret = bt_gattc_read_charc(handle, &req))) {
        LOG_MSGID_I(BT_HOGP_CLIENT, "[BT_HOGP_CLIENT] read, fail! handle:%x att_handle:%x status:%x", 3, handle, att_handle, ret);
    }

    return ret;
}

static bt_handle_t bt_hogp_client_find_att_handle(bt_handle_t conn_handle, uint8_t report_id, uint8_t report_type)
{
    uint8_t i;
    uint8_t link_index;
    bt_handle_t att_handle = 0xFFFF;
    if((link_index = bt_hogp_client_find_context_by_conn_handle(conn_handle)) == BT_HOGP_CLIENT_MAX_LINK_NUM){
        LOG_MSGID_E(BT_HOGP_CLIENT,"[BT_HOGP_CLIENT] bt_hogp_client_find_att_handle, link_index = 0x%x", 1, link_index);
        return att_handle;
    }
    for(i = 0; i < 10; i++){
        if((g_hogp_client_ctx[link_index].hogp_report_char.report_reference[i].report_id == report_id) &&
            (g_hogp_client_ctx[link_index].hogp_report_char.report_reference[i].report_type == report_type)){
            att_handle = g_hogp_client_ctx[link_index].hogp_report_char.value_handle[i];
        }
    }
    LOG_MSGID_I(BT_HOGP_CLIENT, "[BT_HOGP_CLIENT] bt_hogp_client_find_att_handle:%x", 1, att_handle);
    return att_handle;
}

static bt_hogp_report_id_t bt_hogp_client_find_report_id(bt_handle_t conn_handle, bt_handle_t att_handle)
{
    uint8_t i;
    uint8_t link_index;
    bt_hogp_report_id_t report_id = 0xFF;
    bt_gap_le_srv_conn_info_t *conn_info = bt_gap_le_srv_get_conn_info(conn_handle);
    if((link_index = bt_hogp_client_find_context_by_conn_handle(conn_handle)) == BT_HOGP_CLIENT_MAX_LINK_NUM){
        LOG_MSGID_E(BT_HOGP_CLIENT,"[BT_HOGP_CLIENT] bt_hogp_client_find_report_id, link_index = 0x%x", 1, link_index);
        report_id = 0xFE;
        return report_id;
    }
    for(i = 0; i < BT_HID_SERVICE_REPORT_REF_MAX_NUMBER; i++){
        if((g_hogp_client_ctx[link_index].hogp_report_char.value_handle[i] == att_handle) && 
            (memcmp(&(conn_info->peer_addr.addr[0]), &(g_hogp_client_ctx[link_index].bt_addr.addr[0]), sizeof(bt_bd_addr_t)) == 0)){
            report_id = g_hogp_client_ctx[link_index].hogp_report_char.report_reference[i].report_id;
            break;
        }
    }
    LOG_MSGID_I(BT_HOGP_CLIENT, "[BT_HOGP_CLIENT] bt_hogp_client_find_report_id:%x link_index: %d att_handle: 0x%x i:%d", 4, report_id, link_index, att_handle, i);
    return report_id;
}

bt_status_t bt_hogp_client_notify_user(bt_hogp_client_event_t event, bt_hogp_client_para_t *para, bt_status_t status, void *buffer, uint16_t length)
{
    if (hogp_client_callabck == NULL) {
        LOG_MSGID_I(BT_HOGP_CLIENT, "[BT_HOGP_CLIENT] event callback is NULL", 0);
        return BT_STATUS_FAIL;
    }
    status = BT_STATUS_SUCCESS;
    LOG_MSGID_I(BT_HOGP_CLIENT, "[BT_HOGP_CLIENT] bt_hogp_client_notify_user,event:%x", 1, event);
    return hogp_client_callabck(event, para, status, buffer, length);
}

bt_status_t bt_hogp_client_send_output_report(uint8_t  report_id, bt_handle_t conn_handle, void *buffer, uint16_t length)
{
    LOG_MSGID_I(BT_HOGP_CLIENT, "[BT_HOGP_CLIENT] bt_hogp_client_send_output_report, conn_handle = 0x%x,length = %d", 2 ,conn_handle, length);
    bt_status_t status = BT_STATUS_FAIL;
    uint8_t *buf = NULL;
    buf = (uint8_t *)pvPortMalloc(3 + length);
    if (buf == NULL) {
        LOG_MSGID_I(BT_HOGP_CLIENT, "[BT_HOGP_CLIENT]  buffer is NULL", 0);
        return status;
    }
    bt_gattc_write_without_rsp_req_t write_req;
    write_req.attribute_value_length = length + 3;
    write_req.att_req = (bt_att_write_command_t *)buf;
    write_req.att_req->opcode = BT_ATT_OPCODE_WRITE_COMMAND;
    write_req.att_req->attribute_handle = bt_hogp_client_find_att_handle(conn_handle, report_id, BT_HOGP_CLIENT_OUTPUT_REPORT);
    memcpy(write_req.att_req->attribute_value, buffer, length);
    status = bt_gattc_write_without_rsp(conn_handle, 0, &write_req);
    vPortFree(buf);
    return status;
}

static bt_status_t bt_hogp_client_read_long_charc(bt_handle_t handle, uint16_t att_handle)
{
    bt_status_t ret;

     BT_GATTC_NEW_READ_LONG_CHARC_REQ(req, att_handle, 0);
    if (BT_STATUS_SUCCESS != (ret = bt_gattc_read_long_charc(handle, &req))) {
        LOG_MSGID_I(BT_HOGP_CLIENT, "[BT_HOGP_CLIENT] read, fail! handle:%x att_handle:%x status:%x", 3, handle, att_handle, ret);
    }
    return ret;
}


static bt_status_t bt_hogp_client_set_cccd(bt_handle_t handle, uint16_t att_handle, uint16_t cccd)
{
    bt_status_t ret;
    uint8_t p_buf[5];

    BT_GATTC_NEW_WRITE_CHARC_REQ(req, p_buf, att_handle, (uint8_t *)&cccd, BT_HOGP_CCCD_VALUE_LEN);

    if (BT_STATUS_SUCCESS != (ret = bt_gattc_write_charc(handle, &req))) {
        LOG_MSGID_I(BT_HOGP_CLIENT, "[BT_HOGP_CLIENT] set_cccd, fail! handle:%x att_handle:%x ret:%x", 3, handle, att_handle, ret);
    }

    return ret;
}

static bt_handle_t bt_hogp_client_get_handle_by_char_uuid(bt_gattc_discovery_service_t *service, uint16_t uuid)
{
    uint8_t i;
    bt_handle_t handle = 0;
    for(i = 0; i < service->char_count_found; i++){
        if(service->charateristics[i].char_uuid.uuid.uuid16 == uuid){
            handle = service->charateristics[i].value_handle;
            break;
        }
    }
    return handle;
}

static void bt_hogp_client_get_cccd_handle(uint8_t link_index)

{
    uint8_t i, idx = 0;
    for(i = 0; i < g_hogp_client_ctx[link_index].hogp_report_char.charc_num; i++){
        if(g_hogp_client_ctx[link_index].hogp_report_char.report_reference[i].cccd_handle){
            g_hogp_client_ctx[link_index].report_cccd_handle[idx] = g_hogp_client_ctx[link_index].hogp_report_char.report_reference[i].cccd_handle;
            LOG_MSGID_I(BT_HOGP_CLIENT,"[BT_HOGP_CLIENT] bt_hogp_client_get_cccd_handle, cccd_handle = 0x%x, i = %x, idx = %x", 3, g_hogp_client_ctx[link_index].report_cccd_handle[idx], i, idx);
            idx++;
        }
    }
    for(uint8_t num = 0; num < g_hogp_client_ctx[link_index].hogp_report_char.cccd_num; num++){
        LOG_MSGID_I(BT_HOGP_CLIENT,"[BT_HOGP_CLIENT] bt_hogp_client_get_cccd_handle, cccd_handle[%x] = %x", 2, num, g_hogp_client_ctx[link_index].report_cccd_handle[num]);
    }
}

static void bt_hogp_client_get_report_ref_handle_by_uuid(bt_handle_t conn_handle, bt_gattc_discovery_service_t *service, uint16_t char_uuid)
{
    uint8_t i, j, link_index;
    if((link_index = bt_hogp_client_find_context_by_conn_handle(conn_handle)) == BT_HOGP_CLIENT_MAX_LINK_NUM){
        LOG_MSGID_E(BT_HOGP_CLIENT,"[BT_HOGP_CLIENT] bt_hogp_client_get_report_ref_handle_by_uuid, link_index = 0x%x, char_uuid:0x%x", 2, link_index, char_uuid);
        return;
    }
    LOG_MSGID_I(BT_HOGP_CLIENT,"[BT_HOGP_CLIENT] bt_hogp_client_get_report_ref_handle_by_uuid, char = 0x%x, char_uuid:0x%x", 2, service->char_count_found, char_uuid);
    g_hogp_client_ctx[link_index].hogp_report_char.charc_num = 0;
    g_hogp_client_ctx[link_index].hogp_report_char.ref_num = 0;
    g_hogp_client_ctx[link_index].hogp_report_char.cccd_num = 0;
    for(i = 0; i < service->char_count_found; i++){
        if(service->charateristics[i].char_uuid.uuid.uuid16 == char_uuid){
            g_hogp_client_ctx[link_index].hogp_report_char.value_handle[g_hogp_client_ctx[link_index].hogp_report_char.charc_num] = service->charateristics[i].value_handle;
            for(j = 0; j < service->charateristics[i].descr_count_found; j++){
                LOG_MSGID_I(BT_HOGP_CLIENT,"[BT_HOGP_CLIENT] i = %d, j = %d, uuid:0x%x", 3, i, j, service->charateristics[i].descriptor[j].descriptor_uuid.uuid.uuid16);
                if(service->charateristics[i].descriptor[j].descriptor_uuid.uuid.uuid16 ==  BT_SIG_UUID16_CCCD){
                    g_hogp_client_ctx[link_index].hogp_report_char.report_reference[g_hogp_client_ctx[link_index].hogp_report_char.charc_num].cccd_handle = service->charateristics[i].descriptor[j].handle;
                    g_hogp_client_ctx[link_index].hogp_report_char.cccd_num++;
                } else if(service->charateristics[i].descriptor[j].descriptor_uuid.uuid.uuid16 ==  BT_SIG_UUID16_REPORT_REF){
                    g_hogp_client_ctx[link_index].hogp_report_char.report_reference[g_hogp_client_ctx[link_index].hogp_report_char.charc_num].report_ref_handle = service->charateristics[i].descriptor[j].handle;
                    g_hogp_client_ctx[link_index].hogp_report_char.ref_num++;
                }
            }
        g_hogp_client_ctx[link_index].hogp_report_char.charc_num ++;
        }
    }
    bt_hogp_client_get_cccd_handle(link_index);
}


static void bt_hogp_client_action(bt_status_t status, bt_handle_t connection_handle)
{
    bt_hogp_client_state_t state;
    uint8_t link_index;
    if((link_index = bt_hogp_client_find_context_by_conn_handle(connection_handle)) == BT_HOGP_CLIENT_MAX_LINK_NUM){
        LOG_MSGID_E(BT_HOGP_CLIENT,"[BT_HOGP_CLIENT] bt_hogp_client_action, link_index = 0x%x", 1, link_index);
        return;
    }
    state = g_hogp_client_ctx[link_index].hogp_discovery_state;
    LOG_MSGID_I(BT_HOGP_CLIENT,"[BT_HOGP_CLIENT] bt_hogp_client_action, state = 0x%x, link_index = %d", 2, state, link_index);
    switch (state) {
        case HOGP_DISCOVEY_SERVICE_IDLE: {
            break;
        }
        case HOGP_DISCOVEY_SERVICE_HID_INFO: {
            bt_hogp_client_read(connection_handle, g_hogp_client_ctx[link_index].hogp_charc_handle.hid_info_handle);
            break;
        }
        case HOGP_DISCOVEY_SERVICE_REPORT_MAP: {
            if(status != BT_ATT_ERRCODE_CONTINUE){
                bt_hogp_client_read_long_charc(connection_handle, g_hogp_client_ctx[link_index].hogp_charc_handle.report_map_handle);
            }
            break;
        }
        case HOGP_DISCOVEY_SERVICE_REPORT_REF: {
            LOG_MSGID_I(BT_HOGP_CLIENT,"[BT_HOGP_CLIENT] bt_hogp_client_action, ref_num = %d", 1, g_hogp_client_ctx[link_index].hogp_report_char.ref_num);
            if(g_hogp_client_ctx[link_index].hogp_report_char.ref_num != 0){
                g_hogp_client_ctx[link_index].hogp_report_char.ref_num--;
                bt_hogp_client_read(connection_handle,  g_hogp_client_ctx[link_index].hogp_report_char.report_reference[ g_hogp_client_ctx[link_index].hogp_report_char.ref_num].report_ref_handle);
            } else{
                g_hogp_client_ctx[link_index].hogp_discovery_state = HOGP_DISCOVEY_SERVICE_COMPLETE;
            }
            break;
        }
        case HOGP_DISCOVEY_SERVICE_PROTOCOL_MODE:{
            bt_hogp_client_read(connection_handle, g_hogp_client_ctx[link_index].hogp_charc_handle.protocol_mode_handle);
            break;
        }
        case HOGP_DISCOVEY_SERVICE_REPORT:{
            bt_hogp_client_read(connection_handle, g_hogp_client_ctx[link_index].hogp_charc_handle.report_handle);
            break;
        }
        case HOGP_DISCOVEY_SERVICE_SET_CCCD:{
            LOG_MSGID_I(BT_HOGP_CLIENT,"[BT_HOGP_CLIENT] bt_hogp_client_action, cccd_num = %d", 1,  g_hogp_client_ctx[link_index].hogp_report_char.cccd_num);
            if(!g_hogp_client_ctx[link_index].hogp_report_char.cccd_num){
                break;
            }
            for(uint8_t num = 0; num < g_hogp_client_ctx[link_index].hogp_report_char.cccd_num; num++){
                LOG_MSGID_I(BT_HOGP_CLIENT,"[BT_HOGP_CLIENT] bt_hogp_client_action, cccd_handle[%x] = %x", 2, num, g_hogp_client_ctx[link_index].report_cccd_handle[num]);
            }
            g_hogp_client_ctx[link_index].hogp_report_char.cccd_num--;
            bt_hogp_client_set_cccd(connection_handle, g_hogp_client_ctx[link_index].report_cccd_handle[g_hogp_client_ctx[link_index].hogp_report_char.cccd_num], BT_HOGP_ENABLE_NOTIFICATION);
            break;
        }
        case HOGP_DISCOVEY_SERVICE_COMPLETE: {
            bt_gattc_discovery_continue(connection_handle);
            break;
        }
        default:
            break;
    }
}

static void bt_hogp_client_service_discovery_callback(bt_gattc_discovery_event_t *event)
{
    LOG_MSGID_I(BT_HOGP_CLIENT,"[BT_HOGP_CLIENT] bt_hogp_service_discovery_callback, type = 0x%x, conn_handle = 0x%x,last = 0x%x", 3, event->event_type, event->conn_handle, event->last_instance);
    uint8_t link_index;
    if (NULL == event) {
        bt_gattc_discovery_continue(event->conn_handle);
        return;
    }

    if ((BT_GATTC_DISCOVERY_EVENT_FAIL == event->event_type) && (!event->last_instance)) {
        bt_gattc_discovery_continue(event->conn_handle);
        return;
    }
    if((link_index = bt_hogp_client_find_context_by_conn_handle(event->conn_handle)) == BT_HOGP_CLIENT_MAX_LINK_NUM){
        LOG_MSGID_E(BT_HOGP_CLIENT,"[BT_HOGP_CLIENT] bt_hogp_service_discovery_callback, link_index = 0x%x", 1, link_index);
        bt_gattc_discovery_continue(event->conn_handle);
        return;
    }
    LOG_MSGID_I(BT_HOGP_CLIENT, "[BT_HOGP_CLIENT] bt_hogp_client_service_discovery_callback, handle:%x charc_num:%d", 2, event->conn_handle, g_hid_service_discovery.char_count_found);
    if (BT_GATTC_DISCOVERY_EVENT_COMPLETE == event->event_type) {
        g_hogp_client_ctx[link_index].hogp_charc_handle.hid_info_handle = bt_hogp_client_get_handle_by_char_uuid(&g_hid_service_discovery, BT_SIG_UUID16_HIDS_INFO);
        if(g_hogp_client_ctx[link_index].hogp_charc_handle.hid_info_handle == 0){
            LOG_MSGID_I(BT_HOGP_CLIENT,"[BT_HOGP_CLIENT] MISS hid info characteristic", 0);
        }
        g_hogp_client_ctx[link_index].hogp_charc_handle.report_map_handle = bt_hogp_client_get_handle_by_char_uuid(&g_hid_service_discovery, BT_SIG_UUID16_REPORT_MAP);
        if(g_hogp_client_ctx[link_index].hogp_charc_handle.report_map_handle == 0){
            LOG_MSGID_I(BT_HOGP_CLIENT,"[BT_HOGP_CLIENT] MISS report map characteristic", 0);
        }
        g_hogp_client_ctx[link_index].hogp_charc_handle.protocol_mode_handle= bt_hogp_client_get_handle_by_char_uuid(&g_hid_service_discovery, BT_SIG_UUID16_PROTOCOL_MODE);
        if(g_hogp_client_ctx[link_index].hogp_charc_handle.protocol_mode_handle == 0){
            LOG_MSGID_I(BT_HOGP_CLIENT,"[BT_HOGP_CLIENT] MISS protocol mode characteristic", 0);
        }
        g_hogp_client_ctx[link_index].hogp_charc_handle.report_handle = bt_hogp_client_get_handle_by_char_uuid(&g_hid_service_discovery, BT_SIG_UUID16_REPORT);
        if(g_hogp_client_ctx[link_index].hogp_charc_handle.report_handle == 0){
            LOG_MSGID_I(BT_HOGP_CLIENT,"[BT_HOGP_CLIENT] MISS report handle characteristic", 0);
        }
        bt_hogp_client_get_report_ref_handle_by_uuid(event->conn_handle, &g_hid_service_discovery, BT_SIG_UUID16_REPORT);
        
        if(g_hogp_client_ctx[link_index].bonded && (!g_hogp_client_ctx[link_index].rediscovery)){
            g_hogp_client_ctx[link_index].hogp_discovery_state = HOGP_DISCOVEY_SERVICE_SET_CCCD;
        } else {
            g_hogp_client_ctx[link_index].hogp_discovery_state = HOGP_DISCOVEY_SERVICE_HID_INFO;
            g_hogp_client_ctx[link_index].report_map_value = pvPortMalloc(BT_HID_SERVICE_REPORT_MAP_MAX_LENGTH);
        }
        bt_hogp_client_action(0, event->conn_handle);
    }else {
        LOG_MSGID_E(BT_HOGP_CLIENT,"[BT_HOGP_CLIENT] bt_hogp_service_discovery_callback, continue", 0);
        bt_gattc_discovery_continue(event->conn_handle);
    }
}

static void bt_hogp_client_start(bt_handle_t connection_handle)
{
    LOG_MSGID_I(BT_HOGP_CLIENT, "[BT_HOGP_CLIENT] discovery_start connection_handle:0x%x", 1, connection_handle);
    bt_gattc_discovery_start(BT_GATTC_DISCOVERY_USER_HOGP,  connection_handle, FALSE);
}

static void bt_hogp_client_handle_read_rsp(bt_status_t status, bt_gattc_read_rsp_t *rsp)
{  
    bt_hogp_client_state_t state;
    bt_att_read_rsp_t *att_rsp = rsp->att_rsp;
    LOG_MSGID_I(BT_HOGP_CLIENT, "[BT_HOGP_CLIENT] read_rsp, handle:%x rsp_length: 0x%x rsp_data: 0x%x", 3, rsp->connection_handle, rsp->length, att_rsp->attribute_value[0]);
    uint8_t link_index;
    if((link_index = bt_hogp_client_find_context_by_conn_handle(rsp->connection_handle)) == BT_HOGP_CLIENT_MAX_LINK_NUM){
        LOG_MSGID_E(BT_HOGP_CLIENT,"[BT_HOGP_CLIENT] bt_hogp_handle_read_rsp, link_index = 0x%x", 1, link_index);
        return;
    }
    state = g_hogp_client_ctx[link_index].hogp_discovery_state;
    LOG_MSGID_I(BT_HOGP_CLIENT, "[BT_HOGP_CLIENT]  bt_hogp_client_handle_read_rsp state = 0x%x", 1, state);
    switch (state) {
        case HOGP_DISCOVEY_SERVICE_IDLE: {
        break;
        }
        case HOGP_DISCOVEY_SERVICE_HID_INFO: {
            g_hogp_client_ctx[link_index].hogp_discovery_state = HOGP_DISCOVEY_SERVICE_REPORT_MAP;
        break;
        }
        case HOGP_DISCOVEY_SERVICE_REPORT_MAP: {
            LOG_MSGID_I(BT_HOGP_CLIENT, "[BT_HOGP_CLIENT]  report length = 0x%x", 1, g_hogp_client_ctx[link_index].report_map_length);
            memcpy(g_hogp_client_ctx[link_index].report_map_value + g_hogp_client_ctx[link_index].report_map_length, &(att_rsp->attribute_value[0]), rsp->length - 1);
            g_hogp_client_ctx[link_index].report_map_length += rsp->length - 1;
            if(status == BT_STATUS_SUCCESS){
                if(g_hogp_client_ctx[link_index].hogp_charc_handle.protocol_mode_handle){
                    g_hogp_client_ctx[link_index].hogp_discovery_state = HOGP_DISCOVEY_SERVICE_PROTOCOL_MODE;
                } else {
                    g_hogp_client_ctx[link_index].hogp_discovery_state = HOGP_DISCOVEY_SERVICE_REPORT;
                }
                bt_hogp_client_para_t para;
                para.conn_handle = rsp->connection_handle;
                para.report_id = 0xFF;
                bt_hogp_client_notify_user(BT_HOGP_CLIENT_EVENT_REPORT_MAP_IND, &para, status, g_hogp_client_ctx[link_index].report_map_value, g_hogp_client_ctx[link_index].report_map_length);
                vPortFree(g_hogp_client_ctx[link_index].report_map_value);
                LOG_MSGID_I(BT_HOGP_CLIENT, "[BT_HOGP_CLIENT]  Free  report_map_value", 0);
            }
        break;
        }
        case HOGP_DISCOVEY_SERVICE_PROTOCOL_MODE:{
            g_hogp_client_ctx[link_index].hogp_discovery_state = HOGP_DISCOVEY_SERVICE_REPORT;
        break;
        }
        case HOGP_DISCOVEY_SERVICE_REPORT:{
            g_hogp_client_ctx[link_index].hogp_discovery_state = HOGP_DISCOVEY_SERVICE_REPORT_REF;    
        break;
        }
        case HOGP_DISCOVEY_SERVICE_REPORT_REF:{
            g_hogp_client_ctx[link_index].hogp_report_char.report_reference[g_hogp_client_ctx[link_index].hogp_report_char.ref_num].report_id =  att_rsp->attribute_value[0];
            g_hogp_client_ctx[link_index].hogp_report_char.report_reference[g_hogp_client_ctx[link_index].hogp_report_char.ref_num].report_type =  att_rsp->attribute_value[1];
            if(g_hogp_client_ctx[link_index].hogp_report_char.ref_num == 0){
                g_hogp_client_ctx[link_index].hogp_discovery_state = HOGP_DISCOVEY_SERVICE_SET_CCCD;
            }
        break;
        }
        case HOGP_DISCOVEY_SERVICE_COMPLETE: {
        break;
        }
        default:
        break;
    }
    bt_hogp_client_action(status, rsp->connection_handle);
}

static void bt_hogp_client_handle_write_rsp(bt_status_t status, bt_gattc_write_rsp_t *rsp)
{
    uint8_t link_index;
    bt_hogp_client_state_t state;
    bt_handle_t conn_handle;
    if(status != BT_STATUS_SUCCESS){
        conn_handle = ((bt_gattc_error_rsp_t *)rsp)->connection_handle;
        link_index = bt_hogp_client_find_context_by_conn_handle(conn_handle);
        if(link_index == BT_HOGP_CLIENT_MAX_LINK_NUM){
            LOG_MSGID_E(BT_HOGP_CLIENT,"[BT_HOGP_CLIENT] bt_hogp_client_handle_write_rsp, link_index = 0x%x", 1, link_index);
            return;
        }
        LOG_MSGID_I(BT_HOGP_CLIENT, "[BT_HOGP_CLIENT] bt_hogp_client_handle_write_rsp, statu:%x, handle:%x", 2, status, rsp->connection_handle);
        if(g_hogp_client_ctx[link_index].hogp_report_char.cccd_num == 0){
            g_hogp_client_ctx[link_index].hogp_discovery_state = HOGP_DISCOVEY_SERVICE_COMPLETE;
            bt_hogp_client_action(status, conn_handle);
            return;
        }
        g_hogp_client_ctx[link_index].hogp_report_char.cccd_num--;
        bt_hogp_client_set_cccd(conn_handle, g_hogp_client_ctx[link_index].report_cccd_handle[g_hogp_client_ctx[link_index].hogp_report_char.cccd_num], BT_HOGP_ENABLE_NOTIFICATION);
        return;
    }
    LOG_MSGID_I(BT_HOGP_CLIENT, "[BT_HOGP_CLIENT] bt_hogp_client_handle_write_rsp, handle:%x", 1, rsp->connection_handle);

    if((link_index = bt_hogp_client_find_context_by_conn_handle(rsp->connection_handle)) == BT_HOGP_CLIENT_MAX_LINK_NUM){
        LOG_MSGID_E(BT_HOGP_CLIENT,"[BT_HOGP_CLIENT] bt_hogp_client_handle_write_rsp, link_index = 0x%x", 1, link_index);
        return;
    }
    state = g_hogp_client_ctx[link_index].hogp_discovery_state;
    LOG_MSGID_I(BT_HOGP_CLIENT, "[BT_HOGP_CLIENT]  bt_hogp_client_handle_write_rsp state = 0x%x", 1, state);
    switch (state) {
        case HOGP_DISCOVEY_SERVICE_SET_CCCD:{
            if(g_hogp_client_ctx[link_index].hogp_report_char.cccd_num == 0){
                g_hogp_client_ctx[link_index].hogp_discovery_state = HOGP_DISCOVEY_SERVICE_COMPLETE;
            }
        break;
        }
        default:
        break;
    }
    bt_hogp_client_action(status, rsp->connection_handle);
}

static void bt_hogp_client_handle_notification(bt_gatt_handle_value_notification_t *notify)
{
    bt_att_handle_value_notification_t *att_rsp = notify->att_rsp;
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_hogp_client_para_t para;
    para.conn_handle = notify->connection_handle;
    para.report_id = bt_hogp_client_find_report_id(notify->connection_handle, att_rsp->handle);
    if(para.report_id == 0xFE){
        LOG_MSGID_E(BT_HOGP_CLIENT, "[BT_HOGP_CLIENT] bt_hogp_client_handle_notification report_id error!!", 0);
        return;
    }
    LOG_MSGID_I(BT_HOGP_CLIENT, "[BT_HOGP_CLIENT] bt_hogp_client_handle_notification connection_handle = 0x%x, len = %d , report id = 0x%x", 3, notify->connection_handle, notify->length, para.report_id);
    bt_hogp_client_notify_user(BT_HOGP_CLIENT_EVENT_INPUT_REPORT_IND, &para, status, &(att_rsp->attribute_value[0]), notify->length - 3);
}

static bt_status_t bt_hogp_client_common_event_callback(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    LOG_MSGID_I(BT_HOGP_CLIENT, "[BT_HOGP_CLIENT] bt_hogp_client_common_event_callback status = 0x%x, msg:0x%x", 2, status, msg);
    if (NULL == buff) {
        return BT_STATUS_FAIL;
    }
    switch (msg) {
        case BT_GAP_LE_CONNECT_IND: {
            bt_gap_le_connection_ind_t *ind = (bt_gap_le_connection_ind_t *)buff;
            if(ind){
                if(ind->connection_handle == 0){
                    break;
                }
                bt_hogp_client_para_t para;
                para.report_id = 0xFF;
                para.conn_handle = ind->connection_handle;
                bt_hogp_client_notify_user(BT_HOGP_CLIENT_EVENT_CONNECT_IND, &para, status, NULL, 0);
                if(bt_hogp_client_addr_is_bonded(ind->connection_handle)){
                    bt_hogp_client_update_conn(ind->connection_handle);
                } else {
                    bt_hogp_client_new_conn(ind->connection_handle);
                }
            }
            break;
        }
        case BT_GAP_LE_DISCONNECT_IND: {
            bt_hci_evt_disconnect_complete_t *disconn_ind = (bt_hci_evt_disconnect_complete_t *) buff;
            if(disconn_ind){
                bt_hogp_client_free_conn(disconn_ind->connection_handle);
                memset(&g_hid_service_discovery_descriptor, 0, sizeof(bt_gattc_discovery_descriptor_t) * BT_HID_SERVICE_DISCOVERY_CHARACTER_MAX_NUMBER * BT_HID_SERVICE_DISCOVERY_DESCRIPTOR_MAX_NUMBER);
            }
            break;
        }
        case BT_GAP_LE_BONDING_COMPLETE_IND: {
            uint8_t link_index;
            bt_gap_le_bonding_complete_ind_t *bond_ind = (bt_gap_le_bonding_complete_ind_t *)buff;
            link_index = bt_hogp_client_find_context_by_conn_handle(bond_ind->handle);
            if(link_index == BT_HOGP_CLIENT_MAX_LINK_NUM){
                LOG_MSGID_E(BT_HOGP_CLIENT,"[BT_HOGP_CLIENT] bt_hogp_client_common_event_callback, link_index = 0x%x", 1, link_index);
                break;
            }
            g_hogp_client_ctx[link_index].bond_compelte = 1;
            bt_hogp_client_start(bond_ind->handle);
            break;
        }
        case BT_GATTC_READ_CHARC:{
            bt_hogp_client_handle_read_rsp(status, (bt_gattc_read_rsp_t *)buff);
            break;
        }
        case BT_GATTC_WRITE_CHARC:{
            bt_hogp_client_handle_write_rsp(status, (bt_gattc_write_rsp_t *)buff);
            break;
        }
        case BT_GATTC_READ_LONG_CHARC:{
            if ((status == BT_STATUS_SUCCESS) || (status = BT_ATT_ERRCODE_CONTINUE)) {
                bt_hogp_client_handle_read_rsp(status, (bt_gattc_read_rsp_t *)buff);
            }
            break;
        }
        case BT_GATTC_CHARC_VALUE_NOTIFICATION: {
            bt_hogp_client_handle_notification((bt_gatt_handle_value_notification_t *)buff);
            break;
        }
        case BT_GATTC_EXCHANGE_MTU:{
            uint8_t link_index;
            bt_gatt_exchange_mtu_rsp_t *conn_ind = (bt_gatt_exchange_mtu_rsp_t *)buff;
            link_index = bt_hogp_client_find_context_by_conn_handle(conn_ind->connection_handle);
            if(link_index == BT_HOGP_CLIENT_MAX_LINK_NUM){
                LOG_MSGID_E(BT_HOGP_CLIENT,"[BT_HOGP_CLIENT] bt_hogp_client_common_event_callback, link_index = 0x%x", 1, link_index);
                break;
            }
            if(g_hogp_client_ctx[link_index].bond_compelte){
            bt_hogp_client_start(conn_ind->connection_handle);
            }
            break;
        }
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}

bt_status_t bt_hogp_client_init(bt_hogp_client_event_callback callback)
{
    bt_gattc_discovery_status_t ret = BT_GATTC_DISCOVERY_STATUS_FAIL;
    bt_gattc_discovery_user_data_t user_data;
    if (hogp_client_callabck != NULL) {
        LOG_MSGID_I(BT_HOGP_CLIENT, "[BT_HOGP_CLIENT] init fail, had init", 0);
        return BT_STATUS_FAIL;
    }
    uint8_t i, j;
    for(i = 0; i < BT_HOGP_CLIENT_MAX_LINK_NUM; i++){
        memset(&g_hogp_client_ctx[i], 0, sizeof(bt_hogp_client_context_t));
        g_hogp_client_ctx[i].hogp_discovery_state = HOGP_DISCOVEY_SERVICE_IDLE;
    }
    hogp_client_callabck = callback;
    LOG_MSGID_I(BT_HOGP_CLIENT,"[BT_HOGP_CLIENT] init HOGP Client, hogp_client_callabck:%x,callback: %x", 2, &hogp_client_callabck, callback);
    memset(&g_hid_service_discovery_character, 0, sizeof(bt_gattc_discovery_characteristic_t) * BT_HID_SERVICE_DISCOVERY_CHARACTER_MAX_NUMBER);
    memset(&g_hid_service_discovery_descriptor, 0, sizeof(bt_gattc_discovery_descriptor_t) * BT_HID_SERVICE_DISCOVERY_CHARACTER_MAX_NUMBER * BT_HID_SERVICE_DISCOVERY_DESCRIPTOR_MAX_NUMBER);
    for (j = 0; j < BT_HID_SERVICE_DISCOVERY_CHARACTER_MAX_NUMBER; j++) {
        g_hid_service_discovery_character[j].descriptor_count = BT_HID_SERVICE_DISCOVERY_DESCRIPTOR_MAX_NUMBER;
        g_hid_service_discovery_character[j].descriptor = &g_hid_service_discovery_descriptor[j * BT_HID_SERVICE_DISCOVERY_DESCRIPTOR_MAX_NUMBER];
    }

    memset(&g_hid_service_discovery, 0, sizeof(bt_gattc_discovery_service_t));
    g_hid_service_discovery.characteristic_count = BT_HID_SERVICE_DISCOVERY_CHARACTER_MAX_NUMBER;
    g_hid_service_discovery.charateristics = g_hid_service_discovery_character;
    user_data.uuid.type = BLE_UUID_TYPE_16BIT;
    user_data.uuid.uuid.uuid16 = BT_SIG_UUID16_HID;
    user_data.need_cache = true;
    user_data.srv_info = &g_hid_service_discovery;
    user_data.handler = bt_hogp_client_service_discovery_callback;
    ret = bt_gattc_discovery_register_service(BT_GATTC_DISCOVERY_USER_HOGP, &user_data);
    LOG_MSGID_I(BT_HOGP_CLIENT,"[BT_HOGP_CLIENT] init HOGP Client, ret:%x", 1, ret);
    bt_callback_manager_register_callback(bt_callback_type_app_event,
                                          (uint32_t)(MODULE_MASK_GAP | MODULE_MASK_GATT | MODULE_MASK_SYSTEM),
                                          (void *)bt_hogp_client_common_event_callback);
    return BT_STATUS_SUCCESS;
}
