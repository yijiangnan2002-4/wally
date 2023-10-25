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

#include "bt_gap_le_service.h"
#include "bt_connection_manager_internal.h"
#include "bt_device_manager_le.h"
#include "bt_device_manager_db.h"
#include "bt_device_manager_internal.h"
#include "bt_device_manager_link_record.h"
#include "FreeRTOS.h"
#include "task.h"
#include "bt_utils.h"
#ifdef MTK_AWS_MCE_ENABLE
#include "bt_aws_mce_report.h"
#include "bt_aws_mce_srv.h"
#endif
#include "bt_callback_manager.h"

#if _MSC_VER >= 1500
#pragma comment(linker, "/alternatename:_bt_device_manager_link_record_takeover_callback=_default_bt_device_manager_link_record_takeover_callback")
#elif defined(__GNUC__) || defined(__ICCARM__) || defined(__CC_ARM)
#pragma weak bt_device_manager_link_record_takeover_callback = default_bt_device_manager_link_record_takeover_callback
#else
#error "Unsupported Platform"
#endif

static uint8_t g_dm_link_record_buffer_size = 2;
static bt_device_manager_link_record_t g_dm_link_record_cnt;

static void         default_bt_device_manager_link_record_takeover_callback(const bt_device_manager_link_record_item_t *item)
{
    bt_dmgr_report_id("[BT_DM][LINK_RECORD][I] BT Address:0x%x take over, user need implement it", 1, *(uint32_t*)(&item->remote_addr));
}

static uint8_t      bt_device_manager_link_record_find(bt_device_manager_link_t link_type, bt_bd_addr_t *remote_addr)
{
    for (uint8_t i = 0; i < g_dm_link_record_cnt.connected_num; i++) {
        if (!memcmp(&g_dm_link_record_cnt.connected_device[i].remote_addr, remote_addr, sizeof(bt_bd_addr_t)) &&
            link_type == g_dm_link_record_cnt.connected_device[i].link_type) {
            return i;
        }
    }
    return 0xFF;
}

#ifdef MTK_AWS_MCE_ENABLE
static void         bt_device_manager_link_record_sync_info_partner()
{
    bt_aws_mce_srv_mode_t current_mode = bt_device_manager_aws_local_info_get_mode();
    if (current_mode == BT_AWS_MCE_SRV_MODE_BROADCAST || current_mode == BT_AWS_MCE_SRV_MODE_DOUBLE) {
        bt_dmgr_report_id("[BT_DM][LINK_RECORD][I] speaker mode not need sync", 0);
        return;
    }
    bt_status_t status;
    uint32_t data_length = sizeof(g_dm_link_record_cnt);
    uint32_t report_length = sizeof(bt_aws_mce_report_info_t) + data_length + 1;
    bt_aws_mce_report_info_t *dm_report = bt_utils_memory_alloc(report_length);
    if (NULL == dm_report) {
        return;
    }
    memset(dm_report, 0 , sizeof(report_length));
    uint8_t *data_payload = ((uint8_t *)dm_report) + sizeof(bt_aws_mce_report_info_t);
    dm_report->module_id = BT_AWS_MCE_REPORT_MODULE_DM;
    dm_report->param_len = report_length - sizeof(bt_aws_mce_report_info_t);
    dm_report->param = data_payload;
    data_payload[0] = BT_DEVICE_MANAGER_DB_TYPE_LINK_RECORD_INFO;
    memcpy(data_payload + 1, (void*)&g_dm_link_record_cnt, data_length);
    if (BT_STATUS_SUCCESS != (status = bt_aws_mce_report_send_event(dm_report))) {
        bt_dmgr_report_id("[BT_DM][LINK_RECORD][E] Send AWS packet failed status 0x%x.", 1, status);
    }
    bt_utils_memory_free(dm_report);
}
#endif

static void         bt_device_manager_link_record_add(bt_bd_addr_t *address, bt_device_manager_link_t link_type, bt_addr_type_t bd_type)
{
    if (0xFF != bt_device_manager_link_record_find(link_type, address)) {
        bt_dmgr_report_id("[BT_DM][LINK_RECORD][E] Duplicated connected event ", 0);
        return;
    }
    if (g_dm_link_record_cnt.connected_num >= BT_DEVICE_MANAGER_LINK_RECORD_MAXIMUM) {
        bt_dmgr_report_id("[BT_DM][LINK_RECORD][E] Context full !!!", 0);
        return;
    }
    g_dm_link_record_cnt.connected_num++;
    bt_dmgr_report_id("[BT_DM][LINK_RECORD][I] Device:0x%x type:0x%x add, now num:%d", 3, *(uint32_t*)address, link_type, g_dm_link_record_cnt.connected_num);
    bt_device_manager_link_record_item_t temp_cntx[BT_DEVICE_MANAGER_LINK_RECORD_MAXIMUM - 1];
    memcpy(&temp_cntx, &(g_dm_link_record_cnt.connected_device), sizeof(temp_cntx));
    memcpy(&g_dm_link_record_cnt.connected_device[1],&temp_cntx, sizeof(temp_cntx));
    g_dm_link_record_cnt.connected_device[0].link_type = link_type;
    g_dm_link_record_cnt.connected_device[0].remote_bd_type = bd_type;
    memcpy(&(g_dm_link_record_cnt.connected_device[0].remote_addr), address, sizeof(bt_bd_addr_t));
    if (g_dm_link_record_cnt.connected_num >= g_dm_link_record_buffer_size + 1) {
        bt_device_manager_link_record_takeover_callback(&g_dm_link_record_cnt.connected_device[0]);
    }
}

static void         bt_device_manager_link_record_delete(bt_device_manager_link_t link_type, bt_bd_addr_t *address)
{
    uint8_t found_index = bt_device_manager_link_record_find(link_type, address);
    bt_dmgr_report_id("[BT_DM][LINK_RECORD][I] Device:0x%x type:0x%x deleted, before num:%d, found_index:%d", 4,
        *(uint32_t*)address, link_type, g_dm_link_record_cnt.connected_num, found_index);
    if (0xFF == found_index) {
        return;
    }
    for (uint8_t i = found_index; i < g_dm_link_record_cnt.connected_num - 1; i++) {
        g_dm_link_record_cnt.connected_device[i] = g_dm_link_record_cnt.connected_device[i+1];
    }
    g_dm_link_record_cnt.connected_num--;

}

static bt_status_t  bt_device_manager_link_record_gap_event_callback(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    if (NULL == buff || BT_GAP_LE_BONDING_COMPLETE_IND != msg) {
        return BT_STATUS_SUCCESS;
    }
    bt_gap_le_srv_conn_info_t *conn_info = bt_gap_le_srv_get_conn_info(((bt_gap_le_bonding_complete_ind_t *)buff)->handle);
    bt_gap_le_bonding_info_t *bonding_info = NULL;
    if (NULL == conn_info || !(conn_info->link_type & BT_GAP_LE_SRV_LINK_TYPE_LE_AUDIO)) {
        bt_dmgr_report_id("[BT_DM][LINK_RECORD][I] Get le audio conn info fail 0x%x 0x%x", 2, conn_info, (conn_info != NULL ? conn_info->link_type : 0xFFFF));
    } else if (NULL == (bonding_info = bt_device_manager_le_get_bonding_info_by_addr(&(conn_info->peer_addr.addr))) ||
        (0xFF == bonding_info->identity_addr.address.type)) {
        bt_device_manager_link_record_add(&(conn_info->peer_addr.addr), BT_DEVICE_MANAGER_LINK_TYPE_LE, conn_info->peer_addr.type);
    } else {
        bt_device_manager_link_record_add(&(bonding_info->identity_addr.address.addr), BT_DEVICE_MANAGER_LINK_TYPE_LE, bonding_info->identity_addr.address.type);
    }
    return BT_STATUS_SUCCESS;
}

static void         bt_device_manager_link_record_gap_le_service_event_callback(bt_gap_le_srv_event_t event, void *data)
{
    bt_gap_le_srv_disconnect_ind_t *remote_update = (bt_gap_le_srv_disconnect_ind_t *)data;
    bt_gap_le_bonding_info_t *bonding_info = NULL;
    if (NULL == remote_update || BT_GAP_LE_SRV_EVENT_DISCONNECT_IND != event || !(BT_GAP_LE_SRV_LINK_TYPE_LE_AUDIO & remote_update->link_type)) {
        bt_dmgr_report_id("[BT_DM][LINK_RECORD][I] BLE event 0x%x, data 0x%x", 2, event, remote_update);
        return;
    }
    bt_device_manager_link_record_delete(BT_DEVICE_MANAGER_LINK_TYPE_LE, &(remote_update->peer_address.addr));
    if (NULL != (bonding_info = bt_device_manager_le_get_bonding_info_by_addr(&(remote_update->peer_address.addr)))) {
        bt_device_manager_link_record_delete(BT_DEVICE_MANAGER_LINK_TYPE_LE, &(bonding_info->identity_addr.address.addr));    
    }
    return;
}

static bt_status_t  bt_device_manager_link_record_cm_event_callback(bt_cm_event_t event_id, void *params, uint32_t params_len)
{
    bt_cm_remote_info_update_ind_t *remote_update = (bt_cm_remote_info_update_ind_t *)params;
    if (NULL == remote_update || BT_CM_EVENT_REMOTE_INFO_UPDATE != event_id) {
        bt_dmgr_report_id("[BT_DM][LINK_RECORD][I] BT Classic event 0x%x, data 0x%x", 2, event_id, remote_update);
        return BT_STATUS_SUCCESS;
    }
#ifdef MTK_AWS_MCE_ENABLE
    if (BT_AWS_MCE_ROLE_AGENT != bt_device_manager_aws_local_info_get_role() &&
        BT_AWS_MCE_ROLE_NONE != bt_device_manager_aws_local_info_get_role()) {
        return BT_STATUS_SUCCESS;
    }
    /* Sync link record to partner after aws connected. */
    if (!(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service) &&
        (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service)) {
        bt_device_manager_link_record_sync_info_partner();
    }
#endif
    if (true == bt_cm_is_specail_device(&(remote_update->address))) {
        return BT_STATUS_SUCCESS;
    }
    if (remote_update->pre_acl_state < BT_CM_ACL_LINK_CONNECTED && remote_update->acl_state >= BT_CM_ACL_LINK_CONNECTED) {
        /* Classic BT connected. */
        bt_device_manager_link_record_add(&remote_update->address, BT_DEVICE_MANAGER_LINK_TYPE_EDR, BT_ADDR_PUBLIC);
#ifdef MTK_AWS_MCE_ENABLE
        bt_device_manager_link_record_sync_info_partner();
#endif
    } else if (remote_update->acl_state == BT_CM_ACL_LINK_DISCONNECTED) {
        /* Classic BT disconnected. */
        bt_device_manager_link_record_delete(BT_DEVICE_MANAGER_LINK_TYPE_EDR, &remote_update->address);
#ifdef MTK_AWS_MCE_ENABLE
        bt_device_manager_link_record_sync_info_partner();
#endif
    }
    return BT_STATUS_SUCCESS;
}

#if 0
bt_status_t         bt_device_manager_link_record_set_sequence()
{
    return BT_STATUS_FAIL;
}
#endif

const bt_device_manager_link_record_t *
                    bt_device_manager_link_record_get_connected_link()
{
    return &g_dm_link_record_cnt;
}

#ifdef MTK_AWS_MCE_ENABLE
void                bt_device_manager_link_record_aws_update_context(const bt_device_manager_link_record_t *src)
{
    if (NULL == src) {
        bt_dmgr_report_id("[BT_DM][LINK_RECORD][E] Aws update context with null pointer", 0);
        return;
    }
    bt_dmgr_report_id("[BT_DM][LINK_RECORD][I] Dst connected:%d, Src connected:%d", 2, g_dm_link_record_cnt.connected_num, src->connected_num);
    uint8_t src_num = src->connected_num;
    uint8_t dst_num = g_dm_link_record_cnt.connected_num;
    uint8_t src_index = 0, dst_index = 0;
    bt_device_manager_link_record_item_t dst_cntx[BT_DEVICE_MANAGER_LINK_RECORD_MAXIMUM];
    memcpy(&dst_cntx, &g_dm_link_record_cnt.connected_device[0], sizeof(dst_cntx));

    g_dm_link_record_cnt.connected_num = 0;
    while (src_index < src_num || dst_index < dst_num) {
        if (src_index < src_num && BT_DEVICE_MANAGER_LINK_TYPE_LE != src->connected_device[src_index].link_type) {
            g_dm_link_record_cnt.connected_device[g_dm_link_record_cnt.connected_num++] = src->connected_device[src_index++];
            continue;
        }
        if (dst_index < dst_num && BT_DEVICE_MANAGER_LINK_TYPE_LE == dst_cntx[dst_index].link_type) {
            g_dm_link_record_cnt.connected_device[g_dm_link_record_cnt.connected_num++] = dst_cntx[dst_index];
        }
        src_index++;
        dst_index++;
    }
    bt_dmgr_report_id("[BT_DM][LINK_RECORD][I] Connected num %d after merged", 1, g_dm_link_record_cnt.connected_num);
}
#endif

void                bt_device_manager_link_record_set_max_num(uint8_t size)
{
    bt_dmgr_report_id("[BT_DM][LINK_RECORD][I] set max number %d", 1, size);
    g_dm_link_record_buffer_size = size;
}

bt_status_t         bt_device_manager_link_record_init()
{
    bt_dmgr_report_id("[BT_DM][LINK_RECORD][I] link_record_init", 0);
    memset(&g_dm_link_record_cnt, 0, sizeof(g_dm_link_record_cnt));
    bt_callback_manager_register_callback(bt_callback_type_app_event, (uint32_t)MODULE_MASK_GAP, (void *)bt_device_manager_link_record_gap_event_callback);
    bt_gap_le_srv_register_event_callback(&bt_device_manager_link_record_gap_le_service_event_callback);
    bt_cm_register_event_callback(&bt_device_manager_link_record_cm_event_callback);
    return BT_STATUS_SUCCESS;
}

