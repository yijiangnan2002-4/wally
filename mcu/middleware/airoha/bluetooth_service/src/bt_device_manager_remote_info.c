/* Copyright Statement:
 *
 * (C) 2019  Airoha Technology Corp. All rights reserved.
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

#include "FreeRTOS.h"
#ifdef MTK_NVDM_ENABLE
#include "nvkey.h"
#include "nvkey_id_list.h"
#endif /* MTK_NVDM_ENABLE */
#include "bt_gap.h"
#include "bt_gap_le.h"
#include "bt_device_manager.h"
#include "bt_callback_manager.h"
#include "bt_device_manager_config.h"
#include "bt_device_manager_internal.h"
#include "bt_device_manager_db.h"
#ifdef MTK_AWS_MCE_ENABLE
#include "bt_aws_mce_report.h"
#endif
#include "bt_iot_device_white_list.h"
#include "bt_timer_external.h"
#include "bt_device_manager_link_record.h"
#include "bt_utils.h"
#include "bt_device_manager_nvkey_struct.h"

#define UNUSED(x)  ((void)(x))

#ifndef AIR_BT_DEVICE_NAME_IN_PAIRING_DB
#define BT_DM_REMOTE_NON_FLUSH_INFO_MASK_NAME       (0x01)
#endif /* AIR_BT_DEVICE_NAME_IN_PAIRING_DB */
#define BT_DM_REMOTE_NON_FLUSH_INFO_MASK_VERSION    (0x02)
#define BT_DM_REMOTE_NON_FLUSH_INFO_MASK_PNP        (0x04)
#define BT_DM_REMOTE_NON_FLUSH_INFO_MASK_COD        (0x08)
typedef uint8_t bt_dm_remote_non_flush_info_mask_t;

#define BT_DM_REMOTE_NON_FLUSH_RECORD_MAXIMUM       (0x03)

typedef struct {
    bt_bd_addr_t address;
    bt_dm_remote_non_flush_info_mask_t info_valid_set;
#ifndef AIR_BT_DEVICE_NAME_IN_PAIRING_DB
    char remote_name[BT_DEVICE_MANAGER_NAME_LENGTH];
#endif /* ndef AIR_BT_DEVICE_NAME_IN_PAIRING_DB */
    bt_device_manager_db_remote_version_info_t  version_info;
    bt_device_manager_db_remote_pnp_info_t      pnp_info;
    uint32_t     cod;
} bt_device_manager_remote_non_flush_info_item_t;

typedef struct {
    uint8_t     set_index;
    bt_device_manager_remote_non_flush_info_item_t item[BT_DM_REMOTE_NON_FLUSH_RECORD_MAXIMUM];
} bt_device_manager_remote_non_flush_info_t;

static uint8_t g_bt_dm_remote_sequence[BT_DEVICE_MANAGER_MAX_PAIR_NUM];
static bt_device_manager_db_remote_info_t g_bt_dm_remote_list_cnt[BT_DEVICE_MANAGER_MAX_PAIR_NUM];
static bt_device_manager_remote_non_flush_info_t g_dm_remote_non_flush_list_cnt;

#ifdef MTK_AWS_MCE_ENABLE
static void bt_device_manager_aws_mce_packet_callback(bt_aws_mce_report_info_t *para);
static void bt_device_manager_remote_aws_sync_db(bt_device_manager_db_type_t type, bool flush_at_once, uint16_t data_length, uint8_t *data);
#endif

/*
void bt_device_manager_test_dump_device_info()
{
    bt_device_manager_db_remote_info_t *temp_remote = &g_bt_dm_remote_list_cnt[0];
    for (uint32_t index = 0; index < BT_DEVICE_MANAGER_MAX_PAIR_NUM; index++, temp_remote++) {
        if (g_bt_dm_remote_sequence[index]) {
            bt_dmgr_report_id("[BT_DM][REMOTE][I] Dump device index:%d, sequence:%d %02x:%02x:%02x:%02x:%02x:%02x", 8,
                    index, g_bt_dm_remote_sequence[index],
                    temp_remote->address[0], temp_remote->address[1], temp_remote->address[2],
                    temp_remote->address[3], temp_remote->address[4], temp_remote->address[5]);
            bt_dmgr_report_id("[BT_DM][REMOTE][I] Dump device invalid flag:0x%x, iot_id:%d, supported_profies:0x%x", 3,
                    temp_remote->info_valid_flag, temp_remote->iot_id, temp_remote->supported_profiles);
        }
    }
    for (uint32_t find_index = 0; find_index < BT_DM_REMOTE_NON_FLUSH_RECORD_MAXIMUM; find_index++) {
        if (g_dm_remote_non_flush_list_cnt.item[find_index].info_valid_set) {
            bt_dmgr_report_id("[BT_DM][REMOTE][I] Dump non-flush device index:%d, set index:%d, invalid:%x, %02x:%02x:%02x:%02x:%02x:%02x", 9,
                    find_index, g_dm_remote_non_flush_list_cnt.set_index,g_dm_remote_non_flush_list_cnt.item[find_index].info_valid_set,
                    g_dm_remote_non_flush_list_cnt.item[find_index].address[0], g_dm_remote_non_flush_list_cnt.item[find_index].address[1],
                    g_dm_remote_non_flush_list_cnt.item[find_index].address[2], g_dm_remote_non_flush_list_cnt.item[find_index].address[3],
                    g_dm_remote_non_flush_list_cnt.item[find_index].address[4], g_dm_remote_non_flush_list_cnt.item[find_index].address[5]);
        }
    }
}*/

void bt_device_manager_remote_info_init(void)
{
    bt_dmgr_report_id("[BT_DM][REMOTE][I] Remote info init Maximum pair num %d", 1, BT_DEVICE_MANAGER_MAX_PAIR_NUM);
    bt_device_manager_db_storage_t remote_storage = {
        .auto_gen = true,
        .storage_type = BT_DEVICE_MANAGER_DB_STORAGE_TYPE_NVKEY,
        .nvkey_id = NVID_BT_HOST_REMOTE_INFO_01
    };
    if (BT_DEVICE_MANAGER_MAX_PAIR_NUM > 16) {
        bt_utils_assert(0 && "Device maximum number exceed the storage can support");
    } else if (0 == BT_DEVICE_MANAGER_MAX_PAIR_NUM) {
        return;
    }
    memset(&g_dm_remote_non_flush_list_cnt, 0, sizeof(g_dm_remote_non_flush_list_cnt));
    memset(g_bt_dm_remote_list_cnt, 0, sizeof(g_bt_dm_remote_list_cnt));
    memset(g_bt_dm_remote_sequence, 0, sizeof(g_bt_dm_remote_sequence));
    for (uint32_t index = 0; index < BT_DEVICE_MANAGER_MAX_PAIR_NUM; index++) {
        remote_storage.nvkey_id = NVID_BT_HOST_REMOTE_INFO_01 + index;
        bt_device_manager_db_init(BT_DEVICE_MANAGER_DB_TYPE_REMOTE_DEVICE0_INFO + index,
                                  &remote_storage, &g_bt_dm_remote_list_cnt[index], sizeof(g_bt_dm_remote_list_cnt[index]));
    }
    remote_storage.nvkey_id = NVID_BT_HOST_REMOTE_SEQUENCE;
    bt_device_manager_db_init(BT_DEVICE_MANAGER_DB_TYPE_REMOTE_SEQUENCE_INFO,
                              &remote_storage, g_bt_dm_remote_sequence, sizeof(g_bt_dm_remote_sequence));
#ifdef MTK_AWS_MCE_ENABLE
    bt_aws_mce_report_register_callback(BT_AWS_MCE_REPORT_MODULE_DM, bt_device_manager_aws_mce_packet_callback);
#endif
}

bt_status_t bt_device_manager_remote_delete_info(bt_bd_addr_t *addr, bt_device_manager_remote_info_mask_t info_mask)
{
    bt_device_manager_db_remote_info_t *temp_remote = &g_bt_dm_remote_list_cnt[0];
    uint8_t clear_flag[BT_DEVICE_MANAGER_MAX_PAIR_NUM + 1] = {0};
    bt_dmgr_report_id("[BT_DM][REMOTE][I] Delete info addr:0x%x, info mask %d", 2, (NULL == addr ? (uint32_t)addr : *(uint32_t *)addr), info_mask);
    bt_device_manager_db_mutex_take();
    for (uint32_t index = 0; index < BT_DEVICE_MANAGER_MAX_PAIR_NUM; temp_remote++, index++) {
        if (!g_bt_dm_remote_sequence[index]) {
            continue;
        }
        if (NULL == addr || !memcmp(&(temp_remote->address), addr, sizeof(bt_bd_addr_t))) {
            if (info_mask == 0) {
                temp_remote->info_valid_flag = 0;
            } else {
                temp_remote->info_valid_flag &= (~info_mask);
                clear_flag[index + 1] = 1;
            }
        } else {
            continue;
        }
        if (0 == temp_remote->info_valid_flag) {
            for (uint8_t i = 0; i < BT_DEVICE_MANAGER_MAX_PAIR_NUM; i++) {
                if (g_bt_dm_remote_sequence[i] > g_bt_dm_remote_sequence[index]) {
                    g_bt_dm_remote_sequence[i]--;
                }
            }
            if (g_bt_dm_remote_sequence[index]) {
                bt_dmgr_report_id("[BT_DM][REMOTE][I] delete device index:%d, sequence:%d, addr:0x%x", 3,
                                  index, g_bt_dm_remote_sequence[index], *(uint32_t *)(temp_remote->address));
            }
            g_bt_dm_remote_sequence[index] = 0;
            clear_flag[0] = 1;
            clear_flag[index + 1] = 0;
        }
    }
    bt_device_manager_db_mutex_give();
    if (1 == clear_flag[0]) {
        bt_device_manager_db_update(BT_DEVICE_MANAGER_DB_TYPE_REMOTE_SEQUENCE_INFO);
        bt_device_manager_db_flush(BT_DEVICE_MANAGER_DB_TYPE_REMOTE_SEQUENCE_INFO, BT_DEVICE_MANAGER_DB_FLUSH_NON_BLOCK);
#ifdef MTK_AWS_MCE_ENABLE
        bt_device_manager_remote_aws_sync_db(BT_DEVICE_MANAGER_DB_TYPE_REMOTE_SEQUENCE_INFO, true,
                                             sizeof(g_bt_dm_remote_sequence), (void *)g_bt_dm_remote_sequence);
#endif
    }
    for (uint32_t index = 1; index < sizeof(clear_flag); index++) {
        if (1 == clear_flag[index]) {
            bt_device_manager_db_update(BT_DEVICE_MANAGER_DB_TYPE_REMOTE_DEVICE0_INFO + index - 1);
            bt_device_manager_db_flush(BT_DEVICE_MANAGER_DB_TYPE_REMOTE_DEVICE0_INFO + index - 1, BT_DEVICE_MANAGER_DB_FLUSH_NON_BLOCK);
#ifdef MTK_AWS_MCE_ENABLE
            bt_device_manager_remote_aws_sync_db(BT_DEVICE_MANAGER_DB_TYPE_REMOTE_DEVICE0_INFO + index - 1, true,
                                                 sizeof(bt_device_manager_db_remote_info_t), (void *)(g_bt_dm_remote_list_cnt + index - 1));
#endif
        }
    }
    return BT_STATUS_SUCCESS;
}

bt_status_t bt_device_manager_remote_set_seq_num(bt_bd_addr_t addr, uint8_t sequence)
{
    uint32_t index = 0;
    uint32_t find_index = 0xFF;
    bt_device_manager_db_remote_info_t *temp_remote = &g_bt_dm_remote_list_cnt[0];
    bt_dmgr_report_id("[BT_DM][REMOTE][I] Set seq device addr 0x%x, seq:%d", 2, *(uint32_t *)addr, sequence);
    bt_device_manager_db_mutex_take();
    for (index = 0; index < BT_DEVICE_MANAGER_MAX_PAIR_NUM; index++, temp_remote++) {
        if (g_bt_dm_remote_sequence[index] && !memcmp(temp_remote->address, addr, sizeof(bt_bd_addr_t))) {
            find_index = index;
            break;
        }
    }
    if (0xFF == find_index) {
        bt_device_manager_db_mutex_give();
        bt_dmgr_report_id("[BT_DM][REMOTE][W] Set seq fail not find dev", 0);
        return BT_STATUS_FAIL;
    }
    if (sequence == g_bt_dm_remote_sequence[find_index]) {
        bt_device_manager_db_mutex_give();
        return BT_STATUS_SUCCESS;
    } else if (sequence > g_bt_dm_remote_sequence[find_index]) {
        uint8_t max_seq = g_bt_dm_remote_sequence[find_index];
        for (index = 0; index < BT_DEVICE_MANAGER_MAX_PAIR_NUM; index++) {
            max_seq = max_seq < g_bt_dm_remote_sequence[index] ? g_bt_dm_remote_sequence[index] : max_seq;
            if (g_bt_dm_remote_sequence[index] && (g_bt_dm_remote_sequence[index] <= sequence) &&
                (g_bt_dm_remote_sequence[index] > g_bt_dm_remote_sequence[find_index])) {
                g_bt_dm_remote_sequence[index]--;
            }
        }
        sequence = (sequence > max_seq) ? max_seq : sequence;
    } else {
        for (index = 0; index < BT_DEVICE_MANAGER_MAX_PAIR_NUM; index++) {
            if (g_bt_dm_remote_sequence[index] && (g_bt_dm_remote_sequence[index] >= sequence) &&
                (g_bt_dm_remote_sequence[index] < g_bt_dm_remote_sequence[find_index])) {
                g_bt_dm_remote_sequence[index]++;
            }
        }
    }
    g_bt_dm_remote_sequence[find_index] = sequence;
    bt_device_manager_db_mutex_give();
    bt_device_manager_db_update(BT_DEVICE_MANAGER_DB_TYPE_REMOTE_SEQUENCE_INFO);
    bt_device_manager_db_flush(BT_DEVICE_MANAGER_DB_TYPE_REMOTE_SEQUENCE_INFO, BT_DEVICE_MANAGER_DB_FLUSH_NON_BLOCK);
#ifdef MTK_AWS_MCE_ENABLE
    bt_device_manager_remote_aws_sync_db(BT_DEVICE_MANAGER_DB_TYPE_REMOTE_SEQUENCE_INFO, true,
                                         sizeof(g_bt_dm_remote_sequence), (void *)g_bt_dm_remote_sequence);
#endif
    return BT_STATUS_SUCCESS;
}

bt_status_t     bt_device_manager_remote_top(bt_bd_addr_t addr)
{
    return bt_device_manager_remote_set_seq_num(addr, 1);
}

bt_bd_addr_t    *bt_device_manager_remote_get_dev_by_seq_num(uint32_t sequence)
{
    bt_device_manager_db_remote_info_t *temp_remote = &g_bt_dm_remote_list_cnt[0];
    bt_dmgr_report_id("[BT_DM][REMOTE][I] Get dev by seq num %d", 1, sequence);
    for (uint32_t index = 0; sequence && index < BT_DEVICE_MANAGER_MAX_PAIR_NUM; index++, temp_remote++) {
        if (sequence == g_bt_dm_remote_sequence[index]) {
            return &(temp_remote->address);
        }
    }
    return NULL;
}

uint32_t        bt_device_manager_remote_get_paired_num(void)
{
    uint32_t ret = 0;
    bt_device_manager_db_remote_info_t *temp_remote = &g_bt_dm_remote_list_cnt[0];
    for (uint32_t index = 0; index < BT_DEVICE_MANAGER_MAX_PAIR_NUM; index++, temp_remote++) {
        if (g_bt_dm_remote_sequence[index] && (temp_remote->info_valid_flag & BT_DEVICE_MANAGER_REMOTE_INFO_MASK_PAIRED)) {
            ret++;
        }
    }
    bt_dmgr_report_id("[BT_DM][REMOTE][I] Get paiared num = 0x%x", 1, ret);
    return ret;
}

void            bt_device_manager_remote_get_paired_list(bt_device_manager_paired_infomation_t *info, uint32_t *read_count)
{
    uint32_t count = 0;
    bt_device_manager_db_remote_paired_info_t *temp = NULL;
    bt_dmgr_report_id("[BT_DM][REMOTE][I] Get Paired list info:0x%x, read cound:%d", 2, info, *read_count);
    if (NULL == info || *read_count == 0 || NULL == (temp = bt_utils_memory_alloc(sizeof(bt_device_manager_db_remote_paired_info_t)))) {
        return;
    }
    bt_device_manager_db_mutex_take();
    for (uint32_t index = 0; index < BT_DEVICE_MANAGER_MAX_PAIR_NUM; index++) {
        if (count < *read_count) {
            if (BT_STATUS_SUCCESS == bt_device_manager_remote_find_paired_info_by_seq_num((uint8_t)(index + 1), temp)) {
                memcpy(info[count].address, temp->paired_key.address, sizeof(bt_bd_addr_t));
#ifdef AIR_BT_DEVICE_NAME_IN_PAIRING_DB
                memcpy(info[count].name, temp->name, sizeof(info[count].name));
#else
                info[count].name[0] = 0;
#endif /* AIR_BT_DEVICE_NAME_IN_PAIRING_DB */
                count++;
            }
        } else {
            break;
        }
    }
    bt_device_manager_db_mutex_give();
    bt_utils_memory_free((void *)temp);
    *read_count = count;
}

bt_status_t     bt_device_manager_remote_find_paired_info_by_seq_num(uint8_t sequence, bt_device_manager_db_remote_paired_info_t *info)
{
    bt_device_manager_db_remote_info_t *temp_remote = &g_bt_dm_remote_list_cnt[0];
    bt_dmgr_report_id("[BT_DM][REMOTE][I] Find paired info by sequence num %d, info 0x%x", 2, sequence, info);
    if (NULL == info) {
        return BT_STATUS_FAIL;
    }
    for (uint32_t index = 0; sequence && index < BT_DEVICE_MANAGER_MAX_PAIR_NUM; index++, temp_remote++) {
        if (g_bt_dm_remote_sequence[index] == sequence) {
            if (temp_remote->info_valid_flag & BT_DEVICE_MANAGER_REMOTE_INFO_MASK_PAIRED) {
                memcpy(info->paired_key.address, temp_remote->address, sizeof(bt_bd_addr_t));
                memcpy(info->paired_key.key, temp_remote->paired_info.key, sizeof(bt_key_t));
                info->paired_key.key_type = temp_remote->paired_info.key_type;
#ifdef AIR_BT_DEVICE_NAME_IN_PAIRING_DB
                if (temp_remote->info_valid_flag & BT_DEVICE_MANAGER_REMOTE_INFO_MASK_NAME) {
                    memcpy(info->name, temp_remote->device_name, sizeof(info->name));
                } else {
                    info->name[0] = 0;
                }
#else
                info->name[0] = 0;
#endif /* AIR_BT_DEVICE_NAME_IN_PAIRING_DB */
                bt_dmgr_report_id("[BT_DM][REMOTE][I] Find paired info success", 0);
                return BT_STATUS_SUCCESS;
            }
            break;
        }
    }
    return BT_STATUS_FAIL;
}

static bt_status_t bt_device_manager_remote_info_update(bt_bd_addr_t addr, bt_device_manager_remote_info_mask_t type, void *data)
{
    uint8_t temp_addr[6] = {0x00};
    bt_dmgr_report_id("[BT_DM][REMOTE][I] Update info type 0x%x, addr:0x%x", 2, type, (NULL == addr ? (uint32_t)addr : *(uint32_t *)addr));
    if (NULL == addr || 0 == BT_DEVICE_MANAGER_MAX_PAIR_NUM || !memcmp(&temp_addr, (uint8_t *)addr, sizeof(bt_bd_addr_t))) {
        return BT_STATUS_FAIL;
    }
    uint32_t index = 0, item_index = 0xFF;
    bool flush_at_once = false, seq_update = false, item_update = false;
    bt_device_manager_db_remote_info_t *temp_remote = &g_bt_dm_remote_list_cnt[0];
    bt_device_manager_db_remote_info_t *find_remote = temp_remote;
    bt_device_manager_db_mutex_take();
    for (index = 0; index < BT_DEVICE_MANAGER_MAX_PAIR_NUM; index++, temp_remote++) {
        if (g_bt_dm_remote_sequence[index] && !memcmp(temp_remote->address, addr, sizeof(bt_bd_addr_t))) {
            find_remote = temp_remote;
            item_index = index;
            break;
        } else if (item_index == 0xFF && (!g_bt_dm_remote_sequence[index] || BT_DEVICE_MANAGER_MAX_PAIR_NUM == g_bt_dm_remote_sequence[index])) {
            find_remote = temp_remote;
            item_index = index;
        }
    }
    if (item_index >= BT_DEVICE_MANAGER_MAX_PAIR_NUM) {
        bt_device_manager_db_mutex_give();
        return BT_STATUS_FAIL;
    }
    if (BT_DEVICE_MANAGER_MAX_PAIR_NUM == index) {
        bt_dmgr_report_id("[BT_DM][REMOTE][I] New addr 0x%x, index %d, index sequence %d", 3, *(uint32_t *)addr, item_index, g_bt_dm_remote_sequence[item_index]);
        if (g_bt_dm_remote_sequence[item_index]) {
            bt_dmgr_report_id("[BT_DM][REMOTE][I] New need delete device addr 0x%x", 1, *(uint32_t *)(find_remote->address));
        }
        memcpy(find_remote->address, addr, sizeof(bt_bd_addr_t));
        find_remote->info_valid_flag = 0;
        g_bt_dm_remote_sequence[item_index] = 1;
        for (index = 0; index < BT_DEVICE_MANAGER_MAX_PAIR_NUM; index++) {
            if (index != item_index && g_bt_dm_remote_sequence[index]) {
                g_bt_dm_remote_sequence[index]++;
            }
        }
        seq_update = true;
        flush_at_once = true;
    }
    if (!(find_remote->info_valid_flag & type)) {
        find_remote->info_valid_flag |= type;
        item_update = true;
    }
    switch (type) {
        case BT_DEVICE_MANAGER_REMOTE_INFO_MASK_PAIRED: {
            bt_device_manager_db_remote_paired_info_t *info = (void *)data;
            memcpy(find_remote->paired_info.key, info->paired_key.key, sizeof(bt_key_t));
            find_remote->paired_info.key_type = info->paired_key.key_type;
            flush_at_once = true;
            item_update = true;
            break;
        }
#ifdef AIR_BT_DEVICE_NAME_IN_PAIRING_DB
        case BT_DEVICE_MANAGER_REMOTE_INFO_MASK_NAME: {
            memcpy(find_remote->device_name, data, sizeof(find_remote->device_name));
            flush_at_once = true;
            item_update = true;
            break;
        }
#endif /* AIR_BT_DEVICE_NAME_IN_PAIRING_DB */
        case BT_DEVICE_MANAGER_REMOTE_INFO_MASK_PROFILE: {
            bt_device_manager_db_remote_profile_info_t *info = (void *)data;
            if (memcmp(&(find_remote->profile_info), info, sizeof(bt_device_manager_db_remote_profile_info_t))) {
                memcpy(&(find_remote->profile_info), info, sizeof(bt_device_manager_db_remote_profile_info_t));
                item_update = true;
            }
            break;
        }
        case BT_DEVICE_MANAGER_REMOTE_INFO_MASK_IOT_INFO:
            if (find_remote->iot_id != (uint32_t)data) {
                find_remote->iot_id = (uint32_t)data;
                item_update = true;
            }
            break;
        case BT_DEVICE_MANAGER_REMOTE_INFO_MASK_SUPPORTED_PROFILES:
            if (find_remote->supported_profiles != (uint32_t)data) {
                find_remote->supported_profiles = (uint32_t)data;
                item_update = true;
            }
            break;
        default:
            bt_device_manager_db_mutex_give();
            return BT_STATUS_FAIL;
    }
    bt_device_manager_db_mutex_give();
    if (true == seq_update) {
        bt_device_manager_db_update(BT_DEVICE_MANAGER_DB_TYPE_REMOTE_SEQUENCE_INFO);
        bt_device_manager_db_flush(BT_DEVICE_MANAGER_DB_TYPE_REMOTE_SEQUENCE_INFO, BT_DEVICE_MANAGER_DB_FLUSH_NON_BLOCK);
#ifdef MTK_AWS_MCE_ENABLE
        bt_device_manager_remote_aws_sync_db(BT_DEVICE_MANAGER_DB_TYPE_REMOTE_SEQUENCE_INFO, true,
                                             sizeof(g_bt_dm_remote_sequence), (void *)g_bt_dm_remote_sequence);
#endif
    }
    if (true == item_update) {
        bt_device_manager_db_update(BT_DEVICE_MANAGER_DB_TYPE_REMOTE_DEVICE0_INFO + item_index);
        if (flush_at_once == true) {
            bt_device_manager_db_flush(BT_DEVICE_MANAGER_DB_TYPE_REMOTE_DEVICE0_INFO + item_index, BT_DEVICE_MANAGER_DB_FLUSH_NON_BLOCK);
        }
#ifdef MTK_AWS_MCE_ENABLE
        bt_device_manager_remote_aws_sync_db(BT_DEVICE_MANAGER_DB_TYPE_REMOTE_DEVICE0_INFO + item_index, flush_at_once,
                                             sizeof(bt_device_manager_db_remote_info_t), (void *)(g_bt_dm_remote_list_cnt + item_index));
#endif
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_device_manager_remote_info_find(bt_bd_addr_t addr, bt_device_manager_remote_info_mask_t type, void *data)
{
    bt_dmgr_report_id("[BT_DM][REMOTE][I] Find info type 0x%x, data:0x%x", 2, type, data);
    if (0 == BT_DEVICE_MANAGER_MAX_PAIR_NUM) {
        return BT_STATUS_FAIL;
    }
    bt_device_manager_db_remote_info_t *temp_remote = &g_bt_dm_remote_list_cnt[0];
    bt_device_manager_db_remote_info_t *find_remote = NULL;
    for (uint32_t index = 0; index < BT_DEVICE_MANAGER_MAX_PAIR_NUM; index++, temp_remote++) {
        if (g_bt_dm_remote_sequence[index] && !memcmp(temp_remote->address, addr, sizeof(bt_bd_addr_t)) &&
            (temp_remote->info_valid_flag & type)) {
            find_remote = temp_remote;
            break;
        }
    }
    if (NULL == find_remote) {
        bt_dmgr_report_id("[BT_DM][REMOTE][I] Find info fail", 0);
        return BT_STATUS_FAIL;
    }
    if (NULL == data) {
        bt_dmgr_report_id("[BT_DM][REMOTE][E] Find info success but no buffer to store it", 0);
        return BT_STATUS_SUCCESS;
    }
    switch (type) {
        case BT_DEVICE_MANAGER_REMOTE_INFO_MASK_PAIRED: {
            memcpy(((bt_device_manager_db_remote_paired_info_t *)data)->paired_key.address, find_remote->address, sizeof(bt_bd_addr_t));
            memcpy(((bt_device_manager_db_remote_paired_info_t *)data)->paired_key.key, find_remote->paired_info.key, sizeof(bt_key_t));
            ((bt_device_manager_db_remote_paired_info_t *)data)->paired_key.key_type = find_remote->paired_info.key_type;
#ifdef AIR_BT_DEVICE_NAME_IN_PAIRING_DB
            if (find_remote->info_valid_flag & BT_DEVICE_MANAGER_REMOTE_INFO_MASK_NAME) {
                memcpy(((bt_device_manager_db_remote_paired_info_t *)data)->name, find_remote->device_name, sizeof(find_remote->device_name));
            } else {
                ((bt_device_manager_db_remote_paired_info_t *)data)->name[0] = 0;
            }
#endif /* AIR_BT_DEVICE_NAME_IN_PAIRING_DB */
            break;
        }
        case BT_DEVICE_MANAGER_REMOTE_INFO_MASK_PROFILE: {
            memcpy(data, &(find_remote->profile_info), sizeof(bt_device_manager_db_remote_profile_info_t));
            break;
        }
        case BT_DEVICE_MANAGER_REMOTE_INFO_MASK_IOT_INFO: {
            *(uint32_t *)data = find_remote->iot_id;
            break;
        }
        case BT_DEVICE_MANAGER_REMOTE_INFO_MASK_SUPPORTED_PROFILES: {
            *(uint32_t *)data = find_remote->supported_profiles;
            break;
        }
#ifdef AIR_BT_DEVICE_NAME_IN_PAIRING_DB
        case BT_DEVICE_MANAGER_REMOTE_INFO_MASK_NAME: {
            memcpy(data, find_remote->device_name, sizeof(find_remote->device_name));
            break;
        }
#endif /* AIR_BT_DEVICE_NAME_IN_PAIRING_DB */
        default:
            return BT_STATUS_FAIL;
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_device_manager_remote_non_flush_info_update(bt_bd_addr_t addr, bt_dm_remote_non_flush_info_mask_t type, void *data)
{
    uint32_t find_index = 0;
    bt_dmgr_report_id("[BT_DM][REMOTE][I] Non flush info update type 0x%x, data:0x%x", 2, type, data);
    if (NULL == data) {
        return BT_STATUS_FAIL;
    }

    bt_device_manager_db_mutex_take();
    for (find_index = 0; find_index < BT_DM_REMOTE_NON_FLUSH_RECORD_MAXIMUM; find_index++) {
        if (g_dm_remote_non_flush_list_cnt.item[find_index].info_valid_set &&
            !memcmp(addr, g_dm_remote_non_flush_list_cnt.item[find_index].address, sizeof(bt_bd_addr_t))) {
            break;
        }
    }
    if (find_index >= BT_DM_REMOTE_NON_FLUSH_RECORD_MAXIMUM) {
        find_index = g_dm_remote_non_flush_list_cnt.set_index++;
        if (g_dm_remote_non_flush_list_cnt.set_index >= BT_DM_REMOTE_NON_FLUSH_RECORD_MAXIMUM) {
            g_dm_remote_non_flush_list_cnt.set_index = 0;
        }
        g_dm_remote_non_flush_list_cnt.item[find_index].info_valid_set = 0;
        memcpy(g_dm_remote_non_flush_list_cnt.item[find_index].address, addr, sizeof(bt_bd_addr_t));
    }
    switch (type) {
#ifndef AIR_BT_DEVICE_NAME_IN_PAIRING_DB
        case BT_DM_REMOTE_NON_FLUSH_INFO_MASK_NAME:
            memcpy(g_dm_remote_non_flush_list_cnt.item[find_index].remote_name, data, sizeof(g_dm_remote_non_flush_list_cnt.item[find_index].remote_name));
            break;
#endif /* ndef AIR_BT_DEVICE_NAME_IN_PAIRING_DB */
        case BT_DM_REMOTE_NON_FLUSH_INFO_MASK_VERSION:
            memcpy(&g_dm_remote_non_flush_list_cnt.item[find_index].version_info, data, sizeof(g_dm_remote_non_flush_list_cnt.item[find_index].version_info));
            break;
        case BT_DM_REMOTE_NON_FLUSH_INFO_MASK_PNP:
            memcpy(&g_dm_remote_non_flush_list_cnt.item[find_index].pnp_info, data, sizeof(g_dm_remote_non_flush_list_cnt.item[find_index].pnp_info));
            break;
        case BT_DM_REMOTE_NON_FLUSH_INFO_MASK_COD:
            g_dm_remote_non_flush_list_cnt.item[find_index].cod = (uint32_t)data;
            break;
        default:
            bt_device_manager_db_mutex_give();
            return BT_STATUS_FAIL;
    }
    g_dm_remote_non_flush_list_cnt.item[find_index].info_valid_set |= type;
    if (BT_STATUS_SUCCESS == bt_device_manager_remote_info_find(addr, BT_DEVICE_MANAGER_REMOTE_INFO_MASK_PAIRED, NULL) && 
            (0x0F == (g_dm_remote_non_flush_list_cnt.item[find_index].info_valid_set & 0x0F) ||
             (!bt_device_manager_remote_find_iot_id(addr) && (BT_DM_REMOTE_NON_FLUSH_INFO_MASK_PNP & (g_dm_remote_non_flush_list_cnt.item[find_index].info_valid_set))))) {
        bt_iot_device_identify_information_t iot_info;
#ifdef AIR_BT_DEVICE_NAME_IN_PAIRING_DB
        bt_device_manager_remote_info_find(addr, BT_DEVICE_MANAGER_REMOTE_INFO_MASK_NAME, iot_info.device_name);
#else
        memcpy(iot_info.device_name, g_dm_remote_non_flush_list_cnt.item[find_index].remote_name, sizeof(iot_info.device_name));
#endif /* AIR_BT_DEVICE_NAME_IN_PAIRING_DB */
        memcpy(&(iot_info.version_info), &g_dm_remote_non_flush_list_cnt.item[find_index].version_info, sizeof(iot_info.version_info));
        memcpy(&(iot_info.pnp_info), &g_dm_remote_non_flush_list_cnt.item[find_index].pnp_info, sizeof(iot_info.pnp_info));
        iot_info.cod = g_dm_remote_non_flush_list_cnt.item[find_index].cod;
        bt_device_manager_remote_update_iot_id(addr, bt_iot_device_white_list_get_iot_id(&iot_info));
    }
    bt_device_manager_db_mutex_give();
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_device_manager_remote_non_flush_info_find(bt_bd_addr_t addr, bt_dm_remote_non_flush_info_mask_t type, void *data, uint16_t data_length)
{
    uint32_t index = 0;
    bt_dmgr_report_id("[BT_DM][REMOTE][I] Find non flush info buffer type %d, data:0x%x, data_length:%d", 3, type, data, data_length);
    if (NULL == data || 0 == data_length) {
        return BT_STATUS_FAIL;
    }
    for (index = 0; index < BT_DM_REMOTE_NON_FLUSH_RECORD_MAXIMUM; index++) {
        if ((g_dm_remote_non_flush_list_cnt.item[index].info_valid_set & type) &&
            !memcmp(addr, g_dm_remote_non_flush_list_cnt.item[index].address, sizeof(bt_bd_addr_t))) {
            break;
        }
    }
    if (index >= BT_DM_REMOTE_NON_FLUSH_RECORD_MAXIMUM) {
        bt_dmgr_report_id("[BT_DM][REMOTE][I] Find non flush info fail", 0);
        return BT_STATUS_FAIL;
    }
    switch (type) {
#ifndef AIR_BT_DEVICE_NAME_IN_PAIRING_DB
        case BT_DM_REMOTE_NON_FLUSH_INFO_MASK_NAME:
            memcpy(data, g_dm_remote_non_flush_list_cnt.item[index].remote_name, data_length);
            break;
#endif /* ndef AIR_BT_DEVICE_NAME_IN_PAIRING_DB */
        case BT_DM_REMOTE_NON_FLUSH_INFO_MASK_VERSION:
            bt_utils_assert(data_length >= sizeof(g_dm_remote_non_flush_list_cnt.item[index].version_info));
            memcpy(data, &g_dm_remote_non_flush_list_cnt.item[index].version_info, sizeof(g_dm_remote_non_flush_list_cnt.item[index].version_info));
            break;
        case BT_DM_REMOTE_NON_FLUSH_INFO_MASK_PNP:
            bt_utils_assert(data_length >= sizeof(g_dm_remote_non_flush_list_cnt.item[index].pnp_info));
            memcpy(data, &g_dm_remote_non_flush_list_cnt.item[index].pnp_info, sizeof(g_dm_remote_non_flush_list_cnt.item[index].pnp_info));
            break;
        case BT_DM_REMOTE_NON_FLUSH_INFO_MASK_COD:
            bt_utils_assert(data_length >= sizeof(g_dm_remote_non_flush_list_cnt.item[index].cod));
            *(uint32_t *)data = g_dm_remote_non_flush_list_cnt.item[index].cod;
            break;
        default:
            return BT_STATUS_FAIL;
    }
    return BT_STATUS_SUCCESS;
}

bt_status_t bt_device_manager_remote_update_paired_info(bt_bd_addr_t addr, bt_device_manager_db_remote_paired_info_t *info)
{
    bt_status_t ret = bt_device_manager_remote_info_update(addr, BT_DEVICE_MANAGER_REMOTE_INFO_MASK_PAIRED, (void *)info);
    uint32_t cod = bt_device_manager_remote_find_cod(addr);
    if (0 != cod) {
        bt_device_manager_remote_update_cod(addr, cod);
    }
    return ret;
}

bt_status_t bt_device_manager_remote_find_paired_info(bt_bd_addr_t addr, bt_device_manager_db_remote_paired_info_t *info)
{
    return bt_device_manager_remote_info_find(addr, BT_DEVICE_MANAGER_REMOTE_INFO_MASK_PAIRED, (void *)info);
}

bt_status_t bt_device_manager_remote_update_profile_info(bt_bd_addr_t addr, bt_device_manager_db_remote_profile_info_t *info)
{
    if (BT_STATUS_SUCCESS != bt_device_manager_remote_info_find(addr, BT_DEVICE_MANAGER_REMOTE_INFO_MASK_PAIRED, NULL)) {
        return BT_STATUS_FAIL;
    }
    return bt_device_manager_remote_info_update(addr, BT_DEVICE_MANAGER_REMOTE_INFO_MASK_PROFILE, (void *)info);
}

bt_status_t bt_device_manager_remote_find_profile_info(bt_bd_addr_t addr, bt_device_manager_db_remote_profile_info_t *info)
{
    return bt_device_manager_remote_info_find(addr, BT_DEVICE_MANAGER_REMOTE_INFO_MASK_PROFILE, (void *)info);
}

bt_status_t bt_device_manager_remote_update_supported_profiles(bt_bd_addr_t addr, uint32_t profiles)
{
    return bt_device_manager_remote_info_update(addr, BT_DEVICE_MANAGER_REMOTE_INFO_MASK_SUPPORTED_PROFILES, (void *)profiles);
}

uint32_t    bt_device_manager_remote_find_supported_profiles(bt_bd_addr_t addr)
{
    uint32_t supported_profiles = 0;
    bt_device_manager_remote_info_find(addr, BT_DEVICE_MANAGER_REMOTE_INFO_MASK_SUPPORTED_PROFILES, &supported_profiles);
    return supported_profiles;
}

bt_status_t bt_device_manager_remote_update_iot_id(bt_bd_addr_t addr, uint32_t iot_id)
{
    return bt_device_manager_remote_info_update(addr, BT_DEVICE_MANAGER_REMOTE_INFO_MASK_IOT_INFO, (void *)iot_id);
}

uint32_t    bt_device_manager_remote_find_iot_id(bt_bd_addr_t addr)
{
    uint32_t iot_id = 0;
    bt_device_manager_remote_info_find(addr, BT_DEVICE_MANAGER_REMOTE_INFO_MASK_IOT_INFO, &iot_id);
    return iot_id;
}

bt_status_t bt_device_manager_remote_update_cod(bt_bd_addr_t addr, uint32_t cod)
{
    return bt_device_manager_remote_non_flush_info_update(addr, BT_DM_REMOTE_NON_FLUSH_INFO_MASK_COD, (void *)cod);
}

uint32_t    bt_device_manager_remote_find_cod(bt_bd_addr_t addr)
{
    uint32_t cod = 0;
    bt_device_manager_remote_non_flush_info_find(addr, BT_DM_REMOTE_NON_FLUSH_INFO_MASK_COD, (void *)&cod, sizeof(cod));
    return cod;
}

bt_status_t bt_device_manager_remote_update_pnp_info(bt_bd_addr_t addr, bt_device_manager_db_remote_pnp_info_t *info)
{
    return bt_device_manager_remote_non_flush_info_update(addr, BT_DM_REMOTE_NON_FLUSH_INFO_MASK_PNP, (void *)info);
}

bt_status_t bt_device_manager_remote_find_pnp_info(bt_bd_addr_t addr, bt_device_manager_db_remote_pnp_info_t *info)
{
    return bt_device_manager_remote_non_flush_info_find(addr, BT_DM_REMOTE_NON_FLUSH_INFO_MASK_PNP, info, sizeof(*info));
}

bt_status_t bt_device_manager_remote_update_version_info(bt_bd_addr_t addr, bt_device_manager_db_remote_version_info_t *info)
{
    return bt_device_manager_remote_non_flush_info_update(addr, BT_DM_REMOTE_NON_FLUSH_INFO_MASK_VERSION, (void *)info);
}

bt_status_t bt_device_manager_remote_find_version_info(bt_bd_addr_t addr, bt_device_manager_db_remote_version_info_t *info)
{
    return bt_device_manager_remote_non_flush_info_find(addr, BT_DM_REMOTE_NON_FLUSH_INFO_MASK_VERSION, (void *)info, sizeof(*info));
}

bt_status_t bt_device_manager_remote_update_name(bt_bd_addr_t addr, char *name)
{
#ifdef AIR_BT_DEVICE_NAME_IN_PAIRING_DB
    return bt_device_manager_remote_info_update(addr, BT_DEVICE_MANAGER_REMOTE_INFO_MASK_NAME, (void *)name);
#else
    return bt_device_manager_remote_non_flush_info_update(addr, BT_DM_REMOTE_NON_FLUSH_INFO_MASK_NAME, (void *)name);
#endif /* AIR_BT_DEVICE_NAME_IN_PAIRING_DB */
}

bt_status_t bt_device_manager_remote_find_name(bt_bd_addr_t addr, char *name, uint16_t buffer_length)
{
#ifdef AIR_BT_DEVICE_NAME_IN_PAIRING_DB
    bt_status_t return_status;

    if (buffer_length < BT_DEVICE_MANAGER_NAME_LENGTH) {
        char full_name[BT_DEVICE_MANAGER_NAME_LENGTH];

        return_status = bt_device_manager_remote_info_find(addr, BT_DEVICE_MANAGER_REMOTE_INFO_MASK_NAME, full_name);
        if (BT_STATUS_SUCCESS == return_status) {
            memcpy(name, full_name, buffer_length - 1);
            name[buffer_length - 1] = 0; /* Zero terminate */
        }
    } else {
        return_status = bt_device_manager_remote_info_find(addr, BT_DEVICE_MANAGER_REMOTE_INFO_MASK_NAME, name);
    }
    return return_status;
#else
    return bt_device_manager_remote_non_flush_info_find(addr, BT_DM_REMOTE_NON_FLUSH_INFO_MASK_NAME, name, buffer_length);
#endif /* AIR_BT_DEVICE_NAME_IN_PAIRING_DB */
}

#ifdef MTK_AWS_MCE_ENABLE
static void bt_device_manager_aws_mce_packet_callback(bt_aws_mce_report_info_t *para)
{
    if (NULL == para || BT_AWS_MCE_REPORT_MODULE_DM != para->module_id) {
        return;
    }
    bt_device_manager_db_type_t type = ((uint8_t *)para->param)[0];
    uint8_t flush_at_once = ((uint8_t *)para->param)[1];
    bool need_update = false;
    bt_dmgr_report_id("[BT_DM][REMOTE][AWS][I] Packet callback param_len:%d, type:%d, flush at once %d", 3, para->param_len, type, flush_at_once);
    bt_device_manager_db_mutex_take();
    if (BT_DEVICE_MANAGER_DB_TYPE_REMOTE_SEQUENCE_INFO == type) {
        if (memcmp(&g_bt_dm_remote_sequence, ((uint8_t *)para->param) + 2, sizeof(g_bt_dm_remote_sequence))) {
            memcpy(&g_bt_dm_remote_sequence, ((uint8_t *)para->param) + 2, sizeof(g_bt_dm_remote_sequence));
            need_update = true;
        }
    } else if (BT_DEVICE_MANAGER_DB_TYPE_REMOTE_DEVICE0_INFO <= type && BT_DEVICE_MANAGER_DB_TYPE_REMOTE_DEVICE_MAX >= type) {
        uint8_t sequence_num = type - BT_DEVICE_MANAGER_DB_TYPE_REMOTE_DEVICE0_INFO;
        if (memcmp(&(g_bt_dm_remote_list_cnt[sequence_num]), ((uint8_t *)para->param) + 2, sizeof(bt_device_manager_db_remote_info_t))) {
            memcpy(&(g_bt_dm_remote_list_cnt[sequence_num]), ((uint8_t *)para->param) + 2, sizeof(bt_device_manager_db_remote_info_t));
            need_update = true;
        }
    } else if (BT_DEVICE_MANAGER_DB_TYPE_LINK_RECORD_INFO == type) {
        bt_device_manager_link_record_aws_update_context((void *)(((uint8_t *)para->param) + 1));
    } else if (BT_DEVICE_MANAGER_DB_TYPE_MAX == type) {
        memcpy(&g_dm_remote_non_flush_list_cnt, ((uint8_t *)para->param) + 2, sizeof(g_dm_remote_non_flush_list_cnt));
    }
    bt_device_manager_db_mutex_give();
    if (true == need_update) {
        bt_device_manager_db_update(type);
        if (flush_at_once) {
            bt_device_manager_db_flush(type, BT_DEVICE_MANAGER_DB_FLUSH_NON_BLOCK);
        }
    }
}

#ifdef MTK_BT_SPEAKER_ENABLE
//Don't sync remote paired list to parner under speaker mode
static void bt_device_manager_remote_aws_sync_db(bt_device_manager_db_type_t type, bool flush_at_once, uint16_t data_length, uint8_t *data)
{
    bt_dmgr_report_id("[BT_DM][REMOTE][AWS][I] SPEAKER don't sync db type 0x%02X", 1, type);
    UNUSED(type);
    UNUSED(data_length);
    UNUSED(data);
}
#else

static void bt_device_manager_remote_aws_sync_db(bt_device_manager_db_type_t type, bool flush_at_once, uint16_t data_length, uint8_t *data)
{
    bt_status_t status;
    uint32_t report_length = sizeof(bt_aws_mce_report_info_t) + data_length + 2;
    bt_aws_mce_report_info_t *dm_report = bt_utils_memory_alloc(report_length);
    if (NULL == dm_report) {
        return;
    }
    uint8_t *data_payload = ((uint8_t *)dm_report) + sizeof(bt_aws_mce_report_info_t);
    memset(dm_report, 0, report_length);
    dm_report->module_id = BT_AWS_MCE_REPORT_MODULE_DM;
    dm_report->param_len = data_length + 2;
    dm_report->param = (void *)data_payload;
    data_payload[0] = type;
    data_payload[1] = (uint8_t)flush_at_once;
    memcpy(data_payload + 2, (void *)data, data_length);
    status = bt_aws_mce_report_send_event(dm_report);
    bt_dmgr_report_id("[BT_DM][REMOTE][AWS][I] Sync db type 0x%02X, flush at once:%d, status:0x%x", 3, type, flush_at_once, status);
    bt_utils_memory_free((void *)dm_report);
}
#endif

static void bt_device_manager_remote_aws_sync_timer_callback(uint32_t timer_id, uint32_t data)
{
    // Remote device info sync.
    UNUSED(timer_id);
    for (uint32_t index = data; index < BT_DEVICE_MANAGER_MAX_PAIR_NUM; index++) {
        if (g_bt_dm_remote_sequence[index]) {
            bt_device_manager_remote_aws_sync_db(BT_DEVICE_MANAGER_DB_TYPE_REMOTE_DEVICE0_INFO + index, true,
                                         sizeof(bt_device_manager_db_remote_info_t), (void *)(g_bt_dm_remote_list_cnt + index));
            bt_timer_ext_start(BT_DM_PAIRED_INFOR_SYNC_TIMER_ID, index + 1, 10, bt_device_manager_remote_aws_sync_timer_callback);
            return;
        }
    }
    // Sequence info sync.
    bt_device_manager_remote_aws_sync_db(BT_DEVICE_MANAGER_DB_TYPE_REMOTE_SEQUENCE_INFO, true,
                                         sizeof(g_bt_dm_remote_sequence), (void *)g_bt_dm_remote_sequence);
    // Non-flush data sync.
    bt_device_manager_remote_aws_sync_db(BT_DEVICE_MANAGER_DB_TYPE_MAX, false,
                                         sizeof(g_dm_remote_non_flush_list_cnt), (void *)&g_dm_remote_non_flush_list_cnt);
}

void        bt_device_manager_remote_aws_sync_to_partner(void)
{
    bt_dmgr_report_id("[BT_DM][REMOTE][AWS][I] Sync to partner", 0);
    if (0 == BT_DEVICE_MANAGER_MAX_PAIR_NUM) {
        return;
    }
    bt_device_manager_remote_aws_sync_timer_callback(0, 0);
}

#endif

