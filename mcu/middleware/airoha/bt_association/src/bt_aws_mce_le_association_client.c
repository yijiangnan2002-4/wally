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


#include "bt_aws_mce_le_association.h"
#include "bt_aws_mce_le_association_internal.h"

#include "bt_gap_le.h"
#include "bt_hci.h"
#include "bt_gattc.h"
#include "bt_gatts.h"

#include "syslog.h"

#define BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_IDLE                     0x0
#define BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_GET_AGENT_ADDRESS        0x1
#define BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_GET_KEY                  0x2
#define BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_WRITE_CLIENT_ADDRESS     0x3
#define BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_WRITE_AUDIO_LAT          0x4
#define BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_WRITE_VOICE_LAT          0x5
#define BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_GET_NUMBER               0x6
typedef uint8_t bt_aws_mce_le_association_client_state_t;

#define BT_AWS_MCE_LE_ASSOCIATION_CLIENT_WRITE_BUF                      (BT_ATT_DEFAULT_MTU + 2)

typedef struct {
    bt_aws_mce_le_association_state_t                                   state;
    bt_aws_mce_le_association_client_state_t                            sub_state;
    bt_handle_t                                                         handle;
    bt_aws_mce_le_association_pair_mode_t                               mode;
    bt_aws_mce_le_association_service_t                                 service;
    bt_aws_mce_le_association_client_info_t                             client;
    bt_aws_mce_le_association_agent_pair_cnf_t                          pair_cnf;
} bt_aws_mce_le_association_client_context_t;

static bt_aws_mce_le_association_client_context_t bt_assocation_client_ctx = {0};



/**================================================================================*/
/**                                   Internal API                                 */
/**================================================================================*/
static bool bt_aws_mce_le_association_check_char_type(bt_aws_mce_le_association_char_type_t type)
{
    if ((type == BT_AWS_MCE_LE_ASSOCIATION_CHAR_AGENT_ADDR)
        || (type == BT_AWS_MCE_LE_ASSOCIATION_CHAR_SECRET_KEY)
        || (type == BT_AWS_MCE_LE_ASSOCIATION_CHAR_CLIENT_ADDR)
        || (type == BT_AWS_MCE_LE_ASSOCIATION_CHAR_CLIENT_AUDIO_LATENCY)
        || (type == BT_AWS_MCE_LE_ASSOCIATION_CHAR_CLIENT_VOICE_LATENCY)
        || (type == BT_AWS_MCE_LE_ASSOCIATION_CHAR_CLIENT_NO)
        || (type == BT_AWS_MCE_LE_ASSOCIATION_CHAR_CUSTOM_READ_DATA)
        || (type == BT_AWS_MCE_LE_ASSOCIATION_CHAR_CUSTOM_WRITE_DATA)) {
        return true;
    } else {
        return false;
    }
}

static bt_aws_mce_le_association_char_t *bt_aws_mce_le_association_get_char_by_type(bt_aws_mce_le_association_service_t *service, bt_aws_mce_le_association_char_type_t type)
{
    if (service == NULL) {
        return NULL;
    }

    for (int i = 0; i < service->char_count; i++) {
        if (service->chara[i].type == type) {
            return &service->chara[i];
        }
    }
    return NULL;
}

static void bt_aws_mce_le_association_client_handle_error_and_notify_user(bt_aws_mce_le_association_client_context_t *client_ctx)
{
    memset(client_ctx, 0, sizeof(bt_aws_mce_le_association_client_context_t));
    bt_aws_mce_le_association_event_callback(BT_AWS_MCE_LE_ASSOCIATION_EVENT_AGENT_PAIR_CNF, BT_STATUS_FAIL, NULL, 0);
}

static bt_status_t bt_aws_mce_le_association_client_idle_handler(bt_aws_mce_le_association_client_context_t *client_ctx, bt_aws_mce_le_association_client_state_t next_state)
{
    bt_status_t bt_status = BT_STATUS_FAIL;
    if (next_state == BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_GET_AGENT_ADDRESS) {
        bt_aws_mce_le_association_char_t *chara = bt_aws_mce_le_association_get_char_by_type(&client_ctx->service,
                                                                                             BT_AWS_MCE_LE_ASSOCIATION_CHAR_AGENT_ADDR);
        if (chara != NULL) {
            BT_GATTC_NEW_READ_CHARC_REQ(req, chara->value_handle);
            bt_status = bt_gattc_read_charc(client_ctx->handle, &req);
            if (bt_status == BT_STATUS_SUCCESS || bt_status == BT_STATUS_PENDING) {
                client_ctx->sub_state = BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_GET_AGENT_ADDRESS;
                bt_status = BT_STATUS_SUCCESS;
            }
        }
    }
    return bt_status;
}

static bt_status_t bt_aws_mce_le_association_client_agent_addr_handler(bt_aws_mce_le_association_client_context_t *client_ctx, bt_aws_mce_le_association_client_state_t next_state)
{
    bt_status_t bt_status = BT_STATUS_FAIL;
    if (next_state == BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_GET_KEY) {
        bt_aws_mce_le_association_char_t *chara = bt_aws_mce_le_association_get_char_by_type(&client_ctx->service,
                                                                                             BT_AWS_MCE_LE_ASSOCIATION_CHAR_SECRET_KEY);
        if (chara != NULL) {
            BT_GATTC_NEW_READ_CHARC_REQ(req, chara->value_handle);
            bt_status = bt_gattc_read_charc(client_ctx->handle, &req);
            if (bt_status == BT_STATUS_SUCCESS || bt_status == BT_STATUS_PENDING) {
                client_ctx->sub_state = BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_GET_KEY;
                bt_status = BT_STATUS_SUCCESS;
            }
        }
    }
    return bt_status;
}

static bt_status_t bt_aws_mce_le_association_client_get_key_handler(bt_aws_mce_le_association_client_context_t *client_ctx, bt_aws_mce_le_association_client_state_t next_state)
{
    bt_status_t bt_status = BT_STATUS_FAIL;
    if (next_state == BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_WRITE_CLIENT_ADDRESS) {
        bt_aws_mce_le_association_char_t *chara = bt_aws_mce_le_association_get_char_by_type(&client_ctx->service,
                                                                                             BT_AWS_MCE_LE_ASSOCIATION_CHAR_CLIENT_ADDR);
        if (chara != NULL) {
            if (client_ctx->mode == BT_AWS_MCE_LE_ASSOCIATION_PAIR_MODE_NORMAL) {
                /* Pair ind should not write client addr */
            } else {
                uint8_t buff[BT_AWS_MCE_LE_ASSOCIATION_CLIENT_WRITE_BUF] = {0};
                BT_GATTC_NEW_WRITE_CHARC_REQ(req, buff, chara->value_handle,   &client_ctx->client.address, sizeof(bt_bd_addr_t))
                bt_status = bt_gattc_write_charc(client_ctx->handle, &req);
                if (bt_status == BT_STATUS_SUCCESS || bt_status == BT_STATUS_PENDING) {
                    client_ctx->sub_state = BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_WRITE_CLIENT_ADDRESS;
                    bt_status = BT_STATUS_SUCCESS;
                }
            }
        }
    } else if (next_state == BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_WRITE_AUDIO_LAT) {
        bt_aws_mce_le_association_char_t *chara = bt_aws_mce_le_association_get_char_by_type(&client_ctx->service,
                                                                                             BT_AWS_MCE_LE_ASSOCIATION_CHAR_CLIENT_AUDIO_LATENCY);
        if (chara != NULL) {
            if (client_ctx->mode == BT_AWS_MCE_LE_ASSOCIATION_PAIR_MODE_NUMBERED) {
                /* Pair ind should not write audio latency */
            } else {
                uint8_t buff[BT_AWS_MCE_LE_ASSOCIATION_CLIENT_WRITE_BUF] = {0};
                BT_GATTC_NEW_WRITE_CHARC_REQ(req, buff, chara->value_handle, &client_ctx->client.audio_latency, sizeof(uint16_t));
                bt_status = bt_gattc_write_charc(client_ctx->handle, &req);
                if (bt_status == BT_STATUS_SUCCESS || bt_status == BT_STATUS_PENDING) {
                    client_ctx->sub_state = BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_WRITE_AUDIO_LAT;
                    bt_status = BT_STATUS_SUCCESS;
                }
            }
        }
    }
    return bt_status;
}

static bt_status_t bt_aws_mce_le_association_client_write_client_addr_handler(bt_aws_mce_le_association_client_context_t *client_ctx, bt_aws_mce_le_association_client_state_t next_state)
{
    bt_status_t bt_status = BT_STATUS_FAIL;
    if (next_state == BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_WRITE_AUDIO_LAT) {
        bt_aws_mce_le_association_char_t *chara = bt_aws_mce_le_association_get_char_by_type(&client_ctx->service,
                                                                                             BT_AWS_MCE_LE_ASSOCIATION_CHAR_CLIENT_AUDIO_LATENCY);
        if (chara != NULL) {
            if (client_ctx->mode == BT_AWS_MCE_LE_ASSOCIATION_PAIR_MODE_NORMAL) {
                /* Wrongly status */
            } else {
                uint8_t buff[BT_AWS_MCE_LE_ASSOCIATION_CLIENT_WRITE_BUF] = {0};
                BT_GATTC_NEW_WRITE_CHARC_REQ(req, buff, chara->value_handle, &client_ctx->client.audio_latency, sizeof(uint16_t));
                bt_status = bt_gattc_write_charc(client_ctx->handle, &req);
                if (bt_status == BT_STATUS_SUCCESS || bt_status == BT_STATUS_PENDING) {
                    client_ctx->sub_state = BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_WRITE_AUDIO_LAT;
                    bt_status = BT_STATUS_SUCCESS;
                }
            }
        }
    }
    return bt_status;
}

static bt_status_t bt_aws_mce_le_association_client_write_audio_lat_handler(bt_aws_mce_le_association_client_context_t *client_ctx, bt_aws_mce_le_association_client_state_t next_state)
{
    bt_status_t bt_status = BT_STATUS_FAIL;
    if (next_state == BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_WRITE_VOICE_LAT) {
        bt_aws_mce_le_association_char_t *chara = bt_aws_mce_le_association_get_char_by_type(&client_ctx->service,
                                                                                             BT_AWS_MCE_LE_ASSOCIATION_CHAR_CLIENT_VOICE_LATENCY);
        if (chara != NULL) {
            uint8_t buff[BT_AWS_MCE_LE_ASSOCIATION_CLIENT_WRITE_BUF] = {0};
            BT_GATTC_NEW_WRITE_CHARC_REQ(req, buff, chara->value_handle, &client_ctx->client.voice_latency, sizeof(uint16_t));
            bt_status = bt_gattc_write_charc(client_ctx->handle, &req);
            if (bt_status == BT_STATUS_SUCCESS || bt_status == BT_STATUS_PENDING) {
                client_ctx->sub_state = BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_WRITE_VOICE_LAT;
                bt_status = BT_STATUS_SUCCESS;
            }
        }
    }
    return bt_status;
}

static bt_status_t bt_aws_mce_le_association_client_write_voice_lat_handler(bt_aws_mce_le_association_client_context_t *client_ctx, bt_aws_mce_le_association_client_state_t next_state)
{
    bt_status_t bt_status = BT_STATUS_FAIL;
    if (next_state == BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_GET_NUMBER) {
        bt_aws_mce_le_association_char_t *chara = bt_aws_mce_le_association_get_char_by_type(&client_ctx->service,
                                                                                             BT_AWS_MCE_LE_ASSOCIATION_CHAR_CLIENT_NO);
        if (chara != NULL) {
            if (client_ctx->mode == BT_AWS_MCE_LE_ASSOCIATION_PAIR_MODE_NORMAL) {
                /* Don't need get number for normal mode */
            } else {
                BT_GATTC_NEW_READ_CHARC_REQ(req, chara->value_handle);
                bt_status = bt_gattc_read_charc(client_ctx->handle, &req);
                if (bt_status == BT_STATUS_SUCCESS || bt_status == BT_STATUS_PENDING) {
                    client_ctx->sub_state = BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_GET_NUMBER;
                    bt_status = BT_STATUS_SUCCESS;
                }
            }
        }
    }
    return bt_status;
}

static bt_status_t bt_aws_mce_le_association_client_get_number_handler(bt_aws_mce_le_association_client_context_t *client_ctx,
                                                                       bt_aws_mce_le_association_client_state_t next_state)
{
    /* Currently this status is no useful */
    return BT_STATUS_SUCCESS;
}

bt_status_t bt_aws_mce_le_association_client_pairing_state_handler(bt_aws_mce_le_association_client_state_t next_state)
{
    bt_aws_mce_le_association_client_context_t *client_ctx = &bt_assocation_client_ctx;
    bt_status_t bt_status = BT_STATUS_FAIL;

    LOG_MSGID_I(common, "[LE_ASS_MID] state_handler, state=%d sub_state=%d->%d",
                3, client_ctx->state, client_ctx->sub_state, next_state);
    if (client_ctx->state == BT_AWS_MCE_LE_ASSOCIATION_STATE_IDLE) {
        return BT_STATUS_FAIL;
    }

    switch (client_ctx->sub_state) {
        case BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_IDLE: {
            bt_status = bt_aws_mce_le_association_client_idle_handler(client_ctx, next_state);
            break;
        }
        case BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_GET_AGENT_ADDRESS: {
            bt_status = bt_aws_mce_le_association_client_agent_addr_handler(client_ctx, next_state);
            break;
        }
        case BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_GET_KEY: {
            bt_status = bt_aws_mce_le_association_client_get_key_handler(client_ctx, next_state);
            break;
        }
        case BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_WRITE_CLIENT_ADDRESS: {
            bt_status = bt_aws_mce_le_association_client_write_client_addr_handler(client_ctx, next_state);
            break;
        }
        case BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_WRITE_AUDIO_LAT: {
            bt_status = bt_aws_mce_le_association_client_write_audio_lat_handler(client_ctx, next_state);
            break;
        }
        case BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_WRITE_VOICE_LAT: {
            bt_status = bt_aws_mce_le_association_client_write_voice_lat_handler(client_ctx, next_state);
            break;
        }
        case BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_GET_NUMBER: {
            bt_status = bt_aws_mce_le_association_client_get_number_handler(client_ctx, next_state);
            break;
        }
        default: {
            break;
        }
    }
    if (bt_status != BT_STATUS_SUCCESS) {
        LOG_MSGID_E(common, "[LE_ASS_MID] state_handler, bt_status=0x%08X", 1, bt_status);
    }
    return bt_status;
}



/**================================================================================*/
/**                                Default Implement                               */
/**================================================================================*/
bt_status_t bt_aws_mce_le_association_client_event_handler(bt_msg_type_t msg, bt_status_t status, void *buffer)
{
    bt_aws_mce_le_association_client_context_t *client_ctx = &bt_assocation_client_ctx;
    bt_status_t bt_status = BT_STATUS_SUCCESS;

    switch (msg) {
        case BT_GAP_LE_DISCONNECT_IND: {
            bt_hci_evt_disconnect_complete_t *disconn = (bt_hci_evt_disconnect_complete_t *) buffer;
            if (disconn->connection_handle == client_ctx->handle
                && client_ctx->state == BT_AWS_MCE_LE_ASSOCIATION_STATE_PAIRING) {
                /* Enter IDLE mode and notify user */
                bt_status = BT_STATUS_FAIL;
                LOG_MSGID_I(common, "[LE_ASS_MID] client_event_handler, Disconnect state=%d sub_state=%d",
                            2, client_ctx->state, client_ctx->sub_state);
            }
            break;
        }

        case BT_GATTC_READ_CHARC: {
            bt_gattc_read_rsp_t *read_char = (bt_gattc_read_rsp_t *)buffer;
            uint16_t length = read_char->length - 1;
            if (read_char->connection_handle != client_ctx->handle) {
                break;
            }

            LOG_MSGID_I(common, "[LE_ASS_MID] client_event_handler, READ_CHARC state=%d sub_state=%d length=%d status=0x%08X",
                        4, client_ctx->state, client_ctx->sub_state, length, status);
            if (client_ctx->state == BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_IDLE) {
                break;
            } else if (status != BT_STATUS_SUCCESS) {
                bt_status = BT_STATUS_FAIL;
                break;
            }

            if (client_ctx->state == BT_AWS_MCE_LE_ASSOCIATION_STATE_CUSTOM_INTERACTION) {
                uint8_t temp_data[BT_AWS_MCE_LE_ASSOCIATION_MAX_CUSTOM_DATA_LEN] = {0};
                bt_aws_mce_le_association_read_custom_data_cnf_t cnf = {0};
                cnf.handle = client_ctx->handle;
                if (length >= BT_AWS_MCE_LE_ASSOCIATION_MAX_CUSTOM_DATA_LEN) {
                    length = BT_AWS_MCE_LE_ASSOCIATION_MAX_CUSTOM_DATA_LEN;
                }
                memcpy(temp_data, read_char->att_rsp->attribute_value, length);
                cnf.info.data = temp_data;
                client_ctx->state = BT_AWS_MCE_LE_ASSOCIATION_STATE_IDLE;
                bt_aws_mce_le_association_event_callback(BT_AWS_MCE_LE_ASSOCIATION_EVENT_READ_CUSTOM_DATA_CNF, BT_STATUS_SUCCESS, &cnf, sizeof(bt_aws_mce_le_association_read_custom_data_cnf_t));
                break;
            }

            switch (client_ctx->sub_state) {
                case BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_GET_AGENT_ADDRESS: {
                    if (length != sizeof(bt_bd_addr_t)) {
                        bt_status = BT_STATUS_FAIL;
                        break;
                    }
                    memcpy(&client_ctx->pair_cnf.info.address, read_char->att_rsp->attribute_value, length);
                    bt_status = bt_aws_mce_le_association_client_pairing_state_handler(BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_GET_KEY);
                    break;
                }

                case BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_GET_KEY: {
                    if (length != BT_ASSOCIATION_SECRET_KEY_LEN) {
                        bt_status = BT_STATUS_FAIL;
                        break;
                    }

                    memcpy(client_ctx->pair_cnf.info.secret_key, read_char->att_rsp->attribute_value, length);
                    if (client_ctx->mode == BT_AWS_MCE_LE_ASSOCIATION_PAIR_MODE_NORMAL) {
                        bt_status = bt_aws_mce_le_association_client_pairing_state_handler(BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_WRITE_AUDIO_LAT);
                    } else {
                        bt_status = bt_aws_mce_le_association_client_pairing_state_handler(BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_WRITE_CLIENT_ADDRESS);
                    }
                    break;
                }

                case BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_GET_NUMBER: {
                    if (length != sizeof(uint16_t)) {
                        bt_status = BT_STATUS_FAIL;
                        break;
                    }

                    memcpy(&client_ctx->pair_cnf.number, read_char->att_rsp->attribute_value, length);
                    client_ctx->state = BT_AWS_MCE_LE_ASSOCIATION_STATE_IDLE;
                    client_ctx->sub_state = BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_IDLE;
                    client_ctx->pair_cnf.handle = client_ctx->handle;
                    bt_aws_mce_le_association_event_callback(BT_AWS_MCE_LE_ASSOCIATION_EVENT_AGENT_PAIR_CNF,
                                                             BT_STATUS_SUCCESS, &client_ctx->pair_cnf,
                                                             sizeof(bt_aws_mce_le_association_agent_pair_cnf_t));
                    break;
                }

                default: {
                    break;
                }
            }
            break;
        }

        case BT_GATTC_WRITE_CHARC: {
            bt_gattc_write_rsp_t *write_rsp = (bt_gattc_write_rsp_t *)buffer;
            if (write_rsp->connection_handle != client_ctx->handle) {
                break;
            }

            LOG_MSGID_I(common, "[LE_ASS_MID] client_event_handler, WRITE_CHARC state=%d sub_state=%d status=0x%08X",
                        3, client_ctx->state, client_ctx->sub_state, status);
            if (client_ctx->state == BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_IDLE) {
                break;
            } else if (status != BT_STATUS_SUCCESS) {
                bt_status = BT_STATUS_FAIL;
                break;
            }

            if (client_ctx->state == BT_AWS_MCE_LE_ASSOCIATION_STATE_CUSTOM_INTERACTION) {
                bt_aws_mce_le_association_write_custom_data_cnf_t data_cnf = {0};
                data_cnf.handle = client_ctx->handle;
                client_ctx->state = BT_AWS_MCE_LE_ASSOCIATION_STATE_IDLE;
                bt_aws_mce_le_association_event_callback(BT_AWS_MCE_LE_ASSOCIATION_EVENT_WRITE_CUSTOM_DATA_CNF, BT_STATUS_SUCCESS, &data_cnf, sizeof(bt_aws_mce_le_association_write_custom_data_cnf_t));
                break;
            }

            switch (client_ctx->sub_state) {
                case BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_WRITE_CLIENT_ADDRESS: {
                    bt_status = bt_aws_mce_le_association_client_pairing_state_handler(BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_WRITE_AUDIO_LAT);
                    break;
                }
                case BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_WRITE_AUDIO_LAT: {
                    bt_status = bt_aws_mce_le_association_client_pairing_state_handler(BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_WRITE_VOICE_LAT);
                    break;
                }
                case BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_WRITE_VOICE_LAT: {
                    if (client_ctx->mode == BT_AWS_MCE_LE_ASSOCIATION_PAIR_MODE_NUMBERED) {
                        bt_status = bt_aws_mce_le_association_client_pairing_state_handler(BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_GET_NUMBER);
                    } else {
                        /* Number always be zero for normal mode. */
                        client_ctx->state = BT_AWS_MCE_LE_ASSOCIATION_STATE_IDLE;
                        client_ctx->sub_state = BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_IDLE;
                        client_ctx->pair_cnf.number = 0;
                        client_ctx->pair_cnf.handle = client_ctx->handle;
                        bt_aws_mce_le_association_event_callback(BT_AWS_MCE_LE_ASSOCIATION_EVENT_AGENT_PAIR_CNF, BT_STATUS_SUCCESS, &client_ctx->pair_cnf,  sizeof(bt_aws_mce_le_association_agent_pair_cnf_t));
                    }
                    break;
                }

                default:
                    break;
            }
        }
    }

    if (bt_status != BT_STATUS_SUCCESS) {
        bt_aws_mce_le_association_client_handle_error_and_notify_user(client_ctx);
        LOG_MSGID_E(common, "[LE_ASS_MID] client_handle_error, ret=0x%08X", 1, bt_status);
    }
    return bt_status;
}



/**================================================================================*/
/**                                   Public API                                   */
/**================================================================================*/
bt_status_t bt_aws_mce_le_association_client_start_pairing(bt_aws_mce_le_association_pair_agent_req_t *req)
{
    bt_aws_mce_le_association_client_context_t *client_ctx = &bt_assocation_client_ctx;
    if (req == NULL || req->handle == BT_HANDLE_INVALID
        || req->service == NULL || req->service->char_count == 0
        || req->client == NULL) {
        return BT_STATUS_LE_ASSOCIATION_PARAMETER_ERR;
    }

    for (int i = 0; i < req->service->char_count; i++) {
        if (!bt_aws_mce_le_association_check_char_type(req->service->chara[i].type)) {
            return BT_STATUS_LE_ASSOCIATION_PARAMETER_ERR;
        }
        memcpy(&client_ctx->service.chara[i], &req->service->chara[i], sizeof(bt_aws_mce_le_association_char_t));
    }

    client_ctx->handle = req->handle;
    client_ctx->mode = req->mode;
    client_ctx->service.char_count = req->service->char_count;
    client_ctx->state = BT_AWS_MCE_LE_ASSOCIATION_STATE_PAIRING;
    memcpy(&client_ctx->client, req->client, sizeof(bt_aws_mce_le_association_client_info_t));
    client_ctx->sub_state = BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_IDLE;
    return bt_aws_mce_le_association_client_pairing_state_handler(BT_AWS_MCE_LE_ASSOCIATION_CLIENT_STATE_GET_AGENT_ADDRESS);
}

bt_status_t bt_aws_mce_le_association_client_check_adv_data(void *buffer, uint16_t buffer_len, bt_firmware_type_t mode)
{
    bt_status_t bt_status = BT_STATUS_FAIL;
    uint8_t length = 0;
    uint8_t type = 0;
    uint8_t *buf = (uint8_t *)buffer;
    uint8_t *buffer_end = (uint8_t *)buffer + buffer_len;
    char adv_data[] = "AWS 1.0";
    LOG_HEXDUMP_I(common, "[LE_ASS_MID] client_check_adv_data", buf, buffer_len);

    // <len> + 0xFF + "AWS 1.0" + <mode> + <manufacturer>(string, max 10 bytes) + <version>(1 byte)
    if (buf != NULL && buf[0] != 0) {
        while (buf < buffer_end && buf[0] != 0) {
            length = *buf++;
            type = *buf;
            if (mode == BT_FIRMWARE_TYPE_SPEAKER || mode == BT_AWS_MCE_SRV_MODE_BROADCAST) {
                // BT_FIRMWARE_TYPE_SPEAKER
                mode = 0x01;
            }

            if (type == 0xFF) {
                buf++;
                bool aws_data_match = (memcmp(buf, adv_data, strlen(adv_data)) == 0);
                bool mode_match = (*(buf + strlen(adv_data)) == mode);
                bool manufacturer_match = (memcmp(buf + strlen(adv_data) + 1, bt_aws_mce_le_association_manufacturer, strlen(bt_aws_mce_le_association_manufacturer)) == 0);
                bool version_match = (*(buf + strlen(adv_data) + 1 + strlen(bt_aws_mce_le_association_manufacturer)) == bt_aws_mce_le_association_version);
                LOG_MSGID_I(common, "[LE_ASS_MID] %d %d %d %d", 4, aws_data_match, mode_match, manufacturer_match, version_match);
                if (aws_data_match && mode_match && manufacturer_match && version_match) {
                    bt_status = BT_STATUS_SUCCESS;
                    break;
                }
            }
            buf += length - 1;
        }
    }
    return bt_status;
}

bt_status_t bt_aws_mce_le_association_read_custom_data(bt_handle_t handle)
{
    bt_status_t bt_status = BT_STATUS_FAIL;
    bt_aws_mce_le_association_client_context_t *client_ctx = &bt_assocation_client_ctx;
    if (client_ctx == NULL || handle != client_ctx->handle) {
        return bt_status;
    }

    bt_aws_mce_le_association_char_t *chara = bt_aws_mce_le_association_get_char_by_type(&client_ctx->service,
                                                                                         BT_AWS_MCE_LE_ASSOCIATION_CHAR_CUSTOM_READ_DATA);
    if (chara != NULL) {
        BT_GATTC_NEW_READ_CHARC_REQ(req, chara->value_handle);
        bt_status = bt_gattc_read_charc(client_ctx->handle, &req);
        if (bt_status == BT_STATUS_SUCCESS || bt_status == BT_STATUS_PENDING) {
            client_ctx->state = BT_AWS_MCE_LE_ASSOCIATION_STATE_CUSTOM_INTERACTION;
            bt_status = BT_STATUS_SUCCESS;
        }
    }
    return bt_status;
}

bt_status_t bt_aws_mce_le_association_write_custom_data(bt_handle_t handle,  uint8_t *data, uint8_t len)
{
    bt_status_t bt_status = BT_STATUS_FAIL;
    bt_aws_mce_le_association_client_context_t *client_ctx = &bt_assocation_client_ctx;
    if (len > BT_AWS_MCE_LE_ASSOCIATION_MAX_CUSTOM_DATA_LEN) {
        bt_status = BT_STATUS_LE_ASSOCIATION_PARAMETER_ERR;
        return bt_status;
    }

    if (client_ctx == NULL || handle != client_ctx->handle) {
        return bt_status;
    }

    bt_aws_mce_le_association_char_t *chara = bt_aws_mce_le_association_get_char_by_type(&client_ctx->service,
                                                                                         BT_AWS_MCE_LE_ASSOCIATION_CHAR_CUSTOM_WRITE_DATA);
    if (chara == NULL) {
        return bt_status;
    }

    uint8_t buf[BT_AWS_MCE_LE_ASSOCIATION_CLIENT_WRITE_BUF] = {0};
    BT_GATTC_NEW_WRITE_CHARC_REQ(req, buf, chara->value_handle, data, len);
    bt_status = bt_gattc_write_charc(client_ctx->handle, &req);
    if (bt_status == BT_STATUS_SUCCESS || bt_status == BT_STATUS_PENDING) {
        client_ctx->state = BT_AWS_MCE_LE_ASSOCIATION_STATE_CUSTOM_INTERACTION;
        bt_status = BT_STATUS_SUCCESS;
    }
    return bt_status;
}
