/*
* (C) 2020  Airoha Technology Corp. All rights reserved.
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
#include "bt_le_audio_util.h"

#include "ble_pacs.h"

#include "bt_sink_srv_ami.h"

#include "bt_gap_le.h"
#include "bt_gatts.h"

/************************************************
*   ATTRIBUTE  HANDLE
*************************************************/
#define PACS_START_HANDLE                               (0x1200)        /**< PACS service start handle.*/
/* BLE_PACS_SINK_PAC_1 */
#define PACS_VALUE_HANDLE_SINK_PAC_1                    (0x1202)        /**< Sink PAC_1 characteristic handle.*/
#define PACS_VALUE_HANDLE_SINK_LOCATION                 (0x1205)        /**< Sink Location characteristic handle.*/
/* BLE_PACS_SOURCE_PAC_1 */
#define PACS_VALUE_HANDLE_SOURCE_PAC_1                  (0x1208)        /**< Source PAC_1 characteristic handle.*/
#define PACS_VALUE_HANDLE_SOURCE_LOCATION               (0x120B)        /**< Source Location characteristic handle.*/
#define PACS_VALUE_HANDLE_AVAILABILIE_AUDIO_CONTEXTS    (0x120E)        /**< Available Audio Contexts characteristic handle.*/
#define PACS_VALUE_HANDLE_SUPPORTED_AUDIO_CONTEXTS      (0x1211)        /**< Supported Audio Contexts characteristic handle.*/
#define PACS_END_HANDLE                                 (0x1212)        /**< PACS service end handle.*/

/************************************************
*   CHARC INDEX
*************************************************/
typedef enum {
    BLE_PACS_SINK_PAC_1,
    BLE_PACS_SINK_PAC_MAX_NUM,
} ble_pacs_sink_pac_t;

typedef enum {
    BLE_PACS_SOURCE_PAC_1,
    BLE_PACS_SOURCE_PAC_MAX_NUM,
} ble_pacs_source_pac_t;

#define BLE_PACS_CHARC_NORMAL  0

/************************************************
*   UUID
*************************************************/
static const bt_uuid_t BT_SIG_UUID_SINK_PAC                  = BT_UUID_INIT_WITH_UUID16(BT_SIG_UUID16_SINK_PAC);
static const bt_uuid_t BT_SIG_UUID_SINK_LOCATION             = BT_UUID_INIT_WITH_UUID16(BT_SIG_UUID16_SINK_LOCATION);
static const bt_uuid_t BT_SIG_UUID_SOURCE_PAC                = BT_UUID_INIT_WITH_UUID16(BT_SIG_UUID16_SOURCE_PAC);
static const bt_uuid_t BT_SIG_UUID_SOURCE_LOCATION           = BT_UUID_INIT_WITH_UUID16(BT_SIG_UUID16_SOURCE_LOCATION);
static const bt_uuid_t BT_SIG_UUID_AVAILABLE_AUDIO_CONTEXTS  = BT_UUID_INIT_WITH_UUID16(BT_SIG_UUID16_AVAILABLE_AUDIO_CONTEXTS);
static const bt_uuid_t BT_SIG_UUID_SUPPORTED_AUDIO_CONTEXTS  = BT_UUID_INIT_WITH_UUID16(BT_SIG_UUID16_SUPPORTED_AUDIO_CONTEXTS);

/************************************************
*   ATTRIBUTE VALUE HANDLE
*************************************************/
static const ble_pacs_attribute_handle_t g_pacs_att_handle_tbl[] = {
    {BLE_PACS_UUID_TYPE_PACS_SERVICE,                PACS_START_HANDLE},

    /* BLE_PACS_SINK_PAC_1 */
    {BLE_PACS_UUID_TYPE_SINK_PAC,                    PACS_VALUE_HANDLE_SINK_PAC_1},

    /* Sink Audio Locations */
    {BLE_PACS_UUID_TYPE_SINK_LOCATION,               PACS_VALUE_HANDLE_SINK_LOCATION},

    /* BLE_PACS_SOURCE_PAC_1 */
    {BLE_PACS_UUID_TYPE_SOURCE_PAC,                  PACS_VALUE_HANDLE_SOURCE_PAC_1},

    /* Source Audio Locations */
    {BLE_PACS_UUID_TYPE_SOURCE_LOCATION,             PACS_VALUE_HANDLE_SOURCE_LOCATION},

    /* Available Audio Contexts */
    {BLE_PACS_UUID_TYPE_AVAILABLE_AUDIO_CONTEXTS,    PACS_VALUE_HANDLE_AVAILABILIE_AUDIO_CONTEXTS},

    /* Supported Audio Contexts */
    {BLE_PACS_UUID_TYPE_SUPPORTED_AUDIO_CONTEXTS,    PACS_VALUE_HANDLE_SUPPORTED_AUDIO_CONTEXTS},

    {BLE_PACS_UUID_TYPE_INVALID,                     PACS_END_HANDLE}, /* END g_pacs_att_handle_tbl */
};

/************************************************
*   CALLBACK
*************************************************/
/* BLE_PACS_SINK_PAC_1 */
static uint32_t ble_pacs_sink_pac_value_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);
static uint32_t ble_pacs_sink_pac_client_config_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);

/* Sink Audio Locations */
static uint32_t ble_pacs_sink_location_value_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);
static uint32_t ble_pacs_sink_location_client_config_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);

/* BLE_PACS_SOURCE_PAC_1 */
static uint32_t ble_pacs_source_pac_value_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);
static uint32_t ble_pacs_source_pac_client_config_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);

/* Source Audio Locations */
static uint32_t ble_pacs_source_location_value_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);
static uint32_t ble_pacs_source_location_client_config_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);

/* Available Audio Contexts */
static uint32_t ble_pacs_available_audio_contexts_value_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);
static uint32_t ble_pacs_available_audio_contexts_client_config_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);

/* Supported Audio Contexts */
static uint32_t ble_pacs_supported_audio_contexts_value_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);
static uint32_t ble_pacs_supported_audio_contexts_client_config_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);

/************************************************
*   SERVICE TABLE
*************************************************/
/* PACS Service */
BT_GATTS_NEW_PRIMARY_SERVICE_16(ble_pacs_primary_service, BT_GATT_UUID16_PACS_SERVICE);

/* BLE_PACS_SINK_PAC_1 */
BT_GATTS_NEW_CHARC_16(ble_pacs_char4_sink_pac,
                      BT_GATT_CHARC_PROP_READ | BT_GATT_CHARC_PROP_NOTIFY,
                      PACS_VALUE_HANDLE_SINK_PAC_1,
                      BT_SIG_UUID16_SINK_PAC);
BT_GATTS_NEW_CHARC_VALUE_CALLBACK(ble_pacs_sink_pac_value,
                                  BT_SIG_UUID_SINK_PAC,
                                  BT_GATTS_REC_PERM_READABLE_ENCRYPTION,
                                  ble_pacs_sink_pac_value_callback);
BT_GATTS_NEW_CLIENT_CHARC_CONFIG(ble_pacs_sink_pac_config,
                                 BT_GATTS_REC_PERM_READABLE_ENCRYPTION | BT_GATTS_REC_PERM_WRITABLE_ENCRYPTION,
                                 ble_pacs_sink_pac_client_config_callback);

/* Sink Audio Locations */
BT_GATTS_NEW_CHARC_16(ble_pacs_char4_sink_location,
                      BT_GATT_CHARC_PROP_READ | BT_GATT_CHARC_PROP_NOTIFY,
                      PACS_VALUE_HANDLE_SINK_LOCATION,
                      BT_SIG_UUID16_SINK_LOCATION);
BT_GATTS_NEW_CHARC_VALUE_CALLBACK(ble_pacs_sink_location_value,
                                  BT_SIG_UUID_SINK_LOCATION,
                                  BT_GATTS_REC_PERM_READABLE_ENCRYPTION,
                                  ble_pacs_sink_location_value_callback);
BT_GATTS_NEW_CLIENT_CHARC_CONFIG(ble_pacs_sink_location_config,
                                 BT_GATTS_REC_PERM_READABLE_ENCRYPTION | BT_GATTS_REC_PERM_WRITABLE_ENCRYPTION,
                                 ble_pacs_sink_location_client_config_callback);

/* BLE_PACS_SOURCE_PAC_1 */
BT_GATTS_NEW_CHARC_16(ble_pacs_char4_source_pac,
                      BT_GATT_CHARC_PROP_READ | BT_GATT_CHARC_PROP_NOTIFY,
                      PACS_VALUE_HANDLE_SOURCE_PAC_1,
                      BT_SIG_UUID16_SOURCE_PAC);
BT_GATTS_NEW_CHARC_VALUE_CALLBACK(ble_pacs_source_pac_value,
                                  BT_SIG_UUID_SOURCE_PAC,
                                  BT_GATTS_REC_PERM_READABLE_ENCRYPTION,
                                  ble_pacs_source_pac_value_callback);
BT_GATTS_NEW_CLIENT_CHARC_CONFIG(ble_pacs_source_pac_config,
                                 BT_GATTS_REC_PERM_READABLE_ENCRYPTION | BT_GATTS_REC_PERM_WRITABLE_ENCRYPTION,
                                 ble_pacs_source_pac_client_config_callback);

/* Source Audio Locations */
BT_GATTS_NEW_CHARC_16(ble_pacs_char4_source_location,
                      BT_GATT_CHARC_PROP_READ | BT_GATT_CHARC_PROP_NOTIFY,
                      PACS_VALUE_HANDLE_SOURCE_LOCATION,
                      BT_SIG_UUID16_SOURCE_LOCATION);
BT_GATTS_NEW_CHARC_VALUE_CALLBACK(ble_pacs_source_location_value,
                                  BT_SIG_UUID_SOURCE_LOCATION,
                                  BT_GATTS_REC_PERM_READABLE_ENCRYPTION,
                                  ble_pacs_source_location_value_callback);
BT_GATTS_NEW_CLIENT_CHARC_CONFIG(ble_pacs_source_location_config,
                                 BT_GATTS_REC_PERM_READABLE_ENCRYPTION | BT_GATTS_REC_PERM_WRITABLE_ENCRYPTION,
                                 ble_pacs_source_location_client_config_callback);

/* Available Audio Contexts */
BT_GATTS_NEW_CHARC_16(ble_pacs_char4_available_audio_contexts,
                      BT_GATT_CHARC_PROP_READ | BT_GATT_CHARC_PROP_NOTIFY,
                      PACS_VALUE_HANDLE_AVAILABILIE_AUDIO_CONTEXTS,
                      BT_SIG_UUID16_AVAILABLE_AUDIO_CONTEXTS);
BT_GATTS_NEW_CHARC_VALUE_CALLBACK(ble_pacs_available_audio_contexts_value,
                                  BT_SIG_UUID_AVAILABLE_AUDIO_CONTEXTS,
                                  BT_GATTS_REC_PERM_READABLE_ENCRYPTION,
                                  ble_pacs_available_audio_contexts_value_callback);
BT_GATTS_NEW_CLIENT_CHARC_CONFIG(ble_pacs_available_audio_contexts_config,
                                 BT_GATTS_REC_PERM_READABLE_ENCRYPTION | BT_GATTS_REC_PERM_WRITABLE_ENCRYPTION,
                                 ble_pacs_available_audio_contexts_client_config_callback);

/* Supported Audio Contexts */
BT_GATTS_NEW_CHARC_16(ble_pacs_char4_supported_audio_contexts,
                      BT_GATT_CHARC_PROP_READ | BT_GATT_CHARC_PROP_NOTIFY,
                      PACS_VALUE_HANDLE_SUPPORTED_AUDIO_CONTEXTS,
                      BT_SIG_UUID16_SUPPORTED_AUDIO_CONTEXTS);
BT_GATTS_NEW_CHARC_VALUE_CALLBACK(ble_pacs_supported_audio_contexts_value,
                                  BT_SIG_UUID_SUPPORTED_AUDIO_CONTEXTS,
                                  BT_GATTS_REC_PERM_READABLE_ENCRYPTION,
                                  ble_pacs_supported_audio_contexts_value_callback);
BT_GATTS_NEW_CLIENT_CHARC_CONFIG(ble_pacs_supported_audio_contexts_config,
                                 BT_GATTS_REC_PERM_READABLE_ENCRYPTION | BT_GATTS_REC_PERM_WRITABLE_ENCRYPTION,
                                 ble_pacs_supported_audio_contexts_client_config_callback);


static const bt_gatts_service_rec_t *ble_pacs_service_rec[] = {
    (const bt_gatts_service_rec_t *) &ble_pacs_primary_service,
    /* BLE_PACS_SINK_PAC_1 */
    (const bt_gatts_service_rec_t *) &ble_pacs_char4_sink_pac,
    (const bt_gatts_service_rec_t *) &ble_pacs_sink_pac_value,
    (const bt_gatts_service_rec_t *) &ble_pacs_sink_pac_config,

    (const bt_gatts_service_rec_t *) &ble_pacs_char4_sink_location,
    (const bt_gatts_service_rec_t *) &ble_pacs_sink_location_value,
    (const bt_gatts_service_rec_t *) &ble_pacs_sink_location_config,

    /* BLE_PACS_SOURCE_PAC_1 */
    (const bt_gatts_service_rec_t *) &ble_pacs_char4_source_pac,
    (const bt_gatts_service_rec_t *) &ble_pacs_source_pac_value,
    (const bt_gatts_service_rec_t *) &ble_pacs_source_pac_config,

    (const bt_gatts_service_rec_t *) &ble_pacs_char4_source_location,
    (const bt_gatts_service_rec_t *) &ble_pacs_source_location_value,
    (const bt_gatts_service_rec_t *) &ble_pacs_source_location_config,

    (const bt_gatts_service_rec_t *) &ble_pacs_char4_available_audio_contexts,
    (const bt_gatts_service_rec_t *) &ble_pacs_available_audio_contexts_value,
    (const bt_gatts_service_rec_t *) &ble_pacs_available_audio_contexts_config,
    (const bt_gatts_service_rec_t *) &ble_pacs_char4_supported_audio_contexts,
    (const bt_gatts_service_rec_t *) &ble_pacs_supported_audio_contexts_value,
    (const bt_gatts_service_rec_t *) &ble_pacs_supported_audio_contexts_config,
};

const bt_gatts_service_t ble_pacs_service = {
    .starting_handle = PACS_START_HANDLE,
    .ending_handle = PACS_END_HANDLE,
    .required_encryption_key_size = 7,
    .records = ble_pacs_service_rec
};

/************************************************
*   CALLBACK
*************************************************/
/* BLE_PACS_SINK_PAC_1 */
static uint32_t ble_pacs_sink_pac_value_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    if (handle != BT_HANDLE_INVALID && rw == BT_GATTS_CALLBACK_READ) {
        return ble_pacs_gatt_request_handler(BLE_PACS_READ_SINK_PAC, handle, BLE_PACS_SINK_PAC_1, data, size, offset);
    }

    return 0;
}

static uint32_t ble_pacs_sink_pac_client_config_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    if (handle != BT_HANDLE_INVALID) {
        if (rw == BT_GATTS_CALLBACK_WRITE) {
            return ble_pacs_gatt_request_handler(BLE_PACS_WRITE_SINK_PAC_CCCD, handle, BLE_PACS_SINK_PAC_1, data, size, offset);

        } else if (rw == BT_GATTS_CALLBACK_READ) {
            return ble_pacs_gatt_request_handler(BLE_PACS_READ_SINK_PAC_CCCD, handle, BLE_PACS_SINK_PAC_1, data, size, offset);
        }
    }
    return 0;
}

/* Sink Audio Locations */
static uint32_t ble_pacs_sink_location_value_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    if (handle != BT_HANDLE_INVALID && rw == BT_GATTS_CALLBACK_READ) {
        return ble_pacs_gatt_request_handler(BLE_PACS_READ_SINK_LOCATION, handle, BLE_PACS_CHARC_NORMAL, data, size, offset);
    }

    return 0;
}

static uint32_t ble_pacs_sink_location_client_config_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    if (handle != BT_HANDLE_INVALID) {
        if (rw == BT_GATTS_CALLBACK_WRITE) {
            return ble_pacs_gatt_request_handler(BLE_PACS_WRITE_SINK_LOCATION_CCCD, handle, BLE_PACS_CHARC_NORMAL, data, size, offset);

        } else if (rw == BT_GATTS_CALLBACK_READ) {
            return ble_pacs_gatt_request_handler(BLE_PACS_READ_SINK_LOCATION_CCCD, handle, BLE_PACS_CHARC_NORMAL, data, size, offset);
        }
    }
    return 0;
}

/* BLE_PACS_SOURCE_PAC_1 */
static uint32_t ble_pacs_source_pac_value_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    if (handle != BT_HANDLE_INVALID && rw == BT_GATTS_CALLBACK_READ) {
        return ble_pacs_gatt_request_handler(BLE_PACS_READ_SOURCE_PAC, handle, BLE_PACS_SOURCE_PAC_1, data, size, offset);
    }

    return 0;
}

static uint32_t ble_pacs_source_pac_client_config_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    if (handle != BT_HANDLE_INVALID) {
        if (rw == BT_GATTS_CALLBACK_WRITE) {
            return ble_pacs_gatt_request_handler(BLE_PACS_WRITE_SOURCE_PAC_CCCD, handle, BLE_PACS_SOURCE_PAC_1, data, size, offset);

        } else if (rw == BT_GATTS_CALLBACK_READ) {
            return ble_pacs_gatt_request_handler(BLE_PACS_READ_SOURCE_PAC_CCCD, handle, BLE_PACS_SOURCE_PAC_1, data, size, offset);
        }
    }
    return 0;
}

/* Source Audio Locations */
static uint32_t ble_pacs_source_location_value_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    if (handle != BT_HANDLE_INVALID && rw == BT_GATTS_CALLBACK_READ) {
        return ble_pacs_gatt_request_handler(BLE_PACS_READ_SOURCE_LOCATION, handle, BLE_PACS_CHARC_NORMAL, data, size, offset);
    }

    return 0;
}

static uint32_t ble_pacs_source_location_client_config_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    if (handle != BT_HANDLE_INVALID) {
        if (rw == BT_GATTS_CALLBACK_WRITE) {
            return ble_pacs_gatt_request_handler(BLE_PACS_WRITE_SOURCE_LOCATION_CCCD, handle, BLE_PACS_CHARC_NORMAL, data, size, offset);

        } else if (rw == BT_GATTS_CALLBACK_READ) {
            return ble_pacs_gatt_request_handler(BLE_PACS_READ_SOURCE_LOCATION_CCCD, handle, BLE_PACS_CHARC_NORMAL, data, size, offset);
        }
    }
    return 0;
}

/* Available Audio Contexts */
static uint32_t ble_pacs_available_audio_contexts_value_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    if (handle != BT_HANDLE_INVALID && rw == BT_GATTS_CALLBACK_READ) {
        return ble_pacs_gatt_request_handler(BLE_PACS_READ_AVAILABLE_AUDIO_CONTEXTS, handle, BLE_PACS_CHARC_NORMAL, data, size, offset);
    }

    return 0;
}


static uint32_t ble_pacs_available_audio_contexts_client_config_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    if (handle != BT_HANDLE_INVALID) {
        if (rw == BT_GATTS_CALLBACK_WRITE) {
            return ble_pacs_gatt_request_handler(BLE_PACS_WRITE_AVAILABLE_AUDIO_CONTEXTS_CCCD, handle, BLE_PACS_CHARC_NORMAL, data, size, offset);

        } else if (rw == BT_GATTS_CALLBACK_READ) {
            return ble_pacs_gatt_request_handler(BLE_PACS_READ_AVAILABLE_AUDIO_CONTEXTS_CCCD, handle, BLE_PACS_CHARC_NORMAL, data, size, offset);
        }
    }
    return 0;
}

/* Supported Audio Contexts */
static uint32_t ble_pacs_supported_audio_contexts_value_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    if (handle != BT_HANDLE_INVALID && rw == BT_GATTS_CALLBACK_READ) {
        return ble_pacs_gatt_request_handler(BLE_PACS_READ_SUPPORTED_AUDIO_CONTEXTS, handle, BLE_PACS_CHARC_NORMAL, data, size, offset);
    }

    return 0;
}


static uint32_t ble_pacs_supported_audio_contexts_client_config_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    if (handle != BT_HANDLE_INVALID) {
        if (rw == BT_GATTS_CALLBACK_WRITE) {
            return ble_pacs_gatt_request_handler(BLE_PACS_WRITE_SUPPORTED_AUDIO_CONTEXTS_CCCD, handle, BLE_PACS_CHARC_NORMAL, data, size, offset);

        } else if (rw == BT_GATTS_CALLBACK_READ) {
            return ble_pacs_gatt_request_handler(BLE_PACS_READ_SUPPORTED_AUDIO_CONTEXTS_CCCD, handle, BLE_PACS_CHARC_NORMAL, data, size, offset);
        }
    }
    return 0;
}

/************************************************
*   Public functions
*************************************************/
/* BLE_PACS_SINK_PAC_1 */
#ifdef AIR_LE_AUDIO_LC3PLUS_ENABLE
/* BLE_PACS_SINK_PAC_1 */
#define PACS_SINK_PAC_1_RECORD_NUM      0x05
/* BLE_PACS_SOURCE_PAC_1 */
#define PACS_SOURCE_PAC_1_RECORD_NUM    0x05
#else
/* BLE_PACS_SINK_PAC_1 */
#define PACS_SINK_PAC_1_RECORD_NUM      0x04
/* BLE_PACS_SOURCE_PAC_1 */
#define PACS_SOURCE_PAC_1_RECORD_NUM    0x04
#endif
#define PACS_CODEC_CAPABLITIES_LEN      0x13
#define PACS_METADATA_LEN               0x04

static uint8_t g_pacs_codec_capabilities_16k[] = {
    CODEC_CAPABILITY_LEN_SUPPORTED_SAMPLING_FREQUENCY,
    CODEC_CAPABILITY_TYPE_SUPPORTED_SAMPLING_FREQUENCY,
    (uint8_t)SUPPORTED_SAMPLING_FREQ_16KHZ, (uint8_t)(SUPPORTED_SAMPLING_FREQ_16KHZ >> 8),

    CODEC_CAPABILITY_LEN_SUPPORTED_FRAME_DURATIONS,
    CODEC_CAPABILITY_TYPE_SUPPORTED_FRAME_DURATIONS,
    SUPPORTED_FRAME_DURATIONS_7P5_MS | SUPPORTED_FRAME_DURATIONS_10_MS,

    CODEC_CAPABILITY_LEN_AUDIO_CHANNEL_COUNTS,
    CODEC_CAPABILITY_TYPE_AUDIO_CHANNEL_COUNTS,
#ifdef AIR_LE_AUDIO_HEADSET_ENABLE
    AUDIO_CHANNEL_COUNTS_1,// | AUDIO_CHANNEL_COUNTS_2,
#else
    AUDIO_CHANNEL_COUNTS_1,
#endif

    CODEC_CAPABILITY_LEN_SUPPORTED_OCTETS_PER_CODEC_FRAME,
    CODEC_CAPABILITY_TYPE_SUPPORTED_OCTETS_PER_CODEC_FRAME,
    (uint8_t)SUPPORTED_OCTETS_PER_CODEC_FRAME_30_40, (uint8_t)(SUPPORTED_OCTETS_PER_CODEC_FRAME_30_40 >> 8),
    (uint8_t)(SUPPORTED_OCTETS_PER_CODEC_FRAME_30_40 >> 16), (uint8_t)(SUPPORTED_OCTETS_PER_CODEC_FRAME_30_40 >> 24),

    CODEC_CAPABILITY_LEN_SUPPORTED_MAX_CODEC_FRAMES_PER_SDU,
    CODEC_CAPABILITY_TYPE_SUPPORTED_MAX_CODEC_FRAMES_PER_SDU,
#ifdef AIR_LE_AUDIO_HEADSET_ENABLE
    MAX_SUPPORTED_LC3_FRAMES_PER_SDU_1,
#else
    MAX_SUPPORTED_LC3_FRAMES_PER_SDU_1,
#endif
};

static uint8_t g_pacs_codec_capabilities_24k[] = {
    CODEC_CAPABILITY_LEN_SUPPORTED_SAMPLING_FREQUENCY,
    CODEC_CAPABILITY_TYPE_SUPPORTED_SAMPLING_FREQUENCY,
    (uint8_t)SUPPORTED_SAMPLING_FREQ_24KHZ, (uint8_t)(SUPPORTED_SAMPLING_FREQ_24KHZ >> 8),

    CODEC_CAPABILITY_LEN_SUPPORTED_FRAME_DURATIONS,
    CODEC_CAPABILITY_TYPE_SUPPORTED_FRAME_DURATIONS,
    SUPPORTED_FRAME_DURATIONS_7P5_MS | SUPPORTED_FRAME_DURATIONS_10_MS,

    CODEC_CAPABILITY_LEN_AUDIO_CHANNEL_COUNTS,
    CODEC_CAPABILITY_TYPE_AUDIO_CHANNEL_COUNTS,
#ifdef AIR_LE_AUDIO_HEADSET_ENABLE
    AUDIO_CHANNEL_COUNTS_1,// | AUDIO_CHANNEL_COUNTS_2,
#else
    AUDIO_CHANNEL_COUNTS_1,
#endif

    CODEC_CAPABILITY_LEN_SUPPORTED_OCTETS_PER_CODEC_FRAME,
    CODEC_CAPABILITY_TYPE_SUPPORTED_OCTETS_PER_CODEC_FRAME,
    (uint8_t)SUPPORTED_OCTETS_PER_CODEC_FRAME_45_60, (uint8_t)(SUPPORTED_OCTETS_PER_CODEC_FRAME_45_60 >> 8),
    (uint8_t)(SUPPORTED_OCTETS_PER_CODEC_FRAME_45_60 >> 16), (uint8_t)(SUPPORTED_OCTETS_PER_CODEC_FRAME_45_60 >> 24),

    CODEC_CAPABILITY_LEN_SUPPORTED_MAX_CODEC_FRAMES_PER_SDU,
    CODEC_CAPABILITY_TYPE_SUPPORTED_MAX_CODEC_FRAMES_PER_SDU,
#ifdef AIR_LE_AUDIO_HEADSET_ENABLE
    MAX_SUPPORTED_LC3_FRAMES_PER_SDU_1,
#else
    MAX_SUPPORTED_LC3_FRAMES_PER_SDU_1,
#endif
};

static uint8_t g_pacs_codec_capabilities_32k[] = {
    CODEC_CAPABILITY_LEN_SUPPORTED_SAMPLING_FREQUENCY,     /* length */
    CODEC_CAPABILITY_TYPE_SUPPORTED_SAMPLING_FREQUENCY,    /* type */
    (uint8_t)SUPPORTED_SAMPLING_FREQ_32KHZ, (uint8_t)(SUPPORTED_SAMPLING_FREQ_32KHZ >> 8),  /* value */

    CODEC_CAPABILITY_LEN_SUPPORTED_FRAME_DURATIONS,
    CODEC_CAPABILITY_TYPE_SUPPORTED_FRAME_DURATIONS,
    SUPPORTED_FRAME_DURATIONS_7P5_MS | SUPPORTED_FRAME_DURATIONS_10_MS,

    CODEC_CAPABILITY_LEN_AUDIO_CHANNEL_COUNTS,
    CODEC_CAPABILITY_TYPE_AUDIO_CHANNEL_COUNTS,
#ifdef AIR_LE_AUDIO_HEADSET_ENABLE
    AUDIO_CHANNEL_COUNTS_1,// | AUDIO_CHANNEL_COUNTS_2,
#else
    AUDIO_CHANNEL_COUNTS_1,
#endif

    CODEC_CAPABILITY_LEN_SUPPORTED_OCTETS_PER_CODEC_FRAME,
    CODEC_CAPABILITY_TYPE_SUPPORTED_OCTETS_PER_CODEC_FRAME,
    (uint8_t)SUPPORTED_OCTETS_PER_CODEC_FRAME_60_80, (uint8_t)(SUPPORTED_OCTETS_PER_CODEC_FRAME_60_80 >> 8),
    (uint8_t)(SUPPORTED_OCTETS_PER_CODEC_FRAME_60_80 >> 16), (uint8_t)(SUPPORTED_OCTETS_PER_CODEC_FRAME_60_80 >> 24),

    CODEC_CAPABILITY_LEN_SUPPORTED_MAX_CODEC_FRAMES_PER_SDU,
    CODEC_CAPABILITY_TYPE_SUPPORTED_MAX_CODEC_FRAMES_PER_SDU,
#ifdef AIR_LE_AUDIO_HEADSET_ENABLE
    MAX_SUPPORTED_LC3_FRAMES_PER_SDU_1,
#else
    MAX_SUPPORTED_LC3_FRAMES_PER_SDU_1,
#endif
};

static uint8_t g_pacs_codec_capabilities_48k[] = {
    CODEC_CAPABILITY_LEN_SUPPORTED_SAMPLING_FREQUENCY,
    CODEC_CAPABILITY_TYPE_SUPPORTED_SAMPLING_FREQUENCY,
    (uint8_t)SUPPORTED_SAMPLING_FREQ_48KHZ, (uint8_t)(SUPPORTED_SAMPLING_FREQ_48KHZ >> 8),

    CODEC_CAPABILITY_LEN_SUPPORTED_FRAME_DURATIONS,
    CODEC_CAPABILITY_TYPE_SUPPORTED_FRAME_DURATIONS,
    SUPPORTED_FRAME_DURATIONS_7P5_MS | SUPPORTED_FRAME_DURATIONS_10_MS,

    CODEC_CAPABILITY_LEN_AUDIO_CHANNEL_COUNTS,
    CODEC_CAPABILITY_TYPE_AUDIO_CHANNEL_COUNTS,
#ifdef AIR_LE_AUDIO_HEADSET_ENABLE
    AUDIO_CHANNEL_COUNTS_1,// | AUDIO_CHANNEL_COUNTS_2,
#else
    AUDIO_CHANNEL_COUNTS_1,
#endif

    CODEC_CAPABILITY_LEN_SUPPORTED_OCTETS_PER_CODEC_FRAME,
    CODEC_CAPABILITY_TYPE_SUPPORTED_OCTETS_PER_CODEC_FRAME,
    (uint8_t)SUPPORTED_OCTETS_PER_CODEC_FRAME_75_155, (uint8_t)(SUPPORTED_OCTETS_PER_CODEC_FRAME_75_155 >> 8),
    (uint8_t)(SUPPORTED_OCTETS_PER_CODEC_FRAME_75_155 >> 16), (uint8_t)(SUPPORTED_OCTETS_PER_CODEC_FRAME_75_155 >> 24),

    CODEC_CAPABILITY_LEN_SUPPORTED_MAX_CODEC_FRAMES_PER_SDU,
    CODEC_CAPABILITY_TYPE_SUPPORTED_MAX_CODEC_FRAMES_PER_SDU,
#ifdef AIR_LE_AUDIO_HEADSET_ENABLE
    MAX_SUPPORTED_LC3_FRAMES_PER_SDU_1,
#else
    MAX_SUPPORTED_LC3_FRAMES_PER_SDU_1,
#endif
};

#ifdef AIR_LE_AUDIO_LC3PLUS_ENABLE
static uint8_t g_pacs_codec_capabilities_96k[] =
{
    CODEC_CAPABILITY_LEN_SUPPORTED_SAMPLING_FREQUENCY,
    CODEC_CAPABILITY_TYPE_SUPPORTED_SAMPLING_FREQUENCY,
    (uint8_t)SUPPORTED_SAMPLING_FREQ_96KHZ, (uint8_t)(SUPPORTED_SAMPLING_FREQ_96KHZ >> 8),

    CODEC_CAPABILITY_LEN_SUPPORTED_FRAME_DURATIONS,
    CODEC_CAPABILITY_TYPE_SUPPORTED_FRAME_DURATIONS,
    SUPPORTED_FRAME_DURATIONS_10_MS,

    CODEC_CAPABILITY_LEN_AUDIO_CHANNEL_COUNTS,
    CODEC_CAPABILITY_TYPE_AUDIO_CHANNEL_COUNTS,
#ifdef AIR_LE_AUDIO_HEADSET_ENABLE
    AUDIO_CHANNEL_COUNTS_1,// | AUDIO_CHANNEL_COUNTS_2,
#else
    AUDIO_CHANNEL_COUNTS_1,
#endif

    CODEC_CAPABILITY_LEN_SUPPORTED_OCTETS_PER_CODEC_FRAME,
    CODEC_CAPABILITY_TYPE_SUPPORTED_OCTETS_PER_CODEC_FRAME,
    (uint8_t)SUPPORTED_OCTETS_PER_CODEC_FRAME_190_190, (uint8_t)(SUPPORTED_OCTETS_PER_CODEC_FRAME_190_190 >> 8),
    (uint8_t)(SUPPORTED_OCTETS_PER_CODEC_FRAME_190_190 >> 16), (uint8_t)(SUPPORTED_OCTETS_PER_CODEC_FRAME_190_190 >> 24),

    CODEC_CAPABILITY_LEN_SUPPORTED_MAX_CODEC_FRAMES_PER_SDU,
    CODEC_CAPABILITY_TYPE_SUPPORTED_MAX_CODEC_FRAMES_PER_SDU,
#ifdef AIR_LE_AUDIO_HEADSET_ENABLE
    MAX_SUPPORTED_LC3_FRAMES_PER_SDU_1,
#else
    MAX_SUPPORTED_LC3_FRAMES_PER_SDU_1,
#endif
};
#endif

static uint8_t g_pacs_metadata[] = {0x03, 0x01, 0x01, 0x00};

static ble_pacs_pac_record_t g_pacs_pac_1[] = {
    {
        CODEC_ID_LC3,
        PACS_CODEC_CAPABLITIES_LEN,
        &g_pacs_codec_capabilities_16k[0],
        PACS_METADATA_LEN,
        &g_pacs_metadata[0],
    },
    {
        CODEC_ID_LC3,
        PACS_CODEC_CAPABLITIES_LEN,
        &g_pacs_codec_capabilities_32k[0],
        PACS_METADATA_LEN,
        &g_pacs_metadata[0],
    },
    {
        CODEC_ID_LC3,
        PACS_CODEC_CAPABLITIES_LEN,
        &g_pacs_codec_capabilities_24k[0],
        PACS_METADATA_LEN,
        &g_pacs_metadata[0],
    },
    {
        CODEC_ID_LC3,
        PACS_CODEC_CAPABLITIES_LEN,
        &g_pacs_codec_capabilities_48k[0],
        PACS_METADATA_LEN,
        &g_pacs_metadata[0],
    },
#ifdef AIR_LE_AUDIO_LC3PLUS_ENABLE
    {
        CODEC_ID_LC3PLUS_CBR,
        PACS_CODEC_CAPABLITIES_LEN,
        &g_pacs_codec_capabilities_96k[0],
        PACS_METADATA_LEN,
        &g_pacs_metadata[0],
    },
#endif
};

/* BLE_PACS_PAC_1 */
ble_pacs_pac_t g_pacs_pac_empty = {
    0,
    NULL,
};

/* BLE_PACS_SINK_PAC_1 */
ble_pacs_pac_t g_pacs_sink_pac_1 = {
    PACS_SINK_PAC_1_RECORD_NUM,
    &g_pacs_pac_1[0],
};

ble_pacs_pac_t g_pacs_source_pac_1 = {
    PACS_SOURCE_PAC_1_RECORD_NUM,
    &g_pacs_pac_1[0],
};

const ble_pacs_attribute_handle_t *ble_pacs_get_attribute_handle_tbl(void)
{
    return g_pacs_att_handle_tbl;
}

bool ble_pacs_support_lc3plus(void)
{
#ifdef AIR_LE_AUDIO_LC3PLUS_ENABLE
    return true;
#else
    return false;
#endif
}

uint8_t ble_pacs_get_sink_pac_number(void)
{
    return BLE_PACS_SINK_PAC_MAX_NUM;
}

uint8_t ble_pacs_get_source_pac_number(void)
{
    return BLE_PACS_SOURCE_PAC_MAX_NUM;
}

bt_status_t ble_pacs_set_pac_record_availability(bt_le_audio_direction_t direction, bool available)
{
    //bt_handle_t handle = BT_HANDLE_INVALID;
    bt_status_t status = BT_STATUS_FAIL;
    if (direction == AUDIO_DIRECTION_SINK) {
        status = ble_pacs_set_pac(direction, BLE_PACS_SINK_PAC_1, (available ? &g_pacs_sink_pac_1 : &g_pacs_pac_empty));
    } else if (direction == AUDIO_DIRECTION_SOURCE) {
        status = ble_pacs_set_pac(direction, BLE_PACS_SOURCE_PAC_1, (available ? &g_pacs_source_pac_1 : &g_pacs_pac_empty));
    }

    return status;
}

bt_status_t ble_pacs_switch_pac_record(uint8_t channel_num)
{
    ble_pacs_pac_t *p_pac = ble_pacs_get_pac(AUDIO_DIRECTION_SINK, BLE_PACS_SINK_PAC_1);
    uint8_t i, channel_count = 0, frame_per_sdu = 0;
    uint8_t *p_temp = NULL;

    if (NULL == p_pac || !channel_num) {
        return BT_STATUS_FAIL;
    }

    for (i = 0; i < p_pac->num_of_record; i++) {

        p_temp = ble_pacs_get_ltv_value_from_codec_specific_capabilities(CODEC_CAPABILITY_TYPE_AUDIO_CHANNEL_COUNTS,
            p_pac->record[i].codec_specific_capabilities_length, p_pac->record[i].codec_specific_capabilities);

        if (p_temp != NULL) {
            channel_count = channel_num;
            if (channel_count != *p_temp) {
                *p_temp = channel_count;
            }
        }

        p_temp = ble_pacs_get_ltv_value_from_codec_specific_capabilities(CODEC_CAPABILITY_TYPE_SUPPORTED_MAX_CODEC_FRAMES_PER_SDU,
            p_pac->record[i].codec_specific_capabilities_length, p_pac->record[i].codec_specific_capabilities);

        if (p_temp != NULL) {
            frame_per_sdu = channel_num;
            if (frame_per_sdu != *p_temp) {
                *p_temp = frame_per_sdu;
            }
        }
    }

    p_pac = ble_pacs_get_pac(AUDIO_DIRECTION_SOURCE, BLE_PACS_SOURCE_PAC_1);

    for (i = 0; i < p_pac->num_of_record; i++) {

        p_temp = ble_pacs_get_ltv_value_from_codec_specific_capabilities(CODEC_CAPABILITY_TYPE_AUDIO_CHANNEL_COUNTS,
            p_pac->record[i].codec_specific_capabilities_length, p_pac->record[i].codec_specific_capabilities);

        if (p_temp != NULL) {
            channel_count = channel_num;
            if (channel_count != *p_temp) {
                *p_temp = channel_count;
            }
        }

        p_temp = ble_pacs_get_ltv_value_from_codec_specific_capabilities(CODEC_CAPABILITY_TYPE_SUPPORTED_MAX_CODEC_FRAMES_PER_SDU,
            p_pac->record[i].codec_specific_capabilities_length, p_pac->record[i].codec_specific_capabilities);

        if (p_temp != NULL) {
            frame_per_sdu = channel_num;
            if (frame_per_sdu != *p_temp) {
                *p_temp = frame_per_sdu;
            }
        }
    }

    if (channel_count || frame_per_sdu) {
        for (i = 0; i < ble_pacs_get_sink_pac_number(); i++) {
            ble_pacs_send_sink_pac_notify(BT_HANDLE_INVALID, i);
        }

        for (i = 0; i < ble_pacs_get_source_pac_number(); i++) {
            ble_pacs_send_source_pac_notify(BT_HANDLE_INVALID, i);
        }
        return BT_STATUS_SUCCESS;
    }

    return BT_STATUS_FAIL;
}

void ble_pacs_init_parameter(void)
{
    #ifndef AIR_LE_AUDIO_HEADSET_ENABLE
    audio_channel_t channel = ami_get_audio_channel();
    #endif

#ifdef AIR_LE_AUDIO_HEADSET_ENABLE
    ble_pacs_set_audio_location(AUDIO_DIRECTION_SINK, AUDIO_LOCATION_FRONT_LEFT | AUDIO_LOCATION_FRONT_RIGHT);

    ble_pacs_set_audio_location(AUDIO_DIRECTION_SOURCE, AUDIO_LOCATION_FRONT_LEFT);

#else
    ble_pacs_set_audio_location(AUDIO_DIRECTION_SINK, (channel == AUDIO_CHANNEL_NONE) ? AUDIO_LOCATION_NONE : (channel == AUDIO_CHANNEL_R) ? AUDIO_LOCATION_FRONT_RIGHT : AUDIO_LOCATION_FRONT_LEFT);

    ble_pacs_set_audio_location(AUDIO_DIRECTION_SOURCE, (channel == AUDIO_CHANNEL_NONE) ? AUDIO_LOCATION_NONE : (channel == AUDIO_CHANNEL_R) ? AUDIO_LOCATION_FRONT_RIGHT : AUDIO_LOCATION_FRONT_LEFT);
#endif

    ble_pacs_set_available_audio_contexts(AUDIO_CONTENT_TYPE_ALL,
                                          AUDIO_CONTENT_TYPE_CONVERSATIONAL | AUDIO_CONTENT_TYPE_UNSPECIFIED | AUDIO_CONTENT_TYPE_RINGTONE);

    ble_pacs_set_supported_audio_contexts(AUDIO_CONTENT_TYPE_ALL,
                                          AUDIO_CONTENT_TYPE_ALL);

    ble_pacs_set_pac(AUDIO_DIRECTION_SINK, BLE_PACS_SINK_PAC_1, &g_pacs_sink_pac_1);

    ble_pacs_set_pac(AUDIO_DIRECTION_SOURCE, BLE_PACS_SOURCE_PAC_1, &g_pacs_source_pac_1);
}

bool ble_pacs_set_cccd_handler(bt_handle_t conn_handle, uint16_t attr_handle, uint16_t value)
{
    if (attr_handle < PACS_END_HANDLE && attr_handle > PACS_START_HANDLE) {
        ble_pacs_set_cccd_with_att_handle(conn_handle, attr_handle, value);
        return true;
    }
    return false;
}

bt_le_audio_cccd_record_t* ble_pacs_get_cccd_handler(bt_handle_t conn_handle, uint32_t *num)
{
    if (num == NULL) {
        return NULL;
    }

    return ble_pacs_get_cccd_with_att_handle(conn_handle, num);
}

