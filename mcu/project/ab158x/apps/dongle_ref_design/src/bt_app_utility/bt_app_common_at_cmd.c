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
#include "FreeRTOS.h"
#include "atci.h"
#include "bt_type.h"
#include "bt_app_common.h"
#if defined(MTK_FOTA_VIA_RACE_CMD) && defined(MTK_RACE_CMD_ENABLE)
#include "race_fota.h"
#endif
#ifdef MTK_IAP2_PROFILE_ENABLE
#include "apps_debug.h"
#include "mfi_coprocessor_api.h"
#endif

#if defined(SUPPORT_ROLE_HANDOVER_SERVICE)
#include "bt_role_handover.h"
#endif
#ifdef __BT_FAST_PAIR_ENABLE__
#include "app_fast_pair.h"
#endif
#include "bt_gap_le.h"
#include "bt_customer_config.h"

#ifdef AIR_LE_AUDIO_ENABLE
#include "app_le_audio.h"
#include "bt_gattc.h"
#include "bt_gatts.h"
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
#include "app_le_audio_ucst.h"
#include "app_le_audio_ucst_utillity.h"
#include "app_le_audio_bcst_utillity.h"
#include "ble_micp.h"
#include "ble_csip.h"
#include "ble_vcp.h"
#include "bt_le_audio_source.h"
#include "app_le_audio_air.h"
#include "ble_bap_client.h"
#include "app_le_audio_vcp_volume_controller.h"
#include "app_le_audio_micp_micophone_controller.h"

#endif
#ifdef AIR_LE_AUDIO_BIS_ENABLE
#include "app_le_audio_bcst_utillity.h"
#endif
#ifdef AIR_LE_AUDIO_BA_ENABLE
#include "app_le_audio_ba.h"
#include "app_le_audio_ba_utillity.h"
#endif
#include "bt_le_audio_source.h"
#include "ble_tmap.h"
#include "ble_bap_client.h"
#include "ble_bap.h"
#include "ble_bass.h"
#endif

#ifdef MTK_IAP2_PROFILE_ENABLE
#include "aacp_i2c.h"
#endif
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
#include "app_dongle_ull_le_hid.h"
#include "bt_ull_le_hid_service.h"
#endif

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
#include "bt_ull_service.h"
#include "bt_ull_le_service.h"
#include "app_ull_dongle_le.h"
#include "app_ull_dongle_idle_activity.h"
#endif

#include "app_dongle_connection_common.h"
#include "semphr.h"
#include "bt_utils.h"
#ifdef AIR_LE_AUDIO_HAPC_ENABLE
#include "ble_hapc.h"
#include "ble_iac.h"
#endif
#include "bt_device_manager_le.h"

#ifdef AIR_PURE_GAMING_ENABLE
#include "pka_porting_layer.h"
#endif
#include "app_dongle_ull_le_hid.h"

extern bt_status_t bt_app_common_advtising_stop(void);
static atci_status_t bt_app_comm_at_cmd_ble_adv_hdl(atci_parse_cmd_param_t *parse_cmd);
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
static atci_status_t bt_app_comm_at_cmd_cis_TO_hdl(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t bt_app_comm_at_cmd_disconnect_all_hdl(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t bt_app_comm_at_cmd_set_timer_hdl(atci_parse_cmd_param_t *parse_cmd);
#endif
static atci_status_t bt_app_comm_at_cmd_fota_hdl(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t bt_app_comm_at_cmd_swtich_dongle_mode_hdl(atci_parse_cmd_param_t *parse_cmd);

#if defined(MTK_AWS_MCE_ENABLE)
static atci_status_t bt_app_comm_at_cmd_rho_hdl(atci_parse_cmd_param_t *parse_cmd);
#endif
extern void bt_app_common_start_scan();
extern void bt_app_common_stop_scan();
extern const bt_bd_addr_t *bt_app_common_get_local_random_addr(void);
static atci_status_t bt_app_comm_at_cmd_ble_scan_hdl(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t bt_app_comm_at_cmd_ble_cancel_conn_hdl(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t bt_app_comm_at_cmd_ble_random_addr_hdl(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t bt_app_comm_at_cmd_set_fast_pair_tx_power_level(atci_parse_cmd_param_t *parse_cmd);

#ifdef MTK_IAP2_PROFILE_ENABLE
static atci_status_t bt_app_comm_at_cmd_get_mfi_cert_hdl(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t bt_app_comm_at_cmd_get_mfi_response_hdl(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t bt_app_comm_at_cmd_check_mfi_chip_hdl(atci_parse_cmd_param_t *parse_cmd);
#endif
static atci_status_t bt_app_comm_at_cmd_sniff_mode(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t bt_app_comm_at_cmd_ext_ble_adv_hdl(atci_parse_cmd_param_t *parse_cmd);

bt_status_t bt_app_common_ext_advertising_stop_test(uint8_t instance);
bt_status_t bt_app_common_ext_advertising_start_test(uint8_t instance);

#ifdef AIR_LE_AUDIO_ENABLE
static atci_status_t bt_app_comm_at_cmd_le_audio_hdl(atci_parse_cmd_param_t *parse_cmd);
extern bt_handle_t app_le_audio_ucst_get_handle(uint8_t link_idx);
extern void app_le_audio_ucst_set_cig_parameter_test(uint8_t bn, uint8_t nse, uint8_t ft, uint16_t iso_interval);
extern void app_le_audio_ucst_delete_device(bt_addr_t *addr);
extern ble_tmap_role_t ble_tmas_get_role(void);
extern void ble_mcs_set_media_state(uint8_t service_idx, ble_mcs_media_state_t state);
extern void ble_mcs_set_track_duration(uint8_t service_idx, int32_t duration);
extern void ble_mcs_set_track_position(uint8_t service_idx, int32_t position);
extern bt_status_t ble_tbs_terminate_call(uint8_t service_idx, ble_tbs_call_index_t call_idx, ble_tbs_termination_reason_t reason);
#endif /* AIR_LE_AUDIO_ENABLE */

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
static atci_status_t bt_app_comm_at_cmd_le_ull_hdl(atci_parse_cmd_param_t *parse_cmd);
#endif /* AIR_BLE_ULTRA_LOW_LATENCY_ENABLE */

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
static atci_status_t bt_app_comm_at_cmd_ull_hid_hdl(atci_parse_cmd_param_t *parse_cmd);
#endif

SemaphoreHandle_t g_at_cmd_mutex_handle = NULL;

static atci_status_t bt_app_comm_at_cmd_hfp_codec_set_hdl(atci_parse_cmd_param_t *parse_cmd);
#ifdef AIR_BT_SOURCE_ENABLE
static atci_status_t bt_app_comm_at_cmd_dongle_bqb_hdl(atci_parse_cmd_param_t *parse_cmd);
#endif
static atci_status_t bt_app_comm_at_cmd_le_bond_hdl(atci_parse_cmd_param_t *parse_cmd);


static atci_cmd_hdlr_item_t bt_app_comm_at_cmd[] = {
    {
        .command_head = "AT+BLEADV",    /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_ble_adv_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
    {
        .command_head = "AT+SETINTERVAL",    /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_cis_TO_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
    {
        .command_head = "AT+DISCONNECTALL",    /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_disconnect_all_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
    {
        .command_head = "AT+SETWINDOW",    /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_set_timer_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
#endif
    {
        .command_head = "AT+FOTA",    /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_fota_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    }
#if defined(MTK_AWS_MCE_ENABLE) && defined(SUPPORT_ROLE_HANDOVER_SERVICE)
    ,
    {
        .command_head = "AT+RHO",    /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_rho_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    }
#endif
    ,
    {
        .command_head = "AT+BLESCAN",    /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_ble_scan_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
    {
        .command_head = "AT+BLECANCELCONN",    /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_ble_cancel_conn_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
    {
        .command_head = "AT+BLERANDOMADDR",    /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_ble_random_addr_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
    {
        .command_head = "AT+FPSTXPOWER",
        .command_hdlr = bt_app_comm_at_cmd_set_fast_pair_tx_power_level,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
#ifdef MTK_IAP2_PROFILE_ENABLE
    {
        .command_head = "AT+GETMFICERT",    /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_get_mfi_cert_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
    {
        .command_head = "AT+GETMFIRESP",    /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_get_mfi_response_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
    {
        .command_head = "AT+CHECKMFICHIP",    /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_check_mfi_chip_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
#endif
    {
        .command_head = "AT+SNIFF",    /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_sniff_mode,
        .hash_value1 = 0,
        .hash_value2 = 0,
    }
    ,
    {
        .command_head = "AT+EXTBLEADV", /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_ext_ble_adv_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
    {
        .command_head = "AT+HFPCODEC", /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_hfp_codec_set_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
#ifdef AIR_LE_AUDIO_ENABLE
    {
        .command_head = "AT+LEAUDIO",       /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_le_audio_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
#endif
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
    {
        .command_head = "AT+LEULL",       /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_le_ull_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
#endif
    {
        .command_head = "AT+DONGLE_MODE", /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_swtich_dongle_mode_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
#ifdef AIR_BT_SOURCE_ENABLE
    {
        .command_head = "AT+DONGLE_BQB", /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_dongle_bqb_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
#endif
    {
        .command_head = "AT+LEBOND", /**< AT command string. */
        .command_hdlr = bt_app_comm_at_cmd_le_bond_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
        {
            .command_head = "AT+ULL_HID",       /**< AT command string. */
            .command_hdlr = bt_app_comm_at_cmd_ull_hid_hdl,
            .hash_value1 = 0,
            .hash_value2 = 0,
        },
#endif
};
static atci_status_t bt_app_comm_at_cmd_sniff_mode(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_OK};

    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
    bt_init_feature_mask_t bt_customer_config_feature = bt_customer_config_get_feature_mask_configuration();
    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            /* AT+SNIFF=<action>
                * action: "ON" / "OFF"
                */
            if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "OFF", 3)) {
                bt_customer_config_feature |= BT_INIT_FEATURE_MASK_DISABLE_SNIFF_MODE;
            } else if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "ON", 2)) {
                bt_customer_config_feature &= ~BT_INIT_FEATURE_MASK_DISABLE_SNIFF_MODE;
            }

            break;
        }

        default:
            break;
    }

    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}


static atci_status_t bt_app_comm_at_cmd_ble_adv_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};

    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            /* AT+BLEADV=<action>
                        * action: "ON" / "OFF"
                        */
            if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "OFF", 3)) {
                /* BLE ADV OFF */
                bt_status_t status = bt_app_common_advtising_stop();
                if (status == BT_STATUS_SUCCESS) {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                }
            } else if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "ON", 2)) {
                /* BLE ADV ON */
                bt_status_t status = bt_app_common_start_ble_adv_with_default_interval();
                if (status == BT_STATUS_SUCCESS) {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                }
            }

            break;
        }

        default:
            break;
    }

    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
static atci_status_t bt_app_comm_at_cmd_cis_TO_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};

    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            uint16_t CIS_TO = atoi(parse_cmd->string_ptr + parse_cmd->name_len + 1);
            LOG_MSGID_I(BT_APP, "set CIS_TO :%d", 1, CIS_TO);
            extern uint16_t g_app_hid_create_cis_timeout;
            g_app_hid_create_cis_timeout = CIS_TO;
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

static atci_status_t bt_app_comm_at_cmd_disconnect_all_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    app_dongle_ull_le_hid_disconnect_all_device(BT_HCI_STATUS_UNSPECIFIED_ERROR);
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};

    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}

static atci_status_t bt_app_comm_at_cmd_set_timer_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};

    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            uint16_t timer = atoi(parse_cmd->string_ptr + parse_cmd->name_len + 1);
            LOG_MSGID_I(BT_APP, "set timer :%d", 1, timer);
            extern uint16_t g_app_hid_establish_wait;
            g_app_hid_establish_wait = timer;
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

static atci_status_t bt_app_comm_at_cmd_fota_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};

    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            /* AT+FOTA=<action>
                        * action: "CANCEL"
                        */
            if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "CANCEL", 6)) {
#if defined(MTK_FOTA_VIA_RACE_CMD) && defined(MTK_RACE_CMD_ENABLE)
                /* STOP FOTA */
                if (RACE_ERRCODE_SUCCESS == race_fota_cancel()) {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                }
#endif
            }

            break;
        }

        default:
            break;
    }

    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}

void bt_app_common_at_cmd_print_report(char *string)
{
    atci_response_t *response = (atci_response_t *)pvPortMalloc(sizeof(atci_response_t));
    if (response != NULL) {
        memset(response, 0, sizeof(*response));
    } else {
        return;
    }
    snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE,
             "%s\n", (char *)string);
    response->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
    response->response_len = strlen((char *)response->response_buf);
    if (response->response_len > 0) {
        atci_send_response(response);
    }
    vPortFree(response);
    return;
}

#if defined(MTK_AWS_MCE_ENABLE) && defined(SUPPORT_ROLE_HANDOVER_SERVICE)
void bt_app_common_at_cmd_rho_srv_status_callback(const bt_bd_addr_t *addr, bt_aws_mce_role_t role, bt_role_handover_event_t event, bt_status_t status)
{
    atci_response_t *response = (atci_response_t *)pvPortMalloc(sizeof(atci_response_t));
    LOG_MSGID_I(BT_APP, "RHO srv status_callback role 0x%x, event %d, status 0x%x", 3, role, event, status);
    if (response != NULL) {
        memset(response, 0, sizeof(*response));
    } else {
        return;
    }
    switch (event) {
        case BT_ROLE_HANDOVER_PREPARE_REQ_IND: {
            snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE,
                     "Alert:RHO prepare\r\n");
            break;
        }
        case BT_ROLE_HANDOVER_START_IND: {
            snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE,
                     "Alert:RHO start\r\n");
            break;
        }
        case BT_ROLE_HANDOVER_COMPLETE_IND: {
            uint8_t role_string[20] = {0};
            if (status == BT_STATUS_SUCCESS) {
                if (role == BT_AWS_MCE_ROLE_AGENT) {
                    strcpy((char *)role_string, "(Partner now).\r\n");
                } else if (role == BT_AWS_MCE_ROLE_PARTNER) {
                    strcpy((char *)role_string, "(Agent now).\r\n");
                } else {
                    strcpy((char *)role_string, "(error now)!\r\n");
                }
                snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE,
                         "Alert:RHO succuss%s", (char *)role_string);
            } else {
                if (role == BT_AWS_MCE_ROLE_AGENT) {
                    strcpy((char *)role_string, "(Agent now).\r\n");
                } else if (role == BT_AWS_MCE_ROLE_PARTNER) {
                    strcpy((char *)role_string, "(Partner now).\r\n");
                } else {
                    strcpy((char *)role_string, "(error now)!\r\n");
                }
                snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE,
                         "Alert:RHO fail%s", (char *)role_string);
            }
            break;
        }
    }

    response->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
    response->response_len = strlen((char *)response->response_buf);
    if (response->response_len > 0) {
        atci_send_response(response);
    }
    vPortFree(response);
}
#ifdef MTK_BT_TIMER_EXTERNAL_ENABLE
#include "bt_timer_external.h"
#define BT_EXT_TIMER_TEST_TIMER_ID             (BT_TIMER_EXT_GROUP_SINK(9))
#define BT_EXT_TIMER_TEST_TIMER_DUR            (20 * 1000)
uint32_t g_timer_data = 0x1234;

void bt_app_common_timer_test_cb(uint32_t timer_id, uint32_t data)
{
    LOG_MSGID_I(BT_APP, "bt_app_common_timer_test_cb, id is 0x%8x, data is 0x%4x\r\n", 2, timer_id, data);
}
#endif
static atci_status_t bt_app_comm_at_cmd_rho_hdl(atci_parse_cmd_param_t *parse_cmd)
{
#if defined(SUPPORT_ROLE_HANDOVER_SERVICE)
    uint32_t response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
    LOG_MSGID_I(BT_APP, "bt_app_comm_at_cmd_rho_hdl", 0);

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_ACTIVE: {
            /* AT+RHO */
            bt_status_t status = bt_role_handover_start();
            if (BT_STATUS_SUCCESS == status || BT_STATUS_BUSY == status) {
                response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            } else {
                LOG_MSGID_I(BT_APP, "bt_app_comm_at_cmd_rho_hdl fail 0x%x", 1, status);
            }

            break;
        }

        case ATCI_CMD_MODE_EXECUTION: {
            /* AT+RHO=<action>
                        * action: "TIMER"
                        */
            if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "TIMER", 5)) {
                /* START A TIMER */
                LOG_MSGID_I(BT_APP, "start an 20s test timer!", 0);
#ifdef MTK_BT_TIMER_EXTERNAL_ENABLE
                bt_timer_ext_status_t timer_ret = BT_TIMER_EXT_STATUS_SUCCESS;
                timer_ret = bt_timer_ext_start(BT_EXT_TIMER_TEST_TIMER_ID, (uint32_t)g_timer_data, BT_EXT_TIMER_TEST_TIMER_DUR, bt_app_common_timer_test_cb);
                if (timer_ret != BT_TIMER_EXT_STATUS_SUCCESS) {
                    LOG_MSGID_I(BT_APP, "start test timer fail!", 0);
                }
                LOG_MSGID_I(BT_APP, "start test timer success, id is 0x%8x, data is 0x%4x\r\n", 2, BT_EXT_TIMER_TEST_TIMER_ID, g_timer_data);
                response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
#endif
            }

            break;
        }

        default:
            break;
    }

    atci_response_t *response = (atci_response_t *)pvPortMalloc(sizeof(atci_response_t));
    if (response != NULL) {
        memset(response, 0, sizeof(*response));
    } else {
        return ATCI_RESPONSE_FLAG_APPEND_ERROR;
    }

    response->response_flag = response_flag;
    response->response_len = strlen((char *)response->response_buf);
    atci_send_response(response);
    vPortFree(response);
#endif /*defined(SUPPORT_ROLE_HANDOVER_SERVICE)*/
    return ATCI_STATUS_OK;
}
#endif

static atci_status_t bt_app_comm_at_cmd_ble_scan_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};

    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            /* AT+BLESCAN=<action>
                        * action: "ON" / "OFF"
                        */
            if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "OFF", 3)) {
                /* BLE SCAN OFF */
                bt_app_common_stop_scan();
                response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            } else if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "ON", 2)) {
                /* BLE SCAN ON */
                bt_app_common_start_scan();
                response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            }

            break;
        }

        default:
            break;
    }

    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}

static atci_status_t bt_app_comm_at_cmd_ble_cancel_conn_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};

    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_ACTIVE: {
            /* AT+BLECANCELCONN*/
            bt_gap_le_cancel_connection();
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

static atci_status_t bt_app_comm_at_cmd_ble_random_addr_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};

    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            /* AT+BLERANDOMADDR=<action>
                        * action: "GET"
                        */
            if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "GET", 3)) {
                uint8_t temp_str[30] = {0};
                bt_bd_addr_t *addr = NULL;
                addr = (bt_bd_addr_t *)bt_app_common_get_local_random_addr();
                snprintf((char *)temp_str, sizeof(temp_str), "0x%.2X:%.2X:%.2X:%.2X:%.2X:%.2X",
                         (*addr)[5], (*addr)[4], (*addr)[3], (*addr)[2], (*addr)[1], (*addr)[0]);
                snprintf((char *)response.response_buf, sizeof(response.response_buf), "+Get addrss:%s\r\n", (char *)temp_str);
                response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            }

            break;
        }

        default:
            break;
    }

    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}

static atci_status_t bt_app_comm_at_cmd_set_fast_pair_tx_power_level(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t *response = (atci_response_t *)malloc(sizeof(atci_response_t));
    if (NULL == response) {
        return ATCI_STATUS_ERROR;
    }
    memset(response, 0 , sizeof(atci_response_t));
    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;

#ifdef AIR_BT_FAST_PAIR_ENABLE
    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            int8_t tx_power_level = atoi(parse_cmd->string_ptr + parse_cmd->name_len + 1);
            LOG_MSGID_I(BT_APP, "set fast pair tx power level :%d", 1, tx_power_level);

            app_fast_pair_set_tx_power_level(tx_power_level);
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            break;
        }
        default:
            break;
    }
#endif

    response->response_len = strlen((char *)response->response_buf);
    atci_send_response(response);
    free(response);
    return ATCI_STATUS_OK;
}

#ifdef MTK_IAP2_PROFILE_ENABLE

#define MAX_MFI_CERT_DATA_LEN 640
static atci_status_t bt_app_comm_at_cmd_get_mfi_cert_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    int                 r;
    uint8_t             *accessory_cert;
    uint32_t            accessory_cert_len = MAX_MFI_CERT_DATA_LEN;
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};

    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            accessory_cert = (uint8_t *)pvPortMalloc(MAX_MFI_CERT_DATA_LEN);
            if (!accessory_cert) {
                APPS_LOG_MSGID_I("malloc mfi cert data buf failed.", 0);
                break;
            }
            memset(accessory_cert, 0, MAX_MFI_CERT_DATA_LEN);

            r = mfi_coprocessor_get_accessory_cert(&accessory_cert, &accessory_cert_len);
            if (r < 0) {
                APPS_LOG_MSGID_I("get mfi cert data failed.", 0);
            } else {
                APPS_LOG_MSGID_I("get mfi cert data len: %d.", 1, accessory_cert_len);
                APPS_LOG_DUMP_I("mfi data:", accessory_cert, accessory_cert_len, 0);
            }
            vPortFree(accessory_cert);
        }
        default:
            break;
    }

    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}


#define MAX_MFI_RESP_DATA_LEN 64
#define MAX_MFI_CHALLENGE_DATA_LEN 32
static uint8_t mfi_challenge_data[MAX_MFI_CHALLENGE_DATA_LEN] = {0};
static atci_status_t bt_app_comm_at_cmd_get_mfi_response_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    int                 r;
    uint8_t             *resp_data;
    uint32_t            resp_data_len = MAX_MFI_RESP_DATA_LEN;
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};

    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            resp_data = (uint8_t *)pvPortMalloc(MAX_MFI_RESP_DATA_LEN);
            if (!resp_data) {
                APPS_LOG_MSGID_I("malloc mfi resp data buf failed.", 0);
                break;
            }
            memset(resp_data, 0, MAX_MFI_CERT_DATA_LEN);

            r = mfi_coprocessor_gen_challenge_response(mfi_challenge_data, MAX_MFI_CHALLENGE_DATA_LEN, &resp_data, &resp_data_len);
            if (r < 0) {
                APPS_LOG_MSGID_I("get mfi resp data failed.", 0);
            } else {
                APPS_LOG_MSGID_I("get mfi resp data len: %d.", 1, resp_data_len);
                APPS_LOG_DUMP_I("mfi data:", resp_data, resp_data_len, 0);
            }
            vPortFree(resp_data);
        }
        default:
            break;
    }

    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}


#define REG_DATA_LEN_1 1
#define REG_DATA_LEN_4 4
static bool check_mfi_version_data(atci_response_t *response)
{
    uint8_t temp_buf[4] = {0};
    int32_t ret = 0;
    char *ret_str = NULL;

    /* get device version */
    ret = aacp_i2c_read(AACP_DEVICE_VERSION, temp_buf, REG_DATA_LEN_1);
    if (ret != REG_DATA_LEN_1) {
        ret_str = "FAIL: get device version fail.";
        goto func_ret;
    }
    if (temp_buf[0] != 0x07) {
        ret_str = "FAIL: invalid device version";
        goto func_ret;
    }

    /* get authentication revision */
    memset(temp_buf, 0x00, 4);
    ret = aacp_i2c_read(AACP_FIRMWARE_VERSION, temp_buf, REG_DATA_LEN_1);
    if (ret != REG_DATA_LEN_1) {
        ret_str = "FAIL: get authentication version fail.";
        goto func_ret;
    }
    if (temp_buf[0] != 0x01) {
        ret_str = "FIAL: invalid authentication version.";
        goto func_ret;
    }

    /* get authentication protocal major version */
    memset(temp_buf, 0x00, 4);
    ret = aacp_i2c_read(AACP_AUTH_PROTO_MAJOR_VERSION, temp_buf, REG_DATA_LEN_1);
    if (ret != REG_DATA_LEN_1) {
        ret_str = "FAIL: get authentication protocal major version fail.";
        goto func_ret;
    }
    if (temp_buf[0] != 0x03) {
        ret_str = "FIAL: invalid authentication protocal major version.";
        goto func_ret;
    }

    /* get authentication protocal minor version */
    memset(temp_buf, 0x00, 4);
    ret = aacp_i2c_read(AACP_AUTH_PROTO_MINOR_VERSION, temp_buf, REG_DATA_LEN_1);
    if (ret != REG_DATA_LEN_1) {
        ret_str = "FAIL: get authentication protocal minor version fail.";
        goto func_ret;
    }
    if (temp_buf[0] != 0x00) {
        ret_str = "FIAL: invalid authentication protocal minor version.";
        goto func_ret;
    }

    /* get device id */
    memset(temp_buf, 0x00, 4);
    ret = aacp_i2c_read(AACP_DEVICE_ID, temp_buf, REG_DATA_LEN_4);
    if (ret != REG_DATA_LEN_4) {
        ret_str = "FAIL: get device id fail.";
        goto func_ret;
    }
    if (temp_buf[0] != 0x00 || temp_buf[1] != 0x00 ||
        temp_buf[2] != 0x03 || temp_buf[3] != 0x00) {
        ret_str = "FIAL: invalid device id.";
        goto func_ret;
    }

func_ret:
    if (ret_str != NULL) {
        snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, ret_str);
        response->response_flag = ATCI_RESPONSE_FLAG_AUTO_APPEND_LF_CR;
        return false;
    }
    return true;
}


static bool check_mfi_certifacation_data(atci_response_t *response)
{
    uint32_t idx = 0;
    bool check_result = false;
    int32_t ret = 0;
    uint8_t *accessory_cert;
    uint32_t accessory_cert_len = MAX_MFI_CERT_DATA_LEN;

    /* request certification data */
    accessory_cert = (uint8_t *)pvPortMalloc(MAX_MFI_CERT_DATA_LEN);
    if (!accessory_cert) {
        snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE,
                 "FAIL: malloc memory fail.");
        response->response_flag = ATCI_RESPONSE_FLAG_AUTO_APPEND_LF_CR;
        goto func_ret;
    }

    memset(accessory_cert, 0x00, MAX_MFI_CERT_DATA_LEN);
    ret = mfi_coprocessor_get_accessory_cert(&accessory_cert, &accessory_cert_len);
    if (ret < 0) {
        snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE,
                 "FAIL: get certification data fail.");
        response->response_flag = ATCI_RESPONSE_FLAG_AUTO_APPEND_LF_CR;
        goto func_ret;
    }

    /* check certification data */
    //on aacp 3.0, the certification data length is between 607 and 609, inclusive.
    if (accessory_cert_len < 607 || accessory_cert_len > 609) {
        snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE,
                 "FAIL: certification data length is %d.", (int)accessory_cert_len);
        response->response_flag = ATCI_RESPONSE_FLAG_AUTO_APPEND_LF_CR;
        goto func_ret;
    }

    for (idx = 0; idx < accessory_cert_len; idx++) {
        if (accessory_cert[idx] != 0x00) {
            break;
        }
    }
    if (idx == accessory_cert_len) {
        snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE,
                 "FAIL: certification data is padding by 0x00.");
        response->response_flag = ATCI_RESPONSE_FLAG_AUTO_APPEND_LF_CR;
        goto func_ret;
    }

    if (accessory_cert != NULL) {
        vPortFree(accessory_cert);
        accessory_cert = NULL;
    }
    check_result = true;

func_ret:
    if (accessory_cert != NULL) {
        vPortFree(accessory_cert);
        accessory_cert = NULL;
    }
    return check_result;
}


static bool check_mfi_challenge_response(atci_response_t *response)
{
    bool check_result = false;
    int32_t ret = 0;
    uint32_t idx = 0;
    uint8_t *challenge_data = NULL;
    uint8_t *challenge_response_data = NULL;
    uint32_t challenge_response_len = 0;

    /* send dummy challenge data to chip */
    challenge_data = (uint8_t *)pvPortMalloc(MAX_MFI_CHALLENGE_DATA_LEN);
    if (!challenge_data) {
        snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE,
                 "FAIL: malloc memory fail.");
        response->response_flag = ATCI_RESPONSE_FLAG_AUTO_APPEND_LF_CR;
        goto func_ret;
    } else {
        memset(challenge_data, 0x77, MAX_MFI_CHALLENGE_DATA_LEN);
    }

    challenge_response_data = (uint8_t *)pvPortMalloc(MAX_MFI_RESP_DATA_LEN);
    if (!challenge_response_data) {
        snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE,
                 "FAIL: malloc memory fail.");
        response->response_flag = ATCI_RESPONSE_FLAG_AUTO_APPEND_LF_CR;
        goto func_ret;
    } else {
        memset(challenge_response_data, 0x00, MAX_MFI_RESP_DATA_LEN);
    }

    ret = mfi_coprocessor_gen_challenge_response(challenge_data,
                                                 MAX_MFI_CHALLENGE_DATA_LEN,
                                                 &challenge_response_data,
                                                 &challenge_response_len);
    if (ret < 0) {
        snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE,
                 "FAIL: get challenge response data fail.");
        response->response_flag = ATCI_RESPONSE_FLAG_AUTO_APPEND_LF_CR;
        goto func_ret;
    }

    /* check challenge response data */
    // on aacp 3.0, the challenge response data length is 64
    if (challenge_response_len != MAX_MFI_RESP_DATA_LEN) {
        snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE,
                 "FAIL: challenge response data length is %d.", (int)challenge_response_len);
        response->response_flag = ATCI_RESPONSE_FLAG_AUTO_APPEND_LF_CR;
        goto func_ret;
    }

    for (idx = 0; idx < challenge_response_len; idx++) {
        if (challenge_response_data[idx] != 0x00) {
            break;
        }
    }
    if (idx == challenge_response_len) {
        snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE,
                 "FAIL: challenge response data is padding by 0x00.");
        response->response_flag = ATCI_RESPONSE_FLAG_AUTO_APPEND_LF_CR;
        goto func_ret;
    }

    if (challenge_data != NULL) {
        vPortFree(challenge_data);
        challenge_data = NULL;
    }

    if (challenge_response_data != NULL) {
        vPortFree(challenge_response_data);
        challenge_response_data = NULL;
    }

    check_result = true;

func_ret:
    if (challenge_data != NULL) {
        vPortFree(challenge_data);
        challenge_data = NULL;
    }
    if (challenge_response_data != NULL) {
        vPortFree(challenge_response_data);
        challenge_response_data = NULL;
    }
    return check_result;
}


static atci_status_t bt_app_comm_at_cmd_check_mfi_chip_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};
    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;

    do {
        if (!check_mfi_version_data(&response)) {
            break;
        }

        if (!check_mfi_certifacation_data(&response)) {
            break;
        }

        if (!check_mfi_challenge_response(&response)) {
            break;
        }
    } while (0);

    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}

#endif

static atci_status_t bt_app_comm_at_cmd_ext_ble_adv_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t *response = pvPortMalloc(sizeof(atci_response_t));
    if (response == NULL) {
        return ATCI_STATUS_ERROR;
    }
    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            /* AT+EXTBLEADV=<action>,<instance>
                        * action: "ON" / "OFF"
                        */
            if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "OFF", 3)) {
                /* BLE ADV OFF */
                uint8_t instance = (uint8_t)strtoul(parse_cmd->string_ptr + parse_cmd->name_len + 5, NULL, 16);
                bt_status_t status = bt_app_common_ext_advertising_stop_test(instance);
                if (status == BT_STATUS_SUCCESS) {
                    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                }
            } else if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "ON", 2)) {
                /* BLE ADV ON */
                uint8_t instance = (uint8_t)strtoul(parse_cmd->string_ptr + parse_cmd->name_len + 4, NULL, 16);
                bt_status_t status = bt_app_common_ext_advertising_start_test(instance);
                if (status == BT_STATUS_SUCCESS) {
                    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                }
            }
            break;
        }

        default:
            break;
    }

    response->response_len = strlen((char *)response->response_buf);
    atci_send_response(response);
    vPortFree(response);
    return ATCI_STATUS_OK;
}

#ifdef AIR_LE_AUDIO_ENABLE
static uint32_t bt_app_comm_at_cmd_le_audio_scan_hdl(char *pChar)
{
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
    /* Scan device */
    /* AT+LEAUDIO=SCAN,<ACTION> */
    /* <ACTION>: ON, OFF, REST */

    if (0 == memcmp(pChar, "ON", 2)) {
        /* start scan devices */
        /* AT+LEAUDIO=SCAN,ON */
        app_le_audio_ucst_start_scan_device();

    } else if (0 == memcmp(pChar, "OFF", 3)) {
        /* stop scan devices */
        /* AT+LEAUDIO=SCAN,OFF */
        app_le_audio_ucst_stop_scan_device();

    } else if (0 == memcmp(pChar, "RESET", 5)) {
        /* Stop scan and start find devices */
        /* AT+LEAUDIO=SCAN,RESET */
        app_le_audio_ucst_find_device();

    } else {
        return ATCI_RESPONSE_FLAG_APPEND_ERROR;
    }

    return ATCI_RESPONSE_FLAG_APPEND_OK;
#else
    return ATCI_RESPONSE_FLAG_APPEND_ERROR;
#endif
}

static uint32_t bt_app_comm_at_cmd_le_audio_bond_hdl(char *pChar)
{
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
    /* Bonding information */
    /* AT+LEAUDIO=BOND,<ACTION> */
    /* <ACTION>: RM */

    if (0 == memcmp(pChar, "RM", 2)) {
        /* Remove bond */
        /* AT+LEAUDIO=BOND,RM */
        app_le_audio_ucst_reset_all_bonded_info();

    } else {
        return ATCI_RESPONSE_FLAG_APPEND_ERROR;
    }

    return ATCI_RESPONSE_FLAG_APPEND_OK;
#else
    return ATCI_RESPONSE_FLAG_APPEND_ERROR;
#endif
}

static uint32_t bt_app_comm_at_cmd_le_audio_sirk_hdl(char *pChar)
{
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
    /* Set SIRK */
    /* AT+LEAUDIO=SIRK,<ACTION>,<PARAM> */
    /* <ACTION>: SET, GET */

    if (0 == memcmp(pChar, "SET", 3)) {
        bt_key_t sirk = {0};
        unsigned int value;
        uint8_t i;

        /* Set SIRK */
        /* AT+LEAUDIO=SIRK,SET,<B0>,<B1>,<B2>,<B3>,<B4>,<B5>,<B6>,<B7>,<B8>,<B9>,<B10>,<B11>,<B12>,<B13>,<B14>,<B15> */

        pChar = strchr(pChar, ',');
        pChar++;

        for (i = 0; i < BT_KEY_SIZE; i++) {
            sscanf(pChar, "%x,", &value);
            sirk[i] = (uint8_t)value;
            pChar += 3;
            //LOG_MSGID_I(BT_APP, "[LE_AUDIO][AT] i: %x %x", 2, i, sirk[i], value);
        }

        LOG_MSGID_I(BT_APP, "[LE_AUDIO][AT] SIRK: %x-%x-%x-%x-%x-%x-%x-%x", 8, sirk[0], sirk[1], sirk[2], sirk[3], sirk[4], sirk[5], sirk[6], sirk[7]);
        LOG_MSGID_I(BT_APP, "[LE_AUDIO][AT] SIRK: %x-%x-%x-%x-%x-%x-%x-%x", 8, sirk[8], sirk[9], sirk[10], sirk[11], sirk[12], sirk[13], sirk[14], sirk[15]);

        /* Connect coordinated set by SIRK */
        app_le_audio_ucst_set_sirk(&sirk, true);
        return ATCI_RESPONSE_FLAG_APPEND_OK;

    } else if (0 == memcmp(pChar, "GET", 3)) {
        char conn_string[60] = {0};
        uint8_t *p_sirk = NULL;

        /* Get SIRK */
        /* AT+LEAUDIO=SIRK,GET\0d\0a */

        if (NULL != (p_sirk = (uint8_t *)app_le_audio_ucst_get_sirk(true))) {
            snprintf((char *)conn_string, 60, "SIRK:%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X\r\n",
                     p_sirk[0], p_sirk[1], p_sirk[2], p_sirk[3], p_sirk[4], p_sirk[5], p_sirk[6], p_sirk[7],
                     p_sirk[8], p_sirk[9], p_sirk[10], p_sirk[11], p_sirk[12], p_sirk[13], p_sirk[14], p_sirk[15]);
            bt_app_common_at_cmd_print_report(conn_string);
            return ATCI_RESPONSE_FLAG_APPEND_OK;
        }
    }
#endif
    return ATCI_RESPONSE_FLAG_APPEND_ERROR;
}

#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
static uint32_t bt_app_comm_at_cmd_le_audio_conn_by_sirk_hdl(char *pChar)
{
    bt_key_t sirk = {0};
    unsigned int value;
    uint8_t i;

    /* Connect coordinated set by SIRK */
    /* AT+LEAUDIO=CONN,SIRK,<B0>,<B1>,<B2>,<B3>,<B4>,<B5>,<B6>,<B7>,<B8>,<B9>,<B10>,<B11>,<B12>,<B13>,<B14>,<B15> */

    for (i = 0; i < BT_KEY_SIZE; i++) {
        sscanf(pChar, "%x,", &value);
        sirk[i] = (uint8_t)value;
        pChar += 3;
    }

    //LOG_MSGID_I(BT_APP, "[LE_AUDIO][AT] SIRK: %x-%x-%x-%x-%x-%x-%x-%x", 8, sirk[0], sirk[1], sirk[2], sirk[3], sirk[4], sirk[5], sirk[6], sirk[7]);
    //LOG_MSGID_I(BT_APP, "[LE_AUDIO][AT] SIRK: %x-%x-%x-%x-%x-%x-%x-%x", 8, sirk[8], sirk[9], sirk[10], sirk[11], sirk[12], sirk[13], sirk[14], sirk[15]);

    app_le_audio_ucst_set_sirk(&sirk, true);
    app_le_audio_ucst_connect_coordinated_set(true);

    return ATCI_RESPONSE_FLAG_APPEND_OK;
}
#endif

#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
extern bt_status_t app_le_audio_start_unicast_ex(const bt_addr_t *addr);
static uint32_t bt_app_comm_at_cmd_le_audio_conn_addr_hdl(char *pChar)
{
    bt_status_t ret = BT_STATUS_FAIL;
    bt_addr_t addr = {0};
    unsigned int value;
    uint8_t i;

    /* Connect device by bd_addr */
    /* AT+LEAUDIO=CONN,ADDR,<ADDR_TYPE>,<B0>:<B1>:<B2>:<B3>:<B4>:<B5> */
    /* <ADDR_TYPE>: 0(public), 1(random) */

    /* address type */
    sscanf(pChar, "%x,", &value);
    addr.type = (uint8_t)value;
    pChar = strchr(pChar, ',');
    pChar++;

    /* address */
    for (i = 0; i < BT_BD_ADDR_LEN; i++) {
        sscanf(pChar, "%x:", &value);
        addr.addr[5 - i] = (uint8_t)value;
        pChar += 3;
    }

    ret = app_le_audio_start_unicast_ex(&addr);

    LOG_MSGID_I(BT_APP, "[LE_AUDIO][AT] C LEA addrType:%x addr:%2x:%2x:%2x:%2x:%2x:%2x, status:0x%x", 8,
                addr.type,
                addr.addr[5],
                addr.addr[4],
                addr.addr[3],
                addr.addr[2],
                addr.addr[1],
                addr.addr[0],
                ret);

    return ATCI_RESPONSE_FLAG_APPEND_OK;
}
#endif

static uint32_t bt_app_comm_at_cmd_le_audio_conn_hdl(char *pChar)
{
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
    /* Create connection */
    /* AT+LEAUDIO=CONN,<ACTION>,<PARAM> */
    /* <ACTION>: SIRK, ADDR */

    if (0 == memcmp(pChar, "SIRK", 4)) {
        /* Connect coordinated set by SIRK */
        /* AT+LEAUDIO=CONN,SIRK,<B0>,<B1>,<B2>,<B3>,<B4>,<B5>,<B6>,<B7>,<B8>,<B9>,<B10>,<B11>,<B12>,<B13>,<B14>,<B15> */
        pChar = strchr(pChar, ',');
        pChar++;
        return bt_app_comm_at_cmd_le_audio_conn_by_sirk_hdl(pChar);

    } else if (0 == memcmp(pChar, "ADDR", 4)) {
        /* Connect device by bd_addr */
        /* AT+LEAUDIO=CONN,ADDR,<ADDR_TYPE>,<B0>:<B1>:<B2>:<B3>:<B4>:<B5> */
        pChar = strchr(pChar, ',');
        pChar++;
        return bt_app_comm_at_cmd_le_audio_conn_addr_hdl(pChar);
    }
#endif
    return ATCI_RESPONSE_FLAG_APPEND_ERROR;
}

static uint32_t bt_app_comm_at_cmd_le_audio_disconn_hdl(char *pChar)
{
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
    bt_status_t ret = BT_STATUS_FAIL;
    bt_addr_t addr = {0};
    unsigned int value;
    uint8_t i;

    /* Disconnection */
    /* AT+LEAUDIO=DISCONN,<ADDR_TYPE>,<B0>:<B1>:<B2>:<B3>:<B4>:<B5> */
    /* <ADDR_TYPE>: 0(public), 1(random) */

    /* address type */
    sscanf(pChar, "%x,", &value);
    addr.type = (uint8_t)value;
    pChar = strchr(pChar, ',');
    pChar++;

    /* address */
    for (i = 0; i < BT_BD_ADDR_LEN; i++) {
        sscanf(pChar, "%x:", &value);
        addr.addr[5 - i] = (uint8_t)value;
        pChar += 3;
    }

    ret = app_le_audio_ucst_disconnect_device(&addr);

    LOG_MSGID_I(BT_APP, "[LE_AUDIO][AT] D LEA addrType:%x addr:%2x:%2x:%2x:%2x:%2x:%2x, status:0x%x", 8,
                addr.type,
                addr.addr[5],
                addr.addr[4],
                addr.addr[3],
                addr.addr[2],
                addr.addr[1],
                addr.addr[0],
                ret);

    return ATCI_RESPONSE_FLAG_APPEND_OK;
#else
    return ATCI_RESPONSE_FLAG_APPEND_ERROR;
#endif
}

#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
static uint32_t bt_app_comm_at_cmd_le_audio_mode_hdl(char *pChar)
{
    bt_hci_cmd_le_set_privacy_mode_t params = {{0},0};
    bt_status_t ret = BT_STATUS_FAIL;
    unsigned int value;
    uint8_t i;

    /* Set Device Mode */
    /* AT+LEAUDIO=MODE,<Device_Mode>,<ADDR_TYPE>,<B0>:<B1>:<B2>:<B3>:<B4>:<B5> */
    /* <Device_Mode>: 0(network), 1(device) */
    /* <ADDR_TYPE>  : 0(public) , 1(random) */

    /* device mode */
    sscanf(pChar, "%x,", &value);
    params.privacy_mode = (uint8_t)value;
    pChar = strchr(pChar, ',');
    pChar++;

    /* address type */
    sscanf(pChar, "%x,", &value);
    params.peer_address.type = (uint8_t)value;
    pChar = strchr(pChar, ',');
    pChar++;

    /* address */
    for (i = 0; i < BT_BD_ADDR_LEN; i++) {
        sscanf(pChar, "%x:", &value);
        params.peer_address.addr[5-i] = (uint8_t)value;
        pChar += 3;
    }

    ret = bt_gap_le_set_privacy_mode(&params);

    LOG_MSGID_I(BT_APP, "[LE_AUDIO][AT] D LEA mode:%x addrType:%x addr:%2x:%2x:%2x:%2x:%2x:%2x, status:0x%x", 9,
                params.privacy_mode,
                params.peer_address.type,
                params.peer_address.addr[5],
                params.peer_address.addr[4],
                params.peer_address.addr[3],
                params.peer_address.addr[2],
                params.peer_address.addr[1],
                params.peer_address.addr[0],
                ret);

    return ATCI_RESPONSE_FLAG_APPEND_OK;
}
#endif

static uint32_t bt_app_comm_at_cmd_le_audio_delete_hdl(char *pChar)
{
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
    bt_addr_t addr = {0};
    unsigned int value;
    uint8_t i;

    /* Disconnection */
    /* AT+LEAUDIO=DELETE,<ADDR_TYPE>,<B0>:<B1>:<B2>:<B3>:<B4>:<B5> */
    /* <ADDR_TYPE>: 0(public), 1(random) */

    /* address type */
    sscanf(pChar, "%x,", &value);
    addr.type = (uint8_t)value;
    pChar = strchr(pChar, ',');
    pChar++;

    /* address */
    for (i = 0; i < BT_BD_ADDR_LEN; i++) {
        sscanf(pChar, "%x:", &value);
        addr.addr[5 - i] = (uint8_t)value;
        pChar += 3;
    }

    app_le_audio_ucst_delete_device(&addr);

    LOG_MSGID_I(BT_APP, "[LE_AUDIO][AT] Del LEA addrType:%x addr:%2x:%2x:%2x:%2x:%2x:%2x", 7,
                addr.type,
                addr.addr[5],
                addr.addr[4],
                addr.addr[3],
                addr.addr[2],
                addr.addr[1],
                addr.addr[0]);

    return ATCI_RESPONSE_FLAG_APPEND_OK;
#else
    return ATCI_RESPONSE_FLAG_APPEND_ERROR;
#endif
}

#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
static uint32_t bt_app_comm_at_cmd_le_audio_call_hdl(char *pChar)
{
    bt_status_t result = BT_STATUS_UNSUPPORTED;
    uint8_t service_idx = ble_tbs_get_gtbs_service_idx();

    /* Call */
    /* AT+LEAUDIO=CALL,<ACTION> */
    /* <ACTION>: END */

    if (0 == memcmp(pChar, "END", 3)) {
        bt_le_audio_source_call_ended_t le_param = {
            .service_index = service_idx,
            .call_index = 1,
        };

        result = bt_le_audio_source_send_action(BT_LE_AUDIO_SOURCE_ACTION_CALL_ENDED, &le_param);
    }

    LOG_MSGID_I(BT_APP, "[AT_CMD] Call result:%x \r\n", 1, result);

    if (BT_STATUS_SUCCESS == result) {
        return ATCI_RESPONSE_FLAG_APPEND_OK;
    }

    return ATCI_RESPONSE_FLAG_APPEND_ERROR;
}
#endif

static uint32_t bt_app_comm_at_cmd_le_audio_mic_hdl(char *pChar, atci_response_t *response)
{
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
    bt_status_t result = BT_STATUS_UNSUPPORTED;

    /* AT+LEAUDIO=MIC,<ACTION> */
    /* <ACTION>: MUTE, UNMUTE */

    if (0 == memcmp(pChar, "MUTE", 4)) {
        /* AT+LEAUDIO=MIC,MUTE */
        result = app_le_audio_micp_control_device(APP_LEA_MICP_OPERATE_MUTE, NULL);

    } else if (0 == memcmp(pChar, "UNMUTE", 6)) {
        /* AT+LEAUDIO=MIC,UNMUTE */
        result = app_le_audio_micp_control_device(APP_LEA_MICP_OPERATE_UNMUTE, NULL);
    } else if (0 == memcmp(pChar, "READ_MUTE", 9)) {
        bt_handle_t handle;
        uint8_t i;
        /* AT+LEAUDIO=MIC,READ_MUTE */
        for (i = 0; i < APP_LE_AUDIO_UCST_LINK_MAX_NUM; i++) {
            if (BT_HANDLE_INVALID != (handle = app_le_audio_ucst_get_handle(i))) {
                result = ble_micp_enhance_mics_read_mute(handle);
                break;
            }
        }
    }
#ifdef APP_LE_AUDIO_UCST_UPLINK_MIX_ENABLE
    else if (0 == memcmp(pChar, "MIX,SET", 7)) {
        /* AT+LEAUDIO=MIC,MIX,SET,<enable>; 0:disable, 1:enable, other:reserved. */
        int32_t enable = 0;

        pChar = strchr(pChar, ',');
        pChar++;

        pChar = strchr(pChar, ',');
        pChar++;

        enable = atoi(pChar);
        LOG_MSGID_I(BT_APP, "[AT_CMD] enable uplink mix:%d \r\n", 1, enable);
        if (0 == enable || 1 == enable) {
            app_le_audio_ucst_set_uplink_mix_status(1 == enable);
            app_le_audio_ucst_set_mic_channel();
            result = BT_STATUS_SUCCESS;
        }
    } else if (0 == memcmp(pChar, "MIX,GET", 7)) {
        /* AT+LEAUDIO=MIC,MIX,GET; 0:disable, 1:enable, other:reserved. */
        snprintf((char *)response->response_buf, sizeof(response->response_buf), "+Get MIC MIX enable:%d\r\n", app_le_audio_ucst_get_uplink_mix_status());
        result = BT_STATUS_SUCCESS;
    }
#endif
    LOG_MSGID_I(BT_APP, "[AT_CMD] MIC result:%x \r\n", 1, result);

    if (BT_STATUS_SUCCESS == result) {
        return ATCI_RESPONSE_FLAG_APPEND_OK;
    }
#endif
    return ATCI_RESPONSE_FLAG_APPEND_ERROR;
}

extern uint8_t g_lea_ucst_pts_set_size;

static uint32_t bt_app_comm_at_cmd_le_audio_csip_hdl(char *pChar)
{
    bt_status_t result = BT_STATUS_UNSUPPORTED;
    bt_handle_t handle = bt_app_common_get_first_conneciton_handle();

    /* AT+LEAUDIO=CSIP,<CONN HDL>,<ACTION> */
    /* <CONN HDL>: connection handle */
    /* <ACTION>: READ_XXX, SET_XXX */

#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
    handle = atoi(pChar);

    pChar = strchr(pChar, ',');
    pChar++;

    if (0 == memcmp(pChar, "READ_SIRK", 9)) {
        /* AT+LEAUDIO=CSIP,<CONN HDL>,READ_SIRK */
        result = ble_csip_read_sirk_req(handle);

    } else if (0 == memcmp(pChar, "READ_COORDINATED_SET_SIZE", 25)) {
        /* AT+LEAUDIO=CSIP,<CONN HDL>,READ_COORDINATED_SET_SIZE */
        result = ble_csip_read_coordinated_set_size_req(handle);

    } else if (0 == memcmp(pChar, "READ_SET_MEMBET_LOCK", 20)) {
        /* AT+LEAUDIO=CSIP,<CONN HDL>,READ_SET_MEMBET_LOCK */
        result = ble_csip_read_set_member_lock_req(handle);

    } else if (0 == memcmp(pChar, "READ_SET_MEMBET_RANK", 20)) {
        /* AT+LEAUDIO=CSIP,<CONN HDL>,READ_SET_MEMBET_RANK */
        result = ble_csip_read_set_member_rank_req(handle);

    } else if (0 == memcmp(pChar, "SET_LOCK", 8)) {
        /* AT+LEAUDIO=CSIP,<CONN HDL>,SET_LOCK */
        result = ble_csip_write_set_member_lock_req(handle, 0x02);

    } else if (0 == memcmp(pChar, "SET_UNLOCK", 10)) {
        /* AT+LEAUDIO=CSIP,<CONN HDL>,SET_UNLOCK */
        result = ble_csip_write_set_member_lock_req(handle, 0x01);

    } else if (0 == memcmp(pChar, "SET_UNVALID_LOCK", 16)) {
        /* AT+LEAUDIO=CSIP,<CONN HDL>,SET_UNVALID_LOCK */
        result = ble_csip_write_set_member_lock_req(handle, 0x00);

    } else if (0 == memcmp(pChar, "PTS_WRITE_SET_SIZE", 18)) {
        /* AT+LEAUDIO=CSIP,<CONN HDL>,PTS_WRITE_SET_SIZE */
        pChar = strchr(pChar, ',');
        pChar++;

        g_lea_ucst_pts_set_size = atoi(pChar);
        LOG_MSGID_I(BT_APP, "[AT_CMD] write PTS set size:%d \r\n", 1, g_lea_ucst_pts_set_size);
        result = BT_STATUS_SUCCESS;

    } else if (0 == memcmp(pChar, "CSIS", 4)) {
        //app_le_audio_pts_set_service_params(false, true);
        result = BT_STATUS_SUCCESS;

    }
#endif

    LOG_MSGID_I(BT_APP, "[AT_CMD] CSIP handle: 0x%x, result:0x%x \r\n", 2, handle, result);

    if (BT_STATUS_SUCCESS != result) {
        return ATCI_RESPONSE_FLAG_APPEND_ERROR;
    }

    return ATCI_RESPONSE_FLAG_APPEND_OK;
}

static uint32_t bt_app_comm_at_cmd_le_audio_volume_hdl(char *pChar)
{
    /* AT+LEAUDIO=VOLUME,<ACTION> */
    /* <ACTION>: MUTE, UNMUTE, UP, DOWN, SET */
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
    if (0 == memcmp(pChar, "MUTE", 4)) {
        /* AT+LEAUDIO=VOLUME,MUTE */
        app_le_audio_vcp_control_device_volume(APP_LEA_VCP_OPERATE_MUTE, NULL);

    } else if (0 == memcmp(pChar, "UNMUTE", 6)) {
        /* AT+LEAUDIO=VOLUME,UNMUTE */
        app_le_audio_vcp_control_device_volume(APP_LEA_VCP_OPERATE_UNMUTE, NULL);

    } else if (0 == memcmp(pChar, "UP", 2)) {
        /* AT+LEAUDIO=VOLUME,UP */
        app_le_audio_vcp_control_device_volume(APP_LEA_VCP_OPERATE_RELATIVE_VOLUME_UP, NULL);

    } else if (0 == memcmp(pChar, "DOWN", 4)) {
        /* AT+LEAUDIO=VOLUME,DOWN */
        app_le_audio_vcp_control_device_volume(APP_LEA_VCP_OPERATE_RELATIVE_VOLUME_DOWN, NULL);

    } else if (0 == memcmp(pChar, "SET", 3)) {
        ble_vcs_volume_t volume;
        /* Set absolute volume */
        /* AT+LEAUDIO=VOLUME,SET,<volume> */
        char *buf = (pChar + 4);
        volume = (uint8_t)strtoul(buf, NULL, 10);
        if (volume > 15) {
            return ATCI_RESPONSE_FLAG_APPEND_ERROR;
        }

        volume *= 0x11;

        app_le_audio_vcp_control_device_volume(APP_LEA_VCP_OPERATE_SET_ABSOLUTE_VOLUME, &volume);
    }
    else if (0 == memcmp(pChar, "READ_STATE", 10)) {
        bt_handle_t handle;
        uint8_t i;
        /* AT+LEAUDIO=VOLUME,READ_STATE */
        for (i = 0; i < APP_LE_AUDIO_UCST_LINK_MAX_NUM; i++) {
            if (BT_HANDLE_INVALID != (handle = app_le_audio_ucst_get_handle(i))) {
                ble_vcp_enhance_vcs_read_volume_state(handle);
            }
        }
    } else if (0 == memcmp(pChar, "READ_FLAG", 9)) {
        bt_handle_t handle;
        uint8_t i;
        for (i = 0; i < APP_LE_AUDIO_UCST_LINK_MAX_NUM; i++) {
            if (BT_HANDLE_INVALID != (handle = app_le_audio_ucst_get_handle(i))) {
                ble_vcp_enhance_vcs_read_volume_flags(handle);
            }
        }
    } else if (0 == memcmp(pChar, "PTS_SET", 7)) {
        //only for PTS set absolute volume
        bt_handle_t handle;
        uint8_t i;
        char *buf = (pChar + 8);
        uint8_t volume = (uint8_t)strtoul(buf, NULL, 10);
        for (i = 0; i < APP_LE_AUDIO_UCST_LINK_MAX_NUM; i++) {
            if (BT_HANDLE_INVALID != (handle = app_le_audio_ucst_get_handle(i))) {
                if (volume <= 15) {
                    volume *= 0x11;
                    app_le_audio_vcp_set_absolute_volume(handle, volume);
                }
            }
        }
    } else if (0 == memcmp(pChar, "R_DOWN", 6)) {
        bt_handle_t handle;
        uint8_t i;
        for (i = 0; i < APP_LE_AUDIO_UCST_LINK_MAX_NUM; i++) {
            if (BT_HANDLE_INVALID != (handle = app_le_audio_ucst_get_handle(i))) {
                app_le_audio_vcp_relative_volume_down(handle, false);
            }
        }
    } else if (0 == memcmp(pChar, "R_UP", 4)) {
        bt_handle_t handle;
        uint8_t i;
        for (i = 0; i < APP_LE_AUDIO_UCST_LINK_MAX_NUM; i++) {
            if (BT_HANDLE_INVALID != (handle = app_le_audio_ucst_get_handle(i))) {
                app_le_audio_vcp_relative_volume_up(handle, false);
            }
        }
    } else if (0 == memcmp(pChar, "UR_DOWN", 7)) {
        bt_handle_t handle;
        uint8_t i;
        for (i = 0; i < APP_LE_AUDIO_UCST_LINK_MAX_NUM; i++) {
            if (BT_HANDLE_INVALID != (handle = app_le_audio_ucst_get_handle(i))) {
                app_le_audio_vcp_relative_volume_down(handle, true);
            }
        }
    } else if (0 == memcmp(pChar, "UR_UP", 5)) {
        bt_handle_t handle;
        uint8_t i;
        for (i = 0; i < APP_LE_AUDIO_UCST_LINK_MAX_NUM; i++) {
            if (BT_HANDLE_INVALID != (handle = app_le_audio_ucst_get_handle(i))) {
                app_le_audio_vcp_relative_volume_up(handle, true);
            }
        }
    }
    else {
        return ATCI_RESPONSE_FLAG_APPEND_ERROR;
    }

    return ATCI_RESPONSE_FLAG_APPEND_OK;
#else
    return ATCI_RESPONSE_FLAG_APPEND_ERROR;
#endif
}

extern bool g_lea_bcst_pa_is_include_meatadata;
static uint32_t bt_app_comm_at_cmd_le_audio_broadcast_hdl(char *pChar)
{
#ifdef AIR_LE_AUDIO_BIS_ENABLE
    /* AT+LEAUDIO=BROADCAST,<ACTION> */
    /* <ACTION>: START, STOP */

    if (0 == memcmp(pChar, "START", 5)) {
        /* AT+LEAUDIO=BROADCAST,START */
        app_le_audio_start_broadcast();

    } else if (0 == memcmp(pChar, "STOP", 4)) {
        /* AT+LEAUDIO=BROADCAST,STOP */
        app_le_audio_stop_broadcast();

    } else if (0 == memcmp(pChar, "SET_METADATA_TRUE", 17)) {
        /* AT+LEAUDIO=BROADCAST,SET_METADATA_TRUE */
        g_lea_bcst_pa_is_include_meatadata = true;

    } else if (0 == memcmp(pChar, "SET_METADATA_FALSE", 18)) {
        /* AT+LEAUDIO=BROADCAST,SET_METADATA_FALSE */
        g_lea_bcst_pa_is_include_meatadata = false;

    } else if (0 == memcmp(pChar, "CODE", 4)) {
        /* AT+LEAUDIO=BROADCAST,CODE */
        uint8_t i, bcst_code[BLE_BASS_BROADCAST_CODE_SIZE] = {0};
        unsigned int value;
        pChar = strchr(pChar, ',');
        pChar++;
        /* code */
        for (i = 0; i < BLE_BASS_BROADCAST_CODE_SIZE; i++) {
            sscanf(pChar, "%2x,", &value);
            bcst_code[i] = value;
            pChar += 3;
        }
        app_le_audio_bcst_set_code(bcst_code);

    } else {
        return ATCI_RESPONSE_FLAG_APPEND_ERROR;
    }

    return ATCI_RESPONSE_FLAG_APPEND_OK;
#else
    return ATCI_RESPONSE_FLAG_APPEND_ERROR;
#endif
}

static uint32_t bt_app_comm_at_cmd_le_audio_unicast_hdl(char *pChar)
{
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
    /* AT+LEAUDIO=UNICAST,<ACTION> */
    /* <ACTION>: START, BI, GROUP */
    if (0 == memcmp(pChar, "START", 5)) {
        /* AT+LEAUDIO=UNICAST,START */
        app_le_audio_start_unicast();

    } else if (0 == memcmp(pChar, "BI", 2)) {
        /* AT+LEAUDIO=UNICAST,BI */
        app_le_audio_ucst_set_create_cis_mode(APP_LE_AUDIO_UCST_CREATE_CIS_ALWAYS_BIDIRECTIONAL);
        app_le_audio_ucst_start();

    } else if (0 == memcmp(pChar, "AC", 2)) {
        /* AT+LEAUDIO=UNICAST,AC,<AC_num> */
        uint8_t ac = APP_LE_AUDIO_UCST_CREATE_CIS_ALWAYS_BIDIRECTIONAL;
        pChar = strchr(pChar, ',');
        pChar++;
        ac += (uint8_t)strtoul(pChar, NULL, 10);
        app_le_audio_ucst_set_create_cis_mode(ac);
        app_le_audio_ucst_start();


    } else if (0 == memcmp(pChar, "CCID", 4)) {
        uint8_t ccid_num = 0;
        uint8_t ccid_list[2] = {0};
        /* AT+LEAUDIO=UNICAST,CCID,<num_of_CCID>,<CCID_0>,<CCID_1> */
        /* <num_of_CCID>: 0, 1, 2 */
        /* If num_of_CCID = 0, reset CCID list */
        /* If num_of_CCID = 1, then CCID_0 is supported */
        /* If num_of_CCID = 2, then CCID_1 is supported */
        pChar = strchr(pChar, ',');
        pChar++;

        if (sscanf(pChar, "%2x,%2x,%2x", (unsigned int *)&ccid_num, (unsigned int *)&ccid_list[0], (unsigned int *)&ccid_list[1]) > 0) {
            app_le_audio_ucst_set_ccid_list(ccid_num, ccid_list);
        }

#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
    } else if (0 == memcmp(pChar, "GROUP_GET", 9)) {
        /* AT+LEAUDIO=UNICAST,GROUP_GET */
        char conn_string[30] = {0};
        uint8_t group;
        group = app_le_audio_ucst_get_active_group();
        LOG_MSGID_I(BT_APP, "[LE_AUDIO][AT] ACTIVE_GROUP:%02x", 1, group);
        snprintf((char *)conn_string, 30, "ACTIVE_GROUP:%02x\r\n", group);
        bt_app_common_at_cmd_print_report(conn_string);

    } else if (0 == memcmp(pChar, "GROUP", 5)) {
        /* AT+LEAUDIO=UNICAST,GROUP,<group_id> */
        uint8_t group;
        pChar = strchr(pChar, ',');
        pChar++;
        group = (uint8_t)strtoul(pChar, NULL, 10);
        app_le_audio_ucst_set_active_group(group);

#endif

    } else if (0 == memcmp(pChar, "READ_CONTEXT", 12)) {
        bt_handle_t handle;
        uint8_t i;
        /* AT+LEAUDIO=UNICAST,READ_CONTEXT */
        for (i = 0; i < APP_LE_AUDIO_UCST_LINK_MAX_NUM; i++) {
            if (BT_HANDLE_INVALID != (handle = app_le_audio_ucst_get_handle(i))) {
                ble_bap_pacs_read_supported_audio_contexts_req(handle);
            }
        }

    } else if (0 == memcmp(pChar, "CODEC", 5)) {
        bt_handle_t handle;
        uint8_t i;
        for (i = 0; i < APP_LE_AUDIO_UCST_LINK_MAX_NUM; i++) {
            if (BT_HANDLE_INVALID != (handle = app_le_audio_ucst_get_handle(i))) {
                (void)app_le_audio_ucst_config_codec(handle);
            }
        }

    } else if (0 == memcmp(pChar, "METADATA", 7)) {
        bt_handle_t handle;
        uint8_t i;
        /* AT+LEAUDIO=UNICAST,METADATA */
        for (i = 0; i < APP_LE_AUDIO_UCST_LINK_MAX_NUM; i++) {
            if (BT_HANDLE_INVALID != (handle = app_le_audio_ucst_get_handle(i))) {
                app_le_audio_ucst_update_metadata(handle);
            }
        }
    } else if (0 == memcmp(pChar, "RELEASE", 7)) {
        bt_handle_t handle;
        uint8_t i;
        /* AT+LEAUDIO=UNICAST,RELEASE */
        for (i = 0; i < APP_LE_AUDIO_UCST_LINK_MAX_NUM; i++) {
            if (BT_HANDLE_INVALID != (handle = app_le_audio_ucst_get_handle(i))) {
                app_le_audio_ucst_release_ase(handle);
            }
        }
    }  else if (0 == memcmp(pChar, "RECEIVER_START_READY", 20)) {
        bt_handle_t handle;
        uint8_t i;
        /* AT+LEAUDIO=UNICAST,RECEIVER_START_READY */
        for (i = 0; i < APP_LE_AUDIO_UCST_LINK_MAX_NUM; i++) {
            if (BT_HANDLE_INVALID != (handle = app_le_audio_ucst_get_handle(i))) {
                app_le_audio_ucst_set_receiver_start_ready(handle);
            }
        }
    } else if (0 == memcmp(pChar, "RECEIVER_STOP_READY", 19)) {
        bt_handle_t handle;
        uint8_t i;
        /* AT+LEAUDIO=UNICAST,RECEIVER_STOP_READY */
        for (i = 0; i < APP_LE_AUDIO_UCST_LINK_MAX_NUM; i++) {
            if (BT_HANDLE_INVALID != (handle = app_le_audio_ucst_get_handle(i))) {
                app_le_audio_ucst_set_receiver_stop_ready(handle);
            }
        }
    } else if (0 == memcmp(pChar, "DISABLE", 7)) {
        bt_handle_t handle;
        uint8_t i;
        /* AT+LEAUDIO=UNICAST,DISABLE */
        for (i = 0; i < APP_LE_AUDIO_UCST_LINK_MAX_NUM; i++) {
            if (BT_HANDLE_INVALID != (handle = app_le_audio_ucst_get_handle(i))) {
                app_le_audio_ucst_disable_ase(handle);
            }
        }
    } else {
        return ATCI_RESPONSE_FLAG_APPEND_ERROR;
    }

    return ATCI_RESPONSE_FLAG_APPEND_OK;
#else
    return ATCI_RESPONSE_FLAG_APPEND_ERROR;
#endif
}

static uint32_t bt_app_comm_at_cmd_le_audio_role_hdl(char *pChar)
{
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
    /* AT+LEAUDIO=ROLE,<ACTION> */
    /* <ACTION>: GET */
    if (0 == memcmp(pChar, "GET", 3)) {
        bt_addr_t addr = {0};
        uint16_t addr_5, addr_4, addr_3, addr_2, addr_1, addr_0;

        pChar = strchr(pChar, ',');
        pChar++;

        if (sscanf(pChar, "%2x:%2x:%2x:%2x:%2x:%2x",
                   (unsigned int *)&addr_5, (unsigned int *)&addr_4, (unsigned int *)&addr_3,
                   (unsigned int *)&addr_2, (unsigned int *)&addr_1, (unsigned int *)&addr_0) > 0) {

            LOG_MSGID_I(BT_APP, "[LE_AUDIO][AT] LEA:%2x:%2x:%2x:%2x:%2x:%2x", 6, addr_5, addr_4, addr_3, addr_2, addr_1, addr_0);

            /* Connect device by bd_addr */
            addr.type = BT_ADDR_RANDOM;
            addr.addr[5] = addr_5;
            addr.addr[4] = addr_4;
            addr.addr[3] = addr_3;
            addr.addr[2] = addr_2;
            addr.addr[1] = addr_1;
            addr.addr[0] = addr_0;

            bt_aws_mce_role_t role;
            role = app_le_audio_air_get_role(&addr);

            char conn_string[30] = {0};
            snprintf((char *)conn_string, 30, "ROLE:%02x\r\n", role);
            bt_app_common_at_cmd_print_report(conn_string);

            return ATCI_RESPONSE_FLAG_APPEND_OK;
        }

        return ATCI_RESPONSE_FLAG_APPEND_ERROR;

    }
#endif
    return ATCI_RESPONSE_FLAG_APPEND_ERROR;
}

static uint32_t bt_app_comm_at_cmd_le_audio_cig_hdl(char *pChar)
{
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
    /* AT+LEAUDIO=CIGTEST,<ACTION> */
    /* <ACTION>: ON, OFF */
    if (0 == memcmp(pChar, "OFF", 3)) {
        /* AT+LEAUDIO=CIGTEST,OFF */
        app_le_audio_ucst_reset_cig_parameter_test();

    } else if (0 == memcmp(pChar, "ON", 2)) {
        /* AT+LEAUDIO=CIGTEST,ON,<BN>,<NSE>,<FT>,<ISO-INTERVAL> */
        pChar = strchr(pChar, ',');
        pChar++;
        uint8_t bn = atoi(pChar);
        pChar = strchr(pChar, ',');
        pChar++;
        uint8_t nse = atoi(pChar);
        pChar = strchr(pChar, ',');
        pChar++;
        uint8_t ft = atoi(pChar);
        pChar = strchr(pChar, ',');
        pChar++;
        uint16_t iso_interval = atoi(pChar);
        app_le_audio_ucst_set_cig_parameter_test(bn, nse, ft, iso_interval);

    } else {
        return ATCI_RESPONSE_FLAG_APPEND_ERROR;
    }

    return ATCI_RESPONSE_FLAG_APPEND_OK;
#else
    return ATCI_RESPONSE_FLAG_APPEND_ERROR;
#endif
}

static uint32_t bt_app_comm_at_cmd_le_audio_config_hdl(char *pChar)
{
    /* AT+LEAUDIO=CONFIG,<ACTION> */
    /* <ACTION>: SET */

    if (0 == memcmp(pChar, "SET", 3)) {
        uint8_t port = 0;

        /* AT+LEAUDIO=CONFIG,SET,<MODE>,<sampling_rate>,<sel_setting><high_reliability> */

        pChar = strchr(pChar, ',');
        pChar++;

        if (0 == memcmp(pChar, "SPK_0", 5)) {
            port = 0;

        } else if (0 == memcmp(pChar, "SPK_1", 5)) {
            port = 1;

        } else if (0 == memcmp(pChar, "MIC_0", 5)) {
            port = 2;

        } else if (0 == memcmp(pChar, "BROADCAST", 9)) {
            port = 3;

        } else {
            return ATCI_RESPONSE_FLAG_APPEND_ERROR;
        }

        pChar = strchr(pChar, ',');
        pChar++;

        uint8_t sampling_rate = atoi(pChar);
        pChar = strchr(pChar, ',');
        pChar++;

        uint8_t sel_setting = atoi(pChar);
        pChar = strchr(pChar, ',');
        pChar++;

        uint8_t high_reliability = atoi(pChar);

        if (3 != port) {
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
            app_le_audio_ucst_set_qos_params(sampling_rate, sel_setting, high_reliability, port);
#else
            return ATCI_RESPONSE_FLAG_APPEND_ERROR;
#endif
        } else {
#ifdef AIR_LE_AUDIO_BIS_ENABLE
            app_le_audio_bcst_set_qos_params(sampling_rate, sel_setting, high_reliability);
            app_le_audio_bcst_write_qos_params_nvkey(sampling_rate, sel_setting, high_reliability);
#else
            return ATCI_RESPONSE_FLAG_APPEND_ERROR;
#endif
        }

    } else {
        return ATCI_RESPONSE_FLAG_APPEND_ERROR;
    }

    return ATCI_RESPONSE_FLAG_APPEND_OK;
}


static uint32_t bt_app_comm_at_cmd_le_audio_tmap_hdl(char *pChar)
{
    /* AT+LEAUDIO=TMAP,<ACTION> */
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
    if (0 == memcmp(pChar, "REMOTE", 6)) {
        bt_handle_t handle;
        bt_status_t ret;
        uint8_t i;

        for (i = 0; i < APP_LE_AUDIO_UCST_LINK_MAX_NUM; i++) {
            if (BT_HANDLE_INVALID != (handle = app_le_audio_ucst_get_handle(i))) {
                ret = ble_tmap_read_role_req(handle);
                LOG_MSGID_I(BT_APP, "[APP][TMAP] TMAS REMOTE Role handle:%04x, status:%04x", 2, handle, ret);
            }
        }
        return ATCI_RESPONSE_FLAG_APPEND_OK;
    }
#endif
    if (0 == memcmp(pChar, "LOCAL", 5)) {
        ble_tmap_role_t role;
        role = ble_tmas_get_role();
        LOG_MSGID_I(BT_APP, "[APP][TMAP] TMAS LOCAL Role:0x%04x", 1, role);
        return ATCI_RESPONSE_FLAG_APPEND_OK;
    }

    return ATCI_RESPONSE_FLAG_APPEND_ERROR;
}

#ifdef AIR_LE_AUDIO_HAPC_ENABLE
static uint32_t bt_app_comm_at_cmd_le_audio_hapc_hdl(char *pChar)
{
    /* AT+LEAUDIO=HAPC,<OP>,<ACTION>,<CONN_HANDLE>,<...> */
    atci_response_flag_t rsp = ATCI_RESPONSE_FLAG_APPEND_ERROR;
    bt_handle_t conn_hdl;
    char *op = pChar, *action;
    uint8_t req = 0;

    if (!(pChar = strchr(pChar, ','))) return rsp;

    pChar++;
    action = pChar;

    if (!(pChar = strchr(pChar, ','))) return rsp;

    pChar++;

    if (0 == memcmp(pChar, "0x", 2)) pChar += 2;

    conn_hdl = strtol(pChar, NULL, 16);

    if (!conn_hdl) conn_hdl = bt_app_common_get_first_conneciton_handle();

    if (0 == memcmp(op, "GET", 3))
    {
        if (0 == memcmp(action, "PRESET", 6))
            req = HAPC_REQ_READ_PRESET;
        else if (0 == memcmp(action, "FEATURE", 8))
            req = HAPC_REQ_READ_HA_FEATURE;
        else if (0 == memcmp(action, "ACTIVE", 6))
            req = HAPC_REQ_READ_ACTIVE_INDEX;
    }
    else if (0 == memcmp(op, "SET", 3))
    {
        if (0 == memcmp(action, "NAME", 4))
            req = HAPC_REQ_WRITE_PRESET_NAME;
        else if (0 == memcmp(action, "ACTIVE", 6))
            req = HAPC_REQ_SET_ACTIVE_PRESET;
        else if (0 == memcmp(action, "NEXT", 4))
            req = HAPC_REQ_SET_NEXT_PRESET;
        else if (0 == memcmp(action, "PREVIOUS", 8))
            req = HAPC_REQ_SET_PREVIOUS_PRESET;
        else if (0 == memcmp(action, "ALERT", 5))
            req = 0xFF;
    }

    LOG_MSGID_I(BT_APP, "[AT_CMD] req:%x \r\n", 1, req);

    switch (req)
    {
        case HAPC_REQ_READ_PRESET:
        {
            /* AT+LEAUDIO=HAPC,GET,PRESET,<CONN_HANDLE>,<START_INDEX>,<COUNT> */
            uint8_t start_idx, count;

            if (!(pChar = strchr(pChar, ','))) return rsp;

            pChar++;
            start_idx = strtol(pChar, NULL, 10);

            if (!start_idx) return rsp;

            if (!(pChar = strchr(pChar, ','))) return rsp;

            pChar++;
            count = strtol(pChar, NULL, 10);

            if (!count) return rsp;

            if (BT_STATUS_SUCCESS != ble_hapc_read_preset(conn_hdl, start_idx, count)) return rsp;

            break;
        }
        case HAPC_REQ_READ_HA_FEATURE:
        {
            /* AT+LEAUDIO=HAPC,GET,FEATURE,<CONN_HANDLE> */
            if (BT_STATUS_SUCCESS != ble_hapc_read_ha_feature(conn_hdl)) return rsp;

            break;
        }
        case HAPC_REQ_READ_ACTIVE_INDEX:
        {
            /* AT+LEAUDIO=HAPC,GET,ACTIVE,<CONN_HANDLE> */
            if (BT_STATUS_SUCCESS != ble_hapc_read_active_index(conn_hdl)) return rsp;

            break;
        }
        case HAPC_REQ_WRITE_PRESET_NAME:
        {
            /* AT+LEAUDIO=HAPC,SET,NAME,<CONN_HANDLE>,<INDEX>,<"PRESET_NAME"> */
            char *s, *e;
            uint8_t idx;

            if (!(pChar = strchr(pChar, ','))) return rsp;

            pChar++;
            idx = strtol(pChar, NULL, 10);

            if (!idx) return rsp;

            if (!(pChar = strchr(pChar, ','))) return rsp;

            pChar++;

            if (!(pChar = strchr(pChar, '"'))) return rsp;

            s = pChar++;

            if (!(pChar = strchr(pChar, '"'))) return rsp;

            e = pChar;

            if (BT_STATUS_SUCCESS != ble_hapc_write_preset_name(conn_hdl, idx, e - s - 1, s + 1)) return rsp;

            break;
        }
        case HAPC_REQ_SET_ACTIVE_PRESET:
        {
           /* AT+LEAUDIO=HAPC,SET,ACTIVE,<CONN_HANDLE>,<SYNC>,<INDEX> */
           uint8_t idx, sync;

            if (!(pChar = strchr(pChar, ','))) return rsp;

            pChar++;
            sync = !!strtol(pChar, NULL, 10);

            if (!(pChar = strchr(pChar, ','))) return rsp;

            pChar++;
            idx = strtol(pChar, NULL, 10);

            if (!idx) return rsp;

            if (BT_STATUS_SUCCESS != ble_hapc_set_active_preset(conn_hdl, idx, sync)) return rsp;

            break;
        }
        case HAPC_REQ_SET_NEXT_PRESET:
        {
            /* AT+LEAUDIO=HAPC,SET,NEXT,<CONN_HANDLE>,<SYNC> */
            uint8_t sync;

            if (!(pChar = strchr(pChar, ','))) return rsp;

            pChar++;
            sync = !!strtol(pChar, NULL, 10);

            if (BT_STATUS_SUCCESS != ble_hapc_set_next_preset(conn_hdl, sync)) return rsp;

            break;
        }
        case HAPC_REQ_SET_PREVIOUS_PRESET:
        {
            /* AT+LEAUDIO=HAPC,SET,PREVIOUS,<CONN_HANDLE>,<SYNC> */
            uint8_t sync;

            if (!(pChar = strchr(pChar, ','))) return rsp;

            pChar++;
            sync = !!strtol(pChar, NULL, 10);

            if (BT_STATUS_SUCCESS != ble_hapc_set_previous_preset(conn_hdl, sync)) return rsp;

            break;
        }
        case 0xFF:
        {
            extern bt_status_t ble_iac_set_alert_level_by_hdl(bt_handle_t conn_hdl, uint8_t alert_level, uint16_t alert_att_hdl);

            /* AT+LEAUDIO=HAPC,SET,ALERT,<CONN_HANDLE>,<LEVEL> */
            uint8_t level;
            uint16_t hdl;
            bt_status_t status;

            if (!(pChar = strchr(pChar, ','))) return rsp;

            pChar++;
            level = strtol(pChar, NULL, 10);

            if (!(pChar = strchr(pChar, ','))) return rsp;

            pChar++;
            hdl = strtol(pChar, NULL, 16);

            if (!hdl) return rsp;

            status = ble_iac_set_alert_level_by_hdl(conn_hdl, level, hdl);
            LOG_MSGID_I(BT_APP, "[AT_CMD]IAC set alert status:%x hdl = 0x%x\r\n", 2, status, hdl);

            if (BT_STATUS_SUCCESS != status) return rsp;

            break;
        }
        default:
            return rsp;
    }

    return ATCI_RESPONSE_FLAG_APPEND_OK;
}
#endif

#if defined(AIR_MS_TEAMS_ENABLE) && defined(AIR_LE_AUDIO_UNICAST_ENABLE)
extern bool g_lea_ucst_pts_remote_call_service;
extern bool app_le_audio_usb_hid_handle_incoming_call(void);
extern bool app_le_audio_usb_hid_handle_outgoing_call(void);
extern bool app_le_audio_usb_hid_handle_call_active(void);
extern bool app_le_audio_usb_hid_handle_call_end(void);
extern bool app_le_audio_usb_hid_handle_call_hold(void);
extern bool app_le_audio_usb_hid_handle_call_unhold(void);
extern bool app_le_audio_usb_hid_handle_call_remotely_hold(void);
extern bool app_le_audio_usb_hid_handle_call_remotely_unhold(void);
extern bool app_le_audio_usb_hid_handle_call_alert(void);
extern bt_status_t ble_tbs_set_bearer_provider_name(uint8_t service_idx, uint8_t *bearer_provider_name, uint16_t bearer_provider_name_len);
extern bt_status_t ble_tbs_set_status_flags(uint8_t service_idx, uint16_t status_flags);
extern bt_status_t ble_tbs_set_bearer_technology(uint8_t service_idx, uint8_t bearer_technology);
extern bt_status_t ble_tbs_set_call_state(uint8_t service_idx, ble_tbs_call_index_t call_idx,ble_tbs_state_t call_state);
#endif

static uint32_t bt_app_comm_at_cmd_le_audio_tbs_hdl(char *pChar)
{
    bt_status_t result = BT_STATUS_UNSUPPORTED;

    /* TBS, Simulate the USB HID event */
    /* AT+LEAUDIO=TBS,<ACTION> */
    /* <ACTION>: INCOMING, OUTGOING, ACTIVE, END, HOLD, UNHOLD */

#if defined(AIR_MS_TEAMS_ENABLE) && defined(AIR_LE_AUDIO_UNICAST_ENABLE)
    g_lea_ucst_pts_remote_call_service = true;

    if (0 == memcmp(pChar, "INCOMING", 8)) {
        app_le_audio_usb_hid_handle_incoming_call();
        result = BT_STATUS_SUCCESS;

    } else if (0 == memcmp(pChar, "OUTGOING", 8)) {
        app_le_audio_usb_hid_handle_outgoing_call();
        result = BT_STATUS_SUCCESS;

    } else if (0 == memcmp(pChar, "ACTIVE", 6)) {
        app_le_audio_usb_hid_handle_call_active();
        result = BT_STATUS_SUCCESS;

    } else if (0 == memcmp(pChar, "END", 3)) {
        app_le_audio_usb_hid_handle_call_end();
        result = BT_STATUS_SUCCESS;

    } else if (0 == memcmp(pChar, "HOLD", 4)) {
        app_le_audio_usb_hid_handle_call_hold();
        result = BT_STATUS_SUCCESS;

    } else if (0 == memcmp(pChar, "UNHOLD", 6)) {
        app_le_audio_usb_hid_handle_call_unhold();
        result = BT_STATUS_SUCCESS;

    } else if (0 == memcmp(pChar, "REMOTE_HOLD", 11)) {
        app_le_audio_usb_hid_handle_call_remotely_hold();
        result = BT_STATUS_SUCCESS;

    } else if (0 == memcmp(pChar, "REMOTE_UNHOLD", 13)) {
        app_le_audio_usb_hid_handle_call_remotely_unhold();
        result = BT_STATUS_SUCCESS;

    } else if (0 == memcmp(pChar, "UPDATE_NAME", 11)) {
        char name[] = "PTS_TBS_NAME";

        if (BT_STATUS_SUCCESS == ble_tbs_set_bearer_provider_name(ble_tbs_get_gtbs_service_idx(), (uint8_t *)name, (uint16_t)strlen(name))) {
            result = BT_STATUS_SUCCESS;
        }

    } else if (0 == memcmp(pChar, "UPDATE_STATUS_FLAG", 18)) {
        if (BT_STATUS_SUCCESS == ble_tbs_set_status_flags(ble_tbs_get_gtbs_service_idx(), 0x0001)) {
            result = BT_STATUS_SUCCESS;
        }
    } else if (0 == memcmp(pChar, "UPDATE_BEAR_TECH", 16)) {
        pChar = strchr(pChar, ',');
        pChar++;
        uint8_t bearer_technology = 0;
        sscanf(pChar,"%x", (unsigned int *)&bearer_technology);
        if (BT_STATUS_SUCCESS == ble_tbs_set_bearer_technology(ble_tbs_get_gtbs_service_idx(), bearer_technology)) {
            result = BT_STATUS_SUCCESS;
        }
    } else if (0 == memcmp(pChar, "ALERT", 5)){
        app_le_audio_usb_hid_handle_call_alert();
        result = BT_STATUS_SUCCESS;
    }
#endif

    LOG_MSGID_I(BT_APP, "[AT_CMD] TBS simulate USB HID event, result:%x \r\n", 1, result);

    if (BT_STATUS_SUCCESS == result) {
        return ATCI_RESPONSE_FLAG_APPEND_OK;
    }

    return ATCI_RESPONSE_FLAG_APPEND_ERROR;
}

extern void bt_le_audio_source_service_discovery_register_csis(void);

#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
static uint32_t bt_app_comm_at_cmd_le_audio_gatt_write_hdl(char *pChar)
{
    uint8_t LE_AUDIO_ATT_HDR_SIZE = 3;

    uint16_t att_handle = 0;
    uint16_t data = 0;
    uint8_t *p_buf = NULL;
    //bt_att_write_req_t req;

    if (NULL == (p_buf = (uint8_t *)le_audio_malloc(LE_AUDIO_ATT_HDR_SIZE + sizeof(data)))) {
        return BT_STATUS_OUT_OF_MEMORY;
    }

    sscanf(pChar, "%2x,%2x", (unsigned int *)&att_handle, (unsigned int *)&data);

    LOG_MSGID_I(BT_APP, "[APP][BAP_PTS] Write command att_handle %2x, data %2x", 2, att_handle, data);
    printf("[APP] %s", pChar);

    // req.opcode = BT_ATT_OPCODE_WRITE_REQUEST;
    // req.attribute_handle = att_handle;//test
    // req.attribute_value[0] = data;//test

    bt_handle_t handle = bt_app_common_get_first_conneciton_handle();

    // if (BT_STATUS_SUCCESS == bt_att_send_data(handle, &req, sizeof(req), NULL)) {
    //     return ATCI_RESPONSE_FLAG_APPEND_OK;
    // }
    BT_GATTC_NEW_WRITE_CHARC_REQ(req, p_buf, att_handle, &data, sizeof(data));
    if (BT_STATUS_SUCCESS == bt_gattc_write_charc(handle, &req)) {
        le_audio_free(p_buf);
        return ATCI_RESPONSE_FLAG_APPEND_OK;
    }
    le_audio_free(p_buf);
    return ATCI_RESPONSE_FLAG_APPEND_ERROR;
}
#endif

static uint32_t bt_app_comm_at_cmd_le_audio_gatt_read_hdl(char *pChar)
{
    //bt_att_read_req_t req;
    uint16_t att_handle = 0;

    sscanf(pChar, "%2x", (unsigned int *)&att_handle);

    LOG_MSGID_I(BT_APP, "[APP][BAP_PTS] Read command att_handle %2x", 1, att_handle);

    // req.opcode = BT_ATT_OPCODE_READ_REQUEST;
    // req.attribute_handle = att_handle;

    bt_handle_t handle = bt_app_common_get_first_conneciton_handle();

    BT_GATTC_NEW_READ_CHARC_REQ(req, att_handle);
    if (BT_STATUS_SUCCESS == bt_gattc_read_charc(handle, &req)) {
        return ATCI_RESPONSE_FLAG_APPEND_OK;
    }
    //if (BT_STATUS_SUCCESS == bt_att_send_data(handle, &req, sizeof(req), NULL)) {
    //    return ATCI_RESPONSE_FLAG_APPEND_OK;
    //}

    return ATCI_RESPONSE_FLAG_APPEND_ERROR;
}

static uint32_t bt_app_comm_at_cmd_le_audio_gatt_write_no_resp_hdl(char *pChar)
{
    uint32_t status = ATCI_RESPONSE_FLAG_APPEND_ERROR;
    uint8_t att_handle = 0;
    uint8_t *pBuf = NULL, data = 0;
    uint16_t len = 1;

    sscanf(pChar, "%2x,%2x", (unsigned int *)&att_handle, (unsigned int *)&data);

    LOG_MSGID_I(BT_APP, "[APP][BAP_PTS] Write command att_handle %2x, data %2x", 2, att_handle, data);

    bt_handle_t handle = bt_app_common_get_first_conneciton_handle();

    if (NULL == (pBuf = (uint8_t*)le_audio_malloc(len))) {
        return BT_STATUS_OUT_OF_MEMORY;
    }

    bt_gattc_write_without_rsp_req_t req;
    req.attribute_value_length = len;
    req.att_req = (bt_att_write_command_t *)(pBuf);
    req.att_req->opcode = BT_ATT_OPCODE_WRITE_COMMAND;
    req.att_req->attribute_handle = att_handle;
    memcpy(req.att_req->attribute_value, &data, len);

    status = bt_gattc_write_without_rsp(handle, 0, &req);

    le_audio_free(pBuf);

    return status;
}

static uint32_t bt_app_comm_at_cmd_le_audio_set_codec_hdl(char *pChar)
{
    bt_handle_t connection_handle = bt_app_common_get_first_conneciton_handle();
    uint32_t status = ATCI_RESPONSE_FLAG_APPEND_ERROR;
    uint16_t ase_id = 10;
    sscanf(pChar, "%2x", (unsigned int *)&ase_id);
    LOG_MSGID_I(BT_APP, "[APP][BAP_PTS] ase_id %2x", 1, ase_id);

    ble_bap_ascs_config_codec_operation_t *config = pvPortMalloc(2 + 3 * sizeof(ble_bap_ascs_config_codec_operation_t));

    if(config == NULL) {
        return status;
    }

    uint8_t codec_id[AUDIO_CODEC_ID_SIZE] = CODEC_ID_LC3;
    memset(config, 0, 2 + sizeof(ble_bap_ascs_config_codec_operation_t));
    config->opcode = 0x01;
    config->num_of_ase = 1;

    config->param[0].ase_id = ase_id;
    config->param[0].target_latency = 1;
    config->param[0].target_phy = 2;
    config->param[0].codec_id.coding_format = codec_id[0];
    config->param[0].codec_id.company_id = 0;
    config->param[0].codec_id.vendor_specific_codec_id = 0;
    config->param[0].codec_specific_configuration_length = 19;
    config->param[0].codec_specific_configuration.sampling_freq_len = CODEC_CONFIGURATION_LEN_SAMPLING_FREQUENCY;
    config->param[0].codec_specific_configuration.sampling_freq_type = CODEC_CONFIGURATION_TYPE_SAMPLING_FREQUENCY;
    config->param[0].codec_specific_configuration.sampling_freq_value = CODEC_CONFIGURATION_SAMPLING_FREQ_16KHZ;
    config->param[0].codec_specific_configuration.frame_duration_len = CODEC_CONFIGURATION_LEN_FRAME_DURATIONS;
    config->param[0].codec_specific_configuration.frame_duration_type = CODEC_CONFIGURATION_TYPE_FRAME_DURATIONS;
    config->param[0].codec_specific_configuration.frame_duration_value = FRAME_DURATIONS_10_MS;
    config->param[0].codec_specific_configuration.octets_per_codec_frame_len = CODEC_CONFIGURATION_LEN_OCTETS_PER_CODEC_FRAME;
    config->param[0].codec_specific_configuration.octets_per_codec_frame_type = CODEC_CONFIGURATION_TYPE_OCTETS_PER_CODEC_FRAME;
    config->param[0].codec_specific_configuration.octets_per_codec_frame_value = 40;
    config->param[0].codec_specific_configuration.audio_channel_alloaction_len = CODEC_CONFIGURATION_LEN_AUDIO_CHANNEL_ALLOCATION;
    config->param[0].codec_specific_configuration.audio_channel_alloaction_type = CODEC_CONFIGURATION_TYPE_AUDIO_CHANNEL_ALLOCATION;
    config->param[0].codec_specific_configuration.audio_channel_alloaction_value = AUDIO_LOCATION_FRONT_LEFT;
    config->param[0].codec_specific_configuration.codec_frame_blocks_per_sdu_len = CODEC_CONFIGURATION_LEN_CODEC_FRAME_BLOCKS_PER_SDU,
    config->param[0].codec_specific_configuration.codec_frame_blocks_per_sdu_type = CODEC_CONFIGURATION_TYPE_CODEC_FRAME_BLOCKS_PER_SDU,
    config->param[0].codec_specific_configuration.codec_frame_blocks_per_sdu_value = 1,

    status = ble_bap_ascs_config_codec(connection_handle, config);
    vPortFree(config);
    return status;
}

static uint32_t bt_app_comm_at_cmd_le_audio_set_qos_hdl(char *pChar)
{
    bt_handle_t connection_handle = bt_app_common_get_first_conneciton_handle();
    ble_bap_ascs_config_qos_operation_t *config = NULL;
    uint32_t status = ATCI_RESPONSE_FLAG_APPEND_ERROR;
    config = pvPortMalloc(2 + sizeof(ble_bap_config_qos_param_t));
    if(config == NULL) {
        return status;
    }

    memset(config, 0, 2 + sizeof(ble_bap_config_qos_param_t));

    uint16_t ase_id = 10;
    sscanf(pChar, "%2x", (unsigned int *)&ase_id);
    LOG_MSGID_I(BT_APP, "[APP][BAP_PTS] qos ase_id %2x", 1, ase_id);

    config->opcode = ASE_OPCODE_CONFIG_QOS;
    config->num_of_ase = 1;
    config->param[0].ase_id = ase_id;
    config->param[0].cig_id = 0x01;
    config->param[0].cis_id = 0x03;//CIS_ID_CALL_0;
    config->param[0].sdu_interval = 10000;
    config->param[0].framing = 0;
    config->param[0].phy = 2;
    config->param[0].maximum_sdu_size = 40;
    config->param[0].retransmission_number = 2;
    config->param[0].transport_latency = 95;
    config->param[0].presentation_delay = 0x009C40;

    status = ble_bap_ascs_config_qos(connection_handle, config);
    vPortFree(config);
    return status;
}

static uint32_t bt_app_comm_at_cmd_le_audio_ba_hdl(char *pChar)
{
#ifdef AIR_LE_AUDIO_BA_ENABLE
    /* AT+LEAUDIO=BA,<ACTION> */
    /* <ACTION>: START,SCAN,SYNC,PLAY,PAUSE */
    if (0 == memcmp(pChar, "START", 5)) {
        app_le_audio_start_broadcast_assistant();

    } else if (0 == memcmp(pChar, "SCAN", 4)) {
        app_le_audio_ba_start(APP_LE_AUDIO_BA_COMMANDER_ONLY_MODE);

    } else if (0 == memcmp(pChar, "SYNC", 4)) {
        uint16_t addr_type,addr_5,addr_4,addr_3,addr_2,addr_1,addr_0, advertising_sid;
        pChar = strchr(pChar, ',');
        pChar++;

        if (sscanf(pChar, "%2x,%2x:%2x:%2x:%2x:%2x:%2x, %2x", (unsigned int *)&addr_type, (unsigned int *)&addr_5, (unsigned int *)&addr_4,
                    (unsigned int *)&addr_3, (unsigned int *)&addr_2, (unsigned int *)&addr_1, (unsigned int *)&addr_0, (unsigned int *)&advertising_sid) > 0) {

                bt_addr_t src_addr;
                src_addr.type = addr_type;
                src_addr.addr[5] = addr_5;
                src_addr.addr[4] = addr_4;
                src_addr.addr[3] = addr_3;
                src_addr.addr[2] = addr_2;
                src_addr.addr[1] = addr_1;
                src_addr.addr[0] = addr_0;

                LOG_MSGID_I(common, "Sync BMS addr_type:%x, addr:%x-%x-%x-%x-%x-%x" , 7,
                            addr_type, addr_5, addr_4, addr_3, addr_2, addr_1, addr_0);

                ble_bap_sync_broadcast_source_with_periodic_advertising(src_addr, advertising_sid);
        }

    } else if (0 == memcmp(pChar, "PLAY", 4)) {
        app_le_audio_ba_play_all(1, 1, 0);

    } else if (0 == memcmp(pChar, "PAUSE", 5)) {
        app_le_audio_ba_pause_all(0, 0, 0);

    } else if (0 == memcmp(pChar, "CODE", 4)) {
        /* Commander */
        uint8_t i, bcst_code[BLE_BASS_BROADCAST_CODE_SIZE] = {0};
        uint8_t link_idx = 0, source_id = 0;
        unsigned int value;
        pChar = strchr(pChar, ',');
        pChar++;

        sscanf(pChar, "%2x,", &value);
        link_idx= (uint8_t)value;
        pChar = strchr(pChar, ',');
        pChar++;

        sscanf(pChar, "%2x,", &value);
        source_id = (uint8_t)value;
        pChar = strchr(pChar, ',');
        pChar++;

        /* code */
        for (i = 0; i < BLE_BASS_BROADCAST_CODE_SIZE; i++) {
            sscanf(pChar, "%2x,", &value);
            bcst_code[i]= value;
            pChar += 3;
        }
        app_le_audio_ba_set_broadcast_code(link_idx, source_id, bcst_code);

    } else if (0 == memcmp(pChar, "GETPA", 5)) {
        bt_addr_t addr = app_le_audio_ba_get_pa();
        char conn_string[50] = {0};

        snprintf((char *)conn_string, 50, "Sync PA with %02x:%02x:%02x:%02x:%02x:%02x\r\n", addr.addr[5], addr.addr[4], addr.addr[3], addr.addr[2], addr.addr[1], addr.addr[0]);
        bt_app_common_at_cmd_print_report(conn_string);

    } else if (0 == memcmp(pChar, "AUTO", 4)) {
        unsigned int value;

        pChar = strchr(pChar, ',');
        pChar++;
        sscanf(pChar, "%2x,", &value);

        app_le_audio_ba_set_auto_play((bool)value);

    } else if (0 == memcmp(pChar, "ADD", 3)) {
        /* AT+LEAUDIO=BA,ADD,<is_sync_pa>,<is_sync_bis>,<bis_index> */
        /* <is_sync_pa>    : 0(not sync)       , 1(past avaliable), 2(no past) */
        /* <is_sync_bis>   : 0(no sync bis)    , 1(sync bis) */
        /* <bis_index>     : All link sync same bis index */
        uint32_t bis_index = 0;
        unsigned int value[3];
        uint8_t i;

        for (i = 0; i < 3; i++) {
            pChar = strchr(pChar, ',');
            pChar++;
            sscanf(pChar, "%2x,", &value[i]);
        }

        if (0 != ((uint8_t)value[2])) {
            value[2]--;
            bis_index = 1 << ((uint8_t)value[2]);
        }

        app_le_audio_ba_play_all((uint8_t)value[0], (bool)value[1], bis_index);

    } else if (0 == memcmp(pChar, "MODIFY", 6)) {
        /* AT+LEAUDIO=BA,MODIFY,<is_sync_pa>,<is_sync_bis>,bis_index> */
        /* <is_sync_pa>    : 0(not sync)       , 1(past avaliable), 2(no past) */
        /* <is_sync_bis>   : 0(no sync bis)    , 1(sync bis) */
        /* <bis_index>     : All link sync same bis index */
        uint32_t bis_index = 0;
        unsigned int value[3];
        uint8_t i;

        for (i = 0; i < 3; i++) {
            pChar = strchr(pChar, ',');
            pChar++;
            sscanf(pChar, "%2x,", &value[i]);
        }

        if (0 != ((uint8_t)value[2])) {
            value[2]--;
            bis_index = 1 << ((uint8_t)value[2]);
        }

        app_le_audio_ba_pause_all((uint8_t)value[0], (bool)value[1], bis_index);

    } else {
        return ATCI_RESPONSE_FLAG_APPEND_ERROR;
    }

    return ATCI_RESPONSE_FLAG_APPEND_OK;
#else
    return ATCI_RESPONSE_FLAG_APPEND_ERROR;
#endif
}

extern void bt_le_audio_source_service_discovery_register_csis(void);

static atci_status_t bt_app_comm_at_cmd_le_audio_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_OK};
    char *pChar = NULL;

    if (parse_cmd->mode == ATCI_CMD_MODE_EXECUTION) {
        pChar = (parse_cmd->string_ptr + parse_cmd->name_len + 1);

        if (0 == memcmp(pChar, "SIRK", 4)) {
            /* Set SIRK */
            /* AT+LEAUDIO=SIRK,<ACTION>,<PARAM> */
            /* <ACTION>: SET, GET */
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_audio_sirk_hdl(pChar);

        } else if (0 == memcmp(pChar, "SCAN", 4)) {
            /* Bonding information */
            /* AT+LEAUDIO=SCAN,<ACTION> */
            /* <ACTION>: ON, OFF, RESET */
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_audio_scan_hdl(pChar);

        } else if (0 == memcmp(pChar, "BOND", 4)) {
            /* Bonding information */
            /* AT+LEAUDIO=BOND,<ACTION> */
            /* <ACTION>: RM */
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_audio_bond_hdl(pChar);

        } else if (0 == memcmp(pChar, "CONN", 4)) {
            /* Create connection */
            /* AT+LEAUDIO=CONN,<ACTION>,<PARAM> */
            /* <ACTION>: SIRK, ADDR */
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_audio_conn_hdl(pChar);

        } else if (0 == memcmp(pChar, "ROLE", 4)) {
            /* Unicast */
            /* AT+LEAUDIO=ROLE,<ACTION> */
            /* <ACTION>: GET */
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_audio_role_hdl(pChar);

        } else if (0 == memcmp(pChar, "DISCONN", 7)) {
            /* Disconnection */
            /* AT+LEAUDIO=DISCONN,<ADDR_TYPE>,<B0>:<B1>:<B2>:<B3>:<B4>:<B5> */
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_audio_disconn_hdl(pChar);

        } else if (0 == memcmp(pChar, "DELETE", 6)) {
            /* Disconnection */
            /* AT+LEAUDIO=DELETE,<ADDR_TYPE>,<B0>:<B1>:<B2>:<B3>:<B4>:<B5> */
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_audio_delete_hdl(pChar);

        } else if (0 == memcmp(pChar, "MIC", 3)) {
            /* Bonding information */
            /* AT+LEAUDIO=MIC,<ACTION> */
            /* <ACTION>: MUTE, UNMUTE */
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_audio_mic_hdl(pChar, &response);

        } else if (0 == memcmp(pChar, "CSIP", 4)) {
            /* CSIP */
            /* AT+LEAUDIO=CSIP,<CONN ORDER>,<ACTION> */
            /* <ACTION>: READ_XXX, SET_XXX */
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_audio_csip_hdl(pChar);

        } else if (0 == memcmp(pChar, "PTS_DISCOVER_CSIP", 17)) {
            /* CSIP */
            /* AT+LEAUDIO=PTS_DISCOVER_CSIP */
            bt_le_audio_source_service_discovery_register_csis();
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;

        } else if (0 == memcmp(pChar, "VOLUME", 6)) {
            /* Volume */
            /* AT+LEAUDIO=VOLUME,<ACTION> */
            /* <ACTION>: MUTE, UNMUTE, UP, DOWN, SET */
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_audio_volume_hdl(pChar);

        } else if (0 == memcmp(pChar, "BROADCAST", 9)) {
            /* Broadcast */
            /* AT+LEAUDIO=BROADCAST,<ACTION> */
            /* <ACTION>: START, STOP */
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_audio_broadcast_hdl(pChar);

        } else if (0 == memcmp(pChar, "CIGTEST", 7)) {
            /* CIG test */
            /* AT+LEAUDIO=CIGTEST,<ACTION> */
            /* <ACTION>: ON, OFF */
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_audio_cig_hdl(pChar);

        } else if (0 == memcmp(pChar, "CONFIG", 6)) {
            /* CIG test */
            /* AT+LEAUDIO=CONFIG,<ACTION> */
            /* <ACTION>: SET, RESET */
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_audio_config_hdl(pChar);

        } else if (0 == memcmp(pChar, "UNICAST", 7)) {
            /* Unicast */
            /* AT+LEAUDIO=UNICAST,<ACTION> */
            /* <ACTION>: START, BI */
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_audio_unicast_hdl(pChar);
        } else if (0 == memcmp(pChar, "BA", 2)) {
            /* BA */
            /* AT+LEAUDIO=BA,<ACTION> */
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_audio_ba_hdl(pChar);

        }
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
        else if (0 == memcmp(pChar, "UPDATE", 6)) {
            /* LE audio dongle test, will remove this at cmd after test complete. */
            /* AT+LEAUDIO=CONN_UPDATE,handle,interval,latency,supervision_timeout,ce.*/
            printf("[APP][COMMON] set LE connection update interval1\r\n");
            bt_hci_cmd_le_connection_update_t new_param;
            if (sscanf(pChar + strlen("UPDATE,"), "%x,%x,%x,%x,%x",
                       (unsigned int *)&new_param.connection_handle,
                       (unsigned int *)&new_param.conn_interval_min,
                       (unsigned int *)&new_param.conn_latency,
                       (unsigned int *)&new_param.supervision_timeout,
                       (unsigned int *)&new_param.minimum_ce_length) > 0) {
                new_param.conn_interval_max = new_param.conn_interval_min;
                new_param.maximum_ce_length = new_param.minimum_ce_length;
                bt_status_t status = bt_gap_le_update_connection_parameter((const bt_hci_cmd_le_connection_update_t *)(&new_param));
                LOG_MSGID_I(BT_APP, "[APP][COMMON] set LE connection update interval parameter handle = %02x, interval = %02x, latency = %02x,\
                            supervision_timeout = %02x, ce_length = %02x , status = %02x", 6, new_param.connection_handle, new_param.conn_interval_min, \
                            new_param.conn_latency, new_param.supervision_timeout, new_param.minimum_ce_length, status);
                if (status == BT_STATUS_SUCCESS) {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                } else {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                }
            } else {
                response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                LOG_MSGID_I(BT_APP, "[APP][COMMON] set LE connection update interval parameter fail", 0);
            }
        }
#endif
#ifdef AIR_LE_AUDIO_HAPC_ENABLE
        else if (0 == memcmp(pChar, "HAPC", 4)) {
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_audio_hapc_hdl(pChar);
        }
#endif
        else if (0 == memcmp(pChar, "TMAP", 4)) {
            /* TMAP */
            /* AT+LEAUDIO=TMAP,<ACTION> */
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_audio_tmap_hdl(pChar);
        } else if (0 == memcmp(pChar, "PTS_DISCOVER_CSIP", 17)) {
            /* CSIP */
            /* AT+LEAUDIO=PTS_DISCOVER_CSIP */
            bt_le_audio_source_service_discovery_register_csis();
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
        } else if (0 == memcmp(pChar, "CALL", 4)) {
            /* Call */
            /* AT+LEAUDIO=CALL,<ACTION> */
            /* <ACTION>: ACCEPT, TERMINATE, PLACE */
            pChar = strchr(pChar, ',');
            pChar++;
            #ifdef AIR_LE_AUDIO_UNICAST_ENABLE
            response.response_flag = bt_app_comm_at_cmd_le_audio_call_hdl(pChar);
            #else
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            #endif
        } else if (0 == memcmp(pChar, "MODE", 4)) {
            /* Set Device Mode */
            /* AT+LEAUDIO=MODE,<Device_Mode>,<ADDR_TYPE>,<B0>:<B1>:<B2>:<B3>:<B4>:<B5> */
            pChar = strchr(pChar, ',');
            pChar++;
            #ifdef AIR_LE_AUDIO_UNICAST_ENABLE
            response.response_flag = bt_app_comm_at_cmd_le_audio_mode_hdl(pChar);
            #else
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            #endif
        } else if (0 == memcmp(pChar, "WRITE", 5)) {
            /* AT+LEAUDIO=WRITE,<ATT_HANDLE>,<DATA> */
            /* <ATT_HANDLE>: 1 Byte hex*/
            /* <DATA>: 1 Byte hex*/
            /* Example: AT+LEAUDIO=WRITE,01,FF*/
            /* Example: AT+LEAUDIO=WRITE,0A,FF*/
            LOG_MSGID_I(BT_APP, "[APP][PTS_BAP] PTS test Write Gatt", 0);
            pChar = strchr(pChar, ',');
            pChar++;
            #ifdef AIR_LE_AUDIO_UNICAST_ENABLE
            response.response_flag = bt_app_comm_at_cmd_le_audio_gatt_write_hdl(pChar);
            #else
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            #endif
        } else if (0 == memcmp(pChar, "READ", 4)) {
            /* AT+LEAUDIO=READ,<ATT_HANDLE>*/
            /* <ATT_HANDLE>: 1 Byte hex*/
            /* Example: AT+LEAUDIO=READ,01*/
            /* Example: AT+LEAUDIO=READ,0A*/
            LOG_MSGID_I(BT_APP, "[APP][PTS_BAP] PTS test Read Gatt", 0);
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_audio_gatt_read_hdl(pChar);
        } else if (0 == memcmp(pChar, "W_NORSP", 7)) {
            /* AT+LEAUDIO=WRITECMD,<ATT_HANDLE>,<DATA>*/
            /* <ATT_HANDLE>: 1 Byte hex*/
            /* <DATA>: 1 Byte hex*/
            /* Example: AT+LEAUDIO=WRITECMD,01,FF*/
            /* Example: AT+LEAUDIO=WRITECMD,0A,FF*/
            LOG_MSGID_I(BT_APP, "[APP][PTS_BAP] PTS test Write No Resp Gatt", 0);
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_audio_gatt_write_no_resp_hdl(pChar);
        } else if (0 == memcmp(pChar, "CODEC", 5)) {
            /* AT+LEAUDIO=CODEC,<ASE_ID>*/
            /* <ASE_ID>: 1 Byte hex*/
            /* Example: AT+LEAUDIO=CODEC,01*/
            /* Example: AT+LEAUDIO=CODEC,A1*/
            LOG_MSGID_I(BT_APP, "[APP][PTS] PTS codec", 0);
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_audio_set_codec_hdl(pChar);
        } else if (0 == memcmp(pChar, "QOS", 3)) {
            /* AT+LEAUDIO=QOS,<ASE_ID>*/
            /* <ASE_ID>: 1 Byte hex*/
            /* Example: AT+LEAUDIO=QOS,01*/
            /* Example: AT+LEAUDIO=QOS,A1*/
            LOG_MSGID_I(BT_APP, "[APP][PTS] PTS qos", 0);
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_audio_set_qos_hdl(pChar);
        } else if (0 == memcmp(pChar, "SET_MEDIA_STATE", 15)) {
            /* AT+LEAUDIO=SET_MEDIA_STATE,<state>*/
            pChar = strchr(pChar, ',');
            pChar++;
            int state = 0;
            sscanf(pChar, "%d", &state);
            ble_mcs_set_media_state(0, state);
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
        } else if (0 == memcmp(pChar, "SET_TRACK_DURATION", 18)) {
            /* AT+LEAUDIO=SET_TRACK_DURATION,<DURATION>*/
            pChar = strchr(pChar, ',');
            pChar++;
            int duration = 0;
            sscanf(pChar, "%d", &duration);
            ble_mcs_set_track_duration(0, duration);
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
        } else if (0 == memcmp(pChar, "SET_TRACK_POSITION", 18)) {
            /* AT+LEAUDIO=SET_TRACK_POSITION,<POSITION>*/
            pChar = strchr(pChar, ',');
            pChar++;
            int duration = 0;
            sscanf(pChar, "%d", &duration);
            ble_mcs_set_track_position(0, duration);
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
        } else if (0 == memcmp(pChar, "SET_CALL_TERMINATE", 18)) {
            pChar = strchr(pChar, ',');
            pChar++;
            unsigned int service_idx;
            unsigned int call_idx;
            unsigned int reason;
            sscanf(pChar, "%2x,%2x,%2x", &service_idx, &call_idx, &reason);
            if (ble_tbs_terminate_call(service_idx, call_idx, reason) == BT_STATUS_SUCCESS) {
                response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            }

        } else if (0 == memcmp(pChar, "TBS", 3)) {
            /* TBS */
            /* AT+LEAUDIO=TBS,<ACTION> */
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_audio_tbs_hdl(pChar);

        } else if (0 == memcmp(pChar, "GET_FIRST_HANDLE", 16)) {
            /* AT+LEAUDIO=GET_FIRST_HANDLE */
            char str[50];

            snprintf((char *)str, 50, "get first connection handle:0x%04x\r\n", bt_app_common_get_first_conneciton_handle());
            bt_app_common_at_cmd_print_report(str);
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;

        } else {
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
        }
    }

    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}

#endif  /* AIR_LE_AUDIO_ENABLE */

static atci_status_t bt_app_comm_at_cmd_hfp_codec_set_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    char *token = strtok(parse_cmd->string_ptr, ",");
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_AUTO_APPEND_LF_CR};

    if (strstr(token, "set") != NULL) {
        token = strtok(NULL, ",");

        response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
        if (strstr(token, "CVSD") != NULL) {
            update_hfp_audio_codec(BT_HFP_CODEC_TYPE_CVSD);
        } else if (strstr(token, "MSBC") != NULL) {
            update_hfp_audio_codec(BT_HFP_CODEC_TYPE_MSBC);
        } else {
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            char *pstr = (char *)response.response_buf;
            snprintf(pstr, ATCI_UART_TX_FIFO_BUFFER_SIZE, "unknown type, ");
        }

        response.response_len = strlen((char *)response.response_buf);
    } else {
        bt_hfp_audio_codec_type_t type = get_hfp_audio_codec();
        char *pstr = (char *)response.response_buf;
        snprintf(pstr, ATCI_UART_TX_FIFO_BUFFER_SIZE,
                 "codec mask: 0x%x", type);

        response.response_flag = ATCI_RESPONSE_FLAG_AUTO_APPEND_LF_CR;
        response.response_len = strlen((char *)response.response_buf);
    }
    atci_send_response(&response);
    return ATCI_STATUS_OK;

}


void bt_app_comm_at_cmd_init(void)
{
#if defined(MTK_AWS_MCE_ENABLE) && defined(SUPPORT_ROLE_HANDOVER_SERVICE)
    bt_role_handover_callbacks_t callbacks = {0};
    callbacks.status_cb = bt_app_common_at_cmd_rho_srv_status_callback;
    bt_role_handover_register_callbacks(BT_ROLE_HANDOVER_MODULE_BT_CMD, &callbacks);
#endif

    atci_register_handler(bt_app_comm_at_cmd, sizeof(bt_app_comm_at_cmd) / sizeof(atci_cmd_hdlr_item_t));
}

static void app_at_cmd_mutex_create(void)
{
    if (g_at_cmd_mutex_handle == NULL) {
        vSemaphoreCreateBinary(g_at_cmd_mutex_handle);
    }
}

static void app_at_cmd_mutex_lock(void)
{
    if (g_at_cmd_mutex_handle != NULL) {
        xSemaphoreTake(g_at_cmd_mutex_handle, portMAX_DELAY);
    }
}

static void app_at_cmd_mutex_unlock(void)
{
    if (g_at_cmd_mutex_handle != NULL) {
        /* In new FreeRTOS version, it would assert if mutex is taken and given in different tasks. */
        xSemaphoreGive(g_at_cmd_mutex_handle);
    }
}

static atci_status_t bt_app_comm_at_cmd_swtich_dongle_mode_hdl(atci_parse_cmd_param_t *parse_cmd)
{

    LOG_MSGID_I(BT_APP, "[LE_AUDIO][AT] DONGLE_MODE", 0);
    /* AT+DONGLE_MODE=<ACTION> */
    /* <ACTION>: ULLV2, BROADCAST */

    bt_status_t result;
    atci_response_t *response = pvPortMalloc(sizeof(atci_response_t));
    if (NULL == response) {
        assert(0);
        return ATCI_STATUS_ERROR;
    }
    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
    app_dongle_cm_dongle_mode_t dongle_mode;
    app_at_cmd_mutex_create();
     //get next mode
    if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "ULLV2", 5)) {
        /* AT+DONGLE_MODE=ULLV2 */
        dongle_mode = APP_DONGLE_CM_DONGLE_MODE_ULL_V2;
    } else if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "BROADCAST", 9)) {
        /* AT+DONGLE_MODE=BROADCAST */
        dongle_mode = APP_DONGLE_CM_DONGLE_MODE_LEA_BIS;
    } else if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "UNICAST", 7)) {
        dongle_mode = APP_DONGLE_CM_DONGLE_MODE_LEA_CIS;
    } else if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "ULLV1", 5)) {
        dongle_mode = APP_DONGLE_CM_DONGLE_MODE_ULL_V1;
    } else {
        dongle_mode = 0x00;
    }
    app_at_cmd_mutex_lock();
    bt_utils_mutex_lock();
    result = app_dongle_cm_switch_dongle_mode(dongle_mode);
    bt_utils_mutex_unlock();
    app_at_cmd_mutex_unlock();
    if (!result) {
        response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
    }
    response->response_len = strlen((char *)response->response_buf);
    atci_send_response(response);
    vPortFree(response);
    return ATCI_STATUS_OK;
}

#ifdef AIR_BT_SOURCE_ENABLE
extern uint16_t g_ag_bqb_feature_mask;
static atci_status_t bt_app_comm_at_cmd_dongle_bqb_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t *response = pvPortMalloc(sizeof(atci_response_t));
    if (NULL == response) {
        assert(0);
        return ATCI_STATUS_ERROR;
    }
    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;

    if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "ag_feature_mask", strlen("ag_feature_mask"))) {
        g_ag_bqb_feature_mask = (uint16_t)strtoul(parse_cmd->string_ptr + parse_cmd->name_len + 1 + strlen("ag_feature_mask,"), NULL, 16);
        response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
    }

    response->response_len = strlen((char *)response->response_buf);
    atci_send_response(response);
    vPortFree(response);
    return ATCI_STATUS_OK;
}
#endif

extern bt_status_t bt_device_manager_le_get_bonding_info_by_link_type(bt_gap_le_srv_link_t link_type, bt_device_manager_le_bonded_info_t *infos, uint8_t *count);
static atci_status_t bt_app_comm_at_cmd_le_bond_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t *response = pvPortMalloc(sizeof(atci_response_t));
    if (NULL == response) {
        assert(0);
        return ATCI_STATUS_ERROR;
    }
    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;

    if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "GET LEA", strlen("GET LEA"))) {
        bt_device_manager_le_bonded_info_t bond_info[8] = {0};
        uint32_t bond_info_count = 8;
        if (bt_device_manager_le_get_bonding_info_by_link_type(BT_GAP_LE_SRV_LINK_TYPE_LE_AUDIO, bond_info, (uint8_t *)&bond_info_count) == BT_STATUS_SUCCESS) {
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            uint32_t response_offset = 0;
            uint8_t *p_remote_address = NULL;
            uint8_t *p_ltk = NULL;
            bt_bd_addr_t default_address = {0};
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            for (uint32_t i = 0; i < bond_info_count; i++) {
                if (memcmp(&bond_info[i].info.identity_addr.address.addr, default_address, sizeof(bt_bd_addr_t)) == 0) {
                    p_remote_address = (uint8_t *)&bond_info[i].bt_addr.addr;
                } else {
                    p_remote_address = (uint8_t *)&bond_info[i].info.identity_addr.address.addr;
                }
                p_ltk = (uint8_t *)&bond_info[i].info.local_key.encryption_info.ltk;
                response_offset += snprintf((char *)response->response_buf + response_offset, sizeof(response->response_buf) - response_offset, "address %02x:%02x:%02x:%02x:%02x:%02x\r\n",\
                                        p_remote_address[0], p_remote_address[1], p_remote_address[2], p_remote_address[3], p_remote_address[4], p_remote_address[5]);
                if (response_offset >= ATCI_UART_TX_FIFO_BUFFER_SIZE) {
                    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                    break;
                }

                response_offset += snprintf((char *)response->response_buf + response_offset, sizeof(response->response_buf) - response_offset, "LTK %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\r\n",\
                                        p_ltk[0], p_ltk[1], p_ltk[2], p_ltk[3], p_ltk[4], p_ltk[5], p_ltk[6], p_ltk[7], p_ltk[8], p_ltk[9], p_ltk[10], p_ltk[11],\
                                        p_ltk[12], p_ltk[13], p_ltk[14], p_ltk[15]);
                if (response_offset >= ATCI_UART_TX_FIFO_BUFFER_SIZE) {
                    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                    break;
                }
            }
        }
    }

    response->response_len = strlen((char *)response->response_buf);
    atci_send_response(response);
    vPortFree(response);
    return ATCI_STATUS_OK;
}

void app_dongle_cm_notify_switch_dongle_mode_result(bt_status_t status, app_dongle_cm_dongle_mode_t dongle_mode)
{
    atci_response_heavy_data_t response = {NULL, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};
    response.response_flag = (BT_STATUS_SUCCESS == status) ? ATCI_RESPONSE_FLAG_APPEND_OK : ATCI_RESPONSE_FLAG_APPEND_ERROR;

    switch (dongle_mode) {
        case APP_DONGLE_CM_DONGLE_MODE_ULL_V1: {
            response.response_len = sizeof("SWITCH MODE: ULL V1 ");
            response.response_buf = pvPortMalloc(response.response_len);
            if (!response.response_buf) {
                LOG_MSGID_I(BT_APP, "[APP_DONGLE_CM][AT] OOM!", 0);
                return;
            }
            snprintf((char *)response.response_buf, response.response_len, "SWITCH MODE: ULL V1 ");
        } break;

        case APP_DONGLE_CM_DONGLE_MODE_ULL_V2: {
            response.response_len = sizeof("SWITCH MODE: ULL V2 ");
            response.response_buf = pvPortMalloc(response.response_len);
            if (!response.response_buf) {
                LOG_MSGID_I(BT_APP, "[APP_DONGLE_CM][AT] OOM!", 0);
                return;
            }

            snprintf((char *)response.response_buf, response.response_len, "SWITCH MODE: ULL V2 ");
        } break;

        case APP_DONGLE_CM_DONGLE_MODE_LEA_CIS: {
            response.response_len = sizeof("SWITCH MODE: LEA UNICAST ");
            response.response_buf = pvPortMalloc(response.response_len);
            if (!response.response_buf) {
                LOG_MSGID_I(BT_APP, "[APP_DONGLE_CM][AT] OOM!", 0);
                return;
            }
            snprintf((char *)response.response_buf, response.response_len, "SWITCH MODE: LEA UNICAST ");
        } break;

        case APP_DONGLE_CM_DONGLE_MODE_LEA_BIS: {
            response.response_len = sizeof("SWITCH MODE: LEA BROCAST ");
            response.response_buf = pvPortMalloc(response.response_len);
            if (!response.response_buf) {
                LOG_MSGID_I(BT_APP, "[APP_DONGLE_CM][AT] OOM!", 0);
                return;
            }

            snprintf((char *)response.response_buf, response.response_len, "SWITCH MODE: LEA BROCAST ");
        } break;

        case APP_DONGLE_CM_DONGLE_MODE_BTA: {
            response.response_len = sizeof("SWITCH MODE: BTA ");
            response.response_buf = pvPortMalloc(response.response_len);
            if (!response.response_buf) {
                LOG_MSGID_I(BT_APP, "[APP_DONGLE_CM][AT] OOM!", 0);
                return;
            }

            snprintf((char *)response.response_buf, response.response_len, "SWITCH MODE: BTA ");

        } break;

        default:
            break;
    }
    atci_send_heavy_response(&response);
    if (response.response_buf) {
        vPortFree(response.response_buf);
    }
}


#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
static uint32_t bt_app_comm_at_cmd_le_ull_check_hdl(char *pChar)
{
    /* AT+LEULL=CHECK,<ACTION> */
    /* <ACTION>: STREAMING */
    if (0 == memcmp(pChar, "STREAMING", 9)) {
        app_ull_dongle_le_is_streaming();
        return ATCI_RESPONSE_FLAG_APPEND_OK;
    } else if (0 == memcmp(pChar, "CONNECTION", 10)) {
        app_ull_dongle_le_is_connected();
        return ATCI_RESPONSE_FLAG_APPEND_OK;
    } else if (0 == memcmp(pChar, "UUID", 4)) {
#ifdef APPS_ULL_V2_128_BIT_UUID
        const uint8_t uuid[16] = APPS_ULL_V2_128_BIT_UUID;
#else
        const bt_ull_le_uuid_t *uuid = bt_ull_le_srv_get_uuid();
#endif
        LOG_MSGID_I(BT_APP, "[LE_ULL][AT] UUID: %x-%x-%x-%x-%x-%x-%x-%x", 8, uuid[0], uuid[2], uuid[4], uuid[6], uuid[8], uuid[10], uuid[12], uuid[15]);
        return ATCI_RESPONSE_FLAG_APPEND_OK;
    } else if (0 == memcmp(pChar, "RSI", 3)) {
        bt_ull_le_rsi_t rsi;
        bt_ull_le_srv_get_rsi(rsi);
        LOG_MSGID_I(BT_APP, "[LE_ULL][AT] RSI: %x-%x-%x-%x-%x-%x", 6, rsi[0], rsi[1], rsi[2], rsi[3], rsi[4], rsi[5]);
        return ATCI_RESPONSE_FLAG_APPEND_OK;
    } else {
        return ATCI_RESPONSE_FLAG_APPEND_ERROR;
    }

    return ATCI_RESPONSE_FLAG_APPEND_OK;
}

static uint32_t bt_app_comm_at_cmd_le_ull_control_hdl(char *pChar)
{
    /* AT+LEULL=CONTROL,<ACTION> */
    /* <ACTION>: LOCK */
    if (0 == memcmp(pChar, "LOCK", 4)) {
        bt_ull_le_srv_lock_streaming(true);
        return ATCI_RESPONSE_FLAG_APPEND_OK;
    } else if (0 == memcmp(pChar, "UNLOCK", 6)) {
        bt_ull_le_srv_lock_streaming(false);
        return ATCI_RESPONSE_FLAG_APPEND_OK;
    } else {
        return ATCI_RESPONSE_FLAG_APPEND_ERROR;
    }
}

static uint32_t bt_app_comm_at_cmd_le_ull_scan_hdl(char *pChar)
{
    /* Bonding information */
    /* AT+LEULL=SCAN,<ACTION> */
    /* <ACTION>: ON, OFF */

    if (0 == memcmp(pChar, "ON", 2)) {
        /* start scan devices */
        app_ull_dongle_le_start_scan();
        return ATCI_RESPONSE_FLAG_APPEND_OK;

    } else if (0 == memcmp(pChar, "OFF", 3)) {
        /* start scan devices */
        app_ull_dongle_le_stop_scan();
        return ATCI_RESPONSE_FLAG_APPEND_OK;

    } else if (0 == memcmp(pChar, "CS", 2)) {
        /* find devices */
        app_ull_dongle_le_scan_device(APP_ULL_DONGLE_LE_SCAN_COORDINATED_SET_BY_SIRK);
        return ATCI_RESPONSE_FLAG_APPEND_OK;
    }

    return ATCI_RESPONSE_FLAG_APPEND_ERROR;
}

static uint32_t bt_app_comm_at_cmd_le_ull_bond_hdl(char *pChar)
{
    /* Bonding information */
    /* AT+LEULL=BOND,<ACTION> */
    /* <ACTION>: RM */

    if (0 == memcmp(pChar, "RM", 2)) {
        /* Remove bond */
        app_ull_dongle_le_reset_bonded_info();
        return ATCI_RESPONSE_FLAG_APPEND_OK;
    }

    return ATCI_RESPONSE_FLAG_APPEND_ERROR;
}

extern bt_key_t *bt_ull_le_srv_get_sirk(void);
extern bt_status_t bt_ull_le_srv_set_sirk(bt_key_t sirk);

static uint32_t bt_app_comm_at_cmd_le_ull_sirk_hdl(char *pChar)
{
    /* SIRK */
    if (0 == memcmp(pChar, "SET", 3)) {
        bt_key_t sirk = {0};

        /* Set SIRK */
        /* AT+LEULL=SIRK,SET,<B0>,<B1>,<B2>,<B3>,<B4>,<B5>,<B6>,<B7>,<B8>,<B9>,<B10>,<B11>,<B12>,<B13>,<B14>,<B15> */
        pChar = strchr(pChar, ',');
        pChar++;

        if (sscanf(pChar, "%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x",
                   (unsigned int *)&sirk[0], (unsigned int *)&sirk[1], (unsigned int *)&sirk[2], (unsigned int *)&sirk[3],
                   (unsigned int *)&sirk[4], (unsigned int *)&sirk[5], (unsigned int *)&sirk[6], (unsigned int *)&sirk[7],
                   (unsigned int *)&sirk[8], (unsigned int *)&sirk[9], (unsigned int *)&sirk[10], (unsigned int *)&sirk[11],
                   (unsigned int *)&sirk[12], (unsigned int *)&sirk[13], (unsigned int *)&sirk[14], (unsigned int *)&sirk[15]) > 0) {

            LOG_MSGID_I(BT_APP, "[LE_ULL][AT] SET SIRK: %x-%x-%x-%x-%x-%x-%x-%x", 8, sirk[0], sirk[1], sirk[2], sirk[3], sirk[4], sirk[5], sirk[6], sirk[7]);
            LOG_MSGID_I(BT_APP, "[LE_ULL][AT] SET SIRK: %x-%x-%x-%x-%x-%x-%x-%x", 8, sirk[8], sirk[9], sirk[10], sirk[11], sirk[12], sirk[13], sirk[14], sirk[15]);

            /* Connect coordinated set by SIRK */
            app_ull_dongle_le_set_sirk(&sirk, true);
            return ATCI_RESPONSE_FLAG_APPEND_OK;
        }
    } else if (0 == memcmp(pChar, "GET", 3)) {
        bt_key_t *p_sirk, sirk;

        if (NULL != (p_sirk = app_ull_dongle_le_get_sirk(true))) {

            memcpy(&sirk, p_sirk, sizeof(bt_key_t));
            LOG_MSGID_I(BT_APP, "[LE_ULL][AT] GET SIRK: %x-%x-%x-%x-%x-%x-%x-%x", 8, sirk[0], sirk[1], sirk[2], sirk[3], sirk[4], sirk[5], sirk[6], sirk[7]);
            LOG_MSGID_I(BT_APP, "[LE_ULL][AT] GET SIRK: %x-%x-%x-%x-%x-%x-%x-%x", 8, sirk[8], sirk[9], sirk[10], sirk[11], sirk[12], sirk[13], sirk[14], sirk[15]);
            bt_ull_le_srv_set_sirk(sirk);
            return ATCI_RESPONSE_FLAG_APPEND_OK;
        }
        LOG_MSGID_I(BT_APP, "[LE_ULL][AT] GET SIRK fail", 0);
    }

    return ATCI_RESPONSE_FLAG_APPEND_ERROR;
}

static uint32_t bt_app_comm_at_cmd_le_ull_conn_by_sirk_hdl(char *pChar)
{
    bt_key_t sirk = {0};

    /* AT+LEULL=CONN,SIRK,<B0>,<B1>,<B2>,<B3>,<B4>,<B5>,<B6>,<B7>,<B8>,<B9>,<B10>,<B11>,<B12>,<B13>,<B14>,<B15> */

    if (sscanf(pChar, "%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x,%2x",
               (unsigned int *)&sirk[0], (unsigned int *)&sirk[1], (unsigned int *)&sirk[2], (unsigned int *)&sirk[3],
               (unsigned int *)&sirk[4], (unsigned int *)&sirk[5], (unsigned int *)&sirk[6], (unsigned int *)&sirk[7],
               (unsigned int *)&sirk[8], (unsigned int *)&sirk[9], (unsigned int *)&sirk[10], (unsigned int *)&sirk[11],
               (unsigned int *)&sirk[12], (unsigned int *)&sirk[13], (unsigned int *)&sirk[14], (unsigned int *)&sirk[15]) > 0) {

        LOG_MSGID_I(BT_APP, "[LE_ULL][AT] SIRK: %x-%x-%x-%x-%x-%x-%x-%x", 8, sirk[0], sirk[1], sirk[2], sirk[3], sirk[4], sirk[5], sirk[6], sirk[7]);
        LOG_MSGID_I(BT_APP, "[LE_ULL][AT] SIRK: %x-%x-%x-%x-%x-%x-%x-%x", 8, sirk[8], sirk[9], sirk[10], sirk[11], sirk[12], sirk[13], sirk[14], sirk[15]);

        /* Connect coordinated set by SIRK */
        bt_ull_le_srv_set_sirk(sirk);
        app_ull_dongle_le_scan_device(APP_ULL_DONGLE_LE_SCAN_COORDINATED_SET_BY_SIRK);
        return ATCI_RESPONSE_FLAG_APPEND_OK;
    }

    return ATCI_RESPONSE_FLAG_APPEND_ERROR;
}

static uint32_t bt_app_comm_at_cmd_le_ull_conn_addr_hdl(char *pChar)
{
    bt_addr_t addr = {0};
    uint16_t addr_5, addr_4, addr_3, addr_2, addr_1, addr_0;

    /* AT+LEULL=CONN,ADDR,<B0>:<B1>:<B2>:<B3>:<B4>:<B5> */

    if (sscanf(pChar, "%2x:%2x:%2x:%2x:%2x:%2x",
               (unsigned int *)&addr_5, (unsigned int *)&addr_4, (unsigned int *)&addr_3,
               (unsigned int *)&addr_2, (unsigned int *)&addr_1, (unsigned int *)&addr_0) > 0) {

        LOG_MSGID_I(BT_APP, "[LE_ULL][AT] C LEA:%2x:%2x:%2x:%2x:%2x:%2x", 6, addr_5, addr_4, addr_3, addr_2, addr_1, addr_0);

        /* Connect device by bd_addr */
        addr.type = BT_ADDR_RANDOM;
        addr.addr[5] = addr_5;
        addr.addr[4] = addr_4;
        addr.addr[3] = addr_3;
        addr.addr[2] = addr_2;
        addr.addr[1] = addr_1;
        addr.addr[0] = addr_0;
        app_ull_dongle_le_connect(&addr);
        return ATCI_RESPONSE_FLAG_APPEND_OK;
    }

    return ATCI_RESPONSE_FLAG_APPEND_ERROR;
}

static uint32_t bt_app_comm_at_cmd_le_ull_conn_hdl(char *pChar)
{
    /* Create connection */
    /* AT+LEULL=CONN,<ACTION>,<PARAM> */
    /* <ACTION>: SIRK, ADDR */

    if (0 == memcmp(pChar, "SIRK", 4)) {
        /* Connect coordinated set by SIRK */
        /* AT+LEULL=CONN,SIRK,<B0>,<B1>,<B2>,<B3>,<B4>,<B5>,<B6>,<B7>,<B8>,<B9>,<B10>,<B11>,<B12>,<B13>,<B14>,<B15> */
        pChar = strchr(pChar, ',');
        pChar++;
        return bt_app_comm_at_cmd_le_ull_conn_by_sirk_hdl(pChar);

    } else if (0 == memcmp(pChar, "ADDR", 4)) {
        /* Connect device by bd_addr */
        /* AT+LEULL=CONN,ADDR,<B0>:<B1>:<B2>:<B3>:<B4>:<B5> */
        pChar = strchr(pChar, ',');
        pChar++;
        return bt_app_comm_at_cmd_le_ull_conn_addr_hdl(pChar);
    }

    return ATCI_RESPONSE_FLAG_APPEND_ERROR;
}

static uint32_t bt_app_comm_at_cmd_le_ull_disconn_hdl(char *pChar)
{
    /* Disconnection */
    /* AT+LEULL=DISCONN,<ACTION>,<PARAM> */
    /* <ACTION>: ADDR */

    if (0 == memcmp(pChar, "ADDR", 4)) {
        bt_addr_t addr = {0};
        uint16_t addr_5, addr_4, addr_3, addr_2, addr_1, addr_0;

        /* Connect device by bd_addr */
        /* AT+LEULL=DISCONN,ADDR,<B0>:<B1>:<B2>:<B3>:<B4>:<B5> */
        pChar = strchr(pChar, ',');
        pChar++;

        if (sscanf(pChar, "%2x:%2x:%2x:%2x:%2x:%2x",
                   (unsigned int *)&addr_5, (unsigned int *)&addr_4, (unsigned int *)&addr_3,
                   (unsigned int *)&addr_2, (unsigned int *)&addr_1, (unsigned int *)&addr_0) > 0) {

            LOG_MSGID_I(BT_APP, "[LE_ULL][AT] Disconn LEA:%2x:%2x:%2x:%2x:%2x:%2x", 6, addr_5, addr_4, addr_3, addr_2, addr_1, addr_0);

            /* Disconnect device by bd_addr */
            addr.type = BT_ADDR_RANDOM;
            addr.addr[5] = addr_5;
            addr.addr[4] = addr_4;
            addr.addr[3] = addr_3;
            addr.addr[2] = addr_2;
            addr.addr[1] = addr_1;
            addr.addr[0] = addr_0;
            app_ull_dongle_le_disconnect_device(&addr);
            return ATCI_RESPONSE_FLAG_APPEND_OK;
        }
    } else if (0 == memcmp(pChar, "ALL", 3)) {
        app_ull_dongle_le_disconnect_all_device();
        return ATCI_RESPONSE_FLAG_APPEND_OK;
    }

    return ATCI_RESPONSE_FLAG_APPEND_ERROR;
}

static uint32_t bt_app_comm_at_cmd_le_ull_delete_hdl(char *pChar)
{
    /* Disconnection */
    /* AT+LEULL=DELETE,<ACTION>,<PARAM> */
    /* <ACTION>: ADDR */

    if (0 == memcmp(pChar, "ADDR", 4)) {
        bt_addr_t addr = {0};
        uint16_t addr_5, addr_4, addr_3, addr_2, addr_1, addr_0;

        /* Connect device by bd_addr */
        /* AT+LEULL=DELETE,ADDR,<B0>:<B1>:<B2>:<B3>:<B4>:<B5> */
        pChar = strchr(pChar, ',');
        pChar++;

        if (sscanf(pChar, "%2x:%2x:%2x:%2x:%2x:%2x",
                   (unsigned int *)&addr_5, (unsigned int *)&addr_4, (unsigned int *)&addr_3,
                   (unsigned int *)&addr_2, (unsigned int *)&addr_1, (unsigned int *)&addr_0) > 0) {

            LOG_MSGID_I(BT_APP, "[LE_ULL][AT] Delete LEA:%2x:%2x:%2x:%2x:%2x:%2x", 6, addr_5, addr_4, addr_3, addr_2, addr_1, addr_0);

            /* Disconnect device by bd_addr */
            addr.type = BT_ADDR_RANDOM;
            addr.addr[5] = addr_5;
            addr.addr[4] = addr_4;
            addr.addr[3] = addr_3;
            addr.addr[2] = addr_2;
            addr.addr[1] = addr_1;
            addr.addr[0] = addr_0;
            app_ull_dongle_le_delete_device(&addr);
            return ATCI_RESPONSE_FLAG_APPEND_OK;
        }
    }

    return ATCI_RESPONSE_FLAG_APPEND_ERROR;
}

static uint32_t bt_app_comm_at_cmd_le_ull_call_hdl(char *pChar)
{
    return 0xFF;
}

#if 0
static uint32_t bt_app_comm_at_cmd_le_ull_call_hdl(char *pChar)
{
    bt_status_t result = BT_STATUS_UNSUPPORTED;
    uint8_t service_idx = ble_tbs_get_gtbs_service_idx();

    /* Call */
    /* AT+LEULL=CALL,<ACTION> */
    /* <ACTION>: INCOMING, ACCEPT, TERMINATE */
    if (0 == memcmp(pChar, "INCOMING", 5)) {
        /* disable audio streaming before incoming call */
        uint8_t uri[6] = {'a', 'i', 'r', 'o', 'h', 'o'};
        //app_le_audio_set_usb_hid_call(false);
        bt_le_audio_source_call_set_incoming_call(ble_tbs_get_gtbs_service_idx(), uri, 6, NULL, 0);
        result = BT_STATUS_SUCCESS;

    } else if (0 == memcmp(pChar, "PLACE", 5)) {
        /* disable audio streaming before outgoing call */
        uint8_t uri[7] = {'a', 'i', 'r', 'o', 'h', 'o', '1'};
        //app_le_audio_unicast_stop_stream();
        bt_le_audio_source_call_originate(service_idx, uri, 7);
        result = BT_STATUS_SUCCESS;

    } else if (0 == memcmp(pChar, "ACCEPT", 6)) {
        bt_le_audio_source_call_accept_t le_param = {
            .service_index = service_idx,
            .call_index = 1,
        };
        if (BLE_TBS_STATE_DIALING == bt_le_audio_source_call_get_state(service_idx, 1)) {
            bt_le_audio_source_call_alerting_t alerting = {
                .service_index = service_idx,
                .call_index = 1,
            };
            bt_le_audio_source_send_action(BT_LE_AUDIO_SOURCE_ACTION_CALL_ALERTING, &alerting);
        }
        result = bt_le_audio_source_send_action(BT_LE_AUDIO_SOURCE_ACTION_CALL_ACCEPT, &le_param);

    } else if (0 == memcmp(pChar, "TERMINATE", 9)) {
        bt_le_audio_source_call_terminate_t le_param = {
            .service_index = service_idx,
            .call_index = 1,
            .reason = BLE_TBS_TERMINATION_REASON_REMOTE_PARTY_END_CALL,
        };
        result = bt_le_audio_source_send_action(BT_LE_AUDIO_SOURCE_ACTION_CALL_TERMINATE, &le_param);
    }

    LOG_MSGID_I(BT_APP, "[AT_CMD] Call result:%x \r\n", 1, result);

    if (BT_STATUS_SUCCESS == result) {
        return ATCI_RESPONSE_FLAG_APPEND_OK;
    }

    return ATCI_RESPONSE_FLAG_APPEND_ERROR;
}
#endif
static uint32_t bt_app_comm_at_cmd_le_ull_media_hdl(char *pChar)
{
    bt_status_t result = BT_STATUS_UNSUPPORTED;

    /* AT+LEULL=MEDIA,<ACTION> */
    /* <ACTION>: PLAY, STOP, NEXT */
    if (0 == memcmp(pChar, "PLAY", 4)) {
        bt_ull_streaming_t stream;
        uint8_t port;
        sscanf(pChar + 5, "%d", (int *)(&(port)));
        stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
        stream.port = port;    /* gaming or chat */
        result = bt_ull_action(BT_ULL_ACTION_START_STREAMING, &stream, sizeof(stream));
    } else if (0 == memcmp(pChar, "STOP", 4)) {
        bt_ull_streaming_t stream;
        uint8_t port;
        sscanf(pChar + 5, "%d", (int *)(&(port)));
        stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
        stream.port = port;    /* gaming or chat */
        result = bt_ull_action(BT_ULL_ACTION_STOP_STREAMING, &stream, sizeof(stream));
    } else if (0 == memcmp(pChar, "CS", 2)) {
        bt_ull_le_find_cs_info_t cs_req = {
            .duration = APPS_ULL_LE_FIND_CS_DURATION,//30s
            .conn_handle = BT_HANDLE_INVALID,
        };
        result = bt_ull_action(BT_ULL_ACTION_FIND_CS_INFO, &cs_req, sizeof(bt_ull_le_find_cs_info_t));
    } else if (0 == memcmp(pChar, "VOLUME", 6)) {
        bt_ull_volume_t vol;
        uint8_t port;
        sscanf(pChar + 7, "%d", (int *)(&(port)));
        vol.streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
        vol.streaming.port = port;   /* gaming or chat */
        vol.action = BT_ULL_VOLUME_ACTION_SET_ABSOLUTE_VOLUME;
        vol.channel = BT_ULL_AUDIO_CHANNEL_DUAL;
        vol.volume = 0x11;
        //vol.gain = 0x1111;
        result = bt_ull_action(BT_ULL_ACTION_SET_STREAMING_VOLUME, &vol, sizeof(vol));
    } else if (0 == memcmp(pChar, "MUTE", 4)) {
        bt_ull_streaming_t stream;
        uint8_t port;
        sscanf(pChar + 7, "%d", (int *)(&(port)));
        stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
        stream.port = port;    /* gaming or chat */
        result = bt_ull_action(BT_ULL_ACTION_SET_STREAMING_MUTE, &stream, sizeof(stream));
    } else if (0 == memcmp(pChar, "UNMUTE", 6)) {
        bt_ull_streaming_t stream;
        uint8_t port;
        sscanf(pChar + 7, "%d", (int *)(&(port)));
        stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
        stream.port = port;    /* gaming or chat */
        result = bt_ull_action(BT_ULL_ACTION_SET_STREAMING_UNMUTE, &stream, sizeof(stream));
    } else if (0 == memcmp(pChar, "SAMPLERATE", 10)) {
        bt_ull_sample_rate_t sp_rate;
        sp_rate.streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
        sp_rate.streaming.port = 0;    /* gaming */

        sp_rate.sample_rate = 64000;
        result = bt_ull_action(BT_ULL_ACTION_SET_STREAMING_SAMPLE_RATE, &sp_rate, sizeof(sp_rate));
    } else if (0 == memcmp(pChar, "LATENCY", 7)) {
        bt_ull_latency_t set_latency;
        set_latency.streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
        set_latency.streaming.port = 0; /* gaming */
        set_latency.latency = 10;
        result = bt_ull_action(BT_ULL_ACTION_SET_STREAMING_LATENCY, &set_latency, sizeof(set_latency));
    } else if (0 == memcmp(pChar, "RATIOMIX", 8)) {
        bt_ull_mix_ratio_t mix_ratio;
        mix_ratio.num_streaming = 2;
        mix_ratio.streamings[0].streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
        mix_ratio.streamings[0].streaming.port = 0; /* gaming streaming port */
        mix_ratio.streamings[1].streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_SPEAKER;
        mix_ratio.streamings[1].streaming.port = 1; /* chat streaming port */

        mix_ratio.streamings[0].ratio = 20;
        mix_ratio.streamings[1].ratio = 80;
        result = bt_ull_action(BT_ULL_ACTION_SET_STREAMING_MIX_RATIO, &mix_ratio, sizeof(mix_ratio));
    } else if (0 == memcmp(pChar, "TXDATA", 6)) {
        bt_ull_user_data_t tx_data;
        app_ull_user_data_t *ull_app_data = NULL;
        ull_app_data = (app_ull_user_data_t *)pvPortMalloc(sizeof(app_ull_user_data_t));
        if (ull_app_data) {
            ull_app_data->user_evt = ULL_EVT_DONGLE_MODE;
            ull_app_data->data_len = 1;
            ull_app_data->data[0] = APP_DONGLE_MODE_PC;
            tx_data.user_data = (uint8_t *)ull_app_data;
            tx_data.user_data_length = sizeof(app_ull_user_data_t);
            result = bt_ull_action(BT_ULL_ACTION_TX_USER_DATA, &tx_data, sizeof(tx_data));

            vPortFree(ull_app_data);
        }

    }

    LOG_MSGID_I(BT_APP, "[LE_ULL][AT_CMD] ULL LE Media result:%x \r\n", 1, result);

    if (BT_STATUS_SUCCESS == result) {
        return ATCI_RESPONSE_FLAG_APPEND_OK;
    }

    return ATCI_RESPONSE_FLAG_APPEND_ERROR;
}

#ifdef AIR_WIRELESS_MIC_ENABLE
static uint32_t bt_app_comm_at_cmd_le_ull_lineout_hdl(char *pChar)
{
    bt_status_t result = BT_STATUS_UNSUPPORTED;

    /* AT+LEULL=LINEOUT,<ACTION> */
    /* <ACTION>: MUTE, UNMUTE */
    if (0 == memcmp(pChar, "PLAY", 4)) {
        bt_ull_streaming_t stream;
        stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_LINE_OUT;
        stream.port = 0;
        result = bt_ull_action(BT_ULL_ACTION_START_STREAMING, &stream, sizeof(stream));
    } else if (0 == memcmp(pChar, "STOP", 4)) {
        bt_ull_streaming_t stream;
        stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_LINE_OUT;
        stream.port = 0;
        result = bt_ull_action(BT_ULL_ACTION_STOP_STREAMING, &stream, sizeof(stream));
    } else if (0 == memcmp(pChar, "VOLUME", 6)) {
        bt_ull_volume_t vol;
        char *buf = (pChar + 7);
        vol.streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_LINE_OUT;
        vol.streaming.port = 0;
        vol.action = BT_ULL_VOLUME_ACTION_SET_ABSOLUTE_VOLUME;
        vol.channel = BT_ULL_AUDIO_CHANNEL_DUAL;
        vol.volume = (uint8_t)strtoul(buf, NULL, 10);
        result = bt_ull_action(BT_ULL_ACTION_SET_STREAMING_VOLUME, &vol, sizeof(vol));
    } else if (0 == memcmp(pChar, "MUTE", 4)) {
        bt_ull_streaming_t stream;
        stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_LINE_OUT;
        stream.port = 0;
        result = bt_ull_action(BT_ULL_ACTION_SET_STREAMING_MUTE, &stream, sizeof(stream));
    } else if (0 == memcmp(pChar, "UNMUTE", 6)) {
        bt_ull_streaming_t stream;
        stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_LINE_OUT;
        stream.port = 0;
        result = bt_ull_action(BT_ULL_ACTION_SET_STREAMING_MUTE, &stream, sizeof(stream));
    } else if (0 == memcmp(pChar, "SAMPLERATE", 10)) {
        bt_ull_sample_rate_t sp_rate;
        sp_rate.streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_LINE_OUT;
        sp_rate.streaming.port = 0;
        sp_rate.sample_rate = 48000;
        result = bt_ull_action(BT_ULL_ACTION_SET_STREAMING_SAMPLE_RATE, &sp_rate, sizeof(sp_rate));
    } else if (0 == memcmp(pChar, "LATENCY", 7)) {
        bt_ull_latency_t set_latency;
        set_latency.streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_LINE_OUT;
        set_latency.streaming.port = 0;
        set_latency.latency = 5;
        result = bt_ull_action(BT_ULL_ACTION_SET_STREAMING_LATENCY, &set_latency, sizeof(set_latency));
    }

    LOG_MSGID_I(BT_APP, "[LE_ULL][AT_CMD] ULL LE Lineout result:%x \r\n", 1, result);

    if (BT_STATUS_SUCCESS == result) {
        return ATCI_RESPONSE_FLAG_APPEND_OK;
    }

    return ATCI_RESPONSE_FLAG_APPEND_ERROR;

}

static uint32_t bt_app_comm_at_cmd_le_ull_i2sout_hdl(char *pChar)
{
    bt_status_t result = BT_STATUS_UNSUPPORTED;

    /* AT+LEULL=LINEOUT,<ACTION> */
    /* <ACTION>: MUTE, UNMUTE */
    if (0 == memcmp(pChar, "PLAY", 4)) {
        bt_ull_streaming_t stream;
        stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_I2S_OUT;
        stream.port = 0;
        result = bt_ull_action(BT_ULL_ACTION_START_STREAMING, &stream, sizeof(stream));
    } else if (0 == memcmp(pChar, "STOP", 4)) {
        bt_ull_streaming_t stream;
        stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_I2S_OUT;
        stream.port = 0;
        result = bt_ull_action(BT_ULL_ACTION_STOP_STREAMING, &stream, sizeof(stream));
    } else if (0 == memcmp(pChar, "VOLUME", 6)) {
        bt_ull_volume_t vol;
        char *buf = (pChar + 7);
        vol.streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_I2S_OUT;
        vol.streaming.port = 0;
        vol.action = BT_ULL_VOLUME_ACTION_SET_ABSOLUTE_VOLUME;
        vol.channel = BT_ULL_AUDIO_CHANNEL_DUAL;
        vol.volume = (uint8_t)strtoul(buf, NULL, 10);
        result = bt_ull_action(BT_ULL_ACTION_SET_STREAMING_VOLUME, &vol, sizeof(vol));
    } else if (0 == memcmp(pChar, "MUTE", 4)) {
        bt_ull_streaming_t stream;
        stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_I2S_OUT;
        stream.port = 0;
        result = bt_ull_action(BT_ULL_ACTION_SET_STREAMING_MUTE, &stream, sizeof(stream));
    } else if (0 == memcmp(pChar, "UNMUTE", 6)) {
        bt_ull_streaming_t stream;
        stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_I2S_OUT;
        stream.port = 0;
        result = bt_ull_action(BT_ULL_ACTION_SET_STREAMING_MUTE, &stream, sizeof(stream));
    } else if (0 == memcmp(pChar, "SAMPLERATE", 10)) {
        bt_ull_sample_rate_t sp_rate;
        sp_rate.streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_I2S_OUT;
        sp_rate.streaming.port = 0;
        sp_rate.sample_rate = 48000;
        result = bt_ull_action(BT_ULL_ACTION_SET_STREAMING_SAMPLE_RATE, &sp_rate, sizeof(sp_rate));
    } else if (0 == memcmp(pChar, "LATENCY", 7)) {
        bt_ull_latency_t set_latency;
        set_latency.streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_I2S_OUT;
        set_latency.streaming.port = 0;
        set_latency.latency = 5;
        result = bt_ull_action(BT_ULL_ACTION_SET_STREAMING_LATENCY, &set_latency, sizeof(set_latency));
    }

    LOG_MSGID_I(BT_APP, "[LE_ULL][AT_CMD] ULL LE I2sout result:%x \r\n", 1, result);

    if (BT_STATUS_SUCCESS == result) {
        return ATCI_RESPONSE_FLAG_APPEND_OK;
    }

    return ATCI_RESPONSE_FLAG_APPEND_ERROR;

}

#endif

static uint32_t bt_app_comm_at_cmd_le_ull_mic_hdl(char *pChar)
{
    bt_status_t result = BT_STATUS_UNSUPPORTED;

    /* AT+LEULL=MIC,<ACTION> */
    /* <ACTION>: MUTE, UNMUTE */
    if (0 == memcmp(pChar, "PLAY", 4)) {
        bt_ull_streaming_t stream;
        stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
        stream.port = 0;    /* mic */
        result = bt_ull_action(BT_ULL_ACTION_START_STREAMING, &stream, sizeof(stream));
    } else if (0 == memcmp(pChar, "STOP", 4)) {
        bt_ull_streaming_t stream;
        stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
        stream.port = 0;    /* mic */
        result = bt_ull_action(BT_ULL_ACTION_STOP_STREAMING, &stream, sizeof(stream));
    } else if (0 == memcmp(pChar, "VOLUME", 6)) {
        bt_ull_volume_t vol;
        vol.streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
        vol.streaming.port = 0;    /* mic */
        vol.action = BT_ULL_VOLUME_ACTION_SET_ABSOLUTE_VOLUME;
        vol.channel = BT_ULL_AUDIO_CHANNEL_DUAL;
        vol.volume = 0x22;
        //vol.gain = 0x2222;
        result = bt_ull_action(BT_ULL_ACTION_SET_STREAMING_VOLUME, &vol, sizeof(vol));
    } else if (0 == memcmp(pChar, "MUTE", 4)) {
        bt_ull_streaming_t stream;
        stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
        stream.port = 0;    /* mic */
        result = bt_ull_action(BT_ULL_ACTION_SET_STREAMING_MUTE, &stream, sizeof(stream));
    } else if (0 == memcmp(pChar, "UNMUTE", 6)) {
        bt_ull_streaming_t stream;
        stream.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
        stream.port = 0;    /* mic */
        result = bt_ull_action(BT_ULL_ACTION_SET_STREAMING_MUTE, &stream, sizeof(stream));
    } else if (0 == memcmp(pChar, "SAMPLERATE", 10)) {
        bt_ull_sample_rate_t sp_rate;
        sp_rate.streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
        sp_rate.streaming.port = 0;    /* mic */

        sp_rate.sample_rate = 48000;
        result = bt_ull_action(BT_ULL_ACTION_SET_STREAMING_SAMPLE_RATE, &sp_rate, sizeof(sp_rate));
    } else if (0 == memcmp(pChar, "LATENCY", 7)) {
        bt_ull_latency_t set_latency;
        set_latency.streaming.streaming_interface = BT_ULL_STREAMING_INTERFACE_MICROPHONE;
        set_latency.streaming.port = 0; /* mic */
        set_latency.latency = 5;
        result = bt_ull_action(BT_ULL_ACTION_SET_STREAMING_LATENCY, &set_latency, sizeof(set_latency));
    }

    LOG_MSGID_I(BT_APP, "[LE_ULL][AT_CMD] ULL LE MIC result:%x \r\n", 1, result);

    if (BT_STATUS_SUCCESS == result) {
        return ATCI_RESPONSE_FLAG_APPEND_OK;
    }

    return ATCI_RESPONSE_FLAG_APPEND_ERROR;

}

#if 0
static uint32_t bt_app_comm_at_cmd_le_ull_volume_hdl(char *pChar)
{
    /* AT+LEULL=VOLUME,<ACTION> */
    /* <ACTION>: MUTE, UNMUTE, UP, DOWN, SET */
    if (0 == memcmp(pChar, "MUTE", 4)) {
        bt_le_audio_source_send_action(BT_LE_AUDIO_SOURCE_ACTION_VOLUME_MUTE, NULL);

    } else if (0 == memcmp(pChar, "UNMUTE", 6)) {
        bt_le_audio_source_send_action(BT_LE_AUDIO_SOURCE_ACTION_VOLUME_UNMUTE, NULL);

    } else if (0 == memcmp(pChar, "UP", 2)) {
        bt_le_audio_source_send_action(BT_LE_AUDIO_SOURCE_ACTION_VOLUME_UNMUTE_UP, NULL);

    } else if (0 == memcmp(pChar, "DOWN", 4)) {
        bt_le_audio_source_send_action(BT_LE_AUDIO_SOURCE_ACTION_VOLUME_UNMUTE_DOWN, NULL);

    } else if (0 == memcmp(pChar, "SET", 3)) {
        bt_le_audio_source_volume_set_absolute_volume_t absolute;

        /* AT+LEAUDIO=VOLUME,SET,<volume> */
        char *buf = (pChar + 4);
        absolute.volume = (uint8_t)strtoul(buf, NULL, 10);
        if (absolute.volume > 15) {
            return ATCI_RESPONSE_FLAG_APPEND_ERROR;
        }

        absolute.volume *= 0x11;
        if (BT_STATUS_SUCCESS != bt_le_audio_source_send_action(BT_LE_AUDIO_SOURCE_ACTION_VOLUME_SET_ABSOLUTE_VOLUME, &absolute)) {
            return ATCI_RESPONSE_FLAG_APPEND_ERROR;
        }
    }

    return ATCI_RESPONSE_FLAG_APPEND_OK;
}
static uint32_t bt_app_comm_at_cmd_le_ull_cig_hdl(char *pChar)
{
    /* AT+LEULL=CIGTEST,<ACTION> */
    /* <ACTION>: ON, OFF */
    if (0 == memcmp(pChar, "OFF", 3)) {
        app_le_audio_ucst_reset_cig_parameter_test();

    } else if (0 == memcmp(pChar, "ON", 2)) {
        /* AT+LEAUDIO=CIGTEST,ON,<BN>,<NSE>,<FT>,<ISO-INTERVAL> */
        pChar = strchr(pChar, ',');
        pChar++;
        uint8_t bn = atoi(pChar);
        pChar = strchr(pChar, ',');
        pChar++;
        uint8_t nse = atoi(pChar);
        pChar = strchr(pChar, ',');
        pChar++;
        uint8_t ft = atoi(pChar);
        pChar = strchr(pChar, ',');
        pChar++;
        uint16_t iso_interval = atoi(pChar);
        app_le_audio_ucst_set_cig_parameter_test(bn, nse, ft, iso_interval);

    } else {
        return ATCI_RESPONSE_FLAG_APPEND_ERROR;
    }

    return ATCI_RESPONSE_FLAG_APPEND_OK;
}

static uint32_t bt_app_comm_at_cmd_le_ull_config_hdl(char *pChar)
{
    /* AT+LEAUDIO=CONFIG,<ACTION> */
    /* <ACTION>: SET */
    if (0 == memcmp(pChar, "SET", 3)) {
        uint8_t port = 0;
        /* AT+LEAUDIO=CONFIG,SET,<MODE>,<sampling_rate>,<sel_setting><high_reliability> */
        pChar = strchr(pChar, ',');
        pChar++;

        if (0 == memcmp(pChar, "SPK_0", 5)) {
            port = 0;

        } else if (0 == memcmp(pChar, "SPK_1", 5)) {
            port = 1;

        } else if (0 == memcmp(pChar, "MIC_0", 5)) {
            port = 2;

        } else if (0 == memcmp(pChar, "BROADCAST", 9)) {
            port = 3;

        } else {
            return ATCI_RESPONSE_FLAG_APPEND_ERROR;
        }

        pChar = strchr(pChar, ',');
        pChar++;

        uint8_t sampling_rate = atoi(pChar);
        pChar = strchr(pChar, ',');
        pChar++;

        uint8_t sel_setting = atoi(pChar);
        pChar = strchr(pChar, ',');
        pChar++;

        uint8_t high_reliability = atoi(pChar);

        if (3 != port) {
            app_le_audio_ucst_set_qos_params(sampling_rate, sel_setting, high_reliability, port);

        } else {
            app_le_audio_bcst_set_qos_params(sampling_rate, sel_setting, high_reliability);
        }

    } else {
        return ATCI_RESPONSE_FLAG_APPEND_ERROR;
    }

    return ATCI_RESPONSE_FLAG_APPEND_OK;
}
#endif

static atci_status_t bt_app_comm_at_cmd_le_ull_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_OK};
    char *pChar = NULL;

    if (parse_cmd->mode == ATCI_CMD_MODE_EXECUTION) {
        pChar = (parse_cmd->string_ptr + parse_cmd->name_len + 1);

        if (0 == memcmp(pChar, "SIRK", 4)) {
            /* Set SIRK */
            /* AT+LEULL=SIRK,SET,<B0>,<B1>,<B2>,<B3>,<B4>,<B5>,<B6>,<B7>,<B8>,<B9>,<B10>,<B11>,<B12>,<B13>,<B14>,<B15> */
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_ull_sirk_hdl(pChar);

        } else if (0 == memcmp(pChar, "SCAN", 4)) {
            /* Bonding information */
            /* AT+LEULL=SCAN,<ACTION> */
            /* <ACTION>: ON, OFF*/
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_ull_scan_hdl(pChar);

        } else if (0 == memcmp(pChar, "BOND", 4)) {
            /* Bonding information */
            /* AT+LEULL=BOND,<ACTION> */
            /* <ACTION>: RM */
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_ull_bond_hdl(pChar);

        } else if (0 == memcmp(pChar, "CONN", 4)) {
            /* Create connection */
            /* AT+LEULL=CONN,<ACTION>,<PARAM> */
            /* <ACTION>: SIRK, ADDR */
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_ull_conn_hdl(pChar);

        } else if (0 == memcmp(pChar, "DISCONN", 7)) {
            /* Disconnection */
            /* AT+LEULL=DISCONN,<ACTION>,<PARAM> */
            /* <ACTION>: ADDR */
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_ull_disconn_hdl(pChar);

        }  else if (0 == memcmp(pChar, "DELETE", 6)) {
            /* Disconnection */
            /* AT+LEULL=DISCONN,<ACTION>,<PARAM> */
            /* <ACTION>: ADDR */
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_ull_delete_hdl(pChar);

        } else if (0 == memcmp(pChar, "MEDIA", 5)) {
            /* Media */
            /* AT+LEULL=MEDIA,<ACTION> */
            /* <ACTION>: PLAY, PAUSE */
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_ull_media_hdl(pChar);
        } else if (0 == memcmp(pChar, "CALL", 4)) {
            /* Call */
            /* AT+LEULL=CALL,<ACTION> */
            /* <ACTION>: ACCEPT, TERMINATE, PLACE */
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_ull_call_hdl(pChar);
        } else if (0 == memcmp(pChar, "MIC", 3)) {
            /* Bonding information */
            /* AT+LEULL=MIC,<ACTION> */
            /* <ACTION>: PLAY, MUTE, UNMUTE, STOP */
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_ull_mic_hdl(pChar);

        }  else if (0 == memcmp(pChar, "CHECK", 5)) {
            /* AT+LEULL=CHECK,<ACTION> */
            /* <ACTION>: STREAMING, CONNECTION */
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_ull_check_hdl(pChar);
        }  else if (0 == memcmp(pChar, "CONTROL", 7)) {
            /* AT+LEULL=CONTROL,<ACTION> */
            /* <ACTION>: LOCK, UNLOCK */
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_ull_control_hdl(pChar);
        }
#ifdef AIR_WIRELESS_MIC_ENABLE
        else if (0 == memcmp(pChar, "LINEOUT", 7)) {
            /* Bonding information */
            /* AT+LEULL=LINEOUT,<ACTION> */
            /* <ACTION>: PLAY, MUTE, UNMUTE, STOP */
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_ull_lineout_hdl(pChar);

        } else if (0 == memcmp(pChar, "I2SOUT", 6)) {
            /* Bonding information */
            /* AT+LEULL=I2SOUT,<ACTION> */
            /* <ACTION>: PLAY, MUTE, UNMUTE, STOP */
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_ull_i2sout_hdl(pChar);
        }
#endif
#if 0
        else if (0 == memcmp(pChar, "VOLUME", 6)) {
            /* Volume */
            /* AT+LEULL=VOLUME,<ACTION> */
            /* <ACTION>: MUTE, UNMUTE, UP, DOWN, SET */
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_audio_volume_hdl(pChar);

        } else if (0 == memcmp(pChar, "CIGTEST", 7)) {
            /* CIG test */
            /* AT+LEAUDIO=CIGTEST,<ACTION> */
            /* <ACTION>: ON, OFF */
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_audio_cig_hdl(pChar);

        } else if (0 == memcmp(pChar, "CONFIG", 6)) {
            /* CIG test */
            /* AT+LEAUDIO=CONFIG,<ACTION> */
            /* <ACTION>: SET, RESET */
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_le_audio_config_hdl(pChar);

        } else if (0 == memcmp(pChar, "UPDATE", 6)) {
            /* LE audio dongle test, will remove this at cmd after test complete. */
            /* AT+LEAUDIO=CONN_UPDATE,handle,interval,latency,supervision_timeout,ce.*/
            printf("[APP][COMMON] set LE connection update interval1\r\n");
            bt_hci_cmd_le_connection_update_t new_param;
            if (sscanf(pChar + strlen("UPDATE,"), "%x,%x,%x,%x,%x",
                       (unsigned int *)&new_param.connection_handle,
                       (unsigned int *)&new_param.conn_interval_min,
                       (unsigned int *)&new_param.conn_latency,
                       (unsigned int *)&new_param.supervision_timeout,
                       (unsigned int *)&new_param.minimum_ce_length) > 0) {
                new_param.conn_interval_max = new_param.conn_interval_min;
                new_param.maximum_ce_length = new_param.minimum_ce_length;
                bt_status_t status = bt_gap_le_update_connection_parameter((const bt_hci_cmd_le_connection_update_t *)(&new_param));
                LOG_MSGID_I(BT_APP, "[APP][COMMON] set LE connection update interval parameter handle = %02x, interval = %02x, latency = %02x,\
                                        supervision_timeout = %02x, ce_length = %02x , status = %02x", 6, new_param.connection_handle, new_param.conn_interval_min, \
                            new_param.conn_latency, new_param.supervision_timeout, new_param.minimum_ce_length, status);
                if (status == BT_STATUS_SUCCESS) {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                } else {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                }
            } else {
                response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                LOG_MSGID_I(BT_APP, "[APP][COMMON] set LE connection update interval parameter fail", 0);
            }
        }
#endif
        else {
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
        }
    }

    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}

#endif  /* AIR_BLE_ULTRA_LOW_LATENCY_ENABLE */

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
static uint32_t bt_app_comm_at_cmd_ull_hid_scan_hdl(char *pChar)
{
    /* Bonding information */
    /* AT+LEULL=SCAN,<ACTION> */
    /* <ACTION>: ON, OFF */

    if (0 == memcmp(pChar, "ON", 2)) {
        /* start scan devices */
        app_dongle_ull_le_hid_start_scan(APP_DONGLE_ULL_LE_HID_SCAN_AUDIO | APP_DONGLE_ULL_LE_HID_SCAN_KEYBOARD | APP_DONGLE_ULL_LE_HID_SCAN_MOUSE);
        return ATCI_RESPONSE_FLAG_APPEND_OK;

    } else if (0 == memcmp(pChar, "OFF", 3)) {
        /* start scan devices */
        app_dongle_ull_le_hid_stop_scan(APP_DONGLE_ULL_LE_HID_SCAN_AUDIO | APP_DONGLE_ULL_LE_HID_SCAN_KEYBOARD | APP_DONGLE_ULL_LE_HID_SCAN_MOUSE);
        return ATCI_RESPONSE_FLAG_APPEND_OK;
    }

    return ATCI_RESPONSE_FLAG_APPEND_ERROR;
}

static uint32_t bt_app_comm_at_cmd_ull_hid_conn_hdl(char *pChar)
{
    /* Create connection */
    /* AT+LEULL=CONN,<ACTION>,<PARAM> */
    /* <ACTION>: SIRK, ADDR */
    uint8_t device_type = BT_ULL_LE_HID_SRV_DEVICE_NONE;
    if (0 == memcmp(pChar, "HEADSET", 7)) {
        device_type = BT_ULL_LE_HID_SRV_DEVICE_HEADSET;
    } else if (0 == memcmp(pChar, "KEYBOARD", 8)) {
        device_type = BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD;

    } else if (0 == memcmp(pChar, "MOUSE", 5)) {
        device_type = BT_ULL_LE_HID_SRV_DEVICE_MOUSE;
    }
    pChar = strchr(pChar, ',');
    pChar++;
    bt_addr_t addr = {0};
    uint16_t addr_5, addr_4, addr_3, addr_2, addr_1, addr_0;
    /* AT+LEULL=CONN,ADDR,<B0>:<B1>:<B2>:<B3>:<B4>:<B5> */
    if (sscanf(pChar, "%2x:%2x:%2x:%2x:%2x:%2x",
               (unsigned int *)&addr_5, (unsigned int *)&addr_4, (unsigned int *)&addr_3,
               (unsigned int *)&addr_2, (unsigned int *)&addr_1, (unsigned int *)&addr_0) > 0) {

        LOG_MSGID_I(BT_APP, "[ULL_HID][AT] Conn type: %d, addr:%2x:%2x:%2x:%2x:%2x:%2x", 7, device_type, addr_5, addr_4, addr_3, addr_2, addr_1, addr_0);

        /* Connect device by bd_addr */
        addr.type = BT_ADDR_RANDOM;
        addr.addr[5] = addr_5;
        addr.addr[4] = addr_4;
        addr.addr[3] = addr_3;
        addr.addr[2] = addr_2;
        addr.addr[1] = addr_1;
        addr.addr[0] = addr_0;
        app_dongle_ull_le_hid_connect_device(device_type, &addr);
        return  ATCI_RESPONSE_FLAG_APPEND_OK;
    }

    return ATCI_RESPONSE_FLAG_APPEND_ERROR;
}

static uint32_t bt_app_comm_at_cmd_ull_hid_clear_bond_list_hdl(char *pChar)
{
    uint8_t device_type = BT_ULL_LE_HID_SRV_DEVICE_NONE;
    if (0 == memcmp(pChar, "HEADSET", 7)) {
        device_type = BT_ULL_LE_HID_SRV_DEVICE_HEADSET;
    } else if (0 == memcmp(pChar, "KEYBOARD", 8)) {
        device_type = BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD;

    } else if (0 == memcmp(pChar, "MOUSE", 5)) {
        device_type = BT_ULL_LE_HID_SRV_DEVICE_MOUSE;
    }
    if (!bt_ull_le_hid_srv_clear_bonded_list(device_type)) {
        return ATCI_RESPONSE_FLAG_APPEND_OK;
    }
    return ATCI_RESPONSE_FLAG_APPEND_ERROR;
}

static atci_status_t bt_app_comm_at_cmd_ull_hid_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_OK};
    char *pChar = NULL;
    if (parse_cmd->mode == ATCI_CMD_MODE_EXECUTION) {
        pChar = (parse_cmd->string_ptr + parse_cmd->name_len + 1);
        if (0 == memcmp(pChar, "SCAN", 4)) {
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_ull_hid_scan_hdl(pChar);
        } else if (0 == memcmp(pChar, "CONNECT", 7)) {
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_ull_hid_conn_hdl(pChar);

        } else if (0 == memcmp(pChar, "CLEAR_BL", 8)) {
            pChar = strchr(pChar, ',');
            pChar++;
            response.response_flag = bt_app_comm_at_cmd_ull_hid_clear_bond_list_hdl(pChar);
        } else if (0 == memcmp(pChar, "CREATE_ALL_CIS", 14)) {
            pChar = strchr(pChar, ',');
            pChar++;
            bt_status_t status = app_dongle_ull_le_hid_start_up();
            response.response_flag = (status == BT_STATUS_SUCCESS || status == BT_STATUS_PENDING) ? ATCI_RESPONSE_FLAG_APPEND_OK : ATCI_RESPONSE_FLAG_APPEND_ERROR;
        } else if (0 == memcmp(pChar, "SET_SCENARIO", 12)) {
            pChar = strchr(pChar, ',');
            pChar++;
            uint8_t scenario = 0;
            sscanf(pChar, "%d", (int *)(&(scenario)));
            bt_status_t status = app_dongle_ull_le_hid_set_scenaraio(scenario);
            response.response_flag = (status == BT_STATUS_SUCCESS || status == BT_STATUS_PENDING) ? ATCI_RESPONSE_FLAG_APPEND_OK : ATCI_RESPONSE_FLAG_APPEND_ERROR;
        } else if (0 == memcmp(pChar, "GET_SCENARIO", 12)) {
            pChar = strchr(pChar, ',');
            pChar++;
            uint8_t scenario = 0;
            bt_status_t status = app_dongle_ull_le_hid_get_scenaraio(&scenario);
            response.response_flag = (status == BT_STATUS_SUCCESS || status == BT_STATUS_PENDING) ? ATCI_RESPONSE_FLAG_APPEND_OK : ATCI_RESPONSE_FLAG_APPEND_ERROR;
            snprintf((char *) response.response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "SCENARIO = %d\r\n", (int)scenario);
        }
#ifdef AIR_PURE_GAMING_ENABLE
        else if (0 == memcmp(pChar, "SPEC_SCAN", 9)) {
            pChar = strchr(pChar, ',');
            pChar++;
            uint8_t enable = 0;
            sscanf(pChar, "%d", (int *)(&enable));
            bool ret = pka_free_sche_enable_spec_scan(enable);
            response.response_flag = ret ? ATCI_RESPONSE_FLAG_APPEND_OK : ATCI_RESPONSE_FLAG_APPEND_ERROR;
        }
        else if (0 == memcmp(pChar, "SET_PEC_PRAR", 12)) {
            pChar = strchr(pChar, ',');
            pChar++;
            uint32_t PEC_high_bound = 0;
            sscanf(pChar, "%d", (int *)(&PEC_high_bound));

            pChar = strchr(pChar, ',');
            pChar++;
            uint32_t PEC_low_bound = 0;
            sscanf(pChar, "%d", (int *)(&PEC_low_bound));

            pChar = strchr(pChar, ',');
            pChar++;
            uint32_t PEC_timer = 0;
            sscanf(pChar, "%d", (int *)(&PEC_timer));

            LOG_MSGID_I(BT_APP, "[LE_ULL][AT_CMD] PEC_high_bound(%d), PEC_low_bound(%d), PEC_timer(%d)", 3, PEC_high_bound, PEC_low_bound, PEC_timer);

            pka_free_sche_set_pec_para(PEC_high_bound, PEC_low_bound, PEC_timer);

            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
        }
#endif
    }
    response.response_len = strlen((char *)response.response_buf);
    atci_send_response(&response);
    return ATCI_STATUS_OK;
}

#endif

