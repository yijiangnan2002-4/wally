/*
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
#ifdef AIR_LE_AUDIO_ENABLE

#include "apps_debug.h"

#include "bt_gap_le.h"
#include "bt_gatts.h"

#include "bt_type.h"
#include "bt_connection_manager.h"

/************************************************
 *   Macro and Structure
*************************************************/
#define LOG_TAG     "[LEA][DHSS][BLE]"

#define DHSS_START_HANDLE               (0xA401)
#define DHSS_PAIR_ADDR_VALUE_HANDLE     (0xA403)
#define DHSS_END_HANDLE                 (0xA403)

#define DHSS_DATA_OFFSET_LE_ADDR_TYPE   6
#define DHSS_DATA_OFFSET_LE_ADDR        7

#define DHSS_DATA_SIZE_PAIR_ADDRESS     13      /* EDR Addr (6 bytes) + LE AddrType (1 byte) + LE Addr (6 bytes) */

#define DUAL_HEADSET_SINK_SERVICE_UUID  \
    {{0xac, 0x55, 0x11, 0x6a, 0xf2, 0x1b, 0x9c, 0x83, \
      0xeb, 0x4c, 0x7c, 0x29, 0xe6, 0x2f, 0xbf, 0x32 }}

#define DUAL_HEADSET_PAIR_ADDRESS_CHARC_UUID    \
    {{0x55, 0x43, 0x44, 0xf4, 0x9f, 0xa8, 0xdc, 0x8b, \
      0x31, 0x46, 0x53, 0x9d, 0xf5, 0x84, 0xb1, 0x5a }}

const bt_uuid_t DUAL_HEADSET_PAIR_ADDRESS_CHARC_UUID128 = {
    {
        0x55, 0x43, 0x44, 0xf4, 0x9f, 0xa8, 0xdc, 0x8b,
        0x31, 0x46, 0x53, 0x9d, 0xf5, 0x84, 0xb1, 0x5a
    }
};

static uint32_t app_le_audio_dhss_pair_address_charc_value_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);
extern bool app_le_audio_dhss_get_peer_le_addr(bt_addr_t *addr);

BT_GATTS_NEW_PRIMARY_SERVICE_128(ble_dhss_primary_service, DUAL_HEADSET_SINK_SERVICE_UUID);

BT_GATTS_NEW_CHARC_128(ble_dhss_pair_address_charc, BT_GATT_CHARC_PROP_READ,
                       DHSS_PAIR_ADDR_VALUE_HANDLE, DUAL_HEADSET_PAIR_ADDRESS_CHARC_UUID);
BT_GATTS_NEW_CHARC_VALUE_CALLBACK(ble_dhss_pair_address_charc_value, DUAL_HEADSET_PAIR_ADDRESS_CHARC_UUID128,
                                  BT_GATTS_REC_PERM_READABLE, app_le_audio_dhss_pair_address_charc_value_callback);

static const bt_gatts_service_rec_t *ble_dhss_service_rec[] = {
    (const bt_gatts_service_rec_t *) &ble_dhss_primary_service,
    (const bt_gatts_service_rec_t *) &ble_dhss_pair_address_charc,
    (const bt_gatts_service_rec_t *) &ble_dhss_pair_address_charc_value,
};

const bt_gatts_service_t ble_dhss_service = {
    .starting_handle = DHSS_START_HANDLE,
    .ending_handle = DHSS_END_HANDLE,
    .required_encryption_key_size = 0,
    .records = ble_dhss_service_rec,
};

/**************************************************************************************************
 * Static function
**************************************************************************************************/
static void app_le_audio_dhss_get_edr_addr(uint8_t *buf)
{
    bt_bd_addr_t *edr_addr = NULL;
    uint8_t i;

    if (NULL == buf) {
        //APPS_LOG_MSGID_E(LOG_TAG" get_edr_addr, NULL buf", 0);
        return;
    }

    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    if (role == BT_AWS_MCE_ROLE_PARTNER) {
        edr_addr = bt_connection_manager_device_local_info_get_peer_aws_address();
    } else {
        edr_addr = bt_connection_manager_device_local_info_get_local_address();
    }

    if (NULL == edr_addr) {
        //APPS_LOG_MSGID_E(LOG_TAG" get_edr_addr, NULL EDR Addr", 0);
        return;
    }

    uint8_t *addr = (uint8_t *)edr_addr;
    for (i = 0; i < BT_BD_ADDR_LEN ; i++) {
        buf[i] = addr[BT_BD_ADDR_LEN - i - 1];
    }

    APPS_LOG_MSGID_I(LOG_TAG" get_edr_addr, [%02X] addr=%02X:%02X:%02X:%02X:%02X:%02X",
                     7, role, buf[5], buf[4], buf[3], buf[2], buf[1], buf[0]);
}

static uint32_t app_le_audio_dhss_pair_address_charc_value_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    bt_addr_t le_addr;

    if (BT_HANDLE_INVALID == handle) {
        return 0;
    } else if (rw != BT_GATTS_CALLBACK_READ) {
        return 0;
    } else if (NULL == data) {
        return DHSS_DATA_SIZE_PAIR_ADDRESS;
    }

    memset(data, 0, DHSS_DATA_SIZE_PAIR_ADDRESS);
    if (app_le_audio_dhss_get_peer_le_addr(&le_addr)) {
        uint8_t *dhss_buf = (uint8_t *)data;
        /* EDR Addr */
        app_le_audio_dhss_get_edr_addr(dhss_buf);
        /* LE Addr */
        memcpy(&dhss_buf[DHSS_DATA_OFFSET_LE_ADDR_TYPE], &le_addr, sizeof(bt_addr_t));

        APPS_LOG_MSGID_I(LOG_TAG" read EDR_Peer_LE_ADDR, edr=%02X:%02X:%02X:%02X:%02X:%02X",
                         6, dhss_buf[0], dhss_buf[1], dhss_buf[2], dhss_buf[3], dhss_buf[4], dhss_buf[5]);
        APPS_LOG_MSGID_I(LOG_TAG" read EDR_Peer_LE_ADDR, le_addr=%d %02X:%02X:%02X:%02X:%02X:%02X",
                         7, dhss_buf[6], dhss_buf[7], dhss_buf[8], dhss_buf[9], dhss_buf[10], dhss_buf[11], dhss_buf[12]);
    } else {
        APPS_LOG_MSGID_E(LOG_TAG" read EDR_Peer_LE_ADDR, get peer LE Addr fail", 0);
    }

    return DHSS_DATA_SIZE_PAIR_ADDRESS;
}

#endif  /* AIR_LE_AUDIO_ENABLE */

