/* Copyright Statement:
 *
 * (C) 2023  Airoha Technology Corp. All rights reserved.
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

#include "bt_ull_le_hid_device_manager.h"
#include "bt_ull_le_utility.h"
#include "bt_ull_le_hid_utility.h"
#include "hal_trng.h"
#include "nvkey.h"
#include "nvkey_id_list.h"
#define BT_ULL_HID_DM_LOG   "[ULL][LE][HID][DM] "
static uint8_t uni_aa[BT_ULL_LE_HID_DM_UNI_AA_LEN] = {0x6D, 0xEB, 0x98, 0xE9};
static uint8_t uni_ltk[BT_ULL_LE_HID_DM_LTK_LEN]   = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10};
static uint8_t uni_skd[BT_ULL_LE_HID_DM_SKD_LEN]   = {0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20};
static uint8_t uni_iv[BT_ULL_LE_HID_DM_IV_LEN]     = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38};

typedef struct {
    bool                        used;
    bt_ull_le_hid_dm_device_info_t device_info;
} bt_ull_le_hid_dm_device_nvkey_t;

bt_ull_le_hid_dm_device_nvkey_t g_headset[BT_ULL_LE_HID_DM_DEVICE_NUM_MAX];
bt_ull_le_hid_dm_device_nvkey_t g_mouse[BT_ULL_LE_HID_DM_DEVICE_NUM_MAX];
bt_ull_le_hid_dm_device_nvkey_t g_keyboard[BT_ULL_LE_HID_DM_DEVICE_NUM_MAX];
bool g_ull_le_hid_device_manager_init = false;
static bool g_ull_le_hid_dm_test_mode = false;
bt_status_t bt_ull_le_hid_dm_init(void)
{
    nvkey_status_t status = NVKEY_STATUS_ERROR;
    if (g_ull_le_hid_device_manager_init) {
        return BT_STATUS_SUCCESS;
    }
    uint32_t size1 = sizeof(bt_ull_le_hid_dm_device_nvkey_t) * BT_ULL_LE_HID_DM_DEVICE_NUM_MAX;
    uint32_t size2 = sizeof(bt_ull_le_hid_dm_device_nvkey_t) * BT_ULL_LE_HID_DM_DEVICE_NUM_MAX;
    uint32_t size3 = sizeof(bt_ull_le_hid_dm_device_nvkey_t) * BT_ULL_LE_HID_DM_DEVICE_NUM_MAX;
    bt_ull_le_srv_memset(&g_headset[0], 0, size1);
    bt_ull_le_srv_memset(&g_mouse[0], 0, size2);
    bt_ull_le_srv_memset(&g_keyboard[0], 0, size3);
    status = nvkey_read_data(NVID_BT_HOST_ULL_HID_HS_INFO, (uint8_t *)&g_headset[0], &size1);
    if (NVKEY_STATUS_OK != status && NVKEY_STATUS_ITEM_NOT_FOUND != status) {
        ull_report_error(BT_ULL_HID_DM_LOG"dm_init_device_info, error status-1:%d, size: %d", 2, status, size1);
        return BT_STATUS_FAIL;
    }
    bt_ull_le_hid_srv_print_addr(&g_headset[0].device_info.addr);
    status = nvkey_read_data(NVID_BT_HOST_ULL_HID_KB_INFO, (uint8_t *)&g_keyboard[0], &size2);
    if (NVKEY_STATUS_OK != status && NVKEY_STATUS_ITEM_NOT_FOUND != status) {
        ull_report_error(BT_ULL_HID_DM_LOG"dm_init_device_info, error status-2:%d, size: %d", 2, status, size2);
        return BT_STATUS_FAIL;
    }
    status = nvkey_read_data(NVID_BT_HOST_ULL_HID_MS_INFO, (uint8_t *)&g_mouse[0], &size3);
    if (NVKEY_STATUS_OK != status && NVKEY_STATUS_ITEM_NOT_FOUND != status) {
        ull_report_error(BT_ULL_HID_DM_LOG"dm_init_device_info, error status-3:%d, size: %d", 2, status, size3);
        return BT_STATUS_FAIL;
    }
    g_ull_le_hid_device_manager_init = true;
    ull_report(BT_ULL_HID_DM_LOG"dm_init_device_info, init success!!", 0);

    return BT_STATUS_SUCCESS;
}

bt_status_t bt_ull_le_hid_dm_write_nvdm(bt_ull_le_hid_srv_device_t device_type)
{
    nvkey_status_t status = NVKEY_STATUS_ERROR;
    uint32_t size = sizeof(bt_ull_le_hid_dm_device_nvkey_t) * BT_ULL_LE_HID_DM_DEVICE_NUM_MAX;
    switch (device_type) {
    case BT_ULL_LE_HID_SRV_DEVICE_HEADSET: {
        status = nvkey_write_data(NVID_BT_HOST_ULL_HID_HS_INFO, (uint8_t *)&g_headset[0], size);
        break;
    }
    case BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD: {
        status = nvkey_write_data(NVID_BT_HOST_ULL_HID_KB_INFO, (uint8_t *)&g_keyboard[0], size);
        break;
    }
    case BT_ULL_LE_HID_SRV_DEVICE_MOUSE: {
        status = nvkey_write_data(NVID_BT_HOST_ULL_HID_MS_INFO, (uint8_t *)&g_mouse[0], size);
        break;
    }
    default:
        break;
    }
    ull_report(BT_ULL_HID_DM_LOG"dm_write_nvdm, status: %d", 1, status);
    return NVKEY_STATUS_OK == status ? BT_STATUS_SUCCESS : BT_STATUS_FAIL;
}

static bt_ull_le_hid_dm_device_nvkey_t *bt_ull_le_hid_dm_get_available_info(bt_ull_le_hid_srv_device_t device_type)
{
    uint8_t i = 0x0;
    bt_ull_le_hid_dm_device_nvkey_t temp_device[BT_ULL_LE_HID_DM_DEVICE_NUM_MAX] = {0};
    switch (device_type) {
    case BT_ULL_LE_HID_SRV_DEVICE_HEADSET: {
        if (BT_ULL_LE_HID_DM_DEVICE_NUM_MAX > 1) {
            for (i = 0; i < BT_ULL_LE_HID_DM_DEVICE_NUM_MAX; i ++) {
                bt_ull_le_srv_memcpy(&temp_device[i], &g_headset[i], sizeof(bt_ull_le_hid_dm_device_nvkey_t));
            }
            bt_ull_le_srv_memset(&g_headset[0], 0, sizeof(bt_ull_le_hid_dm_device_nvkey_t) * BT_ULL_LE_HID_DM_DEVICE_NUM_MAX);
            for (i = 0; i < BT_ULL_LE_HID_DM_DEVICE_NUM_MAX - 1; i ++) {
                bt_ull_le_srv_memcpy(&g_headset[i+1], &temp_device[i], sizeof(bt_ull_le_hid_dm_device_nvkey_t));
            }
        } else if (BT_ULL_LE_HID_DM_DEVICE_NUM_MAX == 1) {
            bt_ull_le_srv_memset(&g_headset[0], 0, sizeof(bt_ull_le_hid_dm_device_nvkey_t) * BT_ULL_LE_HID_DM_DEVICE_NUM_MAX);
        } else {
             assert(0);
        }
        return &g_headset[0];
        break;
    }
    case BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD: {
        if (BT_ULL_LE_HID_DM_DEVICE_NUM_MAX > 1) {
            for (i = 0; i < BT_ULL_LE_HID_DM_DEVICE_NUM_MAX; i ++) {
                bt_ull_le_srv_memcpy(&temp_device[i], &g_keyboard[i], sizeof(bt_ull_le_hid_dm_device_nvkey_t));
            }
            bt_ull_le_srv_memset(&g_keyboard[0], 0, sizeof(bt_ull_le_hid_dm_device_nvkey_t) * BT_ULL_LE_HID_DM_DEVICE_NUM_MAX);
            for (i = 0; i < BT_ULL_LE_HID_DM_DEVICE_NUM_MAX - 1; i ++) {
                bt_ull_le_srv_memcpy(&g_keyboard[i+1], &temp_device[i], sizeof(bt_ull_le_hid_dm_device_nvkey_t));
            }
        } else if (BT_ULL_LE_HID_DM_DEVICE_NUM_MAX == 1) {
            bt_ull_le_srv_memset(&g_keyboard[0], 0, sizeof(bt_ull_le_hid_dm_device_nvkey_t) * BT_ULL_LE_HID_DM_DEVICE_NUM_MAX);
        } else {
             assert(0);
        }
        return &g_keyboard[0];
        break;

    }
    case BT_ULL_LE_HID_SRV_DEVICE_MOUSE: {
        if (BT_ULL_LE_HID_DM_DEVICE_NUM_MAX > 1) {
            for (i = 0; i < BT_ULL_LE_HID_DM_DEVICE_NUM_MAX; i ++) {
                bt_ull_le_srv_memcpy(&temp_device[i], &g_mouse[i], sizeof(bt_ull_le_hid_dm_device_nvkey_t));
            }
            bt_ull_le_srv_memset(&g_mouse[0], 0, sizeof(bt_ull_le_hid_dm_device_nvkey_t) * BT_ULL_LE_HID_DM_DEVICE_NUM_MAX);
            for (i = 0; i < BT_ULL_LE_HID_DM_DEVICE_NUM_MAX - 1; i ++) {
                bt_ull_le_srv_memcpy(&g_mouse[i+1], &temp_device[i], sizeof(bt_ull_le_hid_dm_device_nvkey_t));
            }
        } else if (BT_ULL_LE_HID_DM_DEVICE_NUM_MAX == 1) {
            bt_ull_le_srv_memset(&g_mouse[0], 0, sizeof(bt_ull_le_hid_dm_device_nvkey_t) * BT_ULL_LE_HID_DM_DEVICE_NUM_MAX);
        } else {
             assert(0);
        }
        return &g_mouse[0];
        break;
    }
    default:
        break;
    }
    return NULL;

}

bt_ull_le_hid_dm_device_info_t *bt_ull_le_hid_dm_read_device_info(bt_ull_le_hid_srv_device_t device_type, bt_addr_t *addr)
{
    uint8_t i = 0x0;
    switch (device_type) {
    case BT_ULL_LE_HID_SRV_DEVICE_HEADSET: {
        for (i = 0; i < BT_ULL_LE_HID_DM_DEVICE_NUM_MAX; i ++) {
            if (!bt_ull_le_srv_memcmp(addr, &g_headset[i].device_info.addr, sizeof(bt_addr_t)) && g_headset[i].used) {
                return &g_headset[i].device_info;
            }
        }
        break;
    }
    case BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD: {
        for (i = 0; i < BT_ULL_LE_HID_DM_DEVICE_NUM_MAX; i ++) {
            if (!bt_ull_le_srv_memcmp(addr, &g_keyboard[i].device_info.addr, sizeof(bt_addr_t)) && g_keyboard[i].used) {
                return &g_keyboard[i].device_info;
            }
        }
        break;
    }
    case BT_ULL_LE_HID_SRV_DEVICE_MOUSE: {
        for (i = 0; i < BT_ULL_LE_HID_DM_DEVICE_NUM_MAX; i ++) {
            if (!bt_ull_le_srv_memcmp(addr, &g_mouse[i].device_info.addr, sizeof(bt_addr_t)) && g_mouse[i].used) {
                return &g_mouse[i].device_info;
            }
        }
        break;
    }
    default:
        break;
    }
    return NULL;
}

bt_status_t bt_ull_le_hid_dm_write_device_info(bt_ull_le_hid_dm_device_info_t *info)
{
    ull_report(BT_ULL_HID_DM_LOG"dm_write_device_info, init: %d, test mode: %d", 2, g_ull_le_hid_device_manager_init, g_ull_le_hid_dm_test_mode);
    if (!info || !g_ull_le_hid_device_manager_init || g_ull_le_hid_dm_test_mode) {
        return BT_STATUS_FAIL;
    }
    if (bt_ull_le_hid_dm_read_device_info(info->device_type, &info->addr)) {
        ull_report_error(BT_ULL_HID_DM_LOG"dm_write_device_info, device has exist!!", 0);
        return BT_STATUS_SUCCESS;
    }

    bt_ull_le_hid_dm_device_nvkey_t *device = bt_ull_le_hid_dm_get_available_info(info->device_type);
    if (NULL == device) {
        ull_report_error(BT_ULL_HID_DM_LOG"dm_write_device_info, no source!!", 0);
        return BT_STATUS_FAIL;
    }
    device->used = true;
    bt_ull_le_srv_memcpy(&device->device_info, info, sizeof(bt_ull_le_hid_dm_device_info_t));
    bt_ull_le_hid_srv_print_addr(&info->addr);
    return bt_ull_le_hid_dm_write_nvdm(info->device_type);
}


#if defined (AIR_PURE_GAMING_ENABLE)
#include "bt_avm.h"

uint8_t bt_ull_le_hid_dm_get_default_tx_gc()
{
    #define TX_GC_SIZE (8)
    #define DEFAULT_TX_GC_VALUE (0x3D)

    uint8_t tx_gc_buf[TX_GC_SIZE] = {0};
    uint32_t tx_gc_len = sizeof(tx_gc_buf);
    nvkey_status_t status = nvkey_read_data(NVID_CAL_PWR_CTL_MP_K, (uint8_t *)tx_gc_buf, &tx_gc_len);
    if (status != NVKEY_STATUS_OK && NVKEY_STATUS_ITEM_NOT_FOUND != status) {
        ull_report_error(BT_ULL_HID_DM_LOG"bt_ull_le_hid_dm_get_default_tx_gc fail!", 0);
        return DEFAULT_TX_GC_VALUE;
    }

    ull_report(BT_ULL_HID_DM_LOG"bt_ull_le_hid_dm_get_default_tx_gc, tx gc(%02X %02X %02X %02X %02X %02X %02X %02X)", 8, \
    tx_gc_buf[0], tx_gc_buf[1], tx_gc_buf[2], tx_gc_buf[3], tx_gc_buf[4], tx_gc_buf[5], tx_gc_buf[6], tx_gc_buf[7]);

    if (tx_gc_buf[4] != tx_gc_buf[6]) {
        ull_report_error(BT_ULL_HID_DM_LOG"bt_ull_le_hid_dm_get_default_tx_gc LE1M(0x%X) is not equal with LE2M(0x%X)!", 2, tx_gc_buf[4], tx_gc_buf[6]);
        return DEFAULT_TX_GC_VALUE;
    }

    return tx_gc_buf[4] ;
}

uint8_t bt_ull_le_hid_dm_get_qc_tx_gc()
{
    #define QC_TX_GC_VALUE (0x23)

    uint8_t tx_gc_buf[1] = {0};
    uint32_t tx_gc_len = sizeof(tx_gc_buf);
    nvkey_status_t status = nvkey_read_data(NVID_APP_DONGLE_QC_TX_GC, (uint8_t *)tx_gc_buf, &tx_gc_len);
    if (status != NVKEY_STATUS_OK && NVKEY_STATUS_ITEM_NOT_FOUND != status) {
        ull_report_error(BT_ULL_HID_DM_LOG"bt_ull_le_hid_dm_get_qc_tx_gc fail!", 0);
        return QC_TX_GC_VALUE;
    }
    ull_report(BT_ULL_HID_DM_LOG"bt_ull_le_hid_dm_get_qc_tx_gc, tx gc(0x%02X)", 1, tx_gc_buf[0]);

    return tx_gc_buf[0];
}

bt_status_t bt_avm_vendor_set_tx_gc_value(uint8_t value)
{
    bt_avm_vendor_set_tx_power_t tx_power;
    tx_power.info[0] = 0x41;
    tx_power.info[1] = 0x50;
    tx_power.info[2] = 0x50;
    tx_power.info[3] = 0x7E;
    tx_power.info[4] = value;
    tx_power.info[5] = value;
    return bt_avm_vendor_set_tx_power(&tx_power);
}

bt_status_t bt_ull_le_hid_dm_enter_test_mode(bt_ull_le_hid_srv_device_t device_type)
{
    ull_report(BT_ULL_HID_DM_LOG"bt_ull_le_hid_dm_enter_test_mode, dt: %d", 1, device_type);

    switch (device_type) {
        case BT_ULL_LE_HID_SRV_DEVICE_HEADSET: {
            uint32_t size = sizeof(bt_ull_le_hid_dm_device_nvkey_t) * BT_ULL_LE_HID_DM_DEVICE_NUM_MAX;
            bt_ull_le_srv_memset(&g_headset[0], 0, size);
            bt_ull_le_hid_dm_device_nvkey_t dv_info;
            dv_info.used = true;
            dv_info.device_info.device_type = device_type;
            bt_addr_t test_addr = {0x01, {0x01, 0x02, 0x03, 0x04, 0x05, 0x07}};
            uint8_t headset_test_uni_aa[BT_ULL_LE_HID_DM_UNI_AA_LEN] = {0x61, 0x72, 0x83, 0x86};
            bt_ull_le_srv_memcpy(&dv_info.device_info.addr, &test_addr, sizeof(bt_addr_t));
            bt_ull_le_srv_memcpy(&dv_info.device_info.uni_aa, &headset_test_uni_aa, BT_ULL_LE_HID_DM_UNI_AA_LEN);
            bt_ull_le_srv_memcpy(&dv_info.device_info.ltk, &uni_ltk, BT_ULL_LE_HID_DM_LTK_LEN);
            bt_ull_le_srv_memcpy(&dv_info.device_info.skd, &uni_skd, BT_ULL_LE_HID_DM_SKD_LEN);
            bt_ull_le_srv_memcpy(&dv_info.device_info.iv, &uni_iv, BT_ULL_LE_HID_DM_IV_LEN);
            bt_ull_le_srv_memcpy(&g_headset[0], &dv_info, sizeof(bt_ull_le_hid_dm_device_nvkey_t));
            break;
        }
        case BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD: {
            uint32_t size = sizeof(bt_ull_le_hid_dm_device_nvkey_t) * BT_ULL_LE_HID_DM_DEVICE_NUM_MAX;
            bt_ull_le_srv_memset(&g_keyboard[0], 0, size);
            bt_ull_le_hid_dm_device_nvkey_t dv_info;
            dv_info.used = true;
            dv_info.device_info.device_type = device_type;
            bt_addr_t test_addr = {0x01, {0x01, 0x02, 0x03, 0x04, 0x05, 0x08}};
            uint8_t kb_test_uni_aa[BT_ULL_LE_HID_DM_UNI_AA_LEN] = {0x61, 0x72, 0x83, 0x97};
            bt_ull_le_srv_memcpy(&dv_info.device_info.addr, &test_addr, sizeof(bt_addr_t));
            bt_ull_le_srv_memcpy(&dv_info.device_info.uni_aa, &kb_test_uni_aa, BT_ULL_LE_HID_DM_UNI_AA_LEN);
            bt_ull_le_srv_memcpy(&dv_info.device_info.ltk, &uni_ltk, BT_ULL_LE_HID_DM_LTK_LEN);
            bt_ull_le_srv_memcpy(&dv_info.device_info.skd, &uni_skd, BT_ULL_LE_HID_DM_SKD_LEN);
            bt_ull_le_srv_memcpy(&dv_info.device_info.iv, &uni_iv, BT_ULL_LE_HID_DM_IV_LEN);
            bt_ull_le_srv_memcpy(&g_keyboard[0], &dv_info, sizeof(bt_ull_le_hid_dm_device_nvkey_t));
            break;
        }
        case BT_ULL_LE_HID_SRV_DEVICE_MOUSE: {
            uint32_t size = sizeof(bt_ull_le_hid_dm_device_nvkey_t) * BT_ULL_LE_HID_DM_DEVICE_NUM_MAX;
            bt_ull_le_srv_memset(&g_mouse[0], 0, size);
            bt_ull_le_hid_dm_device_nvkey_t dv_info;
            dv_info.used = true;
            dv_info.device_info.device_type = device_type;
            bt_addr_t test_addr = {0x01, {0x01, 0x02, 0x03, 0x04, 0x05, 0x06}};

#ifdef AIR_QC_DONGLE_ENABLE
            uint8_t mouse_test_uni_aa[BT_ULL_LE_HID_DM_UNI_AA_LEN] = {0x61, 0x72, 0x83, 0xEF};
#else
            uint8_t mouse_test_uni_aa[BT_ULL_LE_HID_DM_UNI_AA_LEN] = {0x62, 0x73, 0x84, 0xF0};
#endif
            bt_ull_le_srv_memcpy(&dv_info.device_info.addr, &test_addr, sizeof(bt_addr_t));
            bt_ull_le_srv_memcpy(&dv_info.device_info.uni_aa, &mouse_test_uni_aa, BT_ULL_LE_HID_DM_UNI_AA_LEN);
            bt_ull_le_srv_memcpy(&dv_info.device_info.ltk, &uni_ltk, BT_ULL_LE_HID_DM_LTK_LEN);
            bt_ull_le_srv_memcpy(&dv_info.device_info.skd, &uni_skd, BT_ULL_LE_HID_DM_SKD_LEN);
            bt_ull_le_srv_memcpy(&dv_info.device_info.iv, &uni_iv, BT_ULL_LE_HID_DM_IV_LEN);
            bt_ull_le_srv_memcpy(&g_mouse[0], &dv_info, sizeof(bt_ull_le_hid_dm_device_nvkey_t));
            break;
        }
        default:
            return BT_STATUS_FAIL;
    }
    g_ull_le_hid_dm_test_mode = true;

    uint8_t qc_tx_gc_value = bt_ull_le_hid_dm_get_qc_tx_gc();
    return bt_avm_vendor_set_tx_gc_value(qc_tx_gc_value);
}

bt_status_t bt_ull_le_hid_dm_exit_test_mode(bt_ull_le_hid_srv_device_t device_type)
{
    ull_report(BT_ULL_HID_DM_LOG"bt_ull_le_hid_dm_exit_test_mode, dt: %d", 1, device_type);

    uint32_t size = sizeof(bt_ull_le_hid_dm_device_nvkey_t) * BT_ULL_LE_HID_DM_DEVICE_NUM_MAX;
    bt_status_t status = BT_STATUS_FAIL;
    switch (device_type) {
        case BT_ULL_LE_HID_SRV_DEVICE_MOUSE: {
            bt_ull_le_srv_memset(&g_mouse[0], 0, size);
            status = nvkey_read_data(NVID_BT_HOST_ULL_HID_MS_INFO, (uint8_t *)&g_mouse[0], &size);
            if (NVKEY_STATUS_OK != status && NVKEY_STATUS_ITEM_NOT_FOUND != status) {
                ull_report_error(BT_ULL_HID_DM_LOG"bt_ull_le_hid_dm_exit_test_mode, error status-1:%d, size: %d", 2, status, size);
                return BT_STATUS_FAIL;
            }
            bt_ull_le_hid_srv_print_addr(&g_mouse[0].device_info.addr);
            break;
        }
        case BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD: {
            bt_ull_le_srv_memset(&g_keyboard[0], 0, size);
            status = nvkey_read_data(NVID_BT_HOST_ULL_HID_KB_INFO, (uint8_t *)&g_keyboard[0], &size);
            if (NVKEY_STATUS_OK != status && NVKEY_STATUS_ITEM_NOT_FOUND != status) {
                ull_report_error(BT_ULL_HID_DM_LOG"bt_ull_le_hid_dm_exit_test_mode, error status-1:%d, size: %d", 2, status, size);
                return BT_STATUS_FAIL;
            }
            bt_ull_le_hid_srv_print_addr(&g_keyboard[0].device_info.addr);
            break;
        }
        case BT_ULL_LE_HID_SRV_DEVICE_HEADSET: {
            bt_ull_le_srv_memset(&g_headset[0], 0, size);
            status = nvkey_read_data(NVID_BT_HOST_ULL_HID_HS_INFO, (uint8_t *)&g_headset[0], &size);
            if (NVKEY_STATUS_OK != status && NVKEY_STATUS_ITEM_NOT_FOUND != status) {
                ull_report_error(BT_ULL_HID_DM_LOG"bt_ull_le_hid_dm_exit_test_mode, error status-1:%d, size: %d", 2, status, size);
                return BT_STATUS_FAIL;
            }
            bt_ull_le_hid_srv_print_addr(&g_headset[0].device_info.addr);
            break;
        }
        default:
            return BT_STATUS_FAIL;
    }
    g_ull_le_hid_dm_test_mode = false;

    uint8_t default_tx_gc_value = bt_ull_le_hid_dm_get_default_tx_gc();
    return bt_avm_vendor_set_tx_gc_value(default_tx_gc_value);
}
#endif

bt_status_t bt_ull_le_hid_dm_shift_device_front(bt_ull_le_hid_srv_device_t device_type, bt_addr_t *addr)
{
    uint8_t i, j = 0;
    bt_status_t status = BT_STATUS_FAIL;
    ull_report(BT_ULL_HID_DM_LOG"bt_ull_le_hid_dm_shift_device_front, init: %d, test mode: %d", 2, g_ull_le_hid_device_manager_init, g_ull_le_hid_dm_test_mode);
    if (!g_ull_le_hid_device_manager_init || g_ull_le_hid_dm_test_mode) {
        return BT_STATUS_FAIL;
    }
    bt_ull_le_hid_dm_device_nvkey_t temp_device[BT_ULL_LE_HID_DM_DEVICE_NUM_MAX] = {0};
    switch (device_type) {
    case BT_ULL_LE_HID_SRV_DEVICE_HEADSET: {
        if (!bt_ull_le_hid_dm_read_device_info(device_type, addr)) {
            return status;
        }
        for (i = 0; i < BT_ULL_LE_HID_DM_DEVICE_NUM_MAX; i ++) {
            if (!bt_ull_le_srv_memcmp(addr, &g_headset[i].device_info.addr, sizeof(bt_addr_t)) && g_headset[i].used) {
                if (0 == i) {
                    return BT_STATUS_SUCCESS;
                }
                bt_ull_le_srv_memcpy(&temp_device[0], &g_headset[i], sizeof(bt_ull_le_hid_dm_device_nvkey_t));
                continue;
            }
            if (j >= BT_ULL_LE_HID_DM_DEVICE_NUM_MAX) {
                return BT_STATUS_FAIL;
            }
            bt_ull_le_srv_memcpy(&temp_device[j+1], &g_headset[i], sizeof(bt_ull_le_hid_dm_device_nvkey_t));
            j ++;
        }
        bt_ull_le_srv_memcpy(&g_headset[0], &temp_device[0], sizeof(bt_ull_le_hid_dm_device_nvkey_t) * BT_ULL_LE_HID_DM_DEVICE_NUM_MAX);
        status = bt_ull_le_hid_dm_write_nvdm(device_type);
        break;
    }
    case BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD: {
        if (!bt_ull_le_hid_dm_read_device_info(device_type, addr)) {
            return status;
        }
        for (i = 0; i < BT_ULL_LE_HID_DM_DEVICE_NUM_MAX; i ++) {
            if (!bt_ull_le_srv_memcmp(addr, &g_keyboard[i].device_info.addr, sizeof(bt_addr_t)) && g_keyboard[i].used) {
                if (0 == i) {
                    return BT_STATUS_SUCCESS;
                }
                bt_ull_le_srv_memcpy(&temp_device[0], &g_keyboard[i], sizeof(bt_ull_le_hid_dm_device_nvkey_t));
                continue;
            }
            if (j >= BT_ULL_LE_HID_DM_DEVICE_NUM_MAX) {
                return BT_STATUS_FAIL;
            }
            bt_ull_le_srv_memcpy(&temp_device[j+1], &g_keyboard[i], sizeof(bt_ull_le_hid_dm_device_nvkey_t));
            j ++;
        }
        bt_ull_le_srv_memcpy(&g_keyboard[0], &temp_device[0], sizeof(bt_ull_le_hid_dm_device_nvkey_t) * BT_ULL_LE_HID_DM_DEVICE_NUM_MAX);
        status = bt_ull_le_hid_dm_write_nvdm(device_type);
        break;

    }
    case BT_ULL_LE_HID_SRV_DEVICE_MOUSE: {
        if (!bt_ull_le_hid_dm_read_device_info(device_type, addr)) {
            return status;
        }
        for (i = 0; i < BT_ULL_LE_HID_DM_DEVICE_NUM_MAX; i ++) {
            if (!bt_ull_le_srv_memcmp(addr, &g_mouse[i].device_info.addr, sizeof(bt_addr_t)) && g_mouse[i].used) {
                if (0 == i) {
                    return BT_STATUS_SUCCESS;
                }
                bt_ull_le_srv_memcpy(&temp_device[0], &g_mouse[i], sizeof(bt_ull_le_hid_dm_device_nvkey_t));
                continue;
            }
            if (j >= BT_ULL_LE_HID_DM_DEVICE_NUM_MAX) {
                return BT_STATUS_FAIL;
            }
            bt_ull_le_srv_memcpy(&temp_device[j+1], &g_mouse[i], sizeof(bt_ull_le_hid_dm_device_nvkey_t));
            j ++;
        }
        bt_ull_le_srv_memcpy(&g_mouse[0], &temp_device[0], sizeof(bt_ull_le_hid_dm_device_nvkey_t) * BT_ULL_LE_HID_DM_DEVICE_NUM_MAX);
        status = bt_ull_le_hid_dm_write_nvdm(device_type);
        break;
    }
    default:
        break;
    }
    return status;
}

bt_status_t bt_ull_le_hid_dm_delete_device_info(bt_ull_le_hid_srv_device_t device_type, bt_addr_t *addr)
{
    uint8_t i , j = 0x0;
    bt_status_t status = BT_STATUS_FAIL;
    ull_report(BT_ULL_HID_DM_LOG"bt_ull_le_hid_dm_delete_device_info, init: %d, test mode: %d", 2, g_ull_le_hid_device_manager_init, g_ull_le_hid_dm_test_mode);
    if (!g_ull_le_hid_device_manager_init || g_ull_le_hid_dm_test_mode) {
        return BT_STATUS_FAIL;
    }

    bt_ull_le_hid_dm_device_nvkey_t temp_device[BT_ULL_LE_HID_DM_DEVICE_NUM_MAX] = {0};
    switch (device_type) {
    case BT_ULL_LE_HID_SRV_DEVICE_HEADSET: {
        if (!bt_ull_le_hid_dm_read_device_info(device_type, addr)) {
            return status;
        }
        for (i = 0; i < BT_ULL_LE_HID_DM_DEVICE_NUM_MAX; i ++) {
            if (!bt_ull_le_srv_memcmp(addr, &g_headset[i].device_info.addr, sizeof(bt_addr_t)) && g_headset[i].used) {
                continue;
            }
            bt_ull_le_srv_memcpy(&temp_device[j], &g_headset[i], sizeof(bt_ull_le_hid_dm_device_nvkey_t));
            j ++;
        }
        bt_ull_le_srv_memcpy(&g_headset[0], &temp_device[0], sizeof(bt_ull_le_hid_dm_device_nvkey_t) * BT_ULL_LE_HID_DM_DEVICE_NUM_MAX);
        status = bt_ull_le_hid_dm_write_nvdm(device_type);
        break;
    }
    case BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD: {
        if (!bt_ull_le_hid_dm_read_device_info(device_type, addr)) {
            return status;
        }
        for (i = 0; i < BT_ULL_LE_HID_DM_DEVICE_NUM_MAX; i ++) {
            if (!bt_ull_le_srv_memcmp(addr, &g_keyboard[i].device_info.addr, sizeof(bt_addr_t)) && g_keyboard[i].used) {
                continue;
            }
            bt_ull_le_srv_memcpy(&temp_device[j], &g_keyboard[i], sizeof(bt_ull_le_hid_dm_device_nvkey_t));
            j ++;
        }
        bt_ull_le_srv_memcpy(&g_keyboard[0], &temp_device[0], sizeof(bt_ull_le_hid_dm_device_nvkey_t) * BT_ULL_LE_HID_DM_DEVICE_NUM_MAX);
        status = bt_ull_le_hid_dm_write_nvdm(device_type);
        break;

    }
    case BT_ULL_LE_HID_SRV_DEVICE_MOUSE: {
        if (!bt_ull_le_hid_dm_read_device_info(device_type, addr)) {
            return status;
        }
        for (i = 0; i < BT_ULL_LE_HID_DM_DEVICE_NUM_MAX; i ++) {
            if (!bt_ull_le_srv_memcmp(addr, &g_mouse[i].device_info.addr, sizeof(bt_addr_t)) && g_mouse[i].used) {
                continue;
            }
            bt_ull_le_srv_memcpy(&temp_device[j], &g_mouse[i], sizeof(bt_ull_le_hid_dm_device_nvkey_t));
            j ++;
        }
        bt_ull_le_srv_memcpy(&g_mouse[0], &temp_device[0], sizeof(bt_ull_le_hid_dm_device_nvkey_t) * BT_ULL_LE_HID_DM_DEVICE_NUM_MAX);
        status = bt_ull_le_hid_dm_write_nvdm(device_type);
        break;
    }
    default:
        break;
    }
    return status;
}

uint8_t bt_ull_le_hid_dm_get_bonded_device_num(bt_ull_le_hid_srv_device_t device_type)
{
    if (!g_ull_le_hid_device_manager_init) {
        return 0;
    }

    uint8_t i, count = 0x0;
    switch (device_type) {
    case BT_ULL_LE_HID_SRV_DEVICE_HEADSET: {
        for (i = 0; i < BT_ULL_LE_HID_DM_DEVICE_NUM_MAX; i ++) {
            if (g_headset[i].used) {
                count ++;
            }
        }
        return count;
    }
    case BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD: {
        for (i = 0; i < BT_ULL_LE_HID_DM_DEVICE_NUM_MAX; i ++) {
            if (g_keyboard[i].used) {
                count ++;
            }
        }
        return count;
    }
    case BT_ULL_LE_HID_SRV_DEVICE_MOUSE: {
        for (i = 0; i < BT_ULL_LE_HID_DM_DEVICE_NUM_MAX; i ++) {
            if (g_mouse[i].used) {
                count ++;
            }
        }
        return count;
    }
    default:
        break;
    }
    ull_report(BT_ULL_HID_DM_LOG"dm_get_bonded_device_num, dt: %d, num: %d", 2, device_type, count);
    return count;
}
void bt_ull_le_hid_dm_get_bonded_device_list(bt_ull_le_hid_srv_device_t device_type, uint8_t count, bt_addr_t *list)
{
    if (!list || !count || !g_ull_le_hid_device_manager_init) {
        return;
    }
    uint8_t i = 0x0;
    switch (device_type) {
    case BT_ULL_LE_HID_SRV_DEVICE_HEADSET: {
        for (i = 0; i < count; i ++) {
            if (g_headset[i].used) {
                bt_ull_le_srv_memcpy(&list[i], &g_headset[i].device_info.addr, sizeof(bt_addr_t));
            }
        }
        break;
    }
    case BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD: {
        for (i = 0; i < count; i ++) {
            if (g_keyboard[i].used) {
                bt_ull_le_srv_memcpy(&list[i], &g_keyboard[i].device_info.addr, sizeof(bt_addr_t));
            }
        }
        break;

    }
    case BT_ULL_LE_HID_SRV_DEVICE_MOUSE: {
        for (i = 0; i < count; i ++) {
            if (g_mouse[i].used) {
                bt_ull_le_srv_memcpy(&list[i], &g_mouse[i].device_info.addr, sizeof(bt_addr_t));
            }
        }
        break;
    }
    default:
        break;
    }

}

bool bt_ull_le_hid_dm_is_bonded_device(bt_ull_le_hid_srv_device_t device_type, bt_addr_t *addr)
{
    uint8_t i = 0;
    switch (device_type) {
    case BT_ULL_LE_HID_SRV_DEVICE_HEADSET: {
        for (i = 0; i < BT_ULL_LE_HID_DM_DEVICE_NUM_MAX; i ++) {
            if (!bt_ull_le_srv_memcmp(addr, &g_headset[i].device_info.addr, sizeof(bt_addr_t)) && g_headset[i].used) {
                return true;
            }
        }
        break;
    }
    case BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD: {
        for (i = 0; i < BT_ULL_LE_HID_DM_DEVICE_NUM_MAX; i ++) {
            if (!bt_ull_le_srv_memcmp(addr, &g_keyboard[i].device_info.addr, sizeof(bt_addr_t)) && g_keyboard[i].used) {
                return true;
            }

        }
        break;

    }
    case BT_ULL_LE_HID_SRV_DEVICE_MOUSE: {
        for (i = 0; i < BT_ULL_LE_HID_DM_DEVICE_NUM_MAX; i ++) {
            if (!bt_ull_le_srv_memcmp(addr, &g_mouse[i].device_info.addr, sizeof(bt_addr_t)) && g_mouse[i].used) {
                return true;
            }

        }
        break;
    }
    default:
        break;
    }
    return false;
}

uint8_t *bt_ull_le_hid_dm_generate_uni_aa(void)
{
    uint32_t random_aa = 0x00;
    hal_trng_status_t status = hal_trng_get_generated_random_number(&random_aa);
    if (HAL_TRNG_STATUS_OK != status) {
        ull_report(BT_ULL_HID_DM_LOG"bt_ull_le_hid_dm_generate_uni_aa, fail! status : %d", 1, status);
        return &uni_aa[0];
    }
    bt_ull_le_srv_memcpy(&uni_aa[0], &random_aa, sizeof(BT_ULL_LE_HID_DM_UNI_AA_LEN));
    return &uni_aa[0];
}
uint8_t *bt_ull_le_hid_dm_get_ltk(void)
{
    return &uni_ltk[0];

}
uint8_t *bt_ull_le_hid_dm_get_skd(void)
{
    return &uni_skd[0];

}
uint8_t *bt_ull_le_hid_dm_get_iv(void)
{
    return &uni_iv[0];
}
bt_status_t bt_ull_le_hid_dm_clear_bonded_list(bt_ull_le_hid_srv_device_t device_type)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    uint32_t size = sizeof(bt_ull_le_hid_dm_device_nvkey_t) * BT_ULL_LE_HID_DM_DEVICE_NUM_MAX;
    switch (device_type) {
        case BT_ULL_LE_HID_SRV_DEVICE_HEADSET: {
            bt_ull_le_srv_memset(&g_headset[0], 0, size);
            status = bt_ull_le_hid_dm_write_nvdm(BT_ULL_LE_HID_SRV_DEVICE_HEADSET);
            break;
        }
        case BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD: {
            bt_ull_le_srv_memset(&g_keyboard[0], 0, size);
            status = bt_ull_le_hid_dm_write_nvdm(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
            break;
        }
        case BT_ULL_LE_HID_SRV_DEVICE_MOUSE: {
            bt_ull_le_srv_memset(&g_mouse[0], 0, size);
            status = bt_ull_le_hid_dm_write_nvdm(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
            break;
        }
        default:
            break;
    }
    ull_report(BT_ULL_HID_DM_LOG"bt_ull_le_hid_dm_clear_bonded_list, dt: %d, status: %x", 2, device_type, status);
    return status;
}


