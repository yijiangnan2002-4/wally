/* Copyright Statement:
 *
 * (C) 2022  Airoha Technology Corp. All rights reserved.
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
#include "atci.h"
#include "at_command_bt.h"
#include "bt_type.h"
#include "bt_connection_manager.h"
#include "bt_source_srv_utils.h"
#include "bt_source_srv.h"
#include "bt_source_srv_hfp.h"
#include "bt_source_srv_hfp_call_manager.h"
#include "avm_external.h"

static void bt_source_srv_bqb_call_change( bt_source_srv_call_state_t call_state, bt_source_srv_call_index_t call_index);

static atci_status_t bt_source_srv_atci_it_handler(atci_parse_cmd_param_t *parse_cmd);
#define CMD_PARAM(s) s, bt_source_srv_strlen(s)
static atci_cmd_hdlr_item_t bt_source_srv_atci_cmd[] = {
    {
        .command_head = "AT+BTSOURCEIT",               /* INTERNAL USE, IT TEST */
        .command_hdlr = bt_source_srv_atci_it_handler,
        .hash_value1 = 0,
        .hash_value2 = 0,
    }
};

static void bt_source_srv_cmd_copy_str_to_addr(uint8_t *addr, const char *str)
{
    unsigned int i, value;
    int using_long_format = 0;
    int using_hex_sign = 0;
    int result = 0;
    if (str[2] == ':' || str[2] == '-') {
        using_long_format = 1;
    }
    if (str[1] == 'x') {
        using_hex_sign = 2;
    }
    for (i = 0; i < 6; i++) {
        result = sscanf(str + using_hex_sign + i * (2 + using_long_format), "%02x", &value);
        if (result <= 0) {
            LOG_MSGID_E(source_srv, "[SOURCE][SRV][AT] addr convert fail", 0);
        }
        addr[5 - i] = (uint8_t) value;
    }
}

static bt_status_t bt_source_srv_cmd_entry(const char *string)
{
    LOG_I(source_srv, "[SOURCE][SRV][AT] cmd string = %s", string);
    bt_status_t status = BT_STATUS_FAIL;
    if (0 == bt_source_srv_memcmp(string, CMD_PARAM("CONNECT"))) {
        bt_cm_connect_t connect_param = {{0}};
        char *address_str = (char *)string + strlen("CONNECT,");
        bt_source_srv_cmd_copy_str_to_addr((void *)&connect_param.address, address_str);
        connect_param.profile =
            BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP_AG) | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SOURCE);
        status = bt_cm_connect(&connect_param);
        LOG_MSGID_I(source_srv, "[SOURCE][SRV][AT] connect status = %02x", 1, status);
    } else if (0 == bt_source_srv_memcmp(string, CMD_PARAM("DISCONNECT"))) {
        bt_cm_connect_t disconnect_param = {{0}};
        char *address_str = (char *)string + strlen("DISCONNECT,");
        bt_source_srv_cmd_copy_str_to_addr((void *)&disconnect_param.address, address_str);
        disconnect_param.profile = BT_CM_PROFILE_SERVICE_MASK_ALL;
        status = bt_cm_disconnect(&disconnect_param);
    } else if (0 == bt_source_srv_memcmp(string, CMD_PARAM("SET_BQB_MODE"))) {
        extern void bt_source_srv_music_cntx_set_bqb_flag();
        bt_source_srv_music_cntx_set_bqb_flag();
        status = BT_STATUS_SUCCESS;
    } else if (0 == bt_source_srv_memcmp(string, CMD_PARAM("NOTIFY_CHANGE"))) {
        extern bt_status_t bt_source_srv_avrcp_BQB_notify_volume_change(bool is_notify, uint8_t volume);
        uint8_t bt_volume = (uint8_t)strtoul(string + strlen("VOLUME_CHANGE,"), NULL, 16);
        bt_source_srv_avrcp_BQB_notify_volume_change(1, bt_volume);
        status = BT_STATUS_SUCCESS;
    } else if (0 == bt_source_srv_memcmp(string, CMD_PARAM("VOLUME_CHANGE"))) {
        LOG_I(source_srv, "[SOURCE][SRV][AT]VOL_CHANGE");
        bt_source_srv_volume_change_t volume_change = {0};
        volume_change.port = (uint8_t)strtoul(string + strlen("VOLUME_CHANGE,"), NULL, 16);
        volume_change.volume_value = (uint8_t)strtoul(string + strlen("VOLUME_CHANGE,") + 3, NULL, 16);
        extern void bt_source_srv_music_cntx_set_bqb_flag();
        bt_source_srv_music_cntx_set_bqb_flag();
        status = bt_source_srv_send_action(BT_SOURCE_SRV_ACTION_VOLUME_CHANGE, &volume_change, sizeof(bt_source_srv_volume_change_t));
    } else if (0 == bt_source_srv_memcmp(string, CMD_PARAM("MUTE"))) {
        bt_source_srv_audio_mute_t mute = {0};
        mute.port = (uint8_t)strtoul(string + strlen("MUTE,"), NULL, 16);
        status = bt_source_srv_send_action(BT_SOURCE_SRV_ACTION_MUTE, &mute, sizeof(bt_source_srv_audio_mute_t));
    } else if (0 == bt_source_srv_memcmp(string, CMD_PARAM("UNMUTE"))) {
        bt_source_srv_audio_unmute_t unmute = {0};
        unmute.port = (uint8_t)strtoul(string + strlen("UNMUTE,"), NULL, 16);
        status = bt_source_srv_send_action(BT_SOURCE_SRV_ACTION_MUTE, &unmute, sizeof(bt_source_srv_audio_unmute_t));
    }
#ifdef AIR_SOURCE_SRV_HFP_ENABLE
    else if (0 == bt_source_srv_memcmp(string, CMD_PARAM("NEW CALL"))) {
        bt_source_srv_call_state_t call_state = (uint8_t)strtoul(string + strlen("NEW CALL,"), NULL, 16);
        bt_source_srv_new_call_t new_call = {
            .type = BT_SOURCE_SRV_TYPE_HFP,
        };
        new_call.hfp_new_call.state = call_state;
        new_call.hfp_new_call.mode = BT_SOURCE_SRV_HFP_CALL_MODE_VOICE;
        new_call.hfp_new_call.mpty = BT_SOURCE_SRV_HFP_CALL_MPTY_INACTIVE;
        if (bt_source_srv_hfp_call_is_exist()) {
            new_call.hfp_new_call.number = (uint8_t *)"7654321";
            new_call.hfp_new_call.number_length = strlen("7654321");
        } else {
            new_call.hfp_new_call.number = (uint8_t *)"1234567";
            new_call.hfp_new_call.number_length = strlen("1234567");
        }
        new_call.hfp_new_call.iac = BT_SOURCE_SRV_HFP_CALL_IAC_WITHOUT;
        LOG_MSGID_I(source_srv, "[SOURCE][SRV][AT] new call state = %02x", 1, call_state);
        bt_source_srv_bqb_call_change(call_state, BT_SOURCE_SRV_CALL_INVALID_INDEX);
        status = bt_source_srv_send_action(BT_SOURCE_SRV_ACTION_NEW_CALL, &new_call, sizeof(bt_source_srv_new_call_t));
    } else if (0 == bt_source_srv_memcmp(string, CMD_PARAM("CALL CHANGE"))) {
        bt_source_srv_call_index_t call_index = (uint8_t)strtoul(string + strlen("CALL CHANGE,"), NULL, 16);
        bt_source_srv_call_state_t call_state = (uint8_t)strtoul(string + strlen("CALL CHANGE,") + 2, NULL, 16);
        bt_source_srv_call_state_change_t call_change = {
            .type = BT_SOURCE_SRV_TYPE_HFP,
        };
        call_change.hfp_call_change.index = call_index;
        call_change.hfp_call_change.state = call_state;
        call_change.hfp_call_change.mpty = BT_SOURCE_SRV_HFP_CALL_MPTY_INACTIVE;
        LOG_MSGID_I(source_srv, "[SOURCE][SRV][AT] call change index = %02x, state = %02x", 2, call_index, call_state);
        bt_source_srv_bqb_call_change(call_state, call_index);
        status = bt_source_srv_send_action(BT_SOURCE_SRV_ACTION_CALL_STATE_CHANGE, &call_change, sizeof(bt_source_srv_call_state_change_t));
    } else if (0 == bt_source_srv_memcmp(string, CMD_PARAM("SERVICE"))) {
        bt_source_srv_service_availability_change_t service_availability = {
            .service_ability = (uint8_t)strtoul(string + strlen("SERVICE,"), NULL, 16)
        };
        status = bt_source_srv_send_action(BT_SOURCE_SRV_ACTION_SERVICE_AVAILABILITY_CHANGE, &service_availability, sizeof(bt_source_srv_service_availability_change_t));
    } else if (0 == bt_source_srv_memcmp(string, CMD_PARAM("SIGNAL"))) {
        bt_source_srv_signal_strength_change_t signal_strength = {
            .signal_strength = (uint8_t)strtoul(string + strlen("SIGNAL,"), NULL, 16)
        };
        status = bt_source_srv_send_action(BT_SOURCE_SRV_ACTION_SIGNAL_STRENGTH_CHANGE, &signal_strength, sizeof(bt_source_srv_signal_strength_change_t));
    } else if (0 == bt_source_srv_memcmp(string, CMD_PARAM("ROAMING"))) {
        bt_source_srv_roaming_status_change_t roaming_status = {
            .roaming_state = (uint8_t)strtoul(string + strlen("ROAMING,"), NULL, 16)
        };
        status = bt_source_srv_send_action(BT_SOURCE_SRV_ACTION_ROAMING_STATUS_CHANGE, &roaming_status, sizeof(bt_source_srv_roaming_status_change_t));
    } else if (0 == bt_source_srv_memcmp(string, CMD_PARAM("BATTERY"))) {
        bt_source_srv_battery_level_change_t battery_level = {
            .battery_level = (uint8_t)strtoul(string + strlen("BATTERY,"), NULL, 16)
        };
        status = bt_source_srv_send_action(BT_SOURCE_SRV_ACTION_BATTERY_LEVEL_CHANGE, &battery_level, sizeof(bt_source_srv_battery_level_change_t));
    } else if (0 == bt_source_srv_memcmp(string, CMD_PARAM("VRA"))) {
        bt_source_srv_voice_recognition_state_change_t va_state = {
            .status = (uint8_t)strtoul(string + strlen("VRA,"), NULL, 16)
        };
        status = bt_source_srv_send_action(BT_SOURCE_SRV_ACTION_VOICE_RECOGNITION_STATE_CHANGE, &va_state, sizeof(bt_source_srv_voice_recognition_state_change_t));
    } else if (0 == bt_source_srv_memcmp(string, CMD_PARAM("SAP"))) {
        status = bt_source_srv_send_action(BT_SOURCE_SRV_ACTION_SWITCH_AUDIO_PATH, NULL, 0);
    } else if (0 == bt_source_srv_memcmp(string, CMD_PARAM("SET BQB FLAG"))) {
        status = bt_source_srv_hfp_set_bqb_flag((uint8_t)strtoul(string + strlen("SET BQB FLAG,"), NULL, 16));
    } else if (0 == bt_source_srv_memcmp(string, CMD_PARAM("REMOVE BQB FLAG"))) {
        status = bt_source_srv_hfp_remove_bqb_flag((uint8_t)strtoul(string + strlen("REMOVE BQB FLAG,"), NULL, 16));
    } else if (0 == bt_source_srv_memcmp(string, CMD_PARAM("SUPPORT SALVE ROLE:"))) {
        bool support_salve_role = strtoul(string + strlen("SUPPORT SALVE ROLE:"), NULL, 10);
        bt_source_dongle_pka_allow_bqb(support_salve_role);
        status = BT_STATUS_SUCCESS;
    } else if (0 == bt_source_srv_memcmp(string, CMD_PARAM("SCRC"))) {
        bt_source_srv_send_custom_result_code_t custom_result_code = {
            .result_code = "+CUSTOM_TEST",
            .length = strlen("+CUSTOM_TEST"),
        };
        status = bt_source_srv_send_action(BT_SOURCE_SRV_ACTION_SEND_CUSTOM_RESULT_CODE, &custom_result_code, sizeof(bt_source_srv_send_custom_result_code_t));
    }
#endif
#ifdef AIR_SOURCE_SRV_AVRCP_CT_BQB_ENABLE
    else if (0 == bt_source_srv_memcmp(string, CMD_PARAM("AVRCPBQB,"))) {
        extern atci_status_t bt_avrcp_bqb_atci_callback(const char *string);
        status = bt_avrcp_bqb_atci_callback(string + strlen("AVRCPBQB,"));
    }
#endif
    return status;
}

static atci_status_t bt_source_srv_atci_it_handler(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t *response = (atci_response_t *)bt_source_srv_memory_alloc(sizeof(atci_response_t));
    if (NULL == response) {
        LOG_MSGID_E(source_srv, "[SOURCE][SRV][AT] malloc heap memory fail", 0);
        return ATCI_STATUS_ERROR;
    }

    bt_source_srv_memset(response, 0, sizeof(atci_response_t));

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: /* rec: AT+BTSINKIT=<ACTION, PARAMS> */
            if (0 == bt_source_srv_memcmp(parse_cmd->string_ptr + 5, "SOURCEIT", strlen("SOURCEIT"))) {
                bt_status_t result;
                result = bt_source_srv_cmd_entry(parse_cmd->string_ptr + 14);
                if (result == BT_STATUS_SUCCESS) {
                    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                } else {
                    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                }
            } else {
                response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            }
            response->response_len = strlen((char *)response->response_buf);
            atci_send_response(response);
            break;

        default :
            /* others are invalid command format */
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            response->response_len = strlen((char *)response->response_buf);
            atci_send_response(response);
            break;
    }
    bt_source_srv_memory_free(response);
    return ATCI_STATUS_OK;
}


void bt_source_srv_at_cmd_init(void)
{
    atci_status_t ret;

    ret = atci_register_handler(bt_source_srv_atci_cmd, sizeof(bt_source_srv_atci_cmd) / sizeof(atci_cmd_hdlr_item_t));
    if (ret != ATCI_STATUS_OK) {
        LOG_MSGID_E(source_srv, "[SOURCE][SRV][AT] register handler fail status = %02x", 1, ret);
    }
}

typedef struct {
    bt_source_srv_call_index_t   call_index;
    bt_source_srv_call_state_t   call_state;
} bt_source_srv_bqb_call_context_t;

#define BT_SOURCE_SRV_BQB_CALL_MAX_NUM   2

static bt_source_srv_bqb_call_context_t g_bqb_call_context[BT_SOURCE_SRV_BQB_CALL_MAX_NUM
                                                          ] = {0};

static bt_source_srv_bqb_call_context_t *bt_source_srv_bqb_call_get_free_context(void)
{
    bt_source_srv_bqb_call_context_t *context = NULL;
    for (uint32_t i = 0; i < BT_SOURCE_SRV_BQB_CALL_MAX_NUM
            ; i++) {
        if (g_bqb_call_context[i].call_state == BT_SOURCE_SRV_CALL_STATE_NONE) {
            context = &g_bqb_call_context[i];
            break;
        }
    }
    LOG_MSGID_I(source_srv, "[SOURCE][SRV][BQB] get free call bqb context = %02x", 1, context);
    return context;
}

static bt_source_srv_bqb_call_context_t *bt_source_srv_bqb_call_find_context_by_new_index(void)
{
    bt_source_srv_bqb_call_context_t *context = NULL;
    for (uint32_t i = 0; i < BT_SOURCE_SRV_BQB_CALL_MAX_NUM
            ; i++) {
        if ((g_bqb_call_context[i].call_index == 0) && (g_bqb_call_context[i].call_state != BT_SOURCE_SRV_CALL_STATE_NONE)) {
            context = &g_bqb_call_context[i];
            break;
        }
    }
    LOG_MSGID_I(source_srv, "[SOURCE][SRV][BQB] find call bqb context = %02x by new index", 1, context);
    return context;
}

static bt_source_srv_bqb_call_context_t *bt_source_srv_bqb_call_find_context_by_index(bt_source_srv_call_index_t call_index)
{
    bt_source_srv_bqb_call_context_t *context = NULL;
    for (uint32_t i = 0; i < BT_SOURCE_SRV_BQB_CALL_MAX_NUM; i++) {
        if (g_bqb_call_context[i].call_index == call_index) {
            context = &g_bqb_call_context[i];
            break;
        }
    }
    LOG_MSGID_I(source_srv, "[SOURCE][SRV][BQB] find call bqb context = %02x by  index = %02x", 2, context, call_index);
    return context;
}

static void bt_source_srv_bqb_call_change( bt_source_srv_call_state_t call_state, bt_source_srv_call_index_t call_index)
{
    LOG_MSGID_I(source_srv, "[SOURCE][SRV][BQB] call change = %02x, call index = %02x", 2, call_state, call_index);
    bt_source_srv_bqb_call_context_t *call_bqb_context = NULL;
    if (call_index == BT_SOURCE_SRV_CALL_INVALID_INDEX) {
        call_bqb_context = bt_source_srv_bqb_call_get_free_context();
    } else {
        call_bqb_context = bt_source_srv_bqb_call_find_context_by_index(call_index);
    }

    if (call_bqb_context == NULL) {
        return;
    }

    call_bqb_context->call_state = call_state;
}

static void bt_source_srv_bqb_call_change_notify(bt_source_srv_bqb_call_context_t *bqb_call_context, bt_source_srv_call_index_t call_index, bt_source_srv_call_state_t call_state)
{
    bqb_call_context->call_state = call_state;

    bt_source_srv_call_state_change_t call_change = {
        .type = BT_SOURCE_SRV_TYPE_HFP,
    };
    call_change.hfp_call_change.index = call_index;
    call_change.hfp_call_change.state = call_state;
    call_change.hfp_call_change.mpty = BT_SOURCE_SRV_HFP_CALL_MPTY_INACTIVE;
    bt_source_srv_send_action(BT_SOURCE_SRV_ACTION_CALL_STATE_CHANGE, &call_change, sizeof(bt_source_srv_call_state_change_t));
}

void bt_source_srv_bqb_event_callback(bt_source_srv_event_t event_id, void *parameter, uint32_t length)
{
    LOG_MSGID_I(source_srv, "[SOURCE][SRV][BQB] event callback = %02x", 1, event_id);
    switch (event_id) {
        case BT_SOURCE_SRV_EVENT_CALL_INDEX_IND: {
            bt_source_srv_call_index_ind_t *call_index = (bt_source_srv_call_index_ind_t *)parameter;
            bt_source_srv_bqb_call_context_t *call_bqb_context = bt_source_srv_bqb_call_find_context_by_new_index();
            if (call_bqb_context == NULL) {
                break;
            }
            LOG_MSGID_E(source_srv, "[SOURCE][SRV][BQB] new call index = %02x", 1, call_index->index);
            call_bqb_context->call_index = call_index->index;
        }
        break;
        case BT_SOURCE_SRV_EVENT_ACCEPT_CALL: {
            bt_source_srv_accept_call_t *accept_call = (bt_source_srv_accept_call_t *)parameter;
            bt_source_srv_bqb_call_context_t *call_bqb_context = bt_source_srv_bqb_call_find_context_by_index(accept_call->index);
            if (call_bqb_context == NULL) {
                break;
            }

            if ((call_bqb_context->call_state == BT_SOURCE_SRV_CALL_STATE_INCOMING) || (call_bqb_context->call_state == BT_SOURCE_SRV_CALL_STATE_WAITING)) {
                bt_source_srv_bqb_call_change_notify(call_bqb_context, accept_call->index, BT_SOURCE_SRV_CALL_STATE_ACTIVE);
            } else {
                LOG_MSGID_E(source_srv, "[SOURCE][SRV][BQB] accept call state error = %02x", 1, call_bqb_context->call_state);
            }
        }
        break;
        case BT_SOURCE_SRV_EVENT_REJECT_CALL: {
            bt_source_srv_reject_call_t *reject_call = (bt_source_srv_reject_call_t *)parameter;
            bt_source_srv_bqb_call_context_t *call_bqb_context = bt_source_srv_bqb_call_find_context_by_index(reject_call->index);
            if (call_bqb_context == NULL) {
                break;
            }

            if ((call_bqb_context->call_state == BT_SOURCE_SRV_CALL_STATE_INCOMING) || (call_bqb_context->call_state == BT_SOURCE_SRV_CALL_STATE_WAITING)) {
                bt_source_srv_bqb_call_change_notify(call_bqb_context, reject_call->index, BT_SOURCE_SRV_CALL_STATE_NONE);
            } else {
                LOG_MSGID_E(source_srv, "[SOURCE][SRV][BQB] reject call state error = %02x", 1, call_bqb_context->call_state);
            }
        }
        break;
        case BT_SOURCE_SRV_EVENT_TERMINATE_CALL: {
            bt_source_srv_terminate_call_t *terminate_call = (bt_source_srv_terminate_call_t *)parameter;
            bt_source_srv_bqb_call_context_t *call_bqb_context = bt_source_srv_bqb_call_find_context_by_index(terminate_call->index);
            if (call_bqb_context == NULL) {
                break;
            }

            if ((call_bqb_context->call_state == BT_SOURCE_SRV_CALL_STATE_ACTIVE) || (call_bqb_context->call_state == BT_SOURCE_SRV_CALL_STATE_LOCAL_HELD)) {
                bt_source_srv_bqb_call_change_notify(call_bqb_context, terminate_call->index, BT_SOURCE_SRV_CALL_STATE_NONE);
            } else {
                LOG_MSGID_E(source_srv, "[SOURCE][SRV][BQB] terminate call state error = %02x", 1, call_bqb_context->call_state);
            }
        }
        break;
        case BT_SOURCE_SRV_EVENT_HOLD: {
            bt_source_srv_hold_t *hold_call = (bt_source_srv_hold_t *)parameter;
            bt_source_srv_bqb_call_context_t *call_bqb_context = bt_source_srv_bqb_call_find_context_by_index(hold_call->index);
            if (call_bqb_context == NULL) {
                break;
            }

            if (call_bqb_context->call_state == BT_SOURCE_SRV_CALL_STATE_ACTIVE) {
                bt_source_srv_bqb_call_change_notify(call_bqb_context, hold_call->index, BT_SOURCE_SRV_CALL_STATE_LOCAL_HELD);
            } else {
                LOG_MSGID_E(source_srv, "[SOURCE][SRV][BQB] terminate call state error = %02x", 1, call_bqb_context->call_state);
            }
        }
        break;
        case BT_SOURCE_SRV_EVENT_UNHOLD: {
            bt_source_srv_unhold_t *unhold_call = (bt_source_srv_unhold_t *)parameter;
            bt_source_srv_bqb_call_context_t *call_bqb_context = bt_source_srv_bqb_call_find_context_by_index(unhold_call->index);
            if (call_bqb_context == NULL) {
                break;
            }

            if (call_bqb_context->call_state == BT_SOURCE_SRV_CALL_STATE_LOCAL_HELD) {
                bt_source_srv_bqb_call_change_notify(call_bqb_context, unhold_call->index, BT_SOURCE_SRV_CALL_STATE_ACTIVE);
            } else {
                LOG_MSGID_E(source_srv, "[SOURCE][SRV][BQB] terminate call state error = %02x", 1, call_bqb_context->call_state);
            }
        }
        break;
        default:
            break;
    }
}
