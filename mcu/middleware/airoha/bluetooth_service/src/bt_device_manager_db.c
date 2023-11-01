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

#ifdef MTK_NVDM_ENABLE
#include "nvdm.h"
#include "nvkey.h"
#include "nvkey_id_list.h"
#endif /* MTK_NVDM_ENABLE */
#include "bt_device_manager_internal.h"
#include "bt_device_manager_db.h"
#include "bt_utils.h"

extern uint32_t bt_os_layer_create_mutex(void);
extern void     bt_os_layer_take_mutex(uint32_t mutex_id);
extern void     bt_os_layer_give_mutex(uint32_t mutex_id);

#define BT_DEVICE_MANAGER_DB_FLAG_INIT      (0x01)
#define BT_DEVICE_MANAGER_DB_FLAG_DIRTY     (0x02)
typedef uint8_t bt_device_manager_db_flag_t;

typedef struct {
    bt_device_manager_db_flag_t flag;
    bt_device_manager_db_type_t type;
    uint16_t need_write_count;
    uint16_t writing_count;
    uint32_t buffer_size;
    uint8_t *buffer;
    bt_device_manager_db_storage_t storage;
} bt_device_manager_db_cnt_t;

static bt_device_manager_db_cnt_t dev_db_cnt[BT_DEVICE_MANAGER_DB_TYPE_MAX];
static uint32_t g_dev_db_mutex = 0;

#define BT_DM_DB_LOG_HANDLE_OPEN        0x01
#define BT_DM_DB_LOG_HANDLE_CLOSE       0x02
#define BT_DM_DB_LOG_HANDLE_UPDATE      0x03
#define BT_DM_DB_LOG_HANDLE_FLUSH       0x04
typedef uint8_t bt_dm_db_log_handle_t;

static void bt_device_manager_db_log_print(bt_dm_db_log_handle_t type, bt_device_manager_db_type_t db_type)
{
    bt_dmgr_report_id("[BT_DM][DB][I] Handle type:%d, db_type:%d", 2, type, db_type);
}

static void bt_device_manager_db_after_flash_complete_cb(nvkey_status_t status, void *user_data)
{
    uint16_t writing_count = ((uint32_t)user_data) & 0xFFFF;
    bt_device_manager_db_type_t db_type = ((uint32_t)user_data) >> 16;
    bt_dmgr_report_id("[BT_DM][DB][I] Non-blocking flush db result:%d, type:%d, writing_count:%d, record_count:%d", 4,
                      status, db_type, writing_count, dev_db_cnt[db_type].writing_count);
    if (NVKEY_STATUS_OK == status || NVKEY_STATUS_ITEM_MERGED_BY_NBW == status) {
        int16_t diff_count = dev_db_cnt[db_type].writing_count - writing_count;
        dev_db_cnt[db_type].writing_count = (diff_count < 0 ? 0 : diff_count);
    }
}

static bool bt_device_manager_db_storage_write(bt_device_manager_db_type_t db_type, bt_device_manager_db_flush_t block)
{
    bt_dmgr_report_id("[BT_DM][DB][I] Storage write db type %d, storage type %d, size %d", 3,
                      db_type, dev_db_cnt[db_type].storage.storage_type, dev_db_cnt[db_type].buffer_size);
    int32_t result = -1000;
#ifdef MTK_NVDM_ENABLE
    if (BT_DEVICE_MANAGER_DB_STORAGE_TYPE_NVDM == dev_db_cnt[db_type].storage.storage_type) {
        if (BT_DEVICE_MANAGER_DB_FLUSH_BLOCK == block) {
            result = (int32_t)nvdm_write_data_item(dev_db_cnt[db_type].storage.nvdm_group_str,
                                          dev_db_cnt[db_type].storage.nvdm_item_str, NVDM_DATA_ITEM_TYPE_RAW_DATA,
                                          dev_db_cnt[db_type].buffer, dev_db_cnt[db_type].buffer_size);
        } else {
            uint32_t user_data = dev_db_cnt[db_type].need_write_count | (((uint16_t)db_type) << 16);
            result = (int32_t)nvdm_write_data_item_non_blocking(dev_db_cnt[db_type].storage.nvdm_group_str,
                                                       dev_db_cnt[db_type].storage.nvdm_item_str, NVDM_DATA_ITEM_TYPE_RAW_DATA,
                                                       dev_db_cnt[db_type].buffer, dev_db_cnt[db_type].buffer_size,
                                                       (nvdm_user_callback_t)bt_device_manager_db_after_flash_complete_cb, (void *)user_data);
        }
    } else if (BT_DEVICE_MANAGER_DB_STORAGE_TYPE_NVKEY == dev_db_cnt[db_type].storage.storage_type) {
        if (BT_DEVICE_MANAGER_DB_FLUSH_BLOCK == block) {
            result = (int32_t)nvkey_write_data(dev_db_cnt[db_type].storage.nvkey_id, dev_db_cnt[db_type].buffer,
                                      dev_db_cnt[db_type].buffer_size);
        } else {
            uint32_t user_data = dev_db_cnt[db_type].need_write_count | (((uint16_t)db_type) << 16);
            result = (int32_t)nvkey_write_data_non_blocking(dev_db_cnt[db_type].storage.nvkey_id, dev_db_cnt[db_type].buffer,
                                                   dev_db_cnt[db_type].buffer_size, bt_device_manager_db_after_flash_complete_cb, (void *)user_data);
        }
    }
#endif
    if (0 != result) {
        bt_dmgr_report_id("[BT_DM][DB][E] Storage write fail, result: %d", 1, result);
        return false;
    }
    return true;
}

static bool bt_device_manager_db_storage_read(bt_device_manager_db_type_t db_type)
{
    bt_dmgr_report_id("[BT_DM][DB][I] Storage read db type %d, storage type %d", 2,
                      db_type, dev_db_cnt[db_type].storage.storage_type);
    int32_t result = -1000;
    uint32_t temp_size = dev_db_cnt[db_type].buffer_size;;
#ifdef MTK_NVDM_ENABLE
    if (BT_DEVICE_MANAGER_DB_STORAGE_TYPE_NVDM == dev_db_cnt[db_type].storage.storage_type) {
        result = (int32_t)nvdm_read_data_item(dev_db_cnt[db_type].storage.nvdm_group_str,
                                                dev_db_cnt[db_type].storage.nvdm_item_str, dev_db_cnt[db_type].buffer, &temp_size);

    } else if (BT_DEVICE_MANAGER_DB_STORAGE_TYPE_NVKEY == dev_db_cnt[db_type].storage.storage_type) {
        result = (int32_t)nvkey_read_data(dev_db_cnt[db_type].storage.nvkey_id,
                                                dev_db_cnt[db_type].buffer, &temp_size);        
    }
#endif
    bt_dmgr_report_id("[BT_DM][DB][I] Storage real size %d, read size %d, result %d", 3,
                          dev_db_cnt[db_type].buffer_size, temp_size, result);
    if (NVKEY_STATUS_ITEM_NOT_FOUND == result && true == dev_db_cnt[db_type].storage.auto_gen) {
        memset((void *)dev_db_cnt[db_type].buffer, 0, dev_db_cnt[db_type].buffer_size);
        return bt_device_manager_db_storage_write(db_type, BT_DEVICE_MANAGER_DB_FLUSH_BLOCK);
    } else if (0 != result) {
        return false;
    }
    return true;
}

void bt_device_manager_db_init(bt_device_manager_db_type_t db_type,
                               bt_device_manager_db_storage_t *storage, void *db_buffer, uint32_t buffer_size)
{
    bt_dmgr_report_id("[BT_DM][DB][I] Init db_type:%d, db_buffer:0x%x, buffer_size:%d", 3, db_type, db_buffer, buffer_size);
    if (NULL == db_buffer || NULL == storage || 0 == buffer_size || db_type >= BT_DEVICE_MANAGER_DB_TYPE_MAX) {
        return;
    }
    dev_db_cnt[db_type].buffer_size = buffer_size;
    dev_db_cnt[db_type].buffer = (uint8_t *)db_buffer;
    dev_db_cnt[db_type].flag = BT_DEVICE_MANAGER_DB_FLAG_INIT;
    dev_db_cnt[db_type].type = db_type;
    dev_db_cnt[db_type].need_write_count = 0;
    dev_db_cnt[db_type].writing_count = 0;
    memcpy(&(dev_db_cnt[db_type].storage), storage, sizeof(*storage));
    bool ret = bt_device_manager_db_storage_read(db_type);
    bt_utils_assert(ret == true && "Init local info db fail");
}

void bt_device_manager_db_open(bt_device_manager_db_type_t db_type)
{
    bt_device_manager_db_log_print(BT_DM_DB_LOG_HANDLE_OPEN, db_type);
    if (db_type >= BT_DEVICE_MANAGER_DB_TYPE_MAX) {
        return;
    }
    if (!(dev_db_cnt[db_type].flag & BT_DEVICE_MANAGER_DB_FLAG_INIT)) {
        bool ret = bt_device_manager_db_storage_read(db_type);
        bt_utils_assert(ret == true && "Open db fail");
        dev_db_cnt[db_type].flag |= BT_DEVICE_MANAGER_DB_FLAG_INIT;
    }
    dev_db_cnt[db_type].need_write_count = 0;
    dev_db_cnt[db_type].writing_count = 0;
}

void bt_device_manager_db_close(bt_device_manager_db_type_t db_type)
{
    bt_device_manager_db_log_print(BT_DM_DB_LOG_HANDLE_CLOSE, db_type);
    if (db_type >= BT_DEVICE_MANAGER_DB_TYPE_MAX) {
        return;
    }
    dev_db_cnt[db_type].flag &= (~BT_DEVICE_MANAGER_DB_FLAG_INIT);
    dev_db_cnt[db_type].need_write_count = 0;
    dev_db_cnt[db_type].writing_count = 0;
}

void bt_device_manager_db_update(bt_device_manager_db_type_t db_type)
{
    bt_device_manager_db_log_print(BT_DM_DB_LOG_HANDLE_UPDATE, db_type);
    if (db_type >= BT_DEVICE_MANAGER_DB_TYPE_MAX || !(dev_db_cnt[db_type].flag & BT_DEVICE_MANAGER_DB_FLAG_INIT)) {
        return;
    }
    dev_db_cnt[db_type].need_write_count++;
}

bool bt_device_manager_db_read(bt_device_manager_db_type_t db_type, void *db_buffer, uint32_t buffer_size)
{
    bt_dmgr_report_id("[BT_DM][DB][I] read db_type:%d, db_buffer:0x%x, buffer_size:%d", 3, db_type, db_buffer, buffer_size);
    if (NULL == db_buffer || 0 == buffer_size || db_type >= BT_DEVICE_MANAGER_DB_TYPE_MAX) {
        return false;
    }
    int32_t result = -1000;
    uint32_t temp_size = buffer_size;
#ifdef MTK_NVDM_ENABLE
    if (BT_DEVICE_MANAGER_DB_STORAGE_TYPE_NVDM == dev_db_cnt[db_type].storage.storage_type) {
        result = (int32_t)nvdm_read_data_item(dev_db_cnt[db_type].storage.nvdm_group_str,
                                                dev_db_cnt[db_type].storage.nvdm_item_str, db_buffer, &temp_size);

    } else if (BT_DEVICE_MANAGER_DB_STORAGE_TYPE_NVKEY == dev_db_cnt[db_type].storage.storage_type) {
        result = (int32_t)nvkey_read_data(dev_db_cnt[db_type].storage.nvkey_id,
                                                db_buffer, &temp_size);        
    }
#endif
    bt_dmgr_report_id("[BT_DM][DB][I] Storage real size %d, read size %d, result %d", 3,
                          dev_db_cnt[db_type].buffer_size, temp_size, result);
    if (0 != result) {
        return false;
    }
    return true;
}

void bt_device_manager_db_flush(bt_device_manager_db_type_t db_type, bt_device_manager_db_flush_t block)
{
    bt_device_manager_db_log_print(BT_DM_DB_LOG_HANDLE_FLUSH, db_type);
    if (db_type >= BT_DEVICE_MANAGER_DB_TYPE_MAX || !(dev_db_cnt[db_type].flag & BT_DEVICE_MANAGER_DB_FLAG_INIT)) {
        return;
    }
    bt_dmgr_report_id("[BT_DM][DB][I] Flush db need_write_count:%d, writing_count:%d, block:%d", 3,
        dev_db_cnt[db_type].need_write_count, dev_db_cnt[db_type].writing_count, block);
    bt_device_manager_db_mutex_take();
    if (dev_db_cnt[db_type].need_write_count || (BT_DEVICE_MANAGER_DB_FLUSH_BLOCK == block && dev_db_cnt[db_type].writing_count)) {
        bool ret = bt_device_manager_db_storage_write(db_type, block);
        bt_utils_assert(ret == true && " flush db fail");
        dev_db_cnt[db_type].need_write_count = 0;
        dev_db_cnt[db_type].writing_count = 
            (BT_DEVICE_MANAGER_DB_FLUSH_NON_BLOCK == block ? (dev_db_cnt[db_type].writing_count + dev_db_cnt[db_type].need_write_count) : 0);
    }
    bt_device_manager_db_mutex_give();
}

void bt_device_manager_db_flush_all(bt_device_manager_db_flush_t block)
{
    bt_device_manager_db_type_t db_item = 0;
    bt_device_manager_db_mutex_take();
    while (db_item < BT_DEVICE_MANAGER_DB_TYPE_MAX) {
        bt_device_manager_db_flush(db_item, block);
        db_item++;
    }
    bt_device_manager_db_mutex_give();
}

void bt_device_manager_db_mutex_take()
{
    if (0 == g_dev_db_mutex) {
        g_dev_db_mutex = bt_os_layer_create_mutex();
        bt_utils_assert(0 != g_dev_db_mutex);
    }
    bt_os_layer_take_mutex(g_dev_db_mutex);
}

void bt_device_manager_db_mutex_give()
{
    if (g_dev_db_mutex) {
        bt_os_layer_give_mutex(g_dev_db_mutex);
    }
}

