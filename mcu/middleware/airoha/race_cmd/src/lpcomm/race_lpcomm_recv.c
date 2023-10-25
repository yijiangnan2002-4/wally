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

#include "race_cmd_feature.h"
#ifdef RACE_LPCOMM_ENABLE
#include "race_lpcomm_util.h"
#include "race_lpcomm_recv.h"
#include "race_lpcomm_agent.h"
#include "race_lpcomm_partner.h"

RACE_ERRCODE race_lpcomm_data_recv_hdl(race_lpcomm_packet_struct *packet, const race_lpcomm_data_recv_hdl_struct *data,
                                                      race_lpcomm_role_enum role, uint8_t device_id)
{
    /*RACE_LOG_MSGID_F("packet:%x, packet->process_id:%d, device_id:%d, data:%x", 4,
                      packet,
                      packet ? packet->process_id : 0,
                      device_id,
                      data);*/

    if (!packet || !packet->process_id || !data) {
        return RACE_ERRCODE_PARAMETER_ERROR;
    }

    int32_t i = 0;
    race_lpcomm_packet_type_enum packet_type = RACE_LPCOMM_PACKET_TYPE_NONE;
    const race_lpcomm_data_recv_hdl_struct *data_recv_hdl_array = data;

    RACE_LOG_MSGID_I("packet_type:%x cmd_id:%x channel_id:%d process_id:%d trans_method:%d payload_len:%d", 6,
                     packet->packet_type, packet->cmd_id, packet->channel_id, packet->process_id, packet->trans_method, packet->payload_len);
    packet_type = packet->packet_type;
#ifdef RACE_FOTA_CMD_ENABLE
    if (RACE_APP_ID_FOTA == packet->app_id &&
        ((RACE_LPCOMM_PACKET_TYPE_RACE_CMD_RSP == packet_type ||
          RACE_LPCOMM_PACKET_TYPE_COMMON_RSP == packet_type) &&
         !race_fota_is_race_fota_running()))
#else
    if (RACE_APP_ID_FOTA == packet->app_id &&
        ((RACE_LPCOMM_PACKET_TYPE_RACE_CMD_RSP == packet_type ||
          RACE_LPCOMM_PACKET_TYPE_COMMON_RSP == packet_type)))
#endif
    {
        /* FOTA stop procedure does not remove the REQ from the retry list if it exists. Therefore, remove it here. */
        //RACE_LOG_MSGID_W("Drop FOTA packet_type:%d cmd_id:%d because FOTA is not running right now.", 2, packet_type, packet->cmd_id);
#ifdef RACE_FOTA_CMD_ENABLE
        race_lpcomm_retry_list_struct *list_node = race_lpcomm_retry_list_node_find_by_process_id(packet->process_id);
        if (list_node) {
            race_lpcomm_retry_drop_packet(list_node);
        }
#endif
        return RACE_ERRCODE_WRONG_STATE;
    }

    while (RACE_INVALID_CMD_ID != data_recv_hdl_array[i].cmd_id) {
        if (packet->cmd_id == data_recv_hdl_array[i].cmd_id) {
            if (((RACE_LPCOMM_PACKET_TYPE_RACE_CMD_REQ == packet_type &&
                  RACE_LPCOMM_PACKET_CLASS_RACE_CMD == data_recv_hdl_array[i].packet_class) ||
                 (RACE_LPCOMM_PACKET_TYPE_COMMON_REQ == packet_type &&
                  RACE_LPCOMM_PACKET_CLASS_COMMON == data_recv_hdl_array[i].packet_class)) &&
                data_recv_hdl_array[i].req_hdl) {
                return data_recv_hdl_array[i].req_hdl(packet, device_id);
            } else if ((RACE_LPCOMM_PACKET_TYPE_RACE_CMD_RSP == packet_type &&
                        RACE_LPCOMM_PACKET_CLASS_RACE_CMD == data_recv_hdl_array[i].packet_class) ||
                       (RACE_LPCOMM_PACKET_TYPE_COMMON_RSP == packet_type &&
                        RACE_LPCOMM_PACKET_CLASS_COMMON == data_recv_hdl_array[i].packet_class)) {
                if ((RACE_LPCOMM_ROLE_PARTNER == role && data_recv_hdl_array[i].rsp_hdl) || (RACE_LPCOMM_ROLE_AGENT == role)) {
#ifdef RACE_LPCOMM_RETRY_ENABLE
                /* Remove req from the retry-list */
#ifdef RACE_FOTA_CMD_ENABLE
                    race_lpcomm_retry_list_struct *list_node = race_lpcomm_retry_list_node_find_by_process_id(packet->process_id);
                    if (list_node) {
                        race_lpcomm_packet_free(list_node->data);
                        race_lpcomm_retry_list_remove(list_node);
                        race_lpcomm_retry_list_node_free(list_node);

                        /* Free process_status in rsp_hdl if needed. */
                        if (data_recv_hdl_array[i].rsp_hdl) {
                            return data_recv_hdl_array[i].rsp_hdl(packet, device_id);
                        } else {
                            configASSERT(0);
                        }
                    } else {
                        RACE_LOG_MSGID_I("Drop the RSP for there's no REQ in the retry_list.", 0);
                    }
#endif
#else
                    if (data_recv_hdl_array[i].rsp_hdl) {
                        /* Free process_status in rsp_hdl if needed. */
                        return data_recv_hdl_array[i].rsp_hdl(packet, device_id);
                    } else {
                        configASSERT(0);
                    }
#endif /* RACE_LPCOMM_RETRY_ENABLE */
                }
            } else if (RACE_LPCOMM_PACKET_CLASS_COMMON == data_recv_hdl_array[i].packet_class &&
                       RACE_LPCOMM_PACKET_TYPE_COMMON_NOTI == packet_type &&
                       data_recv_hdl_array[i].noti_hdl) {
                return data_recv_hdl_array[i].noti_hdl(packet, device_id);
            }

            return RACE_ERRCODE_FAIL;
        }

        i++;
    }
    return RACE_ERRCODE_FAIL;

}

RACE_ERRCODE race_lpcomm_data_recv_msg_process(race_general_msg_t *msg)
{
    race_lpcomm_packet_struct *packet = NULL;
    race_lpcomm_role_enum role = RACE_LPCOMM_ROLE_NONE;

    if (!msg || !msg->msg_data) {
        return RACE_ERRCODE_PARAMETER_ERROR;
    }

    packet = (race_lpcomm_packet_struct *)msg->msg_data;

    /*RACE_LOG_MSGID_I("packet_type:%x, cmd_id:%x, channel_id:%d, process_id:%d, trans_method:%d, payload_len:%d, dev_id:%d", 7,
                     packet->packet_type,
                     packet->cmd_id,
                     packet->channel_id,
                     packet->process_id,
                     packet->trans_method,
                     packet->payload_len,
                     msg->dev_t);*/
    /* If REQ or RSP is received before RHO and processed after RHO, it will be dispatched to the wrong role handler. However, sender role can fix this.
     * Therefore, event if RACE_AWS_ENABLE is enabled, as long as RACE_LPCOMM_SENDER_ROLE_ENABLE is enabled, use the sender role to dispatch
     * the packets.
     */
#ifndef RACE_LPCOMM_SENDER_ROLE_ENABLE
    role = race_lpcomm_role_get(packet->trans_method);
#else
    role = RACE_LPCOMM_PACKET_GET_SENDER_ROLE(packet->packet_type);
    /* Convert the sender role to the receiver role. */
    if (RACE_LPCOMM_ROLE_AGENT == role) {
        role = RACE_LPCOMM_ROLE_PARTNER;
    } else if (RACE_LPCOMM_ROLE_PARTNER == role) {
        role = RACE_LPCOMM_ROLE_AGENT;
    }

    packet->packet_type = RACE_LPCOMM_PACKET_GET_PACKET_TYPE(packet->packet_type);
#endif

    if (RACE_LPCOMM_ROLE_AGENT == role) {
        race_lpcomm_agent_data_recv_hdl(packet, role, msg->dev_t);
    } else if (RACE_LPCOMM_ROLE_PARTNER == role) {
        race_lpcomm_partner_data_recv_hdl(packet, role, msg->dev_t);
    } else {
        RACE_LOG_MSGID_W("unknown role:%d", 1, role);
    }

    return RACE_ERRCODE_SUCCESS;
}

#endif /* RACE_LPCOMM_ENABLE */

