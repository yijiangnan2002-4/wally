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

#if defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)

#include "app_lea_service_adv_mgr.h"

#include "app_lea_service.h"
#include "app_lea_service_conn_mgr.h"
#include "app_lea_service_event.h"
#include "app_lea_service_sync_info.h"
#include "app_lea_service_target_addr.h"

#ifdef AIR_LE_AUDIO_ENABLE
#include "ble_ascs_def.h"
#include "ble_bass_def.h"
#include "ble_cas_def.h"
#include "ble_csis.h"
#include "ble_pacs.h"
#include "bt_gap_le_audio.h"
#include "app_le_audio.h"
#ifdef AIR_LE_AUDIO_GMAP_ENABLE
#include "ble_gmas_def.h"
#endif
#endif
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
#include "bt_ull_le.h"
#include "bt_ull_le_service.h"
#include "apps_customer_config.h"
#endif

#include "bt_hci.h"
#include "bt_sink_srv.h"
#include "bt_sink_srv_le.h"
#include "bt_sink_srv_le_cap_audio_manager.h"

#ifdef MTK_AWS_MCE_ENABLE
#include "bt_aws_mce_srv.h"
#include "apps_aws_sync_event.h"
#endif
#ifdef AIR_SPEAKER_ENABLE
#include "app_speaker_srv.h"
#endif

#include "app_bt_state_service.h"
#include "apps_debug.h"
#include "apps_events_bt_event.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"
#include "apps_config_features_dynamic_setting.h"
#include "multi_ble_adv_manager.h"

#include "bt_connection_manager.h"
#include "bt_connection_manager_internal.h"
#include "bt_customer_config.h"
#include "bt_device_manager.h"
#include "ui_shell_manager.h"
#include "FreeRTOS.h"

#include "app_bt_conn_manager.h"
#include "bt_sink_srv_ami.h"
#ifdef AIR_SMART_CHARGER_ENABLE
#include "app_smcharger.h"
#endif
#ifdef AIR_LE_AUDIO_HAPS_ENABLE
#include "ble_haps.h"
#endif



#define LOG_TAG     "[LEA][ADV]"

typedef enum {
    APP_LEA_ADV_ERROR_OK                       = 0,
    APP_LEA_ADV_ERROR_LEA_DISABLE,
    APP_LEA_ADV_ERROR_MP_TEST_MODE,
    APP_LEA_ADV_ERROR_MAX_LINK,
    APP_LEA_ADV_ERROR_MAX_SUPPORT_LINK,
    APP_LEA_ADV_ERROR_ADV_MODE,
    APP_LEA_ADV_ERROR_NO_TARGET_ADDR,
    APP_LEA_ADV_ERROR_WIRED_AUDIO,
    APP_LEA_ADV_ERROR_WIRELESS_MIC_DISALLOW,
    APP_LEA_ADV_ERROR_APP_CONN_DISALLOW,
    APP_LEA_ADV_ERROR_UPDATE_STOP,
    APP_LEA_ADV_ERROR_CHARGER_LID_CLOSE,
    APP_LEA_ADV_ERROR_SPEAKER_MODE_SWITCHING,
    APP_LEA_ADV_ERROR_SPEAKER_BROADCAST_MODE,
    APP_LEA_ADV_ERROR_SPEAKER_DOUBLE_MODE,
    APP_LEA_ADV_ERROR_TWS_AIR_PAIRING,
} app_lea_adv_error_code_t;

typedef struct {
    uint8_t     mode;
    uint32_t    timeout;
} app_lea_adv_param_t;

#define APP_LE_AUDIO_RSI_LENGTH             6

#ifdef AIR_HEADSET_ENABLE
#define APP_LEA_AD_TYPE_APPEARANCE BT_SIG_AD_TYPE_APPEARANCE_HEADSET
#elif defined(AIR_SPEAKER_ENABLE)
#define APP_LEA_AD_TYPE_APPEARANCE BT_SIG_AD_TYPE_APPEARANCE_SPEAKER
#else
#define APP_LEA_AD_TYPE_APPEARANCE BT_SIG_AD_TYPE_APPEARANCE_EARBUD
#endif

static bool         app_lea_adv_enabled = FALSE;
static uint32_t     app_lea_adv_timeout = APP_LE_AUDIO_ADV_TIME;   /* 0 - always */

static uint16_t     app_lea_adv_interval_min = APP_LE_AUDIO_ADV_INTERVAL_MIN_S;
static uint16_t     app_lea_adv_interval_max = APP_LE_AUDIO_ADV_INTERVAL_MAX_S;

static uint32_t     app_lea_adv_time_tick = 0;

typedef enum {
    APP_LEA_ADV_TIMEOUT_ACTION_STOP                 = 0,
    APP_LEA_ADV_TIMEOUT_ACTION_START,
} app_lea_adv_timeout_action_t;

static uint8_t      app_lea_adv_mode = APP_LEA_ADV_MODE_NONE;

typedef struct {
    uint32_t        sub_mode_bitmask;
    uint8_t         sub_mode;
    uint8_t         sub_mode_num;
    uint8_t         index;
    uint8_t         direct_index;
    uint8_t         direct_num;
    bool            targeted_flag_on_general;
} PACKED app_lea_adv_mgr_context_t;
static app_lea_adv_mgr_context_t  app_lea_adv_mgr_ctx = {0};

#ifdef AIR_LE_AUDIO_ENABLE
extern ble_tmap_role_t ble_tmas_get_role(void);
uint8_t app_lea_adv_cas_announcement_type           = 0xFF;
#endif
#if defined(AIR_LE_AUDIO_ENABLE) && defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
bool                app_lea_adv_add_lea_data        = FALSE;
bool                app_lea_adv_add_ull_data        = FALSE;
#elif defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
bool                app_lea_adv_add_ull_data        = FALSE;
#endif
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
// Note: After Dongle and device both enter ULL pairing mode, dongle will only scan pair_mode=1 ADV and connect it
#define APP_LEA_ULL_ADV_PAIR_MODE_LEN               2
bool                app_lea_adv_ull_pair_mode       = FALSE;
bool                app_lea_adv_ull_reconnect_mode  = FALSE;
#endif
#if defined(AIR_WIRELESS_MIC_ENABLE)
#define APP_WIRELESS_MIC_AA                         (0xE898EB6D)
#endif

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
static const uint8_t APPS_ULL2_128_BIT_UUID[] = APPS_ULL2_128_BIT_UUID_DEF;
#endif

#pragma weak app_lea_service_adv_customer_data = default_app_lea_service_adv_customer_data
void app_lea_service_adv_customer_data(uint8_t *adv_data, uint8_t *len);
void default_app_lea_service_adv_customer_data(uint8_t *adv_data, uint8_t *len)
{
    //APPS_LOG_MSGID_E(LOG_TAG" default_app_lea_service_adv_customer_data", 0);
}



/**================================================================================*/
/**                                   Internal API                                 */
/**================================================================================*/
extern bt_status_t bt_gatts_service_set_le_audio_device_name(const uint8_t *device_name, uint16_t length);
static void app_lea_adv_mgr_do_stop_advertising(void);

static bool app_lea_adv_mgr_is_enter_slow_phase(void)
{
    if (app_lea_adv_time_tick == 0) {
        return TRUE;
    }

    uint32_t start_tick = app_lea_adv_time_tick;
    uint32_t end_tick = xTaskGetTickCount();
    uint32_t duration_tick = 0;
    if (end_tick > start_tick) {
        duration_tick = end_tick - start_tick;
    } else {
        duration_tick = (0xFFFFFFFF - (start_tick - end_tick)) + 1;
    }
    return (duration_tick >= APP_LE_AUDIO_ADV_FAST_TIME);
}

#ifdef AIR_LE_AUDIO_DIRECT_ADV
static uint8_t app_lea_adv_mgr_convert_addr_type(bt_addr_type_t addr_type)
{
    uint8_t direct_adv_addr_type = 0xFF;       // 0xFF is invalid value
    if (addr_type == BT_ADDR_PUBLIC || addr_type == BT_ADDR_PUBLIC_IDENTITY) {
        direct_adv_addr_type = 0;
    } else if (addr_type == BT_ADDR_RANDOM || addr_type == BT_ADDR_RANDOM_IDENTITY) {
        direct_adv_addr_type = 1;
    }
    return direct_adv_addr_type;
}
#endif

static bool app_lea_adv_mgr_is_esco_ongoing(void)
{
    bool ret = FALSE;
    bt_sink_srv_device_state_t state_list[3] = {0};
    bt_sink_srv_get_device_state(NULL, state_list, 3);
    for (int i = 0; i < 3; i++) {
        if (state_list[i].sco_state == BT_SINK_SRV_SCO_CONNECTION_STATE_CONNECTED) {
            ret = TRUE;
            break;
        }
    }
    return ret;
}

static uint8_t app_lea_adv_mgr_check_adv_allow(uint8_t mode)
{
    uint8_t error_code = APP_LEA_ADV_ERROR_OK;
    uint8_t cur_conn_num = app_lea_conn_mgr_get_conn_num();
    uint8_t support_max_conn_num = app_lea_conn_mgr_get_support_max_conn_num();
    if (apps_config_features_is_mp_test_mode()) {
        error_code = APP_LEA_ADV_ERROR_MP_TEST_MODE;
    } else if (cur_conn_num == APP_LEA_MAX_CONN_NUM) {
        error_code = APP_LEA_ADV_ERROR_MAX_LINK;
    } else if (cur_conn_num == support_max_conn_num) {
        error_code = APP_LEA_ADV_ERROR_MAX_SUPPORT_LINK;
    } else if (mode == APP_LEA_ADV_MODE_NONE) {
        error_code = APP_LEA_ADV_ERROR_ADV_MODE;
    } else if (!app_bt_conn_manager_allow_le_adv()) {
        error_code = APP_LEA_ADV_ERROR_APP_CONN_DISALLOW;
    }
#ifdef AIR_SMART_CHARGER_ENABLE
    app_smcharger_state charger_state = app_smcharger_get_state();
    if (charger_state == STATE_SMCHARGER_LID_CLOSE || charger_state == STATE_SMCHARGER_OFF) {
        error_code = APP_LEA_ADV_ERROR_CHARGER_LID_CLOSE;
    }
#endif
#if defined(AIR_LE_AUDIO_ENABLE) && !defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
    if (app_lea_feature_mode == APP_LEA_FEATURE_MODE_OFF) {
        error_code = APP_LEA_ADV_ERROR_LEA_DISABLE;
    }
#endif
#ifdef APP_LE_AUDIO_ADV_STOP_ADV_WHEN_WIRED_AUDIO
    extern bool app_le_audio_is_wired_audio(void);
    if (app_le_audio_is_wired_audio()) {
        error_code = APP_LEA_ADV_ERROR_WIRED_AUDIO;
    }
#endif
#ifdef AIR_WIRELESS_MIC_ENABLE  // Headset project
    extern bool app_le_ull_adv_disable_by_cmd(void);
    if (app_le_ull_adv_disable_by_cmd()) {
        error_code = APP_LEA_ADV_ERROR_WIRELESS_MIC_DISALLOW;
    }
#endif
#ifdef AIR_SPEAKER_ENABLE
    bt_aws_mce_srv_mode_t aws_mode = bt_aws_mce_srv_get_mode();
    if (app_speaker_is_switch_mode_ongoing()) {
        error_code = APP_LEA_ADV_ERROR_SPEAKER_MODE_SWITCHING;
    } else if (aws_mode == BT_AWS_MCE_SRV_MODE_BROADCAST) {
        error_code = APP_LEA_ADV_ERROR_SPEAKER_BROADCAST_MODE;
    }
#ifdef MTK_BT_SPEAKER_DISABLE_DOUBLE_LEA_CIS
    // Disable LEA ADV (CIS) on double mode
    else if (aws_mode == BT_AWS_MCE_SRV_MODE_DOUBLE) {
        error_code = APP_LEA_ADV_ERROR_SPEAKER_DOUBLE_MODE;
    }
#endif
#endif
#ifdef AIR_TWS_ENABLE
    uint8_t zero_addr[BT_BD_ADDR_LEN] = {0};
    bt_bd_addr_t *bd_addr = bt_device_manager_aws_local_info_get_peer_address();
    uint8_t *aws_peer_addr = (uint8_t *)(*bd_addr);
    if (aws_peer_addr != NULL && memcmp(aws_peer_addr, zero_addr, BT_BD_ADDR_LEN) == 0) {
        error_code = APP_LEA_ADV_ERROR_TWS_AIR_PAIRING;
    }
#endif

    return error_code;
}

#ifdef AIR_LE_AUDIO_ENABLE
static void app_lea_adv_mgr_copy_classic_name(char *device_name, const char *classic_name)
{
    int classic_name_len = strlen(classic_name);
    int prefix_len = strlen(APP_LEA_ADV_NAME_PREFIX);
    memcpy(device_name, APP_LEA_ADV_NAME_PREFIX, prefix_len);

    uint8_t max_len_bt_name = BT_GAP_LE_MAX_DEVICE_NAME_LENGTH - 1 - prefix_len;
    uint8_t lea_bt_name = (classic_name_len > max_len_bt_name ? max_len_bt_name : classic_name_len);
    memcpy(device_name + prefix_len, classic_name, lea_bt_name);
}
#endif

static uint32_t app_lea_adv_mgr_get_adv_data_internal(bool want_lea_targeted_flag, multi_ble_adv_info_t *adv_data)
{
#if defined(AIR_WIRELESS_MIC_ENABLE)
    if (adv_data->adv_param != NULL && adv_data->adv_data != NULL && adv_data->scan_rsp != NULL) {
        bt_ull_le_set_adv_scan_access_addr_t aa = {0};
        uint32_t aa_param = APP_WIRELESS_MIC_AA;
        memcpy(aa.acess_addr, (void *)&aa_param, BT_ULL_LE_ACCESS_ADDR_LENGTH);
        bt_status_t bt_status = bt_ull_le_srv_set_access_address(&aa);
        APPS_LOG_MSGID_I(LOG_TAG"[WIRELESS_MIC] get_adv_data, set AA 0x%08X", 1, bt_status);
    }
#endif

    /* SCAN RSP */
    if (NULL != adv_data->scan_rsp) {
        adv_data->scan_rsp->data_length = 0;
    }

    /* ADV DATA */
#ifdef AIR_LE_AUDIO_DUALMODE_ENABLE
    bool is_enable_dual_mode = app_lea_service_is_enable_dual_mode();
#endif
    if ((NULL != adv_data->adv_data) && (NULL != adv_data->adv_data->data)) {
        uint8_t len = 0;
        bool need_flag = TRUE;
        // bool discoverable = (app_lea_adv_mode == APP_LEA_ADV_MODE_GENERAL);// app_bt_service_is_visible();
                bool discoverable = app_bt_service_is_visible();  // jira207

#if defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
        uint8_t rsi[APP_LE_AUDIO_RSI_LENGTH] = {0};
#endif
        adv_data->adv_data->data[len] = 2;
        adv_data->adv_data->data[len + 1] = BT_GAP_LE_AD_TYPE_FLAG;
#ifdef AIR_LE_AUDIO_DUALMODE_ENABLE
        if (is_enable_dual_mode) {
#ifdef MTK_AWS_MCE_ENABLE
            /* BR/EDR is only supported on primary earbud */
            if (app_le_audio_is_primary_earbud()) {
                (discoverable) ? (adv_data->adv_data->data[len + 2] = BT_GAP_LE_AD_FLAG_GENERAL_DISCOVERABLE) : (need_flag = FALSE);
            } else {
                adv_data->adv_data->data[len + 2] = BT_GAP_LE_AD_FLAG_BR_EDR_NOT_SUPPORTED;
            }
#else
            (discoverable) ? (adv_data->adv_data->data[len + 2] = BT_GAP_LE_AD_FLAG_GENERAL_DISCOVERABLE) : (need_flag = FALSE);
#endif
        } else if (discoverable) {
            adv_data->adv_data->data[len + 2] = BT_GAP_LE_AD_FLAG_BR_EDR_NOT_SUPPORTED | BT_GAP_LE_AD_FLAG_GENERAL_DISCOVERABLE;
        } else {
            adv_data->adv_data->data[len + 2] = BT_GAP_LE_AD_FLAG_BR_EDR_NOT_SUPPORTED;
        }
#else
        if (discoverable) {
            adv_data->adv_data->data[len + 2] = BT_GAP_LE_AD_FLAG_BR_EDR_NOT_SUPPORTED | BT_GAP_LE_AD_FLAG_GENERAL_DISCOVERABLE;
        } else {
            adv_data->adv_data->data[len + 2] = BT_GAP_LE_AD_FLAG_BR_EDR_NOT_SUPPORTED;
        }
#endif
        if (need_flag) {
            len += 3;
        }

#ifdef AIR_LE_AUDIO_ENABLE
        uint8_t announcement_type = ANNOUNCEMENT_TYPE_GENERAL;
        if (want_lea_targeted_flag) {
            announcement_type = ANNOUNCEMENT_TYPE_TARGETED;
        }

        uint16_t sink_conent = 0;
        uint16_t source_conent = 0;
        ble_pacs_get_available_audio_contexts(&sink_conent, &source_conent);

        /* adv_data: RSI */
        adv_data->adv_data->data[len] = 7;
        adv_data->adv_data->data[len + 1] = BT_GAP_LE_AD_TYPE_RSI;
        ble_csis_get_rsi(rsi);
        memcpy(&adv_data->adv_data->data[len + 2], rsi, sizeof(rsi));
        len += 8;

#ifdef AIR_LE_AUDIO_HAPS_ENABLE
        /* adv_data: AD_TYPE_SERVICE_DATA (HAPS)*/
        adv_data->adv_data->data[len] = 3;
        adv_data->adv_data->data[len + 1] = BT_GAP_LE_AD_TYPE_SERVICE_DATA;
        /* HAPS UUID: 2 bytes */
        adv_data->adv_data->data[len + 2] = (BT_GATT_UUID16_HAS_SERVICE & 0x00FF);
        adv_data->adv_data->data[len + 3] = ((BT_GATT_UUID16_HAS_SERVICE & 0xFF00) >> 8);
        len += 4;
#endif

        /* adv_data: AD_TYPE_SERVICE_DATA (BAP)*/
        adv_data->adv_data->data[len] = 9;
        adv_data->adv_data->data[len + 1] = BT_GAP_LE_AD_TYPE_SERVICE_DATA;
        adv_data->adv_data->data[len + 2] = (BT_GATT_UUID16_ASCS_SERVICE & 0x00FF);
        adv_data->adv_data->data[len + 3] = ((BT_GATT_UUID16_ASCS_SERVICE & 0xFF00) >> 8);
        adv_data->adv_data->data[len + 4] = announcement_type;
        memcpy(&adv_data->adv_data->data[len + 5], &sink_conent, 2);
        memcpy(&adv_data->adv_data->data[len + 7], &source_conent, 2);
        adv_data->adv_data->data[len + 9] = 0x00; /* Length of the Metadata field = 0 */
        len += 10;

        /* adv_data: TX_POWER (BAP)*/
        adv_data->adv_data->data[len] = 2;
        adv_data->adv_data->data[len + 1] = BT_GAP_LE_AD_TYPE_TX_POWER;
        adv_data->adv_data->data[len + 2] = 0x7F;
        len += 3;

        /* adv_data: AD_TYPE_APPEARANCE (TMAP) */
        adv_data->adv_data->data[len] = 3;
        adv_data->adv_data->data[len + 1] = BT_GAP_LE_AD_TYPE_APPEARANCE;
        /* value: 2 bytes */
        adv_data->adv_data->data[len + 2] = (APP_LEA_AD_TYPE_APPEARANCE & 0x00FF);
        adv_data->adv_data->data[len + 3] = ((APP_LEA_AD_TYPE_APPEARANCE & 0xFF00) >> 8);
        len += 4;

        /* adv_data: AD_TYPE_SERVICE_DATA (TMAS)*/
        adv_data->adv_data->data[len] = 5;
        adv_data->adv_data->data[len + 1] = BT_GAP_LE_AD_TYPE_SERVICE_DATA;
        /* TMAS UUID: 2 bytes */
        adv_data->adv_data->data[len + 2] = (BT_SIG_UUID16_TMAS & 0x00FF);
        adv_data->adv_data->data[len + 3] = ((BT_SIG_UUID16_TMAS & 0xFF00) >> 8);
        /* TMAS Data: 2 bytes */
        ble_tmap_role_t tmas_role = ble_tmas_get_role();
        adv_data->adv_data->data[len + 4] = (tmas_role & 0x00FF);
        adv_data->adv_data->data[len + 5] = ((tmas_role & 0xFF00) >> 8);
        len += 6;
#ifdef AIR_LE_AUDIO_GMAP_ENABLE
        /* adv_data: AD_TYPE_SERVICE_DATA (GMAS)*/
        adv_data->adv_data->data[len] = 4;
        adv_data->adv_data->data[len + 1] = BT_GAP_LE_AD_TYPE_SERVICE_DATA;
        /* GMAS UUID: 2 bytes */
        adv_data->adv_data->data[len + 2] = (BT_SIG_UUID16_GMAS & 0x00FF);
        adv_data->adv_data->data[len + 3] = ((BT_SIG_UUID16_GMAS & 0xFF00) >> 8);
        /* GMAS Data: 1 bytes */
        adv_data->adv_data->data[len + 4] = BLE_GMAP_ROLE_MASK_UGT | BLE_GMAP_ROLE_MASK_BGR;
        len += 5;
#endif
        /* adv_data: AD_TYPE_SERVICE_DATA (BASS)*/
        adv_data->adv_data->data[len] = 3;
        adv_data->adv_data->data[len + 1] = BT_GAP_LE_AD_TYPE_SERVICE_DATA;
        /* BASS UUID: 2 bytes */
        adv_data->adv_data->data[len + 2] = (BT_SIG_UUID16_BASS & 0x00FF);
        adv_data->adv_data->data[len + 3] = ((BT_SIG_UUID16_BASS & 0xFF00) >> 8);
        len += 4;

        /* adv_data: AD_TYPE_SERVICE_DATA (CAS)*/
        adv_data->adv_data->data[len] = 4;
        adv_data->adv_data->data[len + 1] = BT_GAP_LE_AD_TYPE_SERVICE_DATA;
        /* CAS UUID: 2 bytes */
        adv_data->adv_data->data[len + 2] = (BT_SIG_UUID16_CAS & 0x00FF);
        adv_data->adv_data->data[len + 3] = ((BT_SIG_UUID16_CAS & 0xFF00) >> 8);
        adv_data->adv_data->data[len + 4] = announcement_type;
        if (app_lea_adv_cas_announcement_type != 0xFF) {
            /* Only For AT CMD Setting */
            adv_data->adv_data->data[len + 4] = app_lea_adv_cas_announcement_type;
            app_lea_adv_cas_announcement_type = 0xFF;
        }
        len += 5;

#ifdef AIR_LE_AUDIO_DUALMODE_ENABLE
        if (is_enable_dual_mode) {
            adv_data->adv_data->data[len] = 0x0B;
            adv_data->adv_data->data[len + 1] = BT_GAP_LE_AD_TYPE_16_BIT_UUID_PART;
            adv_data->adv_data->data[len + 2] = (BT_GATT_UUID16_ASCS_SERVICE & 0x00FF);
            adv_data->adv_data->data[len + 3] = ((BT_GATT_UUID16_ASCS_SERVICE & 0xFF00) >> 8);
            adv_data->adv_data->data[len + 4] = (BT_SIG_UUID16_BASS & 0x00FF);
            adv_data->adv_data->data[len + 5] = ((BT_SIG_UUID16_BASS & 0xFF00) >> 8);
            adv_data->adv_data->data[len + 6] = (BT_GATT_UUID16_PACS_SERVICE & 0x00FF);
            adv_data->adv_data->data[len + 7] = ((BT_GATT_UUID16_PACS_SERVICE & 0xFF00) >> 8);
            adv_data->adv_data->data[len + 8] = (BT_SIG_UUID16_VOLUME_CONTROL_SERVICE & 0x00FF);
            adv_data->adv_data->data[len + 9] = ((BT_SIG_UUID16_VOLUME_CONTROL_SERVICE & 0xFF00) >> 8);
            adv_data->adv_data->data[len + 10] = (BT_SIG_UUID16_MICS & 0x00FF);
            adv_data->adv_data->data[len + 11] = ((BT_SIG_UUID16_MICS & 0xFF00) >> 8);
            len += 12;
        }
#endif

        app_lea_service_adv_customer_data(adv_data->adv_data->data, &len);

        uint16_t device_name_len = 0;
        char device_name[BT_GAP_LE_MAX_DEVICE_NAME_LENGTH] = {0};
        const bt_gap_config_t *bt_config = bt_customer_config_get_gap_config();
        const char *classic_name = bt_config->device_name;
#ifdef AIR_LE_AUDIO_DUALMODE_ENABLE
        if (is_enable_dual_mode) {
            int classic_name_len = strlen(classic_name);
            memcpy(device_name, classic_name, classic_name_len);
        } else {
            app_lea_adv_mgr_copy_classic_name(device_name, classic_name);
        }
#else
        app_lea_adv_mgr_copy_classic_name(device_name, classic_name);
#endif

        if (0 != (device_name_len = strlen((char *)device_name))) {
            /* adv_data: AD_TYPE_NAME_COMPLETE*/
            adv_data->adv_data->data[len] = device_name_len + 1;
            adv_data->adv_data->data[len + 1] = BT_GAP_LE_AD_TYPE_NAME_COMPLETE;
            memcpy(&adv_data->adv_data->data[len + 2], device_name, device_name_len);
            len += 2 + device_name_len;

            /*set GASTT GAP service device name*/
            bt_gatts_service_set_le_audio_device_name((const uint8_t *)device_name, device_name_len);
        }

        adv_data->adv_data->data_length = len;
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
        if (!app_lea_adv_add_lea_data) {
            // Keep flag AD
            memset(adv_data->adv_data->data + 3, 0, len - 3);
            len = 3;
            adv_data->adv_data->data_length = len;
        }
#endif
#endif /* AIR_LE_AUDIO_ENABLE */

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
#ifdef AIR_LE_AUDIO_ENABLE        // LE AUDIO + LE ULL co-exist
        if (app_lea_adv_add_ull_data) {
#ifndef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
            if (!app_lea_adv_add_lea_data) {
                /* adv_data: RSI */
                adv_data->adv_data->data[len] = APP_LE_AUDIO_RSI_LENGTH + 1;
                adv_data->adv_data->data[len + 1] = BT_GAP_LE_AD_TYPE_RSI;
                bt_ull_le_srv_get_rsi(rsi);
                memcpy(&adv_data->adv_data->data[len + 2], rsi, sizeof(rsi));
                len += APP_LE_AUDIO_RSI_LENGTH + 2;
            }

            adv_data->adv_data->data[len] = BT_ULL_LE_MAX_UUID_LENGTH + 1;
            adv_data->adv_data->data[len + 1] = BT_GAP_LE_AD_TYPE_128_BIT_UUID_DATA;
            memcpy(&adv_data->adv_data->data[len + 2], APPS_ULL2_128_BIT_UUID, BT_ULL_LE_MAX_UUID_LENGTH);
            len += BT_ULL_LE_MAX_UUID_LENGTH + 2;

            adv_data->adv_data->data[len] = APP_LEA_ULL_ADV_PAIR_MODE_LEN + 3;
            adv_data->adv_data->data[len + 1] = BT_GAP_LE_AD_TYPE_MANUFACTURER_SPECIFIC;
            adv_data->adv_data->data[len + 2] = 0x94;
            adv_data->adv_data->data[len + 3] = 0x00;
            adv_data->adv_data->data[len + 4] = 0xFF;
            adv_data->adv_data->data[len + 5] = app_lea_adv_ull_pair_mode;
            len += APP_LEA_ULL_ADV_PAIR_MODE_LEN + 4;

            if (app_lea_adv_ull_reconnect_mode) {
                adv_data->adv_data->data[len] = APP_LEA_ULL_ADV_PAIR_MODE_LEN + 3;
                adv_data->adv_data->data[len + 1] = BT_GAP_LE_AD_TYPE_MANUFACTURER_SPECIFIC;
                adv_data->adv_data->data[len + 2] = 0x94;
                adv_data->adv_data->data[len + 3] = 0x00;
                adv_data->adv_data->data[len + 4] = 0xFF;
                adv_data->adv_data->data[len + 5] = 0x03;
                len += APP_LEA_ULL_ADV_PAIR_MODE_LEN + 4;
            }
#else
            if (app_lea_adv_add_ull_data) {
                /* ULL2 with HID uuid */
                adv_data->adv_data->data[len] = 3;
                adv_data->adv_data->data[len + 1] = 0x19;
                adv_data->adv_data->data[len + 2] = 0x81;
                adv_data->adv_data->data[len + 3] = 0x02;
                len += 4;
            }
#endif
            adv_data->adv_data->data_length = len;
        }
#else                             // Only LE ULL

#ifndef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
        /* adv_data: RSI */
        adv_data->adv_data->data[len] = APP_LE_AUDIO_RSI_LENGTH + 1;
        adv_data->adv_data->data[len + 1] = BT_GAP_LE_AD_TYPE_RSI;
        bt_ull_le_srv_get_rsi(rsi);
        memcpy(&adv_data->adv_data->data[len + 2], rsi, sizeof(rsi));
        len += APP_LE_AUDIO_RSI_LENGTH + 2;

        /* service data */
        adv_data->adv_data->data[len] = BT_ULL_LE_MAX_UUID_LENGTH + 1;
        adv_data->adv_data->data[len + 1] = BT_GAP_LE_AD_TYPE_128_BIT_UUID_DATA;
        memcpy(&adv_data->adv_data->data[len + 2], APPS_ULL2_128_BIT_UUID, BT_ULL_LE_MAX_UUID_LENGTH);
        len += BT_ULL_LE_MAX_UUID_LENGTH + 2;

        adv_data->adv_data->data[len] = APP_LEA_ULL_ADV_PAIR_MODE_LEN + 3;
        adv_data->adv_data->data[len + 1] = BT_GAP_LE_AD_TYPE_MANUFACTURER_SPECIFIC;
        adv_data->adv_data->data[len + 2] = 0x94;
        adv_data->adv_data->data[len + 3] = 0x00;
        adv_data->adv_data->data[len + 4] = 0xFF;
        adv_data->adv_data->data[len + 5] = app_lea_adv_ull_pair_mode;
        len += APP_LEA_ULL_ADV_PAIR_MODE_LEN + 4;

        if (app_lea_adv_ull_reconnect_mode) {
            adv_data->adv_data->data[len] = APP_LEA_ULL_ADV_PAIR_MODE_LEN + 3;
            adv_data->adv_data->data[len + 1] = BT_GAP_LE_AD_TYPE_MANUFACTURER_SPECIFIC;
            adv_data->adv_data->data[len + 2] = 0x94;
            adv_data->adv_data->data[len + 3] = 0x00;
            adv_data->adv_data->data[len + 4] = 0xFF;
            adv_data->adv_data->data[len + 5] = 0x03;
            len += APP_LEA_ULL_ADV_PAIR_MODE_LEN + 4;
        }
#else
        /* ULL2 with HID uuid */
        adv_data->adv_data->data[len] = 3;
        adv_data->adv_data->data[len + 1] = 0x19;
        adv_data->adv_data->data[len + 2] = 0x81;
        adv_data->adv_data->data[len + 3] = 0x02;
        len += 4;
#endif /* #ifndef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE */
        adv_data->adv_data->data_length = len;
#endif
#endif
    }

    uint16_t adv_interval_min = app_lea_adv_interval_min;
    uint16_t adv_interval_max = app_lea_adv_interval_max;
#if defined(AIR_WIRELESS_MIC_ENABLE) && defined(AIR_AUDIO_ULD_CODEC_ENABLE)
    adv_interval_min = APP_LE_AUDIO_ADV_INTERVAL_MIN;
    adv_interval_max = APP_LE_AUDIO_ADV_INTERVAL_MAX;
#else
    uint8_t sub_mode = app_lea_adv_mgr_ctx.sub_mode;
    if (sub_mode == APP_LEA_ADV_SUB_MODE_ACTIVE_RECONNECT || sub_mode == APP_LEA_ADV_SUB_MODE_DIRECT) {
        adv_interval_min = ((adv_interval_min <= APP_LE_AUDIO_ADV_INTERVAL_M) ? adv_interval_min : APP_LE_AUDIO_ADV_INTERVAL_M);
        adv_interval_max = ((adv_interval_max <= APP_LE_AUDIO_ADV_INTERVAL_M) ? adv_interval_max : APP_LE_AUDIO_ADV_INTERVAL_M);
    }
#endif

    if (NULL != adv_data->adv_param) {
        adv_data->adv_param->advertising_event_properties = BT_HCI_ADV_EVT_PROPERTIES_MASK_CONNECTABLE;
        /* Interval should be no larger than 100ms when discoverable */
        adv_data->adv_param->primary_advertising_interval_min = adv_interval_min;
        adv_data->adv_param->primary_advertising_interval_max = adv_interval_max;
        adv_data->adv_param->primary_advertising_channel_map = 0x07;
#ifdef AIR_LE_AUDIO_DUALMODE_ENABLE
        if (is_enable_dual_mode) {
#ifdef MTK_AWS_MCE_ENABLE
            adv_data->adv_param->own_address_type = BT_ADDR_LE_PUBLIC;
#else //#ifdef MTK_AWS_MCE_ENABLE
            adv_data->adv_param->own_address_type = BT_ADDR_PUBLIC;
#endif
        } else {
            adv_data->adv_param->own_address_type = BT_ADDR_RANDOM;
        }
#else //#ifdef AIR_LE_AUDIO_DUALMODE_ENABLE
        adv_data->adv_param->own_address_type = BT_ADDR_RANDOM;
#endif
        adv_data->adv_param->advertising_filter_policy = 0;
        adv_data->adv_param->advertising_tx_power = 0x7F;
        adv_data->adv_param->primary_advertising_phy = BT_HCI_LE_ADV_PHY_1M;
        adv_data->adv_param->secondary_advertising_phy = BT_HCI_LE_ADV_PHY_1M;
    }
    return 0;
}

static uint32_t app_lea_adv_mgr_get_adv_data(multi_ble_adv_info_t *adv_data)
{
#ifdef AIR_LE_AUDIO_ENABLE
    uint8_t sub_mode = app_lea_adv_mgr_ctx.sub_mode;
    bool want_lea_targeted_flag = FALSE;
    if (sub_mode == APP_LEA_ADV_SUB_MODE_ACTIVE_RECONNECT || sub_mode == APP_LEA_ADV_SUB_MODE_DIRECT) {
        want_lea_targeted_flag = TRUE;
    } else if (sub_mode == APP_LEA_ADV_SUB_MODE_GENERAL && app_lea_adv_mgr_ctx.targeted_flag_on_general) {
        want_lea_targeted_flag = TRUE;
    }

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
    if (sub_mode == APP_LEA_ADV_SUB_MODE_ACTIVE_RECONNECT) {
        uint8_t active_num = 0;
        app_lea_conn_mgr_get_reconnect_info(FALSE, NULL, &active_num, NULL);
        if (active_num == 1) {
            bt_addr_t addr_list[1] = {0};
            uint8_t list_num = 1;
            app_lea_conn_mgr_get_reconnect_addr(sub_mode, addr_list, &list_num);
            uint8_t conn_type = app_lea_conn_mgr_get_conn_type_by_addr(addr_list[0].addr);
            if (conn_type == APP_LEA_CONN_TYPE_LE_ULL) {
                //APPS_LOG_MSGID_E(LOG_TAG"[SUB_MODE] get_adv_data, ONE-ULL2 not targeted flag", 0);
                want_lea_targeted_flag = FALSE;
            }
        }
    }
#endif

    uint32_t ret = app_lea_adv_mgr_get_adv_data_internal(want_lea_targeted_flag, adv_data);

#ifdef AIR_LE_AUDIO_DIRECT_ADV
    if (sub_mode == APP_LEA_ADV_SUB_MODE_DIRECT && adv_data->adv_param != NULL) {
        bt_addr_t addr_list[APP_LEA_MAX_TARGET_NUM] = {0};
        uint8_t list_num = APP_LEA_MAX_TARGET_NUM;
        app_lea_conn_mgr_get_reconnect_addr(sub_mode, addr_list, &list_num);
        uint8_t index = app_lea_adv_mgr_ctx.direct_index;

        uint8_t direct_adv_addr_type = app_lea_adv_mgr_convert_addr_type(addr_list[index].type);
        adv_data->adv_param->advertising_event_properties |= BT_HCI_ADV_EVT_PROPERTIES_MASK_DIRECTED;
        adv_data->adv_param->advertising_event_properties &= (~BT_HCI_ADV_EVT_PROPERTIES_MASK_SCANNABLE);
        adv_data->adv_param->peer_address.type = direct_adv_addr_type;
        memcpy(adv_data->adv_param->peer_address.addr, addr_list[index].addr, sizeof(bt_bd_addr_t));
        bool is_enable_dual_mode = app_lea_service_is_enable_dual_mode();
        if (is_enable_dual_mode) {
            if (app_lea_conn_mgr_is_support_addr_resolution(addr_list[index].addr)) {
                adv_data->adv_param->own_address_type = BT_ADDR_RANDOM_IDENTITY; // ToDo, BT_ADDR_PUBLIC_IDENTITY
            } else {
#ifdef AIR_TWS_ENABLE
                adv_data->adv_param->own_address_type = BT_ADDR_LE_PUBLIC;
#else
                adv_data->adv_param->own_address_type = BT_ADDR_PUBLIC;
#endif
            }
        } else {
            adv_data->adv_param->own_address_type = BT_ADDR_RANDOM;
        }

//        if (adv_data->adv_data != NULL) {
//            adv_data->adv_data->data_length = 0;
//        }
        if (adv_data->scan_rsp != NULL) {
            adv_data->scan_rsp->data_length = 0;
        }
    }
#endif
    if (adv_data->adv_param != NULL) {
        if (sub_mode == APP_LEA_ADV_SUB_MODE_GENERAL) {
            adv_data->adv_param->advertising_filter_policy = BT_HCI_ADV_FILTER_ACCEPT_SCAN_CONNECT_FROM_ALL;
        } else if (sub_mode == APP_LEA_ADV_SUB_MODE_ACTIVE_RECONNECT) {
            adv_data->adv_param->advertising_filter_policy = BT_HCI_ADV_FILTER_ACCEPT_SCAN_CONNECT_IN_WHITE_LIST;
        } else if (sub_mode == APP_LEA_ADV_SUB_MODE_INACTIVE) {
            adv_data->adv_param->advertising_filter_policy = BT_HCI_ADV_FILTER_ACCEPT_SCAN_CONNECT_IN_WHITE_LIST;
        }
    }
#elif defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)     // Only ULL2 Enable
    uint32_t ret = app_lea_adv_mgr_get_adv_data_internal(FALSE, adv_data);
    if (adv_data->adv_param != NULL) {
        if (app_lea_adv_mode == APP_LEA_ADV_MODE_GENERAL) {
            adv_data->adv_param->advertising_filter_policy = BT_HCI_ADV_FILTER_ACCEPT_SCAN_CONNECT_FROM_ALL;
        } else if (app_lea_adv_mode == APP_LEA_ADV_MODE_TARGET_ALL) {
            adv_data->adv_param->advertising_filter_policy = BT_HCI_ADV_FILTER_ACCEPT_SCAN_CONNECT_IN_WHITE_LIST;
        } else if (app_lea_adv_mode == APP_LEA_ADV_MODE_TARGET) {
            adv_data->adv_param->advertising_filter_policy = BT_HCI_ADV_FILTER_ACCEPT_SCAN_CONNECT_IN_WHITE_LIST;
        }
    }
#endif

    return ret;
}

static uint32_t app_lea_adv_mgr_get_adv_data_common(multi_ble_adv_info_t *adv_data)
{
    return app_lea_adv_mgr_get_adv_data(adv_data);
}

#ifdef AIR_LE_AUDIO_ENABLE
static void app_lea_adv_mgr_change_adv_sub_mode(void)
{
    if (app_lea_adv_mgr_ctx.sub_mode == APP_LEA_ADV_SUB_MODE_DIRECT
        && app_lea_adv_mgr_ctx.direct_num > 1
        && app_lea_adv_mgr_ctx.direct_index < app_lea_adv_mgr_ctx.direct_num - 1) {
        app_lea_adv_mgr_ctx.direct_index++;
        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LEA_CHANGE_ADV_SUB_MODE);
        ui_shell_send_event(FALSE, EVENT_PRIORITY_MIDDLE,
                            EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LEA_CHANGE_ADV_SUB_MODE,
                            NULL, 0, NULL, APP_LE_AUDIO_ADV_GENERAL_FLAG_CHANGE_TIME);
        return;
    }

    app_lea_adv_mgr_ctx.index++;
    if (app_lea_adv_mgr_ctx.index == APP_LEA_ADV_SUB_MODE_MAX) {
        app_lea_adv_mgr_ctx.index = APP_LEA_ADV_SUB_MODE_GENERAL;
    }

    do {
        if (app_lea_adv_mgr_ctx.sub_mode_bitmask == 0) {
            APPS_LOG_MSGID_E(LOG_TAG" change_adv_sub_mode, bitmask=0", 0);
            break;
        }

        uint8_t sub_mode_bit = APP_LEA_ADV_SUB_MODE_MASK(app_lea_adv_mgr_ctx.index);
        if ((app_lea_adv_mgr_ctx.sub_mode_bitmask & sub_mode_bit) > 0) {
            app_lea_adv_mgr_ctx.sub_mode = app_lea_adv_mgr_ctx.index;
            app_lea_adv_mgr_ctx.direct_index = 0;

            if (app_lea_adv_mgr_is_enter_slow_phase()) {
                app_lea_adv_interval_min = APP_LE_AUDIO_ADV_INTERVAL_MIN_L;
                app_lea_adv_interval_max = APP_LE_AUDIO_ADV_INTERVAL_MAX_L;
            }

            ui_shell_remove_event(EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LEA_CHANGE_ADV_SUB_MODE);
            ui_shell_send_event(FALSE, EVENT_PRIORITY_MIDDLE,
                                EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LEA_CHANGE_ADV_SUB_MODE,
                                NULL, 0, NULL, APP_LE_AUDIO_ADV_GENERAL_FLAG_CHANGE_TIME);
            break;
        } else {
            app_lea_adv_mgr_ctx.index++;
        }
    } while (1);
}

static void app_lea_adv_mgr_init_adv_sub_mode(void)
{
    uint8_t direct_num = 0;
    uint8_t active_num = 0;
    uint8_t inactive_num = 0;
    app_lea_conn_mgr_get_reconnect_info(TRUE, &direct_num, &active_num, &inactive_num);

    uint8_t sub_mode_num = 0;
    memset(&app_lea_adv_mgr_ctx, 0, sizeof(app_lea_adv_mgr_context_t));

    if (app_lea_adv_mode == APP_LEA_ADV_MODE_GENERAL) {
        app_lea_adv_mgr_ctx.sub_mode_bitmask |= APP_LEA_ADV_SUB_MODE_MASK(APP_LEA_ADV_SUB_MODE_GENERAL);
        app_lea_adv_mgr_ctx.sub_mode = APP_LEA_ADV_SUB_MODE_GENERAL;
        sub_mode_num++;

        if (direct_num == 0 && active_num > 0) {
            app_lea_adv_mgr_ctx.targeted_flag_on_general = TRUE;
            APPS_LOG_MSGID_W(LOG_TAG"[SUB_MODE] init_adv_sub_mode, targeted_flag on general_adv", 0);
        } else {
            if (active_num > 0) {
                app_lea_adv_mgr_ctx.sub_mode_bitmask |= APP_LEA_ADV_SUB_MODE_MASK(APP_LEA_ADV_SUB_MODE_ACTIVE_RECONNECT);
                sub_mode_num++;
            }
            if (direct_num > 0) {
                app_lea_adv_mgr_ctx.sub_mode_bitmask |= APP_LEA_ADV_SUB_MODE_MASK(APP_LEA_ADV_SUB_MODE_DIRECT);
                sub_mode_num++;
                app_lea_adv_mgr_ctx.direct_num = direct_num;
            }
        }
    }

    if (app_lea_adv_mode == APP_LEA_ADV_MODE_TARGET || app_lea_adv_mode == APP_LEA_ADV_MODE_TARGET_ALL) {
        if (active_num > 0) {
            app_lea_adv_mgr_ctx.sub_mode_bitmask |= APP_LEA_ADV_SUB_MODE_MASK(APP_LEA_ADV_SUB_MODE_ACTIVE_RECONNECT);
            app_lea_adv_mgr_ctx.sub_mode = APP_LEA_ADV_SUB_MODE_ACTIVE_RECONNECT;
            sub_mode_num++;
        }
#ifdef AIR_LE_AUDIO_DIRECT_ADV
        if (app_lea_adv_mode == APP_LEA_ADV_MODE_TARGET_ALL && inactive_num > 0) {
#else
        // No direct ADV, Mix inactive/active to one "targeted announcement flag" ADV
        if (app_lea_adv_mode == APP_LEA_ADV_MODE_TARGET_ALL && inactive_num > 0 && active_num == 0) {
#endif
            app_lea_adv_mgr_ctx.sub_mode_bitmask |= APP_LEA_ADV_SUB_MODE_MASK(APP_LEA_ADV_SUB_MODE_INACTIVE);
            if (app_lea_adv_mgr_ctx.sub_mode == APP_LEA_ADV_SUB_MODE_NONE) {
                app_lea_adv_mgr_ctx.sub_mode = APP_LEA_ADV_SUB_MODE_INACTIVE;
            }
            sub_mode_num++;
        }
        if (direct_num > 0) {
            app_lea_adv_mgr_ctx.sub_mode_bitmask |= APP_LEA_ADV_SUB_MODE_MASK(APP_LEA_ADV_SUB_MODE_DIRECT);
            if (app_lea_adv_mgr_ctx.sub_mode == APP_LEA_ADV_SUB_MODE_NONE) {
                app_lea_adv_mgr_ctx.sub_mode = APP_LEA_ADV_SUB_MODE_DIRECT;
            }
            sub_mode_num++;
            app_lea_adv_mgr_ctx.direct_num = direct_num;
        }
    }

    app_lea_adv_mgr_ctx.sub_mode_num = sub_mode_num;

    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LEA_CHANGE_ADV_SUB_MODE);
    if (sub_mode_num > 1) {
        // Move to next valid index from index=0
        do {
            uint8_t sub_mode_bit = APP_LEA_ADV_SUB_MODE_MASK(app_lea_adv_mgr_ctx.index);
            if ((app_lea_adv_mgr_ctx.sub_mode_bitmask & sub_mode_bit) > 0) {
                break;
            } else {
                app_lea_adv_mgr_ctx.index++;
            }
        } while (1);

        ui_shell_send_event(FALSE, EVENT_PRIORITY_MIDDLE,
                            EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LEA_CHANGE_ADV_SUB_MODE,
                            NULL, 0, NULL, APP_LE_AUDIO_ADV_GENERAL_FLAG_CHANGE_TIME);
    } else if (sub_mode_num == 1 && app_lea_adv_mgr_ctx.sub_mode == APP_LEA_ADV_SUB_MODE_DIRECT && direct_num > 1) {
        ui_shell_send_event(FALSE, EVENT_PRIORITY_MIDDLE,
                            EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LEA_CHANGE_ADV_SUB_MODE,
                            NULL, 0, NULL, APP_LE_AUDIO_ADV_GENERAL_FLAG_CHANGE_TIME);
    }

    APPS_LOG_MSGID_W(LOG_TAG"[SUB_MODE] init_adv_sub_mode, sub_mode_bitmask=0x%04X index=%d sub_mode_num=%d",
                     3, app_lea_adv_mgr_ctx.sub_mode_bitmask, app_lea_adv_mgr_ctx.index, sub_mode_num);
}
#endif

static void app_lea_adv_mgr_update_multi_adv(bool update_white_list)
{
    if (update_white_list) {
#ifdef AIR_LE_AUDIO_ENABLE
        if (app_lea_adv_mode == APP_LEA_ADV_MODE_GENERAL) {
            app_lea_clear_target_addr(TRUE);
        }

        uint32_t sub_mode_bitmask = app_lea_adv_mgr_ctx.sub_mode_bitmask;
        if ((sub_mode_bitmask & APP_LEA_ADV_SUB_MODE_MASK(APP_LEA_ADV_SUB_MODE_INACTIVE)) > 0) {
            app_lea_update_target_add_white_list(FALSE);
        } else if ((sub_mode_bitmask & APP_LEA_ADV_SUB_MODE_MASK(APP_LEA_ADV_SUB_MODE_ACTIVE_RECONNECT)) > 0) {
            // Set all non-connected addr
            app_lea_update_target_add_white_list(FALSE);
        }
#elif defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)     // Only ULL2 Enable
        if (app_lea_adv_mode == APP_LEA_ADV_MODE_GENERAL) {
            app_lea_clear_target_addr(FALSE);
        } else if (app_lea_adv_mode == APP_LEA_ADV_MODE_TARGET_ALL || app_lea_adv_mode == APP_LEA_ADV_MODE_TARGET) {
            // ULL2 Only, all device are active_reconnect_type
            app_lea_update_target_add_white_list(TRUE);
        }
#endif
    }

    APPS_LOG_MSGID_W(LOG_TAG"[SUB_MODE] update_multi_adv, WL=%d bitmask=0x%04X sub_mode_num=%d index=%d sub_mode=%d direct=%d/%d",
                     7, update_white_list, app_lea_adv_mgr_ctx.sub_mode_bitmask, app_lea_adv_mgr_ctx.sub_mode_num,
                     app_lea_adv_mgr_ctx.index, app_lea_adv_mgr_ctx.sub_mode, app_lea_adv_mgr_ctx.direct_index,
                     app_lea_adv_mgr_ctx.direct_num);
    multi_ble_adv_manager_remove_ble_adv(MULTI_ADV_INSTANCE_NOT_RHO, app_lea_adv_mgr_get_adv_data_common);
    multi_ble_adv_manager_add_ble_adv(MULTI_ADV_INSTANCE_NOT_RHO, app_lea_adv_mgr_get_adv_data_common, 1);
    multi_ble_adv_manager_notify_ble_adv_data_changed(MULTI_ADV_INSTANCE_NOT_RHO);
}

static void app_lea_adv_mgr_do_start_advertising(uint8_t mode, uint32_t timeout)
{
    uint8_t old_mode = app_lea_adv_mode;
    bool is_reset_general_adv = FALSE;
    bool visible = app_bt_service_is_visible();
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    APPS_LOG_MSGID_I(LOG_TAG" do_start_advertising, [%02X] start adv_mode=%d->%d timeout=%d visible=%d",
                     5, role, old_mode, mode, timeout, visible);

#if defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
    if (app_lea_adv_ull_pair_mode && mode == APP_LEA_ADV_MODE_TARGET_ALL) {
        APPS_LOG_MSGID_E(LOG_TAG" start_advertising, disable TARGET_ALL for ULL2 pairing mode %d",
                         1, app_lea_adv_mode);
        if (app_lea_adv_mode == APP_LEA_ADV_MODE_GENERAL) {
            return;
        } else {
            mode = APP_LEA_ADV_MODE_GENERAL;
            timeout = 0;
        }
    }
#endif

#if defined(AIR_WIRELESS_MIC_ENABLE)
    if (mode == APP_LEA_ADV_MODE_TARGET_ALL) {
        mode = APP_LEA_ADV_MODE_GENERAL;
        timeout = 0;
        APPS_LOG_MSGID_W(LOG_TAG" do_start_advertising, [%02X] Target_All -> General", 1, role);
    }
#endif

    if (mode == APP_LEA_ADV_MODE_TARGET) {
        uint8_t direct_num = 0;
        uint8_t active_num = 0;
        app_lea_conn_mgr_get_reconnect_info(TRUE, &direct_num, &active_num, NULL);
        if (direct_num == 0 && active_num == 0) {
            mode = APP_LEA_ADV_MODE_TARGET_ALL;
            timeout = 0;
            APPS_LOG_MSGID_W(LOG_TAG" do_start_advertising, no active_reconnect_type, Target -> Target_All", 0);
        }
    }

    if (app_lea_adv_mode == APP_LEA_ADV_MODE_GENERAL && visible && mode != APP_LEA_ADV_MODE_TARGET) {
        if (mode == APP_LEA_ADV_MODE_GENERAL && timeout == APP_LE_AUDIO_ADV_TIME
            && app_lea_adv_timeout == APP_LE_AUDIO_TEMP_GENERAL_ADV_TIME) {
            app_lea_adv_timeout = timeout;
            uint8_t adv_timeout_action = APP_LEA_ADV_TIMEOUT_ACTION_START;
            ui_shell_remove_event(EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LE_AUDIO_ADV_TIMER);
            if (app_lea_adv_timeout > 0) {
                ui_shell_send_event(FALSE, EVENT_PRIORITY_MIDDLE,
                                    EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LE_AUDIO_ADV_TIMER,
                                    (void *)(int)adv_timeout_action, 0, NULL, app_lea_adv_timeout);
            }
            APPS_LOG_MSGID_W(LOG_TAG" start_advertising, switch mode fail from GENERAL Mode timeout=%d",
                             1, timeout);
            return;
        } else {
            APPS_LOG_MSGID_W(LOG_TAG" start_advertising, reset GENERAL Mode %d", 1, app_lea_adv_timeout);
            is_reset_general_adv = TRUE;
            mode = APP_LEA_ADV_MODE_GENERAL;
            timeout = app_lea_adv_timeout;
        }
    }

    app_lea_adv_error_code_t error_code = APP_LEA_ADV_ERROR_OK;
    uint8_t adv_timeout_action = APP_LEA_ADV_TIMEOUT_ACTION_STOP;

    if (mode == APP_LEA_ADV_MODE_GENERAL) {
        adv_timeout_action = APP_LEA_ADV_TIMEOUT_ACTION_START;
    } else if (mode == APP_LEA_ADV_MODE_TARGET) {
        // app_bt_conn_mgr_reconnect_complete->start->remove EVENT_ID_LE_AUDIO_ADV_TIMER when TARGET Mode (Power on reconnect) complete or timeout
        adv_timeout_action = APP_LEA_ADV_TIMEOUT_ACTION_START;
    } else if (mode == APP_LEA_ADV_MODE_TARGET_ALL) {
        if (!app_lea_conn_mgr_is_need_reconnect_adv()) {
            APPS_LOG_MSGID_E(LOG_TAG" do_start_advertising, bond_info empty or already connected", 0);
            app_lea_adv_mgr_do_stop_advertising();
            return;
        }
    }

    error_code = app_lea_adv_mgr_check_adv_allow(mode);
    if (error_code != APP_LEA_ADV_ERROR_OK) {
        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LEA_CANCEL_ADV_RECONNECT_FLAG);
        uint8_t cur_conn_num = app_lea_conn_mgr_get_conn_num();
        uint8_t support_max_conn_num = app_lea_conn_mgr_get_support_max_conn_num();
        APPS_LOG_MSGID_E(LOG_TAG" do_start_advertising, error_code=%d conn_num=%d %d %d",
                         4, error_code, cur_conn_num, APP_LEA_MAX_CONN_NUM, support_max_conn_num);
        return;
    }

    app_lea_adv_mode = mode;

#ifdef AIR_LE_AUDIO_ENABLE
    if (mode == APP_LEA_ADV_MODE_TARGET
        || (old_mode != APP_LEA_ADV_MODE_GENERAL && mode == APP_LEA_ADV_MODE_GENERAL)
        || (old_mode == APP_LEA_ADV_MODE_NONE && mode == APP_LEA_ADV_MODE_TARGET_ALL)) {
        app_lea_adv_time_tick = xTaskGetTickCount();
        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LEA_ADV_FAST_TIMEOUT);
        ui_shell_send_event(FALSE, EVENT_PRIORITY_MIDDLE,
                            EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LEA_ADV_FAST_TIMEOUT,
                            NULL, 0, NULL, APP_LE_AUDIO_ADV_FAST_TIME);
    }
#endif
    /* LE-Audio ADV is long interval when the AWS link is disconnected. */
    if (bt_sink_srv_get_state() >= BT_SINK_SRV_STATE_STREAMING
        || app_lea_adv_mgr_is_esco_ongoing()
        || app_bt_conn_mgr_is_connecting_edr()
#ifdef MTK_AWS_MCE_ENABLE
#ifdef AIR_SPEAKER_ENABLE
        || (BT_AWS_MCE_SRV_MODE_DOUBLE == bt_aws_mce_srv_get_mode() && BT_AWS_MCE_SRV_LINK_NONE == bt_aws_mce_srv_get_link_type())
#else
        || BT_AWS_MCE_SRV_LINK_NONE == bt_aws_mce_srv_get_link_type()
#endif
#endif
        || app_lea_adv_mgr_is_enter_slow_phase()) {
        app_lea_adv_interval_min = APP_LE_AUDIO_ADV_INTERVAL_MIN_L;
        app_lea_adv_interval_max = APP_LE_AUDIO_ADV_INTERVAL_MAX_L;
    } else {
        app_lea_adv_interval_min = APP_LE_AUDIO_ADV_INTERVAL_MIN_S;
        app_lea_adv_interval_max = APP_LE_AUDIO_ADV_INTERVAL_MAX_S;
    }

#ifdef AIR_LE_AUDIO_ENABLE
    app_lea_adv_mgr_init_adv_sub_mode();
#endif
    app_lea_adv_mgr_update_multi_adv(TRUE);

    if (app_lea_adv_mode == APP_LEA_ADV_MODE_TARGET_ALL) {
        app_lea_adv_timeout = 0;        // Must always start ADV for target_all and direct mode
    } else {
        app_lea_adv_timeout = timeout;
    }
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LE_AUDIO_ADV_TIMER);
    if (app_lea_adv_timeout > 0) {
        ui_shell_send_event(FALSE, EVENT_PRIORITY_MIDDLE,
                            EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LE_AUDIO_ADV_TIMER,
                            (void *)(int)adv_timeout_action, 0, NULL, app_lea_adv_timeout);
    }

    app_lea_adv_enabled = TRUE;
    APPS_LOG_MSGID_I(LOG_TAG" do_start_advertising, success adv_mode=%d timeout=%d time_tick=0x%08X interval=0x%04X 0x%04X reset_general_adv=%d",
                     6, app_lea_adv_mode, app_lea_adv_timeout, app_lea_adv_time_tick,
                     app_lea_adv_interval_min, app_lea_adv_interval_max, is_reset_general_adv);

#if defined(MTK_AWS_MCE_ENABLE) && defined(AIR_LE_AUDIO_BOTH_SYNC_INFO)
    app_lea_sync_info_send();
#endif
}

static void app_lea_adv_mgr_do_stop_advertising(void)
{
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LE_AUDIO_ADV_TIMER);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LEA_CHANGE_ADV_SUB_MODE);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LEA_ADV_FAST_TIMEOUT);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LEA_CANCEL_ADV_RECONNECT_FLAG);

    multi_ble_adv_manager_remove_ble_adv(MULTI_ADV_INSTANCE_NOT_RHO, app_lea_adv_mgr_get_adv_data_common);
    multi_ble_adv_manager_notify_ble_adv_data_changed(MULTI_ADV_INSTANCE_NOT_RHO);

    app_lea_adv_enabled = FALSE;
    app_lea_adv_timeout = 0;
    app_lea_adv_mode = APP_LEA_ADV_MODE_NONE;
    memset(&app_lea_adv_mgr_ctx, 0, sizeof(app_lea_adv_mgr_context_t));

    app_lea_clear_target_addr(TRUE);
    APPS_LOG_MSGID_I(LOG_TAG" do_stop_advertising", 0);

#if defined(MTK_AWS_MCE_ENABLE) && defined(AIR_LE_AUDIO_BOTH_SYNC_INFO)
    app_lea_sync_info_send();
#endif
}

static void app_lea_adv_mgr_do_update_advertising_param(void)
{
    if (!app_lea_adv_enabled) {
        APPS_LOG_MSGID_E(LOG_TAG" update_advertising_param, not adv enabled", 0);
        return;
    }

    app_lea_adv_error_code_t error_code = app_lea_adv_mgr_check_adv_allow(app_lea_adv_mode);
    if (error_code != APP_LEA_ADV_ERROR_OK) {
        uint8_t cur_conn_num = app_lea_conn_mgr_get_conn_num();
        uint8_t support_max_conn_num = app_lea_conn_mgr_get_support_max_conn_num();
        APPS_LOG_MSGID_E(LOG_TAG" update_advertising_param, error_code=%d conn_num=%d %d %d",
                         4, error_code, cur_conn_num, APP_LEA_MAX_CONN_NUM, support_max_conn_num);
        return;
    }

    if (app_lea_adv_mgr_ctx.sub_mode_num > 1 || app_lea_adv_mgr_ctx.direct_num > 1) {
        APPS_LOG_MSGID_W(LOG_TAG" update_adv_interval, Not update immediately 0x%04X 0x%04X mode=%d sub_mode_num=%d direct_num=%d",
                         5, app_lea_adv_interval_min, app_lea_adv_interval_max,
                         app_lea_adv_mode, app_lea_adv_mgr_ctx.sub_mode_num, app_lea_adv_mgr_ctx.direct_num);
        return;
    }

    app_lea_adv_mgr_update_multi_adv(FALSE);

    app_lea_adv_enabled = TRUE;

    APPS_LOG_MSGID_I(LOG_TAG" update_advertising_param, adv_mode=%d timeout=%d interval=0x%04X 0x%04X",
                     4, app_lea_adv_mode, app_lea_adv_timeout,
                     app_lea_adv_interval_min, app_lea_adv_interval_max);
}



/**================================================================================*/
/**                                 APP Event Handler                              */
/**================================================================================*/
static void app_lea_adv_mgr_interaction_event_group(uint32_t event_id, void *extra_data, size_t data_len)
{
    if (event_id == APPS_EVENTS_INTERACTION_BT_VISIBLE_NOTIFY) {
        bool visible = (bool)extra_data;
        bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();

        APPS_LOG_MSGID_I(LOG_TAG" BT_VISIBLE_NOTIFY event, [%02X] visible=%d adv_mode=%d",
                         3, role, visible, app_lea_adv_mode);
        if (visible) {
            app_lea_adv_mgr_do_start_advertising(APP_LEA_ADV_MODE_GENERAL, APP_LE_AUDIO_ADV_TIME);
        } else if (app_lea_adv_mode == APP_LEA_ADV_MODE_GENERAL) {
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
            if (app_lea_adv_ull_pair_mode) {
                // do nothing
                APPS_LOG_MSGID_W(LOG_TAG" BT_VISIBLE_NOTIFY event, continue General ADV for ULL2 pairing mode", 0);
            } else {
                app_lea_adv_mgr_do_start_advertising(APP_LEA_ADV_MODE_TARGET_ALL, 0);
            }
#else
            app_lea_adv_mgr_do_start_advertising(APP_LEA_ADV_MODE_TARGET_ALL, 0);
#endif
        }
    }
}

static void app_lea_adv_mgr_bt_sink_event_group(uint32_t event_id, void *extra_data, size_t data_len)
{
    switch (event_id) {
        case BT_SINK_SRV_EVENT_STATE_CHANGE: {
            bt_sink_srv_state_change_t *param = (bt_sink_srv_state_change_t *)extra_data;
            if (param == NULL) {
                break;
            }

            if (param->previous != BT_SINK_SRV_STATE_STREAMING && param->current == BT_SINK_SRV_STATE_STREAMING) {
                APPS_LOG_MSGID_I(LOG_TAG" Music Start", 0);
                app_lea_adv_mgr_update_adv_interval(APP_LE_AUDIO_ADV_INTERVAL_MIN_L, APP_LE_AUDIO_ADV_INTERVAL_MAX_L);
            } else if (param->previous == BT_SINK_SRV_STATE_STREAMING && param->current != BT_SINK_SRV_STATE_STREAMING) {
                APPS_LOG_MSGID_I(LOG_TAG" Music Stop", 0);
                app_lea_adv_mgr_update_adv_interval(APP_LE_AUDIO_ADV_INTERVAL_MIN_S, APP_LE_AUDIO_ADV_INTERVAL_MAX_S);
            }
            break;
        }

        case BT_SINK_SRV_EVENT_HF_SCO_STATE_UPDATE: {
            bt_sink_srv_sco_state_update_t *esco_state = (bt_sink_srv_sco_state_update_t *)extra_data;
            if (esco_state == NULL) {
                break;
            }

            APPS_LOG_MSGID_I(LOG_TAG" eSCO state = %d", 1, esco_state->state);
            if (esco_state->state == BT_SINK_SRV_SCO_CONNECTION_STATE_CONNECTED) {
                app_lea_adv_mgr_update_adv_interval(APP_LE_AUDIO_ADV_INTERVAL_MIN_L, APP_LE_AUDIO_ADV_INTERVAL_MAX_L);
            } else if (esco_state->state == BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED) {
                app_lea_adv_mgr_update_adv_interval(APP_LE_AUDIO_ADV_INTERVAL_MIN_S, APP_LE_AUDIO_ADV_INTERVAL_MAX_S);
            }
            break;
        }

#if defined(AIR_LE_AUDIO_ENABLE) && defined(AIR_LE_AUDIO_CIS_ENABLE)
        case BT_SINK_SRV_EVENT_LE_BIDIRECTION_LEA_UPDATE: {
            bt_sink_srv_bidirection_lea_state_update_t *event = (bt_sink_srv_bidirection_lea_state_update_t *)extra_data;
            if (event == NULL) {
                break;
            }

            APPS_LOG_MSGID_I(LOG_TAG" LEA call state = %d", 1, event->state);
            if (event->state == BT_SINK_SRV_BIDIRECTION_LEA_STATE_ENABLE) {
                app_lea_adv_mgr_update_adv_interval(APP_LE_AUDIO_ADV_INTERVAL_MIN_L, APP_LE_AUDIO_ADV_INTERVAL_MAX_L);
            } else if (event->state == BT_SINK_SRV_BIDIRECTION_LEA_STATE_DISABLE) {
                app_lea_adv_mgr_update_adv_interval(APP_LE_AUDIO_ADV_INTERVAL_MIN_S, APP_LE_AUDIO_ADV_INTERVAL_MAX_S);
            }
            break;
        }
#endif

        default:
            break;
    }
}

static void app_lea_adv_mgr_bt_dm_event_group(uint32_t event_id, void *extra_data, size_t data_len)
{
    bt_device_manager_power_event_t evt;
    bt_device_manager_power_status_t status;
    bt_event_get_bt_dm_event_and_status(event_id, &evt, &status);
    switch (evt) {
        case BT_DEVICE_MANAGER_POWER_EVT_STANDBY_COMPLETE: {
            if (BT_DEVICE_MANAGER_POWER_RESET_TYPE_NORMAL == status) {
                //APPS_LOG_MSGID_I(LOG_TAG" BT_DM POWER OFF", 0);
                app_lea_clear_target_addr(TRUE);
            }
            break;
        }
    }
}

static void app_lea_adv_mgr_lea_event_group(uint32_t event_id, void *extra_data, size_t data_len)
{
    switch (event_id) {
        case EVENT_ID_LE_AUDIO_ADV_TIMER: {
            uint8_t action = (uint8_t)(int)extra_data;
            bool visible = app_bt_service_is_visible();
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
            APPS_LOG_MSGID_I(LOG_TAG" LE Audio event, ADV timeout action=%d visible=%d ull2_pair_mode=%d",
                             3, action, visible, app_lea_adv_ull_pair_mode);
#else
            APPS_LOG_MSGID_I(LOG_TAG" LE Audio event, ADV timeout action=%d visible=%d",
                             2, action, visible);
#endif
            if (action == APP_LEA_ADV_TIMEOUT_ACTION_START) {
                if (visible
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
                    || app_lea_adv_ull_pair_mode
#endif
                   ) {
                    app_lea_adv_mgr_do_start_advertising(APP_LEA_ADV_MODE_GENERAL, APP_LE_AUDIO_ADV_TIME);
                } else {
                    app_lea_adv_mgr_do_start_advertising(APP_LEA_ADV_MODE_TARGET_ALL, 0);
                }
            } else if (!visible) {
                app_lea_adv_mgr_do_stop_advertising();
            }
            break;
        }
        case EVENT_ID_LE_AUDIO_START_ADV: {
            app_lea_adv_param_t *adv_param = (app_lea_adv_param_t *)extra_data;
            app_lea_adv_mgr_do_start_advertising(adv_param->mode, adv_param->timeout);
            break;
        }
        case EVENT_ID_LE_AUDIO_STOP_ADV: {
            app_lea_adv_mgr_do_stop_advertising();
            break;
        }
        case EVENT_ID_LEA_FORCE_UPDATE_ADV: {
            bool general_adv = FALSE;
            bool visible = app_bt_service_is_visible();
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
            if (app_lea_adv_ull_pair_mode || visible) {
                general_adv = TRUE;
            }
#else
            if (visible) {
                general_adv = TRUE;
            }
#endif
            if (general_adv) {
                app_lea_adv_mgr_do_start_advertising(APP_LEA_ADV_MODE_GENERAL, APP_LE_AUDIO_ADV_TIME);
            } else {
                app_lea_adv_mgr_do_start_advertising(APP_LEA_ADV_MODE_TARGET_ALL, 0);
            }
            break;
        }
#ifdef AIR_SMART_CHARGER_ENABLE
        case EVENT_ID_LEA_CLOSE_LID_ACTION: {
            APPS_LOG_MSGID_E(LOG_TAG" LE Audio event, CLOSE_LID_ACTION", 0);
            app_lea_adv_mgr_do_stop_advertising();
            app_lea_service_disconnect(FALSE, APP_LE_AUDIO_DISCONNECT_MODE_ALL,
                                       NULL, BT_HCI_STATUS_REMOTE_TERMINATED_CONNECTION_DUE_TO_POWER_OFF);
            break;
        }
#endif
        case EVENT_ID_LE_AUDIO_GENERAL_ADV_FOR_TEST: {
            // For low power mode testing
            APPS_LOG_MSGID_E(LOG_TAG" LE Audio event, restart GENERAL_ADV for testing on Low power mode", 0);
            app_lea_adv_mgr_do_start_advertising(APP_LEA_ADV_MODE_GENERAL, 0);
            multi_ble_adv_manager_start_ble_adv();
            break;
        }
#ifdef AIR_LE_AUDIO_ENABLE
        case EVENT_ID_LEA_CHANGE_ADV_SUB_MODE: {
            if (app_lea_adv_mgr_ctx.sub_mode_bitmask == 0) {
                APPS_LOG_MSGID_E(LOG_TAG" LE Audio event, CHANGE_ADV_SUB_MODE bitmask=0", 0);
                break;
            }
            app_lea_adv_mgr_change_adv_sub_mode();
            app_lea_adv_mgr_update_multi_adv(FALSE);
            break;
        }
        case EVENT_ID_LEA_ADV_FAST_TIMEOUT: {
            APPS_LOG_MSGID_W(LOG_TAG"[Interval] ADV_FAST_TIMEOUT, slow", 0);
            app_lea_adv_mgr_update_adv_interval(APP_LE_AUDIO_ADV_INTERVAL_MIN_L, APP_LE_AUDIO_ADV_INTERVAL_MAX_L);
            break;
        }
        case EVENT_ID_LEA_CANCEL_ADV_RECONNECT_FLAG: {
            app_lea_conn_mgr_control_temp_reconnect_type(FALSE);
            break;
        }
        case EVENT_ID_LEA_RESET_LEA_DONGLE: {
            app_lea_adv_mgr_do_stop_advertising();
            app_lea_conn_mgr_reset_lea_dongle();
            app_lea_adv_mgr_do_start_advertising(APP_LEA_ADV_MODE_GENERAL, APP_LE_AUDIO_ADV_TIME);
            break;
        }
#endif

#if defined(AIR_LE_AUDIO_ENABLE) && defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
        case EVENT_ID_LE_AUDIO_CONTROL_ADV_DATA: {
            uint32_t adv_param = (uint32_t)extra_data;
            uint8_t adv_type = (uint8_t)((adv_param & 0xFF00) >> 8);
            bool enable = ((uint8_t)(adv_param & 0xFF) > 0);
            APPS_LOG_MSGID_I(LOG_TAG" control_adv_data, adv_type=%d enable=%d app_lea_adv_enabled=%d",
                             3, adv_type, enable, app_lea_adv_enabled);
#if defined(AIR_LE_AUDIO_ENABLE) && defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
            bool need_update = FALSE;
            if (adv_type == APP_LEA_SERVICE_ADV_DATA_ULL) {
                // If already advertising, will add/remove "LEA/ULL ADV Data" via update ADV parameter
                // If not advertising, only disable ADV data, user need to call <app_lea_service_start_advertising> to restart ADV
                app_lea_adv_add_ull_data = enable;
                need_update = TRUE;
            } else if (adv_type == APP_LEA_SERVICE_ADV_DATA_LEA) {
                app_lea_adv_add_lea_data = enable;
                need_update = TRUE;
            }

            if (!app_lea_adv_add_lea_data && !app_lea_adv_add_ull_data) {
                app_lea_adv_mgr_do_stop_advertising();
                need_update = FALSE;
            }

            if (need_update) {
                app_lea_adv_mgr_do_update_advertising_param();
            }
#elif defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
            if (adv_type == APP_LEA_SERVICE_ADV_DATA_ULL) {
                app_lea_adv_add_ull_data = enable;
            }
            if (!app_lea_adv_add_ull_data) {
                app_lea_adv_mgr_stop_advertising(FALSE);
            }
#endif
            break;
        }
#endif

#if defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
        case EVENT_ID_LEA_ULL_PAIR_MODE: {
            bool enable = (bool)(uint32_t)extra_data;
            APPS_LOG_MSGID_I(LOG_TAG" enable_ull2_pair_mode, adv_mode=%d enable=%d", 2, app_lea_adv_mode, enable);
            app_lea_adv_ull_pair_mode = enable;
            if (enable) {
                app_lea_adv_mgr_do_start_advertising(APP_LEA_ADV_MODE_GENERAL, APP_LE_AUDIO_ADV_TIME);
            } else {
                app_lea_adv_mgr_do_start_advertising(APP_LEA_ADV_MODE_TARGET_ALL, 0);
            }
            break;
        }

        case EVENT_ID_LEA_ULL_RECONNECT_MODE: {
            bool enable = (bool)(uint32_t)extra_data;
            APPS_LOG_MSGID_I(LOG_TAG" enable_ull2_reconnect_mode, pre=%d, enable=%d", 2, app_lea_adv_ull_reconnect_mode, enable);
            if (app_lea_adv_ull_reconnect_mode == enable) {
                break;
            }
            if (enable) {
                app_lea_adv_ull_pair_mode = FALSE;
            }
            app_lea_adv_ull_reconnect_mode = enable;
            app_lea_adv_mgr_do_start_advertising(APP_LEA_ADV_MODE_TARGET_ALL, 0);
            break;
        }
#endif
    }
}

#ifdef MTK_AWS_MCE_ENABLE
static void app_lea_adv_mgr_proc_aws_data(void *extra_data, size_t data_len)
{
    uint32_t aws_event_group;
    uint32_t aws_event_id;
    void *p_extra_data = NULL;
    uint32_t extra_data_len = 0;
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bt_aws_mce_report_info_t *aws_data_ind = (bt_aws_mce_report_info_t *)extra_data;

    if (aws_data_ind == NULL || aws_data_ind->module_id != BT_AWS_MCE_REPORT_MODULE_APP_ACTION) {
        return;
    }

    apps_aws_sync_event_decode_extra(aws_data_ind, &aws_event_group, &aws_event_id,
                                     &p_extra_data, &extra_data_len);
    if (aws_event_group == EVENT_GROUP_UI_SHELL_LE_AUDIO) {
        APPS_LOG_MSGID_I(LOG_TAG" AWS_DATA event, [%02X] event_id=%d", 2, role, aws_event_id);
        if (aws_event_id == EVENT_ID_LE_AUDIO_START_ADV) {
            app_lea_adv_param_t *adv_param = (app_lea_adv_param_t *)p_extra_data;
            app_lea_adv_mgr_do_start_advertising(adv_param->mode, adv_param->timeout);
        } else if (aws_event_id == EVENT_ID_LE_AUDIO_STOP_ADV) {
            app_lea_adv_mgr_do_stop_advertising();
        } else if (aws_event_id == EVENT_ID_LEA_RESET_LEA_DONGLE) {
            app_lea_adv_mgr_do_stop_advertising();
            app_lea_conn_mgr_reset_lea_dongle();
            app_lea_adv_mgr_do_start_advertising(APP_LEA_ADV_MODE_GENERAL, APP_LE_AUDIO_ADV_TIME);
        }
    }
}
#endif

static void app_lea_adv_mgr_proc_bt_cm_group(uint32_t event_id, void *extra_data, size_t data_len)
{
    switch (event_id) {
        case BT_CM_EVENT_REMOTE_INFO_UPDATE: {
            bt_cm_remote_info_update_ind_t *remote_update = (bt_cm_remote_info_update_ind_t *)extra_data;
            if (NULL == remote_update) {
                break;
            }

#ifdef MTK_AWS_MCE_ENABLE
            if (!(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service) &&
                (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service)) {
                /* LE-Audio ADV resume to the short interval when the AWS link connected. */
                app_lea_adv_mgr_update_adv_interval(APP_LE_AUDIO_ADV_INTERVAL_MIN_S, APP_LE_AUDIO_ADV_INTERVAL_MAX_S);
            } else if ((BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service)
                       && !(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service)) {
                /* LE-Audio ADV resume to the long interval when the AWS link disconnected. */
                app_lea_adv_mgr_update_adv_interval(APP_LE_AUDIO_ADV_INTERVAL_MIN_L, APP_LE_AUDIO_ADV_INTERVAL_MAX_L);
            }
#endif

            if (BT_CM_ACL_LINK_CONNECTED > remote_update->pre_acl_state
                && BT_CM_ACL_LINK_CONNECTED <= remote_update->acl_state
                && 0 != memcmp(remote_update->address, bt_device_manager_get_local_address(), sizeof(bt_bd_addr_t))) {
                /* LE-Audio ADV resume to the short interval when the EDR link connected. */
                app_lea_adv_mgr_update_adv_interval(APP_LE_AUDIO_ADV_INTERVAL_MIN_S, APP_LE_AUDIO_ADV_INTERVAL_MAX_S);
            } else if ((BT_CM_ACL_LINK_PENDING_CONNECT == remote_update->pre_acl_state || BT_CM_ACL_LINK_CONNECTING == remote_update->pre_acl_state)
                       && BT_CM_ACL_LINK_DISCONNECTED == remote_update->acl_state
                       && 0 != memcmp(remote_update->address, bt_device_manager_get_local_address(), sizeof(bt_bd_addr_t))) {
                /* LE-Audio ADV resume to the short interval when the EDR link connecting fail. */
                app_lea_adv_mgr_update_adv_interval(APP_LE_AUDIO_ADV_INTERVAL_MIN_S, APP_LE_AUDIO_ADV_INTERVAL_MAX_S);
            }
            break;
        }

        default:
            break;
    }
}

static bool app_lea_adv_mgr_proc_ble_ull_group(uint32_t event_id, void *extra_data, size_t data_len)
{
    switch (event_id) {
#if defined(AIR_LE_AUDIO_ENABLE) && defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
        // Need modify if-option if ULL2 could support multi-link independently
        case BT_ULL_EVENT_LE_STREAMING_START_IND: {
            APPS_LOG_MSGID_I(LOG_TAG" BLE ULL2 Start", 0);
            app_lea_adv_mgr_update_adv_interval(APP_LE_AUDIO_ADV_INTERVAL_MIN_L, APP_LE_AUDIO_ADV_INTERVAL_MAX_L);
            break;
        }
        case BT_ULL_EVENT_LE_STREAMING_STOP_IND: {
            APPS_LOG_MSGID_I(LOG_TAG" BLE ULL2 Stop", 0);
            app_lea_adv_mgr_update_adv_interval(APP_LE_AUDIO_ADV_INTERVAL_MIN_S, APP_LE_AUDIO_ADV_INTERVAL_MAX_S);
            break;
        }
#endif
        default:
            break;
    }
    return FALSE;
}



/**================================================================================*/
/**                                     Public API                                 */
/**================================================================================*/
void app_lea_adv_mgr_start_advertising(uint8_t mode, bool sync, uint32_t timeout)
{
    APPS_LOG_MSGID_I(LOG_TAG" start_advertising, mode=%d sync=%d timeout=%d", 3, mode, sync, timeout);
    if (mode == APP_LEA_ADV_MODE_NONE) {
        return;
    }

    app_lea_adv_param_t *adv_param = (app_lea_adv_param_t *)pvPortMalloc(sizeof(app_lea_adv_param_t));
    if (adv_param == NULL) {
        APPS_LOG_MSGID_E(LOG_TAG" start_advertising, malloc error", 0);
        return;
    }

    adv_param->mode = mode;
    adv_param->timeout = timeout;

#ifdef MTK_AWS_MCE_ENABLE
    bt_aws_mce_srv_link_type_t aws_link_type = bt_aws_mce_srv_get_link_type();
    if (sync && aws_link_type != BT_AWS_MCE_SRV_LINK_NONE) {
        bt_status_t bt_status = apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_LE_AUDIO,
                                                               EVENT_ID_LE_AUDIO_START_ADV,
                                                               adv_param,
                                                               sizeof(app_lea_adv_param_t));
        if (bt_status != BT_STATUS_SUCCESS) {
            APPS_LOG_MSGID_E(LOG_TAG" start_advertising, error AWS bt_status=0x%08X", 1, bt_status);
        }
    }
#endif

    ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                        EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LE_AUDIO_START_ADV,
                        (void *)adv_param, sizeof(app_lea_adv_param_t), NULL, 0);
}

void app_lea_adv_mgr_stop_advertising(bool sync)
{
    APPS_LOG_MSGID_I(LOG_TAG" stop_advertising, sync=%d", 1, sync);
    ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                        EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LE_AUDIO_STOP_ADV,
                        NULL, 0, NULL, 0);

#ifdef MTK_AWS_MCE_ENABLE
    bt_aws_mce_srv_link_type_t aws_link_type = bt_aws_mce_srv_get_link_type();
    if (sync && aws_link_type != BT_AWS_MCE_SRV_LINK_NONE) {
        bt_status_t bt_status = apps_aws_sync_event_send(EVENT_GROUP_UI_SHELL_LE_AUDIO,
                                                         EVENT_ID_LE_AUDIO_STOP_ADV);
        if (bt_status != BT_STATUS_SUCCESS) {
            APPS_LOG_MSGID_E(LOG_TAG" stop_advertising, error AWS bt_status=0x%08X", 1, bt_status);
        }
    }
#endif
}

void app_lea_adv_mgr_quick_stop_adv(void)
{
    APPS_LOG_MSGID_I(LOG_TAG" quick_stop_adv", 0);
    app_lea_adv_mgr_do_stop_advertising();
}

void app_lea_adv_mgr_refresh_advertising(uint32_t delay)
{
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LEA_FORCE_UPDATE_ADV);
    ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                        EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LEA_FORCE_UPDATE_ADV,
                        NULL, 0, NULL, delay);
}

uint8_t app_lea_adv_mgr_get_adv_mode(void)
{
    return app_lea_adv_mode;
}

bool app_lea_adv_mgr_update_adv_interval(uint16_t interval_min, uint16_t interval_max)
{
    bt_sink_srv_state_t sink_state = bt_sink_srv_get_state();
    bool esco_ongoing = app_lea_adv_mgr_is_esco_ongoing();
    bool connecting_edr = app_bt_conn_mgr_is_connecting_edr();
    bool turn_down = (interval_min < app_lea_adv_interval_min && interval_max < app_lea_adv_interval_max);
    APPS_LOG_MSGID_I(LOG_TAG" update_adv_interval, adv_enabled=%d sink_state=0x%04X esco_ongoing=%d connecting_edr=%d min=0x%04X->0x%04X max=0x%04X->0x%04X",
                     8, app_lea_adv_enabled, sink_state, esco_ongoing, connecting_edr,
                     app_lea_adv_interval_min, interval_min,
                     app_lea_adv_interval_max, interval_max);

    if (!app_lea_adv_enabled) {
        //APPS_LOG_MSGID_E(LOG_TAG" update_adv_interval, not advertising", 0);
        return FALSE;
    } else if (app_lea_adv_interval_min == interval_min && app_lea_adv_interval_max == interval_max) {
        //APPS_LOG_MSGID_E(LOG_TAG" update_adv_interval, same parameter", 0);
        return FALSE;
    } else if ((interval_min < app_lea_adv_interval_min && interval_max > app_lea_adv_interval_max)
               || (interval_min > app_lea_adv_interval_min && interval_max < app_lea_adv_interval_max)) {
        //APPS_LOG_MSGID_E(LOG_TAG" update_adv_interval, not invalid parameter", 0);
        return FALSE;
    } else if (turn_down) {
        if (sink_state >= BT_SINK_SRV_STATE_STREAMING || esco_ongoing) {
            //APPS_LOG_MSGID_E(LOG_TAG" update_adv_interval, not turn_down when Audio ongoing", 0);
            return FALSE;
        }
#ifdef MTK_AWS_MCE_ENABLE
        if (BT_AWS_MCE_SRV_LINK_NONE == bt_aws_mce_srv_get_link_type()) {
            //APPS_LOG_MSGID_E(LOG_TAG" update_adv_interval, not turn_down when AWS not connected", 0);
            return FALSE;
        }
#endif
        if (connecting_edr) {
            //APPS_LOG_MSGID_E(LOG_TAG" update_adv_interval, not turn_down when BT EDR connecting", 0);
            return FALSE;
        }
    }

    if (turn_down && app_lea_adv_mgr_is_enter_slow_phase()) {
        APPS_LOG_MSGID_E(LOG_TAG"[Interval] update_adv_interval, cannot turn_down for enter_slow_phase", 0);
        return FALSE;
    }

    app_lea_adv_interval_min = interval_min;
    app_lea_adv_interval_max = interval_max;

    app_lea_adv_mgr_do_update_advertising_param();
//    APPS_LOG_MSGID_I(LOG_TAG" update_adv_interval, success min=%d max=%d",
//                     2, app_lea_adv_interval_min, app_lea_adv_interval_max);
    return TRUE;
}

bool app_lea_adv_mgr_control_adv_data(uint8_t adv_type, bool enable)
{
#if defined(AIR_LE_AUDIO_ENABLE) && defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
    if (adv_type > APP_LEA_SERVICE_ADV_DATA_ULL) {
        return FALSE;
    }

    uint32_t adv_param = ((adv_type << 8) | enable);
    ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                        EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LE_AUDIO_CONTROL_ADV_DATA,
                        (void *)adv_param, 0, NULL, 0);
#endif
    return TRUE;
}

#if defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
bool app_lea_adv_mgr_enable_ull2_pairing_mode(bool enable)
{
    ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                        EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LEA_ULL_PAIR_MODE,
                        (void *)enable, 0, NULL, 0);
    return TRUE;
}

bool app_lea_adv_mgr_enable_ull2_reconnect_mode(bool enable)
{
    ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGH,
                        EVENT_GROUP_UI_SHELL_LE_AUDIO, EVENT_ID_LEA_ULL_RECONNECT_MODE,
                        (void *)enable, 0, NULL, 0);
    return TRUE;
}
#endif

void app_lea_adv_mgr_get_adv_info(uint8_t *mode, uint32_t *timeout)
{
    if (mode != NULL) {
        *mode = app_lea_adv_mode;
    }
    if (timeout != NULL) {
        *timeout = app_lea_adv_timeout;
    }
}

void app_lea_adv_mgr_init(void)
{
#if defined(AIR_LE_AUDIO_ENABLE) && defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
    app_lea_adv_add_lea_data = TRUE;
    app_lea_adv_add_ull_data = TRUE;
    if (app_lea_feature_mode == APP_LEA_FEATURE_MODE_OFF) {
        app_lea_adv_add_lea_data = FALSE;
    }
#elif defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
    app_lea_adv_add_ull_data = TRUE;
#endif
}

void app_lea_adv_mgr_proc_ui_shell_event(uint32_t event_group,
                                         uint32_t event_id,
                                         void *extra_data,
                                         size_t data_len)
{
    switch (event_group) {
        case EVENT_GROUP_UI_SHELL_APP_INTERACTION:
            app_lea_adv_mgr_interaction_event_group(event_id, extra_data, data_len);
            break;
        case EVENT_GROUP_UI_SHELL_BT_SINK:
            app_lea_adv_mgr_bt_sink_event_group(event_id, extra_data, data_len);
            break;
        case EVENT_GROUP_UI_SHELL_BT_DEVICE_MANAGER:
            app_lea_adv_mgr_bt_dm_event_group(event_id, extra_data, data_len);
            break;
        case EVENT_GROUP_UI_SHELL_LE_AUDIO:
            app_lea_adv_mgr_lea_event_group(event_id, extra_data, data_len);
            break;
#ifdef MTK_AWS_MCE_ENABLE
        case EVENT_GROUP_UI_SHELL_AWS_DATA:
            app_lea_adv_mgr_proc_aws_data(extra_data, data_len);
            break;
#endif
        case EVENT_GROUP_UI_SHELL_BT_CONN_MANAGER:
            app_lea_adv_mgr_proc_bt_cm_group(event_id, extra_data, data_len);
            break;
        case EVENT_GROUP_BT_ULTRA_LOW_LATENCY:
            app_lea_adv_mgr_proc_ble_ull_group(event_id, extra_data, data_len);
            break;
    }
}

#endif  /* AIR_LE_AUDIO_ENABLE */
