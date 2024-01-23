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

/**
 * File: app_ull_idle_activity.c
 *
 * Description: This file is the activity to handle ultra low latency state.
 *
 * Note: See doc/AB1565_AB1568_Earbuds_Reference_Design_User_Guide.pdf for more detail.
 *
 */

#include "app_ull_idle_activity.h"
#include "app_ull_nvkey_struct.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"
#include "apps_events_key_event.h"
#include "apps_events_bt_event.h"
#include "apps_config_event_list.h"
#include "apps_config_vp_index_list.h"
#include "voice_prompt_api.h"
#include "apps_config_key_remapper.h"
#include "apps_config_led_manager.h"
#include "apps_config_led_index_list.h"
#include "apps_customer_config.h"
#include "app_bt_state_service.h"
#include "multi_ble_adv_manager.h"
#include "bt_app_common.h"
#include "bt_connection_manager.h"
#include "bt_connection_manager_internal.h"
#include "bt_device_manager.h"
#include "bt_device_manager_power.h"
#include "bt_ull_service.h"
#include "bt_sink_srv_ami.h"
#include "nvkey_id_list.h"
#include "atci.h"
#include "bt_customer_config.h"
#ifdef AIR_BT_FAST_PAIR_ENABLE
#include "app_fast_pair.h"
#endif
#if defined(MTK_FOTA_ENABLE) && defined (MTK_FOTA_VIA_RACE_CMD)
#include "race_event.h"
#endif
#include "nvkey.h"
#ifdef MTK_AWS_MCE_ENABLE
#include "apps_aws_sync_event.h"
#include "bt_aws_mce_srv.h"
#include "apps_events_battery_event.h"
#endif
#include "apps_debug.h"

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
#include "bt_ull_le_service.h"
#include "bt_ull_le_utility.h"
#include "bt_ull_le_audio_manager.h"
#include "app_dongle_service.h"
#include "apps_dongle_sync_event.h"
#endif

#ifdef AIR_BLE_ULL_TAKEOVER_ENABLE
#include "app_ull_takeover.h"
#endif

#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
#include "apps_detachable_mic.h"
#endif

#ifdef APPS_SLEEP_AFTER_NO_CONNECTION
#include "app_power_save_utils.h"
#endif
#include "app_lea_service.h"

#ifdef AIR_BLE_ULL_PARING_MODE_ENABLE
#include "app_lea_service_adv_mgr.h"
#include "app_lea_service_conn_mgr.h"
#endif

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
#include "bt_ull_le_hid_service.h"
#include "bt_ull_le_hid_device_manager.h"
#endif

#define LOG_TAG "[ULL_activity]"

/* GAME_MAX_LEVEL --------- BALANCED_LEVEL --------- CHAT_MAX_LEVEL */
#define ULL_MIX_RATIO_GAME_MAX_LEVEL    (0)     /* Gaming is 100%, Chat is 0% */
#define ULL_MIX_RATIO_CHAT_MAX_LEVEL    (20)    /* Gaming is 0%, Chat is 100% */
#define ULL_MIX_RATIO_BALANCED_LEVEL    (10)    /* Gaming is 100%, Chat is 100% */

/* The reconnect timeout, after the time, device will reconnect both SP and dongle. */
#define ULL_SWITCH_CONNECT_TIME         (10 * 1000)

#define ULL_IS_ADDRESS_EMPTY(addr)      (0 == memcmp(addr, s_empty_address, sizeof(bt_bd_addr_t)))
#define ULL_IS_DONGLE_ADDRESS(addr)     (0 == memcmp(addr, s_ull_context.dongle_bt_address, sizeof(bt_bd_addr_t)))
#define ULL_IS_LOCAL_ADDRESS(addr)      (0 == memcmp(addr, *bt_device_manager_get_local_address(), sizeof(bt_bd_addr_t)))
#define ULL_IS_ADDRESS_THE_SAME(addr1, addr2)    (0 == memcmp(addr1, addr2, sizeof(bt_bd_addr_t)))

enum {
    ULL_LINK_MODE_SINGLE,
    ULL_LINK_MODE_MULTIPLE
};

typedef enum {
    ULL_EVENTS_DONGLE_CONNECTED,
    ULL_EVENTS_DONGLE_DISCONNECTED,
    ULL_EVENTS_SP_CONNECTED,
    ULL_EVENTS_SP_DISCONNECTED,
    ULL_EVENTS_AUX_IN,
    ULL_EVENTS_AUX_OUT,
    ULL_EVENTS_USB_AUDIO_IN,
    ULL_EVENTS_USB_AUDIO_OUT,
    ULL_EVENTS_SWITCH_LINK_MODE,
    ULL_EVENTS_SWITCH_GAME_MODE,
} ull_ui_events_t;

static app_ull_context_t s_ull_context;

const static bt_bd_addr_t s_empty_address = { 0, 0, 0, 0, 0, 0};

static bool s_uplink_started = false;

#ifdef MTK_AWS_MCE_ENABLE
static bool s_need_resync_ull_addr;
static bool s_ull_link_mode_synced;
#endif


#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
static bool s_ull_le_hid_connected = false;
#endif

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
#define BT_ULL_LE_MAX_UUID_LENGTH 16
static bool s_ull_adv_en = false;
static bool s_ull_le_connected = false;
static bool s_cmd_adv_disable = false;
static bool s_waiting_disconnect_event = false;
static bool s_hfp_only_enable = true;
bool s_a2dp_standby_enable = false;
static bool s_speaker_mode = false;
static bt_ull_le_device_info_t g_ull_dev_info = {
#ifdef AIR_WIRELESS_MIC_ENABLE
    BT_ULL_MIC_CLIENT,
    2,
    {0},
    {0},
    BT_ULL_LE_CHANNEL_MODE_STEREO,
    {}
#else
#ifdef AIR_HEADSET_ENABLE
    BT_ULL_HEADSET_CLIENT,
    1,
#else
    BT_ULL_EARBUDS_CLIENT,
    2,
#endif
    {0},
    {0},
    BT_ULL_LE_CODEC_LC3PLUS,
    {}
#endif
};

#define APP_ULL2_VER2_0 0
#define APP_ULL2_VER3_0 1
static uint8_t s_ull2_ver;

void app_le_ull_disconnect_dongle();
static atci_status_t app_le_ull_atci_hdl(atci_parse_cmd_param_t *parse_cmd);

static atci_cmd_hdlr_item_t bt_app_le_ull_at_cmd[] = {
    {
        .command_head = "AT+LEULL",       /**< AT command string. */
        .command_hdlr = app_le_ull_atci_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
};

#define DATA_LEN 18
void app_le_ull_get_sirk(bt_key_t *sirk)
{
    uint8_t data[DATA_LEN] = {0};
    uint32_t size = DATA_LEN;

    if (NVKEY_STATUS_OK != nvkey_read_data(NVID_BT_LEA_CSIS_DATA, data, &size)) {
        ;
    } else {
        memcpy(sirk, data, sizeof(bt_key_t));
    }
}

void app_le_ull_write_nvkey_sirk(bt_key_t *sirk)
{
    uint8_t data[DATA_LEN] = {0};
    uint32_t size = DATA_LEN;

    if (NVKEY_STATUS_OK == nvkey_read_data(NVID_BT_LEA_CSIS_DATA, data, &size)) {
        memcpy(data, (uint8_t *)sirk, sizeof(bt_key_t));
        nvkey_write_data(NVID_BT_LEA_CSIS_DATA, data, size);
    }
}

#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
static void app_ull_le_load_and_set_config()
{
    app_ull_abr_parameter_configure_t app_abr_config = {0};
    uint32_t size = sizeof(app_ull_abr_parameter_configure_t);
    nvkey_status_t status = nvkey_read_data(NVID_APP_ULL_ABR_PARAMETER_CFG, (uint8_t *)&app_abr_config, &size);
    if (status != NVKEY_STATUS_OK) {
        APPS_LOG_MSGID_E(LOG_TAG", read NVID_APP_ULL_ABR_PARAMETER_CFG fail", 0);
        return;
    } else {
        APPS_LOG_MSGID_I(LOG_TAG", NVID_BT_CON_ULL_ABR_PARAM_CFG,EN=%d,quality=%d,threshold=%d,resume_time=%d", 4,
                        app_abr_config.enable,
                        app_abr_config.default_audio_quality,
                        app_abr_config.threshold,
                        app_abr_config.resume_time);
    }

    bt_ull_le_adaptive_bitrate_params_t param;
    param.enable = app_abr_config.enable;
    param.report_interval = 10;//10 * 10ms
    param.crc_threshold = param.report_interval / 5 * 3 * app_abr_config.threshold / 100;
    param.rx_timeout_threshold = param.report_interval / 5 * app_abr_config.threshold / 100;
    param.flush_timeout_threshold = param.report_interval / 5 * app_abr_config.threshold / 100;
    param.resume_time = app_abr_config.resume_time;
    bt_ull_le_srv_enable_adaptive_bitrate_mode(&param);

    bt_ull_le_srv_action(BT_ULL_ACTION_SET_AUDIO_QUALITY, &app_abr_config.default_audio_quality, sizeof(bt_ull_le_srv_audio_quality_t));
}
#endif

void app_le_ull_set_advertising_enable(bool enable, bool general_adv, bool pairing);
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE) && defined (MTK_AWS_MCE_ENABLE)
static bool app_ull_le_read_group_device_addr(bt_addr_t *addr)
{
#ifdef MTK_NVDM_ENABLE
    uint32_t size = sizeof(bt_addr_t);
    bt_bd_addr_t zero_addr = {0};
    bt_addr_t group_device_addr = {
        .type = BT_ADDR_PUBLIC,
        .addr = {0}
    };

    nvkey_status_t status = nvkey_read_data(NVID_APP_LEA_DHSS_PAIR_LE_ADDR, (uint8_t *)&group_device_addr, &size);
    if (NVKEY_STATUS_OK != status) {
        APPS_LOG_MSGID_E(LOG_TAG" app_ull_le_read_group_device_addr, error NVKEY status=%d, size: %d", 2, status, size);
        return false;
    }

    if (!memcmp(group_device_addr.addr, (uint8_t *)&zero_addr, sizeof(bt_bd_addr_t))) {
        APPS_LOG_MSGID_E(LOG_TAG", app_ull_le_read_group_device_addr, addr is zero", 0);
        return false;
    }
    memcpy((uint8_t *)addr, (uint8_t *)&group_device_addr, size);
    APPS_LOG_MSGID_I(LOG_TAG" app_ull_le_read_group_device_addr,type: %d, addr: 0x%x-%x-%x-%x-%x-%x", 7, \
                     group_device_addr.type,
                     group_device_addr.addr[0],
                     group_device_addr.addr[1],
                     group_device_addr.addr[2],
                     group_device_addr.addr[3],
                     group_device_addr.addr[4],
                     group_device_addr.addr[5]
                    );
    return true;
#else
    return false;
#endif
}

static void app_ull_le_reverse_copy(uint8_t *dst, uint8_t *src, uint32_t len)
{
    uint8_t i = 0;

    for (i = 0; i < len; i++) {
        dst[i] = src[len - 1 - i];
    }
}

static bool app_ull_le_reverse_compare(uint8_t *dst, uint8_t *src, uint32_t len)
{
    uint8_t i = 0;

    for (i = 0; i < len; i++) {
        if (dst[i] != src[len - 1 - i]) {
            return FALSE;
        }
    }
    return TRUE;
}

void app_ull_le_set_group_device_addr(bt_addr_type_t type, uint8_t *addr)
{
#ifdef MTK_NVDM_ENABLE
    uint8_t i = 0;
    nvkey_status_t status;
    uint32_t size = sizeof(bt_addr_t);
    bt_bd_addr_t zero_addr = {0};
    bt_addr_t group_device_addr = {
        .type = BT_ADDR_PUBLIC,
        .addr = {0}
    };

    if (!memcmp(addr, (uint8_t *)&zero_addr, sizeof(bt_bd_addr_t))) {
        APPS_LOG_MSGID_E(LOG_TAG", app_ull_le_set_group_device_addr, addr is zero", 0);
        return;
    }

    status = nvkey_read_data(NVID_APP_LEA_DHSS_PAIR_LE_ADDR, (uint8_t *)&group_device_addr, &size);
    if (NVKEY_STATUS_OK != status && NVKEY_STATUS_ITEM_NOT_FOUND != status) {
        APPS_LOG_MSGID_E(LOG_TAG" app_ull_le_set_group_device_addr, error NVKEY status=%d, size: %d", 2, status, size);
        return;
    }
    if (app_ull_le_reverse_compare((uint8_t *)&group_device_addr.addr, addr, BT_BD_ADDR_LEN) && type == group_device_addr.type) {
        APPS_LOG_MSGID_E(LOG_TAG", app_ull_le_set_group_device_addr, addr has exist!", 0);
        return;
    }

    group_device_addr.type = type;
    app_ull_le_reverse_copy((uint8_t *)&group_device_addr.addr, addr, BT_BD_ADDR_LEN);
    status = nvkey_write_data(NVID_APP_LEA_DHSS_PAIR_LE_ADDR, (uint8_t *)&group_device_addr, sizeof(bt_addr_t));
    if (status != NVKEY_STATUS_OK) {
        APPS_LOG_MSGID_E(LOG_TAG" app_ull_le_set_group_device_addr, write nvkey error %d", 1, status);
    }

    bt_ull_le_device_info_t *dev_info = &g_ull_dev_info;
    for (i = 0; i < BT_ULL_LE_MAX_LINK_NUM - 1; i ++) {
        memcpy((uint8_t *)&dev_info->group_device_addr[i], (uint8_t *)&group_device_addr, sizeof(bt_addr_t));
    }
    APPS_LOG_MSGID_I(LOG_TAG" app_ull_le_set_group_device_addr, addr: 0x%x-%x-%x-%x-%x-%x", 6, \
                     group_device_addr.addr[0], group_device_addr.addr[1], group_device_addr.addr[2], group_device_addr.addr[3], \
                     group_device_addr.addr[4], group_device_addr.addr[5]);
    bt_ull_le_srv_set_device_info(dev_info);
#endif
}
#endif

#ifdef MTK_AWS_MCE_ENABLE
static void app_ull_check_and_switch_uplink(bool in_ear_update, bool charger_update, bool bat_update);
void app_ull_switch_to_uplink()
{
    bt_ull_le_client_switch_ul_ind_t param;
    param.is_need_switch_ul = true;
    bt_ull_le_srv_action(BT_ULL_ACTION_SWITCH_UPLINK, &param, sizeof(param));
    APPS_LOG_MSGID_I(LOG_TAG", swith to up link", 0);
}
#endif

static void app_le_ull_storage_config()
{
    app_ull_nvdm_config_data_t config_data = {
        .link_mode = s_ull_context.link_mode,
    };
    config_data.hfp_only_enable = s_hfp_only_enable;
    config_data.a2dp_standby_enable = s_a2dp_standby_enable;
    config_data.speaker_mode = s_speaker_mode;
    nvkey_write_data(NVID_APP_ULL_CONFIG, (uint8_t *)&config_data, sizeof(config_data));
}

void app_le_ull_update_ver()
{
    bt_ull_le_scenario_t scenario = BT_ULL_LE_SCENARIO_ULLV2_0;
    if (s_ull2_ver == APP_ULL2_VER3_0) {
        scenario = BT_ULL_LE_SCENARIO_ULLV3_0;
    }
    nvkey_status_t nv_sta = nvkey_write_data(NVID_APP_ULL2_VERSION, &s_ull2_ver, sizeof(uint8_t));
    bt_status_t ret = bt_ull_action(BT_ULL_ACTION_SET_ULL_SCENARIO, &scenario, sizeof(bt_ull_le_scenario_t));
    APPS_LOG_MSGID_I(LOG_TAG" VER switch, type=%d, ret=%d, nv_sta=%d", 3, scenario, ret, nv_sta);
}

static atci_status_t app_le_ull_atci_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    char *pChar = NULL;
    atci_response_t *response = NULL;
    bt_key_t sirk = {0};

    response = (atci_response_t *)pvPortMalloc(sizeof(atci_response_t));
    if (response == NULL) {
        return ATCI_STATUS_ERROR;
    }
    memset(response, 0, sizeof(atci_response_t));
    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;

    /* SIRK */
    /* AT+LEULL=SIRK,<ACTION> */
    /* <ACTION>: SET, GET */

    pChar = parse_cmd->string_ptr + parse_cmd->name_len + 1;
    if (0 == memcmp(pChar, "SIRK", 4)) {
        pChar = strchr(pChar, ',');
        pChar++;

        if (0 == memcmp(pChar, "GET", 3)) {
            uint8_t temp_str[50] = {0};
            app_le_ull_get_sirk(&sirk);
            snprintf((char *)temp_str, sizeof(temp_str), "%.2X,%.2X,%.2X,%.2X,%.2X,%.2X,%.2X,%.2X,%.2X,%.2X,%.2X,%.2X,%.2X,%.2X,%.2X,%.2X",
                     sirk[0], sirk[1], sirk[2], sirk[3], sirk[4], sirk[5], sirk[6], sirk[7],
                     sirk[8], sirk[9], sirk[10], sirk[11], sirk[12], sirk[13], sirk[14], sirk[15]);
            snprintf((char *)response->response_buf, sizeof(response->response_buf), "SIRK:%s\r\n", (char *)temp_str);
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;

        } else if (0 == memcmp(pChar, "SET", 3)) {
            /* AT+LEAUDIO=SIRK,SET,<B0>,<B1>,<B2>,<B3>,<B4>,<B5>,<B6>,<B7>,<B8>,<B9>,<B10>,<B11>,<B12>,<B13>,<B14>,<B15> */
            pChar = strchr(pChar, ',');
            pChar++;
            if (sscanf(pChar, "%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x",
                       (unsigned int *)&sirk[0], (unsigned int *)&sirk[1], (unsigned int *)&sirk[2], (unsigned int *)&sirk[3],
                       (unsigned int *)&sirk[4], (unsigned int *)&sirk[5], (unsigned int *)&sirk[6], (unsigned int *)&sirk[7],
                       (unsigned int *)&sirk[8], (unsigned int *)&sirk[9], (unsigned int *)&sirk[10], (unsigned int *)&sirk[11],
                       (unsigned int *)&sirk[12], (unsigned int *)&sirk[13], (unsigned int *)&sirk[14], (unsigned int *)&sirk[15]) == 16) {
                app_le_ull_write_nvkey_sirk(&sirk);
                response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            } else {
                response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            }
        }
    } else if (0 == memcmp(pChar, "ADV", 3)) {
        pChar = strchr(pChar, ',');
        pChar++;

        if (0 == memcmp(pChar, "GET", 3)) {
            if (s_ull_adv_en) {
                snprintf((char *)response->response_buf, sizeof(response->response_buf), "ENABLE");
            } else {
                snprintf((char *)response->response_buf, sizeof(response->response_buf), "DISABLE");
            }
            response->response_flag = ATCI_RESPONSE_FLAG_AUTO_APPEND_LF_CR;
        } else if (0 == memcmp(pChar, "start", 5)) {
            app_le_ull_set_advertising_enable(true, false, false);
            s_cmd_adv_disable = false;
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            multi_ble_adv_manager_start_ble_adv();
        } else if (0 == memcmp(pChar, "stop", 4)) {
            app_le_ull_set_advertising_enable(false, false, false);
            s_cmd_adv_disable = true;
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            multi_ble_adv_manager_stop_ble_adv();
        } else {
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
        }
    } else if (0 == memcmp(pChar, "GETSTA", 6)) {
        snprintf((char *)response->response_buf, sizeof(response->response_buf), "ULL2STA:%d,%d,%d,%d\r\n",
                 s_ull_adv_en, s_ull_context.link_mode, s_hfp_only_enable, s_a2dp_standby_enable);
        APPS_LOG_MSGID_I(LOG_TAG" ULL2STA:%d,%d,%d,%d", 4,
                         s_ull_adv_en, s_ull_context.link_mode, s_hfp_only_enable, s_a2dp_standby_enable);
        response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
    } else if (0 == memcmp(pChar, "DISCONNECT", 10)) {
        s_cmd_adv_disable = true;
        app_le_ull_set_advertising_enable(false, false, false);
        app_le_ull_disconnect_dongle();
        response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
    } else if (0 == memcmp(pChar, "HFPONLY", 7)) {
        pChar = strchr(pChar, ',');
        pChar++;

        if (0 == memcmp(pChar, "ON", 2)) {
            s_hfp_only_enable = true;
        } else if (0 == memcmp(pChar, "OFF", 3)) {
            s_hfp_only_enable = false;
        }

        app_le_ull_storage_config();
    } else if (0 == memcmp(pChar, "A2DPSTANDBY", 11)) {
        pChar = strchr(pChar, ',');
        pChar++;

        if (0 == memcmp(pChar, "ON", 2)) {
            s_a2dp_standby_enable = true;
            bt_a2dp_enable_service_record(true);
            bt_avrcp_disable_sdp(false);
        } else if (0 == memcmp(pChar, "OFF", 3)) {
            s_a2dp_standby_enable = false;
            bt_a2dp_enable_service_record(false);
            bt_avrcp_disable_sdp(true);
        }

        app_le_ull_storage_config();
    } else if (0 == memcmp(pChar, "SPEAKERMODE", 11)) {
        pChar = strchr(pChar, ',');
        pChar++;

        if (0 == memcmp(pChar, "ON", 2)) {
            s_speaker_mode = true;
        } else if (0 == memcmp(pChar, "OFF", 3)) {
            s_speaker_mode = false;
        }

        app_le_ull_storage_config();
    } else if (0 == memcmp(pChar, "line_in", 7)) {
        pChar = strchr(pChar, ',');
        pChar++;

        bool plug_in = false;
        if (0 == memcmp(pChar, "in", 2)) {
            plug_in = true;
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_LINE_IN_PLUG_STATE,
                                (void *)plug_in, 0, NULL, 0);
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
        } else if (0 == memcmp(pChar, "out", 3)) {
            plug_in = false;
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_LINE_IN_PLUG_STATE,
                                (void *)plug_in, 0, NULL, 0);
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
        } else {
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
        }
    } else if (0 == memcmp(pChar, "usb_in", 6)) {
        pChar = strchr(pChar, ',');
        pChar++;

        bool plug_in = false;
        if (0 == memcmp(pChar, "in", 2)) {
            plug_in = true;
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_USB_PLUG_STATE,
                                (void *)plug_in, 0, NULL, 0);
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
        } else if (0 == memcmp(pChar, "out", 3)) {
            plug_in = false;
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_USB_PLUG_STATE,
                                (void *)plug_in, 0, NULL, 0);
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
        } else {
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
        }
    } else if (0 == memcmp(pChar, "ADDR", 4)) {
        pChar = strchr(pChar, ',');
        pChar++;

        if (0 == memcmp(pChar, "GET", 3)) {
            uint8_t temp_addr[20] = {0};
            bt_bd_addr_t *temp_addr_ptr = NULL;
            temp_addr_ptr =  multi_ble_adv_get_instance_address(MULTI_ADV_INSTANCE_NOT_RHO);
            if (temp_addr_ptr != NULL) {
                memcpy(temp_addr, temp_addr_ptr, sizeof(bt_bd_addr_t));
            }

            snprintf((char *)temp_addr, sizeof(temp_addr), "%.2X:%.2X:%.2X:%.2X:%.2X:%.2X",
                     temp_addr[0], temp_addr[1], temp_addr[2], temp_addr[3], temp_addr[4], temp_addr[5]);
            snprintf((char *)response->response_buf, sizeof(response->response_buf), "ADDR:%s\r\n", (char *)temp_addr);
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
        }
    }
#ifndef AIR_WIRELESS_MIC_ENABLE
    else if (0 == memcmp(pChar, "CODEC", 5)) {
        pChar = strchr(pChar, ',');
        pChar++;
        if (0 == memcmp(pChar, "OPUS", 4)) {
            bt_ull_le_srv_set_client_preferred_codec_type(BT_ULL_LE_CODEC_OPUS);
            bt_ull_le_srv_set_opus_codec_param();
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
        } else if (0 == memcmp(pChar, "LC3PLUS", 7)) {
            bt_ull_le_srv_set_client_preferred_codec_type(BT_ULL_LE_CODEC_LC3PLUS);
            bt_ull_le_srv_reset_stream_ctx();
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
        } else {
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
        }

    }
#endif
#ifdef AIR_DCHS_MODE_ENABLE
    else if (0 == memcmp(pChar, "SAMPLERATE", 10)) {
        pChar = strchr(pChar, ',');
        pChar++;
        if (0 == memcmp(pChar, "96000", 5)) {
            dchs_dl_set_audio_sample_rate(96000);
            bt_ull_le_srv_set_client_preffered_dl_codec_samplerate(96000);
            bt_ull_le_srv_client_preferred_codec_param *codec_param = bt_ull_le_srv_get_client_preferred_codec_param();
            codec_param->dl_samplerate = bt_ull_le_srv_get_client_preffered_dl_codec_samplerate(BT_ULL_LE_SCENARIO_ULLV2_0);
            codec_param->ul_samplerate = bt_ull_le_srv_get_client_preffered_ul_codec_samplerate(BT_ULL_LE_SCENARIO_ULLV2_0);
            bt_ull_le_srv_set_codec_param_by_sample_rate(BT_ULL_ROLE_CLIENT, codec_param->dl_samplerate, codec_param->ul_samplerate);
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
        } else if (0 == memcmp(pChar, "48000", 5)) {
            dchs_dl_set_audio_sample_rate(48000);
            bt_ull_le_srv_set_client_preffered_dl_codec_samplerate(48000);
            bt_ull_le_srv_client_preferred_codec_param *codec_param = bt_ull_le_srv_get_client_preferred_codec_param();
            codec_param->dl_samplerate = bt_ull_le_srv_get_client_preffered_dl_codec_samplerate(BT_ULL_LE_SCENARIO_ULLV2_0);
            codec_param->ul_samplerate = bt_ull_le_srv_get_client_preffered_ul_codec_samplerate(BT_ULL_LE_SCENARIO_ULLV2_0);
            bt_ull_le_srv_set_codec_param_by_sample_rate(BT_ULL_ROLE_CLIENT, codec_param->dl_samplerate, codec_param->ul_samplerate);
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;

        } else {
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
        }
    }
#endif
    else if (0 == memcmp(pChar, "VER", 3)) {
        pChar = strchr(pChar, ',');
        pChar++;
        response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
        if (0 == memcmp(pChar, "3.0", 3)) {
            s_ull2_ver = APP_ULL2_VER3_0;
        } else if (0 == memcmp(pChar, "2.0", 3)) {
            s_ull2_ver = APP_ULL2_VER2_0;
        } else if (0 == memcmp(pChar, "GET", 3)) {
            APPS_LOG_MSGID_I(LOG_TAG" VER=%d", 1, s_ull2_ver);
            snprintf((char *)response->response_buf, sizeof(response->response_buf), "ver=%d, [0->2.0, 1->3.0]", s_ull2_ver);
            goto atci_exit;
        } else {
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
        }
        //app_le_ull_update_ver();
        uint32_t delay_ms = 0;
        if (ULL_LINK_MODE_MULTIPLE== s_ull_context.link_mode && s_ull2_ver == APP_ULL2_VER3_0) {
            delay_ms = 500;
            uint16_t *p_key_action = (uint16_t *)pvPortMalloc(sizeof(uint16_t));
            if (p_key_action) {
                *p_key_action = KEY_ULL_SWITCH_LINK_MODE;
                APPS_LOG_MSGID_I(LOG_TAG"send simulate key action = 0x%x", 1, *p_key_action);
                /* The extra_data of the event is key action. */
                ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_KEY,
                                    INVALID_KEY_EVENT_ID, p_key_action, sizeof(uint16_t), NULL, 0);
            }
        }
        ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_ULL_APP_SWITCH_VER,
                                (void *)(uint32_t)s_ull2_ver, 0, NULL, delay_ms);
    }
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
    else if (0 == memcmp(pChar, "ABR", 3)) {
        pChar = strchr(pChar, ',');
        pChar++;
        bt_status_t status = BT_STATUS_FAIL;
        if (0 == memcmp(pChar, "enable", 6)) {
            pChar = strchr(pChar, ',');
            pChar++;
            uint32_t interval = 0;
            sscanf(pChar, "%d", (int *)(&(interval)));
            bt_ull_le_adaptive_bitrate_params_t adaptive_bitrate_param;
            adaptive_bitrate_param.enable = true;
            adaptive_bitrate_param.crc_threshold = interval / 5 * 3 * 15 / 100;
            adaptive_bitrate_param.flush_timeout_threshold = interval / 5 * 15 / 100;
            adaptive_bitrate_param.report_interval = interval;
            adaptive_bitrate_param.rx_timeout_threshold = interval / 5 * 3 * 15 / 100;
            status = bt_ull_le_srv_enable_adaptive_bitrate_mode(&adaptive_bitrate_param);
            response->response_flag = (BT_STATUS_SUCCESS == status) ? ATCI_RESPONSE_FLAG_APPEND_OK : ATCI_RESPONSE_FLAG_APPEND_ERROR;
        } else if (0 == memcmp(pChar, "disable", 7)) {
            bt_ull_le_adaptive_bitrate_params_t adaptive_bitrate_param;
            adaptive_bitrate_param.enable = false;
            status = bt_ull_le_srv_enable_adaptive_bitrate_mode(&adaptive_bitrate_param);
            response->response_flag = (BT_STATUS_SUCCESS == status) ? ATCI_RESPONSE_FLAG_APPEND_OK : ATCI_RESPONSE_FLAG_APPEND_ERROR;
        } else {
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
        }

    }
    else if (0 == memcmp(pChar, "AUD_QOS", 7)) {
        pChar = strchr(pChar, ',');
        pChar++;
        uint32_t aud_qos_level = 0;
        sscanf(pChar, "%d", (int *)(&(aud_qos_level)));
        bt_status_t status = BT_STATUS_FAIL;
        bt_ull_le_srv_audio_quality_t aud_quality = aud_qos_level;
        status = bt_ull_le_srv_action(BT_ULL_ACTION_SET_AUDIO_QUALITY, &aud_quality, sizeof(bt_ull_le_srv_audio_quality_t));
        response->response_flag = (BT_STATUS_SUCCESS == status) ? ATCI_RESPONSE_FLAG_APPEND_OK : ATCI_RESPONSE_FLAG_APPEND_ERROR;
    }
    else if (0 == memcmp(pChar, "LOW_POWER", 9)) {
        pChar = strchr(pChar, ',');
        pChar++;
        bt_status_t status = BT_STATUS_FAIL;
        if (0 == memcmp(pChar, "ON", 2)) {
            bt_ull_le_srv_audio_quality_t aud_quality = BT_ULL_LE_SRV_AUDIO_QUALITY_LOW_POWER;
            status = bt_ull_le_srv_action(BT_ULL_ACTION_SET_AUDIO_QUALITY, &aud_quality, sizeof(bt_ull_le_srv_audio_quality_t));
            response->response_flag = (BT_STATUS_SUCCESS == status) ? ATCI_RESPONSE_FLAG_APPEND_OK : ATCI_RESPONSE_FLAG_APPEND_ERROR;
        } else if (0 == memcmp(pChar, "OFF", 3)) {
            bt_ull_le_srv_audio_quality_t aud_quality = BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_DEFAULT;
            status = bt_ull_le_srv_action(BT_ULL_ACTION_SET_AUDIO_QUALITY, &aud_quality, sizeof(bt_ull_le_srv_audio_quality_t));
            response->response_flag = (BT_STATUS_SUCCESS == status) ? ATCI_RESPONSE_FLAG_APPEND_OK : ATCI_RESPONSE_FLAG_APPEND_ERROR;
        } else {
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
        }
    }
    else if (0 == memcmp(pChar, "AIRCIS_INACTIVE", 15)) {
        pChar = strchr(pChar, ',');
        pChar++;
        bt_status_t status = BT_STATUS_FAIL;
        if (0 == memcmp(pChar, "ON", 2)) {
            bool aircis_inactive_mode_enable = true;
            status = bt_ull_le_srv_action(BT_ULL_ACTION_SWITCH_AIRCIS_INACTIVE_MODE, &aircis_inactive_mode_enable, sizeof(uint8_t));
            response->response_flag = (BT_STATUS_SUCCESS == status) ? ATCI_RESPONSE_FLAG_APPEND_OK : ATCI_RESPONSE_FLAG_APPEND_ERROR;            
        } else if (0 == memcmp(pChar, "OFF", 3)) {
            bool aircis_inactive_mode_enable = false;
            status = bt_ull_le_srv_action(BT_ULL_ACTION_SWITCH_AIRCIS_INACTIVE_MODE, &aircis_inactive_mode_enable, sizeof(uint8_t));
            response->response_flag = (BT_STATUS_SUCCESS == status) ? ATCI_RESPONSE_FLAG_APPEND_OK : ATCI_RESPONSE_FLAG_APPEND_ERROR;          
        } else {
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
        }
    }
#endif
    else {
        response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
        goto atci_exit;
    }
atci_exit:
    response->response_len = strlen((char *)response->response_buf);
    atci_send_response(response);
    vPortFree(response);
    return ATCI_STATUS_OK;
}

bool app_le_ull_adv_disable_by_cmd()
{
    return s_cmd_adv_disable;
}

static void app_le_ull_callback(bt_ull_event_t event, void *param, uint32_t param_len)
{
    void *extra_data = NULL;
    uint32_t malloc_size = param_len;
    APPS_LOG_MSGID_I(LOG_TAG" set app_le_ull_callback event=%d.", 1, event);
    if (param) {
        if (BT_ULL_EVENT_USER_DATA_IND == event) {
            malloc_size += ((bt_ull_user_data_t *)param)->user_data_length;
        }
        extra_data = pvPortMalloc(malloc_size);
        if (extra_data) {
            memcpy(extra_data, param, param_len);
            if (BT_ULL_EVENT_USER_DATA_IND == event) {
                ((bt_ull_user_data_t *)extra_data)->user_data = ((uint8_t *)extra_data) + sizeof(bt_ull_user_data_t);
                memcpy(((bt_ull_user_data_t *)extra_data)->user_data, ((bt_ull_user_data_t *)param)->user_data, ((bt_ull_user_data_t *)param)->user_data_length);
            }
        } else {
            return;
        }
    }
    ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_BT_ULTRA_LOW_LATENCY,
                        event, extra_data, param_len, NULL, 0);
}

#ifdef AIR_DCHS_MODE_MASTER_ENABLE
#define APP_ULL2_SAMPLE_RATE_48K 0
#define APP_ULL2_SAMPLE_RATE_96K 1
void app_le_ull_set_ull_sample_rate(uint8_t type)
{
    nvkey_write_data(NVID_APP_ULL2_SAMPLE_RATE, &type, sizeof(uint8_t));
    ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                        APPS_EVENTS_INTERACTION_REQUEST_REBOOT, NULL, 0,
                        NULL, 0);
}
#endif

extern void bt_ull_atci_init(void);
static uint32_t app_le_ull_init()
{
#ifdef AIR_DCHS_MODE_MASTER_ENABLE
    /* set ull2 sample rate at first */
    uint8_t sample_rate = APP_ULL2_SAMPLE_RATE_96K;
    uint32_t nvdm_read_size = sizeof(uint8_t);
    if (NVKEY_STATUS_OK != nvkey_read_data(NVID_APP_ULL2_SAMPLE_RATE, &sample_rate, &nvdm_read_size)) {
        sample_rate = APP_ULL2_SAMPLE_RATE_96K;
    }
    uint32_t sample_rate_r = sample_rate == APP_ULL2_SAMPLE_RATE_48K ? 48000 : 96000;
    APPS_LOG_MSGID_I(LOG_TAG" set ull sample rate to=%d.", 1, sample_rate_r);
    dchs_dl_set_audio_sample_rate(sample_rate_r);
    bt_ull_le_srv_set_client_preffered_dl_codec_samplerate(sample_rate_r);
#endif
    bt_ull_init(BT_ULL_ROLE_CLIENT, app_le_ull_callback);
    bt_ull_atci_init();
    bt_ull_le_device_info_t *dev_info = &g_ull_dev_info;
#ifndef AIR_WIRELESS_MIC_ENABLE
#ifdef AIR_HEADSET_ENABLE
    dev_info->client_type = s_speaker_mode ? BT_ULL_SPEAKER_CLIENT : BT_ULL_HEADSET_CLIENT;
    dev_info->size = s_speaker_mode ? 2 : 1;
#endif
#endif
    app_le_ull_get_sirk(&dev_info->sirk);

#ifdef AIR_LE_AUDIO_DUALMODE_ENABLE
    if (app_lea_feature_mode == APP_LEA_FEATURE_MODE_DUAL_MODE) {
#ifdef AIR_HEADSET_ENABLE
            bt_bd_addr_t *local_addr = bt_device_manager_get_local_address();
#else
            bt_bd_addr_t *local_addr = bt_device_manager_aws_local_info_get_fixed_address();
#endif
            memcpy(&dev_info->local_random_addr, local_addr, sizeof(bt_bd_addr_t));

    } else
        multi_ble_adv_manager_get_random_addr_and_adv_handle(MULTI_ADV_INSTANCE_NOT_RHO, &dev_info->local_random_addr, NULL);

#else
    multi_ble_adv_manager_get_random_addr_and_adv_handle(MULTI_ADV_INSTANCE_NOT_RHO, &dev_info->local_random_addr, NULL);
#endif

#if defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE) && defined (MTK_AWS_MCE_ENABLE)
    bt_addr_t addr;
    if (app_ull_le_read_group_device_addr(&addr)) {
        memcpy((uint8_t *)&dev_info->group_device_addr, (uint8_t *)&addr, sizeof(bt_addr_t));
    }
#endif
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
    bt_ull_le_srv_set_device_info(dev_info);
#endif
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
    bt_ull_le_hid_srv_set_device_info(dev_info);
#endif
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
    app_ull_le_load_and_set_config();
#endif
    return 0;
}

void app_le_ull_set_advertising_enable(bool enable, bool general_adv, bool pairing)
{
    s_ull_adv_en = enable;
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_LE_ULL_CONNECT_TIMEOUT);
    if (enable) {
        app_lea_service_control_adv_data(APP_LEA_SERVICE_ADV_DATA_ULL, true);
        app_lea_service_start_advertising((general_adv ? APP_LEA_ADV_MODE_GENERAL : APP_LEA_ADV_MODE_TARGET_ALL),
                                          false, 0);
        if (pairing) {
            // only for ULL2 General ADV (by Pairing Key trigger)
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_LE_ULL_CONNECT_TIMEOUT, NULL, 0,
                                NULL, 120 * 1000);
        }
    } else {
        app_lea_service_control_adv_data(APP_LEA_SERVICE_ADV_DATA_ULL, false);
#ifndef AIR_LE_AUDIO_ENABLE
        app_lea_service_stop_advertising(false);
#endif
    }
    APPS_LOG_MSGID_I(LOG_TAG" set advertising to=%d.", 1, enable);
}

void app_le_ull_disconnect_dongle()
{
    APPS_LOG_MSGID_I(LOG_TAG" app_le_ull_disconnect_dongle.", 0);
    app_lea_service_disconnect(false,
                               APP_LE_AUDIO_DISCONNECT_MODE_DISCONNECT_ULL,
                               NULL,
                               BT_HCI_STATUS_REMOTE_TERMINATED_CONNECTION_DUE_TO_LOW_RESOURCES);
    s_waiting_disconnect_event = true;
}

bool app_ull_is_le_ull_connected(void)
{
    return s_ull_le_connected;
}

#endif

static bt_status_t app_ull_connect_correct_profile(bt_bd_addr_t *addr)
{
    bt_cm_connect_t param;
    bt_status_t ret;
    if (!addr || 0 == memcmp(addr, s_empty_address, sizeof(bt_bd_addr_t))) {
        return BT_STATUS_FAIL;
    }
    if (0 == memcmp(addr, s_ull_context.dongle_bt_address, sizeof(bt_bd_addr_t))) {
        param.profile = BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_CUSTOMIZED_ULL);
    } else {
        param.profile = BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP)
                        | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK)
                        | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AVRCP)
#ifdef MTK_IAP2_PROFILE_ENABLE
                        | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_CUSTOMIZED_IAP2)
#endif
                        ;
    }
    memcpy(param.address, addr, sizeof(bt_bd_addr_t));
    ret = bt_cm_connect(&param);
    APPS_LOG_MSGID_I(LOG_TAG", app_ull_connect_correct_profile() profile %x, ret = %x", 2, param.profile, ret);
    return ret;
}

static void app_ull_get_connected_devices_list(bt_bd_addr_t *conn_sp_list, uint32_t *conn_sp_count, bool *dongle_connected)
{
    bt_bd_addr_t connected_address[3];
    uint32_t connected_num = 3;
    uint32_t i;
    uint32_t sp_count = 0;
    if (dongle_connected) {
        *dongle_connected = false;
    }
    if (conn_sp_list && conn_sp_count && *conn_sp_count > 0) {
        memset(conn_sp_list, 0, (*conn_sp_count) * sizeof(bt_bd_addr_t));
    }
#ifdef AIR_HEADSET_ENABLE
    connected_num = bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK_NONE,
                                                connected_address, connected_num);
#else
    connected_num = bt_cm_get_connected_devices(~BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS),
                                                connected_address, connected_num);
#endif
    for (i = 0; i < connected_num; i++) {
        if (ULL_IS_DONGLE_ADDRESS(connected_address[i])) {
            if (dongle_connected) {
                *dongle_connected = true;
            }
        } else if (!ULL_IS_LOCAL_ADDRESS(connected_address[i])) {
            if (conn_sp_list && conn_sp_count && *conn_sp_count > 0 && sp_count < *conn_sp_count) {
                memcpy(conn_sp_list[sp_count], connected_address[i], sizeof(bt_bd_addr_t));
                sp_count++;
            } else {
                /* List is full. */
                break;
            }
        }
    }
    if (conn_sp_list && conn_sp_count && *conn_sp_count > 0) {
        *conn_sp_count = sp_count;
        APPS_LOG_MSGID_I(LOG_TAG", app_ull_get_connected_devices_list(), search out %d connect SP.", 1, sp_count);
    }
}

static void app_ull_get_connecting_devices_list(bt_bd_addr_t *conn_sp_list, uint32_t *conn_sp_count, bool *dongle_connecting)
{
    bt_bd_addr_t connecting_address[3];
    uint32_t connecting_num = 3;
    uint32_t i;
    uint32_t sp_count = 0;

    if (dongle_connecting) {
        *dongle_connecting = false;
    }
    if (conn_sp_list && conn_sp_count && *conn_sp_count > 0) {
        memset(conn_sp_list, 0, (*conn_sp_count) * sizeof(bt_bd_addr_t));
    }

    connecting_num = bt_cm_get_connecting_devices(BT_CM_PROFILE_SERVICE_MASK_NONE,
                                                  connecting_address, connecting_num);

    for (i = 0; i < connecting_num; i++) {
        if (ULL_IS_DONGLE_ADDRESS(connecting_address[i])) {
            if (dongle_connecting) {
                *dongle_connecting = true;
            }
        } else if (!ULL_IS_LOCAL_ADDRESS(connecting_address[i])) {
            if (conn_sp_list && conn_sp_count && sp_count < *conn_sp_count) {
                memcpy(conn_sp_list[sp_count], connecting_address[i], sizeof(bt_bd_addr_t));
                sp_count++;
            }
        }
    }
    if (conn_sp_list && conn_sp_count && *conn_sp_count > 0) {
        *conn_sp_count = sp_count;
        APPS_LOG_MSGID_I(LOG_TAG", app_ull_get_connecting_devices_list(), search out %d connect SP.", 1, sp_count);
    }
}

static void app_ull_get_connected_connecting_devices_list(bt_bd_addr_t *conn_sp_list, uint32_t *conn_sp_count, bool *dongle_connected)
{
    bt_bd_addr_t connecting_address[2];
    uint32_t connecting_num = 3;
    uint32_t i;
    uint32_t j;
    uint32_t sp_count = 0;

    if (conn_sp_count) {
        sp_count = *conn_sp_count;
    }
    app_ull_get_connected_devices_list(conn_sp_list, &sp_count, dongle_connected);

    connecting_num = bt_cm_get_connecting_devices(BT_CM_PROFILE_SERVICE_MASK_NONE,
                                                  connecting_address, connecting_num);

    for (i = 0; i < connecting_num; i++) {
        if (ULL_IS_DONGLE_ADDRESS(connecting_address[i])) {
            if (dongle_connected) {
                *dongle_connected = true;
            }
        } else if (!ULL_IS_LOCAL_ADDRESS(connecting_address[i])) {
            bool duplicated_sp;
            if (conn_sp_list && conn_sp_count && *conn_sp_count > 0 && sp_count < *conn_sp_count) {
                duplicated_sp = false;
                for (j = 0; j < sp_count; j++) {
                    if (ULL_IS_ADDRESS_THE_SAME(connecting_address[i], conn_sp_list[j])) {
                        duplicated_sp = true;
                        APPS_LOG_MSGID_I(LOG_TAG", duplicate sp in connecting list", 0);
                        break;
                    }
                }
                if (!duplicated_sp) {
                    memcpy(conn_sp_list[sp_count], connecting_address[i], sizeof(bt_bd_addr_t));
                    sp_count++;
                }
            }
        }
    }
    if (conn_sp_list && conn_sp_count && *conn_sp_count > 0) {
        *conn_sp_count = sp_count;
        APPS_LOG_MSGID_I(LOG_TAG", app_ull_get_connected_connecting_devices_list(), search out %d connect SP.", 1, sp_count);
    }
}

#ifdef AIR_LE_AUDIO_ENABLE
extern bool app_ull_is_lea_connected(void);
#endif
static void app_ull_process_events(ull_ui_events_t event, bt_bd_addr_t *current_addr)
{
    bt_cm_connect_t cm_param;
    bt_bd_addr_t connect_sp[2];
    uint32_t connect_num = 2;
    bool dongle_connected;
    uint32_t i;

    switch (event) {
        case ULL_EVENTS_DONGLE_CONNECTED: {
            bt_bd_addr_t *connected_sp = NULL;
            bt_bd_addr_t connecting_sp[2];
            uint32_t connecting_num = 2;

            s_ull_le_connected = true;
#ifdef APPS_SLEEP_AFTER_NO_CONNECTION
            app_power_save_utils_notify_mode_changed(false, NULL);
#endif
            app_ull_get_connecting_devices_list(connecting_sp, &connecting_num, NULL);
            app_ull_get_connected_devices_list(connect_sp, &connect_num, NULL);
            if (ULL_LINK_MODE_MULTIPLE == s_ull_context.link_mode
                && !s_a2dp_standby_enable) {
                bt_a2dp_enable_service_record(false);
                bt_avrcp_disable_sdp(true);
            }

#ifdef MTK_AWS_MCE_ENABLE
            bool connected = true;
            apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                           APPS_EVENTS_INTERACTION_ULL_DONGLE_CONNECTION_CHANGE,
                                           &connected, sizeof(connected));
#endif

#ifdef AIR_BLE_ULL_TAKEOVER_ENABLE
            if (ULL_LINK_MODE_SINGLE == s_ull_context.link_mode || !s_hfp_only_enable) {
                bt_app_common_pre_set_ultra_low_latency_retry_count(BT_APP_COMMON_ULL_LATENCY_MODULE_DISCOVERABLE, APPS_ULL_STREAMING_RETRY_COUNT_FOR_SINGLE_LINK);
                bt_app_common_pre_set_ultra_low_latency_retry_count(BT_APP_COMMON_ULL_LATENCY_MODULE_RECONNECT, APPS_ULL_STREAMING_RETRY_COUNT_FOR_SINGLE_LINK);
                bt_app_common_apply_ultra_low_latency_retry_count();
                APPS_LOG_MSGID_I(LOG_TAG ", take over enabled, will not dis sp here, mode=%d, en=%d.", 2,
                                 s_ull_context.link_mode, s_hfp_only_enable);
                break;
            }
#endif
            /* When dongle connected,
             * if single mode, disconnect all other device.
             * if multi mode, keep HPF only.
             */
            app_ull_get_connected_devices_list(connect_sp, &connect_num, NULL);
            for (i = 0; i < connect_num; i++) {
                if (ULL_LINK_MODE_SINGLE == s_ull_context.link_mode) {
                    cm_param.profile = BT_CM_PROFILE_SERVICE_MASK_ALL;
                } else {
                    if (i == 0) {
                        /* Multi link mode support the first SP connect HFP. */
                        connected_sp = &connect_sp[i];
                        cm_param.profile = ~(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP)
                                             | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS));
                        if (s_a2dp_standby_enable) {
                            cm_param.profile &= ~(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK)
                                                  | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AVRCP));
                        }
                    } else {
                        cm_param.profile = BT_CM_PROFILE_SERVICE_MASK_ALL;
                    }
                }
                memcpy(cm_param.address, connect_sp[i], sizeof(bt_bd_addr_t));
                bt_cm_disconnect(&cm_param);
            }

            /* Set latency. */
#ifdef MTK_AWS_MCE_ENABLE
            if (connected_sp == NULL && BT_AWS_MCE_ROLE_PARTNER == bt_device_manager_aws_local_info_get_role()) {
                if (BT_AWS_MCE_SRV_LINK_NORMAL == bt_aws_mce_srv_get_link_type()) {
                    connected_sp = &connect_sp[0];//workaround, no useful, just make it not equal NULL
                }
            }
#endif
            if (ULL_LINK_MODE_SINGLE == s_ull_context.link_mode) {
                bt_app_common_pre_set_ultra_low_latency_retry_count(BT_APP_COMMON_ULL_LATENCY_MODULE_RECONNECT, APPS_ULL_STREAMING_RETRY_COUNT_FOR_SINGLE_LINK);
            } else if (NULL != connected_sp) {
                if (s_a2dp_standby_enable) {
                    bt_app_common_pre_set_ultra_low_latency_retry_count(BT_APP_COMMON_ULL_LATENCY_MODULE_RECONNECT, BT_APP_COMMON_ULL_STREAM_RETRY_COUNT_FOR_MULTI_LINK_A2DP);
                } else {
                    bt_app_common_pre_set_ultra_low_latency_retry_count(BT_APP_COMMON_ULL_LATENCY_MODULE_RECONNECT, BT_APP_COMMON_ULL_STREAM_RETRY_COUNT_FOR_MULTI_LINK);
                }
            }
#ifdef AIR_LE_AUDIO_ENABLE
            else if (app_ull_is_lea_connected()) {
                bt_app_common_pre_set_ultra_low_latency_retry_count(BT_APP_COMMON_ULL_LATENCY_MODULE_RECONNECT, BT_ULL_LE_SRV_LATENCY_MULTI_LINK_LEA_7_5MS_30MS_STANDBY_MODE);
            }
#endif
            else {
                bt_app_common_pre_set_ultra_low_latency_retry_count(BT_APP_COMMON_ULL_LATENCY_MODULE_RECONNECT, BT_APP_COMMON_ULL_STREAM_RETRY_COUNT_FOR_CONNECTING);
            }
            bt_app_common_apply_ultra_low_latency_retry_count();
            ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_ULL_RECONNECT_TIMEOUT);
            break;
        }
        case ULL_EVENTS_DONGLE_DISCONNECTED: {
            s_ull_le_connected = false;
#ifdef APPS_SLEEP_AFTER_NO_CONNECTION
            app_power_save_utils_notify_mode_changed(false, NULL);
#endif
            if (!s_a2dp_standby_enable) {
                bt_a2dp_enable_service_record(true);
                bt_avrcp_disable_sdp(false);
            }
            if (s_waiting_disconnect_event) {
                s_waiting_disconnect_event = false;
            }

            /* To avoid smart phone reconnet A2DP when BT OFF -> ON. */
            cm_param.profile = 0;
            if (!s_a2dp_standby_enable) {
                cm_param.profile |= BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK);
            }
#ifdef MTK_IAP2_PROFILE_ENABLE
            cm_param.profile |= BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_CUSTOMIZED_IAP2)
#endif
                                ;
            if (cm_param.profile != BT_CM_PROFILE_SERVICE_MASK_NONE
                && APP_BT_STATE_POWER_STATE_NONE_ACTION == app_bt_connection_service_get_current_status()->target_power_state
                && APP_BT_STATE_POWER_STATE_ENABLED == app_bt_connection_service_get_current_status()->current_power_state) {
                app_ull_get_connected_connecting_devices_list(connect_sp, &connect_num, NULL);
#ifdef MTK_AWS_MCE_ENABLE
                if(BT_AWS_MCE_ROLE_PARTNER == bt_device_manager_aws_local_info_get_role()) {
                    connect_num = 0;
                }
#endif
                for (i = 0; i < connect_num; i++) {
                    memcpy(cm_param.address, connect_sp[i], sizeof(bt_bd_addr_t));
                    bt_cm_connect(&cm_param);
                }
            }
#ifdef MTK_AWS_MCE_ENABLE
            bool connected = false;
            apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                           APPS_EVENTS_INTERACTION_ULL_DONGLE_CONNECTION_CHANGE,
                                           &connected, sizeof(connected));
#endif
            break;
        }
        case ULL_EVENTS_SP_CONNECTED: {
            ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_ULL_RECONNECT_TIMEOUT);
#ifdef AIR_BLE_ULL_TAKEOVER_ENABLE
            if (ULL_LINK_MODE_SINGLE == s_ull_context.link_mode || !s_hfp_only_enable) {
                APPS_LOG_MSGID_I(LOG_TAG ", take over enabled, will not dis dongle here, mode=%d, en=%d.", 2,
                                 s_ull_context.link_mode, s_hfp_only_enable);
                break;
            }
#endif
            app_ull_get_connected_connecting_devices_list(connect_sp, &connect_num, NULL);
            if (ULL_LINK_MODE_SINGLE == s_ull_context.link_mode && s_ull_le_connected) {
                app_le_ull_disconnect_dongle();
            }

            bt_app_common_pre_set_ultra_low_latency_retry_count(BT_APP_COMMON_ULL_LATENCY_MODULE_DISCOVERABLE, APPS_ULL_STREAMING_RETRY_COUNT_FOR_SINGLE_LINK);
            if (ULL_LINK_MODE_SINGLE == s_ull_context.link_mode) {
                bt_app_common_pre_set_ultra_low_latency_retry_count(BT_APP_COMMON_ULL_LATENCY_MODULE_RECONNECT, APPS_ULL_STREAMING_RETRY_COUNT_FOR_SINGLE_LINK);
            } else {
                bt_app_common_pre_set_ultra_low_latency_retry_count(BT_APP_COMMON_ULL_LATENCY_MODULE_DISCOVERABLE, BT_APP_COMMON_ULL_STREAM_RETRY_COUNT_FOR_MULTI_LINK);
                if (s_a2dp_standby_enable) {
                    bt_app_common_pre_set_ultra_low_latency_retry_count(BT_APP_COMMON_ULL_LATENCY_MODULE_RECONNECT, BT_APP_COMMON_ULL_STREAM_RETRY_COUNT_FOR_MULTI_LINK_A2DP);
                } else {
                    bt_app_common_pre_set_ultra_low_latency_retry_count(BT_APP_COMMON_ULL_LATENCY_MODULE_RECONNECT, BT_APP_COMMON_ULL_STREAM_RETRY_COUNT_FOR_MULTI_LINK);
                }
            }
            //bt_app_common_apply_ultra_low_latency_retry_count();
            break;
        }
        case ULL_EVENTS_SP_DISCONNECTED: {
            if (ULL_LINK_MODE_MULTIPLE == s_ull_context.link_mode) {
                bt_app_common_pre_set_ultra_low_latency_retry_count(BT_APP_COMMON_ULL_LATENCY_MODULE_RECONNECT, BT_APP_COMMON_ULL_STREAM_RETRY_COUNT_FOR_CONNECTING);
                bt_app_common_apply_ultra_low_latency_retry_count();
                if (APP_BT_STATE_POWER_STATE_NONE_ACTION == app_bt_connection_service_get_current_status()->target_power_state
                    && APP_BT_STATE_POWER_STATE_ENABLED == app_bt_connection_service_get_current_status()->current_power_state) {
                    /* Try to reconnect dongle when SP disconnected in multi link mode. */
                    if (!s_ull_le_connected) {
                        app_lea_adv_mgr_enable_ull2_reconnect_mode(true);
                        app_le_ull_set_advertising_enable(true, false, false);
                    } else {
                        if (!s_a2dp_standby_enable) {
                            cm_param.profile = BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK) | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AVRCP);
                            for (i = 0; i < connect_num; i++) {
                                memcpy(cm_param.address, connect_sp[i], sizeof(bt_bd_addr_t));
                                bt_cm_disconnect(&cm_param);
                            }
                        }
                    }
                } else {
                    APPS_LOG_MSGID_I(LOG_TAG ", sp disconnected, adv not enabled, target_power_state=%d, cur_power_sta=%d, aux=%d, usb=%d", 4,
                                     app_bt_connection_service_get_current_status()->target_power_state,
                                     app_bt_connection_service_get_current_status()->current_power_state,
                                     s_ull_context.aux_state,
                                     s_ull_context.usb_audio_state);
                }
            } else {
                if (!s_ull_le_connected) {
                    app_lea_adv_mgr_enable_ull2_reconnect_mode(true);
                    app_le_ull_set_advertising_enable(true, false, false);
                }
            }
            break;
        }
        case ULL_EVENTS_SWITCH_LINK_MODE: {
            if (BT_DEVICE_MANAGER_POWER_STATE_ACTIVE != bt_device_manager_power_get_power_state(BT_DEVICE_TYPE_CLASSIC)) {
                break;
            }
            if (ULL_LINK_MODE_SINGLE == s_ull_context.link_mode) {
                s_ull_context.link_mode = ULL_LINK_MODE_MULTIPLE;
            } else {
                s_ull_context.link_mode = ULL_LINK_MODE_SINGLE;
            }
#ifdef MTK_AWS_MCE_ENABLE
            if (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT) {
                /* Notify partner mode changed. */
                app_ull_nvdm_config_data_t config_data = {
                    .link_mode = s_ull_context.link_mode,
#if (defined AIR_APP_ULL_GAMING_MODE_UI_ENABLE)
                    .game_mode = s_ull_context.game_mode,
#endif
                };
                config_data.hfp_only_enable = s_hfp_only_enable;
                config_data.a2dp_standby_enable = s_a2dp_standby_enable;
                bt_status_t bt_status = apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_ULL_LINK_MODE_CHANGE,
                                                                       &(config_data), sizeof(config_data));
                if (BT_STATUS_SUCCESS != bt_status) {
                    s_ull_link_mode_synced = false;
                } else {
                    s_ull_link_mode_synced = true;
                }
            }
#endif
            app_ull_get_connected_devices_list(connect_sp, &connect_num, &dongle_connected);
            if (ULL_LINK_MODE_MULTIPLE == s_ull_context.link_mode) {
                bt_app_common_pre_set_ultra_low_latency_retry_count(BT_APP_COMMON_ULL_LATENCY_MODULE_RECONNECT, BT_APP_COMMON_ULL_STREAM_RETRY_COUNT_FOR_CONNECTING);

                if (!s_a2dp_standby_enable) {
                    bt_a2dp_enable_service_record(false);
                    bt_avrcp_disable_sdp(true);
                } else {
                    bt_a2dp_enable_service_record(true);
                    bt_avrcp_disable_sdp(false);
                }

                if (0 == connect_num && !s_ull_le_connected) {
                    /* If not connected any SRC, try to connect SP and dongle. */
                    bt_bd_addr_t connecting_sp[2];
                    uint32_t connecting_num = 2;
                    app_ull_get_connected_connecting_devices_list(connecting_sp, &connecting_num, &dongle_connected);
                    if (!s_ull_le_connected) {
                        app_lea_adv_mgr_enable_ull2_reconnect_mode(true);
                        app_le_ull_set_advertising_enable(true, false, false);
                    }
                    if (connecting_num == 0) {
                        bt_device_manager_paired_infomation_t paired_info[2];
                        uint32_t paired_info_count = 2;
                        bt_device_manager_get_paired_list(paired_info, &paired_info_count);
                        for (i = 0; i < paired_info_count; i++) {
                            if (!ULL_IS_DONGLE_ADDRESS(paired_info[i].address)) {
                                app_ull_connect_correct_profile(&paired_info[i].address);
                                break;
                            }
                        }
                    }
                } else if (!s_ull_le_connected && connect_num <= 1) {
                    app_lea_adv_mgr_enable_ull2_reconnect_mode(true);
                    app_le_ull_set_advertising_enable(true, false, false);
                }
            } else {
                bt_app_common_pre_set_ultra_low_latency_retry_count(BT_APP_COMMON_ULL_LATENCY_MODULE_RECONNECT, APPS_ULL_STREAMING_RETRY_COUNT_FOR_SINGLE_LINK);
                s_ull_context.link_mode = ULL_LINK_MODE_SINGLE;
                cm_param.profile = BT_CM_PROFILE_SERVICE_MASK_ALL;
                if (!s_ull_le_connected && connect_num > 0) {
                    /* When sp connected and dongle is not connected, disconnect dongle */
                    app_le_ull_set_advertising_enable(true, false, false);
                    app_le_ull_disconnect_dongle();
                } else if (s_ull_le_connected) {
                    connect_num = sizeof(connect_sp) / sizeof(connect_sp[0]);
                    app_ull_get_connected_connecting_devices_list(connect_sp, &connect_num, NULL);
                    for (i = 0; i < connect_num; i++) {
                        memcpy(cm_param.address, connect_sp[i], sizeof(bt_bd_addr_t));
                        bt_cm_disconnect(&cm_param);
                    }
                }
            }
            bt_app_common_apply_ultra_low_latency_retry_count();
            app_le_ull_storage_config();
            break;
        }
        default:
            break;
    }
}

#if defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE) && !defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE)
static void app_ull_reconnect_request_process(void) {
    bt_bd_addr_t addr_list[2];
    uint32_t addr_list_len = 2;
    bt_bd_addr_t connecting_list[2];
    uint32_t connecting_list_len = 2;
    bool ull_connected = false;
    bool ull_connecting = false;
    bt_status_t sta = BT_STATUS_FAIL;
    bt_cm_connect_t param;
    uint32_t i;
    voice_prompt_param_t vp = {0};
    vp.vp_index = VP_INDEX_SUCCEED;

    if (ULL_LINK_MODE_MULTIPLE == s_ull_context.link_mode) {
        return;
    }

    app_ull_get_connected_devices_list(addr_list, &addr_list_len, &ull_connected);
    app_ull_get_connecting_devices_list(connecting_list, &connecting_list_len, &ull_connecting);

    APPS_LOG_MSGID_I(LOG_TAG", app_ull_reconnect_request_process connected[sp:dongle][%d:%d], connecting[sp:dongle][%d:%d]", 4,
                     addr_list_len, s_ull_le_connected, connecting_list_len, ull_connecting);
    if (s_ull_le_connected && addr_list_len > 0) {
        voice_prompt_play_vp_failed();
        return;
    } else if (addr_list_len > 0 || !s_ull_le_connected) {
        /* When SP is connected or ull is not connecting, reconnect dongle. */
        param.profile = BT_CM_PROFILE_SERVICE_MASK_ALL;
        for (i = 0; i < addr_list_len; i++) {
            memcpy(param.address, addr_list[i], sizeof(bt_bd_addr_t));
            bt_cm_disconnect(&param);
        }
        for (i = 0; i < connecting_list_len; i++) {
            memcpy(param.address, connecting_list[i], sizeof(bt_bd_addr_t));
            bt_cm_disconnect(&param);
        }
        app_lea_adv_mgr_enable_ull2_reconnect_mode(true);
        if (s_ull_adv_en) {
            sta = BT_STATUS_SUCCESS;
        } else {
            app_le_ull_set_advertising_enable(true, false, false);
        }
        voice_prompt_play(&vp, NULL);
    } else if (s_ull_le_connected || (connecting_list_len == 0 && s_ull_adv_en)) {
        /* Reconnect SP. */
        /* Disconnect dongle. */
        //app_le_ull_set_advertising_enable(false);
        app_le_ull_disconnect_dongle();
        if (connecting_list_len > 0) {
            sta = BT_STATUS_SUCCESS;
        } else {
            bt_device_manager_paired_infomation_t pair_info[2];
            uint32_t paired_number = 2;
            bt_bd_addr_t *sp_addr = NULL;
            bt_device_manager_get_paired_list(pair_info, &paired_number);
            for (i = 0; i < paired_number; i++) {
                if (!ULL_IS_DONGLE_ADDRESS(pair_info[i].address)) {
                    sp_addr = &pair_info[i].address;
                    break;
                }
            }
            if (sp_addr != NULL) {
                /* Update time. */
                ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_ULL_RECONNECT_TIMEOUT);
                ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                    APPS_EVENTS_INTERACTION_ULL_RECONNECT_TIMEOUT, NULL, 0,
                                    NULL, ULL_SWITCH_CONNECT_TIME);
                sta = app_ull_connect_correct_profile(sp_addr);
                APPS_LOG_MSGID_I(LOG_TAG", app_ull_reconnect_request_process result:%x.", 1, sta);
            }
        }
        if (BT_STATUS_SUCCESS != sta) {
            vp.vp_index = VP_INDEX_FAILED;
        }
        voice_prompt_play(&vp, NULL);
    } else if (!s_ull_le_connected && addr_list_len == 0 && s_ull_adv_en && connecting_list_len > 0) {
        /* Keep connecting status when both is connecting. */
        voice_prompt_play(&vp, NULL);
        //apps_config_set_vp(VP_INDEX_SUCCEED, false, 0, VOICE_PROMPT_PRIO_MEDIUM, false, NULL);
    }
}
#endif

static bool app_ull_proc_ui_shell_group(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len) {
    /* UI shell internal event must process by this activity, so default is true. */
    bool ret = true;
    switch (event_id) {
        case EVENT_ID_SHELL_SYSTEM_ON_CREATE: {
            memset(&s_ull_context, 0, sizeof(s_ull_context));
            self->local_context = &s_ull_context;
            app_ull_nvdm_config_data_t config_data;
            bt_bd_addr_t dongle_addr;
            uint32_t nvdm_read_size = sizeof(bt_bd_addr_t);
#ifdef AIR_APP_ULL_GAMING_MODE_UI_ENABLE
            s_ull_context.link_mode = ULL_LINK_MODE_MULTIPLE;
#endif
            if (NVKEY_STATUS_OK == nvkey_read_data(NVID_APP_ULL_PEER_BT_ADDRESS, dongle_addr, &nvdm_read_size)) {
                memcpy(s_ull_context.dongle_bt_address, dongle_addr, sizeof(bt_bd_addr_t));
            }
            nvdm_read_size = sizeof(config_data);
            if (NVKEY_STATUS_OK == nvkey_read_data(NVID_APP_ULL_CONFIG, (uint8_t *)&config_data, &nvdm_read_size)) {
                s_ull_context.link_mode = config_data.link_mode;
#ifdef AIR_APP_ULL_GAMING_MODE_UI_ENABLE
                s_ull_context.game_mode = config_data.game_mode;
#endif
                s_hfp_only_enable = config_data.hfp_only_enable;
                s_a2dp_standby_enable = config_data.a2dp_standby_enable;
                s_speaker_mode = config_data.speaker_mode;
            }
            nvdm_read_size = sizeof(uint8_t);
            if (NVKEY_STATUS_OK != nvkey_read_data(NVID_APP_ULL2_VERSION, &s_ull2_ver, &nvdm_read_size)) {
#ifdef AIR_ULL_AUDIO_V3_ENABLE
                s_ull2_ver = APP_ULL2_VER3_0;
#else
                s_ull2_ver = APP_ULL2_VER2_0;
#endif
            }
            app_le_ull_update_ver();
            APPS_LOG_MSGID_I(LOG_TAG", ull speaker mode en=%d, ver=%d", 2, s_speaker_mode, s_ull2_ver);
            app_lea_adv_mgr_enable_ull2_reconnect_mode(true);
#ifndef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
            if (ULL_LINK_MODE_MULTIPLE == s_ull_context.link_mode) {
                bt_app_common_pre_set_ultra_low_latency_retry_count(BT_APP_COMMON_ULL_LATENCY_MODULE_RECONNECT, BT_APP_COMMON_ULL_STREAM_RETRY_COUNT_FOR_CONNECTING);
            } else {
                bt_app_common_pre_set_ultra_low_latency_retry_count(BT_APP_COMMON_ULL_LATENCY_MODULE_RECONNECT, APPS_ULL_STREAMING_RETRY_COUNT_FOR_SINGLE_LINK);
            }
            bt_app_common_apply_ultra_low_latency_retry_count();

            APPS_LOG_MSGID_I(LOG_TAG": create, init link mode:%d", 1, s_ull_context.link_mode);
#else
            app_le_ull_init();
#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
            app_detachable_mic_det_init();
#endif
#ifndef APP_CONN_MGR_RECONNECT_CONTROL
            if (ULL_AUX_USB_AUDIO_STATE_IN != s_ull_context.aux_state && ULL_AUX_USB_AUDIO_STATE_IN != s_ull_context.usb_audio_state) {
                app_le_ull_set_advertising_enable(true, false, false);
            }
#endif
#endif
            atci_status_t ret = atci_register_handler(bt_app_le_ull_at_cmd, sizeof(bt_app_le_ull_at_cmd) / sizeof(atci_cmd_hdlr_item_t));
            APPS_LOG_MSGID_I(LOG_TAG" register atci handler ret=%d.", 1, ret);
#if defined(AIR_BLE_ULL_TAKEOVER_ENABLE) && !defined(AIR_DCHS_MODE_ENABLE) && !defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) && !defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
            ui_shell_start_activity(self, app_ull_takeover_activity_proc, ACTIVITY_PRIORITY_MIDDLE, NULL, 0);
#endif
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
            if (bt_ull_le_hid_srv_get_bonded_device_num(BT_ULL_LE_HID_SRV_DEVICE_HEADSET) > 0) {
                app_le_ull_set_advertising_enable(false, false, false);
            }
#endif
            break;
        }
        default:
            ret = false;
            break;
    }
    return ret;
}

#ifdef AIR_ROTARY_ENCODER_ENABLE
static bool app_ull_proc_rotary_event_group(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len) {
    /* UI shell internal event must process by this activity, so default is true. */
    bool ret = false;

    bsp_rotary_encoder_port_t port;
    bsp_rotary_encoder_event_t event;
    bt_status_t bt_status;
    uint32_t rotary_data;
    if (!extra_data) {
        return ret;
    }
    apps_config_key_action_t key_action = *(uint16_t *)extra_data;
    app_event_rotary_event_decode(&port, &event, &rotary_data, event_id);
    switch (key_action) {
        case KEY_AUDIO_MIX_RATIO_GAME_ADD:
        case KEY_AUDIO_MIX_RATIO_CHAT_ADD: {
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
            if ((app_dongle_service_get_dongle_mode() == APP_DONGLE_SERVICE_DONGLE_MODE_XBOX)
                || (app_dongle_service_get_dongle_mode() == APP_DONGLE_SERVICE_DONGLE_MODE_PC)) {
                apps_dongle_sync_event_send(EVENT_GROUP_UI_SHELL_KEY, key_action);
                ret = true;
                break;
            }
#endif
            uint8_t target_level = 0;
            bt_ull_mix_ratio_t mix_ratio;
            mix_ratio.num_streaming = BT_ULL_MAX_STREAMING_NUM;
            mix_ratio.streamings[0].streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
            mix_ratio.streamings[0].streaming.port = 0; /* gaming streaming port */
            mix_ratio.streamings[1].streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
            mix_ratio.streamings[1].streaming.port = 1; /* chat streaming port */
            if (KEY_AUDIO_MIX_RATIO_CHAT_ADD == key_action) {
                if (s_ull_context.current_mix_ratio_level + rotary_data < ULL_MIX_RATIO_CHAT_MAX_LEVEL) {
                    target_level = s_ull_context.current_mix_ratio_level + rotary_data;
                } else {
                    target_level = ULL_MIX_RATIO_CHAT_MAX_LEVEL;
                }
            } else {
                if (s_ull_context.current_mix_ratio_level > ULL_MIX_RATIO_GAME_MAX_LEVEL + rotary_data) {
                    target_level = s_ull_context.current_mix_ratio_level - rotary_data;
                } else {
                    target_level = ULL_MIX_RATIO_GAME_MAX_LEVEL;
                }
            }
            if (target_level != s_ull_context.current_mix_ratio_level) {
                mix_ratio.streamings[0].ratio = (target_level <= ULL_MIX_RATIO_BALANCED_LEVEL) ?
                                                100 :
                                                100 * (ULL_MIX_RATIO_CHAT_MAX_LEVEL - target_level) / (ULL_MIX_RATIO_CHAT_MAX_LEVEL - ULL_MIX_RATIO_BALANCED_LEVEL);
                mix_ratio.streamings[1].ratio = (target_level >= ULL_MIX_RATIO_BALANCED_LEVEL) ?
                                                100 :
                                                100 * (target_level - ULL_MIX_RATIO_GAME_MAX_LEVEL) / (ULL_MIX_RATIO_BALANCED_LEVEL - ULL_MIX_RATIO_GAME_MAX_LEVEL);
                bt_status = bt_ull_action(BT_ULL_ACTION_SET_STREAMING_MIX_RATIO, &mix_ratio, sizeof(mix_ratio));
                if (BT_STATUS_SUCCESS == bt_status) {
                    s_ull_context.current_mix_ratio_level = target_level;
                }
            }
            ret = true;
            break;
        }
        default:
            break;
    }

    return ret;
}
#endif

#if defined(MTK_AWS_MCE_ENABLE)
#ifdef MTK_IN_EAR_FEATURE_ENABLE
#include "app_in_ear_utils.h"
#endif
#ifdef AIR_SMART_CHARGER_ENABLE
#include "app_smcharger.h"
#else
#if defined(MTK_BATTERY_MANAGEMENT_ENABLE) && !(defined (AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE) || defined(AIR_DCHS_MODE_SLAVE_ENABLE))
#include "app_battery_idle_activity.h"
#include "app_battery_transient_activity.h"
#endif
#endif
static void app_ull_check_and_switch_uplink(bool in_ear_update, bool charger_update, bool bat_update)
{
    bool switch_to_per = false;
    bool switch_to_local = false;

#ifdef MTK_IN_EAR_FEATURE_ENABLE
    app_in_ear_state_t in_ear_sta = app_in_ear_get_state();
    if (in_ear_update) {
        bool in_ear = app_in_ear_get_own_state();
        bool per_in_ear = app_in_ear_get_peer_state();
        if (!in_ear && per_in_ear) {
            switch_to_per = true;
        } else if (in_ear && !per_in_ear) {
            switch_to_local = true;
        }
        APPS_LOG_MSGID_I(LOG_TAG", app_ull_check_and_switch_uplink in_ear local=%d, per=%d", 2, in_ear, per_in_ear);
    }
#endif

    bool charging = false;
    bool per_charging = false;
#ifdef AIR_SMART_CHARGER_ENABLE
    if (charger_update) {
        charging = app_smcharger_is_charging() == APP_SMCHARGER_IN;
        per_charging = app_smcharger_peer_is_charging() == APP_SMCHARGER_IN;
        if (charging && !per_charging) {
            switch_to_per = true;
        } else if (per_charging && !charging) {
            switch_to_local = true;
        }
        APPS_LOG_MSGID_I(LOG_TAG", app_ull_check_and_switch_uplink charger local=%d, per=%d", 2, charging, per_charging);
    }
#endif

    if (bat_update) {
        uint8_t bat = 0;
        uint8_t per_bat = 0;
#ifdef AIR_SMART_CHARGER_ENABLE
        uint8_t case_bat = 0;
        app_smcharger_get_battery_percent(&bat, &per_bat, &case_bat);
#else
#if defined(MTK_BATTERY_MANAGEMENT_ENABLE) && !(defined (AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE) || defined(AIR_DCHS_MODE_SLAVE_ENABLE))
        app_battery_get_battery_percent(&bat, &per_bat);
        charging = bat & PARTNER_BATTERY_CHARGING;
        per_charging = per_bat & PARTNER_BATTERY_CHARGING;
#endif
#endif
        if (!charging && !per_charging
#ifdef MTK_IN_EAR_FEATURE_ENABLE
            && (in_ear_sta == APP_IN_EAR_STA_BOTH_IN || APP_IN_EAR_STA_BOTH_OUT == in_ear_sta)
#endif
        ) {
            if (bat + 30 < per_bat) {
                switch_to_per = true;
            } else if (per_bat + 30 < bat) {
                switch_to_local = true;
            }
        }
        APPS_LOG_MSGID_I(LOG_TAG", app_ull_check_and_switch_uplink battery local=%d, per=%d", 2, bat, per_bat);
    }

    if (switch_to_local) {
        app_ull_switch_to_uplink();
    }

    if (switch_to_per) {
        apps_aws_sync_event_send(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_SWITCH_TO_ULL2_UPLINK);
    }
}
#endif

static bool app_ull_proc_interaction_event_group(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len) {
    /* UI shell internal event must process by this activity, so default is true. */
    bool ret = false;
    switch (event_id) {
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
        case APPS_EVENTS_INTERACTION_LE_ULL_CONNECT_TIMEOUT: {
#ifdef AIR_BLE_ULL_PARING_MODE_ENABLE
            app_lea_adv_mgr_enable_ull2_pairing_mode(false);
#endif
            break;
        }
#endif
        case APPS_EVENTS_INTERACTION_ULL_RECONNECT_TIMEOUT:
#ifdef MTK_AWS_MCE_ENABLE
            if (bt_device_manager_aws_local_info_get_role() != BT_AWS_MCE_ROLE_PARTNER
                && bt_device_manager_aws_local_info_get_role() != BT_AWS_MCE_ROLE_CLINET)
#endif
            {
                /* Reconnect both device. */
                bt_bd_addr_t reconnecting_addr[2];
                uint32_t reconnecting_addr_len = 2;
                bt_bd_addr_t connectied_addr[2];
                uint32_t connected_addr_len = 2;
                bool dongle_connecting = false;
                bool dongle_connected = false;
                uint32_t i;
                app_ull_get_connecting_devices_list(reconnecting_addr, &reconnecting_addr_len, &dongle_connecting);
                app_ull_get_connected_devices_list(connectied_addr, &connected_addr_len, &dongle_connected);
                APPS_LOG_MSGID_I(LOG_TAG", ULL_RECONNECT_TIMEOUT = connected[sp:dongle], connecting[sp:dongle][%d:%d]", 4,
                                 connected_addr_len > 0, dongle_connected, reconnecting_addr_len > 0, dongle_connecting);
                if (connected_addr_len > 0 || s_ull_le_connected) {
                    /* Already connected, not need reconnect. */
                    break;
                }
                if (reconnecting_addr_len == 0) {
                    bt_device_manager_paired_infomation_t pair_info[2];
                    uint32_t paired_number = 2;
                    bt_bd_addr_t *sp_addr = NULL;
                    bt_device_manager_get_paired_list(pair_info, &paired_number);
                    for (i = 0; i < paired_number; i++) {
                        if (!ULL_IS_DONGLE_ADDRESS(pair_info[i].address)) {
                            sp_addr = &pair_info[i].address;
                            break;
                        }
                    }
                    app_ull_connect_correct_profile(sp_addr);
                }
            }
            break;
#ifdef MTK_AWS_MCE_ENABLE
        case APPS_EVENTS_INTERACTION_PARTNER_SWITCH_TO_AGENT: {
            if (ULL_LINK_MODE_MULTIPLE == s_ull_context.link_mode) {
                bt_bd_addr_t connect_sp[1];
                uint32_t connect_num = 1;
                app_ull_get_connected_devices_list(connect_sp, &connect_num, NULL);
                if (connect_num > 0) {
                    if (s_a2dp_standby_enable) {
                        bt_app_common_pre_set_ultra_low_latency_retry_count(BT_APP_COMMON_ULL_LATENCY_MODULE_RECONNECT, BT_APP_COMMON_ULL_STREAM_RETRY_COUNT_FOR_MULTI_LINK_A2DP);
                    } else {
                        bt_app_common_pre_set_ultra_low_latency_retry_count(BT_APP_COMMON_ULL_LATENCY_MODULE_RECONNECT, BT_APP_COMMON_ULL_STREAM_RETRY_COUNT_FOR_MULTI_LINK);
                    }
                } else {
                    bt_app_common_pre_set_ultra_low_latency_retry_count(BT_APP_COMMON_ULL_LATENCY_MODULE_RECONNECT, BT_APP_COMMON_ULL_STREAM_RETRY_COUNT_FOR_CONNECTING);
                }
            } else {
                bt_app_common_pre_set_ultra_low_latency_retry_count(BT_APP_COMMON_ULL_LATENCY_MODULE_RECONNECT, APPS_ULL_STREAMING_RETRY_COUNT_FOR_SINGLE_LINK);
            }
            break;
        }
#endif
        case APPS_EVENTS_INTERACTION_SET_ULL_MIX_RATIO: {
            bt_status_t bt_status;
            uint8_t *target_level = (uint8_t *)extra_data;
            bt_ull_mix_ratio_t mix_ratio;
            mix_ratio.num_streaming = BT_ULL_MAX_STREAMING_NUM;
            mix_ratio.streamings[0].streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
            mix_ratio.streamings[0].streaming.port = 0; /* gaming streaming port */
            mix_ratio.streamings[1].streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
            mix_ratio.streamings[1].streaming.port = 1; /* chat streaming port */
            if (*target_level != s_ull_context.current_mix_ratio_level) {
                mix_ratio.streamings[0].ratio = (*target_level <= ULL_MIX_RATIO_BALANCED_LEVEL) ?
                                                100 :
                                                100 * (ULL_MIX_RATIO_CHAT_MAX_LEVEL - *target_level) / (ULL_MIX_RATIO_CHAT_MAX_LEVEL - ULL_MIX_RATIO_BALANCED_LEVEL);
                mix_ratio.streamings[1].ratio = (*target_level >= ULL_MIX_RATIO_BALANCED_LEVEL) ?
                                                100 :
                                                100 * (*target_level - ULL_MIX_RATIO_GAME_MAX_LEVEL) / (ULL_MIX_RATIO_BALANCED_LEVEL - ULL_MIX_RATIO_GAME_MAX_LEVEL);
                bt_status = bt_ull_action(BT_ULL_ACTION_SET_STREAMING_MIX_RATIO, &mix_ratio, sizeof(mix_ratio));
                if (BT_STATUS_SUCCESS == bt_status) {
                    s_ull_context.current_mix_ratio_level = *target_level;
                }
            }
            ret = true;
            break;
        }
        case APPS_EVENTS_INTERACTION_SET_ULL_MIC_VOL: {
            uint8_t *mic_vol = (uint8_t *)extra_data;
            bt_ull_streaming_info_t info = {0};
            bt_ull_streaming_t streaming = {
                .streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE,
                .port = 0,
            };

            if ((BT_STATUS_SUCCESS == bt_ull_get_streaming_info(streaming, &info)) && info.is_playing) {
                bt_status_t bt_status;
                bt_ull_volume_t volume_param;
                volume_param.streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
                volume_param.streaming.port = 0;
                volume_param.channel = BT_ULL_VOLUME_CHANNEL_DUEL;
                volume_param.volume = *mic_vol;
                volume_param.action = BT_ULL_VOLUME_ACTION_SET_ABSOLUTE_VOLUME;
                bt_status = bt_ull_action(BT_ULL_ACTION_SET_STREAMING_VOLUME, &volume_param, sizeof(volume_param));
                if (BT_STATUS_SUCCESS == bt_status) {
                    s_ull_context.mic_vol = *mic_vol;
                }
            }
            ret = true;
            break;
        }
#if defined(AIR_AUDIO_DETACHABLE_MIC_ENABLE) && !defined(BT_ULL_EVENT_LE_HID_SERVICE_CONNECTED_IND)
        case APPS_EVENTS_INTERACTION_SWITCH_MIC: {
            APPS_LOG_MSGID_I(LOG_TAG": received mic type switch", 0);
            if (bt_ull_le_am_is_playing(BT_ULL_LE_AM_UL_MODE)) {
                bt_ull_streaming_t stream;
                stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
                stream.port = 0;
                bt_ull_le_srv_action(BT_ULL_ACTION_STOP_STREAMING, &stream, sizeof(stream));
                s_ull_context.is_resume_streaming = true;
                app_detachable_mic_switch();
            } else {
                app_detachable_mic_switch();
            }
            break;
        }
#endif
        case APPS_EVENTS_INTERACTION_ULL_APP_SWITCH_VER: {
            app_le_ull_update_ver();
            break;
        }
#if defined(MTK_AWS_MCE_ENABLE)
        case APPS_EVENTS_INTERACTION_APP_LE_ULL_CHECK_AND_SWITCH_UPLINK: {
            uint32_t val = (uint32_t)extra_data;
            if (val == 0) {
                app_ull_check_and_switch_uplink(true, false, false);
            } else if (val == 1) {
                app_ull_check_and_switch_uplink(false, false, true);
            } else if (val == 2) {
                app_ull_check_and_switch_uplink(false, true, false);
            }
            break;
        }
#endif
        default:
            break;
    }

    return ret;
}

static bool app_ull_proc_key_event_group(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len, bool from_aws) {
    /* UI shell internal event must process by this activity, so default is true. */
    bool ret = false;
    uint8_t key_id;
    airo_key_event_t key_event;

    app_event_key_event_decode(&key_id, &key_event, event_id);

    apps_config_key_action_t action;
    if (extra_data) {
        action = *(uint16_t *)extra_data;
    } else {
        action = apps_config_key_event_remapper_map_action(key_id, key_event);
    }

#ifdef MTK_AWS_MCE_ENABLE
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
    if (!from_aws &&
        (action == KEY_ULL_RECONNECT || action == KEY_ULL_SWITCH_LINK_MODE || action == KEY_ULL_SWITCH_GAME_MODE || action == KEY_ULL_AIR_PAIRING)) {
        apps_aws_sync_event_send(EVENT_GROUP_UI_SHELL_KEY, action);
    }
#else
    if (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_PARTNER) {
        switch (action) {
            case KEY_MUTE_MIC:
            case KEY_ULL_RECONNECT:
            case KEY_ULL_SWITCH_LINK_MODE:
            case KEY_ULL_SWITCH_GAME_MODE: {
                if (bt_aws_mce_srv_get_link_type() != BT_AWS_MCE_SRV_LINK_NONE) {
                    if (BT_STATUS_SUCCESS != apps_aws_sync_event_send(EVENT_GROUP_UI_SHELL_KEY, action)) {
                        voice_prompt_param_t vp = {0};
                        vp.vp_index = VP_INDEX_FAILED;
                        voice_prompt_play(&vp, NULL);
                        //apps_config_set_vp(VP_INDEX_FAILED, false, 0, VOICE_PROMPT_PRIO_MEDIUM, false, NULL);
                    }
                } else {
                    voice_prompt_param_t vp = {0};
                    vp.vp_index = VP_INDEX_FAILED;
                    voice_prompt_play(&vp, NULL);
                    //apps_config_set_vp(VP_INDEX_FAILED, false, 0, VOICE_PROMPT_PRIO_MEDIUM, false, NULL);
                    APPS_LOG_MSGID_I(LOG_TAG", Partner aws disconnected for key action: %x", 1, action);
                }
                ret = true;
                return ret;
                break;
            }
            default:
                break;
        }
    }
#endif
#endif

    switch (action) {
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
        case KEY_ULL_AIR_PAIRING: {
#ifdef AIR_BLE_ULL_PARING_MODE_ENABLE
            app_lea_adv_mgr_enable_ull2_pairing_mode(true);
#endif /* AIR_BLE_ULL_PARING_MODE_ENABLE */
#ifdef AIR_LEA_ULL2_KEY_TRIGGER_GENERAL_ADV
            app_le_ull_set_advertising_enable(true, true, true);
#else
            app_le_ull_set_advertising_enable(true, false, false);
#endif /* AIR_LEA_ULL2_KEY_TRIGGER_GENERAL_ADV */
        }
        break;
#endif /* AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE */
        case KEY_MUTE_MIC: {
            s_ull_context.mic_mute = !s_ull_context.mic_mute;
            bt_ull_streaming_t streaming;
            bt_ull_action_t action;
            if (s_ull_context.mic_mute) {
                action = BT_ULL_ACTION_SET_STREAMING_MUTE;
            } else {
                action = BT_ULL_ACTION_SET_STREAMING_UNMUTE;
            }
            streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
            streaming.port = 0;
            bt_ull_action(action, &streaming, sizeof(streaming));
            //ret = true;
            break;
        }
#ifndef AIR_ONLY_DONGLE_MODE_ENABLE
        case KEY_ULL_RECONNECT: {
#ifdef MTK_AWS_MCE_ENABLE
            if (bt_device_manager_aws_local_info_get_role() != BT_AWS_MCE_ROLE_CLINET)
#endif
            {
#if !defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE)
                app_ull_reconnect_request_process();
#endif
            }
            ret = true;
            break;
        }
        case KEY_ULL_SWITCH_LINK_MODE: {
            APPS_LOG_MSGID_I(LOG_TAG", link mode change, old:%d", 1, s_ull_context.link_mode);
#ifdef MTK_AWS_MCE_ENABLE
            if (bt_device_manager_aws_local_info_get_role() != BT_AWS_MCE_ROLE_CLINET)
#endif
            {
                app_ull_process_events(ULL_EVENTS_SWITCH_LINK_MODE, NULL);
                voice_prompt_param_t vp = {0};
                vp.vp_index = VP_INDEX_DOUBLE;
                voice_prompt_play(&vp, NULL);
                //apps_config_set_vp(VP_INDEX_DOUBLE, false, 0, VOICE_PROMPT_PRIO_MEDIUM, false, NULL);
            }
            break;
        }
#if (defined AIR_APP_ULL_GAMING_MODE_UI_ENABLE)
        case KEY_ULL_SWITCH_GAME_MODE : {
            APPS_LOG_MSGID_I(LOG_TAG", game mode change, old:%d", 1, s_ull_context.game_mode);
#ifdef MTK_AWS_MCE_ENABLE
            if (bt_device_manager_aws_local_info_get_role() != BT_AWS_MCE_ROLE_CLINET)
#endif
            {
                app_ull_process_events(ULL_EVENTS_SWITCH_GAME_MODE, NULL);
            }
            break;
        }
#endif
#endif
        default:
            break;
    }

    return ret;
}

#ifdef MTK_AWS_MCE_ENABLE
extern void bt_aws_mce_role_recovery_unlock(void);
#endif

static bool app_ull_proc_bt_cm_group(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len) {
    bool ret = false;
    app_ull_context_t *local_context = (app_ull_context_t *)self->local_context;
#ifdef MTK_AWS_MCE_ENABLE
    bt_status_t bt_status;
#endif
    if (NULL == local_context) {
        return ret;
    }
    switch (event_id) {
        case BT_CM_EVENT_POWER_STATE_UPDATE: {
            break;
        }
        case BT_CM_EVENT_REMOTE_INFO_UPDATE: {
            bt_cm_remote_info_update_ind_t *remote_update = (bt_cm_remote_info_update_ind_t *)extra_data;
            if (NULL == remote_update) {
                break;
            }
#ifdef MTK_AWS_MCE_ENABLE
            if (BT_AWS_MCE_ROLE_AGENT == bt_device_manager_aws_local_info_get_role() ||
                BT_AWS_MCE_ROLE_NONE == bt_device_manager_aws_local_info_get_role())
#endif
            {
                if (BT_CM_ACL_LINK_CONNECTED > remote_update->pre_acl_state && BT_CM_ACL_LINK_CONNECTED <= remote_update->acl_state) {
#ifdef MTK_AWS_MCE_ENABLE
                    if (!ULL_IS_ADDRESS_THE_SAME(remote_update->address, *(bt_device_manager_get_local_address()))) {
                        app_ull_process_events(ULL_EVENTS_SP_CONNECTED, &remote_update->address);
                    }
#else
                    app_ull_process_events(ULL_EVENTS_SP_CONNECTED, &remote_update->address);
#endif
                } else if ((BT_CM_ACL_LINK_CONNECTED <= remote_update->pre_acl_state && BT_CM_ACL_LINK_CONNECTED > remote_update->acl_state)
                           || (BT_CM_ACL_LINK_DISCONNECTING == remote_update->pre_acl_state && BT_CM_ACL_LINK_DISCONNECTED == remote_update->acl_state && BT_HCI_STATUS_UNKNOWN_CONNECTION_IDENTIFIER != remote_update->reason)) {
#ifdef MTK_AWS_MCE_ENABLE
                    if (!ULL_IS_ADDRESS_THE_SAME(remote_update->address, *(bt_device_manager_get_local_address()))) {
                        app_ull_process_events(ULL_EVENTS_SP_DISCONNECTED, &remote_update->address);
                    }
#else
                    app_ull_process_events(ULL_EVENTS_SP_DISCONNECTED, &remote_update->address);
#endif
                }
#ifdef MTK_AWS_MCE_ENABLE
                if ((~remote_update->pre_connected_service) & remote_update->connected_service
                    & BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS)) {
                    if (s_need_resync_ull_addr
                        && BT_AWS_MCE_ROLE_AGENT == bt_device_manager_aws_local_info_get_role()) {
                        bt_status = apps_aws_sync_event_send_extra(EVENT_GROUP_BT_ULTRA_LOW_LATENCY, BT_ULL_EVENT_PAIRING_COMPLETE_IND,
                                                                   s_ull_context.dongle_bt_address, sizeof(bt_bd_addr_t));
                        if (BT_STATUS_SUCCESS == bt_status) {
                            s_need_resync_ull_addr = false;
                            bt_aws_mce_role_recovery_unlock();
                        }
                    }
                    if (!s_ull_link_mode_synced
                        && BT_AWS_MCE_ROLE_AGENT == bt_device_manager_aws_local_info_get_role()) {
                        app_ull_nvdm_config_data_t config_data = {
                            .link_mode = s_ull_context.link_mode,
#if (defined AIR_APP_ULL_GAMING_MODE_UI_ENABLE)
                            .game_mode = s_ull_context.game_mode,
#endif
                        };
                        config_data.hfp_only_enable = s_hfp_only_enable;
                        config_data.a2dp_standby_enable = s_a2dp_standby_enable;
                        bt_status_t bt_status = apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_ULL_LINK_MODE_CHANGE,
                                                                               &(config_data), sizeof(config_data));
                        if (BT_STATUS_SUCCESS == bt_status) {
                            s_ull_link_mode_synced = true;
                        }
                        APPS_LOG_MSGID_I(LOG_TAG", Resync link mode(%d) game mode(%x) ret %x", 3, s_ull_context.link_mode,
#if (defined AIR_APP_ULL_GAMING_MODE_UI_ENABLE)
                                         s_ull_context.game_mode,
#else
                                         0,
#endif
                                         bt_status);
                    }
                    bool connected = s_ull_context.adv_paused;
                    if (connected) {
                        apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                                       APPS_EVENTS_INTERACTION_ULL_DONGLE_CONNECTION_CHANGE,
                                                       &connected, sizeof(connected));
                    }
                }
#endif
            }
#ifdef MTK_AWS_MCE_ENABLE
            else if (BT_AWS_MCE_ROLE_PARTNER == bt_device_manager_aws_local_info_get_role()) {
                if (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS)
                    & (remote_update->pre_connected_service & (~remote_update->connected_service))) {
#ifndef AIR_ONLY_DONGLE_MODE_ENABLE
                    if (s_ull_context.adv_paused) {
                        multi_ble_adv_manager_resume_ble_adv();
                        s_ull_context.adv_paused = false;
                    }
#endif
                    s_uplink_started = false;
                } else if (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS)
                           & ((~remote_update->pre_connected_service) & remote_update->connected_service)) {
                    if (BT_AWS_MCE_SRV_LINK_NORMAL == bt_aws_mce_srv_get_link_type()) {
                    }
                }
            }
#endif
            if ((~remote_update->pre_connected_service) & remote_update->connected_service
                & BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP)) {
                if (ULL_LINK_MODE_SINGLE == s_ull_context.link_mode && !s_ull_le_connected) {
                    app_ull_connect_correct_profile(&remote_update->address);
                }
            }
            break;
        }
        default:
            ret = false;
            break;
    }
    return ret;
}

static bool app_ull_proc_bt_dm_group(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len) {
    bool ret = false;
    bt_device_manager_power_event_t evt;
    bt_device_manager_power_status_t status;
    bt_event_get_bt_dm_event_and_status(event_id, &evt, &status);

    switch (evt) {
        case BT_DEVICE_MANAGER_POWER_EVT_PREPARE_ACTIVE:
            break;
        case BT_DEVICE_MANAGER_POWER_EVT_PREPARE_STANDBY:
            break;
        case BT_DEVICE_MANAGER_POWER_EVT_ACTIVE_COMPLETE:
#if defined(AIR_BLE_ULL_PARING_MODE_ENABLE) && defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
            if (BT_DEVICE_MANAGER_POWER_STATUS_SUCCESS == status) {
                bt_key_t sirk = {0};
                bt_key_t null_sirk = {0};

                app_le_ull_get_sirk(&sirk);
                if (app_lea_conn_mgr_get_bond_num() == 0 && 0 != memcmp(sirk, null_sirk, sizeof(bt_key_t))) {
                    APPS_LOG_MSGID_I(LOG_TAG", start general advertising if none bond info but valid sirk", 0);
                    app_le_ull_set_advertising_enable(true, true, false);
                }
            }
#endif
            break;
        case BT_DEVICE_MANAGER_POWER_EVT_CLASSIC_ACTIVE_COMPLETE: {
            if (BT_DEVICE_MANAGER_POWER_STATUS_SUCCESS == status) {
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
                if (bt_ull_le_hid_srv_get_bonded_device_num(BT_ULL_LE_HID_SRV_DEVICE_HEADSET) > 0) {
                    app_le_ull_set_advertising_enable(false, false, false);
                    bt_ull_le_hid_srv_create_sync_t con_info;
                    con_info.device_type = BT_ULL_LE_HID_SRV_DEVICE_HEADSET;
                    bt_ull_le_hid_srv_get_bonded_device_list(BT_ULL_LE_HID_SRV_DEVICE_HEADSET, 1, &con_info.peer_addr);
                    bt_ull_le_hid_srv_action(BT_ULL_ACTION_LE_HID_CREATE_SYNC, &con_info, sizeof(con_info));
                }
#endif
            }
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
            if (ULL_LINK_MODE_MULTIPLE == s_ull_context.link_mode) {
                extern bool app_ull_is_sp_connected();
                if (!app_ull_is_sp_connected()
#ifdef AIR_LE_AUDIO_ENABLE
                    && !app_ull_is_lea_connected()
#endif
                   ) {
                    bt_app_common_pre_set_ultra_low_latency_retry_count(BT_APP_COMMON_ULL_LATENCY_MODULE_RECONNECT, BT_APP_COMMON_ULL_STREAM_RETRY_COUNT_FOR_CONNECTING);
                }
            } else {
                bt_app_common_pre_set_ultra_low_latency_retry_count(BT_APP_COMMON_ULL_LATENCY_MODULE_RECONNECT, APPS_ULL_STREAMING_RETRY_COUNT_FOR_SINGLE_LINK);
            }
            bt_app_common_apply_ultra_low_latency_retry_count();
#endif
            break;
        }
        case BT_DEVICE_MANAGER_POWER_EVT_STANDBY_COMPLETE: {
            if (BT_DEVICE_MANAGER_POWER_STATUS_SUCCESS == status) {
#ifdef MTK_AWS_MCE_ENABLE
                bt_aws_mce_role_recovery_unlock();
#endif
            }
            break;
        }
        default:
            break;
    }
    return ret;
}

static bool app_ull_proc_ull_event(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len) {
    bool ret = false;
    voice_prompt_param_t vp = {0};

    switch (event_id) {
        case BT_ULL_EVENT_UPLINK_START_SUCCESS: {
            s_uplink_started = true;
            break;
        }
        case BT_ULL_EVENT_UPLINK_STOP_SUCCESS: {
            s_uplink_started = false;
            break;
        }
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
        case BT_ULL_EVENT_LE_CONNECTED: {
            app_lea_adv_mgr_enable_ull2_reconnect_mode(false);
            bt_ull_le_connected_info_t *con_info = (bt_ull_le_connected_info_t*)extra_data;
            if (con_info->status != BT_STATUS_SUCCESS) {
                APPS_LOG_MSGID_E(LOG_TAG", connected with error=0x%x", 1, con_info->status);
                break;
            }
#ifdef AIR_BLE_ULL_PARING_MODE_ENABLE
            app_lea_adv_mgr_enable_ull2_pairing_mode(false);
#endif
            ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_LE_ULL_CONNECT_TIMEOUT);
            app_ull_process_events(ULL_EVENTS_DONGLE_CONNECTED, NULL);
            vp.vp_index = VP_INDEX_SUCCEED;
            voice_prompt_play(&vp, NULL);
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_UPDATE_MMI_STATE, NULL, 0,
                                NULL, 0);
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN, NULL, 0, NULL, 0);
            break;
        }
        case BT_ULL_EVENT_LE_DISCONNECTED:
            app_ull_process_events(ULL_EVENTS_DONGLE_DISCONNECTED, NULL);
            vp.vp_index = VP_INDEX_FAILED;
            voice_prompt_play(&vp, NULL);
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_UPDATE_MMI_STATE, NULL, 0,
                                NULL, 0);
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN, NULL, 0, NULL, 0);
            break;
        case BT_ULL_EVENT_LE_STREAMING_START_IND:
        case BT_ULL_EVENT_LE_STREAMING_STOP_IND:
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_UPDATE_MMI_STATE, NULL, 0,
                                NULL, 0);
#if defined(AIR_AUDIO_DETACHABLE_MIC_ENABLE) && !defined(BT_ULL_EVENT_LE_HID_SERVICE_CONNECTED_IND)
            if (event_id == BT_ULL_EVENT_LE_STREAMING_STOP_IND) {
                bt_ull_le_streaming_start_ind_t *ull_event = (bt_ull_le_streaming_start_ind_t *)extra_data;
                if (ull_event != NULL
                    && ull_event->stream_mode == BT_ULL_LE_STREAM_MODE_UPLINK
                    && s_ull_context.is_resume_streaming) {
                    bt_ull_streaming_t stream;
                    stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
                    stream.port = 0;
                    bt_ull_le_srv_action(BT_ULL_ACTION_START_STREAMING, &stream, sizeof(stream));
                    s_ull_context.is_resume_streaming = false;
                }
            }
#endif
            break;
#endif
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
        case BT_ULL_EVENT_LE_HID_SERVICE_CONNECTED_IND:
            s_ull_le_hid_connected = true;
            break;
        case BT_ULL_EVENT_LE_HID_DISCONNECTED_IND: {
            s_ull_le_hid_connected = false;
            if (extra_data) {
                bt_ull_le_hid_srv_disconnected_ind_t *dis = (bt_ull_le_hid_srv_disconnected_ind_t*)extra_data;
                bt_ull_le_hid_srv_create_sync_t con_info;
                con_info.device_type = BT_ULL_LE_HID_SRV_DEVICE_HEADSET;
                memcpy(&con_info.peer_addr, &dis->peer_addr, sizeof(bt_addr_t));
                bt_ull_le_hid_srv_action(BT_ULL_ACTION_LE_HID_CREATE_SYNC, &con_info, sizeof(con_info));
                APPS_LOG_MSGID_I(LOG_TAG", BT_ULL_ACTION_LE_HID_CREATE_SYNC, addr: %2x-%2x..%2x", 3, dis->peer_addr.addr[0], \
                    dis->peer_addr.addr[1], dis->peer_addr.addr[5]);
            }
            break;
        }
#endif
        default:
            break;
    }

    return ret;
}

#ifdef MTK_AWS_MCE_ENABLE
static bool app_ull_proc_aws_data_event_proc(ui_shell_activity_t *self, uint32_t unused_id, void *aws_extra, size_t data_len) {
    bool ret = false;
    bt_aws_mce_report_info_t *aws_data_ind = (bt_aws_mce_report_info_t *)aws_extra;
    if (aws_data_ind) {
            switch (aws_data_ind->module_id) {
                case BT_AWS_MCE_REPORT_MODULE_BATTERY:
                    ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_APP_LE_ULL_CHECK_AND_SWITCH_UPLINK, (void *)1, 0,
                                NULL, 0);
                    break;
#ifdef AIR_SMART_CHARGER_ENABLE
                case BT_AWS_MCE_REPORT_MODULE_SMCHARGER:
                    ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_APP_LE_ULL_CHECK_AND_SWITCH_UPLINK, (void *)2, 0,
                                NULL, 0);
                    break;
#endif
            }
    }
    if (NULL == aws_data_ind || aws_data_ind->module_id != BT_AWS_MCE_REPORT_MODULE_APP_ACTION) {
        return ret;
    }
    uint32_t event_group;
    uint32_t event_id;
    uint8_t *extra_data;
    uint32_t extra_data_len;


    apps_aws_sync_event_decode_extra(aws_data_ind, &event_group, &event_id, (void *)&extra_data, &extra_data_len);
    switch (event_group) {
        case EVENT_GROUP_UI_SHELL_APP_INTERACTION:
            switch (event_id) {
                case APPS_EVENTS_INTERACTION_ULL_DONGLE_CONNECTION_CHANGE: {
                    if (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_PARTNER) {
                        bool connected = *(bool *)extra_data;
                        APPS_LOG_MSGID_I(LOG_TAG", receive dongle connection from agent, %d", 1, connected);
#ifndef AIR_ONLY_DONGLE_MODE_ENABLE
#ifndef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
                        if (connected && !s_ull_context.adv_paused) {
                            multi_ble_adv_manager_pause_ble_adv();
                            s_ull_context.adv_paused = true;
                        } else if (!connected && s_ull_context.adv_paused) {
                            multi_ble_adv_manager_resume_ble_adv();
                            s_ull_context.adv_paused = false;
                        }
#endif
#endif
                    }
                    break;
                }
                case APPS_EVENTS_INTERACTION_ULL_LINK_MODE_CHANGE: {
                    if (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_PARTNER) {
                        app_ull_nvdm_config_data_t *config_data = (app_ull_nvdm_config_data_t *)extra_data;
                        APPS_LOG_MSGID_I(LOG_TAG", receive ULL link mode from agent, %d, current : %d, game_mode %d->%d", 4, config_data->link_mode, s_ull_context.link_mode,
#if (defined AIR_APP_ULL_GAMING_MODE_UI_ENABLE)
                                         s_ull_context.game_mode, config_data->game_mode
#else
                                         0, 0
#endif
                                        );
                        if (config_data->link_mode != s_ull_context.link_mode
#if (defined AIR_APP_ULL_GAMING_MODE_UI_ENABLE)
                            || config_data->game_mode != s_ull_context.game_mode
#endif
                           ) {
                            s_ull_context.link_mode = config_data->link_mode;
#if (defined AIR_APP_ULL_GAMING_MODE_UI_ENABLE)
                            s_ull_context.game_mode = config_data->game_mode;
#endif
                            app_le_ull_storage_config();
                        }
                    }
                    break;
                }
                default:
                    break;
            }
            break;
        case EVENT_GROUP_BT_ULTRA_LOW_LATENCY:
            switch (event_id) {
                case BT_ULL_EVENT_PAIRING_COMPLETE_IND: {
                    APPS_LOG_MSGID_I(LOG_TAG", partner receive ull address size = %d", 1, extra_data_len);
                    if (extra_data && extra_data_len == sizeof(bt_bd_addr_t)) {
                        memcpy(s_ull_context.dongle_bt_address, extra_data, sizeof(bt_bd_addr_t));
                        nvkey_write_data(NVID_APP_ULL_PEER_BT_ADDRESS, extra_data, sizeof(bt_bd_addr_t));
                    }
                    ret = true;
                    break;
                }
                default:
                    break;
            }
            break;
        case EVENT_GROUP_UI_SHELL_KEY:
            switch (event_id) {
#ifndef AIR_ONLY_DONGLE_MODE_ENABLE
                case KEY_ULL_RECONNECT:
                case KEY_ULL_SWITCH_LINK_MODE:
                case KEY_ULL_SWITCH_GAME_MODE:
#endif
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
                case KEY_ULL_AIR_PAIRING:
#endif
                case KEY_MUTE_MIC: {
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
                    ret = app_ull_proc_key_event_group(self, 0, &event_id, 0, true);
#else
                    if (BT_AWS_MCE_ROLE_AGENT == bt_device_manager_aws_local_info_get_role()
                        || BT_AWS_MCE_ROLE_NONE == bt_device_manager_aws_local_info_get_role()) {
                        ret = app_ull_proc_key_event_group(self, 0, &event_id, 0, true);
                    }
#endif
                    break;
                }
                default:
                    break;
            }
            break;
        case APPS_EVENTS_INTERACTION_SWITCH_TO_ULL2_UPLINK:
            app_ull_switch_to_uplink();
            break;
        default:
            break;
    }

    return ret;
}
#endif

#if defined(MTK_FOTA_ENABLE) && defined (MTK_FOTA_VIA_RACE_CMD)
#include "bt_connection_manager_internal.h"
static bool app_ull_proc_fota_group(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len) {
    /* UI shell internal event must process by this activity, so default is true. */
    bool ret = false;

    switch (event_id) {
        case RACE_EVENT_TYPE_FOTA_START: {
            bt_cm_write_scan_mode(BT_CM_COMMON_TYPE_DISABLE, BT_CM_COMMON_TYPE_DISABLE);
            break;
        }
        case RACE_EVENT_TYPE_FOTA_CANCEL: {
            bt_cm_write_scan_mode(BT_CM_COMMON_TYPE_UNKNOW, BT_CM_COMMON_TYPE_ENABLE);
            break;
        }
        default:
            ret = false;
            break;
    }
    return ret;
}
#endif

bool app_ull_idle_activity_proc(
    struct _ui_shell_activity * self,
    uint32_t event_group,
    uint32_t event_id,
    void *extra_data,
    size_t data_len) {
    bool ret = false;

    switch (event_group) {
        case EVENT_GROUP_UI_SHELL_SYSTEM: {
            /* UI Shell internal events, please refer to doc/Airoha_IoT_SDK_UI_Framework_Developers_Guide.pdf. */
            ret = app_ull_proc_ui_shell_group(self, event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_APP_INTERACTION: {
            /* APP interaction events. */
            ret = app_ull_proc_interaction_event_group(self, event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_KEY: {
            /* key events. */
            ret = app_ull_proc_key_event_group(self, event_id, extra_data, data_len, false);
            break;
        }
#ifdef AIR_ROTARY_ENCODER_ENABLE
        case EVENT_GROUP_UI_SHELL_ROTARY_ENCODER: {
            /* Rotary encoder events. */
            ret = app_ull_proc_rotary_event_group(self, event_id, extra_data, data_len);
            break;
        }
#endif
        case EVENT_GROUP_UI_SHELL_BT_CONN_MANAGER: {
            ret = app_ull_proc_bt_cm_group(self, event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_BT_DEVICE_MANAGER: {
            ret = app_ull_proc_bt_dm_group(self, event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_BT_ULTRA_LOW_LATENCY: {
            ret = app_ull_proc_ull_event(self, event_id, extra_data, data_len);
            break;
        }
#ifdef MTK_AWS_MCE_ENABLE
        case EVENT_GROUP_UI_SHELL_AWS_DATA: {
            ret = app_ull_proc_aws_data_event_proc(self, event_id, extra_data, data_len);
            break;
        }
#endif
#if defined(MTK_FOTA_ENABLE) && defined (MTK_FOTA_VIA_RACE_CMD)
        case EVENT_GROUP_UI_SHELL_FOTA: {
            ret = app_ull_proc_fota_group(self, event_id, extra_data, data_len);
            break;
        }
#endif
#if defined(MTK_IN_EAR_FEATURE_ENABLE) && defined(MTK_AWS_MCE_ENABLE)
        case APPS_EVENTS_INTERACTION_IN_EAR_UPDATE_STA: {
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_APP_LE_ULL_CHECK_AND_SWITCH_UPLINK, (void *)0, 0,
                                NULL, 0);
            break;
        }
#endif
#ifdef MTK_AWS_MCE_ENABLE
        case EVENT_GROUP_UI_SHELL_BATTERY: {
            if (event_id == APPS_EVENTS_BATTERY_PERCENT_CHANGE) {
                ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_APP_LE_ULL_CHECK_AND_SWITCH_UPLINK, (void *)1, 0,
                                NULL, 0);
            }
            break;
        }
#endif
        default: {
            break;
        }
    }

    return ret;
}


bool bt_cm_check_connect_request(bt_bd_addr_ptr_t address, uint32_t cod) {
    return true;
}

bool app_ull_idle_activity_allow_a2dp_connect() {
    return s_a2dp_standby_enable;
}

uint32_t bt_cm_get_reconnect_profile(bt_bd_addr_t *addr) {
    uint32_t profiles = BT_CM_PROFILE_SERVICE_MASK_NONE;
    if (!s_ull_le_connected) {
        profiles = bt_customer_config_app_get_cm_config()->power_on_reconnect_profile & (~BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS));
    } else if (ULL_LINK_MODE_MULTIPLE == s_ull_context.link_mode) {
        profiles = BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP);
        if (s_a2dp_standby_enable) {
            profiles |= BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK);
        }
    }
    return profiles;
}

const app_ull_context_t *app_ull_idle_activity_get_current_context(void) {
    return &s_ull_context;
}

uint8_t app_ull_get_mic_vol(void) {
    return s_ull_context.mic_vol;
}
uint8_t app_ull_get_mix_ratio(void) {
    return s_ull_context.current_mix_ratio_level;
}

bool app_ull_is_uplink_open() {
    return s_uplink_started;
}

bool app_ull_is_multi_link_mode() {
    return (s_ull_context.link_mode == ULL_LINK_MODE_MULTIPLE);
}

bool app_ull_is_streaming() {
    if (bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK)) {
        return true;
    }

    if (bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_UPLINK)) {
        return true;
    }

    return false;
}

bool app_ull_is_ul_and_dl_streaming() {
    if (!bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK)) {
        return false;
    }

    if (!bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_UPLINK)) {
        return false;
    }

    return true;
}

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
bool app_ull_is_le_hid_connected() {
    return s_ull_le_hid_connected;
}
#endif

