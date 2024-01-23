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
 * File: app_fast_pair_idle_activity.c
 *
 * Description: This file could initialize fast pair, and receive events to update fast pair status.
 *
 * Note: See doc/Airoha_IoT_SDK_Application_Developers_Guide.pdf for fast pair APP.
 *
 */

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "portable.h"
#include "syslog.h"
#include "bt_fast_pair.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"
#include "apps_config_features_dynamic_setting.h"
#include "app_bt_state_service.h"
#ifdef AIR_MULTI_POINT_ENABLE
#include "app_bt_emp_service.h"
#endif
#include "app_fast_pair.h"
#include "app_fast_pair_nvkey_struct.h"
#include "bt_sink_srv.h"
#include "ui_shell_manager.h"
#include "bt_app_common.h"
#include "bt_device_manager_le.h"
#include "bt_device_manager.h"
#include "apps_debug.h"
#include "bt_init.h"
#include "bt_gap_le.h"
#include "bt_callback_manager.h"
#include "apps_events_bt_event.h"
#include "multi_ble_adv_manager.h"
#include "multi_va_event_id.h"
#include "app_bt_takeover_service.h"
#include "nvkey.h"
#include "nvdm.h"
#include "nvkey_id_list.h"
#ifdef MTK_AWS_MCE_ENABLE
#include "bt_aws_mce_report.h"
#include "bt_sink_srv_ami.h"
#include "bt_aws_mce_srv.h"
#endif
#include "bt_sink_srv_a2dp.h"
#ifdef RACE_FIND_ME_ENABLE
#include "app_fm_activity.h"
#endif
#include "apps_aws_sync_event.h"
#ifdef AIR_SMART_CHARGER_ENABLE
#include "app_smcharger_idle_activity.h"
#else
#if defined(MTK_BATTERY_MANAGEMENT_ENABLE)
#include "app_battery_transient_activity.h"
#endif
#endif
#include "apps_config_key_remapper.h"
#include "apps_events_battery_event.h"
#if defined(MTK_BATTERY_MANAGEMENT_ENABLE)
#include "battery_management.h"
#endif
#include "nvdm_id_list.h"
#include "nvkey_id_list.h"
#include "bt_customer_config.h"
#include "verno.h"
#include "atci.h"
#include "system_daemon.h"
#include "bt_device_manager_link_record.h"

#ifdef AIR_LE_AUDIO_ENABLE
#include "app_le_audio.h"
#include "bt_device_manager_le.h"
#include "bt_sink_srv_le_cap_audio_manager.h"
#include "bt_sink_srv_le_cap_stream.h"
#include "app_lea_service.h"
#include "bt_sink_srv_le.h"
#endif
#include <string.h>

#ifdef MTK_IN_EAR_FEATURE_ENABLE
#include "app_in_ear_utils.h"
#endif
#include "apps_config_event_list.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"

#define LOG_TAG     "[app_fast_pair] "
#define APP_ONLY_ALLOW_TAKE_OVER_BY_FAST_PAIR       (false) /* When it's true, only allow BT take over by fast pair flow. */

#define APP_FAST_PAIR_DISCOVER_MODE_ADV_INTERVAL    (0xA0)  /* The adv interval when BT is discoverable. 0xA0 means 100ms */
#define APP_FAST_PAIR_NON_DISCOVER_MODE_ADV_INTERVAL (0x190) /* The adv interval when BT is not discoverable. 0x190 means 250ms */
#define REMAINING_BATTERY_TIME_WHEN_FULL            (240)   /* The remaining battery time in minutes when battery is 100%. */

#define FAST_PAIR_PRIVATE_PROTECTED     /* If defined, the module id is encrypted and stored in NVKEY. */
#ifdef FAST_PAIR_PRIVATE_PROTECTED

#ifdef AIR_BT_FAST_PAIR_SASS_ENABLE
const static bt_bd_addr_t empty_addr = { 0 };
#endif

/* Encrypt and decrypt key, it's must as same as the "Private Key" in Page "Encryption_NvKey" of config Tool.
 * The config tool while use the same key to encrypt fast pair related nvdm. FW can use the same private key to decrypt it.
*/
const char app_fast_pair_nvdm_key[32] = {0x12, 0x34, 0x56, 0x78, 0x12, 0x34, 0x56, 0x78, \
                                         0x12, 0x34, 0x56, 0x78, 0x12, 0x34, 0x56, 0x78, \
                                         0x12, 0x34, 0x56, 0x78, 0x12, 0x34, 0x56, 0x78, \
                                         0x12, 0x34, 0x56, 0x78, 0x12, 0x34, 0x56, 0x78
                                        };
#endif

#ifdef MTK_AWS_MCE_ENABLE
#define APP_FAST_PAIR_COMPONENT_NUM                 (3)         /* 3 component means left, right and charger case */
#define APP_FAST_PAIR_AWS_CONTEXT_SYNC_ID           (0xE0)      /* The event id of event group EVENT_GROUP_UI_SHELL_BT_FAST_PAIR */
#define APP_FAST_PAIR_AWS_SYNC_SASS_CONTROL_MSG     (0xE1)      /* The event id of event group EVENT_GROUP_UI_SHELL_BT_FAST_PAIR */
#define APP_FAST_PAIR_AWS_SYNC_LE_IO_CAPABILITY     (0xE2)      /* The event id of event group EVENT_GROUP_UI_SHELL_BT_FAST_PAIR */
#else
#define APP_FAST_PAIR_COMPONENT_NUM                 (1)         /* 1 component means headset itself. */
#endif

/* If Fast pair related nvkey is invalid, will use below config. */
#define APP_FAST_PAIR_DEFAULT_MAX_ACCOUNT           (5)         /* Maximum google account. */
#define APP_FAST_PAIR_DEFAULT_TX_POWER_LEVEL        (-20)       /* TX Power level set in fast pair adv data. -20 means -20db. */
#ifdef MTK_AWS_MCE_ENABLE
#define APP_FAST_PAIR_PERSONALIZED_NAME             "Airoha_fast_pair_earbuds"      /* Default name. Because we use EDR name as default, it's not useful now. */
#else
#define APP_FAST_PAIR_PERSONALIZED_NAME             "Airoha_fast_pair_headset"      /* Default name. Because we use EDR name as default, it's not useful now. */
#endif

#ifdef AIR_TWS_ENABLE
#define APP_FAST_PAIR_AIROHA_TEST_MODEL_ID          (0xCAC28A)  /* Default model ID. It's useful if customer not set any model set in config tool. */
/* When there is not set any model id, use the default private key. */
#define APP_FAST_PAIR_AIROHA_TEST_PRIVATE_KEY       {0x3C, 0x2D, 0xAB, 0x1F, 0x22, 0x04, 0xCF, 0xA9, \
                                                     0xD3, 0xB6, 0x5B, 0xE9, 0xBC, 0x0A, 0x45, 0x7C, \
                                                     0x00, 0x5C, 0xE9, 0xE4, 0xC5, 0x72, 0xF8, 0x09, \
                                                     0x55, 0xFF, 0xC8, 0x2C, 0x2B, 0x2D, 0xB1, 0xFD}
#else
#define APP_FAST_PAIR_AIROHA_TEST_MODEL_ID          (0xB2DC3E)
/* When there is not set any model id, use the default private key. */
#define APP_FAST_PAIR_AIROHA_TEST_PRIVATE_KEY       {0xAA, 0xA2, 0x7D, 0xA3, 0x78, 0x59, 0xF3, 0x48,\
                                                     0xFE, 0xDC, 0x07, 0xF1, 0x10, 0x9D, 0x61, 0xC8,\
                                                     0xC2, 0x7B, 0x01, 0xEF, 0xB4, 0x22, 0xC0, 0xAA,\
                                                     0x91, 0xE7, 0xE0, 0xFB, 0x0F, 0x0C, 0x61, 0x7D}
#endif

#define APP_FAST_PAIR_COMPONENT_LEFT                (0) /* In fast pair spec, the battery value of left earbud is bit 0. */
#define APP_FAST_PAIR_COMPONENT_RIGHT               (1) /* In fast pair spec, the battery value of right earbud is bit 1. */
#define APP_FAST_PAIR_COMPONENT_CASE                (2) /* In fast pair spec, the battery value of charger case is bit 2. */

#ifdef AIR_BT_FAST_PAIR_SASS_ENABLE
#define FAST_PAIR_SASS_DEFAULT_ENABLED              (1)
#define FAST_PAIR_SASS_VERSION_CODE                 (0x0201)    /* Spec is 0x0101, but the format of Fast pair is big-endian. */
#ifdef AIR_MULTI_POINT_ENABLE
#define FAST_PAIR_SASS_MULTI_POINT                  (1)
#else
#define FAST_PAIR_SASS_MULTI_POINT                  (0)
#endif
#ifdef MTK_IN_EAR_FEATURE_ENABLE
#define FAST_PAIR_SASS_ON_HEAD_DETECTION            (1)
#else
#define FAST_PAIR_SASS_ON_HEAD_DETECTION            (0)
#endif

#ifdef AIR_BT_FAST_PAIR_LE_AUDIO_ENABLE
typedef struct {
    bt_bd_addr_t addr;
    bt_fast_pair_message_stream_t msg_data;
} __attribute__((packed)) app_fast_pair_sass_aws_sync_ctrl_msg_t;
#endif
#endif /* #ifdef AIR_BT_FAST_PAIR_SASS_ENABLE */

typedef struct {
    uint8_t tone;
#ifdef AIR_TWS_ENABLE
    uint8_t peer_tone;
#endif
} app_fast_pair_find_me_state_t;

#define FAST_PAIR_SPOT_ROTATE_TIME_MS (1024 * 1000)
#define FAST_PAIR_SPOT_UNWANTED_MODE_ROTATE_TIME_MS (24 * 60 * 60 * 1000)

#ifdef AIR_SPOT_ENABLE
static bool s_spot_in_adv;
#define FAST_PAIR_RANDOM_WINDOW 204
#endif

/**
 *  @brief This structure defines the format of the context of the activity.
 */
typedef struct {
    app_fast_pair_nvkey_t           nvkey;              /* The configuration in NVID_APP_FAST_PAIR_CONFIGURE, refer to app_fast_pair_nvkey_t. */
    bt_fast_pair_battery_t          battery;            /* The battery status, refer to bt_fast_pair_battery_t. */
    bt_fast_pair_account_key_list_t *account_key_list;  /* The paired google account. */
    bt_gap_le_advertising_handle_t  adv_handle;         /* The value indicate which advertising handle is for fast pair */
#if APP_ONLY_ALLOW_TAKE_OVER_BY_FAST_PAIR
    bool                            waiting_connect;    /* When it's true, means waiting EDR connection */
#endif
#ifdef AIR_TILE_ENABLE
    bool                            is_classic_bt_on;      /* When it's true, gfp can broadcast adv */
#endif
#ifdef AIR_BT_FAST_PAIR_SASS_ENABLE
    bt_bd_addr_t                    drop_target;
#ifdef AIR_MULTI_POINT_ENABLE
    bt_bd_addr_t                    allow_switch_audio_addr;        /* Record the address of last allow switch audio resource. */
    bt_bd_addr_t                    temp_allow_switch_audio_addr;   /* Record the address of device just connected BT. */
    /*bt_bd_addr_t                    switch_done_target_addr;*/        /* Record the address of target address of switch audio source done. */
    bt_fast_pair_sass_multipoint_audio_src_switch_reason_t switch_reason; /* Record the switch reason of switch audio source done. */
#endif
    uint8_t                         connection_switch_back_event;   /* record last time switch back event. */
    uint8_t                         sass_switch_prefer_flags; /* The SASS multipoint switching preference flag */
#endif
    uint32_t                        model_id;           /* The model id for fast pair. */
#ifdef AIR_SPOT_ENABLE
    bool                            spot_enabled;
#endif
} app_fast_pair_local_context_t;

#define APP_FAST_PAIR_PROTECTED_CRY_LEN             (sizeof(app_fast_pair_protected_t)) /* The size to read NVkey. */

static app_fast_pair_local_context_t g_app_fast_pair_context;   /* The variable records context */

#ifdef RACE_FIND_ME_ENABLE
static app_fast_pair_find_me_state_t s_app_fast_pair_find_me_state;
#endif

#ifdef FAST_PAIR_PRIVATE_PROTECTED
/**
* @brief      This function decrypt data by a private key.
* @param[in]  p_key, the private key.
* @param[in]  p_in, the input data, it is encrypted.
* @param[in]  len, the length of p_in. Note: the buffer of the p_out should be larger than (len - 4). 4 is the size of CRC.
* @param[in]  p_out, the buffer of decrypted output data.
* @return     -1 means CRC fail, > 0 means the output length.
*/
extern int  nvkey_decrypt(unsigned char *p_key, unsigned char *p_in, int len, unsigned char *p_out);
#endif

static bool app_fast_pair_app_event_proc_sass_msg(bt_bd_addr_t *addr, bt_fast_pair_message_stream_t *msg_data, bool from_fp_msg);

/**
 * @brief      The ATCI cmd handler, refer to at_cmd_hdlr_fp.
 * @param[in]  parse_cmd, The value is defined in #atci_parse_cmd_param_t. This parameter is given by the ATCI
 *             parser to indicate the input command data to be transferred to the command handler.
 * @return     ATCI_STATUS_OK means success, otherwise means fail.
 */
static atci_status_t _fast_pair_test_cmd_handler(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t *response = malloc(sizeof(atci_response_t));
    memset(response, 0, sizeof(atci_response_t));
    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
    char *cmd_params = parse_cmd->string_ptr + parse_cmd->name_len + 1; /* At the position behind '='. */
    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            if (0 == memcmp(cmd_params, "Silence_mode,", strlen("Silence_mode,"))) {
                bool enable = *(cmd_params + strlen("Silence_mode,")) == '1' ? true : false;
                bt_fast_pair_set_silence_mode(NULL, enable);
            }
#ifdef AIR_BT_FAST_PAIR_SASS_ENABLE
            /* Format is =SWITCH_ACTIVE,AA:BB:CC:DD:EE:FF,0x80 */
            else if (0 == memcmp(cmd_params, "SWITCH_ACTIVE,", strlen("SWITCH_ACTIVE,"))) {
                uint8_t i = 0;
                bt_bd_addr_t addr = { 0 };
                uint8_t flag = 0;
                char decode_str[3] = { 0 };
                cmd_params = cmd_params + strlen("SWITCH_ACTIVE,");
                for (i = 0; i < sizeof(bt_bd_addr_t); i++) {
                    memcpy(decode_str, cmd_params + 3 * i, sizeof(decode_str) - 1);
                    addr[i] = strtol(decode_str, NULL, 16);
                }
                memcpy(decode_str, cmd_params + 3 * sizeof(bt_bd_addr_t), sizeof(decode_str) - 1);
                flag = strtol(decode_str, NULL, 16);
                bt_fast_pair_message_stream_t message = {
                    .group_ID = BT_FAST_PAIR_MESSAGE_GROUP_SMART_AUDIO_SOURCE_SWITCHING,
                    .code_ID = BT_FAST_PAIR_MESSAGE_CODE_SASS_SWITCH_ACTIVE_AUDIO_SOURCE,
                    .data_length = sizeof(flag) + sizeof(bt_fast_pair_sass_mac_verify_data_t),
                    .data[0] = flag,
                };
                app_fast_pair_app_event_proc_sass_msg(&addr, &message, false);
            }
#endif
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            break;
        }
        default:
            break;
    }
    response->response_len = strlen((char *)response->response_buf);
    atci_send_response(response);
    free(response);
    return ATCI_STATUS_OK;
}

/* The atci command table. */
static atci_cmd_hdlr_item_t fast_pair_test_atci_cmd[] = {
    {
        .command_head = "AT+FASTPAIRTEST",    /**< Test fast pair */
        .command_hdlr = _fast_pair_test_cmd_handler,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
};

/**
* @brief      This function read nvkey to get fast pair configuration.
* @param[in]  context, the context of the app.
* @param[in]  context, the pointer of the a fast_pair_protected data buffer.
* @return     0 means success, -1 means parameter error, -2 means read NVkey fail.
*/
static int8_t app_fast_pair_load_configure_info(app_fast_pair_local_context_t *context, app_fast_pair_protected_t *protected_set)
{
    if (NULL == context) {
        return -1;
    }

    uint32_t size = sizeof(app_fast_pair_nvkey_t);
    if (NVKEY_STATUS_OK != nvkey_read_data(NVID_APP_FAST_PAIR_CONFIGURE, (uint8_t *) & (context->nvkey), &size)) {
        return -2;
    }
    APPS_LOG_MSGID_I(LOG_TAG" load configure data size %d", 1, size);

    size = APP_FAST_PAIR_PERSONALIZED_NAME_SIZE;
    nvkey_read_data(NVID_APP_FAST_PAIR_PERSONALIZED_NAME, (uint8_t *)context->nvkey.personalized_name, &size);

    uint8_t *cry = (uint8_t *)pvPortMalloc(APP_FAST_PAIR_PROTECTED_CRY_LEN);
    if (cry) {
        memset(cry, 0, APP_FAST_PAIR_PROTECTED_CRY_LEN);
    }
    size = APP_FAST_PAIR_PROTECTED_CRY_LEN;
    if (NULL == cry || NVKEY_STATUS_OK != nvkey_read_data(NVID_APP_FAST_PAIR_PROTECTED, cry, &size)) {
        if (cry) {
            vPortFree(cry);
        }
        return -2;
    }
#ifdef FAST_PAIR_PRIVATE_PROTECTED
    /* Decrypt the model set. */
    nvkey_decrypt((unsigned char *)app_fast_pair_nvdm_key, cry, APP_FAST_PAIR_PROTECTED_CRY_LEN, (unsigned char *) protected_set);
#else
    memcpy(protected_set, cry, sizeof(app_fast_pair_protected_t));
#endif
    vPortFree(cry);
    return 0;
}

/**
* @brief      This function read nvkey to get paired accounts.
* @param[in]  context, the context of the app.
*/
static void app_fast_pair_load_account_key(app_fast_pair_local_context_t *context)
{
    nvkey_status_t status;
    uint8_t saved_max_number = context->nvkey.max_account;
    uint32_t size = sizeof(bt_fast_pair_account_key_list_t) +
                    sizeof(bt_fast_pair_account_key_t) * saved_max_number - sizeof(bt_fast_pair_account_key_t);
    memset(context->account_key_list, 0, size);
    status = nvkey_read_data(NVID_APP_FAST_PAIR_ACCOUNT_KEY, (uint8_t *)(context->account_key_list), &size);
    if (NVKEY_STATUS_OK != status) {
        context->account_key_list->max_key_number = context->nvkey.max_account;
        return;
    }
}

/**
* @brief      This function write nvkey to update paired accounts.
* @param[in]  account_key, the array of account_key.
*/
static void app_fast_pair_store_account_key(bt_fast_pair_account_key_list_t *account_key)
{
    nvkey_status_t status;
    uint32_t size = sizeof(bt_fast_pair_account_key_list_t) +
                    sizeof(bt_fast_pair_account_key_t) * account_key->max_key_number - sizeof(bt_fast_pair_account_key_t);

    APPS_LOG_MSGID_E(LOG_TAG" Account key write nvdm size: %d", 1, size);
    status = nvkey_write_data(NVID_APP_FAST_PAIR_ACCOUNT_KEY, (uint8_t *)(account_key), size);
    if (NVKEY_STATUS_OK != status) {
        return;
    }
}

static void app_fast_pair_set_adv_param(multi_ble_adv_info_t *adv_data, uint16_t interval)
{
    if (!adv_data->adv_param) {
        return;
    }
    APPS_LOG_MSGID_E(LOG_TAG" app_fast_pair_set_adv_param", 0);
    bt_app_common_generate_default_adv_data(adv_data->adv_param, NULL, NULL, NULL, 0);
    adv_data->adv_param->primary_advertising_interval_min = interval;
    /* Interval should be no larger than 100ms when discoverable */
    adv_data->adv_param->primary_advertising_interval_max = interval;
}

/**
* @brief      This function is the callback function to implement, refer to get_ble_adv_data_func_t.
* @param[in]  adv_data, a structure need to fill data in the callback function.
*/
static uint32_t app_fast_pair_get_adv_data(multi_ble_adv_info_t *adv_data)
{
    bt_fast_pair_advertising_data_t type;
    bt_fast_pair_status_t ret;
    uint16_t interval;

    if (app_bt_connection_service_get_current_status()->bt_visible) {
        type = BT_FAST_PAIR_ADVERTISING_DATA_MODEL_ID;
        interval = APP_FAST_PAIR_DISCOVER_MODE_ADV_INTERVAL;
    } else {
        type = BT_FAST_PAIR_ADVERTISING_DATA_ACCOUNT_AND_BATTERY;
        if (g_app_fast_pair_context.battery.ui_show) {
            interval = APP_FAST_PAIR_NON_DISCOVER_MODE_ADV_INTERVAL;
        } else {
            interval = APP_FAST_PAIR_DISCOVER_MODE_ADV_INTERVAL;
        }
    }

    app_fast_pair_set_adv_param(adv_data, interval);

    if (adv_data->adv_data) {
        uint8_t data_len = adv_data->adv_data->data_length > 0xFF ? 0xFF : adv_data->adv_data->data_length;
        /* Call API in middleware to get adv data. */
        ret = bt_fast_pair_get_advertising_data(adv_data->adv_data->data, &data_len, type);
        if (ret != BT_FAST_PAIR_STATUS_SUCCESS) {
            return 0;
        } else {
            adv_data->adv_data->data_length = data_len;
        }
    }
    if (adv_data->scan_rsp) {
#ifdef AIR_BT_FAST_PAIR_LE_AUDIO_ENABLE
        adv_data->scan_rsp->data_length = 3;
        adv_data->scan_rsp->data[0] = 2;
        adv_data->scan_rsp->data[1] = BT_GAP_LE_AD_TYPE_FLAG;
        adv_data->scan_rsp->data[2] = BT_GAP_LE_AD_FLAG_GENERAL_DISCOVERABLE | (0x01 << 3) | (0x01 << 4);
#else
        adv_data->scan_rsp->data_length = 0;
#endif
    }

    return 0;
}

extern bt_status_t bt_device_manager_le_generate_rpa(bt_device_manager_le_bonded_info_t *bond_info,
                                              uint8_t *prand, uint32_t prand_length, bt_bd_addr_t *rpa);
/**
* @brief      This function to generate random address when start adv or every 15 min.
*/
static void app_fast_pair_generate_random_address(uint32_t interval)
{
    bt_bd_addr_t addr = { 0 };
//    bt_app_common_generate_random_address(addr);
//    addr[5] |= 0x40;
//    addr[5] &= 0x7f;
    bt_bd_addr_t peer_addr[1] = {
        {0, 0, 0, 0, 0, 0}
    };
    uint8_t count = 1;
    bt_device_manager_le_get_bonded_list(peer_addr, &count);
    bt_gap_le_bonding_info_t bond_info;
    bt_gap_le_bonding_info_t *bonded_info = NULL;
    bt_device_manager_le_bonded_info_t bonded_info_rpa;
    memset(&bond_info, 0, sizeof(bond_info));
    memset(&bonded_info_rpa, 0, sizeof(bt_device_manager_le_bonded_info_t));
    if (count > 0 && ((bonded_info = bt_device_manager_le_get_bonding_info_by_addr(&peer_addr[0])) != NULL)) {
        memcpy(&bond_info, bonded_info, sizeof(bt_gap_le_bonding_info_t));
        //bt_device_manager_le_set_bonding_info_by_addr(&(bond_info.identity_addr.address), &bond_info);
    } else {
        bt_app_common_generate_random_address(peer_addr[0]);
        bond_info.identity_addr.address.type = BT_ADDR_RANDOM;
        memcpy(bond_info.identity_addr.address.addr, peer_addr, sizeof(peer_addr));
        bond_info.key_security_mode = BT_GAP_LE_SECURITY_BONDED_MASK;
        memcpy(bond_info.local_key.identity_info.irk, bt_app_common_get_ble_local_irk(), sizeof(bond_info.local_key.identity_info.irk));
        memset(&bond_info.identity_info.irk, 0xFF, sizeof(bond_info.identity_info.irk));
        bt_device_manager_le_set_bonding_info_by_addr(&(bond_info.identity_addr.address), &bond_info);
    }

    uint32_t key = 0;
    bt_app_common_generate_random_key((uint8_t *)&key, sizeof(key));

    APPS_LOG_MSGID_I(LOG_TAG" app_fast_pair_generate_random_address: count: %d, random: %x", 2, count, key);

    memcpy(&bonded_info_rpa.info, &bond_info, sizeof(bt_gap_le_bonding_info_t));
    bt_device_manager_le_generate_rpa(&bonded_info_rpa, (uint8_t *)&key, 3, &addr);
    multi_ble_adv_manager_set_address(MULTI_ADV_INSTANCE_FAST_PAIR, addr, false);
    bt_fast_pair_set_random_ble_address(true, &addr);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_FAST_PAIR_GENERATE_ADV_ADDR);
    ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                        APPS_EVENTS_INTERACTION_FAST_PAIR_GENERATE_ADV_ADDR, NULL, 0, NULL, interval);
}

/**
* @brief      This function update the fast pair adv.
* @param[in]  local_context, the context of the app.
*/
static void app_fast_pair_trigger_advertising(app_fast_pair_local_context_t *local_context)
{
#ifdef AIR_TILE_ENABLE
    if (!g_app_fast_pair_context.is_classic_bt_on) {
        return;
    }
#endif
    /* For re-start ADV if connected by unexpected devices */
    APPS_LOG_MSGID_I(LOG_TAG" Trigger advertising visibility status: %d", 1, app_bt_connection_service_get_current_status()->bt_visible);

    if (apps_config_features_is_mp_test_mode()) {
        return;
    }
#if 0
    if (bt_sink_srv_get_connected_devices_number() >= 1) {
        /* Once connected one ACL link, then don't start ADV to avoid
         * conflict with other BLE module. */
        APPS_LOG_MSGID_W("[APP_FAST_PAIR]ACL link already exited: %d", 1, is_vis_on);
        return;
    }
#endif
    multi_ble_adv_manager_remove_ble_adv(MULTI_ADV_INSTANCE_FAST_PAIR, app_fast_pair_get_adv_data);
    multi_ble_adv_manager_add_ble_adv(MULTI_ADV_INSTANCE_FAST_PAIR, app_fast_pair_get_adv_data, 1);
    multi_ble_adv_manager_notify_ble_adv_data_changed(MULTI_ADV_INSTANCE_FAST_PAIR);
}

#ifdef AIR_SPOT_ENABLE
#ifdef MTK_AWS_MCE_ENABLE
static bool app_fast_pair_spot_check_eik(uint8_t *buf, uint32_t size)
{
    if (size != 32) {
        return false;
    }
    int idx = 0;
    for (idx = 0; idx < 32; idx++) {
        if (buf[idx] != 0) {
            break;
        }
    }
    if (idx >= 32) {
        return false;
    }
    return true;
}
#endif

bool app_fast_pair_spot_in_adv()
{
    return s_spot_in_adv;
}

static uint8_t temp_buf[64];
static uint8_t s_unwanted_tracking_mode_enable = 0;
#define BT_FAST_PAIR_SPOT_APP_EVENT_UNWANTED_TRACKING_MODE_ENABLE 0x2000
#define BT_FAST_PAIR_SPOT_APP_EVENT_UNWANTED_TRACKING_MODE_DISABLE 0x2001
uint8_t bt_fast_pair_spot_unwanted_tracking_mode(uint8_t en, bool set)
{
    if (!set) {
        static bool s_load = false;
        if (s_load) {
            return s_unwanted_tracking_mode_enable;
        }
        s_load = true;
        nvkey_status_t sta = NVKEY_STATUS_OK;
        uint32_t read_len = sizeof(s_unwanted_tracking_mode_enable);
        sta = nvkey_read_data(NVID_APP_FPS_SPOT_UNTRACKING_MODE, (uint8_t *)&s_unwanted_tracking_mode_enable, &read_len);
        if (sta == NVKEY_STATUS_ITEM_NOT_FOUND) {
            s_unwanted_tracking_mode_enable = 0x0;
        } else if (sta != NVKEY_STATUS_OK) {
            s_unwanted_tracking_mode_enable = 0x0;
            APPS_LOG_MSGID_I(LOG_TAG"[SPOT] load unwanted tracking mode fail", 0);
        }
        APPS_LOG_MSGID_I(LOG_TAG"[SPOT] load unwanted tracking mode sta=%d, ret=%d", 2, s_unwanted_tracking_mode_enable, sta);
    } else {
        s_unwanted_tracking_mode_enable = en;
        nvkey_status_t sta = nvkey_write_data(NVID_APP_FPS_SPOT_UNTRACKING_MODE, (uint8_t *)&s_unwanted_tracking_mode_enable, sizeof(s_unwanted_tracking_mode_enable));
#ifdef MTK_AWS_MCE_ENABLE
        if (bt_connection_manager_device_local_info_get_aws_role() == BT_AWS_MCE_ROLE_AGENT) {
            apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_BT_FAST_PAIR,
                                           en ? BT_FAST_PAIR_SPOT_APP_EVENT_UNWANTED_TRACKING_MODE_ENABLE : BT_FAST_PAIR_SPOT_APP_EVENT_UNWANTED_TRACKING_MODE_DISABLE,
                                           &s_unwanted_tracking_mode_enable, sizeof(s_unwanted_tracking_mode_enable));
        }
#endif
        APPS_LOG_MSGID_I(LOG_TAG"[SPOT] bt_fast_pair_spot_unwanted_tracking_mode set to %d, ret=%d", 2, en, sta);
    }

    /* As spec required, if unwanted tracking mode enabled, the MAC address should be rotate every 24H
     * and roate every 1024s after this mode disabled.
     */
    app_fast_pair_generate_random_address(s_unwanted_tracking_mode_enable ? FAST_PAIR_SPOT_UNWANTED_MODE_ROTATE_TIME_MS : FAST_PAIR_SPOT_ROTATE_TIME_MS);
    if (en) {
        ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                            APPS_EVENTS_INTERACTION_SPOT_GEN_EID_IN_UNWANTED_TRACKING_MODE, NULL, 0, NULL, FAST_PAIR_SPOT_ROTATE_TIME_MS);
    } else {
        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_SPOT_GEN_EID_IN_UNWANTED_TRACKING_MODE);

    }
    return s_unwanted_tracking_mode_enable;
}

static uint32_t app_fast_pair_spot_get_adv_data(multi_ble_adv_info_t *adv_data)
{
    uint16_t interval;

#if 0
    if (app_bt_connection_service_get_current_status()->bt_visible) {
        interval = APP_FAST_PAIR_DISCOVER_MODE_ADV_INTERVAL;
    } else {
        interval = APP_FAST_PAIR_NON_DISCOVER_MODE_ADV_INTERVAL;
    }
#else
    interval = APP_FAST_PAIR_DISCOVER_MODE_ADV_INTERVAL;
#endif
    app_fast_pair_set_adv_param(adv_data, interval);
    if (adv_data->adv_param != NULL) {
        adv_data->adv_param->advertising_event_properties = BT_HCI_ADV_EVT_PROPERTIES_MASK_CONNECTABLE;
    }

    if (adv_data->adv_data != NULL) {
        uint8_t *buf = adv_data->adv_data->data;
        uint16_t *len = &adv_data->adv_data->data_length;

        bool ret = bt_fast_pair_get_spot_adv_data(false, buf, len);
        if (!ret) {
            adv_data->adv_data->data_length = 0;
        }

        APPS_LOG_MSGID_I(LOG_TAG"[SPOT] get adv data, data_length=%d eid=%02X:%02X:%02X:%02X:%02X",
                         6, adv_data->adv_data->data_length, buf[8], buf[9], buf[10], buf[11], buf[12]);
    }
    if (adv_data->scan_rsp) {
        adv_data->scan_rsp->data_length = 0;
    }

    return 0;
}

static void app_fast_pair_spot_adv_callback(bt_fast_pair_spot_adv_event event)
{
    // Need to switch to APP task
    ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_BT_FAST_PAIR,
                        event, NULL, 0, NULL, 0);
}

static void app_fast_pair_spot_enable_advertising(bool enable)
{
    APPS_LOG_MSGID_I(LOG_TAG"[SPOT] enable_advertising, enable=%d", 1, enable);
    s_spot_in_adv = enable;
    if (enable) {
        multi_ble_adv_manager_remove_ble_adv(MULTI_ADV_INSTANCE_FAST_PAIR, app_fast_pair_spot_get_adv_data);
        multi_ble_adv_manager_add_ble_adv(MULTI_ADV_INSTANCE_FAST_PAIR, app_fast_pair_spot_get_adv_data, 1);
        multi_ble_adv_manager_notify_ble_adv_data_changed(MULTI_ADV_INSTANCE_FAST_PAIR);
    } else {
        multi_ble_adv_manager_remove_ble_adv(MULTI_ADV_INSTANCE_FAST_PAIR, app_fast_pair_spot_get_adv_data);
        multi_ble_adv_manager_notify_ble_adv_data_changed(MULTI_ADV_INSTANCE_FAST_PAIR);
#ifdef MTK_AWS_MCE_ENABLE
        if (bt_connection_manager_device_local_info_get_aws_role() != BT_AWS_MCE_ROLE_AGENT) {
            return;
        }
        apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_BT_FAST_PAIR, BT_FAST_PAIR_SPOT_EVENT_STOP_ADV, NULL, 0);
#endif
    }
}

static uint32_t app_fast_pair_spot_gen_rotate_time_ms(uint32_t rotate_interval_ms)
{
     typedef struct {
        mbedtls_entropy_context entropy;
        mbedtls_ctr_drbg_context ctr_drbg;
    } _random_ctx_t;

    static uint8_t s_nonce[8] = {0};
    const uint8_t *pers = (const uint8_t *)"s_p_o_t";
    _random_ctx_t *ctx = (_random_ctx_t *)pvPortMalloc(sizeof(_random_ctx_t));
    if (ctx == NULL) {
        return rotate_interval_ms;
    }
    mbedtls_entropy_init(&ctx->entropy);
    mbedtls_ctr_drbg_init(&ctx->ctr_drbg);

    mbedtls_ctr_drbg_seed(&ctx->ctr_drbg, mbedtls_entropy_func, &ctx->entropy,
                          (const unsigned char *) pers, strlen((const char *)pers));
    mbedtls_ctr_drbg_random(&ctx->ctr_drbg, &s_nonce[0], 8);

    mbedtls_ctr_drbg_free(&ctx->ctr_drbg);
    mbedtls_entropy_free(&ctx->entropy);
    vPortFree(ctx);

    uint32_t cur_eid_counter = bt_fast_pair_spot_get_eid_clock_callback();
    uint32_t rotate_time = rotate_interval_ms / 1000;
    rotate_time += (s_nonce[0] % FAST_PAIR_RANDOM_WINDOW);
    APPS_LOG_MSGID_I(LOG_TAG" random value 0=%d", 1, s_nonce[0]);
    rotate_time -= FAST_PAIR_RANDOM_WINDOW / 2; /* rotate_time >= 1024, it's save to -= FAST_PAIR_RANDOM_WINDOW / 2 */
    rotate_time += cur_eid_counter;
    for (uint32_t i = 0; i < FAST_PAIR_SPOT_ROTATE_TIME_MS / 1000 / FAST_PAIR_RANDOM_WINDOW; i++) {
        APPS_LOG_MSGID_I(LOG_TAG" rotate time1=%d, time2=%d", 2, rotate_time, cur_eid_counter);
        if ((rotate_time & 0xFFFFFC00) <= (cur_eid_counter & 0xFFFFFC00)) {
            rotate_time += FAST_PAIR_RANDOM_WINDOW;
        } else {
            break;
        }
    }

    rotate_time -= cur_eid_counter;
    return rotate_time * 1000;
}

void __app_async_gen_task(void *arg)
{
    uint16_t len = 64;
    bt_fast_pair_get_spot_adv_data(true, temp_buf, &len);
    uint32_t rotate_time = s_unwanted_tracking_mode_enable ? FAST_PAIR_SPOT_UNWANTED_MODE_ROTATE_TIME_MS : FAST_PAIR_SPOT_ROTATE_TIME_MS;
    app_fast_pair_generate_random_address(app_fast_pair_spot_gen_rotate_time_ms(rotate_time));
    app_fast_pair_spot_enable_advertising(true);
    app_fast_pair_trigger_advertising(&g_app_fast_pair_context);
}

void app_fast_pair_gen_eid_and_trigger_adv_async()
{
    system_daemon_task_invoke(__app_async_gen_task, NULL);
}

void __app_async_gen_eid(void *ary)
{
    uint16_t len = 64;
    bt_fast_pair_get_spot_adv_data(true, temp_buf, &len);
    app_fast_pair_spot_enable_advertising(true);
}

void app_fast_pair_gen_eid()
{
    system_daemon_task_invoke(__app_async_gen_eid, NULL);
}

#include "hal_platform.h"
#include "hal_rtc.h"
uint32_t bt_fast_pair_spot_get_eid_clock_callback()
{
    /* Notice!!!
     * The customer should implement this function! If your implement support RTC timer, return the RTC timer
     * with seconds. And your RTC timer must support power loss recover. It's means you should backup the
     * RTC timer to the NVDM every a few hours. If your device power loss and the RTC timer cleared, you should
     * use the value in the NVDM to recover it.
     * If you implement it by the system clock, you should keep the clock is count-up. It's means that you
     * should add the system clock to a base counter, you could set the base counter when the product leaves
     * the factory. And storage the sum of base counter and current system clock to the NVDM every time the
     * system power off and read it as the new base counter when the system boot up again.
     */
#if 0
    static uint32_t last_reboot_value = 0;
    static uint32_t last_value = 0;
    if (last_reboot_value == 0) {
        uint32_t size = sizeof(uint32_t);
        nvkey_status_t sta = nvkey_read_data(NVID_APP_FPS_SPOT_EID_COUNTER, (uint8_t *)&last_reboot_value, &size);
        if (sta != NVKEY_STATUS_OK) {
            last_reboot_value = 1; /* to void reading again. */
        }
        last_value = last_reboot_value;
    }

    uint32_t new_value = last_reboot_value + xTaskGetTickCount() / portTICK_PERIOD_MS / 1000;
    /* update every 5hours */
    if (new_value - last_value >= 60 * 60 * 5) {
        nvkey_write_data(NVID_APP_FPS_SPOT_EID_COUNTER, (uint8_t *)&new_value, sizeof(uint32_t));
        last_value = new_value;
    }
#else
    static uint32_t last_record_value = 0;
    hal_rtc_time_t time = {0};
    static hal_rtc_time_t time1;

    /* Notice! check and revover for power lossing. If your design already implement it, please remove follow logic. */
    if (last_record_value == 0) {
        uint32_t size = sizeof(hal_rtc_time_t);
        hal_rtc_get_time(&time);
        APPS_LOG_MSGID_I(LOG_TAG"hal rtc time check recover, y=%d,m=%d,d=%d,h=%d,m=%d", 5,
                    time.rtc_year, time.rtc_mon, time.rtc_day, time.rtc_hour, time.rtc_min);
        if (nvkey_read_data(NVID_APP_FPS_SPOT_EID_COUNTER, (uint8_t *)&time1, &size) == NVKEY_STATUS_OK) {
            APPS_LOG_MSGID_I(LOG_TAG"hal rtc recorded time, y=%d,m=%d,d=%d,h=%d,m=%d", 5,
                    time1.rtc_year, time1.rtc_mon, time1.rtc_day, time1.rtc_hour, time1.rtc_min);
            if (time.rtc_year <= time1.rtc_year
                && time.rtc_mon <= time1.rtc_mon
                && time.rtc_day <= time1.rtc_day
                && time.rtc_hour <= time1.rtc_hour
                && time.rtc_min <= time1.rtc_min) {
                 APPS_LOG_MSGID_I(LOG_TAG"hal rtc set time=%d.", 1, hal_rtc_set_time(&time1));
            }
        } else {
            //APPS_LOG_MSGID_I(LOG_TAG"hal rtc save firt counter.", 0);
            nvkey_write_data(NVID_APP_FPS_SPOT_EID_COUNTER, (uint8_t *)&time, sizeof(hal_rtc_time_t));
        }
    }

    if (hal_rtc_get_time(&time) != HAL_RTC_STATUS_OK) {
        APPS_LOG_MSGID_I(LOG_TAG"hal rtc get time fail", 0);
        memcpy(&time, &time1, sizeof(hal_rtc_time_t));
    }

    uint32_t new_value = time.rtc_year * (365 * 24 * 60 * 60);// 1 days diff is acceptable.
    new_value += time.rtc_mon * (30 * 24 * 60 * 60); // 1 days diff and fixed 1month diff is acceptable
    new_value += time.rtc_day * (24 * 60 * 60);
    new_value += time.rtc_hour * (60 * 60);
    new_value += time.rtc_min *  60;
    new_value += time.rtc_sec;
    APPS_LOG_MSGID_I(LOG_TAG"bt_fast_pair_spot_get_eid_clock_callback time=%d, y=%d,m=%d,d=%d,h=%d,m=%d", 6,
            new_value, time.rtc_year, time.rtc_mon, time.rtc_day, time.rtc_hour, time.rtc_min);
    /* update every 5hours */
    if (new_value - last_record_value >= 60 * 60 * 5) {
        nvkey_write_data(NVID_APP_FPS_SPOT_EID_COUNTER, (uint8_t *)&time, sizeof(hal_rtc_time_t));
        last_record_value = new_value;
    }
#endif

    return new_value;
}


static void app_fast_pair_spot_find_me(bool l_ring, bool r_ring, uint32_t ring_time)
{
    APPS_LOG_MSGID_I(LOG_TAG" app_fast_pair_spot_find_me R:%d, L:%d, duration:%d",
                     3, r_ring, l_ring, ring_time);
#ifdef RACE_FIND_ME_ENABLE
    app_find_me_param_struct *find_self_param = (app_find_me_param_struct *)pvPortMalloc(sizeof(app_find_me_param_struct));
    if (find_self_param == NULL) {
        return;
    }
    find_self_param->blink = 0;
    find_self_param->duration_seconds = ring_time;
#ifdef AIR_TWS_ENABLE
    app_find_me_param_struct peer_param = {
        .blink = 0,
        .duration_seconds = ring_time
    };
    /* Because in the fast pair spec, the component bit define of ring request is different from the bit define of battery value,
       must convert the left and right. */
    uint8_t battery_item = (AUDIO_CHANNEL_R == ami_get_audio_channel() ? APP_FAST_PAIR_COMPONENT_RIGHT : APP_FAST_PAIR_COMPONENT_LEFT);
    if (APP_FAST_PAIR_COMPONENT_RIGHT == battery_item) {
        find_self_param->tone = l_ring;
        peer_param.tone = r_ring;
    } else {
        find_self_param->tone = r_ring;
        peer_param.tone = l_ring;
    }

    apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                   APPS_EVENTS_INTERACTION_FIND_ME_APP_SYNC_FIND_ME_ACTION, &peer_param, sizeof(peer_param));
#else
    find_self_param->tone = l_ring;
#endif
    ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_FINDME,
                        APP_FIND_ME_EVENT_ID_TRIGGER, find_self_param, sizeof(app_find_me_param_struct), NULL, 0);
#endif
}

static void app_fast_spot_stop_find_me()
{
#ifdef RACE_FIND_ME_ENABLE
    app_find_me_param_struct *find_self_param = (app_find_me_param_struct *)pvPortMalloc(sizeof(app_find_me_param_struct));
    if (find_self_param) {
        find_self_param->blink = 0;
        find_self_param->duration_seconds = 0;
        find_self_param->tone = 0;
#ifdef MTK_AWS_MCE_ENABLE
        apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                       APPS_EVENTS_INTERACTION_FIND_ME_APP_SYNC_FIND_ME_ACTION, find_self_param, sizeof(app_find_me_param_struct));
#endif
        ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_FINDME,
                            APP_FIND_ME_EVENT_ID_TRIGGER, find_self_param, sizeof(app_find_me_param_struct), NULL, 0);
    }
#else
    APPS_LOG_MSGID_E(LOG_TAG"FIND ME NOT ENABLED!", 0);
#endif
}

static void app_fast_pair_ring_request_handle(bt_fast_pair_ring_request_t *ring);
/* !Notice: this function just for reference, please implement it by your ring design. */
static bt_fast_pair_spot_ring_type_t s_current_ring_type = BT_FAST_PAIR_SPOT_RING_TYPE_NONE;
static uint16_t s_remain_time = 0;
bt_fast_pair_status_t bt_fast_pair_spot_ring_callback(bt_fast_pair_spot_ring_req_type_t req_type, uint8_t *data, uint16_t *remain_time)
{
    APPS_LOG_MSGID_I(LOG_TAG"[SPOT] ring req, %d, data: %d, remain_time: %d", 3, req_type, *data, *remain_time);
    switch (req_type) {
        case BT_FAST_PAIR_SPOT_RING_REQ_RING:
            /* Ring req, the *data is ring type */
            s_current_ring_type = *data;
            /* Notice! the time is deciseconds. */
            s_remain_time = ((*remain_time << 8) & 0xFF00) | ((*remain_time >> 8) & 0xFF);
            if (s_current_ring_type == BT_FAST_PAIR_SPOT_RING_TYPE_NONE) {
                /* BT_FAST_PAIR_SPOT_RING_TYPE_NONE means stop ring. */
                s_remain_time = 0;
                /* Every time ringing stops, must call bt_fast_pair_spot_ring_state_update to udpate state.
                 * Because the ringing is stopped by SPOT req, so the state should be BT_FAST_PAIR_SPOT_RING_STOPPED_BY_GATT.
                 * If the ringing is stopped by other reasons, must call this api too.
                 */
                bt_fast_pair_spot_ring_state_update(BT_FAST_PAIR_SPOT_RING_STOPPED_BY_GATT, s_current_ring_type, 0);
                app_fast_spot_stop_find_me();
            } else {
#ifdef AIR_RACE_FIND_ME_ENABLE
                ui_shell_remove_event(EVENT_GROUP_UI_SHELL_FINDME, APP_FIND_ME_EVENT_ID_TRIGGER);
#endif

                ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                    APPS_EVENTS_INTERACTION_SPOT_RING_OVER, NULL, 0, NULL, s_remain_time / 10 * 1000);
                app_fast_pair_spot_find_me(true, true, s_remain_time / 10);
            /* Every time ringing starts, must call bt_fast_pair_spot_ring_state_update to udpate state. */
            bt_fast_pair_spot_ring_state_update(BT_FAST_PAIR_SPOT_RING_STA_STARTED, s_current_ring_type, ((s_remain_time << 8) & 0xFF00) | ((s_remain_time >> 8) & 0xFF));
            }
            break;
        case BT_FAST_PAIR_SPOT_RING_REQ_GET_RING_CAPABLE:
            /* Get the ring capable. */
            *data = BT_FAST_PAIR_SPOT_RING_CAPABLE_SINGLE;
            break;
        case BT_FAST_PAIR_SPOT_RING_REQ_GET_RING_TYPE:
            /* Get the current ring type and remain time, if current not in ringing, the s_remain_time must be zero. */
            *data = s_current_ring_type;
            *remain_time = ((s_remain_time << 8) & 0xFF00) | ((s_remain_time >> 8) & 0xFF);
            break;
    }
    return BT_FAST_PAIR_STATUS_SUCCESS;
}
#endif

/**
* @brief      This function update the battery information.
* @param[in]  context, the context of the app.
* @param[in]  battery, the new battery information.
*/
static void app_fast_pair_update_battery(app_fast_pair_local_context_t *context, bt_fast_pair_battery_t *battery)
{
    if (NULL == context || NULL == battery) {
        return;
    }
    bool need_update = false;
    uint8_t lowest_battery_value = 0xFF; /* The device which is not charging and battery level is lowest. */
    bt_fast_pair_battery_t *cur_cntx = &(context->battery);

    /* When data changed */
    if (cur_cntx->ui_show != battery->ui_show ||
        cur_cntx->battery[APP_FAST_PAIR_COMPONENT_RIGHT].charging != battery->battery[APP_FAST_PAIR_COMPONENT_RIGHT].charging ||
        cur_cntx->battery[APP_FAST_PAIR_COMPONENT_LEFT].charging != battery->battery[APP_FAST_PAIR_COMPONENT_LEFT].charging ||
        cur_cntx->battery[APP_FAST_PAIR_COMPONENT_CASE].charging != battery->battery[APP_FAST_PAIR_COMPONENT_CASE].charging ||
        cur_cntx->battery[APP_FAST_PAIR_COMPONENT_RIGHT].battery_value != battery->battery[APP_FAST_PAIR_COMPONENT_RIGHT].battery_value ||
        cur_cntx->battery[APP_FAST_PAIR_COMPONENT_LEFT].battery_value != battery->battery[APP_FAST_PAIR_COMPONENT_LEFT].battery_value ||
        cur_cntx->battery[APP_FAST_PAIR_COMPONENT_CASE].battery_value != battery->battery[APP_FAST_PAIR_COMPONENT_CASE].battery_value) {
        need_update = true;
        if (0 == battery->battery[APP_FAST_PAIR_COMPONENT_RIGHT].charging) {
            lowest_battery_value = battery->battery[APP_FAST_PAIR_COMPONENT_RIGHT].battery_value;
        }
        if (0 == battery->battery[APP_FAST_PAIR_COMPONENT_LEFT].charging && lowest_battery_value > battery->battery[APP_FAST_PAIR_COMPONENT_LEFT].battery_value) {
            lowest_battery_value = battery->battery[APP_FAST_PAIR_COMPONENT_LEFT].battery_value;
        }
        if (lowest_battery_value != 0xFF) {
            battery->remaining_time = REMAINING_BATTERY_TIME_WHEN_FULL * lowest_battery_value / 100;
        } else {
            battery->remaining_time = BT_FAST_PAIR_REMAINING_TIME_INVALID_VALUE;
        }
    }
    if (true == need_update) {
        context->battery = *battery;
        /* Call middleware API to update battery information. */
        bt_fast_pair_update_battery(cur_cntx);
        /* Because the battery information is contained in adv data, must update adv once. */
        app_fast_pair_trigger_advertising(context);
    }
}

#ifdef RACE_FIND_ME_ENABLE
static void app_fast_pair_notify_ring_state_change(void)
{
    uint8_t message_buffer[sizeof(bt_fast_pair_message_stream_t)];
    bt_fast_pair_message_stream_t *cont = (bt_fast_pair_message_stream_t *)&message_buffer;
    cont->group_ID = BT_FAST_PAIR_MESSAGE_GROUP_DEVICE_ACTION_EVENT;
    cont->code_ID = BT_FAST_PAIR_MESSAGE_CODE_DEVICE_ACTION_EVENT_RING;
    cont->data_length = 0x01;
    cont->data[0] = 0;
#ifdef AIR_TWS_ENABLE
    bt_aws_mce_role_t aws_role = bt_connection_manager_device_local_info_get_aws_role();
    if ((BT_AWS_MCE_ROLE_PARTNER | BT_AWS_MCE_ROLE_CLINET) & aws_role) {
        return;
    }
    if (AUDIO_CHANNEL_R == ami_get_audio_channel()) {
        cont->data[0] = (s_app_fast_pair_find_me_state.tone & 0x1) | ((s_app_fast_pair_find_me_state.peer_tone & 0x1) << 1);
    } else {
        cont->data[0] = (s_app_fast_pair_find_me_state.peer_tone & 0x1) | ((s_app_fast_pair_find_me_state.tone & 0x1) << 1);
    }
#else
    cont->data[0] = s_app_fast_pair_find_me_state.tone & 0x1;
#endif
    APPS_LOG_MSGID_I(LOG_TAG" Ringing state change : %x.", 1, cont->data[0]);
    bt_fast_pair_send_message_stream(NULL, cont);
}
#endif

#ifdef AIR_SMART_CHARGER_ENABLE
/**
* @brief      This function send event to stop find me.
*/
static void app_fast_pair_stop_find_me(void)
{
#ifdef RACE_FIND_ME_ENABLE
    app_find_me_param_struct *find_self_param = (app_find_me_param_struct *)pvPortMalloc(sizeof(app_find_me_param_struct));
    if (find_self_param) {
        find_self_param->blink = 0;
        find_self_param->duration_seconds = 0;
        find_self_param->tone = 0;
        ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_FINDME,
                            APP_FIND_ME_EVENT_ID_TRIGGER, find_self_param, sizeof(app_find_me_param_struct), NULL, 0);
    }
#endif
}
#endif

/**
* @brief      This function process the fast pair ring request from smart phone.
* @param[in]  ring, the structure of ring request.
*/
static void app_fast_pair_ring_request_handle(bt_fast_pair_ring_request_t *ring)
{
    APPS_LOG_MSGID_I(LOG_TAG" Received ring request from remote device R:%d, L:%d, duration:%d",
                     3, ring->ring_status[0], ring->ring_status[1], ring->ring_duration);
#ifdef RACE_FIND_ME_ENABLE
    app_find_me_param_struct *find_self_param = (app_find_me_param_struct *)pvPortMalloc(sizeof(app_find_me_param_struct));
    if (ring->ring_duration == 0xFF) {
        ring->ring_duration = 0;
    }
    if (NULL == find_self_param) {
        return;
    }
    find_self_param->blink = 0;
    find_self_param->duration_seconds = ring->ring_duration;
#ifdef AIR_TWS_ENABLE
    app_find_me_param_struct peer_param = {
        .blink = 0,
        .duration_seconds = ring->ring_duration
    };
    /* Because in the fast pair spec, the component bit define of ring request is different from the bit define of battery value,
       must convert the left and right. */
    uint8_t battery_item = (AUDIO_CHANNEL_R == ami_get_audio_channel() ? APP_FAST_PAIR_COMPONENT_RIGHT : APP_FAST_PAIR_COMPONENT_LEFT);
    if (APP_FAST_PAIR_COMPONENT_RIGHT == battery_item) {
        find_self_param->tone = ring->ring_status[APP_FAST_PAIR_COMPONENT_LEFT];
        peer_param.tone = ring->ring_status[APP_FAST_PAIR_COMPONENT_RIGHT];
    } else {
        find_self_param->tone = ring->ring_status[APP_FAST_PAIR_COMPONENT_RIGHT];
        peer_param.tone = ring->ring_status[APP_FAST_PAIR_COMPONENT_LEFT];
    }
    s_app_fast_pair_find_me_state.peer_tone = peer_param.tone;
    apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_BT_FAST_PAIR,
                                   BT_FAST_PAIR_APP_EVENT_RING_REQUEST, &peer_param, sizeof(peer_param));
#else
    find_self_param->tone = ring->ring_status[0];
#endif
    s_app_fast_pair_find_me_state.tone = find_self_param->tone;
    ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_FINDME,
                        APP_FIND_ME_EVENT_ID_TRIGGER, find_self_param, sizeof(app_find_me_param_struct), NULL, 0);
#endif
}

/**
* @brief      This function process additional data from smart phone.
* @param[in]  local_context, context, the context of the app.
* @param[in]  data, the content of the additional data.
*/
static void app_fast_pair_additional_data_handle(app_fast_pair_local_context_t *local_context, bt_fast_pair_additional_data_t *data, uint32_t data_len)
{
    nvkey_status_t status = 0;
    APPS_LOG_MSGID_I(LOG_TAG" Additional data received data_id %d, data_length %d.", 2, data->data_id, data->data_length);
    /* If the additional data is changing personalized name. */
    if (BT_FAST_PAIR_ADDITIONAL_DATA_ID_PERSONALIZED_NAME == data->data_id) {
        if (data->data_length > sizeof(g_app_fast_pair_context.nvkey.personalized_name) - 1) {
            data->data_length = sizeof(g_app_fast_pair_context.nvkey.personalized_name) - 1;
        }
        memcpy(g_app_fast_pair_context.nvkey.personalized_name, data->data, data->data_length);
        g_app_fast_pair_context.nvkey.personalized_name[data->data_length] = 0;
        if (NVKEY_STATUS_OK != (status = nvkey_write_data(NVID_APP_FAST_PAIR_PERSONALIZED_NAME,
                                                          (uint8_t *) & (g_app_fast_pair_context.nvkey.personalized_name),
                                                          data->data_length + 1))) {
        }
    }
#ifdef AIR_TWS_ENABLE
    apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_BT_FAST_PAIR,
                                   BT_FAST_PAIR_APP_EVENT_ADDITIONAL_DATA, data, data_len);
#endif
}

static void app_fast_pair_update_parameter(void)
{
    bt_fast_pair_update_parameters_t params;
    memset(&params, 0, sizeof(bt_fast_pair_update_parameters_t));
    if (0 != g_app_fast_pair_context.nvkey.tx_power_available) {
        params.tx_power_level = &(g_app_fast_pair_context.nvkey.tx_power_level);
    }
    params.addr_type = BT_ADDR_RANDOM_IDENTITY;
    params.personalized_name = g_app_fast_pair_context.nvkey.personalized_name;
#ifdef AIR_BT_FAST_PAIR_LE_AUDIO_ENABLE
    if (app_lea_service_is_enabled_lea()) {
        params.le_support = true;
    }
#endif

    bt_fast_pair_update_parameters(&params);
}

#ifdef AIR_BT_FAST_PAIR_SASS_ENABLE
static bool app_fast_pair_app_event_proc_sass_msg(bt_bd_addr_t *addr, bt_fast_pair_message_stream_t *msg_data, bool from_fp_msg)
{
    if (msg_data == NULL) {
        return true;
    }
    bool need_send_ack = false;
    bt_fast_pair_acknowledgements_t ack = {
        .action = BT_FAST_PAIR_MESSAGE_CODE_ACKNOWLEDGEMENT_ACK,
        .groupID = BT_FAST_PAIR_MESSAGE_GROUP_SMART_AUDIO_SOURCE_SWITCHING,
        .codeID = msg_data->code_ID,
    };
#ifdef AIR_MULTI_POINT_ENABLE
    bt_device_manager_le_bonded_info_t *bond_info = NULL;
    if (BT_FAST_PAIR_MESSAGE_CODE_SASS_SWITCH_ACTIVE_AUDIO_SOURCE == msg_data->code_ID || BT_FAST_PAIR_MESSAGE_CODE_SASS_SWITCH_BACK == msg_data->code_ID) {
        bond_info = bt_device_manager_le_get_bonding_info_by_addr_ext(addr);
    }
#endif
    switch (msg_data->code_ID) {
        case BT_FAST_PAIR_MESSAGE_CODE_SASS_GET_CAPABILITY: {
            /* Processed by middleware. */
            break;
        }
        case BT_FAST_PAIR_MESSAGE_CODE_SASS_NOTIFY_CAPABILITY: {
            /* No use. */
            break;
        }
#ifdef AIR_MULTI_POINT_ENABLE
        case BT_FAST_PAIR_MESSAGE_CODE_SASS_SET_MULTIPOINT_STATE: {
            bt_fast_pair_sass_set_multipoint_state_data_t *multi_point = (bt_fast_pair_sass_set_multipoint_state_data_t *) & (msg_data->data);
            app_bt_emp_enable(multi_point->switch_on, TRUE);
            break;
        }
#endif
        case BT_FAST_PAIR_MESSAGE_CODE_SASS_SET_SWITCHING_PREFERENCE: {
            bt_fast_pair_sass_set_switching_preference_data_t *set_preference = (bt_fast_pair_sass_set_switching_preference_data_t *) & (msg_data->data);
            g_app_fast_pair_context.sass_switch_prefer_flags = set_preference->switching_preference_flags;
            bt_fast_pair_sass_notify_switching_preference_data_t notify_preference = {
                .switching_preference_flags = set_preference->switching_preference_flags,
                .advanced_switching_settings = set_preference->advanced_switching_settings,
            };
            bt_fast_pair_sass_update_multipoint_switching_preference(&notify_preference, false);
            break;
        }
        case BT_FAST_PAIR_MESSAGE_CODE_SASS_GET_SWITCHING_PREFERENCE: {
            /* Processed by middleware. */
            break;
        }
        case BT_FAST_PAIR_MESSAGE_CODE_SASS_NOTIFY_SWITCHING_PREFERENCE: {
            /* No use. */
            break;
        }
#ifdef AIR_MULTI_POINT_ENABLE
        case BT_FAST_PAIR_MESSAGE_CODE_SASS_SWITCH_ACTIVE_AUDIO_SOURCE: {
            if (addr == NULL) {
                break;
            }
            need_send_ack = true;
            bt_fast_pair_switch_active_audio_source_bit_t switch_audio_flags = *(bt_fast_pair_switch_active_audio_source_bit_t *) & (msg_data->data);
            APPS_LOG_MSGID_I(LOG_TAG" BT_FAST_PAIR_MESSAGE_CODE_SASS_SWITCH_ACTIVE_AUDIO_SOURCE flag 0x%X, addr:[%02X:%02X:%02X:%02X:%02X:%02X]", 7,
                             switch_audio_flags,
                             (*addr)[5], (*addr)[4], (*addr)[3], (*addr)[2], (*addr)[1], (*addr)[0]);
#if defined(AIR_BT_FAST_PAIR_LE_AUDIO_ENABLE)
            if (bond_info) {
                APPS_LOG_MSGID_I(LOG_TAG" BT_FAST_PAIR_MESSAGE_CODE_SASS_SWITCH_ACTIVE_AUDIO_SOURCE identity_addr:[%02X:%02X:%02X:%02X:%02X:%02X]", 6,
                                 bond_info->info.identity_addr.address.addr[5], bond_info->info.identity_addr.address.addr[4],
                                 bond_info->info.identity_addr.address.addr[3], bond_info->info.identity_addr.address.addr[2],
                                 bond_info->info.identity_addr.address.addr[1], bond_info->info.identity_addr.address.addr[0]);
            }
#if defined(AIR_TWS_ENABLE)
            if (bt_connection_manager_device_local_info_get_aws_role() & (BT_AWS_MCE_ROLE_PARTNER)
                && BT_AWS_MCE_SRV_LINK_NONE != bt_aws_mce_srv_get_link_type()) {
                if (bond_info) {
                    app_fast_pair_sass_aws_sync_ctrl_msg_t aws_msg = { 0 };
                    memcpy(&aws_msg.addr, &bond_info->info.identity_addr.address.addr, sizeof(aws_msg.addr));
                    memcpy(&aws_msg.msg_data, msg_data, sizeof(aws_msg.msg_data));
                    apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_BT_FAST_PAIR,
                                                   APP_FAST_PAIR_AWS_SYNC_SASS_CONTROL_MSG,
                                                   &aws_msg,
                                                   sizeof(aws_msg));
                }
                break;
            } else if (bt_connection_manager_device_local_info_get_aws_role() & (BT_AWS_MCE_ROLE_CLIENT)) {
                break;
            }
#endif
#endif
            const bt_device_manager_link_record_t *link_info = bt_device_manager_link_record_get_connected_link();
            bt_sink_srv_device_state_t device_state;
            bt_status_t bt_ret = BT_STATUS_FAIL;
#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
            bt_ret = bt_sink_srv_get_playing_device_state(&device_state);
#endif
            const bt_device_manager_link_record_item_t *switch_away_src_link = NULL;
            const bt_device_manager_link_record_item_t *the_other_src_link = NULL;
            const bt_device_manager_link_record_item_t *current_src_link = NULL;
            uint32_t i;

            for (i = 0; link_info && i < link_info->connected_num; i++) {
                if ((bond_info && BT_DEVICE_MANAGER_LINK_TYPE_LE == link_info->connected_device[i].link_type
                     && memcmp(link_info->connected_device[i].remote_addr, bond_info->info.identity_addr.address.addr, sizeof(bt_bd_addr_t)) == 0)
                    || (BT_DEVICE_MANAGER_LINK_TYPE_EDR == link_info->connected_device[i].link_type
                        && memcmp(link_info->connected_device[i].remote_addr, *addr, sizeof(bt_bd_addr_t)) == 0)) {
                    current_src_link = &link_info->connected_device[i];
                } else {
                    if (the_other_src_link == NULL) {
                        the_other_src_link = &link_info->connected_device[i];
                    }
                }
            }

            if ((1 << BT_FAST_PAIR_SASS_SWITCH_ACTIVE_AUDIO_SOURCE_SWITCH_TO_THIS_BIT) & switch_audio_flags) {
                APPS_LOG_MSGID_I(LOG_TAG" switch to this, means accept switch audio source, current_src_link = 0x%x", 1, current_src_link);
                memcpy(g_app_fast_pair_context.allow_switch_audio_addr, *addr, sizeof(bt_bd_addr_t));
#ifdef AIR_BT_FAST_PAIR_LE_AUDIO_ENABLE
                if (current_src_link && BT_DEVICE_MANAGER_LINK_TYPE_LE == current_src_link->link_type) {
                    bt_sink_srv_cap_am_lea_int_user_conf_set(true, addr);
                } else
#endif
                {
                    bt_sink_srv_a2dp_int_user_conf_set(1, *addr);
                }
                ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                      APPS_EVENTS_INTERACTION_FAST_PAIR_CLEAR_ACTIVE_SWITCH_RECORD);
                ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE,
                                    EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                    APPS_EVENTS_INTERACTION_FAST_PAIR_CLEAR_ACTIVE_SWITCH_RECORD,
                                    NULL, 0, NULL, 1000);

                //break;
            }
            if ((1 << BT_FAST_PAIR_SASS_SWITCH_ACTIVE_AUDIO_SOURCE_SWITCH_TO_THIS_BIT) & switch_audio_flags) {
                switch_away_src_link = the_other_src_link;
            } else {
                switch_away_src_link = current_src_link;
            }
            bt_sink_srv_state_manager_played_device_t played_list[2];
            uint32_t played_list_count = 2;
            played_list_count = bt_sink_srv_state_manager_get_played_device_list(played_list, played_list_count);
            bt_sink_srv_state_manager_played_device_t *last_played = NULL;
            for (i = 0; switch_away_src_link && i < played_list_count; i++) {
                if (memcmp(played_list[i].address, switch_away_src_link->remote_addr, sizeof(played_list[i].address)) != 0) {
                    last_played = &played_list[i];
                    break;
                }
            }

            if (last_played == NULL || switch_away_src_link == NULL) {
                APPS_LOG_MSGID_I(LOG_TAG" Address is NULL: 0x%x, 0x%x", 2, last_played, switch_away_src_link);
                break;
            }

            if (BT_STATUS_SUCCESS == bt_ret) {
                if (memcmp(last_played->address, device_state.address, sizeof(bt_bd_addr_t)) == 0) {
                    /* Already playing. */
                    APPS_LOG_MSGID_I(LOG_TAG" already playing on addr [%02X:%02X:%02X:%02X:%02X:%02X]", 6,
                                     (last_played->address)[5], (last_played->address)[4],
                                     (last_played->address)[3], (last_played->address)[2],
                                     (last_played->address)[1], (last_played->address)[0]);
                    ack.action = BT_FAST_PAIR_MESSAGE_CODE_ACKNOWLEDGEMENT_NAK;
                    ack.reason = BT_FAST_PAIR_ACKNOWLEDGEMENT_NAK_REASON_REDUNDANT_DEVICE_ACTION;
                } else {
                    if ((1 << BT_FAST_PAIR_SASS_SWITCH_ACTIVE_AUDIO_SOURCE_REJECT_SCO) & switch_audio_flags) {
                        if (BT_SINK_SRV_STATE_NONE != device_state.call_state) {
                            bt_sink_srv_send_action(BT_SINK_SRV_ACTION_SWITCH_AUDIO_PATH, (void *)&switch_away_src_link->remote_addr);
                        }
                    }
                    if ((1 << BT_FAST_PAIR_SASS_SWITCH_ACTIVE_AUDIO_SOURCE_RESUME_PLAYING_BIT) & switch_audio_flags) {
                        memcpy(g_app_fast_pair_context.allow_switch_audio_addr, last_played->address, sizeof(bt_bd_addr_t));
                        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                              APPS_EVENTS_INTERACTION_FAST_PAIR_CLEAR_ACTIVE_SWITCH_RECORD);
                        ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE,
                                            EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                            APPS_EVENTS_INTERACTION_FAST_PAIR_CLEAR_ACTIVE_SWITCH_RECORD,
                                            NULL, 0, NULL, 1000);
                        bt_sink_srv_send_action(BT_SINK_SRV_ACTION_PLAY, (void *)&last_played->address);
                    }
                }
            }
            APPS_LOG_MSGID_I(LOG_TAG" switch to play on addr [%02X:%02X:%02X:%02X:%02X:%02X]", 6,
                             (last_played->address)[5], (last_played->address)[4],
                             (last_played->address)[3], (last_played->address)[2],
                             (last_played->address)[1], (last_played->address)[0]);

            if ((1 << BT_FAST_PAIR_SASS_SWITCH_ACTIVE_AUDIO_SOURCE_DISCONNECT) & switch_audio_flags) {
                APPS_LOG_MSGID_I(LOG_TAG" disconnect addr [%02X:%02X:%02X:%02X:%02X:%02X]", 6,
                                 (switch_away_src_link->remote_addr)[5], (switch_away_src_link->remote_addr)[4],
                                 (switch_away_src_link->remote_addr)[3], (switch_away_src_link->remote_addr)[2],
                                 (switch_away_src_link->remote_addr)[1], (switch_away_src_link->remote_addr)[0]);
#ifdef AIR_BT_FAST_PAIR_LE_AUDIO_ENABLE
                if (switch_away_src_link && BT_DEVICE_MANAGER_LINK_TYPE_LE == switch_away_src_link->link_type) {
                    app_lea_service_disconnect(true, APP_LE_AUDIO_DISCONNECT_MODE_DISCONNECT, switch_away_src_link->remote_addr, BT_HCI_STATUS_CONNECTION_TERMINATED_BY_LOCAL_HOST);
                } else
#endif
                {
                    bt_cm_connect_t connect_param = {
                        .profile = BT_CM_PROFILE_SERVICE_MASK_ALL,
                    };
                    memcpy(connect_param.address, switch_away_src_link->remote_addr, sizeof(bt_bd_addr_t));
                    bt_cm_disconnect(&connect_param);
                }
            }
            break;
        }
#endif
        case BT_FAST_PAIR_MESSAGE_CODE_SASS_SWITCH_BACK: {
            uint8_t switch_back_event = msg_data->data[0];
            app_bt_takeover_device_t takeover_device = {0};
            bt_cm_connect_t connect_param;
#ifdef AIR_BT_FAST_PAIR_LE_AUDIO_ENABLE
#ifdef AIR_TWS_ENABLE
            if (bt_connection_manager_device_local_info_get_aws_role() & (BT_AWS_MCE_ROLE_PARTNER)
                && BT_AWS_MCE_SRV_LINK_NONE != bt_aws_mce_srv_get_link_type()) {
                app_fast_pair_sass_aws_sync_ctrl_msg_t aws_msg = { 0 };
                memcpy(&aws_msg.addr, addr, sizeof(aws_msg.addr));
                memcpy(&aws_msg.msg_data, msg_data, sizeof(aws_msg.msg_data));
                apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_BT_FAST_PAIR,
                                               APP_FAST_PAIR_AWS_SYNC_SASS_CONTROL_MSG,
                                               &aws_msg,
                                               sizeof(aws_msg));
                break;
            } else if (bt_connection_manager_device_local_info_get_aws_role() & (BT_AWS_MCE_ROLE_CLIENT)) {
                break;
            }
#endif
            const bt_device_manager_link_record_item_t *current_link = NULL;
#endif
            uint32_t i = 0;
#ifdef AIR_MULTI_POINT_ENABLE
            bt_sink_srv_state_manager_played_device_t played_list[2];
            uint32_t played_list_count = 2;
            played_list_count = bt_sink_srv_state_manager_get_played_device_list(played_list, played_list_count);
            bt_sink_srv_state_manager_played_device_t *last_played = NULL;
            for (i = 0; i < played_list_count; i++) {
                if (memcmp(played_list[i].address, *addr, sizeof(played_list[i].address)) != 0) {
                    last_played = &played_list[i];
                    break;
                }
            }
#endif
            const bt_device_manager_link_record_t *link_info = bt_device_manager_link_record_get_connected_link();
            app_bt_takeover_service_get_last_takeover_device(&takeover_device);
            uint8_t *takeover_addr = takeover_device.addr;
            for (i = 0; link_info && i < link_info->connected_num; i++) {
                if (memcmp(link_info->connected_device[i].remote_addr, addr, sizeof(bt_bd_addr_t)) != 0) {

                } else {
#ifdef AIR_BT_FAST_PAIR_LE_AUDIO_ENABLE
                    current_link = &link_info->connected_device[i];
#endif
                }
            }
            APPS_LOG_MSGID_I(LOG_TAG" BT_FAST_PAIR_MESSAGE_CODE_SASS_SWITCH_BACK disconnected_record:[%02X:%02X:%02X:%02X:%02X:%02X].", 6,
                             takeover_addr[5], takeover_addr[4],
                             takeover_addr[3], takeover_addr[2],
                             takeover_addr[1], takeover_addr[0]);
            APPS_LOG_MSGID_I(LOG_TAG" BT_FAST_PAIR_MESSAGE_CODE_SASS_SWITCH_BACK current addr:[%02X:%02X:%02X:%02X:%02X:%02X].", 6,
                             (*addr)[5], (*addr)[4], (*addr)[3], (*addr)[2], (*addr)[1], (*addr)[0]);

#ifdef AIR_BT_FAST_PAIR_LE_AUDIO_ENABLE
            if (current_link == NULL) {
                break;
            }
            if (!(current_link->link_type == BT_DEVICE_MANAGER_LINK_TYPE_LE)) {
                app_lea_service_disconnect(true, APP_LE_AUDIO_DISCONNECT_MODE_DISCONNECT, current_link->remote_addr, BT_HCI_STATUS_CONNECTION_TERMINATED_BY_LOCAL_HOST);
            } else
#endif
            {
                connect_param.profile = BT_CM_PROFILE_SERVICE_MASK_ALL;
                memcpy(connect_param.address, *addr, sizeof(bt_bd_addr_t));
                bt_cm_disconnect(&connect_param);
            }
#ifdef AIR_MULTI_POINT_ENABLE
            if (last_played) {
                APPS_LOG_MSGID_I(LOG_TAG" BT_FAST_PAIR_MESSAGE_CODE_SASS_SWITCH_BACK another connected device:[%02X:%02X:%02X:%02X:%02X:%02X].", 6,
                                 last_played->address[5], last_played->address[4], last_played->address[3],
                                 last_played->address[2], last_played->address[1], last_played->address[0]);
                if (switch_back_event & BT_FAST_PAIR_SASS_SWITCH_BACK_EVENT_RESUME_PLAYING) {
                    memcpy(g_app_fast_pair_context.allow_switch_audio_addr, last_played->address, sizeof(bt_bd_addr_t));
                    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                          APPS_EVENTS_INTERACTION_FAST_PAIR_CLEAR_ACTIVE_SWITCH_RECORD);
                    ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE,
                                        EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                        APPS_EVENTS_INTERACTION_FAST_PAIR_CLEAR_ACTIVE_SWITCH_RECORD,
                                        NULL, 0, NULL, 1000);
                    bt_sink_srv_send_action(BT_SINK_SRV_ACTION_PLAY, (void *) & (last_played->address));
                }
            }
#endif
            if (0 != memcmp(takeover_addr, empty_addr, sizeof(bt_bd_addr_t))) {
#ifdef AIR_LE_AUDIO_ENABLE
                if (!takeover_device.is_edr) {
                    app_lea_service_start_reconnect_adv(takeover_addr);
                } else
#endif
                {
                    connect_param.profile = BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP) | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK);
                    memcpy(connect_param.address, takeover_addr, sizeof(bt_bd_addr_t));
                    bt_cm_connect(&connect_param);
                }
                g_app_fast_pair_context.connection_switch_back_event = switch_back_event;
            }
            break;
        }
        case BT_FAST_PAIR_MESSAGE_CODE_SASS_NOTIFY_MULTIPOINT_SWITCH: {
            /* Only can be from provider to seeker.*/
            break;
        }
        case BT_FAST_PAIR_MESSAGE_CODE_SASS_GET_CONNECTION_STATUS: {
            /* Processed by middleware. */
            break;
        }
        case BT_FAST_PAIR_MESSAGE_CODE_SASS_NOTIFY_CONNECTION_STATUS: {
            /* Only can be from provider to seeker.*/
            break;
        }
        case BT_FAST_PAIR_MESSAGE_CODE_SASS_NOTIFY_SASS_INITIATED_CONNECTION: {
            /* To-Do. Don't know usage. */
            break;
        }
        case BT_FAST_PAIR_MESSAGE_CODE_SASS_INDICATE_IN_USE_ACCOUNT_KEY: {
            /* Processed by middleware. */
            app_fast_pair_connection_state_change();
            break;
        }
        case BT_FAST_PAIR_MESSAGE_CODE_SASS_SEND_CUSTOM_DATA: {
            if (!app_bt_connection_service_get_current_status()->bt_visible) {
                app_fast_pair_trigger_advertising(&g_app_fast_pair_context);
            }
            break;
        }
        case BT_FAST_PAIR_MESSAGE_CODE_SASS_SET_DROP_CONNECTION_TARGET: {
            if (addr) {
                memcpy(g_app_fast_pair_context.drop_target, *addr, sizeof(bt_bd_addr_t));
            }
            break;
        }
        default:
            break;
    }

    if (need_send_ack && from_fp_msg) {
        if (ack.action == BT_FAST_PAIR_MESSAGE_CODE_ACKNOWLEDGEMENT_ACK) {
            bt_fast_pair_send_acknowledgements(addr, &ack, msg_data->data, msg_data->data_length - sizeof(bt_fast_pair_sass_mac_verify_data_t));
        } else {
            bt_fast_pair_send_acknowledgements(addr, &ack, NULL, 0);
        }
    }

    return true;
}
#endif

/**
* @brief      This function process the events of EVENT_GROUP_UI_SHELL_BT_FAST_PAIR group.
* @param[in]  self, the context pointer of the activity.
* @param[in]  event_id, the current event ID to be handled.
* @param[in]  extra_data, extra data pointer of the current event, NULL means there is no extra data.
* @param[in]  data_len, the length of the extra data. 0 means extra_data is NULL.
* @return     If return true, the current event cannot be handle by the next activity.
*/
static bool app_fast_pair_app_event_callback(struct _ui_shell_activity *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    assert(NULL != self);
    app_bt_event_fast_pair_callback_data_t *format_data = (app_bt_event_fast_pair_callback_data_t *)extra_data;
    APPS_LOG_MSGID_I(LOG_TAG" Fast pair app event id:0x%x, len:%d.", 2, event_id, data_len);
    switch (event_id) {
        case BT_FAST_PAIR_APP_EVENT_NEED_STORE_ACCOUNT_KEY: {
            bt_fast_pair_account_key_list_t *real_key_list = g_app_fast_pair_context.account_key_list;
            bt_fast_pair_account_key_list_t *key_list = (bt_fast_pair_account_key_list_t *)extra_data;
            assert(key_list == real_key_list);
            if (!app_bt_connection_service_get_current_status()->bt_visible) {
                app_fast_pair_trigger_advertising(&g_app_fast_pair_context);
            }
            app_fast_pair_store_account_key(key_list);
#ifdef AIR_TWS_ENABLE
            uint32_t size_account = sizeof(bt_fast_pair_account_key_list_t) +
                                    sizeof(bt_fast_pair_account_key_t) * (g_app_fast_pair_context.account_key_list->max_key_number - 1);
            apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_BT_FAST_PAIR,
                                           BT_FAST_PAIR_APP_EVENT_NEED_STORE_ACCOUNT_KEY, g_app_fast_pair_context.account_key_list, size_account);
#endif
            break;
        }

        case BT_FAST_PAIR_APP_EVENT_PAIR_COMPLETE: {
#if APP_ONLY_ALLOW_TAKE_OVER_BY_FAST_PAIR
            g_app_fast_pair_context.waiting_connect = false;
#endif
            break;
        }
        case BT_FAST_PAIR_APP_EVENT_SERVICE_CONNECTED: {
            break;
        }
        case BT_FAST_PAIR_APP_EVENT_SERVICE_DISCONNECTED: {
            break;
        }
        case BT_FAST_PAIR_APP_EVENT_RING_REQUEST: {
            if (NULL == format_data || data_len <= sizeof(app_bt_event_fast_pair_callback_data_t)) {
                break;
            }
            app_fast_pair_ring_request_handle((bt_fast_pair_ring_request_t *)format_data->data);
            break;
        }
        case BT_FAST_PAIR_APP_EVENT_MESSAGE_STREAM: {
            if (NULL != format_data && data_len >= sizeof(app_bt_event_fast_pair_callback_data_t)) {
                bt_fast_pair_message_stream_t *msg = (bt_fast_pair_message_stream_t *)(format_data->data);
                if (msg->group_ID == BT_FAST_PAIR_MESSAGE_GROUP_DEVICE_ACTION_EVENT
                    && msg->code_ID == BT_FAST_PAIR_MESSAGE_CODE_DEVICE_ACTION_EVENT_RING) {
                    bt_fast_pair_acknowledgements_t ack = {
                        .action = BT_FAST_PAIR_MESSAGE_CODE_ACKNOWLEDGEMENT_ACK,
                        .groupID = BT_FAST_PAIR_MESSAGE_GROUP_DEVICE_ACTION_EVENT,
                        .codeID = BT_FAST_PAIR_MESSAGE_CODE_DEVICE_ACTION_EVENT_RING,
                    };
#ifdef RACE_FIND_ME_ENABLE
                    bt_fast_pair_send_acknowledgements(&(format_data->addr), &ack, msg->data, msg->data_length);
#else
                    ack.action = BT_FAST_PAIR_MESSAGE_CODE_ACKNOWLEDGEMENT_NAK;
                    ack.reason = BT_FAST_PAIR_ACKNOWLEDGEMENT_NAK_REASON_NOT_SUPPORTED;
                    uint8_t current_state[1] = { 0 };
                    bt_fast_pair_send_acknowledgements(&(format_data->addr), &ack, current_state, sizeof(current_state));
#endif
                }
#ifdef AIR_BT_FAST_PAIR_SASS_ENABLE
                if (msg->group_ID == BT_FAST_PAIR_MESSAGE_GROUP_SMART_AUDIO_SOURCE_SWITCHING) {
                    app_fast_pair_app_event_proc_sass_msg(&(format_data->addr), msg, true);
                }
#endif
            }
            break;
        }
        case BT_FAST_PAIR_APP_EVENT_ADDITIONAL_DATA: {
            if (NULL == format_data || data_len <= sizeof(app_bt_event_fast_pair_callback_data_t)) {
                break;
            }
            app_fast_pair_additional_data_handle(&g_app_fast_pair_context, (bt_fast_pair_additional_data_t *)format_data->data, data_len - sizeof(bt_bd_addr_t));
            break;
        }
        case BT_FAST_PAIR_APP_EVENT_ACTIVE_COMPONENTS_REQ: {
            if (NULL == format_data && data_len <= sizeof(app_bt_event_fast_pair_callback_data_t)) {
                break;
            }
            bt_bd_addr_t *bt_addr = &(format_data->addr);
            uint8_t message_buffer[sizeof(bt_fast_pair_message_stream_t)] = { 0 };
            uint8_t components_active_state = 0x01; /* For headset, 0x01 means active. */
#ifdef AIR_TWS_ENABLE
            components_active_state = 0;
            if (0 == g_app_fast_pair_context.battery.battery[APP_FAST_PAIR_COMPONENT_RIGHT].charging
                && g_app_fast_pair_context.battery.battery[APP_FAST_PAIR_COMPONENT_RIGHT].battery_value <= 100) {
                components_active_state |= 0x1;
            }
            if (0 == g_app_fast_pair_context.battery.battery[APP_FAST_PAIR_COMPONENT_LEFT].charging
                && g_app_fast_pair_context.battery.battery[APP_FAST_PAIR_COMPONENT_LEFT].battery_value <= 100) {
                components_active_state |= 0x1 << 1;
            }
#endif

            bt_fast_pair_message_stream_t *cont = (bt_fast_pair_message_stream_t *)&message_buffer;
            cont->group_ID = BT_FAST_PAIR_MESSAGE_GROUP_DEVICE_INFO_EVENT;
            cont->code_ID = BT_FAST_PAIR_MESSAGE_CODE_DEVICE_INFO_EVENT_ACTIVE_COMPONENTS_RSP;
            cont->data_length = 0x01;
            cont->data[0] = components_active_state;
            APPS_LOG_MSGID_I(LOG_TAG" ACTIVE COMPONENTS_RSP : %x.", 1, cont->data[0]);
            bt_fast_pair_send_message_stream(bt_addr, cont);
            break;
        }
        case BT_FAST_PAIR_APP_EVENT_PAIR_STARTED: {
#if APP_ONLY_ALLOW_TAKE_OVER_BY_FAST_PAIR
            g_app_fast_pair_context.waiting_connect = true;
#endif
            break;
        }

#ifdef AIR_BT_FAST_PAIR_SASS_ENABLE
        case BT_FAST_PAIR_APP_EVENT_SASS_UPDATE_IN_USE_ACCOUNT: {
#ifdef AIR_TWS_ENABLE
            const bt_fast_pair_sass_recorded_data_t *p_recorded_data;
            bt_fast_pair_sass_get_internal_data(NULL, &p_recorded_data);
            apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_BT_FAST_PAIR,
                                           BT_FAST_PAIR_APP_EVENT_SASS_UPDATE_IN_USE_ACCOUNT,
                                           &p_recorded_data->inused_account_key_id,
                                           sizeof(p_recorded_data->inused_account_key_id));
#endif
            app_fast_pair_connection_state_change();
            break;
        }
        case BT_FAST_PAIR_APP_EVENT_SASS_UPDATE_CONNECTED_RECORD: {
#ifdef AIR_TWS_ENABLE
            const bt_fast_pair_sass_recorded_data_t *p_recorded_data;
            bt_fast_pair_sass_get_internal_data(NULL, &p_recorded_data);

            apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_BT_FAST_PAIR,
                                           BT_FAST_PAIR_APP_EVENT_SASS_UPDATE_CONNECTED_RECORD,
                                           p_recorded_data->connected_record_array,
                                           sizeof(bt_bd_addr_t) * p_recorded_data->connected_record_length);
#endif
            break;
        }
#if defined(AIR_TWS_ENABLE) && defined(AIR_BT_FAST_PAIR_LE_AUDIO_ENABLE)
        case BT_FAST_PAIR_APP_EVENT_SASS_PEER_STATE_RECEIVED: {
            app_fast_pair_connection_state_change();
            break;
        }
#endif
#endif
#ifdef AIR_SPOT_ENABLE
        case BT_FAST_PAIR_SPOT_EVENT_START_ADV: {
            g_app_fast_pair_context.spot_enabled = true;
            // Use last old EID
            app_fast_pair_gen_eid_and_trigger_adv_async();
#ifdef MTK_AWS_MCE_ENABLE
            if (bt_connection_manager_device_local_info_get_aws_role() != BT_AWS_MCE_ROLE_AGENT) {
                break;
            }
            uint32_t size = 32;
            uint8_t buf[32] = {0};
            nvkey_status_t sta = nvkey_read_data(NVID_APP_FPS_SPOT_EIK, buf, &size);
            if (!app_fast_pair_spot_check_eik(buf, size)) {
                break;
            }
            apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_BT_FAST_PAIR,
                                           BT_FAST_PAIR_SPOT_EVENT_START_ADV,
                                           buf, size);
            APPS_LOG_MSGID_I(LOG_TAG"[SPOT] sync eph key to partner, len=%d, nv_sta=%d", 2, size, sta);
#endif
            break;
        }

        case BT_FAST_PAIR_SPOT_EVENT_STOP_ADV: {
            g_app_fast_pair_context.spot_enabled = false;
            app_fast_pair_spot_enable_advertising(FALSE);
            break;
        }

        case BT_FAST_PAIR_SPOT_EVENT_EID_READY: {
            // Use new EID when first generate EID successfully
            app_fast_pair_spot_enable_advertising(TRUE);
            break;
        }

        case BT_FAST_PAIR_SPOT_EVENT_EID_UPDATED: {
            // Use new EID every 1024 sec
            app_fast_pair_spot_enable_advertising(TRUE);
            break;
        }
#endif
        default:
            break;
    }
    return false;
}

#ifdef AIR_BT_FAST_PAIR_SASS_ENABLE
uint8_t bt_fast_pair_sass_get_connection_state(void)
{
    uint8_t fast_pair_state = BT_FAST_PAIR_CONNECTION_STATE_NO_CONNECTION;
    const bt_device_manager_link_record_t *link_info = bt_device_manager_link_record_get_connected_link();
#ifdef AIR_LE_AUDIO_ENABLE
    bool le_audio_playing = false;
    bt_bd_addr_t *p_remote_addr = NULL;
#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
    bt_sink_srv_device_state_t device_state;
    bt_status_t bt_ret = bt_sink_srv_get_playing_device_state(&device_state);
    p_remote_addr = &(device_state.address);
    APPS_LOG_MSGID_I(LOG_TAG"get_connection_state, get device_state ret = %x, device_state.type = %d", 2, bt_ret, device_state.type);
    if (BT_STATUS_SUCCESS == bt_ret && device_state.type == BT_SINK_SRV_DEVICE_LE) {
        le_audio_playing = true;
    }
#else
    /* Multipoint must be not defined when AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE is not defined */
    if (link_info && link_info->connected_num > 0) {
        le_audio_playing = link_info->connected_device[0].link_type == BT_DEVICE_MANAGER_LINK_TYPE_LE;
        p_remote_addr = &(link_info->connected_device[0].remote_addr);
    }
#endif
#endif
    switch (apps_config_key_get_mmi_state()) {
        case APP_BT_OFF:
            fast_pair_state = BT_FAST_PAIR_CONNECTION_STATE_LE_AUDIO_DISABLE_CONNECTION_SWITCH;
            break;
        case APP_CONNECTABLE:
        case APP_DISCONNECTED:
            if (0 < bt_cm_get_connected_devices(~BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS), NULL, 0)) {
                fast_pair_state = BT_FAST_PAIR_CONNECTION_STATE_NO_CONNECTION;
            } else {
                if (0 < bt_cm_get_connecting_devices(~BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS), NULL, 0)) {
                    fast_pair_state = BT_FAST_PAIR_CONNECTION_STATE_PAGING;
                }
            }
            break;
        case APP_CONNECTED:
            fast_pair_state = BT_FAST_PAIR_CONNECTION_STATE_CONNECTED;
            break;
        case APP_HFP_INCOMING:
        case APP_HFP_OUTGOING:
        case APP_HFP_CALL_ACTIVE:
        case APP_HFP_CALL_ACTIVE_WITHOUT_SCO:
        case APP_HFP_TWC_INCOMING:
        case APP_HFP_TWC_OUTGOING:
        case APP_HFP_MULTIPARTY_CALL:
        case APP_STATE_HELD_ACTIVE:
        case APP_STATE_VA:
            fast_pair_state = BT_FAST_PAIR_CONNECTION_STATE_HFP;
#ifdef AIR_LE_AUDIO_ENABLE
            if (le_audio_playing) {
                fast_pair_state = BT_FAST_PAIR_CONNECTION_STATE_LE_AUDIO_CALL;
            }
#endif
            break;
        case APP_A2DP_PLAYING:
            if (bt_sink_srv_get_music_state() == BT_SINK_SRV_MUSIC_PLAYBACK_STATE_STOPPED) {
                fast_pair_state = BT_FAST_PAIR_CONNECTION_STATE_A2DP_STREAMING;
            } else {
                fast_pair_state = BT_FAST_PAIR_CONNECTION_STATE_A2DP_STREAMING_WITH_AVRCP;
            }
#ifdef AIR_LE_AUDIO_ENABLE
            if (le_audio_playing) {
                bt_le_audio_content_type_t content_type = 0;
                if (p_remote_addr) {
                    content_type = bt_sink_srv_cap_stream_get_streaming_ase_metadata(bt_gap_le_srv_get_conn_handle_by_address(p_remote_addr), AUDIO_DIRECTION_SINK);
                }
                if (AUDIO_CONTENT_TYPE_GAME & content_type) {
                    fast_pair_state = BT_FAST_PAIR_CONNECTION_STATE_LE_AUDIO_MEDIA_STREAMING;
                } else {
                    fast_pair_state = BT_FAST_PAIR_CONNECTION_STATE_LE_AUDIO_MEDIA_STREAMING_WITH_CONTROL;
                }
            }
#endif
            break;
#ifdef AIR_LE_AUDIO_ENABLE
        case APP_LE_AUDIO_BIS_PLAYING:
            fast_pair_state = BT_FAST_PAIR_CONNECTION_STATE_LE_AUDIO_BIS;
            break;
#endif
        case APP_STATE_FIND_ME:
            fast_pair_state = BT_FAST_PAIR_CONNECTION_STATE_NON_AUDIO_DATA_TRANSFERRING;
            break;
        default:
            break;
    }
#ifdef MTK_BT_SPEAKER_DISABLE_BROADCAST_EDR
    if (BT_AWS_MCE_SRV_MODE_BROADCAST == bt_aws_mce_srv_get_mode()) {
        fast_pair_state = BT_FAST_PAIR_CONNECTION_STATE_LE_AUDIO_DISABLE_CONNECTION_SWITCH;
    }
#endif

    /* Connection flags. */
#ifdef MTK_IN_EAR_FEATURE_ENABLE
    fast_pair_state |= (app_in_ear_get_ohd_state() <= APP_IN_EAR_OHD_NONE_DETECTED ? 0 : 1) << BT_FAST_PAIR_CONNECTION_FLAGS_ON_HEAD_DETECTION;
#else
    fast_pair_state |= 0 << BT_FAST_PAIR_CONNECTION_FLAGS_ON_HEAD_DETECTION;
#endif

    uint32_t connected_count = 0;
    if (link_info) {
        connected_count = link_info->connected_num;
    }
    /* uint32_t connected_count = app_bt_state_service_get_connected_exclude_aws(NULL, 0); */

    uint32_t max_connect = 1;
#ifdef AIR_MULTI_POINT_ENABLE
    if (app_bt_emp_is_enable()) {
#ifdef AIR_3_LINK_MULTI_POINT_ENABLE
        max_connect = BT_MAX_LINK_NUM - 1;
#else
        max_connect = 2;
#endif
    }
#endif
    APPS_LOG_MSGID_I(LOG_TAG" connected_count: %d, max_connect: %d", 2, connected_count, max_connect);
    fast_pair_state |= (connected_count >= max_connect ? 0 : 1) << BT_FAST_PAIR_CONNECTION_FLAGS_CONNECTION_AVALIABILITY;
    fast_pair_state |= 0 << BT_FAST_PAIR_CONNECTION_FLAGS_FOCUS_MODE;
    fast_pair_state |= 0 << BT_FAST_PAIR_CONNECTION_FLAGS_AUTO_RECONNECTED;
    return fast_pair_state;
}

uint16_t bt_fast_pair_sass_get_capability_flags(void)
{
    return (FAST_PAIR_SASS_DEFAULT_ENABLED << BT_FAST_PAIR_SASS_CAPABILITY_STATE_BIT)
#ifdef AIR_MULTI_POINT_ENABLE
           | (1 << BT_FAST_PAIR_SASS_CAPABILITY_MULTIPOINT_CONFIG_BIT)
           | ((app_bt_emp_is_enable() ? 1 : 0) << BT_FAST_PAIR_SASS_CAPABILITY_MULTIPOINT_STATE_BIT)
#else
           | (0 << BT_FAST_PAIR_SASS_CAPABILITY_MULTIPOINT_CONFIG_BIT)
           | (0 << BT_FAST_PAIR_SASS_CAPABILITY_MULTIPOINT_STATE_BIT)
#endif
#ifdef MTK_IN_EAR_FEATURE_ENABLE
           | (1 << BT_FAST_PAIR_SASS_CAPABILITY_ON_HEAD_DETECTION_BIT)
           | ((app_in_ear_get_ohd_state() == APP_IN_EAR_OHD_DISABLED ? 0 : 1) << BT_FAST_PAIR_SASS_CAPABILITY_ON_HEAD_DETECTION_CURRENT_BIT)
#else
           | (0 << BT_FAST_PAIR_SASS_CAPABILITY_ON_HEAD_DETECTION_BIT)
           | (0 << BT_FAST_PAIR_SASS_CAPABILITY_ON_HEAD_DETECTION_CURRENT_BIT)
#endif
           ;
}
#endif

#ifdef AIR_LE_AUDIO_ENABLE
bool bt_fast_pair_get_le_addr(bool local_or_peer, uint8_t *type, uint8_t *addr)
{
    bool ret = false;
    ret = app_le_audio_get_addr(local_or_peer, type, addr);

    return ret;
}

bool bt_fast_pair_set_le_io_capability(bool is_display_yesno)
{
    bool ret = false;
    bt_status_t set_result = BT_STATUS_SUCCESS;
    bt_gap_le_smp_io_capability_t io_capability = is_display_yesno ? BT_GAP_LE_SMP_DISPLAY_YES_NO : BT_GAP_LE_SMP_NO_INPUT_NO_OUTPUT;
    set_result = bt_app_common_set_pairing_config_io_capability(BT_GAP_LE_SRV_LINK_TYPE_FAST_PAIR, io_capability);
    set_result = bt_app_common_set_pairing_config_io_capability(BT_GAP_LE_SRV_LINK_TYPE_LE_AUDIO, io_capability);
    if (set_result == BT_STATUS_SUCCESS) {
        ret = true;
#ifdef MTK_AWS_MCE_ENABLE
    if (bt_connection_manager_device_local_info_get_aws_role() == BT_AWS_MCE_ROLE_AGENT 
     && BT_AWS_MCE_SRV_LINK_NONE != bt_aws_mce_srv_get_link_type()) {
        if (apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_BT_FAST_PAIR,
                                           APP_FAST_PAIR_AWS_SYNC_LE_IO_CAPABILITY,
                                           &is_display_yesno, sizeof(bool))!= BT_STATUS_SUCCESS) {
            ret  = false;
        }
    }
#endif

    }
    APPS_LOG_MSGID_I(LOG_TAG" set_le_io_capability: set_result=0x%x, is_display_yesno=%d", 2, set_result, is_display_yesno);
    return ret;
}
#endif

#ifdef AIR_SMART_CHARGER_ENABLE
/**
* @brief      This function process the events of EVENT_GROUP_UI_SHELL_CHARGER_CASE group.
* @param[in]  self, the context pointer of the activity.
* @param[in]  event_id, the current event ID to be handled.
* @param[in]  extra_data, extra data pointer of the current event, NULL means there is no extra data.
* @param[in]  data_len, the length of the extra data. 0 means extra_data is NULL.
* @return     If return true, the current event cannot be handle by the next activity.
*/
static bool app_fast_pair_proc_charger_case_group(ui_shell_activity_t *self,
                                                  uint32_t event_id,
                                                  void *extra_data,
                                                  size_t data_len)
{
    bool ret = false;
    uint8_t battery_item = (AUDIO_CHANNEL_R == ami_get_audio_channel() ? APP_FAST_PAIR_COMPONENT_RIGHT : APP_FAST_PAIR_COMPONENT_LEFT);
    bt_fast_pair_battery_t update_battery = g_app_fast_pair_context.battery;

    switch (event_id) {
        case SMCHARGER_EVENT_NOTIFY_ACTION: {
            app_smcharger_public_event_para_t *event_para = (app_smcharger_public_event_para_t *)extra_data;
            APPS_LOG_MSGID_I(LOG_TAG", SMCharger NOTIFY_ACTION", 1, event_para->action);
            if (event_para->action == SMCHARGER_CHARGER_IN_ACTION) {
                update_battery.battery[battery_item].charging = 1;
                app_fast_pair_stop_find_me();
            } else if (event_para->action == SMCHARGER_OPEN_LID_ACTION) {
                update_battery.ui_show = true;
            } else if (event_para->action == SMCHARGER_CLOSE_LID_ACTION) {
                update_battery.ui_show = false;
            } else if (event_para->action == SMCHARGER_CHARGER_OUT_ACTION) {
                update_battery.ui_show = true;
                update_battery.battery[battery_item].charging = 0;
                app_fast_pair_stop_find_me();
            } else if (event_para->action == SMCHARGER_CASE_BATTERY_REPORT_ACTION) {
                uint8_t case_battery = (uint8_t)((uint32_t)event_para->data);
                update_battery.battery[APP_FAST_PAIR_COMPONENT_CASE].charging = ((case_battery & 0x80) ? 1 : 0);
                update_battery.battery[APP_FAST_PAIR_COMPONENT_CASE].battery_value = (case_battery & 0x7F);
            }
        }
        break;

        default:
            return ret;
    }
    app_fast_pair_update_battery(&g_app_fast_pair_context, &update_battery);
    return ret;
}
#endif

#ifdef AIR_TWS_ENABLE
#ifdef AIR_SMART_CHARGER_ENABLE
/**
* @brief      This function process the aws data of BT_AWS_MCE_REPORT_MODULE_SMCHARGER.
* @param[in]  context, the context of the app.
* @param[in]  report, the aws report data.
*/
static void app_fast_pair_aws_mce_data_smart_charge_callback(app_fast_pair_local_context_t *context,  bt_aws_mce_report_info_t *report)
{
#if 0
    received_charger_exist = (smcharger_context.peer_smcharger_state != STATE_SMCHARGER_OUT_OF_CASE) ? 1 : 0;
    context->battery.battery[battery_item].charging = received_charger_exist;
    if (!(context->battery.battery[APP_FAST_PAIR_COMPONENT_RIGHT].charging) &&
        !(context->battery.battery[APP_FAST_PAIR_COMPONENT_LEFT].charging)) {
        context->battery.ui_show = false;
    } else {
        context->battery.ui_show = true;
    }
    bt_fast_pair_update_battery(&(context->battery));
    app_fast_pair_trigger_advertising(context);
#endif
}
#endif

/**
* @brief      This function process the AWS data of BT_AWS_MCE_REPORT_MODULE_BATTERY.
* @param[in]  context, the context of the app.
* @param[in]  report, the AWS report data.
*/
static void app_fast_pair_aws_mce_data_battery_callback(app_fast_pair_local_context_t *context, bt_aws_mce_report_info_t *report)
{
#if defined(MTK_BATTERY_MANAGEMENT_ENABLE)
    uint8_t received_charger_exist = 0;
    uint8_t received_bat_percent = 0;
    uint8_t battery_item = (AUDIO_CHANNEL_R == ami_get_audio_channel() ? APP_FAST_PAIR_COMPONENT_LEFT : APP_FAST_PAIR_COMPONENT_RIGHT);
    bt_fast_pair_battery_t update_battery = context->battery;

#ifdef AIR_SMART_CHARGER_ENABLE
    app_smcharger_context_t smcharger_context;
    memcpy(&smcharger_context.peer_battery_percent, report->param, sizeof(smcharger_context.peer_battery_percent));
    APPS_LOG_MSGID_I(LOG_TAG", [Agent] Received partner_bat_percent = %d", 1, smcharger_context.peer_battery_percent);
    received_bat_percent = smcharger_context.peer_battery_percent & (~ PARTNER_BATTERY_CHARGING);
    received_charger_exist = (smcharger_context.peer_battery_percent & PARTNER_BATTERY_CHARGING) ? 1 : 0;    /* The bit 7 is charger state */
#else
    battery_local_context_type_t battery_context;
    memcpy(&battery_context.partner_battery_percent, report->param, sizeof(battery_context.partner_battery_percent));
    APPS_LOG_MSGID_I(LOG_TAG", [Agent] Received partner_bat_percent = %d", 1, battery_context.partner_battery_percent);
    received_bat_percent = battery_context.partner_battery_percent & (~ PARTNER_BATTERY_CHARGING);
    received_charger_exist = (battery_context.partner_battery_percent & PARTNER_BATTERY_CHARGING) ? 1 : 0;      /* The bit 7 is charger state */
#endif
    update_battery.battery[battery_item].charging = received_charger_exist;
    update_battery.battery[battery_item].battery_value = received_bat_percent;
    app_fast_pair_update_battery(context, &update_battery);
#endif
}

/**
* @brief      This function process the AWS data of BT_AWS_MCE_REPORT_MODULE_APP_ACTION, it's fast pair internal AWS events.
* @param[in]  context, the context of the app.
* @param[in]  report, the AWS report data.
*/
static void app_fast_pair_aws_mce_data_app_action_callback(app_fast_pair_local_context_t *context, bt_aws_mce_report_info_t *report)
{
    uint32_t    event_group = 0, event_id = 0, extra_aws_len = 0;
    void        *extra_aws = NULL;
    apps_aws_sync_event_decode_extra(report, &event_group, &event_id, &extra_aws, &extra_aws_len);
    if (EVENT_GROUP_UI_SHELL_BT_FAST_PAIR != event_group) {
        return;
    }
#ifdef AIR_SPOT_ENABLE
    if (extra_aws == NULL && event_id < BT_FAST_PAIR_SPOT_APP_EVENT_UNWANTED_TRACKING_MODE_ENABLE) {
        return;
    }
#endif
    switch (event_id) {
        case BT_FAST_PAIR_APP_EVENT_NEED_STORE_ACCOUNT_KEY: {
            /* Received store account key request from agent. */
            bt_fast_pair_account_key_list_t *new_account = (bt_fast_pair_account_key_list_t *)extra_aws;
            if (NULL == new_account || NULL == context->account_key_list ||
                context->account_key_list->max_key_number != new_account->max_key_number
                || extra_aws_len != sizeof(bt_fast_pair_account_key_list_t) +
                sizeof(bt_fast_pair_account_key_t) * (g_app_fast_pair_context.account_key_list->max_key_number - 1)) {
                return;
            }
            if (memcmp(context->account_key_list, new_account, extra_aws_len)) {
                memcpy(context->account_key_list, new_account, extra_aws_len);
                app_fast_pair_store_account_key(context->account_key_list);
            }
            break;
        }
        case BT_FAST_PAIR_APP_EVENT_RING_REQUEST: {
#ifdef RACE_FIND_ME_ENABLE
            /* Received ring request from agent. */
            if (extra_aws_len == sizeof(app_find_me_param_struct)) {
                app_find_me_param_struct *find_me_param =
                    (app_find_me_param_struct *)pvPortMalloc(sizeof(app_find_me_param_struct));
                if (find_me_param) {
                    memcpy(find_me_param, extra_aws, sizeof(app_find_me_param_struct));
                    ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE,
                                        EVENT_GROUP_UI_SHELL_FINDME,
                                        APP_FIND_ME_EVENT_ID_TRIGGER,
                                        find_me_param,
                                        sizeof(app_find_me_param_struct),
                                        NULL, 0);
                }
            }
#endif
            break;
        }
        case BT_FAST_PAIR_APP_EVENT_ADDITIONAL_DATA: {
            /* Received process additional data from agent. */
            bt_fast_pair_additional_data_t *data = (bt_fast_pair_additional_data_t *)extra_aws;
            APPS_LOG_MSGID_I(LOG_TAG" Aws packet additional data received data_id %d, data_length %d.", 2, data->data_id, data->data_length);
            if (BT_FAST_PAIR_ADDITIONAL_DATA_ID_PERSONALIZED_NAME == data->data_id) {
                if (data->data_length > sizeof(context->nvkey.personalized_name) - 1) {
                    data->data_length = sizeof(context->nvkey.personalized_name) - 1;
                }
                memcpy(context->nvkey.personalized_name, data->data, data->data_length);
                context->nvkey.personalized_name[data->data_length] = 0;
                nvkey_write_data(NVID_APP_FAST_PAIR_PERSONALIZED_NAME,
                                 (uint8_t *) & (g_app_fast_pair_context.nvkey.personalized_name),
                                 data->data_length + 1);
                app_fast_pair_update_parameter();
            }
            break;
        }
        case APP_FAST_PAIR_AWS_CONTEXT_SYNC_ID: {
            app_fast_pair_nvkey_t *nvkey_cntx = (app_fast_pair_nvkey_t *)extra_aws;
            memcpy(&(context->nvkey), nvkey_cntx, (sizeof(context->nvkey) - sizeof(context->nvkey.reserved)));
        }
        break;
#ifdef AIR_BT_FAST_PAIR_SASS_ENABLE
        case BT_FAST_PAIR_APP_EVENT_SASS_UPDATE_IN_USE_ACCOUNT: {
            if (extra_aws_len == sizeof(uint8_t)) {
                uint8_t in_use_account_id = *(uint8_t *)extra_aws;
                bt_fast_pair_sass_set_in_used_account_key_id(in_use_account_id);
            }
            break;
        }
        case BT_FAST_PAIR_APP_EVENT_SASS_UPDATE_CONNECTED_RECORD: {
            uint8_t array_len = extra_aws_len / sizeof(bt_bd_addr_t);
            bt_bd_addr_t *array = array_len > 0 ? extra_aws : NULL;
            bt_fast_pair_sass_set_connected_record(array_len, array);
            break;
        }
#if defined(AIR_BT_FAST_PAIR_LE_AUDIO_ENABLE)
        case APP_FAST_PAIR_AWS_SYNC_SASS_CONTROL_MSG: {
            app_fast_pair_sass_aws_sync_ctrl_msg_t *ctrl_msg = (app_fast_pair_sass_aws_sync_ctrl_msg_t *)extra_aws;
            app_fast_pair_app_event_proc_sass_msg(&ctrl_msg->addr, &ctrl_msg->msg_data, false);
            break;
        }
#endif
#endif
#if defined(AIR_BT_FAST_PAIR_LE_AUDIO_ENABLE)
        case APP_FAST_PAIR_AWS_SYNC_LE_IO_CAPABILITY: {
            bool capability = *(bool *)extra_aws;
            bt_fast_pair_set_le_io_capability(capability);
            break;
        }
#endif
#ifdef AIR_SPOT_ENABLE
        case BT_FAST_PAIR_SPOT_EVENT_START_ADV: {
            if (!app_fast_pair_spot_check_eik((uint8_t *)extra_aws, extra_aws_len)) {
                break;
            }
            nvkey_write_data(NVID_APP_FPS_SPOT_EIK, extra_aws, extra_aws_len);
            app_fast_pair_gen_eid_and_trigger_adv_async();
            break;
        }
        case BT_FAST_PAIR_SPOT_EVENT_STOP_ADV: {
            s_spot_in_adv = false;
            uint8_t buf[32] = {0};
            nvkey_write_data(NVID_APP_FPS_SPOT_EIK, buf, 32);
            bt_fast_pair_spot_set_advertising(false);
            break;
        }
        case BT_FAST_PAIR_SPOT_APP_EVENT_UNWANTED_TRACKING_MODE_ENABLE: {
            if (extra_aws) {
                uint8_t *en = (uint8_t *)extra_aws;
                bt_fast_pair_spot_unwanted_tracking_mode(*en, true);
            }
            break;
        }
        case BT_FAST_PAIR_SPOT_APP_EVENT_UNWANTED_TRACKING_MODE_DISABLE: {
            bt_fast_pair_spot_unwanted_tracking_mode(0x0, true);
            break;
        }
#endif
        default:
            break;
    }
}

/**
* @brief      This function process the events of EVENT_GROUP_UI_SHELL_AWS_DATA group.
* @param[in]  self, the context pointer of the activity.
* @param[in]  event_id, the current event ID to be handled.
* @param[in]  extra_data, extra data pointer of the current event, NULL means there is no extra data.
* @param[in]  data_len, the length of the extra data. 0 means extra_data is NULL.
* @return     If return true, the current event cannot be handle by the next activity.
*/
static bool app_fast_pair_aws_mce_data_callback(struct _ui_shell_activity *self,  uint32_t event_id, void *extra_data, size_t data_len)
{
    assert(NULL != self);
    bt_aws_mce_report_info_t *report = (bt_aws_mce_report_info_t *)extra_data;
    if (NULL == report) {
        return false;
    }
    switch (report->module_id) {
        case BT_AWS_MCE_REPORT_MODULE_BATTERY:
            app_fast_pair_aws_mce_data_battery_callback(&g_app_fast_pair_context, report);
            break;
#ifdef AIR_SMART_CHARGER_ENABLE
        case BT_AWS_MCE_REPORT_MODULE_SMCHARGER:
            app_fast_pair_aws_mce_data_smart_charge_callback(&g_app_fast_pair_context, report);
            break;
#endif
        case BT_AWS_MCE_REPORT_MODULE_APP_ACTION:
            app_fast_pair_aws_mce_data_app_action_callback(&g_app_fast_pair_context, report);
            break;
        default:
            return false;
    }
    return false;
}
#endif

/**
* @brief      This function process the events of EVENT_GROUP_UI_SHELL_MULTI_VA group.
* @param[in]  self, the context pointer of the activity.
* @param[in]  event_id, the current event ID to be handled.
* @param[in]  extra_data, extra data pointer of the current event, NULL means there is no extra data.
* @param[in]  data_len, the length of the extra data. 0 means extra_data is NULL.
* @return     If return true, the current event cannot be handle by the next activity.
*/
static bool app_fast_pair_multi_va_event_proc(ui_shell_activity_t *self,  uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;
    switch (event_id) {
        case MULTI_VA_EVENT_NOTIFY_BLE_ADDR_CHANGED: {
            bt_bd_addr_t random_addr;
            if (multi_ble_adv_manager_get_random_addr_and_adv_handle(MULTI_ADV_INSTANCE_FAST_PAIR, &random_addr, NULL)) {
                APPS_LOG_MSGID_I(LOG_TAG", ble address changed : %02X:%02X:%02X:%02X:%02X:%02X", 6,
                                 random_addr[5], random_addr[4], random_addr[3],
                                 random_addr[2], random_addr[1], random_addr[0]);
                bt_fast_pair_set_random_ble_address(false, &random_addr);
            }
            break;
        }
        default:
            break;
    }

    return ret;
}


/**
* @brief      This function to init fast pair.
* @param[in]  self, the context pointer of the activity.
* @return     initialized success or not.
*/
static bool app_fast_pair_init(ui_shell_activity_t *self)
{
    bt_fast_pair_status_t           ret;
    uint32_t    app_fast_pair_model_id = APP_FAST_PAIR_AIROHA_TEST_MODEL_ID;
    uint8_t     app_fast_pair_private_key[32] = APP_FAST_PAIR_AIROHA_TEST_PRIVATE_KEY;
    app_fast_pair_protected_t       *protect_sets;
    bt_bd_addr_t random_addr;
    atci_register_handler(fast_pair_test_atci_cmd, sizeof(fast_pair_test_atci_cmd) / sizeof(atci_cmd_hdlr_item_t));
    protect_sets = (app_fast_pair_protected_t *)pvPortMalloc(sizeof(app_fast_pair_protected_t));
    if (protect_sets != NULL && 0 == app_fast_pair_load_configure_info(&g_app_fast_pair_context, protect_sets)) {
        if (g_app_fast_pair_context.nvkey.selected_set >= APP_FAST_PAIR_MODEL_SET_MAX_NUMBER) {
            g_app_fast_pair_context.nvkey.selected_set = APP_FAST_PAIR_MODEL_SET_MAX_NUMBER - 1;
        }
        app_fast_pair_model_id = protect_sets->sets[g_app_fast_pair_context.nvkey.selected_set].model_id;
        memcpy(app_fast_pair_private_key, &(protect_sets->sets[g_app_fast_pair_context.nvkey.selected_set].private_key), 32 * sizeof(uint8_t));
        vPortFree(protect_sets);
        APPS_LOG_MSGID_W(LOG_TAG" init model id 0x%x, max account %d, tx_power_exist %d, tx_power %d, component num %d select %d!",
                         6, app_fast_pair_model_id, g_app_fast_pair_context.nvkey.max_account, g_app_fast_pair_context.nvkey.tx_power_available,
                         g_app_fast_pair_context.nvkey.tx_power_level, g_app_fast_pair_context.nvkey.component_num, g_app_fast_pair_context.nvkey.selected_set);
        if (!g_app_fast_pair_context.nvkey.fps_enable) {
            return false;
        }
    } else {
        /* Load fail must stop init process */
        g_app_fast_pair_context.nvkey.fps_enable = 1;
        g_app_fast_pair_context.nvkey.max_account = APP_FAST_PAIR_DEFAULT_MAX_ACCOUNT;
        g_app_fast_pair_context.nvkey.selected_set = 0;
        g_app_fast_pair_context.nvkey.tx_power_available = 1;
        g_app_fast_pair_context.nvkey.tx_power_level = APP_FAST_PAIR_DEFAULT_TX_POWER_LEVEL;
        g_app_fast_pair_context.nvkey.component_num = APP_FAST_PAIR_COMPONENT_NUM;
        /* memset(g_app_fast_pair_context.nvkey.personalized_name, 0, sizeof(g_app_fast_pair_context.nvkey.personalized_name)); */
    }
    g_app_fast_pair_context.model_id = app_fast_pair_model_id;
    g_app_fast_pair_context.account_key_list = (bt_fast_pair_account_key_list_t *)pvPortMalloc(sizeof(bt_fast_pair_account_key_list_t) +
                                                                                               (g_app_fast_pair_context.nvkey.max_account - 1) * sizeof(bt_fast_pair_account_key_t));
    if (NULL == g_app_fast_pair_context.account_key_list) {
        return false;
    }
    app_fast_pair_load_account_key(&g_app_fast_pair_context);
    bt_fast_pair_init_parameters_t init_param;
    memset(&init_param, 0, sizeof(init_param));
    init_param.model_id = app_fast_pair_model_id;
    init_param.private_key = app_fast_pair_private_key;
    init_param.tx_power_level = NULL;
    init_param.addr_type = BT_ADDR_RANDOM_IDENTITY;
    init_param.component_num = g_app_fast_pair_context.nvkey.component_num;
    init_param.personalized_name = g_app_fast_pair_context.nvkey.personalized_name;
    init_param.firmware_revision = MTK_FW_VERSION;
#ifdef AIR_BT_FAST_PAIR_SASS_ENABLE
    g_app_fast_pair_context.sass_switch_prefer_flags = (0 << BT_FAST_PAIR_SASS_MULTIPOINT_SWITCH_A_A_BIT)
                                                       | (1 << BT_FAST_PAIR_SASS_MULTIPOINT_SWITCH_H_A_BIT);
    init_param.sass_param.sass_version_code = FAST_PAIR_SASS_VERSION_CODE;
    init_param.sass_param.sass_capability_flags = bt_fast_pair_sass_get_capability_flags();
    init_param.sass_param.switching_preference_flags = g_app_fast_pair_context.sass_switch_prefer_flags;
#endif
    if (0 != g_app_fast_pair_context.nvkey.tx_power_available) {
        init_param.tx_power_level = &(g_app_fast_pair_context.nvkey.tx_power_level);
    }
#ifdef AIR_BT_FAST_PAIR_LE_AUDIO_ENABLE
    if (app_lea_service_is_enabled_lea()) {
        init_param.le_param.le_support = true;
    }
    init_param.le_param.le_passkey = 0;
#endif
#ifdef AIR_TWS_ENABLE
    init_param.aws_device          = true;
#endif

    ret = bt_fast_pair_init(&init_param);
    if (multi_ble_adv_manager_get_random_addr_and_adv_handle(MULTI_ADV_INSTANCE_FAST_PAIR,
                                                             &random_addr,
                                                             &(g_app_fast_pair_context.adv_handle))) {
        bt_fast_pair_set_random_ble_address(false, &random_addr);
    }

    g_app_fast_pair_context.battery.ui_show = false;
    g_app_fast_pair_context.battery.component_num = APP_FAST_PAIR_COMPONENT_NUM;
    g_app_fast_pair_context.battery.remaining_time = BT_FAST_PAIR_REMAINING_TIME_INVALID_VALUE;
    for (uint8_t i = 0; i < APP_FAST_PAIR_COMPONENT_NUM; i++) {
        g_app_fast_pair_context.battery.battery[i].charging = 0;
        g_app_fast_pair_context.battery.battery[i].battery_value = 0xFF;
    }
    if (ret != BT_FAST_PAIR_STATUS_SUCCESS) {
        vPortFree(g_app_fast_pair_context.account_key_list);
        g_app_fast_pair_context.account_key_list = NULL;
        return false;
    }
    return true;
}

uint32_t app_fast_pair_get_model_id(void)
{
    app_fast_pair_protected_t *protect_sets = NULL;
    uint32_t model_id = APP_FAST_PAIR_AIROHA_TEST_MODEL_ID;

    protect_sets = (app_fast_pair_protected_t *)pvPortMalloc(sizeof(app_fast_pair_protected_t));
    if (protect_sets != NULL && 0 == app_fast_pair_load_configure_info(&g_app_fast_pair_context, protect_sets)) {
        if (g_app_fast_pair_context.nvkey.selected_set >= APP_FAST_PAIR_MODEL_SET_MAX_NUMBER) {
            g_app_fast_pair_context.nvkey.selected_set = APP_FAST_PAIR_MODEL_SET_MAX_NUMBER - 1;
        }
        model_id = protect_sets->sets[g_app_fast_pair_context.nvkey.selected_set].model_id;
    }
    if (protect_sets != NULL) {
        vPortFree(protect_sets);
    }

    /* swap byte1 & byte 3 */
    uint8_t *bytes = (uint8_t *)&model_id;
    uint8_t temp = bytes[2];
    bytes[2] = bytes[0];
    bytes[0] = temp;
    return model_id;
}


static bool app_fast_pair_bt_takeover_allow(const bt_bd_addr_t device_addr)
{
#ifdef AIR_BT_FAST_PAIR_SASS_ENABLE
    if (memcmp(g_app_fast_pair_context.drop_target, empty_addr, sizeof(bt_bd_addr_t)) != 0) {
        if (memcmp(g_app_fast_pair_context.drop_target, device_addr, sizeof(bt_bd_addr_t)) == 0) {
            return true;
        } else {
            return false;
        }
    }
#endif
#if APP_ONLY_ALLOW_TAKE_OVER_BY_FAST_PAIR
    return g_app_fast_pair_context.waiting_connect;
#else
    return true;
#endif
}

/**
* @brief      This function process the events of EVENT_GROUP_UI_SHELL_SYSTEM group.
* @param[in]  self, the context pointer of the activity.
* @param[in]  event_id, the current event ID to be handled.
* @param[in]  extra_data, extra data pointer of the current event, NULL means there is no extra data.
* @param[in]  data_len, the length of the extra data. 0 means extra_data is NULL.
* @return     If return true, the current event cannot be handle by the next activity.
*/
extern void bt_fast_pair_at_cmd_init();

static bool app_proc_fast_pair_shell_group(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = true; /* UI shell internal event must process by this activity, so default is true. */
    switch (event_id) {
        case EVENT_ID_SHELL_SYSTEM_ON_CREATE: {
#ifdef AIR_SPOT_ENABLE
            bt_fast_pair_spot_set_adv_callback(app_fast_pair_spot_adv_callback);
#endif
            memset(&g_app_fast_pair_context, 0, sizeof(app_fast_pair_local_context_t));
            app_bt_takeover_service_user_register(APP_BT_TAKEOVER_ID_FASTPAIR, app_fast_pair_bt_takeover_allow);
            ret = app_fast_pair_init(self);
            if (ret == true) {
                /* BT event is not suitable for UI shell scheme.
                * Instead, choose bt callback manager to handle bt related event. */

#if defined(MTK_BATTERY_MANAGEMENT_ENABLE)
#if AIR_TWS_ENABLE
                uint8_t battery_item = (AUDIO_CHANNEL_R == ami_get_audio_channel() ? APP_FAST_PAIR_COMPONENT_RIGHT : APP_FAST_PAIR_COMPONENT_LEFT);
#else
                uint8_t battery_item = 0;
#endif
                bt_fast_pair_battery_t update_battery = g_app_fast_pair_context.battery;
                update_battery.battery[battery_item].charging = battery_management_get_battery_property(BATTERY_PROPERTY_CHARGER_EXIST);
                update_battery.battery[battery_item].battery_value = battery_management_get_battery_property(BATTERY_PROPERTY_CAPACITY);
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DCHS_MODE_MASTER_ENABLE)
                update_battery.battery[battery_item].battery_value = apps_events_get_optimal_battery();
#endif

#ifdef AIR_SMART_CHARGER_ENABLE
                uint8_t case_battery = app_smcharger_read_case_battery_nvkey();
                update_battery.battery[APP_FAST_PAIR_COMPONENT_CASE].charging = ((case_battery & 0x80) ? 1 : 0);
                update_battery.battery[APP_FAST_PAIR_COMPONENT_CASE].battery_value = (case_battery & 0x7F);
#endif
                app_fast_pair_update_battery(&g_app_fast_pair_context, &update_battery);
#endif
                bt_fast_pair_at_cmd_init();
            }
            break;
        }
        default:
            break;
    }
    return ret;
}

/**
* @brief      This function process the events of EVENT_GROUP_UI_SHELL_BATTERY group.
* @param[in]  self, the context pointer of the activity.
* @param[in]  event_id, the current event ID to be handled.
* @param[in]  extra_data, extra data pointer of the current event, NULL means there is no extra data.
* @param[in]  data_len, the length of the extra data. 0 means extra_data is NULL.
* @return     If return true, the current event cannot be handle by the next activity.
*/
static bool app_fast_pair_proc_battery_group(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;
    assert(NULL != self);
#ifdef AIR_TWS_ENABLE
    uint8_t battery_item = (AUDIO_CHANNEL_R == ami_get_audio_channel() ? APP_FAST_PAIR_COMPONENT_RIGHT : APP_FAST_PAIR_COMPONENT_LEFT);
#else
    uint8_t battery_item = 0;
#endif
    bt_fast_pair_battery_t update_battery = g_app_fast_pair_context.battery;
    switch (event_id) {
        case APPS_EVENTS_BATTERY_PERCENT_CHANGE: {
            update_battery.battery[battery_item].battery_value = (uint32_t)extra_data;
            break;
        }
        case APPS_EVENTS_BATTERY_CHARGER_EXIST_CHANGE: {
#if 0
            APPS_LOG_MSGID_I(LOG_TAG"APPS_EVENTS_BATTERY_CHARGER_EXIST_CHANGE : %d", 1, (uint32_t)extra_data);
            g_app_fast_pair_context.battery.battery[battery_item].charging = (uint32_t)extra_data;
            if (!(g_app_fast_pair_context.battery.battery[APP_FAST_PAIR_COMPONENT_RIGHT].charging) &&
                !(g_app_fast_pair_context.battery.battery[APP_FAST_PAIR_COMPONENT_LEFT].charging)) {
                g_app_fast_pair_context.battery.ui_show = false;
            } else {
                g_app_fast_pair_context.battery.ui_show = true;
            }
#endif
        }
        return ret;
        default:
            return ret;
    }
    app_fast_pair_update_battery(&g_app_fast_pair_context, &update_battery);
    return ret;
}

static bool app_fast_pair_proc_bt_group(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;

    switch (event_id) {
        case BT_GAP_LE_RPA_ROTAION_IND: {
            bt_gap_le_rpa_rotation_ind_t *rotation = NULL;
            apps_bt_event_data_t *bt_event_data = (apps_bt_event_data_t *)extra_data;
            if (bt_event_data != NULL) {
                rotation = (bt_gap_le_rpa_rotation_ind_t *)bt_event_data->buffer;
            }
            if (rotation != NULL) {
                bt_gap_le_advertising_handle_t adv_handle = 0;
                APPS_LOG_MSGID_W(LOG_TAG"BT_GAP_LE_RPA_ROTAION_IND, handle %d?=%d, %02X:%02X:%02X:%02X:%02X:%02X", 8,
                                 rotation->handle, adv_handle,
                                 rotation->address[5], rotation->address[4], rotation->address[3],
                                 rotation->address[2], rotation->address[1], rotation->address[0]);
                if (multi_ble_adv_manager_get_random_addr_and_adv_handle(MULTI_ADV_INSTANCE_FAST_PAIR, NULL, &adv_handle)
                    && (rotation->handle == adv_handle)) {
                    bt_fast_pair_set_random_ble_address(true, &(rotation->address));
                }
            }
            break;
        }
        default:
            break;
    }
    return ret;
}

/**
* @brief      This function process the events of EVENT_GROUP_UI_SHELL_BT_SINK group.
* @param[in]  self, the context pointer of the activity.
* @param[in]  event_id, the current event ID to be handled.
* @param[in]  extra_data, extra data pointer of the current event, NULL means there is no extra data.
* @param[in]  data_len, the length of the extra data. 0 means extra_data is NULL.
* @return     If return true, the current event cannot be handle by the next activity.
*/
static bool app_fast_pair_proc_bt_sink_group(struct _ui_shell_activity *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;
    switch (event_id) {
#if defined AIR_BT_FAST_PAIR_SASS_ENABLE
#if defined AIR_MULTI_POINT_ENABLE
        case BT_SINK_SRV_EVENT_PLAYING_DEVICE_CHANGE:
            if (extra_data != NULL) {
                bt_sink_srv_device_state_t *device_state = (bt_sink_srv_device_state_t *)extra_data;
                bt_bd_addr_t *addr = &(device_state->address);
                bt_fast_pair_sass_multipoint_audio_src_switch_reason_t switch_reason;

                if (device_state->music_state == BT_SINK_SRV_STATE_STREAMING) {
                    switch_reason = BT_FAST_PAIR_SASS_MULTIPOINT_AUDIO_SRC_SWITCH_REASON_A2DP;
                } else if (device_state->call_state >= BT_SINK_SRV_STATE_INCOMING || device_state->sco_state == BT_SINK_SRV_SCO_CONNECTION_STATE_CONNECTED) {
                    switch_reason = BT_FAST_PAIR_SASS_MULTIPOINT_AUDIO_SRC_SWITCH_REASON_HFP;
                } else {
                    switch_reason = BT_FAST_PAIR_SASS_MULTIPOINT_AUDIO_SRC_SWITCH_REASON_UNSPECIFIED;
                }
                APPS_LOG_MSGID_I(LOG_TAG"BT_SINK_SRV_EVENT_PLAYING_DEVICE_CHANGE reason : %d", 1, switch_reason);
                bt_fast_pair_sass_update_multipoint_audio_source_switch(addr, switch_reason);
                /*
                g_app_fast_pair_context.switch_reason = switch_reason;
                memcpy(g_app_fast_pair_context.switch_done_target_addr, *addr, sizeof(bt_bd_addr_t));
                */

            }
            break;
#endif
        case BT_SINK_SRV_EVENT_AVRCP_STATUS_CHANGE: {
            if (apps_config_key_get_mmi_state() == APP_A2DP_PLAYING
#ifdef MTK_AWS_MCE_ENABLE
                && (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT || bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_NONE)
#endif
               ) {
                app_fast_pair_connection_state_change();
            }
            break;
        }
        case BT_SINK_SRV_EVENT_STATE_CHANGE: {
            if (APP_A2DP_PLAYING == apps_config_key_get_mmi_state()) {
                bt_sink_srv_state_change_t *param = (bt_sink_srv_state_change_t *) extra_data;
                APPS_LOG_MSGID_I(LOG_TAG" Re-update the connection state when Sink state change", 0);
                /* Re-update the connection state because the last time maybe the active address is NULL. */
                if ((param->previous != BT_SINK_SRV_STATE_STREAMING) && (param->current == BT_SINK_SRV_STATE_STREAMING)) {
                    app_fast_pair_connection_state_change();
                }
            }
            break;
        }
#ifdef AIR_LE_AUDIO_ENABLE
        case LE_SINK_SRV_EVENT_REMOTE_INFO_UPDATE: {
            bt_le_sink_srv_event_remote_info_update_t *update_ind = (bt_le_sink_srv_event_remote_info_update_t *)extra_data;
            if (update_ind == NULL) {
                break;
            }
            if (update_ind->pre_state == BT_BLE_LINK_CONNECTED && update_ind->state == BT_BLE_LINK_DISCONNECTED) {
                if (memcmp(update_ind->address.addr, g_app_fast_pair_context.drop_target, sizeof(g_app_fast_pair_context.drop_target)) == 0) {
                    memset(g_app_fast_pair_context.drop_target, 0, sizeof(g_app_fast_pair_context.drop_target));
                    APPS_LOG_MSGID_I(LOG_TAG"LEA disconnect, remove drop target.", 0);
                    const bt_device_manager_link_record_t *link_info = bt_device_manager_link_record_get_connected_link();
                    if (link_info && link_info->connected_num > 0) {
                        app_fast_pair_connection_state_change();
                    }
                }
            } else if (update_ind->pre_state == BT_BLE_LINK_DISCONNECTED && update_ind->state == BT_BLE_LINK_CONNECTED) {
#if defined AIR_MULTI_POINT_ENABLE
                APPS_LOG_MSGID_I(LOG_TAG"LEA connect, and connected from SP, remember the temp_allow_switch_audio_addr.", 0);
                memcpy(g_app_fast_pair_context.temp_allow_switch_audio_addr, update_ind->address.addr, sizeof(bt_bd_addr_t));
                ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                      APPS_EVENTS_INTERACTION_FAST_PAIR_SASS_CLEAR_TEMP_ALLOW);
                ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                    APPS_EVENTS_INTERACTION_FAST_PAIR_SASS_CLEAR_TEMP_ALLOW, NULL, 0, NULL, 5000);
#endif
                if (g_app_fast_pair_context.connection_switch_back_event == BT_FAST_PAIR_SASS_SWITCH_BACK_EVENT_RESUME_PLAYING) {
                    bt_sink_srv_action_t sink_action = BT_SINK_SRV_ACTION_PLAY;
                    bt_sink_srv_send_action(sink_action, NULL);
                    g_app_fast_pair_context.connection_switch_back_event = 0;
                }
            }
            break;
        }
        case BT_SINK_SRV_EVENT_LE_STREAMING_CONTEXT_UPDATE: {
            bt_sink_srv_le_streaming_context_update_t *stream_update = (bt_sink_srv_le_streaming_context_update_t *)extra_data;
            if (stream_update == NULL) {
                break;
            }
            APPS_LOG_MSGID_I(LOG_TAG"BT_SINK_SRV_EVENT_LE_STREAMING_CONTEXT_UPDATE 0x%X->%X", 2, stream_update->pre_context, stream_update->cur_context);
            if (AUDIO_CONTENT_TYPE_GAME & (stream_update->pre_context ^ stream_update->cur_context)) {
                if (apps_config_key_get_mmi_state() == APP_A2DP_PLAYING) {
                    app_fast_pair_connection_state_change();
                }
            }
            break;
        }

#endif
#endif  /* #if defined AIR_BT_FAST_PAIR_SASS_ENABLE */
        default:
            break;
    }
    return ret;
}

/**
* @brief      This function process the events of EVENT_GROUP_UI_SHELL_BT_CONN_MANAGER group.
* @param[in]  self, the context pointer of the activity.
* @param[in]  event_id, the current event ID to be handled.
* @param[in]  extra_data, extra data pointer of the current event, NULL means there is no extra data.
* @param[in]  data_len, the length of the extra data. 0 means extra_data is NULL.
* @return     If return true, the current event cannot be handle by the next activity.
*/
static bool app_fast_pair_proc_bt_cm_group(struct _ui_shell_activity *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;
    /* APPS_LOG_MSGID_I("[APP_FAST_PAIR], bt cm event_id : 0x%x", 1, event_id); */
#ifdef MTK_AWS_MCE_ENABLE
    /* Only handle bt cm related event in Agent role */
    bt_aws_mce_role_t aws_role = bt_connection_manager_device_local_info_get_aws_role();
    if ((BT_AWS_MCE_ROLE_PARTNER | BT_AWS_MCE_ROLE_CLINET) & aws_role) {
        APPS_LOG_MSGID_W("[APP_FAST_PAIR] Don't init on wrongly role: %d", 1, aws_role);
    }
#endif
    switch (event_id) {
        case BT_CM_EVENT_VISIBILITY_STATE_UPDATE: {
#ifdef MTK_AWS_MCE_ENABLE
            if ((BT_AWS_MCE_ROLE_PARTNER | BT_AWS_MCE_ROLE_CLINET) & aws_role) {
                return ret;
            }
#endif
            bt_cm_visibility_state_update_ind_t *visible_update = (bt_cm_visibility_state_update_ind_t *)extra_data;
            APPS_LOG_MSGID_I("[APP_FAST_PAIR] Visibilty update chage fast pair data.", 0);
            app_fast_pair_trigger_advertising(&g_app_fast_pair_context);
#ifdef AIR_SPOT_ENABLE
            bt_fast_pair_spot_update_pairing_mode_state(visible_update->visibility_state);
#endif
        }
        break;
        case BT_CM_EVENT_REMOTE_INFO_UPDATE: {
            bt_cm_remote_info_update_ind_t *remote_update = (bt_cm_remote_info_update_ind_t *)extra_data;
#ifdef AIR_TWS_ENABLE
            if ((BT_AWS_MCE_ROLE_PARTNER | BT_AWS_MCE_ROLE_CLINET) & aws_role) {
#if defined(AIR_BT_FAST_PAIR_LE_AUDIO_ENABLE)
                if (BT_AWS_MCE_ROLE_PARTNER & aws_role) {
                    if (!(remote_update->pre_connected_service & BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS)) &&
                        (remote_update->connected_service & BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS))) {
                        bt_fast_pair_sass_on_aws_state_change(true);
                    } else if ((remote_update->pre_connected_service & BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS)) &&
                               !(remote_update->connected_service & BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS))) {
                        bt_fast_pair_sass_on_aws_state_change(false);
                        bt_fast_pair_set_le_io_capability(false);
                    }
                }
#endif
                return ret;
            }
#endif
            bt_cm_profile_service_mask_t profile_srv = BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP) | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK);
            if ((remote_update->pre_connected_service & profile_srv) && !(remote_update->connected_service & profile_srv)) {
                /* For fast pair service add workaround, if HFP and A2DP all disconnected, then we need disconnected fast pair service actively. */
                /* bt_fast_pair_terminate(&(remote_update->address)); */
                bt_cm_connect_t disconnect_param;
                memcpy(disconnect_param.address, remote_update->address, sizeof(bt_bd_addr_t));
                disconnect_param.profile = BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_CUSTOMIZED_FAST_PAIR);
                bt_cm_disconnect(&disconnect_param);
            }
#ifdef AIR_TWS_ENABLE
            if (BT_CM_ACL_LINK_CONNECTED <= remote_update->acl_state &&
                !(remote_update->pre_connected_service & BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS)) &&
                (remote_update->connected_service & BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS))) {
                uint32_t size_account = sizeof(bt_fast_pair_account_key_list_t) +
                                        sizeof(bt_fast_pair_account_key_t) * (g_app_fast_pair_context.account_key_list->max_key_number - 1);
                /* Only agent role will enable this module, so don't need judge the role
                           * in event handle. */
                apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_BT_FAST_PAIR,
                                               BT_FAST_PAIR_APP_EVENT_NEED_STORE_ACCOUNT_KEY, g_app_fast_pair_context.account_key_list, size_account);
                apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_BT_FAST_PAIR,
                                               APP_FAST_PAIR_AWS_CONTEXT_SYNC_ID, &(g_app_fast_pair_context.nvkey), sizeof(g_app_fast_pair_context.nvkey) - sizeof(g_app_fast_pair_context.nvkey.reserved));

#ifdef AIR_BT_FAST_PAIR_SASS_ENABLE
                const bt_fast_pair_sass_recorded_data_t *p_recorded_data;
                bt_fast_pair_sass_get_internal_data(NULL, &p_recorded_data);
#if defined(AIR_BT_FAST_PAIR_LE_AUDIO_ENABLE)
                bt_fast_pair_sass_on_aws_state_change(true);
#else
                apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_BT_FAST_PAIR,
                                               BT_FAST_PAIR_APP_EVENT_SASS_UPDATE_IN_USE_ACCOUNT,
                                               &p_recorded_data->inused_account_key_id,
                                               sizeof(p_recorded_data->inused_account_key_id));
#endif
                apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_BT_FAST_PAIR,
                                               BT_FAST_PAIR_APP_EVENT_SASS_UPDATE_CONNECTED_RECORD,
                                               p_recorded_data->connected_record_array,
                                               sizeof(bt_bd_addr_t) * p_recorded_data->connected_record_length);
#endif
            } else if ((remote_update->pre_connected_service & BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS)) &&
                       !(remote_update->connected_service & BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS))) {
                uint8_t peer_battery_item = (AUDIO_CHANNEL_R != ami_get_audio_channel() ? APP_FAST_PAIR_COMPONENT_RIGHT : APP_FAST_PAIR_COMPONENT_LEFT);
                bt_fast_pair_battery_t update_battery = g_app_fast_pair_context.battery;
                update_battery.battery[peer_battery_item].battery_value = 0xFF;
#if defined(AIR_BT_FAST_PAIR_LE_AUDIO_ENABLE)
                bt_fast_pair_sass_on_aws_state_change(false);
#endif
                app_fast_pair_update_battery(&g_app_fast_pair_context, &update_battery);
            }
#endif
#ifdef AIR_BT_FAST_PAIR_SASS_ENABLE
            /*Delete the drop_target. */
            if (BT_CM_ACL_LINK_CONNECTED <= remote_update->pre_acl_state && BT_CM_ACL_LINK_CONNECTED > remote_update->acl_state) {
                if (memcmp(remote_update->address, g_app_fast_pair_context.drop_target, sizeof(g_app_fast_pair_context.drop_target)) == 0) {
                    memset(g_app_fast_pair_context.drop_target, 0, sizeof(g_app_fast_pair_context.drop_target));
                    APPS_LOG_MSGID_I(LOG_TAG"[APP_FAST_PAIR] ACL disconnect, remove drop target.", 0);
                }

                const bt_device_manager_link_record_t *link_info = bt_device_manager_link_record_get_connected_link();
                if (link_info && link_info->connected_num > 0) {
                    app_fast_pair_connection_state_change();
                }
            } else if (BT_CM_ACL_LINK_DISCONNECTED == remote_update->pre_acl_state && BT_CM_ACL_LINK_CONNECTED <= remote_update->acl_state) {
#if defined AIR_MULTI_POINT_ENABLE
                APPS_LOG_MSGID_I(LOG_TAG"ACL connect, and connected from SP, remember the temp_allow_switch_audio_addr.", 0);
                memcpy(g_app_fast_pair_context.temp_allow_switch_audio_addr, remote_update->address, sizeof(bt_bd_addr_t));
                ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                      APPS_EVENTS_INTERACTION_FAST_PAIR_SASS_CLEAR_TEMP_ALLOW);
                ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                    APPS_EVENTS_INTERACTION_FAST_PAIR_SASS_CLEAR_TEMP_ALLOW, NULL, 0, NULL, 30000);
#endif
                const bt_device_manager_link_record_t *link_info = bt_device_manager_link_record_get_connected_link();
                if (link_info && link_info->connected_num > 0) {
                    app_fast_pair_connection_state_change();
                }
            }
            if ((~remote_update->pre_connected_service) & remote_update->connected_service
                & (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK) | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AVRCP))
                && remote_update->connected_service & BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK)
                && remote_update->connected_service & BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AVRCP)) {
                APPS_LOG_MSGID_I(LOG_TAG"A2DP/AVRCP connected, need resume music event : %d", 1, g_app_fast_pair_context.connection_switch_back_event);
                if (g_app_fast_pair_context.connection_switch_back_event == BT_FAST_PAIR_SASS_SWITCH_BACK_EVENT_RESUME_PLAYING) {
                    bt_sink_srv_action_t sink_action = BT_SINK_SRV_ACTION_PLAY;
                    bt_sink_srv_send_action(sink_action, NULL);
                    g_app_fast_pair_context.connection_switch_back_event = 0;
                }
#if defined AIR_MULTI_POINT_ENABLE
                APPS_LOG_MSGID_I(LOG_TAG"both of A2DP and AVRCP connected. update temp_allow_switch_audio_addr timer", 0);
                memcpy(g_app_fast_pair_context.temp_allow_switch_audio_addr, remote_update->address, sizeof(bt_bd_addr_t));
                ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                      APPS_EVENTS_INTERACTION_FAST_PAIR_SASS_CLEAR_TEMP_ALLOW);
                ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                    APPS_EVENTS_INTERACTION_FAST_PAIR_SASS_CLEAR_TEMP_ALLOW, NULL, 0, NULL, 5000);
#endif
            }
#endif
            break;
        }
        default:
            break;
    }
    return ret;
}

/**
* @brief      This function process the events of EVENT_GROUP_UI_SHELL_BT_CONN_MANAGER group.
* @param[in]  self, the context pointer of the activity.
* @param[in]  event_id, the current event ID to be handled.
* @param[in]  extra_data, extra data pointer of the current event, NULL means there is no extra data.
* @param[in]  data_len, the length of the extra data. 0 means extra_data is NULL.
* @return     If return true, the current event cannot be handle by the next activity.
*/
static bool app_fast_pair_proc_bt_dm_group(struct _ui_shell_activity *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;

    bt_device_manager_power_event_t evt;
    bt_device_manager_power_status_t status;
    bt_event_get_bt_dm_event_and_status(event_id, &evt, &status);
    switch (evt) {
        case BT_DEVICE_MANAGER_POWER_EVT_ACTIVE_COMPLETE: {
            if (BT_DEVICE_MANAGER_POWER_STATUS_SUCCESS == status && !apps_config_features_is_mp_test_mode()) {
#ifdef AIR_TILE_ENABLE
                g_app_fast_pair_context.is_classic_bt_on = true;
#endif
#ifdef AIR_SPOT_ENABLE
                uint32_t interval = 15 * 60 * 1000;
                if (g_app_fast_pair_context.spot_enabled) {
                    interval = s_unwanted_tracking_mode_enable ? FAST_PAIR_SPOT_UNWANTED_MODE_ROTATE_TIME_MS : FAST_PAIR_SPOT_ROTATE_TIME_MS;
                }
                app_fast_pair_generate_random_address(interval);
#else
                app_fast_pair_generate_random_address(15 * 60 * 1000);
#endif
                app_fast_pair_trigger_advertising(&g_app_fast_pair_context);
            }
            break;
        }
#ifdef AIR_TILE_ENABLE
        case BT_DEVICE_MANAGER_POWER_EVT_CLASSIC_ACTIVE_COMPLETE: {
            if (BT_DEVICE_MANAGER_POWER_STATUS_SUCCESS == status && !apps_config_features_is_mp_test_mode()) {
                g_app_fast_pair_context.is_classic_bt_on = true;
                app_fast_pair_trigger_advertising(&g_app_fast_pair_context);
            }
            break;
        }
#endif
        case BT_DEVICE_MANAGER_POWER_EVT_STANDBY_COMPLETE: {
            if (BT_DEVICE_MANAGER_POWER_RESET_TYPE_NORMAL == status && !apps_config_features_is_mp_test_mode()) {
                multi_ble_adv_manager_remove_ble_adv(MULTI_ADV_INSTANCE_FAST_PAIR, app_fast_pair_get_adv_data);
            }
            break;
        }
        default:
            break;
    }

    return ret;
}

#ifdef MTK_AWS_MCE_ENABLE
/**
* @brief      This function process the events of EVENT_GROUP_UI_SHELL_AWS group.
* @param[in]  self, the context pointer of the activity.
* @param[in]  event_id, the current event ID to be handled.
* @param[in]  extra_data, extra data pointer of the current event, NULL means there is no extra data.
* @param[in]  data_len, the length of the extra data. 0 means extra_data is NULL.
* @return     If return true, the current event cannot be handle by the next activity.
*/
bool app_fast_pair_proc_aws_event(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;

    switch (event_id) {
        case BT_AWS_MCE_SRV_EVENT_MODE_CHANGED_IND:
            app_fast_pair_update_parameter();
            break;
        default:
            break;
    }
    return ret;
}
#endif


/**
* @brief      This function process the events of EVENT_GROUP_UI_SHELL_APP_INTERACTION.
* @param[in]  self, the context pointer of the activity.
* @param[in]  event_id, the current event ID to be handled.
* @param[in]  extra_data, extra data pointer of the current event, NULL means there is no extra data.
* @param[in]  data_len, the length of the extra data. 0 means extra_data is NULL.
* @return     If return true, the current event cannot be handle by the next activity.
*/

bool app_fast_pair_proc_app_interaction_event(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;

    switch (event_id) {
        case APPS_EVENTS_INTERACTION_SPOT_RING_OVER: {
#ifdef AIR_SPOT_ENABLE
            s_current_ring_type = BT_FAST_PAIR_SPOT_RING_TYPE_NONE;
            s_remain_time = 0;
            bt_fast_pair_spot_ring_state_update(BT_FAST_PAIR_SPOT_RING_STOPPED_BY_TIMEOUT, s_current_ring_type, 0);
#endif
            ret = true;
            break;
        }
#ifdef AIR_TILE_ENABLE
        case APPS_EVENTS_INTERACTION_STOP_FAST_PAIR_ADV: {
            app_fast_pair_enable_advertising(FALSE);
#ifdef AIR_TILE_ENABLE
            g_app_fast_pair_context.is_classic_bt_on = false;
            APPS_LOG_MSGID_I(LOG_TAG"APPS_EVENTS_INTERACTION_STOP_FAST_PAIR_ADV, classic bt on %d", 1, g_app_fast_pair_context.is_classic_bt_on);
#endif
            break;
        }
#endif
#ifdef RACE_FIND_ME_ENABLE
        case APPS_EVENTS_INTERACTION_NOTIFY_FIND_ME_STATE: {
            const app_find_me_context_t *fm_context = app_find_me_idle_activity_get_context();
            if (fm_context->tone != s_app_fast_pair_find_me_state.tone
#ifdef AIR_TWS_ENABLE
                || fm_context->peer_tone != s_app_fast_pair_find_me_state.peer_tone
#endif
               ) {
                s_app_fast_pair_find_me_state.tone = fm_context->tone;
#ifdef AIR_TWS_ENABLE
                s_app_fast_pair_find_me_state.peer_tone = fm_context->peer_tone;
#endif
                app_fast_pair_notify_ring_state_change();
            }
            break;
        }
#endif
#ifdef AIR_BT_FAST_PAIR_SASS_ENABLE
#if defined AIR_MULTI_POINT_ENABLE
        case APPS_EVENTS_INTERACTION_FAST_PAIR_CLEAR_ACTIVE_SWITCH_RECORD: {
            memset(g_app_fast_pair_context.allow_switch_audio_addr, 0, sizeof(bt_bd_addr_t));
            break;
        }
        case APPS_EVENTS_INTERACTION_FAST_PAIR_SASS_CLEAR_TEMP_ALLOW: {
            memset(g_app_fast_pair_context.temp_allow_switch_audio_addr, 0, sizeof(bt_bd_addr_t));
            break;
        }
#endif
        case APPS_EVENTS_INTERACTION_FAST_PAIR_SASS_UPDATE_CONNECTION_STATUS: {
            void app_fast_pair_connection_state_change(void);
            break;
        }
#ifdef MTK_IN_EAR_FEATURE_ENABLE
        case APPS_EVENTS_INTERACTION_IN_EAR_UPDATE_STA: {
            app_in_ear_sta_info_t *sta_info = (app_in_ear_sta_info_t *)extra_data;
            APPS_LOG_MSGID_I(LOG_TAG", [IN_EAR_UPDATE] %d->%d", 2, sta_info->previous, sta_info->current);
            if ((sta_info->previous == APP_IN_EAR_STA_BOTH_OUT && sta_info->current != APP_IN_EAR_STA_BOTH_OUT)
                || (sta_info->previous != APP_IN_EAR_STA_BOTH_OUT && sta_info->current == APP_IN_EAR_STA_BOTH_OUT)) {
                app_fast_pair_connection_state_change();
            }
            break;
        }
#endif
#if defined AIR_MULTI_POINT_ENABLE
        case APPS_EVENTS_INTERACTION_MULTI_POINT_STATE_CHANGED: {
            uint16_t capability = bt_fast_pair_sass_get_capability_flags();
            bt_fast_pair_sass_update_capability(capability, true);
            break;
        }
#endif
#endif
        case APPS_EVENTS_INTERACTION_FAST_PAIR_GENERATE_ADV_ADDR: {
#ifdef AIR_SPOT_ENABLE
            if (g_app_fast_pair_context.spot_enabled) {
                app_fast_pair_gen_eid_and_trigger_adv_async();
            } else {
                app_fast_pair_generate_random_address(15 * 60 * 1000);
                app_fast_pair_trigger_advertising(&g_app_fast_pair_context);
            }
#else
            app_fast_pair_generate_random_address(15 * 60 * 1000);
            app_fast_pair_trigger_advertising(&g_app_fast_pair_context);
#endif
            break;
        }
#ifdef AIR_SPOT_ENABLE
        case APPS_EVENTS_INTERACTION_SPOT_GEN_EID_IN_UNWANTED_TRACKING_MODE: {
            app_fast_pair_gen_eid();
            uint32_t rotate_time = app_fast_pair_spot_gen_rotate_time_ms(FAST_PAIR_SPOT_ROTATE_TIME_MS);
            ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_SPOT_GEN_EID_IN_UNWANTED_TRACKING_MODE, NULL, 0, NULL, rotate_time);
        }
#endif
        default:
            break;
    }
    return ret;
}

#ifdef AIR_SPOT_ENABLE
static bool app_line_in_handle_key_event(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    uint16_t action_id = 0;
    if (extra_data) {
        action_id = *(uint16_t *)extra_data;
        if (action_id == KEY_FIND_MY_STOP_RING) {
            bt_fast_pair_spot_ring_state_update(BT_FAST_PAIR_SPOT_RING_STOPPED_BY_BTN, s_current_ring_type, 0);
            app_fast_spot_stop_find_me();
        }
    }
    return false;
}
#endif

bool app_fast_pair_idle_activity_proc(struct _ui_shell_activity *self, uint32_t event_group, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;
    /* APPS_LOG_MSGID_I("[APP_FAST_PAIR]APP_FAST_PAIR_IDLE_ACT, event_group : %x, evt: %x", 2, event_group, event_id); */
    switch (event_group) {
        case EVENT_GROUP_UI_SHELL_SYSTEM: {
            ret = app_proc_fast_pair_shell_group(self, event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_BATTERY: {
            ret = app_fast_pair_proc_battery_group(self, event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_BT:
            ret = app_fast_pair_proc_bt_group(self, event_id, extra_data, data_len);
            break;
        case EVENT_GROUP_UI_SHELL_BT_SINK: {
            ret = app_fast_pair_proc_bt_sink_group(self, event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_BT_CONN_MANAGER: {
            /* Trigger fast pair behavior */
            ret = app_fast_pair_proc_bt_cm_group(self, event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_BT_DEVICE_MANAGER: {
            /* Trigger fast pair behavior */
            ret = app_fast_pair_proc_bt_dm_group(self, event_id, extra_data, data_len);
            break;
        }
#ifdef MTK_AWS_MCE_ENABLE
        case EVENT_GROUP_UI_SHELL_AWS:
            ret = app_fast_pair_proc_aws_event(self, event_id, extra_data, data_len);
            break;
#endif
#ifdef AIR_SMART_CHARGER_ENABLE
        case EVENT_GROUP_UI_SHELL_CHARGER_CASE:
            ret = app_fast_pair_proc_charger_case_group(self, event_id, extra_data, data_len);
            break;
#endif
        case EVENT_GROUP_UI_SHELL_BT_FAST_PAIR: {
            ret = app_fast_pair_app_event_callback(self, event_id, extra_data, data_len);
            break;
        }
#ifdef AIR_TWS_ENABLE
        case EVENT_GROUP_UI_SHELL_AWS_DATA: {
            /* Just group currently */
            ret = app_fast_pair_aws_mce_data_callback(self, event_id, extra_data, data_len);
            break;
        }
#endif
        case EVENT_GROUP_UI_SHELL_MULTI_VA:
            ret = app_fast_pair_multi_va_event_proc(self, event_id, extra_data, data_len);
            break;

        case EVENT_GROUP_UI_SHELL_APP_INTERACTION:
            ret = app_fast_pair_proc_app_interaction_event(self, event_id, extra_data, data_len);
            break;
#ifdef AIR_SPOT_ENABLE
        case EVENT_GROUP_UI_SHELL_KEY:
            ret =app_line_in_handle_key_event(self, event_id, extra_data, data_len);
            break;
#endif
        default:
            break;
    }

    /* APPS_LOG_MSGID_I("[APP_FAST_PAIR]APP_FAST_PAIR_IDLE_ACT, ret : %x", 1, ret); */
    return ret;
}

/**
 * @brief     The implement of weak symbol function that returns the account key list buffer to fast pair module.
 * @return    A pointer to the account key buffer containing the account key buffer numbers and account key buffer. The pointer should not be NULL and should point to a global variable.
 */
bt_fast_pair_account_key_list_t *bt_fast_pair_get_account_key_list()
{
    return g_app_fast_pair_context.account_key_list;
}

void app_fast_pair_set_tx_power_level(int8_t tx_power)
{
    nvkey_status_t status = 0;
    app_fast_pair_nvkey_t temp_nvkey;
    uint32_t nvkey_size = sizeof(app_fast_pair_nvkey_t);
    if (NVKEY_STATUS_OK != (status = nvkey_read_data(NVID_APP_FAST_PAIR_CONFIGURE,
                                                     (uint8_t *)&temp_nvkey, &nvkey_size))) {
        return;
    }
    g_app_fast_pair_context.nvkey.tx_power_level = tx_power;
    temp_nvkey.tx_power_level = tx_power;
    if (NVKEY_STATUS_OK != (status = nvkey_write_data(NVID_APP_FAST_PAIR_CONFIGURE,
                                                      (uint8_t *)&temp_nvkey, sizeof(app_fast_pair_nvkey_t)))) {
    }
}

void app_fast_pair_enable_advertising(bool enable)
{
    APPS_LOG_MSGID_I(LOG_TAG"[CUST_PAIR] enable_advertising, enable=%d", 1, enable);

    if (enable) {
        app_fast_pair_trigger_advertising(&g_app_fast_pair_context);
    } else {
        multi_ble_adv_manager_remove_ble_adv(MULTI_ADV_INSTANCE_FAST_PAIR, app_fast_pair_get_adv_data);
        multi_ble_adv_manager_notify_ble_adv_data_changed(MULTI_ADV_INSTANCE_FAST_PAIR);
    }
}

bool app_fast_pair_get_is_waiting_connect(void)
{
#if APP_ONLY_ALLOW_TAKE_OVER_BY_FAST_PAIR
    return g_app_fast_pair_context.waiting_connect;
#else
    return true;
#endif
}

#ifdef AIR_BT_FAST_PAIR_SASS_ENABLE
void app_fast_pair_connection_state_change(void)
{
#if defined AIR_MULTI_POINT_ENABLE
#if 0
    bool need_notify_switch = false;
    if (memcmp(g_app_fast_pair_context.switch_done_target_addr, empty_addr, sizeof(bt_bd_addr_t)) != 0) {
        if (g_app_fast_pair_context.switch_reason == BT_FAST_PAIR_SASS_MULTIPOINT_AUDIO_SRC_SWITCH_REASON_A2DP) {
            if (apps_config_key_get_mmi_state() == APP_A2DP_PLAYING
                || apps_config_key_get_mmi_state() == APP_ULTRA_LOW_LATENCY_PLAYING
                || apps_config_key_get_mmi_state() == APP_LE_AUDIO_BIS_PLAYING) {
                need_notify_switch = true;
            }
        } else if (g_app_fast_pair_context.switch_reason == BT_FAST_PAIR_SASS_MULTIPOINT_AUDIO_SRC_SWITCH_REASON_HFP) {
            if ((apps_config_key_get_mmi_state() >= APP_HFP_INCOMING && apps_config_key_get_mmi_state() <= APP_HFP_MULTIPARTY_CALL)
                || apps_config_key_get_mmi_state() == APP_STATE_HELD_ACTIVE) {
                need_notify_switch = true;
            }
        } else {
            if (apps_config_key_get_mmi_state() >= APP_CONNECTED) {
                need_notify_switch = true;
            }
        }
        if (need_notify_switch) {
            bt_fast_pair_sass_update_multipoint_audio_source_switch(&g_app_fast_pair_context.switch_done_target_addr, g_app_fast_pair_context.switch_reason);
            memset(g_app_fast_pair_context.switch_done_target_addr, 0, sizeof(bt_bd_addr_t));
        }
    }
#endif
#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
    if (APP_A2DP_PLAYING == apps_config_key_get_mmi_state()) {
        bt_sink_srv_device_state_t device_state;
        if (BT_STATUS_SUCCESS != bt_sink_srv_get_playing_device_state(&device_state)) {
            APPS_LOG_MSGID_I(LOG_TAG" get active device failed when playing status, need wait", 0);
            return;
        }
    }
#endif
#endif
    bt_fast_pair_sass_update_connection_state(true);
    if (!app_bt_connection_service_get_current_status()->bt_visible) {
        app_fast_pair_trigger_advertising(&g_app_fast_pair_context);
    }
}

#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
bt_sink_srv_allow_result_t app_fast_pair_multi_link_get_allow_play(bt_sink_srv_device_state_t *current_device, bt_sink_srv_device_state_t *coming_device)
{
    bt_sink_srv_allow_result_t allow = BT_SINK_SRV_ALLOW_RESULT_BYPASS;
#ifdef AIR_MULTI_POINT_ENABLE
    if (current_device != NULL && coming_device != NULL) {
        uint8_t current_account_id = bt_fast_pair_sass_get_account_id_by_addr(&current_device->address);
        uint8_t coming_account_id = bt_fast_pair_sass_get_account_id_by_addr(&coming_device->address);
        APPS_LOG_MSGID_I(LOG_TAG" app_fast_pair_multi_link_get_allow_play call: 0x%x, music: 0x%x", 2,
                         coming_device->call_state, coming_device->music_state);
        APPS_LOG_MSGID_I(LOG_TAG" app_fast_pair_multi_link_get_allow_play current addr: %02X:%02X:%02X:%02X:%02X:%02X", 6,
                         current_device->address[5], current_device->address[4], current_device->address[3],
                         current_device->address[2], current_device->address[1], current_device->address[0]);
        APPS_LOG_MSGID_I(LOG_TAG" app_fast_pair_multi_link_get_allow_play coming addr: %02X:%02X:%02X:%02X:%02X:%02X", 6,
                         coming_device->address[5], coming_device->address[4], coming_device->address[3],
                         coming_device->address[2], coming_device->address[1], coming_device->address[0]);
        APPS_LOG_MSGID_I(LOG_TAG" app_fast_pair_multi_link_get_allow_play account_id %d - %d, prefer_flags : 0x%x", 3,
                         current_account_id, coming_account_id, g_app_fast_pair_context.sass_switch_prefer_flags);
#ifdef MTK_AWS_MCE_ENABLE
        if (bt_connection_manager_device_local_info_get_aws_role() & (BT_AWS_MCE_ROLE_PARTNER | BT_AWS_MCE_ROLE_CLIENT)
            && BT_AWS_MCE_SRV_LINK_NONE != bt_aws_mce_srv_get_link_type()) {
            return BT_SINK_SRV_ALLOW_RESULT_BYPASS;
        }
#endif
        if (current_account_id == 0xFF && coming_account_id == 0xFF) {
            allow = BT_SINK_SRV_ALLOW_RESULT_BYPASS;
        } else {
            allow = BT_SINK_SRV_ALLOW_RESULT_ALLOW;
            if (current_device->music_state == BT_SINK_SRV_STATE_STREAMING
                && coming_device->music_state == BT_SINK_SRV_STATE_STREAMING) {
                allow = BT_SINK_SRV_ALLOW_RESULT_ALLOW; /* Already allowed by switch active device message. */
            } else if ((current_device->call_state >= BT_SINK_SRV_STATE_INCOMING || current_device->sco_state == BT_SINK_SRV_SCO_CONNECTION_STATE_CONNECTED)
                       && (coming_device->call_state >= BT_SINK_SRV_STATE_INCOMING || coming_device->sco_state == BT_SINK_SRV_SCO_CONNECTION_STATE_CONNECTED)) {
                if (!(g_app_fast_pair_context.sass_switch_prefer_flags & (1 << BT_FAST_PAIR_SASS_MULTIPOINT_SWITCH_H_H_BIT))) {
                    allow = BT_SINK_SRV_ALLOW_RESULT_DISALLOW;
                }
            } else if ((current_device->call_state >= BT_SINK_SRV_STATE_INCOMING || current_device->sco_state == BT_SINK_SRV_SCO_CONNECTION_STATE_CONNECTED)
                       && coming_device->music_state == BT_SINK_SRV_STATE_STREAMING) {
                if (!(g_app_fast_pair_context.sass_switch_prefer_flags & (1 << BT_FAST_PAIR_SASS_MULTIPOINT_SWITCH_A_H_BIT))) {
                    allow = BT_SINK_SRV_ALLOW_RESULT_DISALLOW;
                }
            } else if (current_device->music_state == BT_SINK_SRV_STATE_STREAMING
                       && (coming_device->call_state >= BT_SINK_SRV_STATE_INCOMING || BT_SINK_SRV_SCO_CONNECTION_STATE_CONNECTED)) {
                if (!(g_app_fast_pair_context.sass_switch_prefer_flags & (1 << BT_FAST_PAIR_SASS_MULTIPOINT_SWITCH_H_A_BIT))) {
                    allow = BT_SINK_SRV_ALLOW_RESULT_DISALLOW;
                }
            }
        }
        if (allow == BT_SINK_SRV_ALLOW_RESULT_ALLOW) {
            ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                  APPS_EVENTS_INTERACTION_FAST_PAIR_SASS_UPDATE_CONNECTION_STATUS);
            ui_shell_send_event(false, EVENT_PRIORITY_MIDDLE, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_FAST_PAIR_SASS_UPDATE_CONNECTION_STATUS, NULL, 0, NULL, 100);
        }
        /*
        if (allow == BT_SINK_SRV_ALLOW_RESULT_ALLOW) {
            bt_fast_pair_sass_multipoint_audio_src_switch_reason_t switch_reason = BT_FAST_PAIR_SASS_MULTIPOINT_AUDIO_SRC_SWITCH_REASON_UNSPECIFIED;
            if (coming_device->music_state == BT_SINK_SRV_STATE_STREAMING) {
                switch_reason = BT_FAST_PAIR_SASS_MULTIPOINT_AUDIO_SRC_SWITCH_REASON_A2DP;
            } else if (coming_device->call_state >= BT_SINK_SRV_STATE_INCOMING) {
                switch_reason = BT_FAST_PAIR_SASS_MULTIPOINT_AUDIO_SRC_SWITCH_REASON_HFP;
            }
            bt_fast_pair_sass_update_multipoint_audio_source_switch(&coming_device->address, switch_reason);
        }
        */
    }
#endif
    APPS_LOG_MSGID_I(LOG_TAG" app_fast_pair_multi_link_get_allow_play result %d", 1, allow);
    return allow;
}
#endif

bool bt_fast_pair_sass_get_active_device_address(bt_bd_addr_t *addr)
{
    bool ret = false;
#ifdef AIR_MULTI_POINT_ENABLE
    bt_sink_srv_device_state_t device_state;
    bt_status_t bt_ret = BT_STATUS_FAIL;
#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
    bt_ret = bt_sink_srv_get_playing_device_state(&device_state);
#endif
    if (BT_STATUS_SUCCESS == bt_ret) {
        ret = true;
#if defined(AIR_LE_AUDIO_ENABLE)
        bt_device_manager_le_bonded_info_t *bond_info = bt_device_manager_le_get_bonding_info_by_addr_ext(&device_state.address);
        if (bond_info) {
            memcpy(*addr, bond_info->info.identity_addr.address.addr, sizeof(bt_bd_addr_t));
        } else
#endif
        {
            memcpy(*addr, device_state.address, sizeof(bt_bd_addr_t));
        }
    } else {
        ret = true;
        memset(*addr, 0, sizeof(bt_bd_addr_t));
    }
    APPS_LOG_MSGID_I(LOG_TAG" bt_fast_pair_sass_get_active_address bt_ret: 0x%x, addr:%02X:%02X:%02X:%02X:%02X:%02X", 7, bt_ret,
                     (*addr)[5], (*addr)[4], (*addr)[3], (*addr)[2], (*addr)[1], (*addr)[0]);
#else
    const bt_device_manager_link_record_t *link_info = bt_device_manager_link_record_get_connected_link();
    if (link_info && link_info->connected_num > 0 && apps_config_key_get_mmi_state() > APP_CONNECTED) {
        memcpy(*addr, link_info->connected_device[0].remote_addr, sizeof(bt_bd_addr_t));
    } else {
        memset(*addr, 0, sizeof(bt_bd_addr_t));
    }
#endif
    return ret;
}

#ifdef AIR_MULTI_POINT_ENABLE
int32_t bt_sink_srv_a2dp_int_user_conf_get(bt_bd_addr_ptr_t coming_addr, bt_bd_addr_ptr_t cur_addr)
{
    if (coming_addr == NULL || cur_addr == NULL) {
        return BT_SINK_SRV_A2DP_INT_USER_CONF_BYPASS;
    }
    if (!(g_app_fast_pair_context.sass_switch_prefer_flags & (1 << BT_FAST_PAIR_SASS_MULTIPOINT_SWITCH_A_A_BIT))) {
        uint8_t account_coming = bt_fast_pair_sass_get_account_id_by_addr((bt_bd_addr_t *)coming_addr);
        uint8_t account_cur = bt_fast_pair_sass_get_account_id_by_addr((bt_bd_addr_t *)cur_addr);
        APPS_LOG_MSGID_I(LOG_TAG" 2 SP connected SASS use the same account key? account id : %d, %d, may need set", 2, account_coming, account_cur);
        APPS_LOG_MSGID_I(LOG_TAG" bt_sink_srv_a2dp_int_user_conf_get current addr: %x:%x:%x:%x:%x:%x", 6,
                         cur_addr[0], cur_addr[1], cur_addr[2],
                         cur_addr[3], cur_addr[4], cur_addr[5]);
        APPS_LOG_MSGID_I(LOG_TAG" bt_sink_srv_a2dp_int_user_conf_get coming addr: %x:%x:%x:%x:%x:%x", 6,
                         coming_addr[0], coming_addr[1], coming_addr[2],
                         coming_addr[3], coming_addr[4], coming_addr[5]);
        if (0 == memcmp(g_app_fast_pair_context.temp_allow_switch_audio_addr, coming_addr, sizeof(bt_bd_addr_t))) {
            memset(g_app_fast_pair_context.temp_allow_switch_audio_addr, 0, sizeof(bt_bd_addr_t));
            ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_FAST_PAIR_SASS_CLEAR_TEMP_ALLOW);
            return BT_SINK_SRV_A2DP_INT_USER_CONF_ALLOW;
        } else if (account_coming == 0xFF && account_cur == 0xFF) {
            return BT_SINK_SRV_A2DP_INT_USER_CONF_BYPASS;
        } else if (account_coming == account_cur) {
            if (0 == memcmp(g_app_fast_pair_context.allow_switch_audio_addr, coming_addr, sizeof(bt_bd_addr_t))) {
                return BT_SINK_SRV_A2DP_INT_USER_CONF_ALLOW;
            } else {
                return BT_SINK_SRV_A2DP_INT_USER_CONF_PENDING;
            }
        } else {
            if (account_cur == 0xFF
                && 0 == memcmp(g_app_fast_pair_context.allow_switch_audio_addr, coming_addr, sizeof(bt_bd_addr_t))) {
                /* Special case, sometimes the account key of cur_addr is 0xFF when SPP disconnected in switching back flow. */
                return BT_SINK_SRV_A2DP_INT_USER_CONF_ALLOW;
            } else {
                return BT_SINK_SRV_A2DP_INT_USER_CONF_DISALLOW;
            }
        }
    } else {
        return BT_SINK_SRV_A2DP_INT_USER_CONF_ALLOW;
    }
}

#ifdef AIR_LE_AUDIO_ENABLE
int32_t bt_sink_srv_cap_am_lea_int_user_conf_get(bt_bd_addr_ptr_t coming_addr, bt_bd_addr_ptr_t cur_addr)
{
    APPS_LOG_MSGID_I(LOG_TAG" 2 SP LE connected SASS use the same account key", 0);
#ifdef MTK_AWS_MCE_ENABLE
    if (bt_connection_manager_device_local_info_get_aws_role() & (BT_AWS_MCE_ROLE_PARTNER | BT_AWS_MCE_ROLE_CLIENT)
        && BT_AWS_MCE_SRV_LINK_NONE != bt_aws_mce_srv_get_link_type()) {
        return BT_SINK_SRV_LEA_INT_USER_CONF_ALLOW;
    }
#endif
    bt_device_manager_le_bonded_info_t *coming_bond_info = bt_device_manager_le_get_bonding_info_by_addr_ext((bt_bd_addr_t *)coming_addr);
    bt_device_manager_le_bonded_info_t *cur_bond_info = bt_device_manager_le_get_bonding_info_by_addr_ext((bt_bd_addr_t *)cur_addr);
    int32_t user_conf_ret = bt_sink_srv_a2dp_int_user_conf_get(coming_bond_info ? coming_bond_info->info.identity_addr.address.addr : coming_addr,
                                                               cur_bond_info ? cur_bond_info->info.identity_addr.address.addr : cur_addr);

    if (user_conf_ret == BT_SINK_SRV_A2DP_INT_USER_CONF_ALLOW || user_conf_ret == BT_SINK_SRV_A2DP_INT_USER_CONF_BYPASS) {
        return BT_SINK_SRV_LEA_INT_USER_CONF_ALLOW;
    } else if (user_conf_ret == BT_SINK_SRV_A2DP_INT_USER_CONF_PENDING) {
        return BT_SINK_SRV_LEA_INT_USER_CONF_PENDING;
    } else {
        return BT_SINK_SRV_LEA_INT_USER_CONF_DISALLOW;
    }
}
#endif
#endif

#endif
