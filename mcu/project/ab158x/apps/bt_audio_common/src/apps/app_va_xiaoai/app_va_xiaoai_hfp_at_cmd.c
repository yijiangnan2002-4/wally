
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
 * File: app_va_xiaoai_hfp_at_cmd.c
 *
 * Description: This file provides XiaoAI HFP AT CMD for XiaoAI activity.
 *
 */

#ifdef AIR_XIAOAI_ENABLE

#include "app_va_xiaoai_hfp_at_cmd.h"

#include "apps_aws_sync_event.h"
#include "apps_config_key_remapper.h"
#include "apps_debug.h"
#include "apps_events_event_group.h"
#include "bt_app_common.h"
#include "bt_callback_manager.h"
#include "bt_device_manager.h"
#include "bt_sink_srv.h"
#include "bt_sink_srv_hf.h"
#ifdef MTK_AWS_MCE_ENABLE
#include "bt_aws_mce_srv.h"
#endif
#include "FreeRTOS.h"
#include "multi_va_manager.h"

#include "xiaoai.h"
#include "app_va_xiaoai_ble_adv.h"
#include "app_va_xiaoai_config.h"
#include "app_va_xiaoai_device_config.h"
#ifdef AIR_XIAOAI_MIUI_FAST_CONNECT_ENABLE
#include "app_va_xiaoai_miui_fast_connect.h"
#endif

#ifdef AIR_LE_AUDIO_ENABLE
#include "app_le_audio.h"
#endif
#ifdef AIR_MULTI_POINT_ENABLE
#include "app_bt_emp_service.h"
#endif

#ifndef PACKED
#define PACKED  __attribute__((packed))
#endif

extern bool app_xiaoai_is_lea_mma_link(void);

#define LOG_TAG                       "[XIAOAI_HF]"

#define APP_XIAOMI_HFP_ATCMD                        "+XIAOMI: "
#define APP_XIAOMI_HFP_ATCMD_LEN                    (strlen(APP_XIAOMI_HFP_ATCMD))
#define APP_XIAOMI_HFP_HEADER_LEN                   10 // <FF><01><02><01><01/03>

#define XIAOAI_HFP_MORE_ATCMD_MAX_LEN               200

#define XIAOAI_MMA_NOTIFY_DEVICE_INFO_OPCODE        0xF4

#define XIAOAI_MMA_MAX_PACKAGE_NAME                 253
// ZMI Customer
#define APP_XIAOMI_HFP_YS_PKG_NAME                  "com.android.yuanshen1"
#define APP_XIAOMI_HFP_YUANSHEN_PKG_NAME            "com.android.yuanshen2"

static uint32_t                                     g_xiaoai_hfp_handle = 0;

static char                                         g_xiaoai_pkg_name[XIAOAI_MMA_MAX_PACKAGE_NAME] = {0};

typedef enum {
    XIAOAI_HFP_ATCMD_ATT_STATUS = 1,
    XIAOAI_HFP_ATCMD_ATT_MIUI_FAST_CONNECT = 2,
    XIAOAI_HFP_ATCMD_ATT_CONFIG = 3
} xiaoai_hfp_miui_attribute;

typedef enum {
    XIAOAI_HFP_ATCMD_TYPE_REPORT = 0x00,
    XIAOAI_HFP_ATCMD_TYPE_RUN_INFO_CHANGED = 0x03,

    XIAOAI_HFP_ATCMD_TYPE_FLAG = 0x01,
    XIAOAI_HFP_ATCMD_TYPE_FEATURE = 0x20,
    XIAOAI_HFP_ATCMD_TYPE_ANC = 0x04,
    XIAOAI_HFP_ATCMD_TYPE_KEY = 0x05,
    XIAOAI_HFP_ATCMD_TYPE_VOICE = 0x06,
    XIAOAI_HFP_ATCMD_TYPE_EQ = 0x07,
    XIAOAI_HFP_ATCMD_TYPE_GAME_MODE = 0x08,
    XIAOAI_HFP_ATCMD_TYPE_ANTI_LOST = 0x0A
} xiaoai_hfp_atcmd_status_type;

typedef enum {
    XIAOAI_HFP_ATCMD_TYPE_CONFIG_NAME = 0x01,
    XIAOAI_HFP_ATCMD_TYPE_CONFIG_ACCOUNT_KEY = 0x03,
    XIAOAI_HFP_ATCMD_TYPE_CONFIG_LEAUDIO = 0x04,

    XIAOAI_HFP_ATCMD_TYPE_CONFIG_PACKAGE_NAME = 0x08
} xiaoai_hfp_atcmd_config_type;

typedef enum {
    XIAOAI_KEY_ANC = 0,
    XIAOAI_KEY_VA,
    XIAOAI_KEY_GAME_MODE,
    XIAOAI_KEY_PREV_MUSIC,
    XIAOAI_KEY_NEXT_MUSIC,
    XIAOAI_KEY_VOL_UP,
    XIAOAI_KEY_VOL_DOWN,
    XIAOAI_KEY_PALY_PAUSE
} xiaoai_key_function;

static xiaoai_key_function app_va_xiaoai_key_click_function(bool left_earbud, airo_key_event_t key_event)
{
    xiaoai_key_function key_function = XIAOAI_KEY_ANC;
    apps_config_state_t mmi_state = apps_config_key_get_mmi_state();
    apps_config_key_action_t key_action = apps_config_key_get_mapper_action(
                                              DEVICE_KEY_POWER, key_event, mmi_state, left_earbud);
    if (key_action == KEY_VA_XIAOAI_START || key_action == KEY_VA_XIAOAI_START_NOTIFY
        || (key_action >= KEY_WAKE_UP_VOICE_ASSISTANT && key_action <= KEY_WAKE_UP_VOICE_ASSISTANT_NOTIFY)) {
        key_function = XIAOAI_KEY_VA;
    } else if (key_action == KEY_AVRCP_PLAY || key_action == KEY_AVRCP_PAUSE) {
        key_function = XIAOAI_KEY_PALY_PAUSE;
    } else if (key_action == KEY_AVRCP_BACKWARD) {
        key_function = XIAOAI_KEY_PREV_MUSIC;
    } else if (key_action == KEY_AVRCP_FORWARD) {
        key_function = XIAOAI_KEY_NEXT_MUSIC;
    } else if (key_action == KEY_VOICE_UP) {
        key_function = XIAOAI_KEY_VOL_UP;
    } else if (key_action == KEY_VOICE_DN) {
        key_function = XIAOAI_KEY_VOL_DOWN;
    } else if (key_action == KEY_PASS_THROUGH || key_action == KEY_ANC
               || key_action == KEY_SWITCH_ANC_AND_PASSTHROUGH) {
        key_function = XIAOAI_KEY_ANC;
    } else if (key_action == KEY_GAMEMODE_ON || key_action == KEY_GAMEMODE_OFF
               || key_action == KEY_GAMEMODE_TOGGLE) {
        key_function = XIAOAI_KEY_GAME_MODE;
    }
    APPS_LOG_MSGID_I(LOG_TAG" double_click left=%d %d->%d",
                     3, left_earbud, key_action, key_function);
    return key_function;
}

typedef enum {
    XIAOAI_KEY_ANC_OFF_ON_PT = 0,  // OFF, ANC_ON, PT_ON
    XIAOAI_KEY_ANC_ON_PT,          // ANC_ON, PT_ON
    XIAOAI_KEY_ANC_OFF_ON,         // OFF, ANC_ON
    XIAOAI_KEY_ANC_OFF_PT,         // OFF, PT_ON
    XIAOAI_KEY_ANC_OFF             // OFF
} xiaoai_key_anc_function;

// Customer configure option: need to implement ANC switch Key
static xiaoai_key_anc_function app_va_xiaoai_key_anc_function(bool left_earbud)
{
    xiaoai_key_anc_function key_anc_function = 0;
    return key_anc_function;
}

static bool app_va_xiaoai_send_hfp_atcmd(uint16_t mma_type, uint8_t miui_attribute, uint8_t *data, uint8_t data_len)
{
    bool ret = FALSE;
    char *atcmd = NULL;
    char *param = NULL;
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    APPS_LOG_MSGID_I(LOG_TAG" send_hfp_atcmd, [%02X] mma_type=%d miui_attribute=%d data=0x%08X data_len=%d",
                     5, role, mma_type, miui_attribute, data, data_len);

    if ((miui_attribute != XIAOAI_HFP_ATCMD_ATT_STATUS
         && miui_attribute != XIAOAI_HFP_ATCMD_ATT_MIUI_FAST_CONNECT
         && miui_attribute != XIAOAI_HFP_ATCMD_ATT_CONFIG)
        || data == NULL || data_len == 0) {
        APPS_LOG_MSGID_E(LOG_TAG" send_hfp_atcmd, parameter fail", 0);
        goto exit;
    }

    atcmd = (char *)pvPortMalloc(XIAOAI_HFP_MORE_ATCMD_MAX_LEN);
    param = (char *)pvPortMalloc(data_len * 2 + 1);
    if (atcmd == NULL || param == NULL) {
        APPS_LOG_MSGID_E(LOG_TAG" send_hfp_atcmd, malloc fail", 0);
        goto exit;
    }
    memset(atcmd, 0, XIAOAI_HFP_MORE_ATCMD_MAX_LEN);
    memset(param, 0, data_len * 2 + 1);

    int atcmd_len = 0;
    xiaoai_conn_info_t conn_info = xiaoai_get_connection_info();
    bool lea_mma = app_xiaoai_is_lea_mma_link();
    bool peer_lea_mma = xiaoai_peer_is_lea_mma_link();
    if (lea_mma || (conn_info.conn_state != XIAOAI_STATE_CONNECTED && peer_lea_mma)) {
        atcmd[0] = 0xFF;
        atcmd[1] = 0x01;
        atcmd[2] = 0x02;
        atcmd[3] = 0x01;
        atcmd[4] = miui_attribute;
        memcpy(atcmd + 5, data, data_len);
        *(atcmd + 5 + data_len) = 0xFF;
        atcmd_len = 5 + data_len + 1;
    } else {
        // Special case for name change
        if (mma_type == XIAOAI_HFP_ATCMD_ATT_CONFIG
            && data[1] == XIAOAI_HFP_ATCMD_TYPE_CONFIG_NAME) {
            snprintf(atcmd, XIAOAI_HFP_MORE_ATCMD_MAX_LEN, "FF010201%02X%02X%02X", miui_attribute, data[0], data[1]);
            memcpy(atcmd + strlen("FF010201FFFFFF"), data + 2, data_len - 2);
            char *last = atcmd + strlen("FF010201FFFFFF") + data_len - 2;
            last[0] = 'F';
            last[1] = 'F';
            atcmd_len = strlen("FF010201FFFFFF") + data_len - 2 + 2;
            APPS_LOG_I("[XIAOAI_HF] HFP ATCMD=%s atcmd_len=%d\r\n", (char *)atcmd, atcmd_len);
        } else {
            for (int i = 0; i < data_len; i++) {
                char temp[3] = {0};
                snprintf(temp, 3, "%02X", data[i]);
                param[i * 2] = temp[0];
                param[i * 2 + 1] = temp[1];
            }

            // AT+XIAOMI=<FF> <01><02><01> <01/03> <length><type><data(LTV List)>... <FF>
            snprintf(atcmd, XIAOAI_HFP_MORE_ATCMD_MAX_LEN, "FF010201%02X%sFF", miui_attribute, param);

            atcmd_len = strlen(atcmd);
            APPS_LOG_I("[XIAOAI_HF] HFP ATCMD=%s atcmd_len=%d\r\n", (char *)atcmd, atcmd_len);
        }
    }

    if (atcmd != NULL && atcmd_len > 0) {
        ret = app_va_xiaoai_send_atcmd(mma_type, atcmd, atcmd_len);
    }

exit:
    if (atcmd != NULL) {
        vPortFree(atcmd);
    }
    if (param != NULL) {
        vPortFree(param);
    }
    return ret;
}

static void app_va_xiaoai_handle_get_status(const char *cmd)
{
    int len = 0;
    int type = 0;
    int flag = 0; // 0 - use current value, 1 - re-detect and use latest value
    int n = sscanf(cmd, "%02x%02x%02x", &len, &type, &flag);
    APPS_LOG_MSGID_I(LOG_TAG" handle_get_status, n=%d len=%d type=%d flag=%d",
                     4, n, len, type, flag);
    if (n == 3 && len == 2) {
        switch (type) {
            case 0: {
                // all
                app_va_xiaoai_hfp_miui_more_atcmd_report_all_status();
                break;
            }
            case XIAOAI_HFP_ATCMD_TYPE_FLAG: {
                app_va_xiaoai_hfp_miui_more_atcmd_report_feature();
                break;
            }
            case XIAOAI_HFP_ATCMD_TYPE_ANC: {
                app_va_xiaoai_hfp_miui_more_atcmd_report_anc();
                break;
            }
            case XIAOAI_HFP_ATCMD_TYPE_KEY: {
                app_va_xiaoai_hfp_miui_more_atcmd_report_key();
                break;
            }
            case XIAOAI_HFP_ATCMD_TYPE_VOICE: {
                app_va_xiaoai_hfp_miui_more_atcmd_report_voice();
                break;
            }
            case XIAOAI_HFP_ATCMD_TYPE_EQ: {
                app_va_xiaoai_hfp_miui_more_atcmd_report_eq();
                break;
            }
            case XIAOAI_HFP_ATCMD_TYPE_GAME_MODE: {
                app_va_xiaoai_hfp_miui_more_atcmd_report_game_mode();
                break;
            }
            case XIAOAI_HFP_ATCMD_TYPE_ANTI_LOST: {
                uint8_t anti_lost_state = app_va_xiaoai_get_anti_lost_state();
                app_va_xiaoai_hfp_miui_more_atcmd_report_anti_lost(anti_lost_state);
                break;
            }
            default: {
                APPS_LOG_MSGID_E(LOG_TAG" handle_get_status error", 0);
                break;
            }
        }
    }
}

static void app_va_xiaoai_handle_config_request(const char *cmd)
{
    int len = 0;
    int type = 0;
    int n = sscanf(cmd, "%02x%02x", &len, &type);
    APPS_LOG_MSGID_I(LOG_TAG" handle_config_request len=%d type=%d",
                     2, len, type);

    if (n == 2 && len >= 2) {
        if (type == XIAOAI_HFP_ATCMD_TYPE_CONFIG_NAME) {
            const char *name = cmd + 4;
            len = len - 1;  // <len><type><name_string>, <type> only 1 bytes
            // <len><type>=2 + 1 '\0'
            uint8_t *report_data = (uint8_t *)pvPortMalloc(2 + XIAOAI_BT_NAME_LEN + 1);
            if (report_data != NULL) {
                memset(report_data, 0, 2 + XIAOAI_BT_NAME_LEN + 1);
                uint8_t copy_len = (len >= XIAOAI_BT_NAME_LEN ? XIAOAI_BT_NAME_LEN : len);
                report_data[0] = copy_len + 1;
                report_data[1] = XIAOAI_HFP_ATCMD_TYPE_CONFIG_NAME;
                memcpy(report_data + 2, name, copy_len);

#if 1
                bool ret = app_va_xiaoai_own_set_device_name(report_data + 2, copy_len);
#else
                // ZMI Customer
                // we shall write nvram both agent and partner, AT command is processed by agent
                //2021.4.1 use interfice provided by Airoha
                const char *device_name = report_data + 2;
                app_set_user_defined_edr_name(device_name);
#endif
                if (ret) {
                    ret = app_va_xiaoai_send_hfp_atcmd(XIAOAI_MMA_TYPE_HEADSET_NAME_MODIFY_RESULT,
                                                       XIAOAI_HFP_ATCMD_ATT_CONFIG,
                                                       report_data, copy_len + 2);
                    APPS_LOG_MSGID_I(LOG_TAG" handle_set_name, send rsp ret=%d", 1, ret);
                }
                vPortFree(report_data);
            } else {
                APPS_LOG_MSGID_I(LOG_TAG" handle_set_name, malloc fail", 0);
            }
        } else if (type == XIAOAI_HFP_ATCMD_TYPE_CONFIG_LEAUDIO) {
            int le_audio_switch = 0;
            sscanf(cmd + 4, "%02x", &le_audio_switch);
            APPS_LOG_MSGID_I(LOG_TAG"[APP_CONN] handle_enable_leaudio, %d",
                             1, le_audio_switch);
//            bool enable = (le_audio_switch == 1);
//            uint8_t addr[BT_BD_ADDR_LEN] = {0};
//            if (enable) {
//                bt_bd_addr_t *bt_bd_addr = bt_hfp_get_bd_addr_by_handle(g_xiaoai_hfp_handle);
//                if (bt_bd_addr != NULL) {
//                    memcpy(addr, (const uint8_t *)*bt_bd_addr, BT_BD_ADDR_LEN);
//                }
//            }

            uint8_t report_data[3] = {0x02, XIAOAI_HFP_ATCMD_TYPE_CONFIG_LEAUDIO, le_audio_switch};
            bool ret = app_va_xiaoai_send_hfp_atcmd(XIAOAI_MMA_TYPE_HEADSET_LE_AUDIO_STATUS,
                                                    XIAOAI_HFP_ATCMD_ATT_CONFIG,
                                                    report_data, 3);
            APPS_LOG_MSGID_I(LOG_TAG" handle_enable_leaudio, send rsp ret=%d", 1, ret);
        }
#ifdef AIR_XIAOAI_MIUI_FAST_CONNECT_ENABLE
        else if (type == XIAOAI_HFP_ATCMD_TYPE_CONFIG_ACCOUNT_KEY) {
            if (len == MIUI_FC_ACCOUNT_KEY_LEN + 1) {
                uint8_t key[MIUI_FC_ACCOUNT_KEY_LEN] = {0};
                for (int i = 0; i < MIUI_FC_ACCOUNT_KEY_LEN; i++) {
                    int temp_key = 0;
                    sscanf(cmd + 4 + (i * 2), "%02X", &temp_key);
                    key[i] = (uint8_t)temp_key;
                }
                miui_fast_connect_set_account_key(key);
                uint8_t *report_data = (uint8_t *)pvPortMalloc(MIUI_FC_ACCOUNT_KEY_LEN + 2);
                if (report_data != NULL) {
                    report_data[0] = MIUI_FC_ACCOUNT_KEY_LEN + 1;
                    report_data[1] = XIAOAI_HFP_ATCMD_TYPE_CONFIG_ACCOUNT_KEY;
                    memcpy(&report_data[2], key, MIUI_FC_ACCOUNT_KEY_LEN);
                    bool ret = app_va_xiaoai_send_hfp_atcmd(XIAOAI_MMA_TYPE_HEADSET_ACCOUNT_KEY_MODIFY_RESULT,
                                                            XIAOAI_HFP_ATCMD_ATT_CONFIG,
                                                            report_data,
                                                            MIUI_FC_ACCOUNT_KEY_LEN + 2);
                    APPS_LOG_MSGID_I(LOG_TAG" handle_config_account_key, send rsp ret=%d", 1, ret);
                    vPortFree(report_data);
                } else {
                    APPS_LOG_MSGID_E(LOG_TAG" handle_config_account_key, malloc fail", 0);
                }
            } else {
                APPS_LOG_MSGID_E(LOG_TAG" handle_config_account_key, error len=%d", 1, len);
            }
        }
#endif
        else if (type == XIAOAI_HFP_ATCMD_TYPE_CONFIG_PACKAGE_NAME) {
            // ZMI Customer
            len = len - 1;
            const char *name = cmd + 4;
            memset(g_xiaoai_pkg_name, 0, XIAOAI_MMA_MAX_PACKAGE_NAME);
            uint8_t copy_len = (len >= XIAOAI_MMA_MAX_PACKAGE_NAME ? (XIAOAI_MMA_MAX_PACKAGE_NAME - 1) : len);
            memcpy(g_xiaoai_pkg_name, name, copy_len);
            APPS_LOG_I(LOG_TAG" handle_config_package_name, name=%d len=%d",
                       g_xiaoai_pkg_name, len);
        }

    }
}

#if 1

#define MIUI_FAST_CONNECT_BLE_ADV_LEN           31
#define MIUI_FAST_CONNECT_REQUEST_ADV_DATA      1
#define MIUI_FAST_CONNECT_REQUEST_SCAN_RSP      2

static void app_va_xiaoai_reply_ble_adv_cmd(bool is_adv_data, uint8_t *buf, uint32_t buf_len)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bool lea_mma = app_xiaoai_is_lea_mma_link();
    bool peer_lea_mma = xiaoai_peer_is_lea_mma_link();
    uint32_t hfp_num = bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP), NULL, 0);
    if (role == BT_AWS_MCE_ROLE_PARTNER || lea_mma || peer_lea_mma || hfp_num == 0) {
        APPS_LOG_MSGID_E(LOG_TAG" reply_ble_adv_cmd, [%02X] fail lea_mma=%d peer_lea_mma=%d hfp_num=%d",
                         4, role, lea_mma, peer_lea_mma, hfp_num);
        return;
    }

    uint8_t *report_data = (uint8_t *)pvPortMalloc(buf_len + 2);
    if (report_data != NULL) {
        report_data[0] = buf_len + 1;
        report_data[1] = (is_adv_data ? MIUI_FAST_CONNECT_REQUEST_ADV_DATA : MIUI_FAST_CONNECT_REQUEST_SCAN_RSP);
        memcpy(&report_data[2], buf, buf_len);
        bool ret = app_va_xiaoai_send_hfp_atcmd((is_adv_data ? XIAOAI_MMA_TYPE_BLE_ADV_DATA : XIAOAI_MMA_TYPE_BLE_ADV_SCAN_RSP),
                                                XIAOAI_HFP_ATCMD_ATT_MIUI_FAST_CONNECT,
                                                report_data,
                                                buf_len + 2);
        APPS_LOG_MSGID_I(LOG_TAG" reply_ble_adv_cmd, type=%d buf_len=%d ret=%d",
                         3, report_data[1], buf_len, ret);
        vPortFree(report_data);
    } else {
        APPS_LOG_MSGID_E(LOG_TAG" reply_ble_adv_cmd, malloc fail", 0);
    }
}

static void app_va_xiaoai_reply_ble_adv_data_cmd()
{
    uint8_t adv_data[MIUI_FAST_CONNECT_BLE_ADV_LEN] = {0};
#ifdef AIR_XIAOAI_MIUI_FAST_CONNECT_ENABLE
    miui_fast_connect_config_ble_adv(adv_data, NULL);
#else
    bt_hci_cmd_le_set_advertising_parameters_t le_adv_param = {0};
    bt_hci_cmd_le_set_advertising_data_t le_adv_data = {0};
    bt_hci_cmd_le_set_scan_response_data_t le_scan_rsp = {0};
    xiaoai_set_ble_adv_info(62, &le_adv_param, &le_adv_data, &le_scan_rsp);
    memcpy(adv_data, le_adv_data.advertising_data, le_adv_data.advertising_data_length);
#endif
    app_va_xiaoai_reply_ble_adv_cmd(TRUE, adv_data, MIUI_FAST_CONNECT_BLE_ADV_LEN);
}

static void app_va_xiaoai_reply_ble_scan_rsp_cmd()
{
    uint8_t scan_rsp[MIUI_FAST_CONNECT_BLE_ADV_LEN] = {0};
#ifdef AIR_XIAOAI_MIUI_FAST_CONNECT_ENABLE
    miui_fast_connect_config_ble_adv(NULL, scan_rsp);
#else
    bt_hci_cmd_le_set_advertising_parameters_t le_adv_param = {0};
    bt_hci_cmd_le_set_advertising_data_t le_adv_data = {0};
    bt_hci_cmd_le_set_scan_response_data_t le_scan_rsp = {0};
    xiaoai_set_ble_adv_info(62, &le_adv_param, &le_adv_data, &le_scan_rsp);
    memcpy(scan_rsp, le_scan_rsp.scan_response_data, le_scan_rsp.scan_response_data_length);
#endif
    app_va_xiaoai_reply_ble_adv_cmd(FALSE, scan_rsp, MIUI_FAST_CONNECT_BLE_ADV_LEN);
}

static void app_va_xiaoai_handle_miui_fast_connect_request(const char *cmd)
{
    int len = 0;
    int type = 0;
    int n = sscanf(cmd, "%02x%02x", &len, &type);
    APPS_LOG_MSGID_I(LOG_TAG" handle_miui_fast_connect_request len=%d type=%d",
                     2, len, type);
    if (n == 2 && len == 1) {
        if (type == MIUI_FAST_CONNECT_REQUEST_ADV_DATA) {
            app_va_xiaoai_reply_ble_adv_data_cmd();
        } else if (type == MIUI_FAST_CONNECT_REQUEST_SCAN_RSP) {
            app_va_xiaoai_reply_ble_scan_rsp_cmd();
        }
    }
}

#endif

static void app_va_xiaoai_atcmd_parse(const char *atcmd)
{
    int atcmd_len = strlen(atcmd) - APP_XIAOMI_HFP_ATCMD_LEN - APP_XIAOMI_HFP_HEADER_LEN;
    if (atcmd_len > 0
        && strstr(atcmd, APP_XIAOMI_HFP_ATCMD) > 0) {
        atcmd += APP_XIAOMI_HFP_ATCMD_LEN + APP_XIAOMI_HFP_HEADER_LEN - 2;
        int type = 0;
        sscanf(atcmd, "%02x", &type);
        APPS_LOG_MSGID_I(LOG_TAG" atcmd_parse type=%d", 1, type);
        if (type == XIAOAI_HFP_ATCMD_ATT_STATUS) {
            app_va_xiaoai_handle_get_status(atcmd + 2);
        } else if (type == XIAOAI_HFP_ATCMD_ATT_CONFIG) {
            app_va_xiaoai_handle_config_request(atcmd + 2);
        } else if (type == XIAOAI_HFP_ATCMD_ATT_MIUI_FAST_CONNECT) {
            app_va_xiaoai_handle_miui_fast_connect_request(atcmd + 2);
        }
    }
}

bt_status_t app_va_xiaoai_hfp_callback(bt_msg_type_t event, bt_status_t status, void *param)
{
    switch (event) {
        case BT_HFP_CUSTOM_COMMAND_RESULT_IND: {
            bt_hfp_custom_command_result_ind_t *ind = (bt_hfp_custom_command_result_ind_t *)param;
            const char *atcmd = ind->result;
            // +XIAOMI: <FF><01><02><01><01/03>"data"<FF>
            if (status == BT_STATUS_SUCCESS && atcmd != NULL) {
                g_xiaoai_hfp_handle = ind->handle;
                app_va_xiaoai_atcmd_parse(atcmd);
            } else {
                APPS_LOG_MSGID_I(LOG_TAG" hfp_callback error, status=0x%08X atcmd=%d",
                                 2, status, atcmd);
            }
            break;
        }
    }
    return BT_STATUS_SUCCESS;
}

bool app_va_xiaoai_send_atcmd(uint16_t mma_type, const char *atcmd, uint32_t atcmd_len)
{
    bool success = FALSE;
    bool lea_aws_success = FALSE;
    bool lea_success = FALSE;
    bool hfp_success = FALSE;
    xiaoai_conn_info_t conn_info = xiaoai_get_connection_info();
    bool lea_mma = app_xiaoai_is_lea_mma_link();
    bool peer_lea_mma = xiaoai_peer_is_lea_mma_link();
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    uint32_t hfp_num = bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP), NULL, 0);

    if (conn_info.conn_state != XIAOAI_STATE_CONNECTED && peer_lea_mma) {
        int total_len = sizeof(xiaoai_at_cmd_param_t) - 1 + atcmd_len;
        uint8_t *data = (uint8_t *)pvPortMalloc(total_len);
        if (data == NULL) {
            APPS_LOG_MSGID_E(LOG_TAG" [LEA_MMA_LINK] send_atcmd, malloc fail", 0);
            return FALSE;
        }
        memset(data, 0, total_len);
        xiaoai_at_cmd_param_t *param = (xiaoai_at_cmd_param_t *)data;
        param->mma_type = mma_type;
        param->atcmd_len = atcmd_len;
        memcpy(&param->atcmd[0], atcmd, atcmd_len);
        bt_status_t status = apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_XIAOAI,
                                                            XIAOAI_EVENT_LEA_SYNC_AT_CMD_TO_PEER,
                                                            (uint8_t *)param,
                                                            total_len);
        vPortFree(param);
        APPS_LOG_MSGID_I(LOG_TAG" [LEA_MMA_LINK] send AT CMD via peer_lea_mma, mma_type=0x%04X status=0x%08X",
                         2, mma_type, status);
        lea_aws_success = (status == BT_STATUS_SUCCESS);
    }

    if (!lea_mma && role != BT_AWS_MCE_ROLE_AGENT && hfp_num == 0) {
        APPS_LOG_MSGID_E(LOG_TAG" send AT CMD, ignore [%02X] conn_info.conn_state=%d lea_mma=%d hfp_num=%d",
                         4, role, conn_info.conn_state, lea_mma, hfp_num);
        goto exit;
    }

    // ZMI Customer, mark for MMA run on LE Audio Link
    if (conn_info.conn_state == XIAOAI_STATE_CONNECTED
        && conn_info.is_le_audio_mma) {
        uint32_t mma_data_len = sizeof(uint8_t) + sizeof(uint16_t) + atcmd_len;
        uint8_t *buf = (uint8_t *)pvPortMalloc(mma_data_len);
        if (buf != NULL) {
            memset(buf, 0, mma_data_len);
            buf[0] = sizeof(uint16_t) + atcmd_len;
            buf[1] = (uint8_t)(mma_type >> 8);
            buf[2] = (uint8_t)(mma_type & 0xFF);
            memcpy(buf + 3, atcmd, atcmd_len);

            lea_success = xiaoai_send_mma_cmd(XIAOAI_MMA_NOTIFY_DEVICE_INFO_OPCODE, buf, mma_data_len);
            vPortFree(buf);
        } else {
            APPS_LOG_MSGID_E(LOG_TAG" send_atcmd, malloc fail", 0);
            lea_success = FALSE;
        }
    }

    if (role == BT_AWS_MCE_ROLE_AGENT && hfp_num > 0) {
        // for MMA_LEA, HFP AT CMD will use HEX not string, covert to original HFP AT CMD
        bt_status_t bt_status = BT_STATUS_SUCCESS;
        if (lea_aws_success || lea_success) {
            char atcmd_str[XIAOAI_HFP_MORE_ATCMD_MAX_LEN] = {0};
            uint16_t atcmd_str_len = 0;
            if (mma_type == XIAOAI_MMA_TYPE_PHONE_MODIFY_HEADSET_NAME) {
                // Special case for name change, <FF><01><02><01><03> <len><01>
                for (int i = 0; i < 5 + 2; i++) {
                    char temp[3] = {0};
                    snprintf(temp, 3, "%02X", atcmd[i]);
                    atcmd_str[i * 2] = temp[0];
                    atcmd_str[i * 2 + 1] = temp[1];
                }
                memcpy(atcmd_str + (5 + 2) * 2, atcmd + 7, atcmd_len - 7);
                atcmd_str_len = strlen(atcmd_str);
            } else {
                for (int i = 0; i < atcmd_len; i++) {
                    char temp[3] = {0};
                    snprintf(temp, 3, "%02X", atcmd[i]);
                    atcmd_str[i * 2] = temp[0];
                    atcmd_str[i * 2 + 1] = temp[1];
                }
                // <FF> <01><02><01> <01/03> <length><type><data(LTV List)>... <FF>
                atcmd_str_len = strlen(atcmd_str);
                APPS_LOG_I("[XIAOAI_HF] handle_mma_atcmd, ATCMD=%s atcmd_str_len=%d\r\n", (char *)atcmd_str, atcmd_str_len);
            }
            bt_status = bt_sink_srv_hf_xiaomi_custom(atcmd_str, atcmd_str_len);
        } else {
            bt_status = bt_sink_srv_hf_xiaomi_custom(atcmd, atcmd_len);
        }
        hfp_success = (bt_status == BT_STATUS_SUCCESS);
    }

exit:
    success = (lea_aws_success || lea_success || hfp_success);
    APPS_LOG_MSGID_I(LOG_TAG" send_atcmd, [%02X] mma_type=0x%04X conn_state=%d is_le_audio_mma=%d lea_aws_success=%d lea_success=%d hfp_success=%d",
                     7, role, mma_type, conn_info.conn_state, conn_info.is_le_audio_mma, lea_aws_success, lea_success, hfp_success);
    return success;
}





/**================================================================================*/
/**                                    HFP AT CMD API                              */
/**================================================================================*/
bool app_va_xiaoai_hfp_miui_basic_atcmd()
{
    bool ret = FALSE;

    char *atcmd = xiaoai_get_miui_at_cmd();
    if (atcmd != NULL) {
        ret = app_va_xiaoai_send_atcmd(XIAOAI_MMA_TYPE_HEADSET_NOTIFY_BASIC_INFO,
                                       atcmd, strlen(atcmd));
        vPortFree(atcmd);
        APPS_LOG_MSGID_I(LOG_TAG" hfp_miui_basic_atcmd", 0);

#ifdef AIR_XIAOAI_MIUI_FAST_CONNECT_ENABLE
        // send MIUI FAST_CONNECT (BLE ADV DATA) again
        if (ret) {
            app_va_xiaoai_report_miui_fast_connect_at_cmd();
        }
#endif
    }
    return ret;
}

void app_va_xiaoai_hfp_miui_more_atcmd_report_feature()
{
#define XIAOMI_HFP_MORE_ATCMD_REPORT_FEATURE_LEN             19
    uint8_t report_data[XIAOMI_HFP_MORE_ATCMD_REPORT_FEATURE_LEN] = {0};

    // bit0 - support upgrade (Yes)
    // bit1 - support anti_lost (Yes)
    // bit2 - support ear detection (Yes)
    // bit3 - support audio mode (No)      // Customer configure option: Audio mode?
    // bit4 - support single ear mode (No)
    // bit5 - support auto ack (Yes)
    // bit6 - support multi connect (Yes)
    // bit7 - support compactness (Yes)
    uint8_t support_feature_flag1 = 0xE7;

    // bit0 - support double click left headset (Yes)
    // bit1 - support double click right headset (Yes)
    // bit2 - support triple click left headset (Yes)
    // bit3 - support triple click right headset (Yes)
    // bit4 - support long press left headset (Yes)
    // bit5 - support long press right headset (Yes)
    // bit6 - support rename headset (Yes)
    // bit7 - support Reversed (No)
    uint8_t support_feature_flag2 = 0x7F;

    // bit0 - support ANC (Yes)
    // bit1 - support voiceprint recognition (Yes)
    // bit2 - support game mode (Yes)
    // bit3 - support voice command training (No)
    // bit4 - support equalizer (Yes)
    // bit5 - Reversed
    // bit6 - Reversed
    // bit7 - Reversed
    uint8_t support_feature_flag3 = 0x17;

    // pack report data
    report_data[0] = 0x12;
    report_data[1] = XIAOAI_HFP_ATCMD_TYPE_REPORT;
    report_data[2] = 0x04;
    report_data[3] = XIAOAI_HFP_ATCMD_TYPE_FLAG;
    report_data[4] = support_feature_flag1;
    report_data[5] = support_feature_flag2;
    report_data[6] = support_feature_flag3;
    report_data[7] = 0x0B;
    report_data[8] = XIAOAI_HFP_ATCMD_TYPE_FEATURE;
    report_data[9] = 0x01;
    // 0 --- MIUI BLE ADV
    // 1 --- MIOT BLE ADV
    // 2 --- Fast Pair with same account
    report_data[10] = 0x01;
    report_data[11] = 0x01;
    report_data[12] = APP_VA_XIAOAI_MAJOR_ID;
    report_data[13] = (APP_VA_XIAOAI_MINOR_ID & 0xFF);
    report_data[14] = ((APP_VA_XIAOAI_MINOR_ID >> 8) & 0xFF);
    report_data[15] = (APP_VA_XIAOAI_VERSION & 0x0F);
    report_data[16] = ((APP_VA_XIAOAI_VERSION >> 4) & 0x0F);
    report_data[17] = ((APP_VA_XIAOAI_VERSION >> 8) & 0x0F);
    report_data[18] = ((APP_VA_XIAOAI_VERSION >> 12) & 0x0F);

    bool ret = app_va_xiaoai_send_hfp_atcmd(XIAOAI_MMA_TYPE_HEADSET_NOTIFY_BASIC_INFO,
                                            XIAOAI_HFP_ATCMD_ATT_STATUS,
                                            report_data,
                                            XIAOMI_HFP_MORE_ATCMD_REPORT_FEATURE_LEN);
    APPS_LOG_MSGID_I(LOG_TAG" miui_more_atcmd_report_feature ret=%d", 1, ret);
}

void app_va_xiaoai_hfp_miui_more_atcmd_report_all_status()
{
#define XIAOAI_HFP_REPORT_ALL_STATUS_LEN      (0x1B + 1)
    uint8_t report_data[XIAOAI_HFP_REPORT_ALL_STATUS_LEN] = {0};
    report_data[0] = 0x1B;
    report_data[1] = XIAOAI_HFP_ATCMD_TYPE_RUN_INFO_CHANGED;
    uint8_t anc_state = app_va_xiaoai_get_anc_state_for_miui();
    uint8_t eq_state = app_va_xiaoai_get_eq_mode();
    uint8_t game_mode_state = app_va_xiaoai_get_game_mode_for_miui();
    uint8_t anti_lost_state = app_va_xiaoai_get_anti_lost_state();
    report_data[2] = 0x02;
    report_data[3] = XIAOAI_HFP_ATCMD_TYPE_ANC;
    report_data[4] = anc_state;
    report_data[5] = 0x07;
    report_data[6] = XIAOAI_HFP_ATCMD_TYPE_KEY;
    report_data[7] = app_va_xiaoai_key_click_function(TRUE, AIRO_KEY_DOUBLE_CLICK);
    report_data[8] = app_va_xiaoai_key_click_function(FALSE, AIRO_KEY_DOUBLE_CLICK);
    report_data[9] = app_va_xiaoai_key_click_function(TRUE, AIRO_KEY_TRIPLE_CLICK);
    report_data[10] = app_va_xiaoai_key_click_function(FALSE, AIRO_KEY_TRIPLE_CLICK);
    report_data[11] = app_va_xiaoai_key_anc_function(TRUE);
    report_data[12] = app_va_xiaoai_key_anc_function(FALSE);
    report_data[13] = 0x05;
    report_data[14] = XIAOAI_HFP_ATCMD_TYPE_VOICE;
    report_data[15] = app_va_xiaoai_is_enable_wwe();
    report_data[16] = app_va_xiaoai_get_voice_recognition_state();
    report_data[17] = 0x1F;
    report_data[18] = 0;
    report_data[19] = 0x02;
    report_data[20] = XIAOAI_HFP_ATCMD_TYPE_EQ;
    report_data[21] = eq_state;
    report_data[22] = 0x02;
    report_data[23] = XIAOAI_HFP_ATCMD_TYPE_GAME_MODE;
    report_data[24] = game_mode_state;
    report_data[25] = 0x02;
    report_data[26] = XIAOAI_HFP_ATCMD_TYPE_ANTI_LOST;
    report_data[27] = anti_lost_state;

    bool ret = app_va_xiaoai_send_hfp_atcmd(XIAOAI_MMA_TYPE_HEADSET_NOTIFY_BASIC_INFO,
                                            XIAOAI_HFP_ATCMD_ATT_STATUS,
                                            report_data,
                                            XIAOAI_HFP_REPORT_ALL_STATUS_LEN);
    APPS_LOG_MSGID_I(LOG_TAG" more_atcmd_report_all_status ret=%d", 1, ret);
}

void app_va_xiaoai_hfp_miui_more_atcmd_report_anc()
{
#define XIAOAI_HFP_REPORT_ANC_LEN      5
    uint8_t report_data[XIAOAI_HFP_REPORT_ANC_LEN] = {0};
    uint8_t anc_state = app_va_xiaoai_get_anc_state_for_miui();
    report_data[0] = 0x04;
    report_data[1] = XIAOAI_HFP_ATCMD_TYPE_REPORT;
    report_data[2] = 0x02;
    report_data[3] = XIAOAI_HFP_ATCMD_TYPE_ANC;
    report_data[4] = anc_state;

    bool ret = app_va_xiaoai_send_hfp_atcmd(XIAOAI_MMA_TYPE_SWICH_ANC_MODE,
                                            XIAOAI_HFP_ATCMD_ATT_STATUS,
                                            report_data,
                                            XIAOAI_HFP_REPORT_ANC_LEN);
    APPS_LOG_MSGID_I(LOG_TAG" more_atcmd_report_anc ret=%d", 1, ret);
}

void app_va_xiaoai_hfp_miui_more_atcmd_report_key()
{
#define XIAOAI_HFP_REPORT_KEY_LEN      10
    uint8_t report_data[XIAOAI_HFP_REPORT_KEY_LEN] = {0};
    report_data[0] = 0x09;
    report_data[1] = XIAOAI_HFP_ATCMD_TYPE_REPORT;
    report_data[2] = 0x07;
    report_data[3] = XIAOAI_HFP_ATCMD_TYPE_KEY;
    report_data[4] = app_va_xiaoai_key_click_function(TRUE, AIRO_KEY_DOUBLE_CLICK);
    report_data[5] = app_va_xiaoai_key_click_function(FALSE, AIRO_KEY_DOUBLE_CLICK);
    report_data[6] = app_va_xiaoai_key_click_function(TRUE, AIRO_KEY_TRIPLE_CLICK);
    report_data[7] = app_va_xiaoai_key_click_function(FALSE, AIRO_KEY_TRIPLE_CLICK);
    report_data[8] = app_va_xiaoai_key_anc_function(TRUE);
    report_data[9] = app_va_xiaoai_key_anc_function(FALSE);

    bool ret = app_va_xiaoai_send_hfp_atcmd(XIAOAI_MMA_TYPE_CUSTOM_KEY,
                                            XIAOAI_HFP_ATCMD_ATT_STATUS,
                                            report_data,
                                            XIAOAI_HFP_REPORT_KEY_LEN);
    APPS_LOG_MSGID_I(LOG_TAG" more_atcmd_report_key ret=%d", 1, ret);
}

void app_va_xiaoai_hfp_miui_more_atcmd_report_voice()
{
#define XIAOAI_HFP_REPORT_VOICE_LEN      8
    uint8_t report_data[XIAOAI_HFP_REPORT_VOICE_LEN] = {0};
    report_data[0] = 0x07;
    report_data[1] = XIAOAI_HFP_ATCMD_TYPE_REPORT;
    report_data[2] = 0x05;
    report_data[3] = XIAOAI_HFP_ATCMD_TYPE_VOICE;
    report_data[4] = app_va_xiaoai_is_enable_wwe();
    report_data[5] = app_va_xiaoai_get_voice_recognition_state();
    // bit0 - support XiaoAI (Yes)
    // bit1 - support prev (Yes)
    // bit2 - support next (Yes)
    // bit3 - support volume up (Yes)
    // bit4 - support volume down (Yes)
    // bit5 - support reject call (No)
    // bit6 - support accept call (No)
    // bit7 - support 0
    report_data[6] = 0x1F;
    report_data[7] = 0;

    bool ret = app_va_xiaoai_send_hfp_atcmd(XIAOAI_MMA_TYPE_HEADSET_NOTIFY_BASIC_INFO,
                                            XIAOAI_HFP_ATCMD_ATT_STATUS,
                                            report_data,
                                            XIAOAI_HFP_REPORT_VOICE_LEN);
    APPS_LOG_MSGID_I(LOG_TAG" more_atcmd_report_voice ret=%d", 1, ret);
}

void app_va_xiaoai_hfp_miui_more_atcmd_report_eq()
{
#define XIAOAI_HFP_REPORT_EQ_LEN      5
    uint8_t report_data[XIAOAI_HFP_REPORT_EQ_LEN] = {0};
    uint8_t eq_state = app_va_xiaoai_get_eq_mode();
    report_data[0] = 0x04;
    report_data[1] = XIAOAI_HFP_ATCMD_TYPE_REPORT;
    report_data[2] = 0x02;
    report_data[3] = XIAOAI_HFP_ATCMD_TYPE_EQ;
    report_data[4] = eq_state;

    bool ret = app_va_xiaoai_send_hfp_atcmd(XIAOAI_MMA_TYPE_EQ_MODE,
                                            XIAOAI_HFP_ATCMD_ATT_STATUS,
                                            report_data,
                                            XIAOAI_HFP_REPORT_EQ_LEN);
    APPS_LOG_MSGID_I(LOG_TAG" more_atcmd_report_eq ret=%d", 1, ret);
}

void app_va_xiaoai_hfp_miui_more_atcmd_report_game_mode()
{
#define XIAOAI_HFP_REPORT_GAME_MODE_LEN      5
    uint8_t report_data[XIAOAI_HFP_REPORT_GAME_MODE_LEN] = {0};
    uint8_t game_mode_state = app_va_xiaoai_get_game_mode_for_miui();
    report_data[0] = 0x04;
    report_data[1] = XIAOAI_HFP_ATCMD_TYPE_REPORT;
    report_data[2] = 0x02;
    report_data[3] = XIAOAI_HFP_ATCMD_TYPE_GAME_MODE;
    report_data[4] = game_mode_state;

    bool ret = app_va_xiaoai_send_hfp_atcmd(XIAOAI_MMA_TYPE_HEADSET_NOTIFY_BASIC_INFO,
                                            XIAOAI_HFP_ATCMD_ATT_STATUS,
                                            report_data,
                                            XIAOAI_HFP_REPORT_GAME_MODE_LEN);
    APPS_LOG_MSGID_I(LOG_TAG" more_atcmd_report_game_mode ret=%d", 1, ret);
}

void app_va_xiaoai_hfp_miui_more_atcmd_report_anti_lost(uint8_t data)
{
#define XIAOAI_HFP_REPORT_ANTI_LOST_LEN      5
    uint8_t report_data[XIAOAI_HFP_REPORT_ANTI_LOST_LEN] = {0};
    report_data[0] = 0x04;
    report_data[1] = XIAOAI_HFP_ATCMD_TYPE_REPORT;
    report_data[2] = 0x02;
    report_data[3] = XIAOAI_HFP_ATCMD_TYPE_ANTI_LOST;
    report_data[4] = data;

    bool ret = app_va_xiaoai_send_hfp_atcmd(XIAOAI_MMA_TYPE_ANTI_LOST,
                                            XIAOAI_HFP_ATCMD_ATT_STATUS,
                                            report_data,
                                            XIAOAI_HFP_REPORT_ANTI_LOST_LEN);
    APPS_LOG_MSGID_I(LOG_TAG" more_atcmd_report_anti_lost, anti_lost=0x%02X ret=%d",
                     2, data, ret);
}

void app_va_xiaoai_hfp_handle_mma_atcmd(uint16_t type, uint8_t *atcmd_data, uint16_t atcmd_data_len)
{
    APPS_LOG_I(LOG_TAG" handle_mma_atcmd, type=0x%04X atcmd=0x%08X atcmd_len=%d",
               3, type, atcmd_data, atcmd_data_len);
    // for MMA_LEA, HFP AT CMD will use HEX not string, covert to original HFP AT CMD
    if (atcmd_data != NULL && atcmd_data_len > 0) {
        char atcmd_str[XIAOAI_HFP_MORE_ATCMD_MAX_LEN] = {0};
        if (type == XIAOAI_MMA_TYPE_PHONE_MODIFY_HEADSET_NAME) {
            // // Special case for name change, <FF><01><02><01><03> <len><01>
            for (int i = 0; i < 5 + 2; i++) {
                char temp[3] = {0};
                snprintf(temp, 3, "%02X", atcmd_data[i]);
                atcmd_str[i * 2] = temp[0];
                atcmd_str[i * 2 + 1] = temp[1];
            }
            memcpy(atcmd_str + (5 + 2) * 2, atcmd_data + 7, atcmd_data_len - 7);
        } else {
            for (int i = 0; i < atcmd_data_len; i++) {
                char temp[3] = {0};
                snprintf(temp, 3, "%02X", atcmd_data[i]);
                atcmd_str[i * 2] = temp[0];
                atcmd_str[i * 2 + 1] = temp[1];
            }
            // <FF> <01><02><01> <01/03> <length><type><data(LTV List)>... <FF>
            atcmd_data_len = strlen(atcmd_str);
            APPS_LOG_I("[XIAOAI_HF] handle_mma_atcmd, ATCMD=%s atcmd_len=%d\r\n", (char *)atcmd_str, atcmd_data_len);
        }

        char *atcmd = atcmd_str + APP_XIAOMI_HFP_HEADER_LEN - 2;
        int type = 0;
        sscanf(atcmd, "%02x", &type);
        if (type == XIAOAI_HFP_ATCMD_ATT_STATUS) {
            app_va_xiaoai_handle_get_status(atcmd + 2);
        } else if (type == XIAOAI_HFP_ATCMD_ATT_CONFIG) {
            app_va_xiaoai_handle_config_request(atcmd + 2);
        }
#ifdef AIR_XIAOAI_MIUI_FAST_CONNECT_ENABLE
        else if (type == XIAOAI_HFP_ATCMD_ATT_MIUI_FAST_CONNECT) {
            app_va_xiaoai_handle_miui_fast_connect_request(atcmd + 2);
        }
#endif
    }

}

void app_va_xiaoai_hfp_at_cmd_register(bool enable)
{
    bt_status_t status = BT_STATUS_FAIL;
    if (enable) {
        status = bt_callback_manager_register_callback(bt_callback_type_app_event,
                                                       MODULE_MASK_HFP, (void *)app_va_xiaoai_hfp_callback);
    } else {
        status = bt_callback_manager_deregister_callback(bt_callback_type_app_event,
                                                         (void *)app_va_xiaoai_hfp_callback);
    }
    APPS_LOG_MSGID_I(LOG_TAG" register HFP enable=%d status=0x%08X",
                     2, enable, status);
}

void app_va_xiaoai_report_miui_fast_connect_at_cmd()
{
    //APPS_LOG_MSGID_I(LOG_TAG" report_miui_fast_connect_at_cmd", 0);
    app_va_xiaoai_reply_ble_adv_data_cmd();
}

// ZMI Customer
bool xiaoai_va_xiaoai_hfp_pkg_is_ys(void)
{
    bool ret = FALSE;
    if (memcmp(g_xiaoai_pkg_name, APP_XIAOMI_HFP_YS_PKG_NAME, strlen(APP_XIAOMI_HFP_YS_PKG_NAME)) == 0
        || memcmp(g_xiaoai_pkg_name, APP_XIAOMI_HFP_YUANSHEN_PKG_NAME, strlen(APP_XIAOMI_HFP_YUANSHEN_PKG_NAME)) == 0) {
        ret = TRUE;
    }
    APPS_LOG_MSGID_I(LOG_TAG" current app pkg is ys ret=%d", 1, ret);
    return ret;
}

void xiaoai_va_xiaoai_hfp_reset_pkg(void)
{
    memset(g_xiaoai_pkg_name, 0, XIAOAI_MMA_MAX_PACKAGE_NAME);
    APPS_LOG_MSGID_I(LOG_TAG" xiaoai_va_xiaoai_hfp_reset_pkg", 0);
}

#endif /* AIR_XIAOAI_ENABLE */
