
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

/**
 * File: app_va_xiaoai_miui_fast_connect.c
 *
 * Description: This file provides MIUI Fast connect feature (same account spec).
 *
 */

#ifdef AIR_XIAOAI_MIUI_FAST_CONNECT_ENABLE

#include "app_va_xiaoai_miui_fast_connect.h"

#include "xiaoai.h"

#include "app_va_xiaoai_config.h"
#include "app_va_xiaoai_ble_adv.h"
#include "apps_debug.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"

#ifdef MTK_AWS_MCE_ENABLE
#include "apps_aws_sync_event.h"
#include "bt_aws_mce_srv.h"
#endif
#include "bt_callback_manager.h"
#include "bt_connection_manager.h"
#include "bt_device_manager.h"
#include "bt_device_manager_le.h"
#include "bt_gatts.h"
#include "bt_gap_le_service.h"
#include "multi_ble_adv_manager.h"
#include "bt_sink_srv.h"
#include "bt_sink_srv_ami.h"
#include "mbedtls/aes.h"
#include "mbedtls/bignum.h"
#include "mbedtls/ecp.h"
#include "mbedtls/ecdh.h"
#include "mbedtls/sha256.h"
#include "nvkey.h"
#include "nvkey_id_list.h"
#include "FreeRTOS.h"
#include "task.h"
#include "ui_shell_manager.h"

#define LOG_TAG           "[MIUI_FC]"

#ifndef PACKED
#define PACKED  __attribute__((packed))
#endif

extern void app_va_xiaoai_miui_fc_start_adv();
extern void bt_device_manager_set_io_capability(bt_gap_io_capability_t io_capability);
extern void bt_os_layer_ecdh256(uint8_t dh_key[32], uint8_t public_key[64], uint8_t private_key[32]);
extern void bt_os_layer_generate_random_block(uint8_t *random_block, uint8_t block_size);

#define MIUI_FC_MIN(X, Y)         ((X) < (Y) ? (X) : (Y))

#define MIUI_FC_REVERSE_UINT32(X)   (((X) << 24) | (((X) << 8) & 0xFF0000) | \
                                    (((X) >> 8) & 0xFF00) | ((X) >> 24))

/*------------------------------------------------------------*/
/*                    BLE GATT Services                       */
/*------------------------------------------------------------*/

#define MIUI_FC_BLE_SRV_UUID                      (0xFE2C)

#define MIUI_FC_BLE_DEFAULT_MTU                   (512)

#define MIUI_FC_PRODUCT_ID_CHAR_UUID              (0xFF10)     /* Product ID Characteristic UUID. */
#define MIUI_FC_PAIRING_CHAR_UUID                 (0xFF11)     /* pairing Characteristic UUID. */
#define MIUI_FC_PASSKEY_CHAR_UUID                 (0xFF12)     /* Passkey Characteristic UUID. */
#define MIUI_FC_ACCOUNT_KEY_CHAR_UUID             (0xFF13)     /* Account key Characteristic UUID. */

#define MIUI_FC_PRODUCT_ID_CHAR_VALUE_HANDLE      (0x00E2)
#define MIUI_FC_PAIRING_CHAR_VALUE_HANDLE         (0x00E4)
#define MIUI_FC_PASSKEY_CHAR_VALUE_HANDLE         (0x00E7)
#define MIUI_FC_ACCOUNT_KEY_CHAR_VALUE_HANDLE     (0x00EA)

#define MIUI_FC_UUID_INIT_WITH_UUID16(x)                                \
   {{0xEA, 0x0B, 0x10, 0x32, 0xDE, 0x01, 0xB0, 0x8E,                    \
    0x14, 0x48, 0x66, 0x83,                                             \
                           (uint8_t)x,                                  \
                                  (uint8_t)(x >> 8),                    \
                                        0x2C, 0xFE}}

const bt_uuid_t MIUI_FC_PRODUCT_ID_CHAR_UUID128 = MIUI_FC_UUID_INIT_WITH_UUID16(MIUI_FC_PRODUCT_ID_CHAR_UUID);
const bt_uuid_t MIUI_FC_PAIRING_CHAR_UUID128 = MIUI_FC_UUID_INIT_WITH_UUID16(MIUI_FC_PAIRING_CHAR_UUID);
const bt_uuid_t MIUI_FC_PASSKEY_CHAR_UUID128 = MIUI_FC_UUID_INIT_WITH_UUID16(MIUI_FC_PASSKEY_CHAR_UUID);
const bt_uuid_t MIUI_FC_ACCOUNT_KEY_CHAR_UUID128 = MIUI_FC_UUID_INIT_WITH_UUID16(MIUI_FC_ACCOUNT_KEY_CHAR_UUID);

static uint32_t miui_fc_product_id_read_char_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);
static uint32_t miui_fc_pairing_write_char_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);
static uint32_t miui_fc_passkey_write_char_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);
static uint32_t miui_fc_account_key_write_char_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);
static uint32_t miui_fc_pairing_char_cccd_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);
static uint32_t miui_fc_passkey_char_cccd_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);
static uint32_t miui_fc_account_key_char_cccd_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);

BT_GATTS_NEW_PRIMARY_SERVICE_16(miui_fc_primary_service, MIUI_FC_BLE_SRV_UUID);

BT_GATTS_NEW_CHARC_128(miui_fc_product_id_char,
                       BT_GATT_CHARC_PROP_READ,
                       MIUI_FC_PRODUCT_ID_CHAR_VALUE_HANDLE, MIUI_FC_UUID_INIT_WITH_UUID16(MIUI_FC_PRODUCT_ID_CHAR_UUID));

BT_GATTS_NEW_CHARC_VALUE_CALLBACK(miui_fc_product_id_char_value, MIUI_FC_PRODUCT_ID_CHAR_UUID128,
                                  BT_GATTS_REC_PERM_READABLE, miui_fc_product_id_read_char_callback);

BT_GATTS_NEW_CHARC_128(miui_fc_pairing_char,
                       BT_GATT_CHARC_PROP_WRITE | BT_GATT_CHARC_PROP_NOTIFY,
                       MIUI_FC_PAIRING_CHAR_VALUE_HANDLE, MIUI_FC_UUID_INIT_WITH_UUID16(MIUI_FC_PAIRING_CHAR_UUID));

BT_GATTS_NEW_CHARC_VALUE_CALLBACK(miui_fc_pairing_char_value, MIUI_FC_PAIRING_CHAR_UUID128,
                                  BT_GATTS_REC_PERM_WRITABLE, miui_fc_pairing_write_char_callback);

BT_GATTS_NEW_CHARC_128(miui_fc_passkey_char,
                       BT_GATT_CHARC_PROP_WRITE | BT_GATT_CHARC_PROP_NOTIFY,
                       MIUI_FC_PASSKEY_CHAR_VALUE_HANDLE, MIUI_FC_UUID_INIT_WITH_UUID16(MIUI_FC_PASSKEY_CHAR_UUID));

BT_GATTS_NEW_CHARC_VALUE_CALLBACK(miui_fc_passkey_char_value, MIUI_FC_PASSKEY_CHAR_UUID128,
                                  BT_GATTS_REC_PERM_WRITABLE, miui_fc_passkey_write_char_callback);

BT_GATTS_NEW_CHARC_128(miui_fc_account_key_char,
                       BT_GATT_CHARC_PROP_WRITE | BT_GATT_CHARC_PROP_NOTIFY, MIUI_FC_ACCOUNT_KEY_CHAR_VALUE_HANDLE, MIUI_FC_UUID_INIT_WITH_UUID16(MIUI_FC_ACCOUNT_KEY_CHAR_UUID));

BT_GATTS_NEW_CHARC_VALUE_CALLBACK(miui_fc_account_key_char_value, MIUI_FC_ACCOUNT_KEY_CHAR_UUID128,
                                  BT_GATTS_REC_PERM_WRITABLE, miui_fc_account_key_write_char_callback);

BT_GATTS_NEW_CLIENT_CHARC_CONFIG(miui_fc_pairing_read_char_cccd,
                                 BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE,
                                 miui_fc_pairing_char_cccd_callback);

BT_GATTS_NEW_CLIENT_CHARC_CONFIG(miui_fc_passkey_read_char_cccd,
                                 BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE,
                                 miui_fc_passkey_char_cccd_callback);

BT_GATTS_NEW_CLIENT_CHARC_CONFIG(miui_fc_account_key_read_char_cccd,
                                 BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE,
                                 miui_fc_account_key_char_cccd_callback);

static const bt_gatts_service_rec_t *miui_fc_service_rec[] = {
    (const bt_gatts_service_rec_t *) &miui_fc_primary_service,
    (const bt_gatts_service_rec_t *) &miui_fc_product_id_char,
    (const bt_gatts_service_rec_t *) &miui_fc_product_id_char_value,
    (const bt_gatts_service_rec_t *) &miui_fc_pairing_char,
    (const bt_gatts_service_rec_t *) &miui_fc_pairing_char_value,
    (const bt_gatts_service_rec_t *) &miui_fc_pairing_read_char_cccd,
    (const bt_gatts_service_rec_t *) &miui_fc_passkey_char,
    (const bt_gatts_service_rec_t *) &miui_fc_passkey_char_value,
    (const bt_gatts_service_rec_t *) &miui_fc_passkey_read_char_cccd,
    (const bt_gatts_service_rec_t *) &miui_fc_account_key_char,
    (const bt_gatts_service_rec_t *) &miui_fc_account_key_char_value,
    (const bt_gatts_service_rec_t *) &miui_fc_account_key_read_char_cccd,
};

const bt_gatts_service_t miui_fast_connect_service = {
    .starting_handle = 0x00E0,
    .ending_handle = 0x00EB,
    .required_encryption_key_size = 0,
    .records = miui_fc_service_rec
};



/*------------------------------------------------------------*/
/*                    Define & Context                        */
/*------------------------------------------------------------*/

#define MIUI_FC_ECDH_PUBLIC_KEY_LEN             64
#define MIUI_FC_ECDH_PRIVATE_KEY_LEN            32
#define MIUI_FC_ECDH_SHARED_KEY_LEN             32

#define MIUI_FC_PRODUCT_ID_LEN                  3

const uint8_t miui_fc_default_private_key[MIUI_FC_ECDH_PRIVATE_KEY_LEN] = APP_MIUI_FC_PRIVATE_KEY;
const uint8_t miui_fc_default_product_id[MIUI_FC_PRODUCT_ID_LEN] = APP_MIUI_FC_PRODUCT_ID;

#define MIUI_FC_ACCOUNT_KEY_NEED_ADV_LEN        5
#define MIUI_FC_ACCOUNT_KEY_HEADER_BYTE         0x04

#define MIUI_FC_PASSKEY_REQUEST_TIMER           (10 * 1000)

#define MIUI_FC_PASSKEY_RESERVED_LEN            12

typedef enum {
    MIUI_FC_MESSAGE_TYPE_PAIRING_REQ = 0,
    MIUI_FC_MESSAGE_TYPE_PAIRING_RSP,
    MIUI_FC_MESSAGE_TYPE_PEER_PASSKEY,
    MIUI_FC_MESSAGE_TYPE_LOCAL_PASSKEY,
    MIUI_FC_MESSAGE_TYPE_ACCOUNT_KEY
} miui_fc_message_type;

typedef enum {
    MIUI_FC_PAIRING_FROM_PEER = 0,
    MIUI_FC_PAIRING_FROM_LOCAL,
    MIUI_FC_PAIRING_RE_WRITE_ACCOUNT_KEY
} miui_fc_pairing_initiate_t;

typedef struct {
    uint8_t         message_type;
    uint8_t         initiate_role;
    uint8_t         local_ble_addr[6];
    uint8_t         peer_edr_addr[6];
    uint8_t         random_data[2];
} PACKED miui_fc_pairing_request_info_t;

typedef struct {
    uint8_t         encrypted_info[16];     // miui_fc_pairing_request_info_t
    uint8_t         public_key[MIUI_FC_ECDH_PUBLIC_KEY_LEN];
} PACKED miui_fc_pairing_request_t;

typedef struct {
    uint8_t         message_type;
    uint8_t         local_edr_addr[6];
    uint8_t         random_data[9];
} PACKED miui_fc_pairing_response_t;

typedef struct {
    uint8_t         message_type;
    uint32_t        passkey: 24;
    uint8_t         reserved[MIUI_FC_PASSKEY_RESERVED_LEN];
} PACKED miui_fc_passkey_request_t;

typedef struct {
    uint8_t         message_type;
    uint8_t         account_key[MIUI_FC_ACCOUNT_KEY_LEN - 1];
} PACKED miui_fc_account_key_request_t;

typedef struct {
    uint8_t         local_private_key[MIUI_FC_ECDH_PRIVATE_KEY_LEN];    // fixed value
    uint8_t         product_id[MIUI_FC_PRODUCT_ID_LEN];                 // fixed value
    uint8_t         account_key[MIUI_FC_ACCOUNT_KEY_LEN];               // saved in nvkey
    uint8_t         random_account_key[MIUI_FC_ACCOUNT_KEY_NEED_ADV_LEN];    // random in bootup and use
    uint8_t         aes_key[16];
    uint16_t        conn_handle;
    uint32_t        passkey;
    bool            discoverable_mode;
    int8_t          rssi;
    uint16_t        mtu;
} PACKED miui_fc_context_t;

miui_fc_context_t       miui_fc_context;



/*------------------------------------------------------------*/
/*                         Encryption                         */
/*------------------------------------------------------------*/

static bool miui_fc_ecdh_gen_shared_key(uint8_t *public_key, uint32_t public_key_len,
                                        uint8_t *private_key, uint32_t private_key_len,
                                        uint8_t *shared_key, uint32_t *shared_key_len)
{
    bool success = FALSE;
    // ECDH - SECP256R1
    if (public_key == NULL || public_key_len != MIUI_FC_ECDH_PUBLIC_KEY_LEN
        || private_key == NULL || private_key_len != MIUI_FC_ECDH_PRIVATE_KEY_LEN
        || shared_key == NULL || shared_key_len == NULL || *shared_key_len < MIUI_FC_ECDH_SHARED_KEY_LEN) {
        APPS_LOG_MSGID_E(LOG_TAG" gen_shared_key, invalid parameter", 0);
        return success;
    }

#if 1
    // Use uECC API
    bt_os_layer_ecdh256(shared_key, public_key, private_key);
    *shared_key_len = MIUI_FC_ECDH_SHARED_KEY_LEN;
    success = TRUE;
#else
    // Use mbedtls API
    int ret = 0;
    mbedtls_ecp_group grp;
    mbedtls_mpi local_private_key;
    mbedtls_ecp_point peer_public_key_point;
    mbedtls_mpi shared_key_mpi;

    mbedtls_ecp_group_init(&grp);
    ret = mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_SECP256R1);
    if (ret != 0) {
        APPS_LOG_MSGID_E(LOG_TAG" gen_shared_key, load EC error %d", 1, ret);
        goto exit;
    }

    mbedtls_mpi_init(&local_private_key);
    ret = mbedtls_mpi_read_binary(&local_private_key, private_key, sizeof(private_key));
    if (ret != 0) {
        APPS_LOG_MSGID_E(LOG_TAG" gen_shared_key, load private_key error %d", 1, ret);
        goto exit;
    }

    // workaround for mbedtls mbedtls_ecp_point_read_binary
    uint8_t public_key1[MIUI_FC_ECDH_PUBLIC_KEY_LEN + 1] = {0};
    public_key1[0] = 0x04;
    memcpy(public_key1, public_key, MIUI_FC_ECDH_PUBLIC_KEY_LEN);
    mbedtls_ecp_point_init(&peer_public_key_point);
    ret = mbedtls_ecp_point_read_binary(&grp, &peer_public_key_point, public_key1, sizeof(public_key1));
    if (ret != 0) {
        APPS_LOG_MSGID_E(LOG_TAG" gen_shared_key, load public_key error %d", 1, ret);
        goto exit;
    }

    mbedtls_mpi_init(&shared_key_mpi);
    ret = mbedtls_ecdh_compute_shared(&grp, &shared_key_mpi,
                                      &peer_public_key_point, &local_private_key,
                                      NULL, NULL);
    if (ret != 0) {
        APPS_LOG_MSGID_E(LOG_TAG" gen_shared_key, compute shared error %d", 1, ret);
        goto exit;
    }

    ret = mbedtls_mpi_write_binary(&shared_key_mpi, shared_key, MIUI_FC_ECDH_SHARED_KEY_LEN);
    *shared_key_len = MIUI_FC_ECDH_SHARED_KEY_LEN;
    success = (ret == 0);

exit:
    mbedtls_ecp_group_free(&grp);
    mbedtls_ecp_point_free(&peer_public_key_point);
    mbedtls_mpi_free(&local_private_key);
    mbedtls_mpi_free(&shared_key_mpi);
#endif

    return success;
}

static bool miui_fc_gen_aes_key(uint8_t *public_key, uint32_t public_key_len)
{
    bool success = FALSE;
    uint8_t shared_key[MIUI_FC_ECDH_SHARED_KEY_LEN] = {0};
    uint32_t shared_key_len = 0;
    success = miui_fc_ecdh_gen_shared_key(public_key, public_key_len,
                                          miui_fc_context.local_private_key,
                                          MIUI_FC_ECDH_PRIVATE_KEY_LEN,
                                          shared_key, &shared_key_len);
    if (!success || shared_key_len != MIUI_FC_ECDH_SHARED_KEY_LEN) {
        APPS_LOG_MSGID_E(LOG_TAG" gen_aes_key, ecdh_gen_shared_key error", 0);
        goto exit;
    }

    uint8_t sha256_out[32] = {0};
    bool ret = mbedtls_sha256_ret(shared_key, shared_key_len, sha256_out, FALSE);
    if (ret != 0) {
        success = FALSE;
        APPS_LOG_MSGID_E(LOG_TAG" gen_aes_key, share256 error", 0);
        goto exit;
    }

    memcpy(miui_fc_context.aes_key, sha256_out, 16);
    success = TRUE;

exit:
    return success;
}

static void miui_fc_aes_encrypt(uint8_t input[16], uint8_t output[16])
{
    mbedtls_aes_context ctx;
    mbedtls_aes_init(&ctx);
    mbedtls_aes_setkey_dec(&ctx, miui_fc_context.aes_key, sizeof(miui_fc_context.aes_key) * 8);
    mbedtls_aes_crypt_ecb(&ctx, MBEDTLS_AES_ENCRYPT, input, output);
    mbedtls_aes_free(&ctx);
}

static void miui_fc_aes_decrypt(uint8_t input[16], uint8_t output[16])
{
    mbedtls_aes_context ctx;
    mbedtls_aes_init(&ctx);
    mbedtls_aes_setkey_dec(&ctx, miui_fc_context.aes_key, sizeof(miui_fc_context.aes_key) * 8);
    mbedtls_aes_crypt_ecb(&ctx, MBEDTLS_AES_DECRYPT, input, output);
    mbedtls_aes_free(&ctx);
}



/*------------------------------------------------------------*/
/*                         BLE Callback                       */
/*------------------------------------------------------------*/

typedef enum {
    MIUI_FC_BLE_CHAR_TYPE_PAIRING = 0,
    MIUI_FC_BLE_CHAR_TYPE_PASSKEY,
    MIUI_FC_BLE_CHAR_TYPE_ACCOUNT_KEY
} miui_fc_ble_char_t;

static bool miui_fc_ble_send_data_imp(miui_fc_ble_char_t char_type, uint8_t *data, uint32_t len)
{
    uint8_t *notify_buf = (uint8_t *)pvPortMalloc(5 + len);
    if (notify_buf == NULL) {
        APPS_LOG_MSGID_E(LOG_TAG" ble_send_data, malloc fail", 0);
        return FALSE;
    }

    bool success = FALSE;
    uint16_t handle = 0;
    if (char_type == MIUI_FC_BLE_CHAR_TYPE_PAIRING) {
        handle = MIUI_FC_PAIRING_CHAR_VALUE_HANDLE;
    } else if (char_type == MIUI_FC_BLE_CHAR_TYPE_PASSKEY) {
        handle = MIUI_FC_PASSKEY_CHAR_VALUE_HANDLE;
    } else if (char_type == MIUI_FC_BLE_CHAR_TYPE_ACCOUNT_KEY) {
        handle = MIUI_FC_ACCOUNT_KEY_CHAR_VALUE_HANDLE;
    }

    bt_gattc_charc_value_notification_indication_t *notify_ind = (bt_gattc_charc_value_notification_indication_t *)notify_buf;
    uint8_t max_retry_count = 0;
    while (max_retry_count < 5) {
        notify_ind->attribute_value_length = 3 + len;
        memcpy(notify_ind->att_req.attribute_value, data, len);
        notify_ind->att_req.handle = handle;
        notify_ind->att_req.opcode = BT_ATT_OPCODE_HANDLE_VALUE_NOTIFICATION;

        bt_status_t bt_status = bt_gatts_send_charc_value_notification_indication(miui_fc_context.conn_handle,
                                                                                  notify_ind);
        if (bt_status == BT_STATUS_SUCCESS) {
            success = TRUE;
            break;
        }
        if (bt_status == BT_STATUS_OUT_OF_MEMORY) {
            APPS_LOG_MSGID_E(LOG_TAG" ble_send_data, OOM retry=%d", 1, max_retry_count);
            max_retry_count++;
            success = FALSE;
        } else {
            APPS_LOG_MSGID_E(LOG_TAG" ble_send_data, fail bt_status=0x%08X", 1, bt_status);
            success = FALSE;
            break;
        }
        vTaskDelay(20);
    }

    if (notify_buf != NULL) {
        vPortFree(notify_buf);
    }
    return success;
}

static bool miui_fc_ble_send_data(miui_fc_ble_char_t char_type, uint8_t *data, uint32_t len)
{
    bool ret = FALSE;
    uint32_t sent_len = 0;
    if (char_type != MIUI_FC_BLE_CHAR_TYPE_PAIRING
        && char_type != MIUI_FC_BLE_CHAR_TYPE_PASSKEY
        && char_type != MIUI_FC_BLE_CHAR_TYPE_ACCOUNT_KEY) {
        APPS_LOG_MSGID_E(LOG_TAG" ble_send_data, error char_type", 0);
        goto exit;
    } else if (miui_fc_context.conn_handle == 0) {
        APPS_LOG_MSGID_E(LOG_TAG" ble_send_data, error handle", 0);
        goto exit;
    } else if (data == NULL || len == 0) {
        APPS_LOG_MSGID_E(LOG_TAG" ble_send_data, error parameter", 0);
        goto exit;
    }

    if (len > miui_fc_context.mtu) {
        uint32_t packet_len = MIUI_FC_MIN((len - sent_len), miui_fc_context.mtu);
        while (sent_len < len) {
            ret = miui_fc_ble_send_data_imp(char_type, data + sent_len, packet_len);
            if (ret) {
                sent_len += packet_len;
                packet_len = MIUI_FC_MIN((len - sent_len), miui_fc_context.mtu);
            } else {
                break;
            }
        }
    } else {
        ret = miui_fc_ble_send_data_imp(char_type, data, len);
    }

exit:
    APPS_LOG_MSGID_I(LOG_TAG" ble_send_data, type=%d len=%d(%d) ret=%d",
                     4, char_type, len, sent_len, ret);
    return ret;
}

static uint32_t miui_fc_product_id_read_char_callback(const uint8_t rw, uint16_t handle,
                                                      void *data, uint16_t size,
                                                      uint16_t offset)
{
    if (rw == BT_GATTS_CALLBACK_READ && data != NULL) {
        memcpy(data, miui_fc_context.product_id, MIUI_FC_PRODUCT_ID_LEN);
        APPS_LOG_MSGID_E(LOG_TAG" product_id_read_char_callback, product_id=0x%08X",
                         1, miui_fc_context.product_id);
        return MIUI_FC_PRODUCT_ID_LEN;
    }
    return 0;
}

static uint32_t miui_fc_pairing_write_char_callback(const uint8_t rw, uint16_t handle,
                                                    void *data, uint16_t size,
                                                    uint16_t offset)
{
    if (handle == BT_HANDLE_INVALID
        || handle != miui_fc_context.conn_handle) {
        APPS_LOG_MSGID_E(LOG_TAG" pairing_write_char_callback, error handle", 0);
        return 0;
    } else if (!miui_fc_context.discoverable_mode) {
        APPS_LOG_MSGID_E(LOG_TAG" pairing_write_char_callback, not discoverable mode", 0);
        return 0;
    } else if (size != sizeof(miui_fc_pairing_request_t)) {
        APPS_LOG_MSGID_E(LOG_TAG" pairing_write_char_callback, not match size %d", 1, size);
        return 0;
    }

    APPS_LOG_MSGID_I(LOG_TAG" pairing_write_char_callback, rw=%d data=0x%08X size=%d",
                     3, rw, data, size);
    if (rw == BT_GATTS_CALLBACK_WRITE) {
        miui_fc_pairing_request_t *req = (miui_fc_pairing_request_t *)pvPortMalloc(sizeof(miui_fc_pairing_request_t));
        if (req != NULL) {
            // Update io_capability in advance because switch to UI_Shell task
            bt_device_manager_set_io_capability(BT_GAP_IO_CAPABILITY_DISPLAY_YES_NO);
            memset(req, 0, sizeof(miui_fc_pairing_request_t));
            memcpy(req, data, sizeof(miui_fc_pairing_request_t));
            ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                                EVENT_GROUP_UI_SHELL_XIAOAI,
                                XIAOAI_EVENT_MIUI_FC_PAIRING_REQUEST,
                                req, sizeof(miui_fc_pairing_request_t), NULL, 0);
        } else {
            APPS_LOG_MSGID_E(LOG_TAG" pairing_write_char_callback, malloc fail", 0);
        }
    }
    return size;
}

static uint32_t miui_fc_passkey_write_char_callback(const uint8_t rw, uint16_t handle,
                                                    void *data, uint16_t size,
                                                    uint16_t offset)
{
    if (handle == BT_HANDLE_INVALID
        || handle != miui_fc_context.conn_handle) {
        APPS_LOG_MSGID_E(LOG_TAG" passkey_write_char_callback, error handle", 0);
        return 0;
    } else if (size != sizeof(miui_fc_passkey_request_t)) {
        APPS_LOG_MSGID_E(LOG_TAG" passkey_write_char_callback, not match size %d", 1, size);
        return 0;
    }

    APPS_LOG_MSGID_I(LOG_TAG" passkey_write_char_callback, rw=%d data=0x%08X size=%d",
                     3, rw, data, size);
    if (rw == BT_GATTS_CALLBACK_WRITE) {
        miui_fc_passkey_request_t *req = (miui_fc_passkey_request_t *)pvPortMalloc(sizeof(miui_fc_passkey_request_t));
        if (req != NULL) {
            memset(req, 0, sizeof(miui_fc_passkey_request_t));
            memcpy(req, data, sizeof(miui_fc_passkey_request_t));
            ui_shell_remove_event(EVENT_GROUP_UI_SHELL_XIAOAI, XIAOAI_EVENT_MIUI_FC_PASSKEY_REQUEST_TIMER);
            ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                                EVENT_GROUP_UI_SHELL_XIAOAI,
                                XIAOAI_EVENT_MIUI_FC_PASSKEY_REQUEST,
                                req, sizeof(miui_fc_passkey_request_t), NULL, 0);
        } else {
            APPS_LOG_MSGID_E(LOG_TAG" passkey_write_char_callback, malloc fail", 0);
        }
    }
    return size;
}

static uint32_t miui_fc_account_key_write_char_callback(const uint8_t rw, uint16_t handle,
                                                        void *data, uint16_t size,
                                                        uint16_t offset)
{
    if (handle == BT_HANDLE_INVALID
        || handle != miui_fc_context.conn_handle) {
        APPS_LOG_MSGID_E(LOG_TAG" account_key_write_char_callback, error handle", 0);
        return 0;
    } else if (size != sizeof(miui_fc_account_key_request_t)) {
        APPS_LOG_MSGID_E(LOG_TAG" account_key_write_char_callback, not match size %d", 1, size);
        return 0;
    }

    APPS_LOG_MSGID_I(LOG_TAG" account_key_write_char_callback, rw=%d data=0x%08X size=%d",
                     3, rw, data, size);
    if (rw == BT_GATTS_CALLBACK_WRITE) {
        miui_fc_account_key_request_t *req = (miui_fc_account_key_request_t *)pvPortMalloc(sizeof(miui_fc_account_key_request_t));
        if (req != NULL) {
            memset(req, 0, sizeof(miui_fc_account_key_request_t));
            memcpy(req, data, sizeof(miui_fc_account_key_request_t));
            ui_shell_remove_event(EVENT_GROUP_UI_SHELL_XIAOAI, XIAOAI_EVENT_MIUI_FC_ACCOUNT_KEY_REQUEST);
            ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                                EVENT_GROUP_UI_SHELL_XIAOAI,
                                XIAOAI_EVENT_MIUI_FC_ACCOUNT_KEY_REQUEST,
                                req, sizeof(miui_fc_account_key_request_t), NULL, 0);
        } else {
            APPS_LOG_MSGID_E(LOG_TAG" account_key_write_char_callback, malloc fail", 0);
        }
    }
    return size;
}

static uint32_t miui_fc_pairing_char_cccd_callback(const uint8_t rw, uint16_t handle,
                                                   void *data, uint16_t size,
                                                   uint16_t offset)
{
    if (handle != miui_fc_context.conn_handle) {
        APPS_LOG_MSGID_E(LOG_TAG" pairing_char_cccd_callback, error handle", 0);
        return 0;
    }
    if (rw == BT_GATTS_CALLBACK_WRITE) {
        uint16_t write_value = *(uint16_t *)data;
        APPS_LOG_MSGID_I(LOG_TAG" pairing_char_cccd_callback, write cccd=%d", 1, write_value);
    }
    return sizeof(uint16_t);
}

static uint32_t miui_fc_passkey_char_cccd_callback(const uint8_t rw, uint16_t handle,
                                                   void *data, uint16_t size,
                                                   uint16_t offset)
{
    if (handle != miui_fc_context.conn_handle) {
        APPS_LOG_MSGID_E(LOG_TAG" passkey_char_cccd_callback, error handle", 0);
        return 0;
    }
    if (rw == BT_GATTS_CALLBACK_WRITE) {
        uint16_t write_value = *(uint16_t *)data;
        APPS_LOG_MSGID_I(LOG_TAG" passkey_char_cccd_callback, write cccd=%d", 1, write_value);
    }
    return sizeof(uint16_t);
}

static uint32_t miui_fc_account_key_char_cccd_callback(const uint8_t rw, uint16_t handle,
                                                       void *data, uint16_t size,
                                                       uint16_t offset)
{
    if (handle != miui_fc_context.conn_handle) {
        APPS_LOG_MSGID_E(LOG_TAG" account_key_char_cccd_callback, error handle", 0);
        return 0;
    }
    if (rw == BT_GATTS_CALLBACK_WRITE) {
        uint16_t write_value = *(uint16_t *)data;
        APPS_LOG_MSGID_I(LOG_TAG" account_key_char_cccd_callback, write cccd=%d", 1, write_value);
    }
    return sizeof(uint16_t);
}

bt_status_t miui_fc_ble_common_event_callback_handler(bt_msg_type_t msg, bt_status_t status, void *buf)
{
    switch (msg) {
        case BT_GAP_LE_CONNECT_IND: {
            bt_gap_le_connection_ind_t *conn_ind = (bt_gap_le_connection_ind_t *)buf;
            if (conn_ind == NULL) {
                return BT_STATUS_FAIL;
            }

            bt_bd_addr_t *connect_addr = (bt_bd_addr_t *)conn_ind->local_addr.addr;
            bt_bd_addr_t random_addr;
            bt_gap_le_advertising_handle_t le_handle;
            bool ret = multi_ble_adv_manager_get_random_addr_and_adv_handle(MULTI_ADV_INSTANCE_XIAOAI,
                                                                            &random_addr,
                                                                            &le_handle);
            if (ret && memcmp(connect_addr, &random_addr, sizeof(bt_bd_addr_t) != 0)) {
                APPS_LOG_MSGID_E(LOG_TAG" ble_common_callback, BT_GAP_LE_CONNECT_IND not miui_fc addr", 0);
                break;
            }

            miui_fc_context.conn_handle = conn_ind->connection_handle;
            miui_fc_context.mtu = 23;
            APPS_LOG_MSGID_I(LOG_TAG" ble_common_callback, BT_GAP_LE_CONNECT_IND conn_handle=0x%04X",
                             1, miui_fc_context.conn_handle);
            break;
        }

        case BT_GAP_LE_DISCONNECT_IND: {
            APPS_LOG_MSGID_I(LOG_TAG" ble_common_callback, BT_GAP_LE_DISCONNECT_CNF", 0);
            bt_hci_evt_disconnect_complete_t *disc_ind = (bt_hci_evt_disconnect_complete_t *)buf;
            if (disc_ind == NULL) {
                return BT_STATUS_FAIL;
            }
            if (disc_ind->connection_handle == miui_fc_context.conn_handle) {
                // reset BT IO Capabillity.
                bt_device_manager_set_io_capability(0xFF);
                miui_fc_context.conn_handle = 0;
                miui_fc_context.mtu = 0;
            }
            break;
        }

        case BT_GAP_READ_RSSI_CNF: {
            bt_gap_read_rssi_cnf_t *rssi_cnf = (bt_gap_read_rssi_cnf_t *)buf;
            APPS_LOG_MSGID_I(LOG_TAG" ble_common_callback, BT_GAP_READ_RSSI_CNF rssi=%d(0x%02X)",
                             2, rssi_cnf->rssi, rssi_cnf->rssi);
            miui_fc_context.rssi = rssi_cnf->rssi;
            break;
        }
        case BT_GAP_USER_CONFIRM_REQ_IND: {
            uint32_t passkey = *(uint32_t *)buf;
            APPS_LOG_MSGID_I(LOG_TAG" ble_common_callback, BT_GAP_USER_CONFIRM_REQ_IND passkey=0x%08X",
                             1, passkey);
            miui_fc_context.passkey = passkey;
            break;
        }
        case BT_GATTS_EXCHANGE_MTU_INDICATION: {
            bt_gatts_exchange_mtu_ind_t *mtu_ind = (bt_gatts_exchange_mtu_ind_t *)buf;
            APPS_LOG_MSGID_I(LOG_TAG" ble_common_callback, EXCHANGE_MTU_INDICATION mtu=%d",
                             1, (mtu_ind->server_mtu - 3));
            miui_fc_context.mtu = (uint16_t)(mtu_ind->server_mtu - 3);
            break;
        }
        case BT_POWER_OFF_CNF: {
            APPS_LOG_MSGID_I(LOG_TAG" ble_common_callback, BT_POWER_OFF_CNF", 0);
            miui_fc_context.conn_handle = 0;
            break;
        }
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}



/*------------------------------------------------------------*/
/*                         Fast Connect                       */
/*------------------------------------------------------------*/
static bool miui_fc_is_account_key_pairing()
{
    bool ret = (miui_fc_context.account_key[0] == MIUI_FC_ACCOUNT_KEY_HEADER_BYTE);
    APPS_LOG_MSGID_I(LOG_TAG" is_account_key_pairing, ret=%d", 1, ret);
    return ret;
}

static bool miui_fc_is_edr_connected()
{
    int device_num = bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK), NULL, 0);
    return (device_num > 0);
}

static uint8_t miui_fc_ble_adv_get_flag_byte()
{
    uint8_t flag = 0;
    bool is_edr_connected = miui_fc_is_edr_connected();
    bt_aws_mce_srv_link_type_t aws_link_type = bt_aws_mce_srv_get_link_type();
    uint32_t pair_num = bt_device_manager_get_paired_number();
    audio_channel_t channel = ami_get_audio_channel();
    bool case_close = xiaoai_one_case_close();
    bool both_out_of_case = xiaoai_both_out_of_case();

    // bit7 - always connectable, headset could be take_over by MIUI_FAST_CONNECT
    flag |= (1 << 7);
    // bit6 - scannable
    if (is_edr_connected && !miui_fc_context.discoverable_mode) {
        // not scannable
    } else {
        flag |= (1 << 6);
    }
    // bit5 - AWS Connection state
    if (aws_link_type != BT_AWS_MCE_SRV_LINK_NONE) {
        flag |= (1 << 5);
    }
    // bit4 - BT pairing flag
    if (pair_num > 0) {
        flag |= (1 << 4);
    }
    // bit3 - BT EDR connection flag
    if (is_edr_connected) {
        flag |= (1 << 3);
    }
    // bit2 - headset both out_of_case flag
    if (both_out_of_case) {
        flag |= (1 << 2);
    }
    // bit1 - Charger case open/close flag
    if (!case_close) {
        flag |= (1 << 1);
    }
    // bit0 - Left/Right
    if (channel == AUDIO_CHANNEL_L) {
        flag |= 1;
    }
    APPS_LOG_MSGID_I(LOG_TAG" BLE_ADV_DATA flag, is_edr_connected=%d aws_link_type=%d pair_num=%d channel=%d case_close=%d both_out_of_case=%d flag=%d",
                     7, is_edr_connected, aws_link_type, pair_num, channel, case_close, both_out_of_case, flag);
    return flag;
}

static void miui_fc_update_ble_adv_data(uint8_t *adv_data)
{
    // AD Type - Flags
    uint8_t byte0_ad_flags_length = 0x02;             // Byte 0, fixed
    uint8_t byte1_ad_flags_type = 0x01;               // Byte 1, fixed
    uint8_t byte2_ad_flags_value = 0x06;              // Byte 2, LE General Discoverable Mode + BR/EDR Not Supported
    // AD Type
    uint8_t byte3_ad_data_length = 0x12;              // Byte 3, fixed 0x12 - 18bytes
    uint8_t byte4_ad_type = 0x16;                     // Byte 4, fixed 0x16 - service data
    uint8_t byte5_ad_srv_uuid1 = (MIUI_FC_BLE_SRV_UUID & 0xFF);         // Byte 5, LSB
    uint8_t byte6_ad_srv_uuid2 = ((MIUI_FC_BLE_SRV_UUID >> 8) & 0xFF);  // Byte 6, MSB

    uint8_t byte7_ad_pairing_type = 1;                // Byte 7, fixed
    uint8_t byte8_ad_major_id = miui_fc_default_product_id[0];      // Byte 8, major ID
    uint8_t byte9_ad_minor_id1 = miui_fc_default_product_id[1];     // Byte 9, minor ID
    uint8_t byte10_ad_minor_id2 = miui_fc_default_product_id[2];    // Byte 10, minor ID

    uint8_t byte11_flag1 = miui_fc_ble_adv_get_flag_byte();         // Byte 11

    uint8_t byte12_flag2 = 0x04;                                    // Byte 12, bit1 - XiaoAI SPP/BLE Connection, bit2 - support VID/PID
    xiaoai_connection_state xiaoai_state = xiaoai_get_connection_state();
    if (xiaoai_state == XIAOAI_STATE_CONNECTED) {
        byte12_flag2 += 0x02;
    }

    // RSSI & battery & broadcast_count
#if 0
    uint8_t byte18_rssi = (uint8_t)miui_fc_context.rssi;
#else
    uint8_t byte18_rssi = 0;
#endif
    uint8_t byte19_right_battery = 0;
    uint8_t byte20_left_battery = 0;
    uint8_t byte21_case_battery = 0;
    xiaoai_get_all_battery_charging_info(&byte20_left_battery, &byte19_right_battery, &byte21_case_battery);
    uint8_t byte22_broadcast_count = xiaoai_get_broadcast_count();  // Byte22, broadcast count

    // Update adv_data
    memset(adv_data, 0, 31);
    adv_data[0] = byte0_ad_flags_length;
    adv_data[1] = byte1_ad_flags_type;
    adv_data[2] = byte2_ad_flags_value;
    adv_data[3] = byte3_ad_data_length;
    adv_data[4] = byte4_ad_type;
    adv_data[5] = byte5_ad_srv_uuid1;
    adv_data[6] = byte6_ad_srv_uuid2;
    adv_data[7] = byte7_ad_pairing_type;
    adv_data[8] = byte8_ad_major_id;
    adv_data[9] = byte9_ad_minor_id1;
    adv_data[10] = byte10_ad_minor_id2;
    adv_data[11] = byte11_flag1;
    adv_data[12] = byte12_flag2;
    // Account Key
    if (miui_fc_is_account_key_pairing()) {
        memcpy(&adv_data[13], miui_fc_context.account_key, MIUI_FC_ACCOUNT_KEY_NEED_ADV_LEN);
    } else {
        memcpy(&adv_data[13], miui_fc_context.random_account_key, MIUI_FC_ACCOUNT_KEY_NEED_ADV_LEN);
    }
    adv_data[18] = byte18_rssi;
    adv_data[19] = byte19_right_battery;
    adv_data[20] = byte20_left_battery;
    adv_data[21] = byte21_case_battery;
    adv_data[22] = byte22_broadcast_count;
    if ((byte12_flag2 & 0x04) > 0) {
        adv_data[23] = (APP_VA_XIAOAI_PID & 0xFF);
        adv_data[24] = ((APP_VA_XIAOAI_PID >> 8) & 0xFF);
        adv_data[25] = (APP_VA_XIAOAI_VID & 0xFF);
        adv_data[26] = ((APP_VA_XIAOAI_VID >> 8) & 0xFF);
    }

    // BLE ADV Log
    uint8_t *adv = adv_data;
    APPS_LOG_MSGID_I(LOG_TAG" ble_adv_data, flag=%02X:%02X:%02X type=%02X:%02X:%02X:%02X:%02X",
                     8, adv[0], adv[1], adv[2], adv[3], adv[4], adv[5], adv[6], adv[7]);
    APPS_LOG_MSGID_I(LOG_TAG" ble_adv_data, major_id=%02X:%02X:%02X", 3, adv[8], adv[9], adv[10]);
    APPS_LOG_MSGID_I(LOG_TAG" ble_adv_data, flag1=%02X flag2=%02X account_key=%02X:%02X:%02X:%02X:%02X",
                     7, adv[11], adv[12], adv[13], adv[14], adv[15], adv[16], adv[17]);
    APPS_LOG_MSGID_I(LOG_TAG" ble_adv_data, RSSI=%02X right=%02X left=%02X case=%02X count=%02X",
                     5, adv[18], adv[19], adv[20], adv[21], adv[22]);
}

static void miui_fc_update_ble_scan_rsp(uint8_t *scan_rsp)
{
#if 0
    // AD Type - Manufacturer Data(md)
    uint8_t byte0_ad_md_length = 0x0D;                // Byte 0, fixed, 0D=13 + 1 + 12
    uint8_t byte1_ad_md_type = 0xFF;                  // Byte 1, fixed
    unsigned short vid = APP_VA_XIAOAI_VID;
    uint8_t byte2_sig_vid0 = ((vid & 0xFF00) >> 8);   // Byte 2~3, SIG VID?
    uint8_t byte3_sig_vid1 = (vid & 0x00FF);
    uint8_t byte4_data_length = 0x08;                 // Byte 4, fixed
    uint8_t byte5_data_type = 0x03;                   // Byte 5, fixed
    uint8_t byte6_spec_ver = 0x02;                    // Byte 6, fixed
    unsigned short pid = APP_VA_XIAOAI_PID;
    uint8_t byte7_pid0 = ((pid & 0xFF00) >> 8);       // Byte 7~8, PID
    uint8_t byte8_pid1 = (pid & 0x00FF);

    // Byte 9~12
    uint8_t *vaddr = NULL;
    xiaoai_conn_info_t conn_info = xiaoai_get_connection_info();
    uint8_t empty_virtual_addr[4] = {0};
    if ((bt_device_manager_get_paired_number() > 0)
        && memcmp(conn_info.virtual_addr, empty_virtual_addr, 4) != 0) {
        vaddr = (uint8_t *)conn_info.virtual_addr;
    } else {
        bt_bd_addr_t *bt_bd_addr = bt_device_manager_get_local_address();
        vaddr = (uint8_t *)(*bt_bd_addr);
    }
    scan_rsp[9] = vaddr[0];
    scan_rsp[10] = vaddr[1];
    scan_rsp[11] = vaddr[2];
    scan_rsp[12] = vaddr[3];

    // Byte 13, XiaoAI SPP/BLE Connection Status, color feature
    if (conn_info.conn_state == XIAOAI_STATE_CONNECTED) {
        scan_rsp[13] = 0x80;   // bit7 = 1
    } else {
        scan_rsp[13] = 0;
    }
    unsigned char color_type = APP_VA_XIAOAI_DEVICE_TYPE;
    scan_rsp[13] = (scan_rsp[13] | ((color_type & 0x0F) << 3));

    // Byte 14~30, 0, reserved
    for (int i = 14; i < 31; i++) {
        scan_rsp[i] = 0;
    }

    // Update other g_xiaoai_ble_rsp_data
    scan_rsp[0] = byte0_ad_md_length;
    scan_rsp[1] = byte1_ad_md_type;
    scan_rsp[2] = byte2_sig_vid0;
    scan_rsp[3] = byte3_sig_vid1;
    scan_rsp[4] = byte4_data_length;
    scan_rsp[5] = byte5_data_type;
    scan_rsp[6] = byte6_spec_ver;
    scan_rsp[7] = byte7_pid0;
    scan_rsp[8] = byte8_pid1;

    // BLE ADV SCAN RSP Log
    APPS_LOG_MSGID_I(LOG_TAG" ble_scan_rsp, VID=%02X%02X PID=%02X%02X vaddr=%02X:%02X:%02X:XX device_type=%02X",
                     8, scan_rsp[2], scan_rsp[3], scan_rsp[7], scan_rsp[8],
                     scan_rsp[9], scan_rsp[10], scan_rsp[11], scan_rsp[13]);
#endif
}

static void miui_fc_interupt_pairing()
{
    bt_device_manager_set_io_capability(0xFF);
    memset(miui_fc_context.aes_key, 0, 16);
    miui_fc_context.passkey = 0;

    APPS_LOG_MSGID_I(LOG_TAG" miui_fc_interupt_pairing, conn_handle=0x%04X", 1, miui_fc_context.conn_handle);

    if (miui_fc_context.conn_handle != BT_HANDLE_INVALID) {
        bt_hci_cmd_disconnect_t param = {0};
        param.connection_handle = miui_fc_context.conn_handle;
        param.reason = BT_HCI_STATUS_REMOTE_USER_TERMINATED_CONNECTION;
        bt_gap_le_disconnect(&param);
    }
}

static void miui_fc_handle_account_key(uint8_t *key, bool is_sync)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    if (miui_fc_is_account_key_pairing()) {
        APPS_LOG_MSGID_E(LOG_TAG" handle_account_key, [%02X] exist account_key", 1, role);
    } else {
        memcpy(miui_fc_context.account_key, key, MIUI_FC_ACCOUNT_KEY_LEN);
        nvkey_status_t status = nvkey_write_data(NVID_APP_XIAOAI_FAST_CONNECT,
                                                 (const uint8_t *)key, MIUI_FC_ACCOUNT_KEY_LEN);
        APPS_LOG_MSGID_I(LOG_TAG" handle_account_key, [%02X] status=%d %02X:%02X:%02X:%02X:%02X",
                         7, role, status, key[0], key[1], key[2], key[3], key[4]);
    }

    bool ret = miui_fc_ble_send_data(MIUI_FC_BLE_CHAR_TYPE_ACCOUNT_KEY,
                                     key, MIUI_FC_ACCOUNT_KEY_LEN);
    APPS_LOG_MSGID_I(LOG_TAG" handle_account_key, reply account_key to peer ret=%d",
                     1, ret);

    if (is_sync) {
#ifdef MTK_AWS_MCE_ENABLE
        bt_status_t bt_status = apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_XIAOAI,
                                                               XIAOAI_EVENT_MIUI_FC_SYNC_ACCOUNT_KEY,
                                                               (void *)key, MIUI_FC_ACCOUNT_KEY_LEN);
        APPS_LOG_MSGID_I(LOG_TAG" handle_account_key, sync bt_status=0x%08X",
                         1, bt_status);
#endif
    }
}

static void miui_fc_bt_cm_event_proc(uint32_t event_id, void *extra_data, size_t data_len)
{
    switch (event_id) {
        case BT_CM_EVENT_REMOTE_INFO_UPDATE: {
            bt_cm_remote_info_update_ind_t *remote_update = (bt_cm_remote_info_update_ind_t *)extra_data;
            if (remote_update != NULL) {
#ifdef MTK_AWS_MCE_ENABLE
                bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
                if (!(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service)
                    && (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service)) {
                    bool account_key_pairing = miui_fc_is_account_key_pairing();
                    APPS_LOG_MSGID_I(LOG_TAG" [%02X] AWS Attached, account_key_pairing=%d",
                                     2, role, account_key_pairing);
                    if (role == BT_AWS_MCE_ROLE_AGENT && account_key_pairing) {
                        apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_XIAOAI,
                                                       XIAOAI_EVENT_MIUI_FC_SYNC_ACCOUNT_KEY,
                                                       (void *)miui_fc_context.account_key,
                                                       MIUI_FC_ACCOUNT_KEY_LEN);
                    }
                } else if ((BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service)
                           && !(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service)) {
                    APPS_LOG_MSGID_I(LOG_TAG" [%02X] AWS Detached", 1, role);
                }
#endif
            }
            break;
        }

        case BT_CM_EVENT_VISIBILITY_STATE_UPDATE: {
            bt_cm_visibility_state_update_ind_t *visible_ind = (bt_cm_visibility_state_update_ind_t *)extra_data;
            if (visible_ind != NULL) {
                APPS_LOG_MSGID_I(LOG_TAG" EDR discoverable_mode %d", 1, visible_ind->visibility_state);
                miui_fc_context.discoverable_mode = visible_ind->visibility_state;
                app_va_xiaoai_miui_fc_start_adv();
            }
            break;
        }
    }
}

static void miui_fc_bt_aws_event_data_proc(uint32_t event_id, void *extra_data, size_t data_len)
{
#ifdef MTK_AWS_MCE_ENABLE
    bt_aws_mce_report_info_t *aws_data_ind = (bt_aws_mce_report_info_t *)extra_data;
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    if (aws_data_ind->module_id == BT_AWS_MCE_REPORT_MODULE_APP_ACTION) {
        uint32_t event_group = 0;
        uint32_t event_id = 0;
        apps_aws_sync_event_decode(aws_data_ind, &event_group, &event_id);

        if (role == BT_AWS_MCE_ROLE_PARTNER && event_group == EVENT_GROUP_UI_SHELL_XIAOAI) {
            APPS_LOG_MSGID_I(LOG_TAG" AWS_DATA event, partner recv XiaoAI event %d",
                             1, event_id);
            if (event_id == XIAOAI_EVENT_MIUI_FC_SYNC_ACCOUNT_KEY) {
                uint8_t *key = (uint8_t *)extra_data;
                miui_fc_handle_account_key(key, FALSE);
            }
        }

    }
#endif
}

static void miui_fc_xiaoai_event_proc(uint32_t event_id, void *extra_data, size_t data_len)
{
    switch (event_id) {
        case XIAOAI_EVENT_MIUI_FC_PAIRING_REQUEST: {
            APPS_LOG_MSGID_I(LOG_TAG" PAIRING_REQUEST event, data_len=%d", 1, data_len);
            if (extra_data == NULL || data_len != sizeof(miui_fc_pairing_request_t)) {
                APPS_LOG_MSGID_E(LOG_TAG" PAIRING_REQUEST event, error extra_data/len", 0);
                miui_fc_interupt_pairing();
                break;
            }
            miui_fc_pairing_request_t *req = (miui_fc_pairing_request_t *)extra_data;
            bool ret = miui_fc_gen_aes_key(req->public_key, MIUI_FC_ECDH_PUBLIC_KEY_LEN);
            if (ret) {
                APPS_LOG_MSGID_I(LOG_TAG" PAIRING_REQUEST event, get_aes_key pass", 0);
                uint8_t output[16] = {0};
                miui_fc_aes_decrypt(req->encrypted_info, output);
                miui_fc_pairing_request_info_t *pairing_info = (miui_fc_pairing_request_info_t *)output;
                APPS_LOG_MSGID_I(LOG_TAG" PAIRING_REQUEST event, pairing_info type=0x%02X initiate=0x%02X",
                                 2, pairing_info->message_type, pairing_info->initiate_role);

                // Use account_key to re-decrypt when ECDH->AES Key decrypt fail
                if (pairing_info->message_type != MIUI_FC_MESSAGE_TYPE_PAIRING_REQ
                    || (pairing_info->initiate_role != MIUI_FC_PAIRING_FROM_PEER
                        && pairing_info->initiate_role != MIUI_FC_PAIRING_FROM_LOCAL
                        && pairing_info->initiate_role != MIUI_FC_PAIRING_RE_WRITE_ACCOUNT_KEY)) {
                    APPS_LOG_MSGID_E(LOG_TAG" PAIRING_REQUEST event, AES Key Decrypt fail", 0);
                    bool exist_account_key = miui_fc_is_account_key_pairing();
                    if (exist_account_key) {
                        memcpy(miui_fc_context.aes_key, miui_fc_context.account_key, MIUI_FC_ACCOUNT_KEY_LEN);
                        miui_fc_aes_decrypt(req->encrypted_info, output);
                        pairing_info = (miui_fc_pairing_request_info_t *)output;
                        APPS_LOG_MSGID_I(LOG_TAG" PAIRING_REQUEST event, re-decrypt pairing_info type=0x%02X initiate=0x%02X",
                                         2, pairing_info->message_type, pairing_info->initiate_role);
                    }
                }

                if (pairing_info->message_type == MIUI_FC_MESSAGE_TYPE_PAIRING_REQ
                    && pairing_info->initiate_role == MIUI_FC_PAIRING_FROM_PEER) {
                    miui_fc_pairing_response_t pairing_rsp = {0};
                    pairing_rsp.message_type = MIUI_FC_MESSAGE_TYPE_PAIRING_RSP;
                    uint8_t *local_addr = (uint8_t *)bt_connection_manager_device_local_info_get_local_address();
                    memcpy(pairing_rsp.local_edr_addr, local_addr, 6);
                    bt_os_layer_generate_random_block(pairing_rsp.random_data, 9);
                    miui_fc_aes_encrypt((uint8_t *)&pairing_rsp, output);

                    ret = miui_fc_ble_send_data(MIUI_FC_BLE_CHAR_TYPE_PAIRING, output, sizeof(miui_fc_pairing_response_t));
                    if (ret) {
                        APPS_LOG_MSGID_I(LOG_TAG" PAIRING_REQUEST event, send_pairing_rsp pass", 0);
                        // set new IO capability for BT passkey exchange and start passkey timer
                        bt_device_manager_set_io_capability(BT_GAP_IO_CAPABILITY_DISPLAY_YES_NO);
                        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_XIAOAI,
                                              XIAOAI_EVENT_MIUI_FC_PASSKEY_REQUEST_TIMER);
                        ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                                            EVENT_GROUP_UI_SHELL_XIAOAI,
                                            XIAOAI_EVENT_MIUI_FC_PASSKEY_REQUEST_TIMER,
                                            NULL, 0,
                                            NULL, MIUI_FC_PASSKEY_REQUEST_TIMER);
                    } else {
                        APPS_LOG_MSGID_E(LOG_TAG" PAIRING_REQUEST event, send_pairing_rsp fail", 0);
                        miui_fc_interupt_pairing();
                    }
                } else {
                    APPS_LOG_MSGID_E(LOG_TAG" PAIRING_REQUEST event, error pairing_info", 0);
                    miui_fc_interupt_pairing();
                }
            } else {
                APPS_LOG_MSGID_E(LOG_TAG" PAIRING_REQUEST event, get_aes_key fail", 0);
                miui_fc_interupt_pairing();
            }
            break;
        }

        case XIAOAI_EVENT_MIUI_FC_PASSKEY_REQUEST_TIMER: {
            APPS_LOG_MSGID_I(LOG_TAG" PASSKEY_REQUEST_TIMER event, clear AES Key", 0);
            memset(miui_fc_context.aes_key, 0, 16);
            miui_fc_interupt_pairing();
            break;
        }

        case XIAOAI_EVENT_MIUI_FC_PASSKEY_REQUEST: {
            APPS_LOG_MSGID_I(LOG_TAG" PASSKEY_REQUEST event, data_len=%d", 1, data_len);
            if (extra_data != NULL && data_len == sizeof(miui_fc_passkey_request_t)) {
                uint8_t output[16] = {0};
                miui_fc_aes_decrypt((uint8_t *)extra_data, output);
                miui_fc_passkey_request_t *req = (miui_fc_passkey_request_t *)output;
                if (req->message_type == MIUI_FC_MESSAGE_TYPE_PEER_PASSKEY) {
                    uint32_t peer_passkey = MIUI_FC_REVERSE_UINT32((req->passkey << 8));
                    bool accept = (peer_passkey == miui_fc_context.passkey);
                    bt_status_t bt_status = bt_gap_reply_user_confirm_request(accept);
                    APPS_LOG_MSGID_I(LOG_TAG" PASSKEY_REQUEST event, accept=%d bt_status=0x%08X",
                                     2, accept, bt_status);
                    if (accept) {
                        req->message_type = MIUI_FC_MESSAGE_TYPE_LOCAL_PASSKEY;
                        req->passkey = MIUI_FC_REVERSE_UINT32((miui_fc_context.passkey << 8));
                        miui_fc_aes_encrypt((uint8_t *)&req, output);

                        bool ret = miui_fc_ble_send_data(MIUI_FC_BLE_CHAR_TYPE_PASSKEY,
                                                         output, sizeof(miui_fc_passkey_request_t));
                        if (ret) {
                            APPS_LOG_MSGID_E(LOG_TAG" PASSKEY_REQUEST event, send_passkey_req pass", 0);
                        } else {
                            APPS_LOG_MSGID_E(LOG_TAG" PASSKEY_REQUEST event, send_passkey_req fail", 0);
                            miui_fc_interupt_pairing();
                        }
                    } else {
                        APPS_LOG_MSGID_E(LOG_TAG" PASSKEY_REQUEST event, error passkey 0x%08X-0x%08X",
                                         2, peer_passkey, miui_fc_context.passkey);
                        miui_fc_interupt_pairing();
                    }
                } else {
                    APPS_LOG_MSGID_E(LOG_TAG" PASSKEY_REQUEST event, error message_type", 0);
                    miui_fc_interupt_pairing();
                }
            } else {
                APPS_LOG_MSGID_E(LOG_TAG" PASSKEY_REQUEST event, error extra_data/len", 0);
                miui_fc_interupt_pairing();
            }
            break;
        }

        case XIAOAI_EVENT_MIUI_FC_ACCOUNT_KEY_REQUEST: {
            APPS_LOG_MSGID_I(LOG_TAG" ACCOUNT_KEY_REQUEST event, data_len=%d", 1, data_len);
            if (extra_data != NULL && data_len == sizeof(miui_fc_account_key_request_t)) {
                uint8_t output[16] = {0};
                miui_fc_aes_decrypt((uint8_t *)extra_data, output);
                miui_fc_account_key_request_t *req = (miui_fc_account_key_request_t *)output;
                if (req->message_type == MIUI_FC_MESSAGE_TYPE_ACCOUNT_KEY) {
                    miui_fc_handle_account_key(output, TRUE);
                    app_va_xiaoai_miui_fc_start_adv();
                    APPS_LOG_MSGID_I(LOG_TAG" ACCOUNT_KEY_REQUEST event, MIUI Fast Connect Pairing pass", 0);
                    // disconnect BLE & reset IO capability after delay_time for reply account_key
                    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_XIAOAI,
                                          XIAOAI_EVENT_MIUI_FC_RESET_REQUEST);
                    ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGNEST,
                                        EVENT_GROUP_UI_SHELL_XIAOAI,
                                        XIAOAI_EVENT_MIUI_FC_RESET_REQUEST,
                                        NULL, 0, NULL, 2000);
                } else {
                    APPS_LOG_MSGID_E(LOG_TAG" ACCOUNT_KEY_REQUEST event, error message_type", 0);
                    miui_fc_interupt_pairing();
                }
            } else {
                APPS_LOG_MSGID_E(LOG_TAG" ACCOUNT_KEY_REQUEST event, error extra_data/len", 0);
                miui_fc_interupt_pairing();
            }
            break;
        }

        case XIAOAI_EVENT_MIUI_FC_RESET_REQUEST: {
            APPS_LOG_MSGID_I(LOG_TAG" RESET_REQUEST event", 0);
            miui_fc_interupt_pairing();
            break;
        }
    }
}



/*------------------------------------------------------------*/
/*                         Public API                         */
/*------------------------------------------------------------*/

void miui_fast_connect_init()
{
    uint8_t account_key[MIUI_FC_ACCOUNT_KEY_LEN] = {0};
    uint32_t size = MIUI_FC_ACCOUNT_KEY_LEN;
    nvkey_status_t status = nvkey_read_data(NVID_APP_XIAOAI_FAST_CONNECT, account_key, &size);
    APPS_LOG_MSGID_I(LOG_TAG" init, read status=%d size=%d", 2, status, size);
    if (status == NVKEY_STATUS_ITEM_NOT_FOUND
        || (status == NVKEY_STATUS_OK && size != MIUI_FC_ACCOUNT_KEY_LEN)) {
        memset(account_key, 0, MIUI_FC_ACCOUNT_KEY_LEN);
        size = MIUI_FC_ACCOUNT_KEY_LEN;
        status = nvkey_write_data(NVID_APP_XIAOAI_FAST_CONNECT, (const uint8_t *)account_key, size);
        APPS_LOG_MSGID_I(LOG_TAG" init, write status=%d", 1, status);
    }

    memset(&miui_fc_context, 0, sizeof(miui_fc_context_t));
    memcpy(miui_fc_context.local_private_key, miui_fc_default_private_key, MIUI_FC_ECDH_PRIVATE_KEY_LEN);
    memcpy(miui_fc_context.product_id, miui_fc_default_product_id, MIUI_FC_PRODUCT_ID_LEN);
    memcpy(miui_fc_context.account_key, account_key, MIUI_FC_ACCOUNT_KEY_LEN);

    bt_bd_addr_t *edr_addr = bt_device_manager_get_local_address();
    // Cur Agent EDR MAC Addr LAP
    miui_fc_context.random_account_key[0] = 0;
    miui_fc_context.random_account_key[1] = 0;
    miui_fc_context.random_account_key[2] = edr_addr[0];
    miui_fc_context.random_account_key[3] = edr_addr[1];
    miui_fc_context.random_account_key[4] = edr_addr[2];
}

void miui_fast_connect_ble_enable(bool enable)
{
    bt_status_t bt_status = BT_STATUS_FAIL;
    if (enable) {
        bt_status = bt_callback_manager_register_callback(bt_callback_type_app_event,
                                                          MODULE_MASK_SYSTEM | MODULE_MASK_GAP | MODULE_MASK_GATT | MODULE_MASK_MM,
                                                          (void *)miui_fc_ble_common_event_callback_handler);
        bt_gatts_set_max_mtu(MIUI_FC_BLE_DEFAULT_MTU);
    } else {
        bt_status = bt_callback_manager_deregister_callback(bt_callback_type_app_event,
                                                            (void *)miui_fc_ble_common_event_callback_handler);
    }
    APPS_LOG_MSGID_I(LOG_TAG" ble_enable, enable=%d bt_status=0x%08X", 2, enable, bt_status);
}

void miui_fast_connect_config_ble_adv(uint8_t *adv_data, uint8_t *scan_rsp)
{
    if ((adv_data == NULL && scan_rsp == NULL)
        || (adv_data != NULL && sizeof(adv_data) != 31)
        || (scan_rsp != NULL && sizeof(scan_rsp) != 31)) {
        APPS_LOG_MSGID_E(LOG_TAG" config_ble_adv, error parameter", 0);
        return;
    }
    if (adv_data != NULL) {
        miui_fc_update_ble_adv_data(adv_data);
    }
    if (scan_rsp != NULL) {
        miui_fc_update_ble_scan_rsp(scan_rsp);
    }
}

void miui_fast_connect_set_account_key(uint8_t *key)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    if (key == NULL || sizeof(key) != MIUI_FC_ACCOUNT_KEY_LEN) {
        APPS_LOG_MSGID_E(LOG_TAG" set_account_key, error key", 0);
        goto exit;
    } else if (key[0] != MIUI_FC_ACCOUNT_KEY_HEADER_BYTE) {
        APPS_LOG_MSGID_E(LOG_TAG" set_account_key, error KEY_HEADER_BYTE %02X",
                         1, key[0]);
        goto exit;
    }

    memcpy(miui_fc_context.account_key, key, MIUI_FC_ACCOUNT_KEY_LEN);
    nvkey_status_t status = nvkey_write_data(NVID_APP_XIAOAI_FAST_CONNECT,
                                             (const uint8_t *)key, MIUI_FC_ACCOUNT_KEY_LEN);
#ifdef MTK_AWS_MCE_ENABLE
    apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_XIAOAI,
                                   XIAOAI_EVENT_MIUI_FC_SYNC_ACCOUNT_KEY,
                                   (void *)key, MIUI_FC_ACCOUNT_KEY_LEN);
#endif
    APPS_LOG_MSGID_I(LOG_TAG" set_account_key, [%02X] nvkey_status=%d %02X:%02X:%02X:%02X:%02X",
                     7, role, status, key[0], key[1], key[2], key[3], key[4]);

exit:
    // Rewrite current account_key
    if (key != NULL) {
        memcpy(key, miui_fc_context.account_key, MIUI_FC_ACCOUNT_KEY_LEN);
    }
}

void miui_fast_connect_proc_ui_shell_event(uint32_t event_group,
                                           uint32_t event_id,
                                           void *extra_data,
                                           uint32_t data_len)
{
    switch (event_group) {
        /* UI Shell APP_INTERACTION events - only for RHO. */
        case EVENT_GROUP_UI_SHELL_APP_INTERACTION:
            //          miui_fc_interaction_event_group(event_id, extra_data, data_len);
            break;
        /* UI Shell BT Connection Manager events. */
        case EVENT_GROUP_UI_SHELL_BT_CONN_MANAGER:
            miui_fc_bt_cm_event_proc(event_id, extra_data, data_len);
            break;
#ifdef MTK_AWS_MCE_ENABLE
        /* UI Shell BT AWS_DATA events. */
        case EVENT_GROUP_UI_SHELL_AWS_DATA:
            miui_fc_bt_aws_event_data_proc(event_id, extra_data, data_len);
            break;
#endif
        /* APP XiaoAI events. */
        case EVENT_GROUP_UI_SHELL_XIAOAI:
            miui_fc_xiaoai_event_proc(event_id, extra_data, data_len);
            break;
    }
}

#endif /* AIR_XIAOAI_MIUI_FAST_CONNECT_ENABLE */
