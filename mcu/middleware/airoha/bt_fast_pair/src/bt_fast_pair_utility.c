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

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "syslog.h"
#include "atci.h"
#include "bt_fast_pair.h"
#include "bt_fast_pair_utility.h"
#include "nvkey.h"
#include "nvkey_id_list.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "task.h"
#include "semphr.h"
#include "task_def.h"
#include "system_daemon.h"
#ifdef AIR_BT_FAST_PAIR_SASS_ENABLE
#include "mbedtls/hkdf.h"
#endif
#if defined(MTK_AWS_MCE_ENABLE)
#include "bt_aws_mce_report.h"
#include "bt_aws_mce_srv.h"
#endif
#include "bt_device_manager.h"

log_create_module(BT_FAST_PAIR, PRINT_LEVEL_INFO);
LOG_CONTROL_BLOCK_DECLARE(BT_FAST_PAIR);

#define LOG_TAG     "[FAST_PAIR][UTILITY]"

#if defined(AIR_TWS_ENABLE) && defined(AIR_LE_AUDIO_ENABLE)
static const bt_bd_addr_t empty_addr = { 0, 0, 0, 0, 0, 0 };
#endif

static atci_status_t bt_fast_pair_at_cmd_set_battery_value(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t bt_fast_pair_at_cmd_set_silence_mode(atci_parse_cmd_param_t *parse_cmd);

void        bt_fast_pair_log_msgid_i(const char *msg, uint32_t arg_cnt, ...)
{
#ifdef MTK_DEBUG_LEVEL_INFO
    va_list ap;
    va_start(ap, arg_cnt);
    log_print_msgid(&log_control_block_BT_FAST_PAIR, PRINT_LEVEL_INFO, msg, arg_cnt, ap);
    va_end(ap);
#endif
}

void        bt_fast_pair_log_msgid_w(const char *msg, uint32_t arg_cnt, ...)
{
#ifdef MTK_DEBUG_LEVEL_WARNING
    va_list ap;
    va_start(ap, arg_cnt);
    log_print_msgid(&log_control_block_BT_FAST_PAIR, PRINT_LEVEL_WARNING, msg, arg_cnt, ap);
    va_end(ap);
#endif
}

void        bt_fast_pair_log_msgid_e(const char *msg, uint32_t arg_cnt, ...)
{
#ifdef MTK_DEBUG_LEVEL_ERROR
    va_list ap;
    va_start(ap, arg_cnt);
    log_print_msgid(&log_control_block_BT_FAST_PAIR, PRINT_LEVEL_ERROR, msg, arg_cnt, ap);
    va_end(ap);
#endif
}

void        bt_fast_pair_log_msgid_d(const char *msg, uint32_t arg_cnt, ...)
{
#ifdef MTK_DEBUG_LEVEL_DEBUG
    va_list ap;
    va_start(ap, arg_cnt);
    log_print_msgid(&log_control_block_BT_FAST_PAIR, PRINT_LEVEL_DEBUG, msg, arg_cnt, ap);
    va_end(ap);
#endif
}

void bt_fast_pair_log_dump_i(const char *func,
                             int line,
                             const void *data,
                             int length,
                             const char *message, ...)
{
#ifdef MTK_DEBUG_LEVEL_DEBUG
    va_list ap;
    va_start(ap, message);
    dump_module_buffer(&log_control_block_BT_FAST_PAIR, func, line, PRINT_LEVEL_INFO, data, length, message, ap);
    va_end(ap);
#endif
}

#if 0
#include "bt_connection_manager.h"
const uint8_t session_nonce[] = { 0xAA, 0xBB, 0xCC, 0xDD, 0x11, 0x22, 0x33, 0x44 };

void bt_fast_pair_test_sass_init_session_data(void)
{
    bt_bd_addr_t addr[2];
    uint32_t addr_size = 2;
    addr_size = bt_cm_get_connected_devices(~BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS), addr, addr_size);
    if (addr_size > 0) {
        uint32_t i;
        uint8_t buff[sizeof(bt_fast_pair_message_stream_t) + BT_FAST_PAIR_MESSAGE_NONCE_LENGTH - 1];
        bt_fast_pair_message_stream_t *msg = (bt_fast_pair_message_stream_t *)buff;
        msg->group_ID = 0x3;
        msg->code_ID = 0xA;
        msg->data_length = 0x0008;
        memcpy(msg->data, session_nonce, 8);
        msg->data_length = BT_FAST_PAIR_REVERSE_UINT16(msg->data_length);
        LOG_HEXDUMP_I(BT_FAST_PAIR, "bt_fast_pair_test_sass_init_session_data", msg->data, BT_FAST_PAIR_MESSAGE_NONCE_LENGTH);
        bt_fast_pair_message_stream_received_data(addr[0], msg, sizeof(bt_fast_pair_message_stream_t) + BT_FAST_PAIR_MESSAGE_NONCE_LENGTH - 1);

        uint8_t buff2[26];
        bt_fast_pair_message_stream_t *indicate_account_msg = (bt_fast_pair_message_stream_t *)buff2;
        indicate_account_msg->group_ID = 0x07;
        indicate_account_msg->code_ID = 0x41;
        indicate_account_msg->data_length = 22;
        memcpy(indicate_account_msg->data, "in-use", sizeof("in-use") - 1);
        bt_os_layer_generate_random_block(indicate_account_msg->data + sizeof("in-use") - 1, 8); /* Message nonce */
        uint32_t nonce_msg_len = BT_FAST_PAIR_MESSAGE_NONCE_LENGTH + BT_FAST_PAIR_MESSAGE_NONCE_LENGTH + sizeof("in-use") - 1;
        uint8_t *nonce_msg = (uint8_t *)pvPortMalloc(nonce_msg_len);
        if (nonce_msg) {
            bt_os_layer_aes_buffer_t sha_data;
            uint8_t sha_output[32];
            memcpy(nonce_msg, session_nonce, BT_FAST_PAIR_MESSAGE_NONCE_LENGTH);
            memcpy(nonce_msg + BT_FAST_PAIR_MESSAGE_NONCE_LENGTH, indicate_account_msg->data + sizeof("in-use") - 1, 8);
            memcpy(nonce_msg + BT_FAST_PAIR_MESSAGE_NONCE_LENGTH + BT_FAST_PAIR_MESSAGE_NONCE_LENGTH, indicate_account_msg->data, indicate_account_msg->data_length - BT_FAST_PAIR_MESSAGE_NONCE_LENGTH - BT_FAST_PAIR_MESSAGE_AUTH_CODE_LENGTH);
            uint8_t *sha_input = (uint8_t *)pvPortMalloc(BT_FAST_PAIR_ACCOUNT_KEY_SIZE + 48 + (nonce_msg_len > 32 ? nonce_msg_len : 32));
            if (sha_input) {
                /* Step 1 */
                memcpy(sha_input, bt_fast_pair_get_account_key_list()->account_key_list[bt_fast_pair_get_account_key_list()->max_key_number - 1], BT_FAST_PAIR_ACCOUNT_KEY_SIZE);
                memset(sha_input + BT_FAST_PAIR_ACCOUNT_KEY_SIZE, 0, 48);
                for (i = 0; i < BT_FAST_PAIR_ACCOUNT_KEY_SIZE + 48; i++) {
                    sha_input[i] ^= 0x36;
                }
                memcpy(sha_input + BT_FAST_PAIR_ACCOUNT_KEY_SIZE + 48, nonce_msg, nonce_msg_len);
                sha_data.buffer = sha_input;
                sha_data.length = BT_FAST_PAIR_ACCOUNT_KEY_SIZE + 48 + nonce_msg_len;
                bt_os_layer_sha256(sha_output, &sha_data);
                LOG_HEXDUMP_I(BT_FAST_PAIR, "bt_fast_pair_test_sass_init_session_data, 1 input", sha_input, BT_FAST_PAIR_ACCOUNT_KEY_SIZE + 48 + nonce_msg_len);
                LOG_HEXDUMP_I(BT_FAST_PAIR, "bt_fast_pair_test_sass_init_session_data, 1 output", sha_output, 32);
                /* Step 2 */
                memcpy(sha_input, bt_fast_pair_get_account_key_list()->account_key_list[bt_fast_pair_get_account_key_list()->max_key_number - 1], BT_FAST_PAIR_ACCOUNT_KEY_SIZE);
                memset(sha_input + BT_FAST_PAIR_ACCOUNT_KEY_SIZE, 0, 48);
                for (i = 0; i < BT_FAST_PAIR_ACCOUNT_KEY_SIZE + 48; i++) {
                    sha_input[i] ^= 0x5C;
                }
                memcpy(sha_input + BT_FAST_PAIR_ACCOUNT_KEY_SIZE + 48, sha_output, 32);
                sha_data.buffer = sha_input;
                sha_data.length = BT_FAST_PAIR_ACCOUNT_KEY_SIZE + 48 + 32;
                bt_os_layer_sha256(sha_output, &sha_data);
                LOG_HEXDUMP_I(BT_FAST_PAIR, "bt_fast_pair_test_sass_init_session_data, 2 input", sha_input, BT_FAST_PAIR_ACCOUNT_KEY_SIZE + 48 + 32);
                LOG_HEXDUMP_I(BT_FAST_PAIR, "bt_fast_pair_test_sass_init_session_data, 2 output", sha_output, 32);
                memcpy(indicate_account_msg->data + BT_FAST_PAIR_MESSAGE_NONCE_LENGTH + sizeof("in-use") - 1, sha_output, 8);
                indicate_account_msg->data_length = BT_FAST_PAIR_REVERSE_UINT16(indicate_account_msg->data_length);
                LOG_HEXDUMP_I(BT_FAST_PAIR, "bt_fast_pair_test_sass_init_session_data, msg total", buff2, sizeof(buff2));
                bt_fast_pair_message_stream_received_data(addr[0], indicate_account_msg, sizeof(buff2));
            }
            vPortFree(nonce_msg);
        }
    }
}

void bt_fast_pair_test_sass_generate_encrypted_connection_status_field(void)
{
    const bt_bd_addr_t addr = { 0x11, 0x22, 0x11, 0x22, 0x11, 0x22 };
    const bt_fast_pair_account_key_t test_account[2] = {
        { 0x04, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF },
        { 0x04, 0x11, 0x22, 0x22, 0x33, 0x33, 0x44, 0x44, 0x55, 0x55, 0x66, 0x66, 0x77, 0x77, 0x88, 0x88 }
    };
    bt_fast_pair_add_addr_session_nonce_mapping(&addr, session_nonce);
    bt_fast_pair_account_key_list_t *account_key_list = bt_fast_pair_get_account_key_list();
    memcpy(account_key_list->account_key_list, test_account, sizeof(test_account));
    bt_fast_pair_store_addr_account_id_mapping(&addr, account_key_list->max_key_number - 2);

    uint8_t salt[] = { 0xC7, 0xC8 };
    uint8_t filter[16] = { 0 };
    uint8_t filter_len = bt_fast_pair_core_excute_bloom_filter(filter, account_key_list, sizeof(salt), salt, account_key_list->max_key_number - 2, 0x6);
    LOG_HEXDUMP_I(BT_FAST_PAIR, "bt_fast_pair_test_sass_generate_encrypted_connection_status_field, filter : %d", filter, sizeof(filter), filter_len);
    uint8_t encrypted_connection_status[BT_FAST_PAIR_SASS_ENCRYPTED_CONNECTION_STATUS_MAX_LENGTH] = { 0 };
    uint32_t encrypted_connection_status_length = BT_FAST_PAIR_SASS_ENCRYPTED_CONNECTION_STATUS_MAX_LENGTH;
    bt_fast_pair_sass_generate_encrypted_connection_status(encrypted_connection_status, &encrypted_connection_status_length, filter);
    LOG_HEXDUMP_I(BT_FAST_PAIR, "bt_fast_pair_test_sass_generate_encrypted_connection_status_field, encrypted_raw", encrypted_connection_status, encrypted_connection_status_length);
}

static atci_status_t
bt_fast_pair_at_cmd_sass_session_init(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};
    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            BT_FAST_PAIR_LOG_I(LOG_TAG"sass_session_init", 0);
            bt_fast_pair_test_sass_init_session_data();
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            break;
        }
        default:
            break;
    }
    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}

static atci_status_t
bt_fast_pair_at_cmd_sass_connection_status_field_test(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};
    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            BT_FAST_PAIR_LOG_I(LOG_TAG"sass_connection_status_field_test", 0);
            bt_fast_pair_test_sass_generate_encrypted_connection_status_field();
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            break;
        }
        default:
            break;
    }
    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}
#endif

static atci_cmd_hdlr_item_t bt_fast_pair_at_cmd[] = {
    {
        .command_head = "AT+FPSBATTERYSET",
        .command_hdlr = bt_fast_pair_at_cmd_set_battery_value,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
    {
        .command_head = "AT+FPSSILENCE",
        .command_hdlr = bt_fast_pair_at_cmd_set_silence_mode,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
#if 0
    {
        .command_head = "AT+FPSSASS_INIT_SESSION",
        .command_hdlr = bt_fast_pair_at_cmd_sass_session_init,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
    {
        .command_head = "AT+FPSSASS_CONN_STATUS_FIELD",
        .command_hdlr = bt_fast_pair_at_cmd_sass_connection_status_field_test,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
#endif
};

#if 0
static atci_status_t
bt_app_comm_at_cmd_set_fast_pair_tx_power_level(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};
    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            int8_t tx_power_level = atoi(parse_cmd->string_ptr + parse_cmd->name_len + 1);
            LOG_MSGID_I(BT_APP, "set fast pair tx power level :%d", 1, tx_power_level);

            app_fast_pair_set_tx_power_level(tx_power_level);
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            break;
        }
        default:
            break;
    }
    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}
#endif

static atci_status_t
bt_fast_pair_at_cmd_set_battery_value(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};
    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            uint8_t battery_raw[sizeof(bt_fast_pair_battery_t) + (2 * sizeof(bt_fast_pair_battery_item_t))];
            bt_fast_pair_battery_t *battery = (bt_fast_pair_battery_t *)battery_raw;
            battery->component_num = 3;
            battery->ui_show = true;
            battery->remaining_time = 0xFFFF;
            battery->battery[0].charging = 0x01;
            battery->battery[0].battery_value = 67;
            battery->battery[1].charging = 0x00;
            battery->battery[1].battery_value = 70;
            battery->battery[2].charging = 0x00;
            battery->battery[2].battery_value = 50;

            bt_fast_pair_update_battery(battery);
            LOG_MSGID_I(BT_APP, "bt_fast_pair_at_cmd_set_battery_value", 0);

            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            break;
        }
        default:
            break;
    }
    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}

static atci_status_t
bt_fast_pair_at_cmd_set_silence_mode(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};
    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            bt_fast_pair_set_silence_mode(NULL, true);
            LOG_MSGID_I(BT_APP, "silence mode", 0);
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            break;
        }
        default:
            break;
    }
    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}

void bt_fast_pair_at_cmd_init(void)
{
    atci_register_handler(bt_fast_pair_at_cmd, sizeof(bt_fast_pair_at_cmd) / sizeof(atci_cmd_hdlr_item_t));
}

bt_fast_pair_nvdm_sta_t bt_fast_pair_nvdm_read(const char *group_name, const char *item_name, uint8_t *buf, uint32_t size)
{
    nvdm_status_t status;
    status = nvdm_read_data_item(group_name, item_name, buf, &size);
    if (status == NVDM_STATUS_OK) {
        return BT_FAST_PAIR_NVDM_STA_SUCCESS;
    } else if (status == NVDM_STATUS_ITEM_NOT_FOUND) {
        return BT_FAST_PAIR_NVDM_STA_NOT_FOUND;
    }

    return BT_FAST_PAIR_NVDM_STA_FAIL;
}

bt_fast_pair_nvdm_sta_t bt_fast_pair_nvdm_write(const char *group_name, const char *data_item_name, const uint8_t *buffer, uint32_t size)
{
    nvdm_status_t status;
    status = nvdm_write_data_item(group_name, data_item_name, NVDM_DATA_ITEM_TYPE_RAW_DATA, buffer, size);
    if (status == NVDM_STATUS_OK) {
        return BT_FAST_PAIR_NVDM_STA_SUCCESS;
    } else if (status == NVDM_STATUS_INSUFFICIENT_SPACE) {
        return BT_FAST_PAIR_NVDM_STA_INSUFFICIENT_SPACE;
    }

    return BT_FAST_PAIR_NVDM_STA_FAIL;
}

extern nvdm_status_t nvdm_query_data_item_length(const char *group_name, const char *data_item_name, uint32_t *size);
bt_fast_pair_nvdm_sta_t bt_fast_pair_nvdm_query_length(const char *group_name, const char *data_item_name, uint32_t *size)
{
    nvdm_status_t status;
    status = nvdm_query_data_item_length(group_name, data_item_name, size);
    if (status == NVDM_STATUS_OK) {
        return BT_FAST_PAIR_NVDM_STA_SUCCESS;
    } else if (status == NVDM_STATUS_ITEM_NOT_FOUND) {
        return BT_FAST_PAIR_NVDM_STA_NOT_FOUND;
    }

    return BT_FAST_PAIR_NVDM_STA_FAIL;

}

bt_fast_pair_nvkey_sta_t bt_fast_pair_nvkey_read(uint16_t id, uint8_t *buf, uint32_t *size)
{
    nvkey_status_t sta = NVKEY_STATUS_OK;
    sta = nvkey_read_data(id, buf, size);
    if (sta == NVKEY_STATUS_OK) {
        return BT_FAST_PAIR_NVKEY_STA_SUCCESS;
    } else if (sta == NVKEY_STATUS_ITEM_NOT_FOUND) {
        return BT_FAST_PAIR_NVKEY_STA_NOT_FOUND;
    }

    return BT_FAST_PAIR_NVKEY_STA_FAIL;
}

bt_fast_pair_nvkey_sta_t bt_fast_pair_nvkey_write(uint16_t id, uint8_t *data, uint32_t size)
{
    nvkey_status_t sta = NVKEY_STATUS_OK;
    sta = nvkey_write_data(id, data, size);
    if (sta == NVKEY_STATUS_OK) {
        return BT_FAST_PAIR_NVKEY_STA_SUCCESS;
    } else if (sta == NVKEY_STATUS_ITEM_NOT_FOUND) {
        return BT_FAST_PAIR_NVKEY_STA_NOT_FOUND;
    }

    return BT_FAST_PAIR_NVKEY_STA_FAIL;
}

bt_fast_pair_nvkey_sta_t bt_fast_pair_nvkey_length(uint16_t id, uint32_t *size)
{
    nvkey_status_t sta = NVKEY_STATUS_OK;
    sta = nvkey_data_item_length(id, size);
    if (sta == NVKEY_STATUS_OK) {
        return BT_FAST_PAIR_NVKEY_STA_SUCCESS;
    } else if (sta == NVKEY_STATUS_ITEM_NOT_FOUND) {
        return BT_FAST_PAIR_NVKEY_STA_NOT_FOUND;
    }

    return BT_FAST_PAIR_NVKEY_STA_FAIL;
}

bt_fast_pair_status_t bt_fast_pair_sha_256_hkdf(const uint8_t *salt,
                                                size_t salt_len, const uint8_t *ikm, size_t ikm_len,
                                                const uint8_t *info, size_t info_len,
                                                uint8_t *okm, size_t okm_len )
{
#ifdef AIR_BT_FAST_PAIR_SASS_ENABLE
    int hkdf_ret = mbedtls_hkdf(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), salt, salt_len, ikm, ikm_len, info, info_len, okm, okm_len);
    LOG_MSGID_I(BT_FAST_PAIR, "bt_fast_pair_sha_256_hkdf = %d", 1, hkdf_ret);
    if (0 == hkdf_ret) {
        return BT_FAST_PAIR_STATUS_SUCCESS;
    } else {
        return BT_FAST_PAIR_STATUS_FAIL;
    }
#else
    return BT_FAST_PAIR_STATUS_FAIL;
#endif
}

void *bt_fast_pair_spot_sys_timer_create(char *timer_name, bool repeat, uint32_t period_ms, void *usr_data, spot_timer_callback cb)
{
    if (timer_name == NULL || strlen(timer_name) == 0) {
        return NULL;
    }

    void *timer_handle = xTimerCreate(timer_name, (TickType_t)(period_ms / portTICK_PERIOD_MS), repeat, usr_data, (void *)cb);

    return timer_handle;
}

bool bt_fast_pair_spot_sys_timer_start(void *timer_handle)
{
    if (xTimerStart(timer_handle, 0) != pdPASS) {
        return false;
    }
    return true;
}

bool bt_fast_pair_spot_sys_timer_stop(void *timer_handle)
{
    if (xTimerStop(timer_handle, 0) != pdPASS) {
        return false;
    }
    return true;
}

bool bt_fast_pair_spot_sys_timer_delete(void *timer_handle)
{
    if (xTimerDelete(timer_handle, 0) != pdPASS) {
        return false;
    }
    return true;
}

uint32_t bt_fast_pair_spot_sys_run_time_second()
{
    uint32_t ret = 0;
    ret = xTaskGetTickCount() / portTICK_PERIOD_MS / 1000;
    return ret;
}

void bt_fast_pair_hex_dump(const char *msg, unsigned char *buf, unsigned int buf_len)
{
    LOG_HEXDUMP_I(BT_FAST_PAIR, msg, buf, buf_len);
}

void bt_fast_pair_task_create(bt_fast_pair_task_t *task)
{
    unsigned long create_result = 0;
    void *task_handle = NULL;

    if (task == NULL) {
        return;
    }

    if (task->function == NULL) {
        return;
    }

    if (memcmp(task->name, "SPTASK", strlen("SPTASK")) != 0) {
        create_result = xTaskCreate(task->function,
                                    task->name,
                                    task->stack_depth,
                                    task->parameter,
                                    (task->is_high_priority ? TASK_PRIORITY_HIGH : TASK_PRIORITY_BELOW_NORMAL), // TASK_PRIORITY_NORMAL
                                    (TaskHandle_t *)&task_handle);
    } else {
        create_result = system_daemon_task_invoke(task->function, task->parameter);
    }

    if (create_result != pdPASS) {
        return;
    }
}

void bt_fast_pair_task_destroy()
{
    vTaskDelete(NULL);
}

uint32_t bt_fast_pair_task_get_running_ms()
{
    return xTaskGetTickCount() / portTICK_PERIOD_MS;
}

void *bt_fast_pair_mutex_create()
{
    SemaphoreHandle_t xSemaphore = xSemaphoreCreateMutex();
    if (xSemaphore == NULL) {
        LOG_MSGID_E(BT_APP, "create mutex fail", 0);
        return NULL;
    } else {
        LOG_MSGID_I(BT_APP, "create mutex success", 0);
    }

    return (void *)xSemaphore;
}

void bt_fast_pair_mutex_take(void *mutex_handle)
{
    LOG_MSGID_I(BT_APP, "take mutex START", 0);
    if (xSemaphoreTake((SemaphoreHandle_t)mutex_handle, (TickType_t)portMAX_DELAY) == pdTRUE) {
        LOG_MSGID_I(BT_APP, "take mutex success", 0);
    } else {
        LOG_MSGID_E(BT_APP, "take mutex fail", 0);
    }
}

void bt_fast_pair_mutex_give(void *mutex_handle)
{
    LOG_MSGID_I(BT_APP, "give mutex START", 0);
    if (xSemaphoreGive((SemaphoreHandle_t)mutex_handle) != pdTRUE) {
        LOG_MSGID_E(BT_APP, "give mutex fail", 0);
    } else {
        LOG_MSGID_I(BT_APP, "give mutex success", 0);
    }
}

bool __attribute__((weak)) bt_fast_pair_spot_unwanted_tracking_mode(bool en, bool set) {
    return false;
}


static bt_fast_pair_sass_peer_state_t *s_sass_peer_state = NULL;

const bt_fast_pair_sass_peer_state_t *bt_fast_pair_utility_get_sass_peer_state(void)
{
    return s_sass_peer_state;
}

void bt_fast_pair_sass_send_state_to_peer(bt_fast_pair_sass_temp_data_t *temp_data, uint8_t in_use_account)
{
#if defined(AIR_TWS_ENABLE) && defined(AIR_LE_AUDIO_ENABLE)
    if (temp_data && BT_AWS_MCE_SRV_LINK_NONE != bt_aws_mce_srv_get_link_type()) {
        uint8_t sending_data[sizeof(uint8_t) + sizeof(bt_fast_pair_sass_peer_state_t) + FAST_PAIR_SPP_MAXIMUM * sizeof(bt_fast_pair_sass_peer_device_t)];
        uint8_t i;
        bt_fast_pair_account_key_list_t *account_key_list = bt_fast_pair_get_account_key_list();
        sending_data[0] = BT_FAST_PAIR_AWS_DATA_SYNC_SASS_STATE;
        bt_fast_pair_sass_peer_state_t *send_struct = (bt_fast_pair_sass_peer_state_t *)&sending_data[1];
        send_struct->peer_list_cnt = 0;
        send_struct->in_use_account_key = in_use_account;
        send_struct->custom_data = temp_data->custom_data;
        for (i = 0; i < FAST_PAIR_SPP_MAXIMUM; i++) {
            if (temp_data->addr_account_map[i].account_index < account_key_list->max_key_number
                && memcmp(temp_data->addr_account_map[i].addr, empty_addr, sizeof(empty_addr)) != 0) {
                memcpy(send_struct->device_list[send_struct->peer_list_cnt].addr, temp_data->addr_account_map[i].addr, sizeof(temp_data->addr_account_map[i].addr));
                send_struct->device_list[send_struct->peer_list_cnt].account_index = temp_data->addr_account_map[i].account_index;
                send_struct->device_list[send_struct->peer_list_cnt].bitmap_index = temp_data->addr_account_map[i].bitmap_index;
                send_struct->device_list[send_struct->peer_list_cnt].flag_playing = temp_data->addr_account_map[i].flag_playing;
                send_struct->peer_list_cnt ++;
            }
        }
        bt_aws_mce_report_info_t aws_data = { 0 };
        aws_data.param = sending_data;
        aws_data.is_sync = false;
        aws_data.sync_time = 0;
        aws_data.module_id = BT_AWS_MCE_REPORT_MODULE_FAST_PAIR;
        aws_data.param_len = sizeof(uint8_t) + sizeof(bt_fast_pair_sass_peer_state_t) + send_struct->peer_list_cnt * sizeof(bt_fast_pair_sass_peer_device_t);
        bt_status_t ret = bt_aws_mce_report_send_event(&aws_data);
        bt_fast_pair_log(LOG_TAG"bt_fast_pair_sass_send_state_to_peer cnt = %d, ret = 0x%x", 2, send_struct->peer_list_cnt, ret);
    }
#endif
}

void bt_fast_pair_sass_send_custom_data_to_peer(uint8_t custom_data)
{
#if defined(AIR_TWS_ENABLE) && defined(AIR_LE_AUDIO_ENABLE)
    if (BT_AWS_MCE_SRV_LINK_NONE != bt_aws_mce_srv_get_link_type()) {
        uint8_t sending_data[sizeof(uint8_t) + sizeof(custom_data)];
        sending_data[0] = BT_FAST_PAIR_AWS_DATA_CUSTOM_DATA;
        sending_data[1] = custom_data;
        bt_aws_mce_report_info_t aws_data = { 0 };
        aws_data.param = sending_data;
        aws_data.is_sync = false;
        aws_data.sync_time = 0;
        aws_data.module_id = BT_AWS_MCE_REPORT_MODULE_FAST_PAIR;
        aws_data.param_len = sizeof(sending_data);
        bt_status_t ret = bt_aws_mce_report_send_event(&aws_data);
        bt_fast_pair_log(LOG_TAG"bt_fast_pair_sass_send_custom_data_to_peer custom_data = %d, ret = 0x%x", 2, custom_data, ret);
    }
#endif
}

void bt_fast_pair_sass_clear_peer_data(void)
{
#if defined(AIR_TWS_ENABLE) && defined(AIR_LE_AUDIO_ENABLE)
    void *free_addr = s_sass_peer_state;
    s_sass_peer_state = NULL;
    if (free_addr) {
        free(free_addr);
    }
#endif
}

bool bt_fast_pair_send_additional_passkey_to_peer(uint32_t add_passkey)
{
    bool send_ret = false;
#if defined(AIR_TWS_ENABLE) && defined(AIR_LE_AUDIO_ENABLE)
    if (BT_AWS_MCE_SRV_LINK_NONE != bt_aws_mce_srv_get_link_type()) {
        uint8_t sending_data[sizeof(uint8_t) + sizeof(add_passkey)];
        sending_data[0] = BT_FAST_PAIR_AWS_DATA_ADD_PASSKEY;
        memcpy(&sending_data[1], &add_passkey, sizeof(uint32_t));
        bt_aws_mce_report_info_t aws_data = { 0 };
        aws_data.param = sending_data;
        aws_data.is_sync = false;
        aws_data.sync_time = 0;
        aws_data.module_id = BT_AWS_MCE_REPORT_MODULE_FAST_PAIR;
        aws_data.param_len = sizeof(sending_data);
        bt_status_t ret = bt_aws_mce_report_send_event(&aws_data);
        if (ret == BT_STATUS_SUCCESS) {
            send_ret = true;
        }
        bt_fast_pair_log(LOG_TAG"bt_fast_pair_send_additional_passkey_to_peer add_passkey=0x%x, ret = 0x%x", 2, add_passkey, ret);
    }
#endif
    return send_ret;
}

void bt_fast_pair_send_additional_passkey_response(bt_fast_pair_peer_add_passkey_rsp_t response)
{
#if defined(AIR_TWS_ENABLE) && defined(AIR_LE_AUDIO_ENABLE)
    if (BT_AWS_MCE_SRV_LINK_NONE != bt_aws_mce_srv_get_link_type()) {
        uint8_t sending_data[sizeof(uint8_t) + sizeof(bt_fast_pair_peer_add_passkey_rsp_t)];
        sending_data[0] = BT_FAST_PAIR_AWS_DATA_ADD_PASSKEY_RSP;
        memcpy(&sending_data[1], &response, sizeof(bt_fast_pair_peer_add_passkey_rsp_t));
        bt_aws_mce_report_info_t aws_data = { 0 };
        aws_data.param = sending_data;
        aws_data.is_sync = false;
        aws_data.sync_time = 0;
        aws_data.module_id = BT_AWS_MCE_REPORT_MODULE_FAST_PAIR;
        aws_data.param_len = sizeof(sending_data);
        bt_status_t ret = bt_aws_mce_report_send_event(&aws_data);
        bt_fast_pair_log(LOG_TAG"bt_fast_pair_send_additional_passkey_to_peer: status=%d, peer_pass_key=0x%x, ret = 0x%x",
                         3, response.status, response.peer_passkey, ret);
    }
#endif
}


#if defined(AIR_TWS_ENABLE) && defined(AIR_LE_AUDIO_ENABLE)
static void bt_fast_pair_utility_aws_report_event_callback(bt_aws_mce_report_info_t *para)
{
    if (para && BT_AWS_MCE_REPORT_MODULE_FAST_PAIR == para->module_id) {
        if (para->param && para->param_len > 0) {
            uint8_t event_id = ((uint8_t *)para->param)[0];
            if (event_id == BT_FAST_PAIR_AWS_DATA_SYNC_SASS_STATE && para->param_len >= sizeof(uint8_t) + sizeof(bt_fast_pair_sass_peer_state_t)) {
                bt_fast_pair_sass_peer_state_t *received_sass_data = (bt_fast_pair_sass_peer_state_t *) & ((uint8_t *)para->param)[1];
                if (sizeof(uint8_t) + sizeof(bt_fast_pair_sass_peer_state_t) + received_sass_data->peer_list_cnt * sizeof(bt_fast_pair_sass_peer_device_t) == para->param_len) {
                    bool need_update = false;
                    if (s_sass_peer_state
                        && memcmp(s_sass_peer_state, received_sass_data, sizeof(bt_fast_pair_sass_peer_state_t) + received_sass_data->peer_list_cnt * sizeof(bt_fast_pair_sass_peer_device_t)) == 0) {
                        /* Not change */
                    } else {
                        need_update = true;
                        if (s_sass_peer_state && s_sass_peer_state->peer_list_cnt == received_sass_data->peer_list_cnt) {
                            /* Size not change */
                        } else {
                            if (s_sass_peer_state) {
                                free(s_sass_peer_state);
                            }
                            s_sass_peer_state = malloc(sizeof(bt_fast_pair_sass_peer_state_t) + received_sass_data->peer_list_cnt * sizeof(bt_fast_pair_sass_peer_device_t));
                        }
                        memcpy(s_sass_peer_state, received_sass_data, sizeof(bt_fast_pair_sass_peer_state_t) + received_sass_data->peer_list_cnt * sizeof(bt_fast_pair_sass_peer_device_t));

                    }
                    bt_fast_pair_log(LOG_TAG"bt_fast_pair_sass_on_aws_data_receive_peer_state, cnt = %d", 1, received_sass_data->peer_list_cnt);
                    if (need_update) {
                        bool local_is_agent = bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT;
                        bt_fast_pair_sass_on_peer_state_updated(local_is_agent);
                    }
                }
            } else if (event_id == BT_FAST_PAIR_AWS_DATA_CUSTOM_DATA && para->param_len == sizeof(uint8_t) + sizeof(uint8_t)) {
                uint8_t custom_data = ((uint8_t *)para->param)[1];
                bt_fast_pair_log(LOG_TAG"bt_fast_pair_sass_on_aws_data_receive_custom_data, = 0x%x", 1, custom_data);
                if (s_sass_peer_state) {
                    s_sass_peer_state->custom_data = custom_data;
                    bool local_is_agent = bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT;
                    bt_fast_pair_sass_on_peer_custom_data_updated(local_is_agent, custom_data);
                }
            } else if (event_id == BT_FAST_PAIR_AWS_DATA_ADD_PASSKEY && para->param_len == sizeof(uint8_t) + sizeof(uint32_t)) {
                uint32_t add_passkey;
                memcpy(&add_passkey, ((uint8_t *)para->param) + 1, sizeof(uint32_t));
                bt_fast_pair_log(LOG_TAG"bt_fast_pair_aws_data_receive_additional_passkey, passkey= 0x%x", 1, add_passkey);
                bt_fast_pair_core_peer_add_passkey_handle(add_passkey);
                // LOG_HEXDUMP_I(BT_FAST_PAIR, LOG_TAG"bt_fast_pair_aws_data_receive_additional_passkey", para->param, para->param_len);
            } else if (event_id == BT_FAST_PAIR_AWS_DATA_ADD_PASSKEY_RSP && para->param_len == sizeof(uint8_t) + sizeof(bt_fast_pair_peer_add_passkey_rsp_t)) {
                bt_fast_pair_peer_add_passkey_rsp_t add_pass_key_rsp = {0};
                memcpy(&add_pass_key_rsp, ((uint8_t *)para->param) + 1, sizeof(bt_fast_pair_peer_add_passkey_rsp_t));
                bt_fast_pair_log(LOG_TAG"bt_fast_pair_aws_data_receive_additional_passkey_rsp: status=%d, peer_pass_key=0x%x",
                                 2, add_pass_key_rsp.status, add_pass_key_rsp.peer_passkey);
                bt_fast_pair_core_peer_add_passkey_rsp(add_pass_key_rsp);
            }
        }
    }
}
#endif

void bt_fast_pair_utility_aws_data_sync_init(void)
{
#if defined(AIR_TWS_ENABLE) && defined(AIR_LE_AUDIO_ENABLE)
    bt_aws_mce_report_register_callback(BT_AWS_MCE_REPORT_MODULE_FAST_PAIR, bt_fast_pair_utility_aws_report_event_callback);
#endif
}

