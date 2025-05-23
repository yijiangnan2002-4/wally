/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

#ifdef MTK_NVDM_ENABLE
#include "nvkey.h"
#include "nvkey_id_list.h"
#endif /* MTK_NVDM_ENABLE */
#include "bt_callback_manager.h"
#include "bt_device_manager_config.h"
#include "bt_device_manager.h"
#include "bt_device_manager_internal.h"
#include "bt_gap.h"
#include "bt_gap_le.h"
#include "bt_type.h"
#include "bt_os_layer_api.h"
#include <string.h>
#include <stdio.h>
#include "syslog.h"
#include "bt_device_manager_db.h"
#include "nvdm_id_list.h"
#include "bt_device_manager_test_mode.h"
#include "bt_device_manager_gatt_cache.h"

#define __BT_DEVICE_MANAGER_DEBUG_INFO__

void bt_sink_paird_list_changed(bt_device_manager_paired_event_t event, bt_bd_addr_ptr_t address);
void default_bt_sink_paird_list_changed(bt_device_manager_paired_event_t event, bt_bd_addr_ptr_t address);
//static bt_status_t bt_device_manager_delete_paired_device_int(bt_bd_addr_ptr_t address);

/* Weak symbol declaration */
#if _MSC_VER >= 1500
#pragma comment(linker, "/alternatename:_bt_sink_paird_list_changed=_default_bt_sink_paird_list_changed")
#elif defined(__GNUC__) || defined(__ICCARM__) || defined(__CC_ARM)
#pragma weak bt_sink_paird_list_changed = default_bt_sink_paird_list_changed
#else
#error "Unsupported Platform"
#endif

static bt_device_manager_db_local_info_t local_info;
static int32_t bt_device_manager_is_init;
static bt_gap_io_capability_t bt_device_manager_io_capability = 0xFF;
static bt_device_manager_mode_type_t dev_mode = BT_DEVICE_MANAGER_NORMAL_MODE;
static bt_bd_addr_t read_name_address;

log_create_module(BT_DM_EDR, PRINT_LEVEL_INFO);

void bt_device_manager_dump_link_key(uint8_t *linkkey)
{
    bt_dmgr_report_id("[BT_DM] link key:%02x,%02x,%02x,%02x,  %02x,%02x,%02x,%02x,  %02x,%02x,%02x,%02x,  %02x,%02x,%02x,%02x", 16,
                      linkkey[0], linkkey[1], linkkey[2], linkkey[3], linkkey[4], linkkey[5], linkkey[6], linkkey[7],
                      linkkey[8], linkkey[9], linkkey[10], linkkey[11], linkkey[12], linkkey[13], linkkey[14], linkkey[15]);
}

void bt_device_manager_dump_bt_address(bt_device_manager_local_info_address_type_t addr_type, uint8_t *address)
{
    bt_dmgr_report_id("[BT_DM] Addr type %d, address:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x", 7, addr_type,
                      address[5], address[4], address[3], address[2], address[1], address[0]);
}

void default_bt_sink_paird_list_changed(bt_device_manager_paired_event_t event, bt_bd_addr_ptr_t address)
{
    return;
}

/*
static void bt_device_manager_notify_paired_list_changed(bt_device_manager_paired_event_t event, bt_bd_addr_ptr_t address)
{
    bt_sink_paird_list_changed(event, address);
    return;
}
*/

bt_status_t bt_device_manager_gap_event_handler(bt_msg_type_t msg, bt_status_t status, void *buff);
void bt_device_manager_get_link_key_handler(bt_gap_link_key_notification_ind_t *key_information);

void bt_device_manager_init(void)
{
    bt_dmgr_report_id("[BT_DM][I]local info init", 0);
    if (bt_device_manager_is_init == 0) {
        bt_device_manager_test_mode_init();
        bt_callback_manager_register_callback(bt_callback_type_app_event, MODULE_MASK_GAP | MODULE_MASK_SYSTEM, (void *)bt_device_manager_gap_event_handler);
        bt_callback_manager_register_callback(bt_callback_type_gap_get_link_key, MODULE_MASK_GAP, (void *)bt_device_manager_get_link_key_handler);
        bt_device_manager_db_storage_t local_storage = {
            .auto_gen = true,
            .storage_type = BT_DEVICE_MANAGER_DB_STORAGE_TYPE_NVKEY,
            .nvkey_id = NVID_BT_HOST_DM_LOCAL_CONFIG,
        };
        memset((void *)&local_info, 0, sizeof(local_info));
        bt_device_manager_db_init(BT_DEVICE_MANAGER_DB_TYPE_LOCAL_INFO, &local_storage, &local_info, sizeof(local_info));
        bt_device_manager_remote_info_init();
        bt_device_manager_gatt_cache_init();
#ifdef MTK_AWS_MCE_ENABLE
        bt_device_manager_aws_local_info_init();
#endif
        bt_device_manager_is_init = 1;
    }
}

uint32_t bt_device_manager_get_paired_number(void)
{
    return bt_device_manager_remote_get_paired_num();
}

void bt_device_manager_get_paired_list(bt_device_manager_paired_infomation_t *info, uint32_t *read_count)
{
    bt_device_manager_remote_get_paired_list(info, read_count);
}

bool bt_device_manager_is_paired(bt_bd_addr_ptr_t address)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_device_manager_db_remote_paired_info_t temp_info;
    status = bt_device_manager_remote_find_paired_info((uint8_t *)address, &temp_info);
    if (status != BT_STATUS_SUCCESS) {
        return false;
    }
    return true;
}

bt_status_t bt_device_manager_delete_paired_device(bt_bd_addr_ptr_t address)
{
       bt_dmgr_report_id("[BT_DM][REMOTE][I] Delete info from bt_device_manager_delete_paired_device",0);

    return bt_device_manager_remote_delete_info((bt_bd_addr_t *)address, BT_DEVICE_MANAGER_REMOTE_INFO_MASK_PAIRED);
}

void bt_device_manager_get_link_key_handler(bt_gap_link_key_notification_ind_t *key_information)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_device_manager_db_remote_paired_info_t temp_info;
    memset(&temp_info, 0, sizeof(temp_info));
    status = bt_device_manager_remote_find_paired_info(key_information->address, &temp_info);
    if (status == BT_STATUS_SUCCESS) {
        memcpy(key_information, &(temp_info.paired_key), sizeof(bt_gap_link_key_notification_ind_t));
#ifdef __BT_DEVICE_MANAGER_DEBUG_INFO__
        bt_device_manager_dump_bt_address(BT_DEVICE_MANAGER_LOCAL_INFO_ADDRESS_REMOTE, (uint8_t *)key_information->address);
        bt_device_manager_dump_link_key((uint8_t *)key_information->key);
#endif
    } else {
        key_information->key_type = BT_GAP_LINK_KEY_TYPE_INVALID;
    }
}

bt_status_t bt_device_manager_unpair_all(void)
{
       bt_dmgr_report_id("[BT_DM][REMOTE][I] Delete info from bt_device_manager_unpair_all",0);
    return bt_device_manager_remote_delete_info(NULL, 0);
}

void bt_device_manager_set_io_capability(bt_gap_io_capability_t io_capability)
{
    bt_device_manager_io_capability = io_capability;
}

bt_status_t bt_device_manager_gap_event_handler(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    bt_dmgr_report_id("[BT_DM][I] Basic event msg:0x%08x status 0x%x", 2, msg, status);
    switch (msg) {
        case BT_GAP_IO_CAPABILITY_REQ_IND: {
            if (0xFF == bt_device_manager_io_capability) {
                bt_gap_reply_io_capability_request(BT_GAP_OOB_DATA_PRESENTED_NONE, BT_GAP_SECURITY_AUTH_REQUEST_GENERAL_BONDING_AUTO_ACCEPTED);
            } else {
                bt_gap_reply_extend_io_capability_request(BT_GAP_OOB_DATA_PRESENTED_NONE,
                                                          BT_GAP_SECURITY_AUTH_REQUEST_MITM_DEDICATED_BONDING, bt_device_manager_io_capability);
            }
            break;
        }
        case BT_GAP_LINK_KEY_NOTIFICATION_IND: {
            /* This event is received before BT_GAP_BONDING_COMPLETE_IND and bonding success or the old link key is phased out. */
            bt_gap_link_key_notification_ind_t *key_info = (bt_gap_link_key_notification_ind_t *)buff;
            if (key_info->key_type != BT_GAP_LINK_KEY_TYPE_INVALID) {
                bt_device_manager_db_remote_paired_info_t paired_info;
                memset(&paired_info, 0, sizeof(paired_info));
                bt_device_manager_remote_find_paired_info(key_info->address, &paired_info);
                memcpy(&(paired_info.paired_key), key_info, sizeof(bt_gap_link_key_notification_ind_t));
                bt_device_manager_remote_update_paired_info(key_info->address, &paired_info);
            } else {
                   bt_dmgr_report_id("[BT_DM][REMOTE][I] Delete info from bt_device_manager_gap_event_handler BT_GAP_LINK_KEY_NOTIFICATION_IND",0);
    
                bt_device_manager_remote_delete_info(&(key_info->address), BT_DEVICE_MANAGER_REMOTE_INFO_MASK_PAIRED);
            }
#ifdef __BT_DEVICE_MANAGER_DEBUG_INFO__
            bt_device_manager_dump_bt_address(BT_DEVICE_MANAGER_LOCAL_INFO_ADDRESS_REMOTE, (uint8_t *)key_info->address);
            bt_device_manager_dump_link_key((uint8_t *)key_info->key);
#endif
            break;
        }
        case BT_GAP_LINK_STATUS_UPDATED_IND: {
            bt_gap_link_status_updated_ind_t *param = (bt_gap_link_status_updated_ind_t *)buff;
            if (BT_STATUS_SUCCESS == status && BT_GAP_LINK_STATUS_CONNECTED_0 < param->link_status) {
                char name[BT_DEVICE_MANAGER_NAME_LENGTH] = {0};
                if (BT_STATUS_SUCCESS != bt_device_manager_remote_find_name((void *)(param->address), name, sizeof(name))) {
                    /* To avoid IOT issue, remove the name request to encryption changed on.*/
                    memcpy(&read_name_address, param->address, sizeof(bt_bd_addr_t));
                    bt_gap_read_remote_name(param->address);
                    bt_gap_read_remote_version_information(param->handle);
                }
            }
            if (BT_GAP_LINK_STATUS_DISCONNECTED == param->link_status && false == bt_device_manager_is_paired((void *)(param->address))) {
                bt_dmgr_report_id("[BT_DM][REMOTE][I] Delete info from bt_device_manager_gap_event_handler BT_GAP_LINK_STATUS_UPDATED_IND",0);
                bt_device_manager_remote_delete_info((void *)(param->address), 0);
            }
            break;
        }
        case BT_GAP_READ_REMOTE_NAME_COMPLETE_IND: {
            bt_gap_read_remote_name_complete_ind_t *p = (bt_gap_read_remote_name_complete_ind_t *)buff;
            if (NULL != p && BT_STATUS_SUCCESS == status) {
                bt_device_manager_remote_update_name((uint8_t *)(p->address), (char *)(p->name));
            }
            break;
        }
        case BT_GAP_READ_REMOTE_VERSION_COMPLETE_IND: {
            bt_gap_read_remote_version_complete_ind_t *complete_ind = (bt_gap_read_remote_version_complete_ind_t *)buff;
            bt_device_manager_db_remote_version_info_t version_info;
            bt_bd_addr_t *remote_device = NULL;
            if (BT_STATUS_SUCCESS == status && NULL != complete_ind && NULL != (remote_device = (void *)bt_gap_get_remote_address(complete_ind->handle))) {
                version_info.version = complete_ind->version;
                version_info.manufacturer_id = complete_ind->manufacturer_id;
                version_info.subversion = complete_ind->subversion;
                bt_device_manager_remote_update_version_info(*remote_device, &version_info);
            } else {
                bt_dmgr_report_id("[BT_DM][E] Read remote version can't find device by handle",0);
            }
            break;
        }
        #if 0
        case BT_GAP_READ_REMOTE_NAME_CNF: {
            if (status != BT_STATUS_SUCCESS) {
                bt_dmgr_report_id("[BT_DM][W] read remote name cnf fail status 0x%x", 1, status);
                //bt_gap_read_remote_name(&read_name_address);
            }
            break;
        }
        case BT_GAP_READ_REMOTE_VERSION_CNF: {
            if (status != BT_STATUS_SUCCESS) {
                bt_dmgr_report_id("[BT_DM][W] read remote version cnf fail status 0x%x", 1, status);
            }
            break;
        }
        #endif
        case BT_POWER_ON_CNF: {
            bt_device_manager_power_on_cnf();
            break;
        }
        case BT_POWER_OFF_CNF: {
            bt_device_manager_power_off_cnf();
            break;
        }
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}

uint32_t bt_device_manager_get_complete_paired_list(bt_device_manager_paired_device_complete_infomation_t *info, uint32_t read_count)
{
    return 0;
}

bt_status_t bt_device_manager_get_complete_paired_info(bt_bd_addr_ptr_t address, bt_device_manager_paired_device_complete_infomation_t *info)
{
    bt_dmgr_report_id("[BT_DM][I] Get complete paired info 0x%x", 1, info);
    if (NULL == address || NULL == info) {
        return BT_STATUS_FAIL;
    }
    memset(info, 0, sizeof(*info));
    return bt_device_manager_remote_find_paired_info((void *)address, (void *)info);
}

bt_status_t bt_device_manager_set_complete_paired_info(bt_bd_addr_ptr_t address, bt_device_manager_paired_device_complete_infomation_t *info)
{
    return bt_device_manager_remote_update_paired_info((void *)address, (void *)info);
}

void bt_device_manager_store_local_address(bt_bd_addr_t *addr)
{
    bt_dmgr_report_id("[BT_DM][I] Store local address:", 0);
    bt_device_manager_dump_bt_address(BT_DEVICE_MANAGER_LOCAL_INFO_ADDRESS_LOCAL, (uint8_t *)addr);
    if (memcmp(&local_info.local_address, addr, sizeof(bt_bd_addr_t))) {
        bt_device_manager_db_mutex_take();
        memcpy(&local_info.local_address, addr, sizeof(bt_bd_addr_t));
        bt_device_manager_db_mutex_give();
        bt_device_manager_db_update(BT_DEVICE_MANAGER_DB_TYPE_LOCAL_INFO);
    }
    bt_device_manager_db_flush(BT_DEVICE_MANAGER_DB_TYPE_LOCAL_INFO, BT_DEVICE_MANAGER_DB_FLUSH_BLOCK);
#ifdef MTK_AWS_MCE_ENABLE
    bt_device_manager_aws_local_info_store_local_address(addr);
    bt_device_manager_aws_local_info_update();
    bt_device_manager_db_flush(BT_DEVICE_MANAGER_DB_TYPE_AWS_LOCAL_INFO, BT_DEVICE_MANAGER_DB_FLUSH_BLOCK);
#endif
}

bt_bd_addr_t *bt_device_manager_get_local_address_internal(void)
{
    bt_dmgr_report_id("[BT_DM][I] Get local address:", 0);
    bt_device_manager_dump_bt_address(BT_DEVICE_MANAGER_LOCAL_INFO_ADDRESS_LOCAL, (uint8_t *) & (local_info.local_address));
    return &local_info.local_address;
}

void bt_device_manager_store_local_address_internal(bt_bd_addr_t *addr)
{
    bt_dmgr_report_id("[BT_DM][I] Set local address internal:", 0);
    bt_device_manager_dump_bt_address(BT_DEVICE_MANAGER_LOCAL_INFO_ADDRESS_LOCAL, (uint8_t *)addr);
#ifdef MTK_AWS_MCE_ENABLE
    bt_device_manager_aws_local_info_store_local_address(addr);
#else
    if (memcmp(&local_info.local_address, addr, sizeof(bt_bd_addr_t))) {
        bt_device_manager_db_mutex_take();
        memcpy(&local_info.local_address, addr, sizeof(bt_bd_addr_t));
        bt_device_manager_db_mutex_give();
        bt_device_manager_db_update(BT_DEVICE_MANAGER_DB_TYPE_LOCAL_INFO);
    }
#endif
}

bt_bd_addr_t *bt_device_manager_get_local_address(void)
{
#ifdef MTK_AWS_MCE_ENABLE
    return bt_device_manager_aws_local_info_get_local_address();
#else
    return bt_device_manager_get_local_address_internal();
#endif
}

#ifdef MTK_BT_SPEAKER_ENABLE
bool bt_connection_manager_device_local_info_get_local_music_volume(uint16_t *volume_data)
{
    *volume_data = local_info.local_music_volume;
    return true;
}

void bt_connection_manager_device_local_info_set_local_music_volume(uint16_t *volume_data)
{
    if (local_info.local_music_volume != *volume_data) {
        bt_device_manager_db_mutex_take();
        local_info.local_music_volume = *volume_data;
        bt_device_manager_db_mutex_give();
        bt_device_manager_db_update(BT_DEVICE_MANAGER_DB_TYPE_LOCAL_INFO);
        bt_device_manager_db_flush(BT_DEVICE_MANAGER_DB_TYPE_LOCAL_INFO, BT_DEVICE_MANAGER_DB_FLUSH_BLOCK);
    }
}
#endif


void bt_device_manager_set_device_mode(bt_device_manager_mode_type_t mode)
{
    bt_dmgr_report_id("[BT_DM][I] Set device mode = %d:", 1, mode);
    dev_mode = mode;
}

