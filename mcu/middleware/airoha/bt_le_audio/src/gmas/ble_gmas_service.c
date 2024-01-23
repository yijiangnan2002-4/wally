/*
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

#include "bt_gap_le.h"
#include "bt_gatts.h"

#include "ble_gmas.h"

/************************************************
* Define
*************************************************/
/**
 * @brief The GMAS service role support option.
 */
#ifdef AIR_BLE_AUDIO_DONGLE_ENABLE
#define BLE_GMAP_ROLE_UGG_ENABLE        1
#define BLE_GMAP_ROLE_UGT_ENABLE        0
#define BLE_GMAP_ROLE_BGS_ENABLE        1
#define BLE_GMAP_ROLE_BGR_ENABLE        0

/**
 * @brief The GMAS service start handle.
 */
#define GMAS_START_HANDLE                 (0xA701)
#define GMAS_VALUE_HANDLE_GMAP_ROLE       (0xA703)
#define GMAS_VALUE_HANDLE_UGG_FEATURE     (0xA705)
#define GMAS_VALUE_HANDLE_BGS_FEATURE     (0xA707)

#define GMAS_END_HANDLE                   (0xA707)
#else
#define BLE_GMAP_ROLE_UGG_ENABLE        0
#define BLE_GMAP_ROLE_UGT_ENABLE        1
#define BLE_GMAP_ROLE_BGS_ENABLE        0
#define BLE_GMAP_ROLE_BGR_ENABLE        1

/**
 * @brief The GMAS service start handle.
 */
#define GMAS_START_HANDLE                 (0xA701)
#define GMAS_VALUE_HANDLE_GMAP_ROLE       (0xA703)
#define GMAS_VALUE_HANDLE_UGT_FEATURE     (0xA705)
#define GMAS_VALUE_HANDLE_BGR_FEATURE     (0xA707)

#define GMAS_END_HANDLE                   (0xA707)
#endif

/**
 * @brief The GMAS service role support feature.
 */
#if BLE_GMAP_ROLE_UGG_ENABLE
//#define BLE_GMAS_UGG_SUPPORT_FEATURE    (BLE_GMAS_UGG_FEATURE_MULTIFRAME | BLE_GMAS_UGG_FEATURE_96K_SOURCE | BLE_GMAS_UGG_FEATURE_MULTI_SINK)
#define BLE_GMAS_UGG_SUPPORT_FEATURE    (BLE_GMAS_UGG_FEATURE_96K_SOURCE | BLE_GMAS_UGG_FEATURE_MULTI_SINK)
#endif
#if BLE_GMAP_ROLE_UGT_ENABLE
#define BLE_GMAS_UGT_SUPPORT_FEATURE    (BLE_GMAS_UGT_FEATURE_SOURCE | BLE_GMAS_UGT_FEATURE_80K_SOURCE | BLE_GMAS_UGT_FEATURE_SINK | \
                                         BLE_GMAS_UGT_FEATURE_64K_SINK | BLE_GMAS_UGT_FEATURE_MULTIFRAME | BLE_GMAS_UGT_FEATURE_MULTI_SINK)
#endif
#if BLE_GMAP_ROLE_BGS_ENABLE
#define BLE_GMAS_BGS_SUPPORT_FEATURE    (BLE_GMAS_BGS_FEATURE_96K)
#endif
#if BLE_GMAP_ROLE_BGR_ENABLE
#define BLE_GMAS_BGR_SUPPORT_FEATURE    (BLE_GMAS_BGR_FEATURE_MULTI_SINK | BLE_GMAS_BGR_FEATURE_MULTIFRAME)
#endif

/************************************************
*   UUID
*************************************************/
static const bt_uuid_t BT_SIG_UUID_GMAS_GMAP_ROLE = BT_UUID_INIT_WITH_UUID16(BT_SIG_UUID16_GMAS_GMAP_ROLE);
#if BLE_GMAP_ROLE_UGG_ENABLE
static const bt_uuid_t BT_SIG_UUID_GMAS_UGG_FEATURE = BT_UUID_INIT_WITH_UUID16(BT_SIG_UUID16_GMAS_UGG_FEATURE);
#endif
#if BLE_GMAP_ROLE_UGT_ENABLE
static const bt_uuid_t BT_SIG_UUID_GMAS_UGT_FEATURE = BT_UUID_INIT_WITH_UUID16(BT_SIG_UUID16_GMAS_UGT_FEATURE);
#endif
#if BLE_GMAP_ROLE_BGS_ENABLE
static const bt_uuid_t BT_SIG_UUID_GMAS_BGS_FEATURE = BT_UUID_INIT_WITH_UUID16(BT_SIG_UUID16_GMAS_BGS_FEATURE);
#endif
#if BLE_GMAP_ROLE_BGR_ENABLE
static const bt_uuid_t BT_SIG_UUID_GMAS_BGR_FEATURE = BT_UUID_INIT_WITH_UUID16(BT_SIG_UUID16_GMAS_BGR_FEATURE);
#endif
/************************************************
*   CALLBACK
*************************************************/
static uint32_t ble_gmas_gmap_role_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);
#if BLE_GMAP_ROLE_UGG_ENABLE
static uint32_t ble_gmas_ugg_feature_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);
#endif
#if BLE_GMAP_ROLE_UGT_ENABLE
static uint32_t ble_gmas_ugt_feature_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);
#endif
#if BLE_GMAP_ROLE_BGS_ENABLE
static uint32_t ble_gmas_bgs_feature_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);
#endif
#if BLE_GMAP_ROLE_BGR_ENABLE
static uint32_t ble_gmas_bgr_feature_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);
#endif

/************************************************
*   SERVICE TABLE
*************************************************/
BT_GATTS_NEW_PRIMARY_SERVICE_16(ble_gmas_primary_service, BT_SIG_UUID16_GMAS);

BT_GATTS_NEW_CHARC_16(ble_gmas_char4_gmap_role, BT_GATT_CHARC_PROP_READ, GMAS_VALUE_HANDLE_GMAP_ROLE, BT_SIG_UUID16_GMAS_GMAP_ROLE);
BT_GATTS_NEW_CHARC_VALUE_CALLBACK(ble_gmas_gmap_role, BT_SIG_UUID_GMAS_GMAP_ROLE, BT_GATTS_REC_PERM_READABLE, ble_gmas_gmap_role_callback);

#if BLE_GMAP_ROLE_UGG_ENABLE
BT_GATTS_NEW_CHARC_16(ble_gmas_char4_ugg_feature, BT_GATT_CHARC_PROP_READ, GMAS_VALUE_HANDLE_UGG_FEATURE, BT_SIG_UUID16_GMAS_UGG_FEATURE);
BT_GATTS_NEW_CHARC_VALUE_CALLBACK(ble_gmas_ugg_feature, BT_SIG_UUID_GMAS_UGG_FEATURE, BT_GATTS_REC_PERM_READABLE, ble_gmas_ugg_feature_callback);
#endif
#if BLE_GMAP_ROLE_UGT_ENABLE
BT_GATTS_NEW_CHARC_16(ble_gmas_char4_ugt_feature, BT_GATT_CHARC_PROP_READ, GMAS_VALUE_HANDLE_UGT_FEATURE, BT_SIG_UUID16_GMAS_UGT_FEATURE);
BT_GATTS_NEW_CHARC_VALUE_CALLBACK(ble_gmas_ugt_feature, BT_SIG_UUID_GMAS_UGT_FEATURE, BT_GATTS_REC_PERM_READABLE, ble_gmas_ugt_feature_callback);
#endif
#if BLE_GMAP_ROLE_BGS_ENABLE
BT_GATTS_NEW_CHARC_16(ble_gmas_char4_bgs_feature, BT_GATT_CHARC_PROP_READ, GMAS_VALUE_HANDLE_BGS_FEATURE, BT_SIG_UUID16_GMAS_BGS_FEATURE);
BT_GATTS_NEW_CHARC_VALUE_CALLBACK(ble_gmas_bgs_feature, BT_SIG_UUID_GMAS_BGS_FEATURE, BT_GATTS_REC_PERM_READABLE, ble_gmas_bgs_feature_callback);
#endif
#if BLE_GMAP_ROLE_BGR_ENABLE
BT_GATTS_NEW_CHARC_16(ble_gmas_char4_bgr_feature, BT_GATT_CHARC_PROP_READ, GMAS_VALUE_HANDLE_BGR_FEATURE, BT_SIG_UUID16_GMAS_BGR_FEATURE);
BT_GATTS_NEW_CHARC_VALUE_CALLBACK(ble_gmas_bgr_feature, BT_SIG_UUID_GMAS_BGR_FEATURE, BT_GATTS_REC_PERM_READABLE, ble_gmas_bgr_feature_callback);
#endif

static const bt_gatts_service_rec_t *ble_gmas_service_rec[] = {
    (const bt_gatts_service_rec_t *) &ble_gmas_primary_service,
    (const bt_gatts_service_rec_t *) &ble_gmas_char4_gmap_role,
    (const bt_gatts_service_rec_t *) &ble_gmas_gmap_role,
#if BLE_GMAP_ROLE_UGG_ENABLE
    (const bt_gatts_service_rec_t *) &ble_gmas_char4_ugg_feature,
    (const bt_gatts_service_rec_t *) &ble_gmas_ugg_feature,
#endif
#if BLE_GMAP_ROLE_UGT_ENABLE
    (const bt_gatts_service_rec_t *) &ble_gmas_char4_ugt_feature,
    (const bt_gatts_service_rec_t *) &ble_gmas_ugt_feature,
#endif
#if BLE_GMAP_ROLE_BGS_ENABLE
    (const bt_gatts_service_rec_t *) &ble_gmas_char4_bgs_feature,
    (const bt_gatts_service_rec_t *) &ble_gmas_bgs_feature,
#endif
#if BLE_GMAP_ROLE_BGR_ENABLE
    (const bt_gatts_service_rec_t *) &ble_gmas_char4_bgr_feature,
    (const bt_gatts_service_rec_t *) &ble_gmas_bgr_feature,
#endif
};

const bt_gatts_service_t ble_gmas_service = {
    .starting_handle = GMAS_START_HANDLE,
    .ending_handle = GMAS_END_HANDLE,
    .required_encryption_key_size = 7,
    .records = ble_gmas_service_rec
};

/************************************************
*   CALLBACK
*************************************************/
static uint32_t ble_gmas_gmap_role_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    if (handle != BT_HANDLE_INVALID && rw == BT_GATTS_CALLBACK_READ) {
        return ble_gmas_gatt_request_handler(BLE_GMAS_READ_ROLE, handle, data, size, offset);
    }

    return 0;
}

#if BLE_GMAP_ROLE_UGG_ENABLE
static uint32_t ble_gmas_ugg_feature_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    if (handle != BT_HANDLE_INVALID && rw == BT_GATTS_CALLBACK_READ) {
        return ble_gmas_gatt_request_handler(BLE_GMAS_READ_UGG_FEATURE, handle, data, size, offset);
    }

    return 0;
}
#endif
#if BLE_GMAP_ROLE_UGT_ENABLE
static uint32_t ble_gmas_ugt_feature_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    if (handle != BT_HANDLE_INVALID && rw == BT_GATTS_CALLBACK_READ) {
        return ble_gmas_gatt_request_handler(BLE_GMAS_READ_UGT_FEATURE, handle, data, size, offset);
    }

    return 0;
}
#endif
#if BLE_GMAP_ROLE_BGS_ENABLE
static uint32_t ble_gmas_bgs_feature_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    if (handle != BT_HANDLE_INVALID && rw == BT_GATTS_CALLBACK_READ) {
        return ble_gmas_gatt_request_handler(BLE_GMAS_READ_BGS_FEATURE, handle, data, size, offset);
    }

    return 0;
}
#endif
#if BLE_GMAP_ROLE_BGR_ENABLE
static uint32_t ble_gmas_bgr_feature_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    if (handle != BT_HANDLE_INVALID && rw == BT_GATTS_CALLBACK_READ) {
        return ble_gmas_gatt_request_handler(BLE_GMAS_READ_BGR_FEATURE, handle, data, size, offset);
    }

    return 0;
}
#endif

/************************************************
* Public Function
*************************************************/
bt_status_t ble_gmas_init(void)
{
    bt_status_t rv;

#if BLE_GMAP_ROLE_UGG_ENABLE
    if ((rv = ble_gmas_set_feature(GMAS_VALUE_HANDLE_UGG_FEATURE, BLE_GMAP_ROLE_MASK_UGG, BLE_GMAS_UGG_SUPPORT_FEATURE)) != BT_STATUS_SUCCESS) {
        return rv;
    }
#endif
#if BLE_GMAP_ROLE_UGT_ENABLE
    if ((rv = ble_gmas_set_feature(GMAS_VALUE_HANDLE_UGT_FEATURE, BLE_GMAP_ROLE_MASK_UGT, BLE_GMAS_UGT_SUPPORT_FEATURE)) != BT_STATUS_SUCCESS) {
        return rv;
    }
#endif
#if BLE_GMAP_ROLE_BGS_ENABLE
    if ((rv = ble_gmas_set_feature(GMAS_VALUE_HANDLE_BGS_FEATURE, BLE_GMAP_ROLE_MASK_BGS, BLE_GMAS_BGS_SUPPORT_FEATURE)) != BT_STATUS_SUCCESS) {
        return rv;
    }
#endif
#if BLE_GMAP_ROLE_BGR_ENABLE
    if ((rv = ble_gmas_set_feature(GMAS_VALUE_HANDLE_BGR_FEATURE, BLE_GMAP_ROLE_MASK_BGR, BLE_GMAS_BGR_SUPPORT_FEATURE)) != BT_STATUS_SUCCESS) {
        return rv;
    }
#endif

    return BT_STATUS_SUCCESS;
}

