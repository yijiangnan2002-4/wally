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

#ifdef MTK_PORT_SERVICE_BT_ENABLE


#include "bt_type.h"
#include "bt_sdp.h"
#include "bt_spp.h"
#include "bt_callback_manager.h"
#include "spp_air.h"
#include "spp_air_interface.h"
#include "syslog.h"
#include "bt_uuid.h"
#include "atci.h"
#include "bt_connection_manager.h"

extern void vPortFree(void *);
extern void *pvPortMalloc(size_t);

log_create_module(SPPAIR_REC, PRINT_LEVEL_INFO);

#define BT_SPP_AIR_SERVER_ID  0x15 /**< the air server ID.*/
#define BT_SPP_AIR_UUID    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF /**< the Spp server UUID.*/
//#define BT_SPP_AIR_UUID 0x00,0x00,0x11,0x01,0x00,0x00,0x10,0x00,0x80,0x00,0x00,0x80,0x5F,0x9B,0x34,0xFB/**< just for test. */

static atci_status_t bt_spp_air_at_cmd_hdl(atci_parse_cmd_param_t *parse_cmd);

static atci_cmd_hdlr_item_t bt_spp_air_at_cmd[] = {
    {
        .command_head = "AT+SPPAIR",    /**< AT command string. */
        .command_hdlr = bt_spp_air_at_cmd_hdl,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
};

/****************************************************************************
 *
 * ROMable data
 * SPP Airoha service record
 ****************************************************************************/
static uint8_t bt_spp_air_service_class_id[] = {
    BT_SPP_SDP_ATTRIBUTE_UUID_LENGTH,
    BT_SPP_SDP_ATTRIBUTE_UUID(BT_SPP_AIR_UUID)
};

static const uint8_t bt_spp_air_protocol_descriptor_list[] = {
    BT_SPP_SDP_ATTRIBUTE_PROTOCOL_DESCRIPTOR(BT_SPP_AIR_SERVER_ID)
};

static const uint8_t bt_app_serial_port_profile_descriptor_list[] = {
    BT_SDP_ATTRIBUTE_HEADER_8BIT(8),
    BT_SDP_ATTRIBUTE_HEADER_8BIT(6),
    BT_SDP_UUID_16BIT(BT_SDP_SERVICE_CLASS_SERIAL_PORT),
    BT_SDP_UINT_16BIT(0x0102)
};

static const uint8_t bt_spp_air_browse_group[] = {
    BT_SPP_SDP_ATTRIBUTE_PUBLIC_BROWSE_GROUP
};

static const uint8_t bt_spp_air_language[] = {
    BT_SPP_SDP_ATTRIBUTE_LANGUAGE
};

static const uint8_t bt_spp_air_service_name[] = {
    BT_SPP_SDP_ATTRIBUTE_SIZE_OF_SERVICE_NAME(10),
    'A', 'i', 'r', 'o', 'h', 'a', '_', 'A', 'P', 'P'
};

static const bt_sdps_attribute_t bt_spp_air_sdp_attributes[] = {
    /* Service Class ID List attribute */
    BT_SPP_SDP_ATTRIBUTE_SERVICE_CLASS_ID_LIST(bt_spp_air_service_class_id),
    /* Protocol Descriptor List attribute */
    BT_SPP_SDP_ATTRIBUTE_PROTOCOL_DESC_LIST(bt_spp_air_protocol_descriptor_list),
    BT_SDP_ATTRIBUTE(BT_SDP_ATTRIBUTE_ID_BT_PROFILE_DESC_LIST, bt_app_serial_port_profile_descriptor_list),
    /* Public Browse Group Service */
    BT_SPP_SDP_ATTRIBUTE_BROWSE_GROUP_LIST(bt_spp_air_browse_group),
    /* Language Base ID List attribute */
    BT_SPP_SDP_ATTRIBUTE_LANGUAGE_BASE_LIST(bt_spp_air_language),
    /* Serial Port Profile Service Name */
    BT_SPP_SDP_ATTRIBUTE_SERVICE_NAME(bt_spp_air_service_name)
};


static const bt_sdps_record_t bt_spp_air_sdp_record = {
    .attribute_list_length = sizeof(bt_spp_air_sdp_attributes),
    .attribute_list = bt_spp_air_sdp_attributes,
};

bt_status_t bt_spp_air_sdp_event_callback_register(void)
{
    bt_uuid_t uuid;

    if (SPP_AIR_STATUS_SUCCESS == spp_air_get_uuid((uint8_t *)&uuid)) {
        LOG_HEXDUMP_I(SPPAIR_REC, "using spp uuid:", &uuid, sizeof(bt_uuid_t));
        memcpy((void *)(bt_spp_air_service_class_id + 3), &uuid, sizeof(bt_uuid_t));
    }

    bt_callback_manager_add_sdp_customized_record(&bt_spp_air_sdp_record);

    /* For BQB test to register AT cmd callback. */
    atci_register_handler(bt_spp_air_at_cmd, sizeof(bt_spp_air_at_cmd) / sizeof(atci_cmd_hdlr_item_t));
    return BT_STATUS_SUCCESS;
}

bt_status_t bt_spp_air_sdp_event_update_record(bt_uuid_t *uuid128)
{
    if (uuid128 == NULL) {
        return BT_STATUS_FAIL;
    }
    LOG_HEXDUMP_I(SPPAIR_REC, "update spp uuid:", uuid128, sizeof(bt_uuid_t));
    memcpy((void *)(bt_spp_air_service_class_id + 3), uuid128, sizeof(bt_uuid_t));

    return bt_sdps_notify_database_update();
}
#endif /*MTK_PORT_SERVICE_BT_ENABLE*/

static void bt_spp_air_cmd_copy_str_to_addr(uint8_t *addr, const char *str)
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
            LOG_MSGID_E(SPPAIR, "[SPP][AIR] addr convert fail", 0);
        }
        addr[5 - i] = (uint8_t) value;
    }
}

static atci_status_t bt_spp_air_at_cmd_hdl(atci_parse_cmd_param_t *parse_cmd)
{
    //atci_response_t response = {{0}, 0, ATCI_RESPONSE_FLAG_APPEND_ERROR};
    atci_response_t *response = pvPortMalloc(sizeof(atci_response_t));
    if (NULL == response) {
        return ATCI_STATUS_ERROR;
    }
    memset(response, 0, sizeof(atci_response_t));
    bt_status_t status = BT_STATUS_FAIL;
    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
    static uint32_t air_bqb_handle = 0;

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "ADD", 3)) {
                status = bt_callback_manager_add_sdp_customized_record(&bt_spp_air_sdp_record);
                if (status == BT_STATUS_SUCCESS) {
                    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                }
            } else if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "REMOVE", 6)) {
                status = bt_callback_manager_remove_sdp_customized_record(&bt_spp_air_sdp_record);
                if (status == BT_STATUS_SUCCESS) {
                    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                }
            } else if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "CONNECT", 7)) {
                bt_bd_addr_t remote_address = {0};
                static uint8_t air_uuid128_default[16] = {
                    0x00,0x00,0x11,0x01,0x00,0x00,0x10,0x00,0x80,0x00,0x00,0x80,0x5F,0x9B,0x34,0xFB
                };
                char *address_str = (char *)(parse_cmd->string_ptr + parse_cmd->name_len + 1 + strlen("CONNECT,"));
                bt_spp_air_cmd_copy_str_to_addr((void *)&remote_address, address_str);
                bt_status_t air_status = spp_air_connect_int(&air_bqb_handle, &remote_address, air_uuid128_default);
                if (air_status == BT_STATUS_SUCCESS) {
                    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                }
            } else if (0 == memcmp(parse_cmd->string_ptr + parse_cmd->name_len + 1, "DISCONNECT", 10)) {
                spp_air_status_t air_status = spp_air_disconnect(air_bqb_handle);
                if (air_status == SPP_AIR_STATUS_SUCCESS) {
                    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                }
            }
        }
        break;
        default:
            break;
    }

    response->response_len = strlen((char *)response->response_buf);
    atci_send_response(response);
    vPortFree(response);
    return ATCI_STATUS_OK;
}
