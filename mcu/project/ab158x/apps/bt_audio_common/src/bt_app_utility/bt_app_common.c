/* Copyright Statement:
 *
 * (C) 2017  Airoha Technology Corp. All rights reserved.
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
#include "timers.h"
#include "apps_events_event_group.h"
#include "bt_hci.h"
#include "bt_uuid.h"

#include "bt_gap_le.h"
#include "bt_gattc.h"
#include "bt_gatts.h"
#include "bt_spp.h"
#include "bt_app_common.h"
#include "bt_callback_manager.h"
#include "bt_gattc_discovery.h"
#include "bt_gatts_service.h"
#include "bt_device_manager_le.h"
#include "bt_device_manager.h"
#include "ui_shell_manager.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"
#ifdef __BLE_FMP__
#include "ble_ias.h"
#include "ble_find_me_server.h"
#endif
#ifdef MTK_BLE_BAS
#include "ble_bas_app.h"
#endif
//#include "ble_dogp_adp_service.h"
#include "syslog.h"
#include "hal_trng.h"
#ifdef MTK_NVDM_ENABLE
#include "nvdm.h"
#include "nvdm_id_list.h"
#include "nvkey_id_list.h"
#include "nvkey.h"
#endif
#include "bt_di.h"
#include "bt_sink_srv.h"
#include "bt_gap_le_service.h"
#include "bt_app_common_at_cmd.h"
#include "bt_customer_config.h"
#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#endif
#include "mbedtls/md5.h"
#include "bt_init.h"

#ifdef MTK_PORT_SERVICE_BT_ENABLE
#include "ble_air_interface.h"
#include "spp_air_interface.h"
#endif

#ifdef MTK_AIRUPDATE_ENABLE
#include "airupdate_interface.h"
#endif

#if defined(SUPPORT_ROLE_HANDOVER_SERVICE)
#include "bt_role_handover.h"
static bool bt_app_adv_restart_flag = false;
#endif

#if defined(MTK_AWS_MCE_ENABLE)
#include "bt_aws_mce_report.h"
#include "bt_aws_mce_srv.h"
#endif

#ifdef MTK_IAP2_PROFILE_ENABLE
#include "iAP2.h"

#ifndef MTK_IAP2_VIA_MUX_ENABLE
#include "serial_port_iap2.h"
#else
#include "mux_iap2.h"
#endif
#endif

#ifdef MTK_AWS_MCE_ROLE_RECOVERY_ENABLE
#include "bt_aws_mce_role_recovery.h"
#endif

#ifdef AIR_APP_MULTI_VA
#include "multi_va_manager.h"
#include "multi_ble_adv_manager.h"
#endif
#include "bt_connection_manager_internal.h"
#include "hal_nvic.h"

#ifdef AIR_LE_AUDIO_ENABLE
#include "bt_gap_le_audio.h"
#include "app_le_audio.h"
#include "bt_sink_srv_le_cap_stream.h"
#include "bt_le_audio_sink.h"
#endif

#if defined(MTK_RACE_CMD_ENABLE)
#include "apps_race_cmd_event.h"
#endif
#if defined(MTK_AWS_MCE_ENABLE)
#include "apps_aws_sync_event.h"
#endif
#ifdef AIR_SWIFT_PAIR_ENABLE
#ifdef AIR_CUST_PAIR_ENABLE
#include "cust_pair.h"
#elif defined(APP_SWIFT_PAIR_LE_EDR_SECURE_MODE)
#include "app_swift_cust_pair.h"
#endif
#endif
#include "apps_control_touch_key_status.h"
#include "bt_device_manager_link_record.h"
#include "bt_gatts_service.h"
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
#include "bt_ull_service.h"
#include "app_ull_idle_activity.h"
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
#include "bt_ull_le_service.h"
#endif
#endif
#ifdef AIR_LE_OTP_ENABLE
#include "ble_otp_discovery.h"
#endif
#ifdef AIR_HEARTHROUGH_MAIN_ENABLE
#include "app_hear_through_adv.h"
#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */

#ifdef AIR_BLE_HRS_ENABLE
#include "ble_app_hrs.h"
#endif

#ifdef AIR_GATT_SRV_CLIENT_ENABLE
#include "bt_gatt_service_client.h"
#endif

log_create_module(BT_APP, PRINT_LEVEL_INFO);
void bt_app_common_reset_context(void);

#define BT_APP_COMMON_GATT_MTU 242

static uint16_t g_conn_interval;
static bool s_waiting_conn_interval;

#define BT_APP_COMMON_UPDATE_INTERVAL_FAIL_RETRY_MAX   3

#define BT_APP_COMMON_LOCAL_KEY_MAX                    2
typedef struct {
    bt_handle_t     retry_handle;
    uint8_t         update_interval_fail_retry_count;
} bt_app_common_context_t;

static bt_app_common_context_t app_common_context = {0};

extern bool bt_app_ctkd_enable;

#ifdef MTK_AWS_MCE_ENABLE
#define BT_APP_COMMON_APPEARANCE  0x0941
#else
#define BT_APP_COMMON_APPEARANCE  0x0942
#endif

static bt_gap_le_smp_pairing_config_t pairing_config_deafult = {//MITM, Bond, OOB
    .maximum_encryption_key_size = 16,
    .io_capability = BT_GAP_LE_SMP_NO_INPUT_NO_OUTPUT,
    .auth_req = BT_GAP_LE_SMP_AUTH_REQ_BONDING,
    .oob_data_flag = BT_GAP_LE_SMP_OOB_DATA_NOT_PRESENTED,
    .initiator_key_distribution = BT_GAP_LE_SMP_KEY_DISTRIBUTE_ENCKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_IDKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_SIGN,
    .responder_key_distribution = BT_GAP_LE_SMP_KEY_DISTRIBUTE_ENCKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_IDKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_SIGN,
};

#ifdef AIR_SPEAKER_ENABLE
static bt_gap_le_smp_pairing_config_t pairing_ass_config = {//MITM, Bond, OOB
    .maximum_encryption_key_size = 16,
    .io_capability = BT_GAP_LE_SMP_NO_INPUT_NO_OUTPUT,
    .auth_req = BT_GAP_LE_SMP_AUTH_REQ_NO_BONDING,//BT_GAP_LE_SMP_AUTH_REQ_BONDING
    .oob_data_flag = BT_GAP_LE_SMP_OOB_DATA_PRESENTED,
    .initiator_key_distribution = BT_GAP_LE_SMP_KEY_DISTRIBUTE_ENCKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_IDKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_SIGN,
    .responder_key_distribution = BT_GAP_LE_SMP_KEY_DISTRIBUTE_ENCKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_IDKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_SIGN,
};
#endif

#if defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
#if defined(AIR_LE_AUDIO_DUALMODE_ENABLE)
static bt_gap_le_smp_pairing_config_t pairing_config_le_audio = {//MITM, Bond, OOB
    .maximum_encryption_key_size = 16,
    .io_capability = BT_GAP_LE_SMP_NO_INPUT_NO_OUTPUT,
    .auth_req = BT_GAP_LE_SMP_AUTH_REQ_BONDING | BT_GAP_LE_SMP_AUTH_REQ_SECURE_CONNECTION,
    .oob_data_flag = BT_GAP_LE_SMP_OOB_DATA_NOT_PRESENTED,
    .initiator_key_distribution = BT_GAP_LE_SMP_KEY_DISTRIBUTE_ENCKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_IDKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_SIGN | BT_GAP_LE_SMP_KEY_DISTRIBUTE_LINK,
#if defined(APP_BT_SWIFT_PAIR_LE_AUDIO_ENABLE) || defined (AIR_BT_FAST_PAIR_LE_AUDIO_ENABLE) || defined (AIR_LE_AUDIO_USE_DIRECT_ADV_TO_ACTIVE_RECONNECT)
    .responder_key_distribution = BT_GAP_LE_SMP_KEY_DISTRIBUTE_ENCKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_IDKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_SIGN | BT_GAP_LE_SMP_KEY_DISTRIBUTE_LINK,
#else
    .responder_key_distribution = BT_GAP_LE_SMP_KEY_DISTRIBUTE_ENCKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_SIGN | BT_GAP_LE_SMP_KEY_DISTRIBUTE_LINK,
#endif
};

#else
static bt_gap_le_smp_pairing_config_t pairing_config_le_audio = {//MITM, Bond, OOB
    .maximum_encryption_key_size = 16,
    .io_capability = BT_GAP_LE_SMP_NO_INPUT_NO_OUTPUT,
    .auth_req = BT_GAP_LE_SMP_AUTH_REQ_BONDING | BT_GAP_LE_SMP_AUTH_REQ_SECURE_CONNECTION,
    .oob_data_flag = BT_GAP_LE_SMP_OOB_DATA_NOT_PRESENTED,
    .initiator_key_distribution = BT_GAP_LE_SMP_KEY_DISTRIBUTE_ENCKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_IDKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_SIGN,
    .responder_key_distribution = BT_GAP_LE_SMP_KEY_DISTRIBUTE_ENCKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_SIGN,

};
#endif
#ifdef MTK_AWS_MCE_ENABLE
static bt_gap_le_smp_pairing_config_t pairing_config_le_audio_secondary = {//MITM, Bond, OOB
    .maximum_encryption_key_size = 16,
    .io_capability = BT_GAP_LE_SMP_NO_INPUT_NO_OUTPUT,
    .auth_req = BT_GAP_LE_SMP_AUTH_REQ_BONDING | BT_GAP_LE_SMP_AUTH_REQ_SECURE_CONNECTION,
    .oob_data_flag = BT_GAP_LE_SMP_OOB_DATA_NOT_PRESENTED,
    .initiator_key_distribution = BT_GAP_LE_SMP_KEY_DISTRIBUTE_ENCKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_IDKEY,
    .responder_key_distribution = BT_GAP_LE_SMP_KEY_DISTRIBUTE_ENCKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_IDKEY,
};

#endif
#endif

#ifdef AIR_SWIFT_PAIR_ENABLE
#ifdef AIR_CUST_PAIR_ENABLE
static bt_gap_le_smp_pairing_config_t pairing_config_cust_ext_pair = {//MITM, Bond, OOB
    .maximum_encryption_key_size = 16,
    .io_capability = BT_GAP_LE_SMP_DISPLAY_ONLY,
    .auth_req = BT_GAP_LE_SMP_AUTH_REQ_BONDING | BT_GAP_LE_SMP_AUTH_REQ_SECURE_CONNECTION | BT_GAP_LE_SMP_AUTH_REQ_MITM,
    .oob_data_flag = BT_GAP_LE_SMP_OOB_DATA_NOT_PRESENTED,
    .initiator_key_distribution = BT_GAP_LE_SMP_KEY_DISTRIBUTE_ENCKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_IDKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_SIGN,
    .responder_key_distribution = BT_GAP_LE_SMP_KEY_DISTRIBUTE_ENCKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_IDKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_SIGN,
};

static bt_gap_le_smp_pairing_config_t pairing_config_cust_std_pair = {
    .maximum_encryption_key_size = 16,
    .io_capability = BT_GAP_LE_SMP_NO_INPUT_NO_OUTPUT,
    .auth_req = BT_GAP_LE_SMP_AUTH_REQ_BONDING | BT_GAP_LE_SMP_AUTH_REQ_SECURE_CONNECTION,
    .oob_data_flag = BT_GAP_LE_SMP_OOB_DATA_NOT_PRESENTED,
    .initiator_key_distribution = BT_GAP_LE_SMP_KEY_DISTRIBUTE_ENCKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_IDKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_SIGN | BT_GAP_LE_SMP_KEY_DISTRIBUTE_LINK,
    .responder_key_distribution = BT_GAP_LE_SMP_KEY_DISTRIBUTE_ENCKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_IDKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_SIGN | BT_GAP_LE_SMP_KEY_DISTRIBUTE_LINK,
};
#elif defined(APP_SWIFT_PAIR_LE_EDR_SECURE_MODE)
static bt_gap_le_smp_pairing_config_t pairing_config_swift_pair = {
    .maximum_encryption_key_size = 16,
    .io_capability = BT_GAP_LE_SMP_NO_INPUT_NO_OUTPUT,
    .auth_req = BT_GAP_LE_SMP_AUTH_REQ_BONDING | BT_GAP_LE_SMP_AUTH_REQ_SECURE_CONNECTION,
    .oob_data_flag = BT_GAP_LE_SMP_OOB_DATA_NOT_PRESENTED,
    .initiator_key_distribution = BT_GAP_LE_SMP_KEY_DISTRIBUTE_ENCKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_IDKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_SIGN | BT_GAP_LE_SMP_KEY_DISTRIBUTE_LINK,
    .responder_key_distribution = BT_GAP_LE_SMP_KEY_DISTRIBUTE_ENCKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_IDKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_SIGN | BT_GAP_LE_SMP_KEY_DISTRIBUTE_LINK,
};
#endif
#endif

static bt_gap_le_smp_pairing_config_t pairing_config_ctkd = {//MITM, Bond, OOB
    .maximum_encryption_key_size = 16,
    .io_capability = BT_GAP_LE_SMP_DISPLAY_YES_NO,
    .auth_req = BT_GAP_LE_SMP_AUTH_REQ_BONDING | BT_GAP_LE_SMP_AUTH_REQ_SECURE_CONNECTION | BT_GAP_LE_SMP_AUTH_REQ_MITM,
    .oob_data_flag = BT_GAP_LE_SMP_OOB_DATA_NOT_PRESENTED,
    .initiator_key_distribution = BT_GAP_LE_SMP_KEY_DISTRIBUTE_ENCKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_IDKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_SIGN | BT_GAP_LE_SMP_KEY_DISTRIBUTE_LINK,
    .responder_key_distribution = BT_GAP_LE_SMP_KEY_DISTRIBUTE_ENCKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_IDKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_SIGN | BT_GAP_LE_SMP_KEY_DISTRIBUTE_LINK,
};

static bt_gap_le_local_key_t local_key = {
    .encryption_info.ltk = { 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc8, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf },
    .master_id.ediv = 0x1005,
    .master_id.rand = { 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7 },
    .identity_info.irk = { 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf },
    .signing_info.csrk = { 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf }
};

typedef struct {
    bt_gap_le_srv_link_t    link_type;
    bt_gap_le_local_key_t   local_key;
} bt_app_common_local_key_config_t;

static bt_app_common_local_key_config_t g_specific_local_key[BT_APP_COMMON_LOCAL_KEY_MAX] = {
    {
        .link_type = BT_GAP_LE_SRV_LINK_TYPE_SWIFT_PAIR,
        .local_key.encryption_info.ltk = { 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc8, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf },
        .local_key.master_id.ediv = 0x1005,
        .local_key.master_id.rand = { 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7 },
        .local_key.identity_info.irk = { 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf },
        .local_key.signing_info.csrk = { 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf }
    },
    {
        .link_type = BT_GAP_LE_SRV_LINK_TYPE_FAST_PAIR,
        .local_key.encryption_info.ltk = { 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc8, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf },
        .local_key.master_id.ediv = 0x1005,
        .local_key.master_id.rand = { 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7 },
        .local_key.identity_info.irk = { 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf },
        .local_key.signing_info.csrk = { 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf }
    },
};

static bool sc_only = false;


//bt_bd_addr_t bt_local_random_addr = {0xC5, 0x01, 0x02, 0x03, 0x04, 0xC5};
static bt_bd_addr_t bt_local_random_addr;
static bt_bd_addr_t bt_aws_agent_random_addr;

//SDP part.
#define BT_SPP_STANDARD_UUID    0x00,0x00,0xFF,0x01,0x00,0x00,0x10,0x00,0x80,0x00,0x00,0x80,0x5F,0x9B,0x34,0xFF

static bt_status_t bt_app_common_gap_param_init(void);

static const uint8_t bt_spp_app_service_class_id[] = {
    BT_SPP_SDP_ATTRIBUTE_UUID_LENGTH,
    BT_SPP_SDP_ATTRIBUTE_UUID(BT_SPP_STANDARD_UUID)
};

static const uint8_t bt_spp_app_protocol_descriptor_list[] = {
    BT_SPP_SDP_ATTRIBUTE_PROTOCOL_DESCRIPTOR(BT_SPP_SERVER_ID_START + 2)
};

static const uint8_t bt_spp_app_browse_group[] = {
    BT_SPP_SDP_ATTRIBUTE_PUBLIC_BROWSE_GROUP
};

static const uint8_t bt_spp_app_language[] = {
    BT_SPP_SDP_ATTRIBUTE_LANGUAGE
};

static const uint8_t bt_spp_app_service_name[] = {
    BT_SPP_SDP_ATTRIBUTE_SIZE_OF_SERVICE_NAME(9),
    'B', 'T', 'N', 'O', 'T', 'I', 'F', 'Y', 'R'
};

static const bt_sdps_attribute_t bt_spp_app_sdp_attributes[] = {
    /* Service Class ID List attribute */
    BT_SPP_SDP_ATTRIBUTE_SERVICE_CLASS_ID_LIST(bt_spp_app_service_class_id),
    /* Protocol Descriptor List attribute */
    BT_SPP_SDP_ATTRIBUTE_PROTOCOL_DESC_LIST(bt_spp_app_protocol_descriptor_list),
    /* Public Browse Group Service */
    BT_SPP_SDP_ATTRIBUTE_BROWSE_GROUP_LIST(bt_spp_app_browse_group),
    /* Language Base ID List attribute */
    BT_SPP_SDP_ATTRIBUTE_LANGUAGE_BASE_LIST(bt_spp_app_language),
    /* Serial Port Profile Service Name */
    BT_SPP_SDP_ATTRIBUTE_SERVICE_NAME(bt_spp_app_service_name)
};


static const bt_sdps_record_t bt_spp_app_sdp_record = {
    .attribute_list_length = sizeof(bt_spp_app_sdp_attributes),
    .attribute_list = bt_spp_app_sdp_attributes,
};

#ifdef AIR_MUX_BT_HID_ENABLE
/* PnP part */
#define MY_BT_DI_VENDOR_ID         0x0E8D   // The vendor ID definition.
#define MY_BT_DI_PRODUCT_ID        0x0900   // A value defined for product ID and it should be managed and assigned by the vendor.
#define MY_BT_DI_VERSION           0x0100   // A value defined for the version number, in this example it's 1.0.0.
#else
/* PnP part */
#define MY_BT_DI_VENDOR_ID         0x0094   // The vendor ID definition.
#define MY_BT_DI_PRODUCT_ID        0x0004   // A value defined for product ID and it should be managed and assigned by the vendor.
#define MY_BT_DI_VERSION           0x0100   // A value defined for the version number, in this example it's 1.0.0.
#endif

static const uint8_t bt_di_service_class_id_list[] = {
    BT_DI_SERVICE_CLASS_ID_LIST
};

static const uint8_t bt_di_specification_id[] = {
    BT_DI_SPECIFICATION_ID
};

static const uint8_t bt_di_vendor_id[] = {
    BT_DI_VENDOR_ID(MY_BT_DI_VENDOR_ID)    // The device ID.
};

static const uint8_t bt_di_product_id[] = {
    BT_DI_PRODUCT_ID(MY_BT_DI_PRODUCT_ID)    // The product ID managed by the vendor to distinguish itself from different products.
};

static const uint8_t bt_di_version[] = {
    BT_DI_VERSION(MY_BT_DI_VERSION)    // The version number assigned by the vendor to identify the device's release number.
};

static const uint8_t bt_di_primary_record[] = {
    BT_DI_PRIMARY_RECORD(BT_DI_PRIMARY_RECORD_TRUE)    // It should be set with #BT_DI_PRIMARY_RECORD_TRUE, when there is only one Device ID service record in the service.
};

static const uint8_t bt_di_vendor_id_source[] = {
    BT_DI_VENDOR_ID_SOURCE(BT_DI_VENDOR_ID_SOURCE_SIG_ASSIGNED)
};

static const bt_sdps_attribute_t bt_di_sdp_attributes[] = {
    BT_SDP_ATTRIBUTE(BT_SDP_ATTRIBUTE_ID_SERVICE_CLASS_ID_LIST, bt_di_service_class_id_list),
    BT_SDP_ATTRIBUTE(BT_DI_SDP_ATTRIBUTE_SPECIFICATION_ID, bt_di_specification_id),
    BT_SDP_ATTRIBUTE(BT_DI_SDP_ATTRIBUTE_VENDOR_ID, bt_di_vendor_id),
    BT_SDP_ATTRIBUTE(BT_DI_SDP_ATTRIBUTE_PRODUCT_ID, bt_di_product_id),
    BT_SDP_ATTRIBUTE(BT_DI_SDP_ATTRIBUTE_VERSION, bt_di_version),
    BT_SDP_ATTRIBUTE(BT_DI_SDP_ATTRIBUTE_PRIMARY_RECORD, bt_di_primary_record),
    BT_SDP_ATTRIBUTE(BT_DI_SDP_ATTRIBUTE_VENDOR_ID_SOURCE, bt_di_vendor_id_source)
};

static const bt_sdps_record_t bt_di_sdp_record = {
    .attribute_list_length = sizeof(bt_di_sdp_attributes),
    .attribute_list = bt_di_sdp_attributes,
};

static bt_handle_t g_le_connection_handle = 0;
static bool g_bt_cm_power_standby_flag = false;

BT_PACKED(
typedef struct {
    bt_handle_t       connection_handle;
    bt_role_t         role;
    bt_addr_t         peer_addr;
    uint8_t           gatts_wait_att_rx_opcode;//use to wait handle value confirmation.
}) app_ble_connection_cb_t;

app_ble_connection_cb_t connection_cb[BT_CONNECTION_MAX] = {{0}};

#if defined(MTK_AWS_MCE_ENABLE) && !defined(AIR_SPEAKER_ENABLE)

#define BT_APP_COMMON_EVENT_SYNC_RANDOM_ADDR        0xff
#define BT_APP_COMMON_EVENT_SYNC_LOCAL_KEY          0xfe
#define BT_APP_COMMON_EVENT_SYNC_LE_AUDIO_INFO      0xfd
#define BT_APP_COMMON_EVENT_SYNC_SIRK               0xfc

#define BT_APP_COMMON_EVENT_SYNC_BOND_INFO_ADD      BT_DEVICE_MANAGER_LE_BONDED_ADD
#define BT_APP_COMMON_EVENT_SYNC_BOND_INFO_REMOVE   BT_DEVICE_MANAGER_LE_BONDED_REMOVE
#define BT_APP_COMMON_EVENT_SYNC_BOND_INFO_CLEAR    BT_DEVICE_MANAGER_LE_BONDED_CLEAR
typedef uint8_t bt_app_common_event_sync_t;

static void bt_app_common_sync_bonded_info(bt_bd_addr_t *addr_list, bt_addr_type_t *type_list, uint8_t count, bt_device_manager_le_bonded_event_t event);
static void bt_app_common_aws_data_received_ind_handler(bt_aws_mce_report_info_t *event);
#endif
void bt_app_common_le_disconnect(bt_handle_t handle);
extern void bt_app_common_at_cmd_print_report(char *string);

#ifdef AIR_LE_AUDIO_ENABLE
extern bool app_le_audio_dhss_read_local_le_addr(void);
extern void app_le_audio_dhss_set_peer_le_addr(bt_addr_type_t type, uint8_t *addr);
extern bool bt_le_audio_sink_load_cccd(bt_handle_t handle);
extern bool bt_le_audio_sink_delete_cccd(bt_addr_t *target_addr);
extern bt_status_t ble_csis_get_sirk(bt_key_t *sirk);
extern void ble_csis_write_nvkey_sirk(bt_key_t *sirk);

bt_status_t bt_app_common_remove_ltk()
{
    pairing_config_le_audio.auth_req = BT_GAP_LE_SMP_AUTH_REQ_NO_BONDING;
    pairing_config_le_audio.initiator_key_distribution = 0x00;
    pairing_config_le_audio.responder_key_distribution = 0x00;
    return BT_STATUS_SUCCESS;
}

bt_status_t bt_app_common_reset_ltk()
{
    pairing_config_le_audio.auth_req = BT_GAP_LE_SMP_AUTH_REQ_BONDING;
    pairing_config_le_audio.initiator_key_distribution = BT_GAP_LE_SMP_KEY_DISTRIBUTE_ENCKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_IDKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_SIGN;
    pairing_config_le_audio.responder_key_distribution = BT_GAP_LE_SMP_KEY_DISTRIBUTE_ENCKEY |  BT_GAP_LE_SMP_KEY_DISTRIBUTE_SIGN;
    return BT_STATUS_SUCCESS;
}

bt_status_t bt_app_common_set_pairing_config(bool is_bond)
{
    bt_gap_le_smp_pairing_config_t pairing_config2 = {//MITM, Bond, OOB
        .maximum_encryption_key_size = 16,
        .io_capability = BT_GAP_LE_SMP_NO_INPUT_NO_OUTPUT,
#if (defined(AIR_LE_AUDIO_ENABLE))
        .auth_req = BT_GAP_LE_SMP_AUTH_REQ_BONDING | BT_GAP_LE_SMP_AUTH_REQ_SECURE_CONNECTION,
#else
        .auth_req = BT_GAP_LE_SMP_AUTH_REQ_BONDING,
#endif
        .oob_data_flag = BT_GAP_LE_SMP_OOB_DATA_NOT_PRESENTED,
        .initiator_key_distribution = BT_GAP_LE_SMP_KEY_DISTRIBUTE_ENCKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_IDKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_SIGN,
#if (defined(AIR_LE_AUDIO_ENABLE))
        .responder_key_distribution = BT_GAP_LE_SMP_KEY_DISTRIBUTE_ENCKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_SIGN,
#else
        .responder_key_distribution = BT_GAP_LE_SMP_KEY_DISTRIBUTE_ENCKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_IDKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_SIGN,
#endif
    };

    if (!is_bond) {
#if (defined(AIR_LE_AUDIO_ENABLE))
        pairing_config2.auth_req = BT_GAP_LE_SMP_AUTH_REQ_SECURE_CONNECTION;
        pairing_config2.oob_data_flag = 0;
        pairing_config2.initiator_key_distribution = 0;
        pairing_config2.responder_key_distribution = 0;
#endif
    }

    bt_device_manager_le_gap_set_pairing_configuration(&pairing_config2);

    return BT_STATUS_SUCCESS;
}
#endif

void bt_app_common_add_connection_info(void *buff)
{
    uint8_t i;

    bt_gap_le_connection_ind_t *connection_ind = (bt_gap_le_connection_ind_t *)buff;
    for (i = 0; i < BT_CONNECTION_MAX; i++) {
        if (connection_cb[i].connection_handle == 0) {
            connection_cb[i].connection_handle = connection_ind->connection_handle;
            connection_cb[i].role = connection_ind->role;
            connection_cb[i].peer_addr = connection_ind->peer_addr;
            break;
        }
    }
    if (i == BT_CONNECTION_MAX) {
        LOG_MSGID_I(BT_APP, "Reach maximum connection\n", 0);
    }
}

bool bt_app_common_is_connection_info_list_empty()
{
    uint8_t i;
    for (i = 0; i < BT_CONNECTION_MAX ; i++) {
        if (connection_cb[i].connection_handle) {
            return false;
        }
    }
    return true;
}

void bt_app_common_dump_connection_info_list()
{
    uint8_t i;
    for (i = 0; i < BT_CONNECTION_MAX ; i++) {
        if (connection_cb[i].connection_handle) {
            LOG_MSGID_I(BT_APP, "Connection Info[%d]: [Addr type: %x]] %02x-%02x-%02x-%02x-%02x-%02x [0x%04x] [Slave? %d]", 10,
                        i, connection_cb[i].peer_addr.type,
                        connection_cb[i].peer_addr.addr[0], connection_cb[i].peer_addr.addr[1],
                        connection_cb[i].peer_addr.addr[2], connection_cb[i].peer_addr.addr[3],
                        connection_cb[i].peer_addr.addr[4], connection_cb[i].peer_addr.addr[5],
                        connection_cb[i].connection_handle, connection_cb[i].role);
        }
    }
}

void bt_app_common_delete_connection_info(void *buff)
{
    bt_hci_evt_disconnect_complete_t *disconnect_complete;
    uint8_t i;

    disconnect_complete = (bt_hci_evt_disconnect_complete_t *) buff;
    for (i = 0; i < BT_CONNECTION_MAX ; i++) {
        if (disconnect_complete->connection_handle == connection_cb[i].connection_handle) {
            connection_cb[i].connection_handle = 0;//clear conneciton info.
            connection_cb[i].gatts_wait_att_rx_opcode = 0;
            break;
        }
    }
    if (i == BT_CONNECTION_MAX) {
        LOG_MSGID_I(BT_APP, "Don't know connection info for deleting.\n", 0);
    }
}

app_ble_connection_cb_t *bt_app_common_find_conneciton_info_by_handle(bt_handle_t target_handle)
{
    uint8_t i;
    for (i = 0; i < BT_CONNECTION_MAX; i++) {
        if (target_handle == connection_cb[i].connection_handle) {
            return &(connection_cb[i]);
        }
    }
    return NULL;
}

bt_handle_t bt_app_common_get_first_conneciton_handle(void)
{
    uint8_t i;
    for (i = 0; i < BT_CONNECTION_MAX; i++) {
        if (connection_cb[i].connection_handle) {
            return connection_cb[i].connection_handle;
        }
    }
    return 0;
}

#define BT_BLE_ADV_TIMER_TIMEROUT_IN_MS    (30000)
static TimerHandle_t bt_ble_adv_timer;

const bt_bd_addr_t *bt_app_common_get_aws_agent_random_address(void)
{
    return (const bt_bd_addr_t *)(&bt_aws_agent_random_addr);
}

const bt_bd_addr_t *bt_app_common_get_local_random_addr(void)
{
    return (const bt_bd_addr_t *)(&bt_local_random_addr);
}

bt_status_t bt_app_common_store_local_random_address(bt_bd_addr_t *addr)
{
    memcpy(&bt_local_random_addr, addr, sizeof(bt_bd_addr_t));

#ifdef MTK_NVDM_ENABLE
    int8_t i;
    uint8_t buffer[20] = {0};
    nvkey_status_t status = NVKEY_STATUS_ERROR;

    /* save address to NVDM */
    for (i = 0; i < 6; ++i) {
        snprintf((char *)buffer + 2 * i, sizeof(buffer) - 2 * i, "%02X", (*addr)[i]);
    }
    LOG_MSGID_I(BT_APP, "[BT]address to write:%02x-%02x-%02x-%02x-%02x-%02x, len:%d \r\n", 7,
                (*addr)[5], (*addr)[4], (*addr)[3], (*addr)[2], (*addr)[1], (*addr)[0],
                strlen((char *)buffer));
    status = nvkey_write_data(NVID_APP_LE_RANDOM_ADDRESS, buffer, strlen((char *)buffer));
    if (NVKEY_STATUS_OK != status) {
        LOG_MSGID_I(BT_APP, "[BT]Failed to store address!\r\n", 0);
        return BT_STATUS_FAIL;
    } else {
        LOG_MSGID_I(BT_APP, "[BT]Successfully store address to NVDM [%02X:%02X:%02X:%02X:%02X:%02X]\r\n", 6, addr[5],
                    (*addr)[4], (*addr)[3], (*addr)[2], (*addr)[1], (*addr)[0]);
    }
#endif
    return BT_STATUS_SUCCESS;

}

void bt_app_common_generate_random_key(uint8_t *key, uint8_t size)
{
    uint32_t random_seed;
    hal_trng_status_t ret = hal_trng_init();
    int8_t i;
    if (HAL_TRNG_STATUS_OK != ret) {
        LOG_MSGID_I(BT_APP, "[BT]generate_random_key--error 1\r\n", 0);
    }
    for (i = 0; i < 30; ++i) {
        ret = hal_trng_get_generated_random_number(&random_seed);
        if (HAL_TRNG_STATUS_OK != ret) {
            LOG_MSGID_I(BT_APP, "[BT]generate_random_key--error 2\r\n", 0);
        }
    }
    for (i = 0; i < (size + 3) / 4; i++) {
        ret = hal_trng_get_generated_random_number(&random_seed);
        if (HAL_TRNG_STATUS_OK != ret) {
            LOG_MSGID_I(BT_APP, "[BT]generate_random_key--error 3\r\n", 0);
        }
        key[0 + 4 * i] = random_seed & 0xFF;
        key[1 + 4 * i] = (random_seed >> 8) & 0xFF;
        key[2 + 4 * i] = (random_seed >> 16) & 0xFF;
        key[3 + 4 * i] = (random_seed >> 24) & 0xFF;
    }

    hal_trng_deinit();
}

bt_status_t bt_app_common_store_local_irk(const uint8_t *buffer, uint8_t size)
{
    if (size == BT_KEY_SIZE) {
        LOG_MSGID_I(BT_APP, "[BT]store local irk:%x%x%x%x %x%x%x%x %x%x%x%x %x%x%x%x", 16,
                    buffer[0], buffer[1], buffer[2], buffer[3],
                    buffer[4], buffer[5], buffer[6], buffer[7],
                    buffer[8], buffer[9], buffer[10], buffer[11],
                    buffer[12], buffer[13], buffer[14], buffer[15]);
    }
#ifdef MTK_NVDM_ENABLE
    nvkey_status_t status = NVKEY_STATUS_ERROR;
    status = nvkey_write_data(NVID_APP_LE_LOCAL_IRK, buffer, size);
    if (NVKEY_STATUS_OK != status) {
        LOG_MSGID_I(BT_APP, "[BT]Failed to store local irk, 0x%x!", 1, status);
        return BT_STATUS_FAIL;
    } else {
        LOG_MSGID_I(BT_APP, "[BT]Successfully store local irk to NVDM", 0);
    }
#endif
    return BT_STATUS_SUCCESS;
}

void bt_app_common_generate_non_resolvable_private_address(bt_bd_addr_t addr)
{
    int8_t i;
    uint32_t random_seed;
#ifdef HAL_TRNG_MODULE_ENABLED
    hal_trng_status_t ret = HAL_TRNG_STATUS_OK;

    ret = hal_trng_init();
    if (HAL_TRNG_STATUS_OK != ret) {
        LOG_MSGID_I(BT_APP, "[BT]generate_nRPA_address--error 1\r\n", 0);
    }
    for (i = 0; i < 30; ++i) {
        ret = hal_trng_get_generated_random_number(&random_seed);
        if (HAL_TRNG_STATUS_OK != ret) {
            LOG_MSGID_I(BT_APP, "[BT]generate_nRPA_address--error 2\r\n", 0);
        }
    }
    /* Randomly generate address */
    ret = hal_trng_get_generated_random_number(&random_seed);
    if (HAL_TRNG_STATUS_OK != ret) {
        LOG_MSGID_I(BT_APP, "[BT]generate_nRPA_address--error 3\r\n", 0);
    }
    addr[0] = random_seed & 0xFF;
    addr[1] = (random_seed >> 8) & 0xFF;
    addr[2] = (random_seed >> 16) & 0xFF;
    addr[3] = (random_seed >> 24) & 0xFF;
    ret = hal_trng_get_generated_random_number(&random_seed);
    if (HAL_TRNG_STATUS_OK != ret) {
        LOG_MSGID_I(BT_APP, "[BT]generate_nRPA_address--error 3\r\n", 0);
    }
    addr[4] = random_seed & 0xFF;
    addr[5] = (random_seed >> 8) & 0x3F;
    addr[5] = addr[5] | 0x30;
    hal_trng_deinit();
#endif
}
void bt_app_common_generate_random_address(bt_bd_addr_t addr)
{
    int8_t i;
    uint32_t random_seed;
#ifdef HAL_TRNG_MODULE_ENABLED
    hal_trng_status_t ret = HAL_TRNG_STATUS_OK;

    ret = hal_trng_init();
    if (HAL_TRNG_STATUS_OK != ret) {
        LOG_MSGID_I(BT_APP, "[BT]generate_random_address--error 1\r\n", 0);
    }
    for (i = 0; i < 30; ++i) {
        ret = hal_trng_get_generated_random_number(&random_seed);
        if (HAL_TRNG_STATUS_OK != ret) {
            LOG_MSGID_I(BT_APP, "[BT]generate_random_address--error 2\r\n", 0);
        }
    }
    /* Randomly generate address */
    ret = hal_trng_get_generated_random_number(&random_seed);
    if (HAL_TRNG_STATUS_OK != ret) {
        LOG_MSGID_I(BT_APP, "[BT]generate_random_address--error 3\r\n", 0);
    }
    addr[0] = random_seed & 0xFF;
    addr[1] = (random_seed >> 8) & 0xFF;
    addr[2] = (random_seed >> 16) & 0xFF;
    addr[3] = (random_seed >> 24) & 0xFF;
    ret = hal_trng_get_generated_random_number(&random_seed);
    if (HAL_TRNG_STATUS_OK != ret) {
        LOG_MSGID_I(BT_APP, "[BT]generate_random_address--error 3\r\n", 0);
    }
    addr[4] = random_seed & 0xFF;
    addr[5] = (random_seed >> 8) & 0xCF;
    addr[5] = addr[5] | 0xC0;
    hal_trng_deinit();
#else
#include "hal_gpt.h"
    uint32_t seed = 0;
    hal_gpt_status_t gpt_ret = HAL_GPT_STATUS_OK;
    gpt_ret = (int32_t)hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &seed);
    if (gpt_ret == HAL_GPT_STATUS_OK) {
        srand(seed);
        addr[0] = rand() & 0xFF;
        addr[1] = rand() & 0xFF;
        addr[2] = rand() & 0xFF;
        addr[3] = rand() & 0xFF;
        addr[4] = rand() & 0xFF;
        addr[5] = rand() & 0xFF;
    } else {
        addr[0] = 0x66;
        addr[1] = 0x77;
        addr[2] = 0xE2;
        addr[3] = 0xE1;
        addr[4] = 0x90;
        addr[5] = 0x00;
    }
#endif
}

static void bt_app_common_generate_local_address(bt_bd_addr_t addr)
{
#ifdef AIR_NVDM_ENABLE
    nvkey_status_t status = NVKEY_STATUS_ERROR;
#endif
    int8_t i;
    uint32_t size = 12;
    uint8_t buffer[18] = {0};
    uint8_t tmp_buf[3] = {0};
    bt_bd_addr_t tempaddr = {0};

    if (memcmp(addr, tempaddr, sizeof(bt_bd_addr_t)) == 0) {
#ifdef MTK_NVDM_ENABLE
        LOG_MSGID_I(BT_APP, "[BT]Try to read from NVDM! \r\n", 0);
        status = nvkey_read_data(NVID_APP_LE_RANDOM_ADDRESS, buffer, &size);
        if (NVKEY_STATUS_OK == status) {
            for (i = 0; i < 6; ++i) {
                tmp_buf[0] = buffer[2 * i];
                tmp_buf[1] = buffer[2 * i + 1];
                addr[i] = (uint8_t)strtoul((char *)tmp_buf, NULL, 16);
            }

            LOG_MSGID_I(BT_APP, "[BT]Read address from NVDM [%02X:%02X:%02X:%02X:%02X:%02X]\r\n", 6, addr[5],
                        addr[4], addr[3], addr[2], addr[1], addr[0]);
            return;
        } else
#endif
        {
#ifndef AIR_LE_AUDIO_ENABLE /* Gen local le rand addr when multi adv init */
            LOG_MSGID_I(BT_APP, "[BT]Failed to Read from NVDM:%d !\r\n", 1, status);
            bt_app_common_generate_random_address(addr);
#endif
        }
    }

#ifndef AIR_LE_AUDIO_ENABLE /* Store local le rand addr when multi adv init */
    bt_app_common_store_local_random_address((bt_bd_addr_t *)addr);
#endif

}

char g_le_device_name[BT_GAP_LE_MAX_DEVICE_NAME_LENGTH] = {0}; /**< Defines the name of the local device with '\0' ending. */


bt_status_t bt_app_common_generate_unique_id(const uint8_t *input,
                                             size_t ilen,
                                             uint8_t unique_id[16])
{
#ifndef MBEDTLS_MD5_C
    return BT_STATUS_FAIL;
#else
    if (!input) {
        return BT_STATUS_FAIL;
    }

    mbedtls_md5(input, ilen, unique_id);
    return BT_STATUS_SUCCESS;
#endif
}

#include "bt_connection_manager.h"

void bt_app_common_generate_device_name(void)
{
    bt_customer_config_get_ble_device_name(g_le_device_name);
    bt_gatts_service_set_gap_device_name((const uint8_t *)g_le_device_name, strlen(g_le_device_name));
    LOG_MSGID_I(BT_APP, "[LE] device name:\r\n", 0);
}


static bool s_ble_adv_disabled = false;
static bt_app_common_ble_adv_status_t bt_app_adv_ongoing = BT_APP_COMMON_BLE_ADV_STOPPED;

static bool ble_adv_increase_interval = false;

static bool bt_app_is_power_on  = false;

#define BT_APP_BLE_AIR_SERVICE_UUID                \
        {0x45, 0x4C, 0x42, 0x61, 0x68, 0x6F, 0x72, 0x69,               \
         0x41, 0x03, 0xAB, 0x2D, 0x4D, 0x49, 0x52, 0x50}

void bt_app_common_cmd_disable_advertising(void)
{
    s_ble_adv_disabled = true;
}

bt_status_t bt_app_common_generate_default_adv_data(bt_hci_le_set_ext_advertising_parameters_t *adv_param,
                                                    bt_gap_le_set_ext_advertising_data_t *adv_data,
                                                    bt_gap_le_set_ext_scan_response_data_t *scan_data,
                                                    uint8_t *unique_id,
                                                    uint8_t unique_id_len)
{

    uint8_t len = 0;
    bt_status_t ret = BT_STATUS_SUCCESS;

    if (adv_param) {
        /*
        uint8_t count = bt_device_manager_le_get_bonded_number();
        bt_bd_addr_t *peer_addr = (bt_bd_addr_t *)pvPortMalloc(count * sizeof(bt_bd_addr_t));
        if (peer_addr) {
            bt_device_manager_le_get_bonded_list(peer_addr, &count);
        } else {
            count = 0;
        }
        */
        adv_param->advertising_event_properties = BT_HCI_ADV_EVT_PROPERTIES_MASK_CONNECTABLE
                                                  | BT_HCI_ADV_EVT_PROPERTIES_MASK_SCANNABLE
                                                  | BT_HCI_ADV_EVT_PROPERTIES_MASK_LEGACY_PDU;
        adv_param->primary_advertising_interval_min = 0;
        adv_param->primary_advertising_interval_max = 0;
        adv_param->primary_advertising_channel_map = 0x07;
        adv_param->own_address_type = BT_ADDR_RANDOM;
        adv_param->advertising_filter_policy = 0;
        adv_param->advertising_tx_power = 0;
        adv_param->primary_advertising_phy = 0x01;
        adv_param->secondary_advertising_max_skip = 0;
        adv_param->secondary_advertising_phy = 0x01;
        adv_param->scan_request_notify_enable = 0;

        /*
        if (count) {
            int i;
            for (i = 0; i < count; i++) {
                bt_gap_le_bonding_info_t *bonded_info = NULL;
                bonded_info = bt_device_manager_le_get_bonding_info_by_addr((bt_bd_addr_t *)(peer_addr + i));
                if ((bonded_info) && (0xFF != bonded_info->identity_addr.address.type)) {
                    adv_param->own_address_type = BT_ADDR_RANDOM_IDENTITY;
                    memcpy(&(adv_param->peer_address), &(bonded_info->identity_addr.address), sizeof(bt_addr_t));
                    if (adv_param->peer_address.type >= 2) {
                        adv_param->peer_address.type -= 2;
                    }
                }
            }
        }
        if (peer_addr) {
            vPortFree(peer_addr);
        }
        */
    }

    if (scan_data) {
        len = strlen(g_le_device_name);
#ifdef MTK_AWS_MCE_ENABLE
        if (len > scan_data->data_length - 2) {
            len = scan_data->data_length - 2;
        }
#endif
        /* scan_data AD_TYPE_NAME_COMPLETE */
        scan_data->data[0] = (len + 1);
        scan_data->data[1] = BT_GAP_LE_AD_TYPE_NAME_COMPLETE;
        memcpy(&scan_data->data[2], g_le_device_name, len);
        scan_data->data_length = (len + 2);
    }

    if (adv_data) {
        bt_bd_addr_t *edr_addr = bt_device_manager_get_local_address();
        uint8_t default_unique_id[16] = {0};
#ifdef MTK_AWS_MCE_ENABLE
        uint32_t max_len = adv_data->data_length;
        len = 0;
#endif
        adv_data->data_length = 0;
#ifdef MTK_AWS_MCE_ENABLE
#ifndef AIR_HEARTHROUGH_MAIN_ENABLE
        if ((unique_id && !unique_id_len) ||
            (!unique_id && unique_id_len)) {
            return BT_STATUS_FAIL;
        }

        if (!unique_id) {
            ret = bt_app_common_generate_unique_id((const uint8_t *)edr_addr,
                                                   BT_BD_ADDR_LEN,
                                                   default_unique_id);
            if (BT_STATUS_SUCCESS != ret) {
                return ret;
            }

            unique_id = default_unique_id;
            unique_id_len = BT_APP_COMMON_UNIQUE_ID_MAX_LEN;
        }

        if (BT_APP_COMMON_UNIQUE_ID_MAX_LEN > unique_id_len) {
            unique_id_len = BT_APP_COMMON_UNIQUE_ID_MAX_LEN;
        }
#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */
#else
        if ((unique_id && !unique_id_len) ||
            (!unique_id && unique_id_len)) {
            return BT_STATUS_FAIL;
        }

        if (!unique_id) {
            ret = bt_app_common_generate_unique_id((const uint8_t *)edr_addr,
                                                   BT_BD_ADDR_LEN,
                                                   default_unique_id);
            if (BT_STATUS_SUCCESS != ret) {
                return ret;
            }

            unique_id = default_unique_id;
            unique_id_len = BT_APP_COMMON_UNIQUE_ID_MAX_LEN;
        }

        if (BT_APP_COMMON_UNIQUE_ID_MAX_LEN > unique_id_len) {
            unique_id_len = BT_APP_COMMON_UNIQUE_ID_MAX_LEN;
        }

#endif
#ifdef MTK_AWS_MCE_ENABLE
        if (len + 3 <= max_len) {
            /* adv_data AD_TYPE_FLAG */
            adv_data->data[0] = 2;
            adv_data->data[1] = BT_GAP_LE_AD_TYPE_FLAG;
            adv_data->data[2] = BT_GAP_LE_AD_FLAG_BR_EDR_NOT_SUPPORTED | BT_GAP_LE_AD_FLAG_GENERAL_DISCOVERABLE;
            len = 3;
        }
#else
            adv_data->data[0] = 2;
            adv_data->data[1] = BT_GAP_LE_AD_TYPE_FLAG;
            adv_data->data[2] = BT_GAP_LE_AD_FLAG_BR_EDR_NOT_SUPPORTED | BT_GAP_LE_AD_FLAG_GENERAL_DISCOVERABLE;
            len = 3;
#endif
#ifdef MTK_AWS_MCE_ENABLE
#ifndef AIR_HEARTHROUGH_MAIN_ENABLE
        if (len + (unique_id_len + 2) <= max_len) {
            /* adv_data AD_TYPE_MANUFACTURER_SPECIFIC */
            adv_data->data[len] = unique_id_len + 1;
            adv_data->data[len + 1] = BT_GAP_LE_AD_TYPE_MANUFACTURER_SPECIFIC;
            memcpy(&adv_data->data[len + 2], (uint8_t *)unique_id, unique_id_len);

            len += (unique_id_len + 2);
        }
#else
        if (len + (BT_BD_ADDR_LEN + sizeof(uint16_t) + 2) <= max_len) {
            uint16_t crc16_value = 0;
            app_hear_through_adv_get_crc16_value(&crc16_value);

            adv_data->data[len] = BT_BD_ADDR_LEN + sizeof(uint16_t) + 1;
            adv_data->data[len + 1] = BT_GAP_LE_AD_TYPE_MANUFACTURER_SPECIFIC;
            memcpy(&adv_data->data[len + 2], (uint8_t *)edr_addr, BT_BD_ADDR_LEN);
            memcpy(&adv_data->data[len + 2 + BT_BD_ADDR_LEN], &crc16_value, sizeof(uint16_t));

            len += (BT_BD_ADDR_LEN + sizeof(uint16_t) + 2);
        }
#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */
#else
            /* adv_data AD_TYPE_MANUFACTURER_SPECIFIC */
            adv_data->data[len] = unique_id_len + 1;
            adv_data->data[len + 1] = BT_GAP_LE_AD_TYPE_MANUFACTURER_SPECIFIC;
            memcpy(&adv_data->data[len + 2], (uint8_t *)unique_id, unique_id_len);
            len += (unique_id_len + 2);
#endif


        /* adv_data AD_TYPE_128_BIT_UUID_COMPLETE */
        uint8_t uuid[] =  BT_APP_BLE_AIR_SERVICE_UUID;
#ifdef MTK_AWS_MCE_ENABLE
        if (len + (sizeof(uuid) + 2) <= max_len) {
#endif
            adv_data->data[len] = sizeof(uuid) + 1;
            adv_data->data[len + 1] = BT_GAP_LE_AD_TYPE_128_BIT_UUID_COMPLETE;
            memcpy(&(adv_data->data[len + 2]), uuid, sizeof(uuid));
            LOG_MSGID_I(BT_APP, "len:%d type:%02x data:%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x", 18,
                        adv_data->data[len], adv_data->data[len + 1],
                        adv_data->data[len + 2], adv_data->data[len + 3], adv_data->data[len + 4], adv_data->data[len + 5],
                        adv_data->data[len + 6], adv_data->data[len + 7], adv_data->data[len + 8], adv_data->data[len + 9],
                        adv_data->data[len + 10], adv_data->data[len + 11], adv_data->data[len + 12], adv_data->data[len + 13],
                        adv_data->data[len + 14], adv_data->data[len + 15], adv_data->data[len + 16], adv_data->data[len + 17]);
            len += (sizeof(uuid) + 2);
#ifdef MTK_AWS_MCE_ENABLE
        }
#endif
        adv_data->data_length = len;
    }

    return ret;
}

bt_status_t bt_app_common_advtising_start(uint16_t adv_interval_min,
                                          uint16_t adv_interval_max,
                                          uint8_t *unique_id,
                                          uint8_t unique_id_len)
{
#define BT_APP_BLE_AIR_SERVICE_UUID                \
        {0x45, 0x4C, 0x42, 0x61, 0x68, 0x6F, 0x72, 0x69,               \
         0x41, 0x03, 0xAB, 0x2D, 0x4D, 0x49, 0x52, 0x50}

    uint8_t len = 0;
    uint8_t count = bt_device_manager_le_get_bonded_number();
    bt_bd_addr_t *peer_addr = (bt_bd_addr_t *)pvPortMalloc(count * sizeof(bt_bd_addr_t));
    if (peer_addr) {
        bt_device_manager_le_get_bonded_list(peer_addr, &count);
    } else {
        count = 0;
    }

    bt_hci_cmd_le_set_advertising_parameters_t adv_param = {
        .advertising_interval_min = 0x0500,
        .advertising_interval_max = 0x0500,
        .advertising_type = BT_HCI_ADV_TYPE_CONNECTABLE_UNDIRECTED,
        .own_address_type = BT_ADDR_RANDOM,
        .advertising_channel_map = 7,
        .advertising_filter_policy = 0
    };

    if (count) {
        int i;
        for (i = 0; i < count; i++) {
            bt_gap_le_bonding_info_t *bonded_info = NULL;
            bonded_info = bt_device_manager_le_get_bonding_info_by_addr((bt_bd_addr_t *)(peer_addr + i));
            if ((bonded_info) && (0xFF != bonded_info->identity_addr.address.type)) {
                adv_param.own_address_type = BT_ADDR_RANDOM_IDENTITY;
                memcpy(&(adv_param.peer_address), &(bonded_info->identity_addr.address), sizeof(bt_addr_t));
                if (adv_param.peer_address.type >= 2) {
                    adv_param.peer_address.type -= 2;
                }
            }
        }
    }
    if (peer_addr) {
        vPortFree(peer_addr);
    }
    bt_hci_cmd_le_set_advertising_data_t adv_data = {
        .advertising_data_length = 31,
    };
    bt_hci_cmd_le_set_scan_response_data_t scan_data = {
        .scan_response_data_length = 31,
    };
    bt_bd_addr_t *edr_addr = bt_device_manager_get_local_address();
    uint8_t default_unique_id[16] = {0};
    bt_status_t ret = BT_STATUS_FAIL;

    if (adv_interval_min < 0x0020 || adv_interval_min > 0x4000 ||
        adv_interval_max < 0x0020 || adv_interval_max > 0x4000 ||
        adv_interval_max < adv_interval_min ||
        (unique_id && !unique_id_len) ||
        (!unique_id && unique_id_len)) {
        return BT_STATUS_FAIL;
    }

    if (!unique_id) {
        ret = bt_app_common_generate_unique_id((const uint8_t *)edr_addr,
                                               BT_BD_ADDR_LEN,
                                               default_unique_id);
        if (BT_STATUS_SUCCESS != ret) {
            return ret;
        }

        unique_id = default_unique_id;
        unique_id_len = BT_APP_COMMON_UNIQUE_ID_MAX_LEN;
    }

    if (BT_APP_COMMON_UNIQUE_ID_MAX_LEN > unique_id_len) {
        unique_id_len = BT_APP_COMMON_UNIQUE_ID_MAX_LEN;
    }

    adv_param.advertising_interval_min = adv_interval_min;
    adv_param.advertising_interval_max = adv_interval_max;

    len = strlen(g_le_device_name);
    if (len + 2 <= BT_HCI_LE_ADVERTISING_DATA_LENGTH_MAXIMUM) {
        /* scan_data AD_TYPE_NAME_COMPLETE */
        scan_data.scan_response_data[0] = len + 1;
        scan_data.scan_response_data[1] = BT_GAP_LE_AD_TYPE_NAME_COMPLETE;
        memcpy(&scan_data.scan_response_data[2], g_le_device_name, len);
        len += 2;
    }

    if (5 + unique_id_len <= BT_HCI_LE_ADVERTISING_DATA_LENGTH_MAXIMUM) {
        /* adv_data AD_TYPE_FLAG */
        adv_data.advertising_data[0] = 2;
        adv_data.advertising_data[1] = BT_GAP_LE_AD_TYPE_FLAG;
        adv_data.advertising_data[2] = BT_GAP_LE_AD_FLAG_BR_EDR_NOT_SUPPORTED | BT_GAP_LE_AD_FLAG_GENERAL_DISCOVERABLE;

        /* adv_data AD_TYPE_MANUFACTURER_SPECIFIC */

        adv_data.advertising_data[3] = unique_id_len + 1;
        adv_data.advertising_data[4] = BT_GAP_LE_AD_TYPE_MANUFACTURER_SPECIFIC;
        memcpy(&adv_data.advertising_data[5], (uint8_t *)unique_id, unique_id_len);
        len = 5 + unique_id_len;
    }

    if (len + 18 <= BT_HCI_LE_ADVERTISING_DATA_LENGTH_MAXIMUM) {
        /* adv_data AD_TYPE_128_BIT_UUID_COMPLETE */
        uint8_t uuid[] =  BT_APP_BLE_AIR_SERVICE_UUID;
        adv_data.advertising_data[len] = 17;
        adv_data.advertising_data[len + 1] = BT_GAP_LE_AD_TYPE_128_BIT_UUID_COMPLETE;
        memcpy(&(adv_data.advertising_data[len + 2]), uuid, sizeof(uuid));
        LOG_MSGID_I(BT_APP, "len:%d type:%02x data:%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x", 18,
                    adv_data.advertising_data[len], adv_data.advertising_data[len + 1],
                    adv_data.advertising_data[len + 2], adv_data.advertising_data[len + 3], adv_data.advertising_data[len + 4], adv_data.advertising_data[len + 5],
                    adv_data.advertising_data[len + 6], adv_data.advertising_data[len + 7], adv_data.advertising_data[len + 8], adv_data.advertising_data[len + 9],
                    adv_data.advertising_data[len + 10], adv_data.advertising_data[len + 11], adv_data.advertising_data[len + 12], adv_data.advertising_data[len + 13],
                    adv_data.advertising_data[len + 14], adv_data.advertising_data[len + 15], adv_data.advertising_data[len + 16], adv_data.advertising_data[len + 17]);
        len = len + 18;
    }

    return bt_app_common_advertising_start_ex(&adv_param, &adv_data, &scan_data);
}

bt_status_t bt_app_common_advertising_start_ex_2(bool update_adv_param,
                                                 const bt_hci_cmd_le_set_advertising_parameters_t *adv_param,
                                                 const bt_hci_cmd_le_set_advertising_data_t *adv_data,
                                                 const bt_hci_cmd_le_set_scan_response_data_t *scan_rsp)
{
    bt_status_t start_ret;
    uint32_t sync_mask;
    bt_hci_cmd_le_set_advertising_enable_t enable = {
        .advertising_enable = BT_HCI_ENABLE
    };

    if ((update_adv_param && adv_param == NULL) || (!update_adv_param && adv_param != NULL)) {
        LOG_MSGID_I(BT_APP, "bt_app_common_advertising_start_ex_2 fail, invalid adv param.", 0);
        return BT_STATUS_FAIL;
    }

    if (BT_APP_COMMON_BLE_ADV_STARTED == bt_app_adv_ongoing && update_adv_param) {
        LOG_MSGID_I(BT_APP, "bt_app_common_advertising_start_ex_2 fail, already started", 0);
        return BT_STATUS_FAIL;
    } else if (BT_APP_COMMON_BLE_ADV_STOPPED == bt_app_adv_ongoing && !update_adv_param) {
        LOG_MSGID_I(BT_APP, "bt_app_common_advertising_start_ex_2 fail, must update param when stopped", 0);
        return BT_STATUS_FAIL;
    } else if (BT_APP_COMMON_BLE_ADV_STOPPING == bt_app_adv_ongoing || BT_APP_COMMON_BLE_ADV_STARTING == bt_app_adv_ongoing) {
        LOG_MSGID_I(BT_APP, "bt_app_common_advertising_start_ex_2 fail, ing state", 0);
        return BT_STATUS_BUSY;
    }
    hal_nvic_save_and_set_interrupt_mask(&sync_mask);
#if defined(MTK_AWS_MCE_ENABLE) && defined(SUPPORT_ROLE_HANDOVER_SERVICE)
    /**
     * @brief
     * If current is running RHO, cannot start BLE ADV.
     */
    bt_role_handover_state_t state = bt_role_handover_get_state();

    if (state == BT_ROLE_HANDOVER_STATE_ONGOING) {
        hal_nvic_restore_interrupt_mask(sync_mask);
        LOG_MSGID_I(BT_APP, "bt_app_common_advertising_start_ex_2 state 0x%x\n", 2, state);
        return BT_STATUS_BUSY;
    }
#endif

    if (bt_app_adv_ongoing == BT_APP_COMMON_BLE_ADV_STOPPED) {
        bt_app_adv_ongoing = BT_APP_COMMON_BLE_ADV_STARTING;
    } else if (bt_app_adv_ongoing == BT_APP_COMMON_BLE_ADV_STARTED) {
        bt_app_adv_ongoing = BT_APP_COMMON_BLE_ADV_UPDATING;
    }
    hal_nvic_restore_interrupt_mask(sync_mask);
    bt_gatts_set_max_mtu(BT_APP_COMMON_GATT_MTU);

    start_ret = bt_gap_le_set_advertising(update_adv_param ? (const bt_hci_cmd_le_set_advertising_enable_t *)&enable : NULL,
                                          (const bt_hci_cmd_le_set_advertising_parameters_t *)adv_param,
                                          (const bt_hci_cmd_le_set_advertising_data_t *)adv_data,
                                          (const bt_hci_cmd_le_set_scan_response_data_t *)scan_rsp);
    if (BT_STATUS_SUCCESS != start_ret) {
        hal_nvic_save_and_set_interrupt_mask(&sync_mask);
        if (BT_APP_COMMON_BLE_ADV_STARTING == bt_app_adv_ongoing) {
            bt_app_adv_ongoing = BT_APP_COMMON_BLE_ADV_STOPPED;
        } else if (BT_APP_COMMON_BLE_ADV_UPDATING == bt_app_adv_ongoing) {
            bt_app_adv_ongoing = BT_APP_COMMON_BLE_ADV_STARTED;
        }
        hal_nvic_restore_interrupt_mask(sync_mask);
        LOG_MSGID_I(BT_APP, "bt_app_common_advertising_start_ex_2 fail : %x",
                    1, start_ret);
    }
    return start_ret;
}

bt_status_t bt_app_common_advertising_start_ex(const bt_hci_cmd_le_set_advertising_parameters_t *adv_param,
                                               const bt_hci_cmd_le_set_advertising_data_t *adv_data,
                                               const bt_hci_cmd_le_set_scan_response_data_t *scan_rsp)
{
    bt_hci_cmd_le_set_advertising_enable_t enable;
    bt_status_t start_ret;
    if (s_ble_adv_disabled) {
        return BT_STATUS_SUCCESS;
    }
    if (BT_APP_COMMON_BLE_ADV_STARTED == bt_app_adv_ongoing) {
        return BT_STATUS_FAIL;
    } else if (BT_APP_COMMON_BLE_ADV_STOPPED != bt_app_adv_ongoing) {
        return BT_STATUS_BUSY;
    }
    bt_gatts_set_max_mtu(BT_APP_COMMON_GATT_MTU);
    bt_app_adv_ongoing = BT_APP_COMMON_BLE_ADV_STARTING;
    enable.advertising_enable = BT_HCI_ENABLE;
    start_ret = bt_gap_le_set_advertising((const bt_hci_cmd_le_set_advertising_enable_t *)&enable,
                                          (const bt_hci_cmd_le_set_advertising_parameters_t *)adv_param,
                                          (const bt_hci_cmd_le_set_advertising_data_t *)adv_data,
                                          (const bt_hci_cmd_le_set_scan_response_data_t *)scan_rsp);
    if (BT_STATUS_SUCCESS != start_ret) {
        bt_app_adv_ongoing = BT_APP_COMMON_BLE_ADV_STOPPED;
        LOG_MSGID_I(BT_APP, "bt_app_common_advertising_start_ex fail : %x",
                    1, start_ret);
    }

    return start_ret;

}

bt_status_t bt_app_common_nrpa_adv_start(void)
{
    bt_hci_cmd_le_set_advertising_enable_t enable;
    bt_status_t start_ret;
    if (BT_APP_COMMON_BLE_ADV_STARTED == bt_app_adv_ongoing) {
        return BT_STATUS_FAIL;
    } else if (BT_APP_COMMON_BLE_ADV_STOPPED != bt_app_adv_ongoing) {
        return BT_STATUS_BUSY;
    }
    bt_app_adv_ongoing = BT_APP_COMMON_BLE_ADV_STARTING;
    enable.advertising_enable = BT_HCI_ENABLE;
    bt_hci_cmd_le_set_advertising_parameters_t adv_param = {
        .advertising_interval_min = 0x0500,
        .advertising_interval_max = 0x0500,
        .advertising_type = BT_HCI_ADV_TYPE_CONNECTABLE_UNDIRECTED,
        .own_address_type = BT_ADDR_RANDOM,
        .advertising_channel_map = 7,
        .advertising_filter_policy = 0
    };
    bt_hci_cmd_le_set_advertising_data_t adv_data = {0};
    adv_data.advertising_data_length = 3;
    adv_data.advertising_data[0] = 2;
    adv_data.advertising_data[1] = BT_GAP_LE_AD_TYPE_FLAG;
    adv_data.advertising_data[2] = 0;
    bt_hci_cmd_le_set_scan_response_data_t scan_rsp = {0};
    scan_rsp.scan_response_data[0] = 22; /* ADV length. */
    scan_rsp.scan_response_data[1] = BT_GAP_LE_AD_TYPE_NAME_COMPLETE;
    start_ret = bt_gap_le_set_advertising(&enable, &adv_param, &adv_data, &scan_rsp);
    if (BT_STATUS_SUCCESS != start_ret) {
        bt_app_adv_ongoing = BT_APP_COMMON_BLE_ADV_STOPPED;
        LOG_MSGID_I(BT_APP, "bt_app_common_advertising_start_ex fail : %x",
                    1, start_ret);
    }

    return start_ret;
}

static bt_status_t bt_app_common_stop_ble_adv(void)
{
    uint32_t sync_mask;
    if (s_ble_adv_disabled) {
        return BT_STATUS_SUCCESS;
    }
    hal_nvic_save_and_set_interrupt_mask(&sync_mask);
    if (BT_APP_COMMON_BLE_ADV_STARTED == bt_app_adv_ongoing) {
        bt_status_t stop_ret;
        bt_hci_cmd_le_set_advertising_enable_t adv_enable = {
            .advertising_enable = BT_HCI_DISABLE,
        };
        bt_app_adv_ongoing = BT_APP_COMMON_BLE_ADV_STOPPING;
        hal_nvic_restore_interrupt_mask(sync_mask);
        stop_ret = bt_gap_le_set_advertising(&adv_enable, NULL, NULL, NULL);
        if (BT_STATUS_SUCCESS != stop_ret) {
            hal_nvic_save_and_set_interrupt_mask(&sync_mask);
            if (BT_APP_COMMON_BLE_ADV_STOPPING == bt_app_adv_ongoing) {
                bt_app_adv_ongoing = BT_APP_COMMON_BLE_ADV_STARTED;
            }
            hal_nvic_restore_interrupt_mask(sync_mask);
            LOG_MSGID_I(BT_APP, "bt_app_common_stop_ble_adv fail : %x",
                        1, stop_ret);
        }
        return stop_ret;
    } else if (BT_APP_COMMON_BLE_ADV_STOPPED == bt_app_adv_ongoing) {
        hal_nvic_restore_interrupt_mask(sync_mask);
        return BT_STATUS_BUSY;
    } else {
        hal_nvic_restore_interrupt_mask(sync_mask);
        return BT_STATUS_FAIL;
    }
}

bt_status_t bt_app_common_trigger_increase_ble_adv(void)
{
    LOG_MSGID_I(BT_APP, "bt_app_common_trigger_increase_ble_adv : %d",
                1, ble_adv_increase_interval);
    if (ble_adv_increase_interval) {
        return bt_app_common_stop_ble_adv();
    } else {
        return BT_STATUS_SUCCESS;
    }
}

bt_app_common_ble_adv_status_t bt_app_common_get_advertising_status(void)
{
    return bt_app_adv_ongoing;
}

bool bt_app_common_get_power_on_status(void)
{
    return bt_app_is_power_on;
}


bool bt_app_common_ble_adv_timer_stop(void)
{
    if (!bt_ble_adv_timer) {
        return TRUE;
    }

    if (xTimerIsTimerActive(bt_ble_adv_timer) == pdTRUE) {
        LOG_MSGID_I(BT_APP, "stop the adv timer", 0);
        return xTimerStop(bt_ble_adv_timer, 0);
    } else {
        return TRUE;
    }
}

static void bt_app_common_ble_adv_timer_hdlr(TimerHandle_t timer_id)
{
    LOG_MSGID_I(BT_APP, "time out, id(0x%04x), g_id(0x%04x)", 2, timer_id, bt_ble_adv_timer);

    if (bt_ble_adv_timer != timer_id) {
        return;
    }

    ble_adv_increase_interval = true;
    // Because directly calling the function may cause system timer task pending, send a event to UI task.
    // bt_app_common_stop_ble_adv();
    ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                        APPS_EVENTS_INTERACTION_INCREASE_BLE_ADV_INTERVAL, NULL, 0,
                        NULL, 0);
    if (BT_APP_COMMON_BLE_ADV_STARTED == bt_app_adv_ongoing) {
        bt_app_common_ble_adv_timer_stop();
    }
}


bt_status_t bt_app_common_ble_adv_timer_start(void)
{
    if (!bt_ble_adv_timer) {
        bt_ble_adv_timer = xTimerCreate("ble_adv_timer",
                                        (BT_BLE_ADV_TIMER_TIMEROUT_IN_MS / portTICK_PERIOD_MS),
                                        pdFALSE, /* Repeat timer */
                                        NULL,
                                        bt_app_common_ble_adv_timer_hdlr);
    }

    if (!bt_ble_adv_timer) {
        return BT_STATUS_FAIL;
    }

    if (xTimerIsTimerActive(bt_ble_adv_timer) == pdFALSE) {
        xTimerStart(bt_ble_adv_timer, 0);
    }
    LOG_MSGID_I(BT_APP, "bt_app_common_ble_adv_timer_start", 0);

    return BT_STATUS_SUCCESS;
}

bt_status_t bt_app_common_advtising_stop(void)
{
    ble_adv_increase_interval = false;
    bt_app_common_ble_adv_timer_stop();
    return bt_app_common_stop_ble_adv();
}

/* Just for ble multiple advertising test AT command.
  */
static uint8_t g_bt_app_common_ext_adv_test_handle = 0;
bt_status_t bt_app_common_ext_advertising_start_test(uint8_t instance)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_bd_addr_t random_address = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x00};
    bt_hci_le_set_ext_advertising_parameters_t ext_adv_para = {
        .advertising_event_properties = BT_HCI_ADV_EVT_PROPERTIES_MASK_SCANNABLE | BT_HCI_ADV_EVT_PROPERTIES_MASK_LEGACY_PDU | BT_HCI_ADV_EVT_PROPERTIES_MASK_CONNECTABLE,
        .primary_advertising_interval_min = 0x00100,
        .primary_advertising_interval_max = 0x00400,
        .primary_advertising_channel_map = 0x07,
        .own_address_type = BT_ADDR_RANDOM,
        .peer_address = {
            .type = BT_ADDR_PUBLIC,
            .addr = {0},
        },
        .advertising_filter_policy = BT_HCI_ADV_FILTER_ACCEPT_SCAN_CONNECT_FROM_ALL,
        .advertising_tx_power = 0,
        .primary_advertising_phy = BT_HCI_LE_ADV_PHY_1M,
        .secondary_advertising_max_skip = 0x00,
        .secondary_advertising_phy = BT_HCI_LE_ADV_PHY_1M,
        .advertisng_SID = 0x00,
        .scan_request_notify_enable = BT_HCI_DISABLE
    };

    bt_gap_le_set_ext_advertising_data_t ext_adv_data = {
        .data_length = 0,
        .data = NULL,
        .fragment_preference = 0x00
    };

    bt_gap_le_set_ext_scan_response_data_t ext_scan_rsp_data = {
        .data_length = 0,
        .data = NULL,
        .fragment_preference = 0x00
    };

    uint8_t adv_data[15] = {0x0E, 0x09, 'L', 'E', '_', 'M', 'U', 'L', '_', 'A', 'D', 'V', '_'};
    uint8_t scan_rsp_data[21] = {0x14, 0x09, 'L', 'E', '_', 'M', 'U', 'L', '_', 'S', 'C', 'A', 'N', '_', 'R', 'E', 'S', 'P', '_'};
    adv_data[13] = ((instance >> 4) < 0xA ? (instance >> 4) + 48 : (instance >> 4) + 55);
    adv_data[14] = ((instance & 0xF) < 0xA ? (instance & 0xF) + 48 : (instance & 0xF) + 55);
    scan_rsp_data[19] = adv_data[13];
    scan_rsp_data[20] = adv_data[14];

    ext_adv_para.advertisng_SID = instance;
    ext_adv_data.data_length = 15;
    ext_adv_data.data = adv_data;
    ext_scan_rsp_data.data_length = 21;
    ext_scan_rsp_data.data = scan_rsp_data;

    random_address[5] = instance;
    if (BT_STATUS_SUCCESS == (status = bt_gap_le_config_extended_advertising(instance, (void *)&random_address, &ext_adv_para, &ext_adv_data, &ext_scan_rsp_data))) {
        g_bt_app_common_ext_adv_test_handle = instance;
    }
    return status;
}

/* Just for ble multiple advertising test AT command.
  */
bt_status_t bt_app_common_ext_advertising_stop_test(uint8_t instance)
{
    bt_hci_le_set_ext_advertising_enable_t ext_adv_enable = {
        .enable = BT_HCI_DISABLE,
        .duration = 0x0000,
        .max_ext_advertising_evts = 0x00,
    };

    return bt_gap_le_enable_extended_advertising(instance, &ext_adv_enable);
}

bt_status_t bt_app_common_start_ble_adv_with_default_interval(void)
{
    bt_app_common_generate_device_name();
    bt_status_t ret = bt_app_common_advtising_start(0xF4, 0xF4, NULL, 0);
    if (BT_STATUS_SUCCESS == ret) {
        ret = bt_app_common_ble_adv_timer_start();
    }
    LOG_MSGID_I(BT_APP, "bt_app_common_ble_advtising_start result = %X", 1, ret);
    return ret;
}

#if 0
static bt_status_t ble_app_common_conn_interval_update(bt_handle_t connection_handle)
{
    bt_hci_cmd_le_connection_update_t conn_params;

    conn_params.supervision_timeout = 0x0258;            /** TBC: 6000ms : 600 * 10 ms. */
    conn_params.connection_handle = connection_handle;

    conn_params.conn_interval_min = 0x0120;/** TBC: 380ms : 288 * 1.25 ms. */
    conn_params.conn_interval_max = 0x0130;/** TBC: 400ms : 320 * 1.25 ms. */
    conn_params.slave_latency = 0;

    return bt_gap_le_update_connection_parameter(&conn_params);
}
#endif

void bt_app_common_stop_scan()
{
    bt_hci_cmd_le_set_scan_enable_t enable;
    enable.le_scan_enable = BT_HCI_DISABLE;
    enable.filter_duplicates = BT_HCI_DISABLE;
    bt_gap_le_set_scan(&enable, NULL);
}

void bt_app_common_start_scan()
{
    bt_hci_cmd_le_set_scan_enable_t enable;
    bt_hci_cmd_le_set_scan_parameters_t param;

    bt_app_common_stop_scan();

    enable.le_scan_enable = BT_HCI_ENABLE;
    enable.filter_duplicates = BT_HCI_ENABLE;
    param.le_scan_type = 0;
    param.own_address_type = BT_HCI_SCAN_ADDR_RANDOM;
    param.scanning_filter_policy = BT_HCI_SCAN_FILTER_ACCEPT_ALL_ADVERTISING_PACKETS;
    param.le_scan_window = 0x60;
    param.le_scan_interval = 0x80;
    bt_gap_le_set_scan(&enable, &param);
}

void bt_app_common_adv_ind_cb(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    uint8_t local_buff[32] = {0};
    uint8_t count, ad_data_len, ad_data_type, ad_data_idx;
    const bt_gap_le_advertising_report_ind_t *report = (bt_gap_le_advertising_report_ind_t *)buff;
    count = 0;
    uint8_t addr_buff[26] = {0};
    if ((report->address.type & 0x0D) == BT_ADDR_PUBLIC) {
        LOG_MSGID_W(BT_APP, "adv report: Public, %02x:%02x:%02x:%02x:%02x:%02x",
                    6,
                    report->address.addr[5],
                    report->address.addr[4],
                    report->address.addr[3],
                    report->address.addr[2],
                    report->address.addr[1],
                    report->address.addr[0]
                   );
        snprintf((char *)addr_buff, 26, "Public, %02x:%02x:%02x:%02x:%02x:%02x", report->address.addr[5],
                 report->address.addr[4],
                 report->address.addr[3],
                 report->address.addr[2],
                 report->address.addr[1],
                 report->address.addr[0]);
    } else if ((report->address.type & 0x0D) == BT_ADDR_RANDOM) {
        LOG_MSGID_W(BT_APP, "adv report: Random, %02x:%02x:%02x:%02x:%02x:%02x",
                    6,
                    report->address.addr[5],
                    report->address.addr[4],
                    report->address.addr[3],
                    report->address.addr[2],
                    report->address.addr[1],
                    report->address.addr[0]
                   );
        snprintf((char *)addr_buff, 26, "Random, %02x:%02x:%02x:%02x:%02x:%02x", report->address.addr[5],
                 report->address.addr[4],
                 report->address.addr[3],
                 report->address.addr[2],
                 report->address.addr[1],
                 report->address.addr[0]);
    }
    while (count < report->data_length) {
        ad_data_len = report->data[count];
        /* Error handling for data length over 30 bytes. */
        if (ad_data_len >= 0x1F) {
            LOG_MSGID_W(BT_APP, "AD Data Length Error\n", 0);
            break;
        }
        ad_data_type = report->data[count + 1];
        count += 2;
        if (ad_data_type == BT_GAP_LE_AD_TYPE_NAME_COMPLETE) {
            for (ad_data_idx = 0; ad_data_idx < (ad_data_len - 1); ad_data_idx++, count++) {
                local_buff[ad_data_idx] = report->data[count];
            }
            uint8_t report_string[90] = {0};
            snprintf((char *)report_string, 90, "ADV report %s Complete Name: %s", addr_buff, local_buff);
#ifndef AIR_SPEAKER_ENABLE
            bt_app_common_at_cmd_print_report((char *)report_string);
#endif
        } else {
            count += (ad_data_len - 1);
        }
    }
    LOG_MSGID_W(BT_APP, "adv report parse out", 0);
}

static bt_status_t bt_app_common_check_ble_conn_interval(bt_handle_t connection_handle,
                                                         bt_sink_srv_state_change_t *state_change_param)
{
#ifdef AIR_LE_AUDIO_ENABLE
    return BT_STATUS_SUCCESS;
#else
    if (s_waiting_conn_interval) {
        LOG_MSGID_I(BT_APP, "Set connection interval is busy", 0);
        return BT_STATUS_BUSY;
    } else {
        uint16_t target_interval = 0;
        bt_sink_srv_state_t bt_sink_state;
        if (state_change_param) {
            bt_sink_state = state_change_param->current;
        } else {
            bt_sink_state = bt_sink_srv_get_state();
        }
        if (bt_sink_state >= BT_SINK_SRV_STATE_INCOMING && bt_sink_state <= BT_SINK_SRV_STATE_MULTIPARTY) {
            if (g_conn_interval < 0x0078) { /** TBC: 360ms : 288 * 1.25 ms. */
                target_interval = 0x0078;
            }
        } else {
            if (g_conn_interval < 0x18 || g_conn_interval > 0x28) { /** TBC: 30ms : 24 * 1.25 ms. */
                target_interval = 0x18;
            }
        }
        if (0 != target_interval) {
            bt_status_t set_param_ret;
            bt_hci_cmd_le_connection_update_t param;
            param.connection_handle = connection_handle;
            param.conn_interval_min = target_interval;
            param.conn_interval_max = target_interval;
            param.conn_latency = 0;
            param.supervision_timeout = 0x0258;/** TBC: 6000ms : 600 * 10 ms. */
            s_waiting_conn_interval = true;
            LOG_MSGID_I(BT_APP, "Set connection interval : 0x%x->0x%x",
                        2, g_conn_interval, target_interval);
            set_param_ret = bt_gap_le_update_connection_parameter(&param);
            LOG_MSGID_I(BT_APP, "Set connection interval : 0x%x, result = %d",
                        2, target_interval, set_param_ret);
            if (BT_STATUS_SUCCESS != set_param_ret) {
                s_waiting_conn_interval = false;
            }
            return set_param_ret;
        } else {
            LOG_MSGID_I(BT_APP, "connection interval has been correct : 0x%x",
                        1, g_conn_interval);
            return BT_STATUS_SUCCESS;
        }
    }
#endif
}

static bt_status_t bt_app_common_event_callback(bt_msg_type_t msg, bt_status_t status, void *buff)
{
#ifndef AIR_LE_AUDIO_ENABLE
    LOG_MSGID_I(BT_APP, "bt_app_common_event_callback: status(0x%04x), msg(0x%04x)", 2, status, msg);
#endif

    if (BT_GAP_LE_CONNECTION_UPDATE_IND == msg) {
        bt_gap_le_connection_update_ind_t *ind = (bt_gap_le_connection_update_ind_t *)buff;
        LOG_MSGID_I(BT_APP, "Receive CONNECTION UPDATE: event_id = %x, connection interval = %d\n", 2, msg, ind->conn_interval);
        g_conn_interval = ind->conn_interval;
        s_waiting_conn_interval = false;
        bt_app_common_check_ble_conn_interval(ind->conn_handle, NULL);
        if ((app_common_context.retry_handle == ind->conn_handle) && (status == BT_STATUS_SUCCESS)) {
            LOG_MSGID_I(BT_APP, "[APP][COMMON] update interval retry success handle = %02x", 1, ind->conn_handle);
            app_common_context.update_interval_fail_retry_count = 0;
        }
    } else if (BT_GAP_LE_CONNECTION_UPDATE_CNF == msg) {
        LOG_MSGID_I(BT_APP, "CONNECTION CONFIRM status 0x%08x", 1, status);
        if (status != BT_STATUS_SUCCESS) {
            if (app_common_context.update_interval_fail_retry_count >= BT_APP_COMMON_UPDATE_INTERVAL_FAIL_RETRY_MAX) {
                LOG_MSGID_I(BT_APP, "[APP][COMMON] update interval retry count over %02x", 1, BT_APP_COMMON_UPDATE_INTERVAL_FAIL_RETRY_MAX);
                app_common_context.update_interval_fail_retry_count = 0;
                return BT_STATUS_SUCCESS;
            }
            bt_handle_t conn_handle = bt_app_common_get_first_conneciton_handle();
            s_waiting_conn_interval = false;
            if (conn_handle) {
                bt_status_t update_status = bt_app_common_check_ble_conn_interval(conn_handle, NULL);
                if (update_status == BT_STATUS_SUCCESS) {
                    app_common_context.update_interval_fail_retry_count++;
                    app_common_context.retry_handle = conn_handle;
                } else {
                    app_common_context.update_interval_fail_retry_count = 0;
                }
                LOG_MSGID_I(BT_APP, "[APP][COMMON] update interval retry status = %02x", 1, update_status);
            }
        }
    }
    //adv cnf needs to handle error state.
    if (status != BT_STATUS_SUCCESS && msg != BT_GAP_LE_SET_ADVERTISING_CNF) {
        return BT_STATUS_SUCCESS;
    }

    switch (msg) {
        case BT_POWER_ON_CNF: {
            bt_app_is_power_on = true;
            bt_app_common_generate_local_address(bt_local_random_addr);
#ifdef AIR_TWS_ENABLE
#ifndef AIR_LE_AUDIO_ENABLE
            /* Set local le rand addr when multi adv enable */
            bt_gap_le_set_random_address((bt_bd_addr_ptr_t)bt_local_random_addr);
#else
            app_le_audio_dhss_read_local_le_addr();
#endif
#else
            bt_gap_le_set_random_address((bt_bd_addr_ptr_t)bt_local_random_addr);
#endif
        }
        bt_gatts_set_max_mtu(BT_APP_COMMON_GATT_MTU);
        break;

        case BT_POWER_OFF_CNF: {
            bt_app_is_power_on = false;
            bt_app_common_reset_context();
        }
        break;

        case BT_GAP_LE_SET_RANDOM_ADDRESS_CNF:
            //bt_app_common_advtising_init();
            break;

        case BT_GAP_LE_CONNECT_IND: {
            bt_gap_le_connection_ind_t *connection_ind = (bt_gap_le_connection_ind_t *)buff;
            g_conn_interval = connection_ind->conn_interval;
            s_waiting_conn_interval = false;
            g_le_connection_handle = connection_ind->connection_handle;
            g_bt_cm_power_standby_flag = false;
            LOG_MSGID_I(BT_APP, "Connect complete, conn_handle(0x%04x), conn_interval(0x%x).\r\n",
                        2, connection_ind->connection_handle, g_conn_interval);
            char conn_string[30] = {0};
            snprintf((char *)conn_string, 30, "LE connected, handle 0x%04x", connection_ind->connection_handle);
            bt_app_common_at_cmd_print_report(conn_string);
#ifdef MTK_AWS_MCE_ENABLE
            //bt_gap_le_bond(connection_ind->connection_handle, &pairing_config);
#else
            if (bt_app_ctkd_enable) {
                bt_gap_le_bond(connection_ind->connection_handle, &pairing_config_ctkd);
            }
#endif
            bt_app_common_add_connection_info(buff);
            ble_adv_increase_interval = false;
            bt_app_adv_ongoing = BT_APP_COMMON_BLE_ADV_STOPPED;
            bt_app_common_ble_adv_timer_stop();
#ifdef MTK_AWS_MCE_ENABLE
           //bt_app_common_check_ble_conn_interval(connection_ind->connection_handle, NULL);
#else
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
           bool ull2_link = bt_gap_le_check_remote_features(connection_ind->connection_handle, BT_GAP_LE_ULL2_0);
           if (!ull2_link) {
               LOG_MSGID_I(BT_APP, "bt_app_common_event_callback, is_ull: %d", 1, ull2_link);
               bt_app_common_check_ble_conn_interval(connection_ind->connection_handle, NULL);
           }
#else
           bt_app_common_check_ble_conn_interval(connection_ind->connection_handle, NULL);
#endif
#endif
        }
        break;

        case BT_GAP_LE_BONDING_REPLY_REQ_IND: {
            if (buff == NULL) {
                return status;
            }
            bt_gap_le_bonding_reply_req_ind_t *ind = (bt_gap_le_bonding_reply_req_ind_t *)buff;
#ifdef AIR_SPEAKER_ENABLE
            if (ind->method == BT_GAP_LE_SMP_OOB) {
                bt_gap_le_bonding_reply_t reply = {
                    .oob_data = {0x61, 0x68, 0x6F, 0x72, 0x69, 0x61, 0x11, 0x8F, 0x58, 0x4C, 0xFF, 0x10, 0x75, 0x4E, 0x28, 0xA2}
                };
                bt_gap_le_bonding_reply(ind->handle, (bt_gap_le_bonding_reply_t const * const)&reply);
            }
#endif

            if (ind->method & BT_GAP_LE_SMP_PASSKEY_DISPLAY_MASK) {
                LOG_MSGID_I(BT_APP, "------------------->Passkey: %06u<-----------------\n", 1, ind->passkey_display);
#ifdef MTK_AWS_MCE_ENABLE
            } else {
                //bt_gap_le_bonding_reply_t rsp = {{{0}}};
                //user implement
#else
            } else if (ind->method & BT_GAP_LE_SMP_NUMERIC_COMPARISON_MASK) {
                if (bt_app_ctkd_enable) {
                    bt_gap_le_bonding_reply_t rsp = {{{0}}};
                    rsp.nc_value_matched = true;
                    bt_gap_le_bonding_reply(ind->handle, (bt_gap_le_bonding_reply_t const * const)&rsp);
                }
                //user implement
#endif
            }
        }
        break;

        case BT_GAP_LE_DISCONNECT_IND: {
            bt_gap_le_disconnect_ind_t *disc_ind = (bt_gap_le_disconnect_ind_t *)buff;
            char conn_string[31] = {0};
            snprintf((char *)conn_string, 31, "LE disconnected, handle 0x%04x", disc_ind->connection_handle);

            bt_app_common_at_cmd_print_report(conn_string);
            //bt_app_common_advtising_init();
            bt_app_common_delete_connection_info(buff);
#ifdef AIR_LE_OTP_ENABLE
            ble_otp_reset_service_attribute(disc_ind->connection_handle);
#endif
            g_bt_cm_power_standby_flag = false;
            g_le_connection_handle = 0;
        }
        break;

        case BT_GAP_SET_SCAN_MODE_CNF:
            break;

        case BT_GAP_LE_SET_ADVERTISING_CNF: {
            if (BT_APP_COMMON_BLE_ADV_STOPPING == bt_app_adv_ongoing) {
                bt_app_adv_ongoing = (status == BT_STATUS_SUCCESS) ? BT_APP_COMMON_BLE_ADV_STOPPED : BT_APP_COMMON_BLE_ADV_STARTED;
            } else if (BT_APP_COMMON_BLE_ADV_STARTING == bt_app_adv_ongoing) {
                bt_app_adv_ongoing = (status == BT_STATUS_SUCCESS) ? BT_APP_COMMON_BLE_ADV_STARTED : BT_APP_COMMON_BLE_ADV_STOPPED;
            } else if (BT_APP_COMMON_BLE_ADV_UPDATING == bt_app_adv_ongoing) {
                bt_app_adv_ongoing = BT_APP_COMMON_BLE_ADV_STARTED;
            }
#if defined(MTK_AWS_MCE_ENABLE) && defined(SUPPORT_ROLE_HANDOVER_SERVICE)
            bt_role_handover_state_t state = bt_role_handover_get_state();
            LOG_MSGID_I(BT_APP, "BT_GAP_LE_SET_ADVERTISING_CNF state 0x%x, adv %d\n", 2, state, bt_app_adv_restart_flag);

            if (state == BT_ROLE_HANDOVER_STATE_ONGOING) {
                if (bt_app_adv_ongoing == BT_APP_COMMON_BLE_ADV_STARTED) {
                    bt_app_common_advtising_stop();
                } else if (bt_app_adv_restart_flag
#if !defined(BT_ROLE_HANDOVER_WITH_SPP_BLE)
                           && bt_app_common_is_connection_info_list_empty()
#endif
                          ) {
                    bt_role_handover_reply_prepare_request(BT_ROLE_HANDOVER_MODULE_BLE_APP);
                }
            }
#endif
            if (BT_APP_COMMON_BLE_ADV_STOPPED == bt_app_adv_ongoing && ble_adv_increase_interval) {
                ble_adv_increase_interval = false;
                bt_app_common_advtising_start(0x29C, 0x29C, NULL, 0);
            }
        }
        break;
        case BT_GAP_LE_ADVERTISING_REPORT_IND:
            bt_app_common_adv_ind_cb(msg, status, buff);
            break;
        case BT_GAP_LE_CONFIG_EXTENDED_ADVERTISING_CNF: {
            bt_gap_le_config_extended_advertising_cnf_t *cnf = (bt_gap_le_config_extended_advertising_cnf_t *)buff;
            if (cnf) {
                LOG_MSGID_I(BT_APP, "Config extend advertising cnf handle: 0x%02X, power: 0x%02X \n", 2, cnf->handle, cnf->selected_tx_power);
                bt_hci_le_set_ext_advertising_enable_t ext_adv_enable = {
                    .enable = BT_HCI_ENABLE,
                    .duration = 0x0000,
                    .max_ext_advertising_evts = 0x00,
                };

                bt_gap_le_enable_extended_advertising(cnf->handle, &ext_adv_enable);
            } else {
                LOG_MSGID_E(BT_APP, "Config extend advertising cnf is null \n", 0);
            }
        }
        break;
        case BT_GAP_LE_ENABLE_EXTENDED_ADVERTISING_CNF: {
            bt_gap_le_enable_extended_advertising_cnf_t *cnf = (bt_gap_le_enable_extended_advertising_cnf_t *)buff;
            if (cnf) {
                LOG_MSGID_I(BT_APP, "enalbe extend advertising cnf handle: 0x%02X, enable: %d\n", 2, cnf->handle, cnf->enable);
            } else {
                LOG_MSGID_E(BT_APP, "enable extend advertising cnf is null \n", 0);
            }
        }
        break;
#ifdef AIR_LE_AUDIO_ENABLE
        case BT_GAP_LE_CIS_ESTABLISHED_IND: {
            bt_gap_le_cis_established_ind_t *ind = (bt_gap_le_cis_established_ind_t *)buff;
            char conn_string[40] = {0};
            snprintf((char *)conn_string, 40, "CIS connected, handle 0x%04x", ind->connection_handle);
            bt_app_common_at_cmd_print_report(conn_string);
            break;
        }
        case BT_GAP_LE_CIS_TERMINATED_IND: {
            bt_gap_le_cis_terminated_ind_t *ind = (bt_gap_le_cis_terminated_ind_t *)buff;
            char conn_string[40] = {0};
            snprintf((char *)conn_string, 40, "CIS disconnected, handle 0x%04x", ind->connection_handle);
            bt_app_common_at_cmd_print_report(conn_string);
            break;
        }
#ifdef AIR_LE_AUDIO_BIS_ENABLE
        case BT_GAP_LE_BIG_SYNC_ESTABLISHED_IND: {
            bt_gap_le_big_sync_established_ind_t *ind = (bt_gap_le_big_sync_established_ind_t *)buff;
            char conn_string[40] = {0};
            if (ind->status == BT_STATUS_SUCCESS) {
                snprintf((char *)conn_string, 40, "BIG established, handle 0x%04x", ind->big_handle);
            } else {
                snprintf((char *)conn_string, 40, "BIG established fail, handle 0x%04x", ind->big_handle);
            }
            bt_app_common_at_cmd_print_report(conn_string);

            bt_sink_srv_cap_stream_service_big_t *big_info = bt_sink_srv_cap_stream_get_service_big();
            if (big_info != NULL) {
                if (big_info->num_bis == 2) {
                    snprintf((char *)conn_string, 40, "BIS indices [%d,%d]", big_info->bis_indices[0], big_info->bis_indices[1]);
                } else if (big_info->num_bis == 1) {
                    snprintf((char *)conn_string, 40, "BIS indices [%d]", big_info->bis_indices[0]);
                }
                bt_app_common_at_cmd_print_report(conn_string);
            }
            break;
        }
        case BT_GAP_LE_BIG_SYNC_LOST_IND: {
            bt_gap_le_big_sync_lost_ind_t *ind = (bt_gap_le_big_sync_lost_ind_t *)buff;
            char conn_string[40] = {0};
            snprintf((char *)conn_string, 40, "BIG lost, handle 0x%04x", ind->big_handle);
            bt_app_common_at_cmd_print_report(conn_string);
            break;
        }
#endif
#endif
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}

void bt_app_common_le_disconnect(bt_handle_t handle)
{
    bt_hci_cmd_disconnect_t disconnect_para = {
        .connection_handle = 0x0200,
        .reason = BT_HCI_STATUS_REMOTE_USER_TERMINATED_CONNECTION,
    };

    disconnect_para.connection_handle = handle;
    bt_gap_le_disconnect(&disconnect_para);
}

void bt_app_common_on_bt_sink_event_callback(bt_sink_srv_event_t event_id, void *param, uint32_t param_len)
{
#ifdef MTK_AWS_MCE_ENABLE
    if (BT_AWS_MCE_ROLE_AGENT == bt_device_manager_aws_local_info_get_role())
#endif
    {
        if (BT_SINK_SRV_EVENT_STATE_CHANGE == event_id && param) {
            bt_handle_t conn_handle = bt_app_common_get_first_conneciton_handle();
            bt_sink_srv_event_param_t *event = (bt_sink_srv_event_param_t *)param;
            LOG_MSGID_I(BT_APP, "bt_app_common_on_bt_sink_event_callback, conn_handle = %d", 1, conn_handle);
            if (conn_handle && event) {
                bt_app_common_check_ble_conn_interval(conn_handle, &event->state_change);
            }
        }
    }
}

#if defined(MTK_AWS_MCE_ENABLE) && defined(SUPPORT_ROLE_HANDOVER_SERVICE)
BT_PACKED(
typedef struct {
    bool advertising_restart_flag;
    bt_handle_t le_connection_handle;
})bt_app_common_rho_data_t;

//todo, ble addr is different after rho.
bt_status_t bt_app_common_role_handover_service_allowed(const bt_bd_addr_t *addr)
{
    if (bt_app_adv_ongoing) {
        bt_app_adv_restart_flag = true;
        return BT_STATUS_PENDING;
    } else {
        LOG_MSGID_I(BT_APP, "bt_app_common_role_handover_service_allowed, bt_app_adv_ongoing = %d, empty = %d",
                    2, bt_app_adv_ongoing, bt_app_common_is_connection_info_list_empty());
        return BT_STATUS_SUCCESS;
    }
}

uint8_t bt_app_common_role_handover_service_get_length(const bt_bd_addr_t *addr)
{
    if (bt_app_adv_restart_flag) {
        return sizeof(bt_app_common_rho_data_t);
    } else {
        return 0;
    }
}

bt_status_t bt_app_common_role_handover_service_get_data(const bt_bd_addr_t *addr, void *data)
{
    if (bt_app_adv_restart_flag) {
        bt_app_common_rho_data_t *data_p = (bt_app_common_rho_data_t *)data;
        data_p->advertising_restart_flag = bt_app_adv_restart_flag;
        data_p->le_connection_handle = g_le_connection_handle;
    }
    return BT_STATUS_SUCCESS;
}

bt_status_t bt_app_common_role_handover_service_update(bt_role_handover_update_info_t *info)
{
    if (info->role == BT_AWS_MCE_ROLE_PARTNER && info->data != NULL) {
        bt_app_common_rho_data_t *data_p = (bt_app_common_rho_data_t *)info->data;
        bt_app_adv_restart_flag = data_p->advertising_restart_flag;
        g_le_connection_handle = data_p->le_connection_handle;
    }
    return BT_STATUS_SUCCESS;
}

void bt_app_common_role_handover_service_status_callback(const bt_bd_addr_t *addr, bt_aws_mce_role_t role, bt_role_handover_event_t event, bt_status_t status)
{
    LOG_MSGID_I(BT_APP, "RHO srv status_callback role 0x%x, event %d, status 0x%x, adv_ongoing %d, adv_restart_flag %d", 5,
                role, event, status, bt_app_adv_ongoing, bt_app_adv_restart_flag);
    switch (event) {
        //todo, if ADV is stopped or connection is disc due to RHO, do we need to start again in agent part after RHO success.
        case BT_ROLE_HANDOVER_PREPARE_REQ_IND: {
            if (role == BT_AWS_MCE_ROLE_AGENT) {
                if (bt_app_adv_ongoing == BT_APP_COMMON_BLE_ADV_STARTED) {
                    bt_app_common_advtising_stop();
                }
            }
            break;
        }
        case BT_ROLE_HANDOVER_COMPLETE_IND: {
            if (bt_app_adv_restart_flag && ((status == BT_STATUS_SUCCESS && role == BT_AWS_MCE_ROLE_PARTNER)
                                            || (status != BT_STATUS_SUCCESS && role == BT_AWS_MCE_ROLE_AGENT))) {
                LOG_MSGID_I(BT_APP, "bt_app_common_role_handover_service_status_callback restart ble adv for old role: %x",
                            1, role);
                //bt_app_common_start_ble_adv_with_default_interval();
                bt_app_common_generate_device_name();
            }
            bt_app_adv_restart_flag = false;
            break;
        }
    }
}
#endif

void bt_app_common_dm_le_event_callback(bt_device_manager_le_bonded_event_t event, bt_addr_t *address)
{
#if defined(MTK_AWS_MCE_ENABLE) && !defined(AIR_SPEAKER_ENABLE)
    if (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT) {
        bt_app_common_sync_bonded_info(&(address->addr), &(address->type), 1, event);
    }
#endif

#ifdef AIR_LE_AUDIO_ENABLE
    switch (event) {
        case BT_DEVICE_MANAGER_LE_BONDED_REMOVE:
            bt_le_audio_sink_delete_cccd(address);
            break;
        case BT_DEVICE_MANAGER_LE_BONDED_CLEAR:
            bt_le_audio_sink_delete_cccd(NULL);
            break;
    }
#endif
}

extern bt_status_t bt_dm_le_remove_bonded_device(bt_addr_t *peer_addr, bool is_clear);
static bt_status_t bt_app_common_get_pairing_config(bt_gap_le_bonding_start_ind_t *ind)
{
#if defined(AIR_SWIFT_PAIR_ENABLE) && defined(AIR_CUST_PAIR_ENABLE)
    extern bool cust_pair_srv_is_cust_pairing_ongoing(void);
    bool is_cust_ongoing = cust_pair_srv_is_cust_pairing_ongoing();
    cust_pair_conn_type conn_type = cust_pair_get_conn_type(ind->handle);
    bool is_cust_link = (conn_type == CUST_PAIR_CONN_TYPE_CUST);
#else
    bool is_cust_ongoing = FALSE;
    bool is_cust_link = FALSE;
#endif
    bt_gap_le_srv_conn_info_t *conn_info = bt_gap_le_srv_get_conn_info(ind->handle);
    LOG_MSGID_I(BT_APP, "[APP][COMMON][CUST_PAIR][SWIFT_PAIR] get connection, handle=0x%04X link_type=0x%08X is_cust_link=%d is_cust_ongoing=%d is_previously_bonded=%d",
                5, ind->handle, (conn_info != NULL ? conn_info->link_type : 0xFFFF),
                is_cust_link, is_cust_ongoing, ind->is_previously_bonded);

#if defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
    if (conn_info != NULL && !is_cust_link) {
        if ((conn_info->link_type & BT_GAP_LE_SRV_LINK_TYPE_LE_AUDIO) > 0) {
#ifdef MTK_AWS_MCE_ENABLE
#ifdef AIR_LE_AUDIO_DUALMODE_ENABLE
            /* BR/EDR is only supported on primary earbud */
            if (app_le_audio_is_primary_earbud()) {
#else
            /* BR/EDR is only supported on Agent */
            bt_aws_mce_role_t role = bt_device_manager_aws_local_info_get_role();
            if (BT_AWS_MCE_ROLE_AGENT == role) {
#endif
                ind->pairing_config_req = pairing_config_le_audio;
            } else {
                ind->pairing_config_req = pairing_config_le_audio_secondary;
            }
#else
            ind->pairing_config_req = pairing_config_le_audio;
#endif

#ifdef AIR_LE_AUDIO_DUALMODE_ENABLE
            if (false == app_lea_service_is_enable_dual_mode()) {
                ind->pairing_config_req.initiator_key_distribution &= ~BT_GAP_LE_SMP_KEY_DISTRIBUTE_LINK;
                ind->pairing_config_req.responder_key_distribution &= ~BT_GAP_LE_SMP_KEY_DISTRIBUTE_LINK;
            }
#endif

            if (ind->is_previously_bonded) {
                bt_dm_le_remove_bonded_device(&conn_info->peer_addr, false);
            }
            return BT_STATUS_SUCCESS;
        }
    } else {
        LOG_MSGID_W(BT_APP, "[APP][COMMON] get connection link type by handle = %02x not find info", 1, ind->handle);
    }
#endif

#ifdef AIR_SWIFT_PAIR_ENABLE
#ifdef AIR_CUST_PAIR_ENABLE
    if (conn_type == CUST_PAIR_CONN_TYPE_CUST) {
        LOG_MSGID_I(BT_APP, "[CUST_PAIR] get_pairing_config, CUST success %d", 1, is_cust_ongoing);
        if (is_cust_ongoing) {
            ind->pairing_config_req = pairing_config_cust_ext_pair;
        } else {
            ind->pairing_config_req = pairing_config_cust_std_pair;
        }
        return BT_STATUS_SUCCESS;
    } else if (conn_type == CUST_PAIR_CONN_TYPE_STD) {
        LOG_MSGID_I(BT_APP, "[CUST_PAIR] get_pairing_config, SWIFT success", 0);
        ind->pairing_config_req = pairing_config_cust_std_pair;
        return BT_STATUS_SUCCESS;
    }
#elif defined(APP_SWIFT_PAIR_LE_EDR_SECURE_MODE)
    if (conn_info != NULL && (conn_info->link_type & BT_GAP_LE_SRV_LINK_TYPE_SWIFT_PAIR) > 0) {
        LOG_MSGID_I(BT_APP, "[SWIFT_PAIR] get_pairing_config, LE/EDR_SECURE success", 0);
        ind->pairing_config_req = pairing_config_swift_pair;
        return BT_STATUS_SUCCESS;
    }
#endif
#endif

#ifdef AIR_SPEAKER_ENABLE
    ind->pairing_config_req = pairing_ass_config;
#else
    ind->pairing_config_req = pairing_config_deafult;
#endif
    return BT_STATUS_SUCCESS;
}

bt_gap_le_local_key_t *bt_app_common_get_local_key_by_handle(bt_handle_t handle)
{
    LOG_MSGID_I(BT_APP, "[APP][COMMON] get local key by handle = %02x", 2, handle);
    bt_gap_le_srv_conn_info_t *conn_info = bt_gap_le_srv_get_conn_info(handle);
    if (conn_info == NULL) {
        return NULL;
    }
    LOG_MSGID_I(BT_APP, "[APP][COMMON] get local key by handle = %02x link type = %02x attribute = %02x", 3, handle, conn_info->link_type, conn_info->attribute);
    if ((conn_info->link_type & BT_GAP_LE_SRV_LINK_TYPE_SWIFT_PAIR) > 0) {
        return &g_specific_local_key[0].local_key;
    }
    return NULL;
}

static bt_status_t bt_app_common_event_callback_register(void)
{
    LOG_MSGID_I(BT_APP, "bt_app_common_event_callback_register: enter\r\n", 1);
#if defined(MTK_AWS_MCE_ENABLE) && defined(SUPPORT_ROLE_HANDOVER_SERVICE)
    bt_role_handover_callbacks_t callbacks = {0};
    callbacks.allowed_cb = bt_app_common_role_handover_service_allowed;
    callbacks.get_len_cb = bt_app_common_role_handover_service_get_length;
    callbacks.get_data_cb = bt_app_common_role_handover_service_get_data;
    callbacks.update_cb = bt_app_common_role_handover_service_update;
    callbacks.status_cb = bt_app_common_role_handover_service_status_callback;
    bt_role_handover_register_callbacks(BT_ROLE_HANDOVER_MODULE_BLE_APP, &callbacks);
#endif
#if defined(MTK_AWS_MCE_ENABLE) && !defined(AIR_SPEAKER_ENABLE)
    bt_aws_mce_report_register_callback(BT_AWS_MCE_REPORT_MODULE_BLE_APP, bt_app_common_aws_data_received_ind_handler);
#endif

    bt_callback_manager_register_callback(bt_callback_type_app_event, MODULE_MASK_GAP | MODULE_MASK_SYSTEM, (void *)bt_app_common_event_callback);
    bt_callback_manager_register_callback(bt_callback_type_gap_le_get_pairing_config, 0, (void *)bt_app_common_get_pairing_config);
    bt_callback_manager_register_callback(bt_callback_type_gap_le_get_local_key_by_handle, 0, (void *)bt_app_common_get_local_key_by_handle);
    bt_callback_manager_add_sdp_customized_record(&bt_spp_app_sdp_record);
    bt_callback_manager_add_sdp_customized_record(&bt_di_sdp_record);
    bt_device_manager_le_bonded_event_callback callback = bt_app_common_dm_le_event_callback;
    bt_device_manager_le_register_callback(callback);
#ifdef AIR_BT_HID_ENABLE
    extern const bt_sdps_record_t bt_hid_sdp_record_1;
    LOG_MSGID_I(BT_APP, "bt_app_common_event_callback_register: HID sdp added", 0);
#ifndef AIR_HEAD_TRACKER_ENABLE
    bt_callback_manager_add_sdp_customized_record(&bt_hid_sdp_record_1);
#endif
#endif
    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_app_common_gap_param_init(void)
{
    uint16_t appearance = BT_APP_COMMON_APPEARANCE;
#if 0
    bt_gatts_le_gap_ppcp_t   gap_conn_params;

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = 0x0120;
    gap_conn_params.max_conn_interval = 0x0130;
    gap_conn_params.slave_latency     = 0x00;
    gap_conn_params.supervision_timeout  = 0x0258;
#endif
    //if (0 != (memcmp(g_le_device_name, bt_gatts_service_get_gap_device_name(),strlen(g_le_device_name)))) {

    //}
    bt_gatts_service_set_gap_appearance(appearance);
    //bt_gatts_service_set_gap_ppcp(&gap_conn_params);
    bt_gatts_service_set_gap_central_address_resolution(0);

    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_app_common_security_param_init(void)
{
#ifdef MTK_NVDM_ENABLE
    nvkey_status_t status = NVKEY_STATUS_ERROR;
#endif

    bt_key_t temp_irk = {0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf };
    uint8_t buffer[BT_KEY_SIZE] = {0};
    uint32_t size = BT_KEY_SIZE;

    if (memcmp(local_key.identity_info.irk, temp_irk, size) == 0) {
#ifdef MTK_NVDM_ENABLE
        status = nvkey_read_data(NVID_APP_LE_LOCAL_IRK, buffer, &size);
        if (NVKEY_STATUS_OK == status) {
            memcpy(local_key.identity_info.irk, buffer, size);
        } else
#endif
        {
            size = BT_KEY_SIZE;
            bt_app_common_generate_random_key(buffer, size);
            memcpy(local_key.identity_info.irk, buffer, size);
            bt_app_common_store_local_irk((const uint8_t *)&buffer, size);
        }
        LOG_MSGID_I(BT_APP, "[BT]local irk:%x%x%x%x %x%x%x%x %x%x%x%x %x%x%x%x", 16,
                    buffer[0], buffer[1], buffer[2], buffer[3],
                    buffer[4], buffer[5], buffer[6], buffer[7],
                    buffer[8], buffer[9], buffer[10], buffer[11],
                    buffer[12], buffer[13], buffer[14], buffer[15]);

    }

    for (uint32_t i = 0; i < BT_APP_COMMON_LOCAL_KEY_MAX; i++) {
        size = BT_KEY_SIZE;
        if (memcmp(g_specific_local_key[i].local_key.identity_info.irk, temp_irk, size) == 0) {
#ifdef MTK_NVDM_ENABLE
            status = nvkey_read_data(NVID_APP_LE_SPECIFIC_LOCAL_IRK_1 + i, buffer, &size);
            if (NVKEY_STATUS_OK == status) {
                memcpy(g_specific_local_key[i].local_key.identity_info.irk, buffer, size);
            } else
#endif
           {
                bt_app_common_generate_random_key(buffer, BT_KEY_SIZE);
                memcpy(g_specific_local_key[i].local_key.identity_info.irk, buffer, BT_KEY_SIZE);
#ifdef MTK_NVDM_ENABLE
                status = nvkey_write_data(NVID_APP_LE_SPECIFIC_LOCAL_IRK_1 + i, buffer, BT_KEY_SIZE);
                if (NVKEY_STATUS_OK != status) {
                    LOG_MSGID_I(BT_APP, "[APP][COMMON] specific index = %02x local irk write nvkey fail = %02x", 2, i, status);
                } else {
                    LOG_MSGID_I(BT_APP, "[APP][COMMON] specific index = %02x local irk write nvkey success", 1, i);
                }
#endif
           }
           LOG_MSGID_I(BT_APP, "[APP][COMMON] specific index = %02x local irk:%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x", 17, i,
                        buffer[0], buffer[1], buffer[2], buffer[3],
                        buffer[4], buffer[5], buffer[6], buffer[7],
                        buffer[8], buffer[9], buffer[10], buffer[11],
                        buffer[12], buffer[13], buffer[14], buffer[15]);
        }
    }

    bt_device_manager_le_gap_set_local_configuration(&local_key, sc_only);
    return BT_STATUS_SUCCESS;
}

uint8_t *bt_app_common_get_ble_local_irk(void)
{
    return (uint8_t *)&local_key.identity_info.irk;
}

uint8_t *bt_app_common_get_local_irk_by_link_type(bt_gap_le_srv_link_t link_type)
{
    for (uint32_t i = 0; i < BT_APP_COMMON_LOCAL_KEY_MAX; i++) {
        if ((g_specific_local_key[i].link_type & link_type) > 0) {
            LOG_MSGID_I(BT_APP, "[APP][COMMON] get local irk by link type = %02x success", 1, link_type);
            return (uint8_t *)&g_specific_local_key[i].local_key.identity_info.irk;
        }
    }
    LOG_MSGID_I(BT_APP, "[APP][COMMON] get local irk by link type = %02x fail", 1, link_type);
    return (uint8_t *)&local_key.identity_info.irk;
}

bool bt_app_common_bond_le_ass(bt_handle_t handle)
{
#ifdef AIR_SPEAKER_ENABLE
    bt_status_t bt_status = bt_gap_le_bond(handle, &pairing_ass_config);
    return (bt_status == BT_STATUS_SUCCESS);
#else
    return FALSE;
#endif
}

static void apps_bt_services_init(void)
{
    LOG_MSGID_I(BT_APP, "bt services init.", 0);
#if defined(MTK_AWS_MCE_ENABLE)
    bt_aws_mce_report_init();
#endif

    // Init bt connection manager.

    bt_cm_init(bt_customer_config_get_cm_config());
#ifdef MTK_AWS_MCE_ROLE_RECOVERY_ENABLE
    bt_aws_mce_role_recovery_init();
#endif
    //bt_connection_manager_init(bt_customer_config_get_bt_connection_manager_features());
    // Init bt sink service.
    LOG_MSGID_I(BT_APP, "bt sink features: 0x%x.", 1, bt_customer_config_get_bt_sink_features()->features);
    bt_sink_srv_init(bt_customer_config_get_bt_sink_features());

    bt_device_manager_link_record_init();
#ifndef AIR_MULTI_POINT_ENABLE
    bt_device_manager_link_record_set_max_num(1);
#endif
#ifdef MTK_IAP2_PROFILE_ENABLE
#ifndef MTK_IAP2_VIA_MUX_ENABLE
    serial_port_iap2_register_callbacks();
#else
    mux_iap2_register_callbacks();
#endif
#endif

}

void bt_app_common_init(void)
{
    bt_gap_le_srv_config_t le_srv_config = {
        .max_advertising_num = 5,
        .max_connection_num = 4
    };
    bt_device_manager_le_init();
    bt_gattc_discovery_init();
    bt_gap_le_srv_init(&le_srv_config);
#ifdef AIR_GATT_SRV_CLIENT_ENABLE
    bt_gatt_srv_client_init();
#endif
#ifdef __BLE_FMP__
    ble_fmp_server_init();
#endif
#ifdef MTK_BLE_BAS
    ble_bas_app_init();
#endif

    bt_app_common_generate_device_name();
    bt_app_common_gap_param_init();

    bt_app_common_security_param_init();
    bt_app_common_event_callback_register();

    bt_app_comm_at_cmd_init();
    apps_bt_services_init();
#ifdef MTK_PORT_SERVICE_BT_ENABLE
    ble_air_main();
    bt_spp_air_main();
#ifdef MTK_AIRUPDATE_ENABLE
    airupdate_main();
#endif
#endif

#ifdef AIR_BLE_HRS_ENABLE
    ble_hrs_app_init();
#endif

#if defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
    app_lea_service_init();
#ifdef AIR_HEADSET_ENABLE
#ifdef AIR_LE_AUDIO_ENABLE
    /* only for headset */
    {
        bt_key_t sirk = {0}, zero = {0};
        ble_csis_get_sirk(&sirk);
        /* if SIRK is all zero, generate it */
        if (0 == memcmp(zero, sirk, sizeof(bt_key_t))) {
            bt_app_common_generate_random_key((uint8_t *)sirk, sizeof(bt_key_t));
            ble_csis_set_sirk(sirk);
            ble_csis_write_nvkey_sirk(&sirk);
        }
    }
#endif
#endif
#endif
    bt_gatts_service_init();
#ifdef RACE_AWS_ENABLE
    race_app_aws_init();
#endif
}

void bt_app_common_reset_context(void)
{
    //LE connection should be destroyed when BT power off.
    memset(&connection_cb, 0, sizeof(connection_cb));

    //Advertisng will stopped when power off happened.
    bt_app_adv_ongoing = BT_APP_COMMON_BLE_ADV_STOPPED;

    g_le_connection_handle = 0;
    g_bt_cm_power_standby_flag = false;
#if defined(SUPPORT_ROLE_HANDOVER_SERVICE)
    bt_app_adv_restart_flag = false;
#endif
}

#if defined(MTK_AWS_MCE_ENABLE) && !defined(AIR_SPEAKER_ENABLE)

void bt_app_common_sync_random_addr(void)
{
    bt_aws_mce_report_info_t report_info = {0};
    uint8_t len = sizeof(bt_bd_addr_t) + 1;//add 1 byte for event type
    uint8_t *aws_packet = (uint8_t *)pvPortMalloc(len);
    if (aws_packet != NULL) {
        report_info.module_id = BT_AWS_MCE_REPORT_MODULE_BLE_APP;
        report_info.param_len = len;
        report_info.param = (void *)aws_packet;
        aws_packet[0] = BT_APP_COMMON_EVENT_SYNC_RANDOM_ADDR;

#if defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
#ifdef AIR_LE_AUDIO_DUALMODE_ENABLE
        if (app_lea_service_is_enable_dual_mode())
            memcpy(aws_packet + 1, bt_device_manager_aws_local_info_get_fixed_address(), sizeof(bt_bd_addr_t));
        else {
            bt_gap_le_advertising_handle_t adv_handle = 0;
            if (!multi_ble_adv_manager_get_random_addr_and_adv_handle(MULTI_ADV_INSTANCE_NOT_RHO, (bt_bd_addr_t *)&aws_packet[1], &adv_handle)) {
                LOG_MSGID_I(BT_APP, "bt_app_common_sync_random_addr, get address fail", 0);
                vPortFree(aws_packet);
                return;
            }
        }
#else
        bt_gap_le_advertising_handle_t adv_handle = 0;
        if (!multi_ble_adv_manager_get_random_addr_and_adv_handle(MULTI_ADV_INSTANCE_NOT_RHO, (bt_bd_addr_t *)&aws_packet[1], &adv_handle)) {
            LOG_MSGID_I(BT_APP, "bt_app_common_sync_random_addr, get address fail", 0);
            vPortFree(aws_packet);
            return;
        }
#endif
#else
        memcpy(aws_packet + 1, &bt_local_random_addr, sizeof(bt_bd_addr_t));
#endif

        bt_status_t ret = bt_aws_mce_report_send_event(&report_info);
        LOG_MSGID_I(BT_APP, "bt_app_common_sync_random_addr len 0x%x, ret %d\n", 2, len, ret);
        vPortFree(aws_packet);
    } else {
        LOG_MSGID_I(BT_APP, "bt_app_common_sync_random_addr OOM, len 0x%x", 1, len);
    }
}

static void bt_app_common_sync_local_key(void)
{
    bt_aws_mce_report_info_t report_info = {0};
    uint8_t len = sizeof(bt_gap_le_local_key_t) + 1;//add 1 byte for event type
    uint8_t *aws_packet = (uint8_t *)pvPortMalloc(len);
    if (aws_packet != NULL) {
        report_info.module_id = BT_AWS_MCE_REPORT_MODULE_BLE_APP;
        report_info.param_len = len;
        report_info.param = (void *)aws_packet;
        aws_packet[0] = BT_APP_COMMON_EVENT_SYNC_LOCAL_KEY;
        memcpy(aws_packet + 1, &local_key, sizeof(bt_gap_le_local_key_t));
        bt_status_t ret = bt_aws_mce_report_send_event(&report_info);
        LOG_MSGID_I(BT_APP, "bt_app_common_sync_local_key len 0x%x, ret %d\n", 2, len, ret);
        vPortFree(aws_packet);
    } else {
        LOG_MSGID_I(BT_APP, "bt_app_common_sync_local_key OOM, len 0x%x", 1, len);
    }
}

#ifdef AIR_LE_AUDIO_ENABLE
static void bt_app_common_sync_sirk(bt_key_t sirk)
{
    bt_aws_mce_report_info_t report_info = {0};
    uint8_t len = sizeof(bt_key_t) + 1;//add 1 byte for event type
    uint8_t *aws_packet = (uint8_t *)pvPortMalloc(len);
    if (aws_packet != NULL) {
        report_info.module_id = BT_AWS_MCE_REPORT_MODULE_BLE_APP;
        report_info.param_len = len;
        report_info.param = (void *)aws_packet;
        aws_packet[0] = BT_APP_COMMON_EVENT_SYNC_SIRK;
        memcpy(aws_packet + 1, sirk, sizeof(bt_key_t));
        bt_status_t ret = bt_aws_mce_report_send_event(&report_info);
        LOG_MSGID_I(BT_APP, "bt_app_common_sync_sirk len 0x%x, ret %d\n", 2, len, ret);
        vPortFree(aws_packet);
    } else {
        LOG_MSGID_I(BT_APP, "bt_app_common_sync_sirk OOM, len 0x%x", 1, len);
    }
}
#endif

extern bool bt_device_manager_le_is_sync_bond_info(bt_addr_t *bt_addr);
extern bt_gap_le_srv_link_t bt_device_manager_le_get_link_type_by_addr(bt_bd_addr_t *remote_addr);
extern bt_status_t bt_device_manager_le_set_link_type_by_addr(bt_addr_t *address, bt_gap_le_srv_link_t link_type);
static void bt_app_common_sync_bonded_info(bt_bd_addr_t *addr_list, bt_addr_type_t *type_list, uint8_t count, bt_device_manager_le_bonded_event_t event)
{
#if 0
    bt_aws_mce_report_info_t report_info = {0};
#ifdef MTK_AWS_MCE_ENABLE
    uint8_t len = sizeof(bt_device_manager_le_sync_packet_t) + 1;
#else
    uint8_t len = sizeof(bt_device_manager_le_bonded_info_t) + 1;
#endif
    uint8_t *aws_packet = (uint8_t *)pvPortMalloc(len);
    bool is_sync_flag = false;
    if (aws_packet != NULL) {
        uint16_t i = 0;
        report_info.module_id = BT_AWS_MCE_REPORT_MODULE_BLE_APP;
        report_info.param_len = len;
        report_info.param = (void *)aws_packet;
        for (i = 0; i < count; i++) {
            aws_packet[0] = event;
            bt_bd_addr_t *debug_addr = addr_list + i;
            switch (event) {
                case BT_DEVICE_MANAGER_LE_BONDED_ADD: {
#ifdef MTK_AWS_MCE_ENABLE
                    bt_device_manager_le_sync_packet_t dm_sync_info;
                    bt_device_manager_le_bonded_info_t *p_bonded_info = bt_device_manager_le_get_bonding_info_by_addr_ext(addr_list + i);
                    if (p_bonded_info != NULL) {
                        if (!bt_device_manager_le_is_sync_bond_info(&p_bonded_info->bt_addr)) {
                            LOG_MSGID_I(BT_APP, "[APP][COMMON] Not need sync bond info to Partner", 0);
                            is_sync_flag = false;
                            break;
                        }
                        is_sync_flag = true;
                        dm_sync_info.link_type = bt_device_manager_le_get_link_type_by_addr(addr_list + i);
                        memcpy(&dm_sync_info.bonded_info, p_bonded_info, sizeof(bt_device_manager_le_bonded_info_t));
                        memcpy(aws_packet + 1, &dm_sync_info, sizeof(bt_device_manager_le_sync_packet_t));
                    } else {
                        LOG_MSGID_I(BT_APP, "bt_app_common_sync_bonded_info get bonded info fail", 0);
                    }
#else
                bt_device_manager_le_bonded_info_t *infos = bt_device_manager_le_get_bonding_info_by_addr_ext(addr_list + i);
                if (infos != NULL) {
                    if (!bt_device_manager_le_is_sync_bond_info(&infos->bt_addr)) {
                        LOG_MSGID_I(BT_APP, "[APP][COMMON] Not need sync bond info to Partner", 0);
                        is_sync_flag = false;
                        break;
                    }
                    is_sync_flag = true;
                    memcpy(aws_packet + 1, infos, sizeof(bt_device_manager_le_bonded_info_t));
                } else {
                    LOG_MSGID_I(BT_APP, "bt_app_common_sync_bonded_info get bonded info fail", 0);
                }
#endif
                }
                break;
                case BT_DEVICE_MANAGER_LE_BONDED_REMOVE:
                    report_info.param_len = sizeof(bt_addr_t) + 1;// 1 byte for action event (add/remove/clear)
                    aws_packet[1] = type_list[i];
                    memcpy(aws_packet + 2, addr_list + i, sizeof(bt_bd_addr_t));
                    break;
                case BT_DEVICE_MANAGER_LE_BONDED_CLEAR:
                    report_info.param_len = 1;// 1 byte for action event (add/remove/clear)
                    debug_addr = NULL;
                    break;
            }
            bt_status_t ret = BT_STATUS_FAIL;
            if (is_sync_flag) {
                ret = bt_aws_mce_report_send_event(&report_info);
            }
            if (debug_addr != NULL) {
                LOG_MSGID_I(BT_APP, "bt_app_common_sync_bonded_info event 0x%x, len 0x%x, ret 0x%x, addr:%02X:%02X:%02X:%02X:%02X:%02X", 9,
                            event, report_info.param_len, ret,
                            (*debug_addr)[5],
                            (*debug_addr)[4],
                            (*debug_addr)[3],
                            (*debug_addr)[2],
                            (*debug_addr)[1],
                            (*debug_addr)[0]);
            } else {
                LOG_MSGID_I(BT_APP, "bt_app_common_sync_bonded_info event 0x%x, len 0x%x, ret 0x%x", 3, event, report_info.param_len, ret);
            }
        }
        vPortFree(aws_packet);
    } else {
        LOG_MSGID_I(BT_APP, "bt_app_common_sync_bonded_info OOM, len 0x%x", 1, len);
    }
#endif
}

static void bt_app_common_sync_all_bonded_info()
{
    uint8_t count = bt_device_manager_le_get_bonded_number();
    uint8_t len = count * sizeof(bt_bd_addr_t);
    LOG_MSGID_I(BT_APP, "bt_app_common_sync_all_bonded_info, len 0x%x, count %d", 2, len, count);
    bt_bd_addr_t *addr_list = (bt_bd_addr_t *)pvPortMalloc(len);
    if (addr_list != NULL) {
        bt_device_manager_le_get_bonded_list(addr_list, &count);
        bt_app_common_sync_bonded_info(addr_list, NULL, count, BT_DEVICE_MANAGER_LE_BONDED_ADD);
        vPortFree(addr_list);
    } else {
        LOG_MSGID_I(BT_APP, "bt_app_common_sync_all_bonded_info OOM", 0);
    }
}

static void bt_app_common_aws_data_received_ind_handler(bt_aws_mce_report_info_t *event)
{
    uint8_t *aws_data = (uint8_t *)event->param;
    if (event->module_id == BT_AWS_MCE_REPORT_MODULE_BLE_APP) {
        bt_app_common_event_sync_t event_type = aws_data[0];
        bt_bd_addr_t peer_addr;
        bt_bd_addr_t *debug_addr = &peer_addr;
        switch (event_type) {
//#ifndef AIR_LE_AUDIO_ENABLE
#if !(defined(AIR_LE_AUDIO_ENABLE) && defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE))
            case BT_APP_COMMON_EVENT_SYNC_RANDOM_ADDR:
                memcpy(&bt_aws_agent_random_addr, aws_data + 1, sizeof(bt_bd_addr_t));
                debug_addr = &bt_aws_agent_random_addr;
                if (0 != memcmp(&bt_aws_agent_random_addr, &bt_local_random_addr, sizeof(bt_bd_addr_t))) {
                    bt_gap_le_set_random_address((bt_bd_addr_ptr_t)bt_aws_agent_random_addr);
                    bt_app_common_store_local_random_address(&bt_aws_agent_random_addr);
                }
                break;
#endif
#if !(defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE))
            case BT_APP_COMMON_EVENT_SYNC_LOCAL_KEY:
                if (0 != memcmp(&local_key, aws_data + 1, sizeof(bt_gap_le_local_key_t))) {
                    memcpy(&local_key, aws_data + 1, sizeof(bt_gap_le_local_key_t));
                    bt_app_common_store_local_irk((const uint8_t *) & (local_key.identity_info.irk), BT_KEY_SIZE);
                    bt_device_manager_le_gap_set_local_configuration(&local_key, sc_only);
                }
                break;
#endif
            case BT_APP_COMMON_EVENT_SYNC_BOND_INFO_ADD: {
#ifdef MTK_AWS_MCE_ENABLE
                bt_device_manager_le_sync_packet_t sync_data;
                memcpy(&sync_data, aws_data + 1, sizeof(bt_device_manager_le_sync_packet_t));
                bt_device_manager_le_bonded_info_t *bond_info_data = &sync_data.bonded_info;
                bt_gap_le_srv_link_t link_type = sync_data.link_type;

                memcpy(debug_addr, &(bond_info_data->bt_addr.addr), sizeof(bt_bd_addr_t));
                bt_status_t status = bt_device_manager_le_set_bonding_info_by_addr(&(bond_info_data->bt_addr), &(bond_info_data->info));
                if (status != BT_STATUS_SUCCESS) {
                    LOG_MSGID_I(BT_APP, "bt_app_common_aws_data_received_ind_handler, set bonded info fail", 0);
                }
                status = bt_device_manager_le_set_link_type_by_addr(&(bond_info_data->bt_addr), link_type);
                if (status != BT_STATUS_SUCCESS) {
                    LOG_MSGID_I(BT_APP, "bt_app_common_aws_data_received_ind_handler, set link type fail", 0);
                }
#else
            bt_device_manager_le_bonded_info_t data;
            memcpy(&data, aws_data + 1, sizeof(bt_device_manager_le_bonded_info_t));
            memcpy(debug_addr, &(data.bt_addr.addr), sizeof(bt_bd_addr_t));
            bt_status_t status = bt_device_manager_le_set_bonding_info_by_addr(&(data.bt_addr), &(data.info));
            if (status != BT_STATUS_SUCCESS) {
                LOG_MSGID_I(BT_APP, "bt_app_common_aws_data_received_ind_handler, set bonded info fail", 0);
            }
#endif
            }
            break;
            case BT_APP_COMMON_EVENT_SYNC_BOND_INFO_REMOVE: {
                bt_addr_t addr;
                memcpy(&addr, aws_data + 1, sizeof(bt_addr_t));
                memcpy(debug_addr, aws_data + 2, sizeof(bt_bd_addr_t));
                bt_device_manager_le_remove_bonded_device(&addr);
            }
            break;
            case BT_APP_COMMON_EVENT_SYNC_BOND_INFO_CLEAR:
                bt_device_manager_le_clear_all_bonded_info();
                debug_addr = NULL;
                break;
#ifdef AIR_LE_AUDIO_ENABLE
            case BT_APP_COMMON_EVENT_SYNC_SIRK: {
                LOG_MSGID_I(BT_APP, "bt_app_common_aws_data_received_ind_handler harry0527 BT_APP_COMMON_EVENT_SYNC_SIRK 0x%x", 1, event_type);
                bt_key_t sirk = {0};
                memcpy(sirk, aws_data + 1, sizeof(bt_key_t));
                ble_csis_set_sirk(sirk);
                ble_csis_write_nvkey_sirk(&sirk);
                break;
            }
            break;
#endif
            default:
                LOG_MSGID_I(BT_APP, "bt_app_common_aws_data_received_ind_handler invalid event type 0x%x", 1, event_type);
                break;
        }

#if defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
        if (BT_APP_COMMON_EVENT_SYNC_RANDOM_ADDR == event_type) {
#ifdef AIR_LE_AUDIO_DUALMODE_ENABLE
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
            app_ull_le_set_group_device_addr(BT_ADDR_PUBLIC, aws_data + 1);
#endif

#ifdef AIR_LE_AUDIO_ENABLE
        if (app_lea_service_is_enable_dual_mode())
            app_le_audio_dhss_set_peer_le_addr(BT_ADDR_PUBLIC, aws_data + 1);
        else
            app_le_audio_dhss_set_peer_le_addr(BT_ADDR_RANDOM, aws_data + 1);
#endif
#else //#ifdef AIR_LE_AUDIO_DUALMODE_ENABLE

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
            app_ull_le_set_group_device_addr(BT_ADDR_RANDOM, aws_data + 1);
#endif

#ifdef AIR_LE_AUDIO_ENABLE
        if (BT_APP_COMMON_EVENT_SYNC_RANDOM_ADDR == event_type) {
            app_le_audio_dhss_set_peer_le_addr(BT_ADDR_RANDOM, aws_data + 1);
        }
#endif
#endif
        }
#endif
        if (debug_addr != NULL) {
            LOG_MSGID_I(BT_APP, "bt_app_common_aws_data_received_ind_handler, event 0x%x, addr:%02X:%02X:%02X:%02X:%02X:%02X", 7,
                        event_type,
                        (*debug_addr)[5],
                        (*debug_addr)[4],
                        (*debug_addr)[3],
                        (*debug_addr)[2],
                        (*debug_addr)[1],
                        (*debug_addr)[0]);
        } else {
            LOG_MSGID_I(BT_APP, "bt_app_common_aws_data_received_ind_handler, event 0x%x", 1, event_type);
        }

    }
}
#endif /* MTK_AWS_MCE_ENABLE */

bool bt_app_common_sink_event_proc(uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;
#if defined(MTK_AWS_MCE_ENABLE)
    //bt_sink_srv_event_param_t *event = (bt_sink_srv_event_param_t *)extra_data;

    switch (event_id) {
        default: {
            break;
        }
    }
#endif
    return ret;
}

void bt_app_common_set_ctkd_config()
{
    bt_device_manager_le_gap_set_pairing_configuration(&pairing_config_ctkd);
}

bool bt_app_common_cm_event_proc(uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;
#ifdef MTK_AWS_MCE_ENABLE
    bt_aws_mce_role_t role = bt_device_manager_aws_local_info_get_role();

    if ((BT_CM_EVENT_REMOTE_INFO_UPDATE == event_id) && (NULL != extra_data)) {
        bt_cm_remote_info_update_ind_t *remote_update = (bt_cm_remote_info_update_ind_t *)extra_data;

        if (BT_AWS_MCE_ROLE_AGENT == role) {
            if ((!(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service))
                && (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service)) {
#if !defined(AIR_SPEAKER_ENABLE)
                bt_app_common_sync_random_addr();
                bt_app_common_sync_local_key();
                bt_app_common_sync_all_bonded_info();
#if (defined(MTK_AWS_MCE_ENABLE) && defined(MTK_RACE_CMD_ENABLE))
                uint8_t temp_touch_key_status = 0;
                temp_touch_key_status = apps_get_touch_control_status();
                LOG_MSGID_I(BT_APP, "App_pre_pro agent get touch_key_status=0x%02X", 1, temp_touch_key_status);
                bt_status_t send_state = apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                                                        APPS_EVENTS_INTERACTION_SYNC_TOUCH_KEY_STATUS, &temp_touch_key_status, sizeof(uint8_t));
                if (BT_STATUS_SUCCESS == send_state) {
                    LOG_MSGID_I(BT_APP, "App_pre_pro agent sync touch key status to partner success.", 0);
                } else if (BT_STATUS_FAIL == send_state) {
                    LOG_MSGID_I(BT_APP, "App_pre_pro agent sync touch key status to partner fail.", 0);
                }
#endif
#ifdef AIR_LE_AUDIO_ENABLE
                {
                    bt_key_t sirk = {0}, zero = {0},fixbugsirk={0x00,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55};
                    ble_csis_get_sirk(&sirk);
                    /* if SIRK is all zero, generate it */
                    if(0 == memcmp(fixbugsirk, sirk, sizeof(bt_key_t)))
                    {
                        LOG_MSGID_I(BT_APP, "App_pre_pro sirk=0x555555555,rewrite random harry0527 ", 0);
                    }
                    if ((0 == memcmp(zero, sirk, sizeof(bt_key_t)))||(0 == memcmp(fixbugsirk, sirk, sizeof(bt_key_t)))) {
                        bt_app_common_generate_random_key((uint8_t *)sirk, sizeof(bt_key_t));
                        LOG_MSGID_I(BT_APP, "App_pre_pro bt_app_common_generate_random_key harry0527 ", 0);
                        ble_csis_set_sirk(sirk);
                        ble_csis_write_nvkey_sirk(&sirk);
                    }
                    bt_app_common_sync_sirk(sirk);
                }
#endif
#endif /*#if !defined(AIR_SPEAKER_ENABLE)*/
            }
            if (!remote_update->pre_acl_state && remote_update->acl_state) {
                bt_app_common_generate_device_name();
            }
        }
#if (defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)) && !defined(AIR_SPEAKER_ENABLE)
        /* SYNC parnter address to agent (DHSS use) */
        else if (BT_AWS_MCE_ROLE_PARTNER == role) {
            if (!(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service)
                && (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service)) {
                bt_app_common_sync_random_addr();
            }
        }
#endif
    }

#endif
    return ret;
}

#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
static uint16_t s_bt_app_ull_value[BT_APP_COMMON_ULL_LATENCY_MODULE_MAX] = {
    0,
};
void bt_app_common_pre_set_ultra_low_latency_retry_count(bt_app_common_ull_latency_module_t module, bt_app_common_ull_stream_retry_count_t retry_count)
{
    if (module < BT_APP_COMMON_ULL_LATENCY_MODULE_MAX && s_bt_app_ull_value[module] != retry_count) {
        LOG_MSGID_I(BT_APP, "bt_app_common_pre_set_ultra_low_latency_retry_count[%d] %d->%d", 3, module, s_bt_app_ull_value[module], retry_count);
        s_bt_app_ull_value[module] = retry_count;
    }
}

void bt_app_common_apply_ultra_low_latency_retry_count(void)
{
#ifndef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
    uint16_t retry_count = 0;
    uint32_t i;
    for (i = 0; i < BT_APP_COMMON_ULL_LATENCY_MODULE_MAX; i++) {
        if (s_bt_app_ull_value[i] > retry_count) {
            retry_count = s_bt_app_ull_value[i];
        }
    }
    if (retry_count > 0) {
        /* The latency must be larger when multi link connected */
        if (retry_count < BT_APP_COMMON_ULL_STREAM_RETRY_COUNT_FOR_MULTI_LINK && bt_cm_get_connected_devices(~BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS), NULL, 0) > 1) {
            retry_count = BT_APP_COMMON_ULL_STREAM_RETRY_COUNT_FOR_MULTI_LINK;
        }
        LOG_MSGID_I(BT_APP, "bt_app_common_apply_ultra_low_latency_retry_count %d", 1, retry_count);
        bt_ull_latency_t set_latency;
        set_latency.streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
        set_latency.streaming.port = 0;
        set_latency.latency = (retry_count * 5 + 1) / 2 + 15; /* Basic latency is 15ms, and every time retry sending use 2.5ms. Round up. */
        bt_ull_action(BT_ULL_ACTION_SET_STREAMING_LATENCY, &set_latency, sizeof(set_latency));
    }
#else
    extern bool app_ull_is_le_ull_connected(void);
    extern bool app_ull_is_multi_link_mode(void);

    uint8_t max_latency = BT_ULL_LE_SRV_LATENCY_DEFAULT;
    for (uint32_t i = 0; i < BT_APP_COMMON_ULL_LATENCY_MODULE_MAX; i++) {
        if (s_bt_app_ull_value[i] > max_latency) {
            max_latency = s_bt_app_ull_value[i];
        }
    }
    if (app_ull_is_le_ull_connected() && app_ull_is_multi_link_mode()) {
        if (max_latency < BT_ULL_LE_SRV_LATENCY_MULTI_LINK_HFP_STANDBY_MODE) {
            max_latency = BT_ULL_LE_SRV_LATENCY_MULTI_LINK_HFP_STANDBY_MODE;
        }
    }
    LOG_MSGID_I(BT_APP, "bt_app_common_apply_ultra_low_latency_retry_count %d", 1, max_latency);
    bt_ull_latency_t set_latency;
    set_latency.streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
    set_latency.streaming.port = 0;
    set_latency.latency = max_latency;
    bt_ull_action(BT_ULL_ACTION_SET_STREAMING_LATENCY, &set_latency, sizeof(set_latency));
#endif
}

#endif

bt_status_t bt_app_common_set_pairing_config_io_capability(bt_gap_le_srv_link_t link_type, bt_gap_le_smp_io_capability_t io_capability)
{
#if defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
    switch (link_type) {
        case BT_GAP_LE_SRV_LINK_TYPE_LE_AUDIO: 
        case BT_GAP_LE_SRV_LINK_TYPE_FAST_PAIR: {
#ifdef MTK_AWS_MCE_ENABLE
            bt_aws_mce_role_t role = bt_device_manager_aws_local_info_get_role();
            if (BT_AWS_MCE_ROLE_PARTNER == role) {
                pairing_config_le_audio_secondary.io_capability = io_capability;
            } else
#endif
            {
                pairing_config_le_audio.io_capability = io_capability;
            }
        }
        break;
        default:
            return BT_STATUS_FAIL;
    }
#endif //AIR_LE_AUDIO_ENABLE
    return BT_STATUS_SUCCESS;
}

bt_status_t bt_app_common_set_pairing_distribute_ctkd(bt_gap_le_srv_link_t link_type, bool is_ctkd_support)
{
    switch (link_type) {
        case BT_GAP_LE_SRV_LINK_TYPE_CUST_PAIR: {
#if defined(AIR_SWIFT_PAIR_ENABLE) && defined(AIR_CUST_PAIR_ENABLE)
            if (is_ctkd_support) {
                pairing_config_cust_ext_pair.initiator_key_distribution |= BT_GAP_LE_SMP_KEY_DISTRIBUTE_LINK;
                pairing_config_cust_ext_pair.responder_key_distribution |= BT_GAP_LE_SMP_KEY_DISTRIBUTE_LINK;
            } else {
                pairing_config_cust_ext_pair.initiator_key_distribution &= ~BT_GAP_LE_SMP_KEY_DISTRIBUTE_LINK;
                pairing_config_cust_ext_pair.responder_key_distribution &= ~BT_GAP_LE_SMP_KEY_DISTRIBUTE_LINK;
            }
#endif
        }
        break;
        default:
            return BT_STATUS_FAIL;
    }
    return BT_STATUS_SUCCESS;
}

#include "avm_external.h"
void bt_app_common_read_link_quality(bt_app_common_link_quality_type_t type, void* param)
{
    if (param == NULL && type != BT_APP_COMMON_LINK_QUALITY_BT_CRC) {
        return;
    }
    switch (type) {
        case BT_APP_COMMON_LINK_QUALITY_SPP_RSSI: {
            uint32_t handle = *(uint32_t*)param;
            const bt_bd_addr_t *addr = bt_spp_get_bd_addr_by_handle(handle);
            if (!addr) {
                return;
            }

            bt_gap_connection_handle_t gap_handle = bt_gap_get_handle_by_address((const bt_bd_addr_t*) addr);
            bt_gap_read_rssi(gap_handle);
            break;
        }
        case BT_APP_COMMON_LINK_QUALITY_BLE_RSSI: {
            bt_handle_t handle = *(bt_handle_t*)param;
            bt_hci_cmd_read_rssi_t read_rssi = {
                .handle = handle,
            };
            bt_gap_le_read_rssi(&read_rssi);
            break;
        }
        case BT_APP_COMMON_LINK_QUALITY_BT_CRC: {
            ESCO_ISO_STATISTIC_STRU *esco_stru = (ESCO_ISO_STATISTIC_STRU *)bt_pka_get_esco_iso_statistic();
            uint8_t esco_rate = 0;
            if (esco_stru->esco_packet_cnt != 0) {
                esco_rate = 0xff - esco_stru->esco_bad_packet_cnt * 0xff / (esco_stru->esco_packet_cnt + esco_stru->esco_bad_packet_cnt);
            }
            uint8_t iso_rate = 0;
            if (esco_stru->iso_packet_cnt != 0) {
                iso_rate = 0xff - esco_stru->iso_bad_packet_cnt * 0xff / (esco_stru->iso_packet_cnt + esco_stru->iso_bad_packet_cnt);
            }
            LOG_MSGID_I(BT_APP, "esco bad pkg=%d, good pkg cnt=%d, iso bad pkg=%d, good pkg cnt=%d, q1=%d, q2=%d", 6,
                        esco_stru->esco_bad_packet_cnt,
                        esco_stru->esco_packet_cnt,
                        esco_stru->iso_bad_packet_cnt,
                        esco_stru->iso_packet_cnt,
                        esco_rate, iso_rate);
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_ESCO_CRC_RATE, (void*)(esco_rate << 8 | iso_rate), 0,
                                NULL, 0);
            bt_pka_clear_esco_iso_statistic();
            break;
        }
    }
}



