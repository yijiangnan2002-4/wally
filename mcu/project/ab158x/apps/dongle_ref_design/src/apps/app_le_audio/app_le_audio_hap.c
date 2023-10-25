/* Copyright Statement:
 *
 * (C) 2023  Airoha Technology Corp. All rights reserved.
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
#ifdef AIR_LE_AUDIO_ENABLE
#ifdef AIR_LE_AUDIO_HAPC_ENABLE
#include "atci.h"
#include "app_le_audio_hapc.h"


 /**************************************************************************************************
 * Define
 **************************************************************************************************/


 /**************************************************************************************************
 * Structure
 **************************************************************************************************/

 /**************************************************************************************************
 * Variable
 **************************************************************************************************/



 /**************************************************************************************************
 * Prototype
 **************************************************************************************************/



 /**************************************************************************************************
 * Static Functions
 **************************************************************************************************/


static void app_le_audio_ucst_show_hapc_info_to_atci(ble_hapc_event_t event, void *msg)
{
    atci_response_t *response = (atci_response_t *)pvPortMalloc(sizeof(atci_response_t));
    char *p;
    int len;

    if (!response) return;

    memset(response, 0, sizeof(*response));
    p = (char *)response->response_buf;
    *p++ = '\r';
    *p++ = '\n';
    len = sizeof(response->response_buf) - 2;

    switch (event)
    {
        case BLE_HAPC_EVT_DISCOVERY_COMPLETE:
        {
            ble_hapc_evt_discovery_complete_t *ind = (ble_hapc_evt_discovery_complete_t *)msg;
            snprintf(p, len, "HAPS[%x]:discovery complete[%d][%d]\r\n", ind->conn_hdl, (ind->status == BT_STATUS_SUCCESS ? true : false), ind->ha_ctrl_pt_support);
            break;
        }
        case BLE_HAPC_EVT_HA_FEATURE_IND:
        {
            ble_hapc_evt_ha_feature_ind_t *ind = (ble_hapc_evt_ha_feature_ind_t *)msg;
            snprintf(p, len, "HAPS[%x]:FEATURE:0x%x\r\n", ind->conn_hdl, ind->feature);
            break;
        }
        case BLE_HAPC_EVT_PRESET_ACTIVE_IND:
        {
            ble_hapc_evt_preset_active_ind_t *ind = (ble_hapc_evt_preset_active_ind_t *)msg;
            snprintf(p, len, "HAPS[%x]:ACTIVE[%d]\r\n", ind->conn_hdl, ind->index);
            break;
        }
        case BLE_HAPC_EVT_PRESET_DELETED_IND:
        {
            ble_hapc_evt_preset_deleted_ind_t *ind = (ble_hapc_evt_preset_deleted_ind_t *)msg;
            snprintf(p, len, "HAPS[%x]:DELETED[%d]\r\n", ind->conn_hdl, ind->index);
            break;
        }
        case BLE_HAPC_EVT_PRESET_AVAILABLE_IND:
        {
            ble_hapc_evt_preset_available_ind_t *ind = (ble_hapc_evt_preset_available_ind_t *)msg;
            snprintf(p, len, "HAPS[%x]:AVAILABLE[%d]\r\n", ind->conn_hdl, ind->index);
            break;
        }
        case BLE_HAPC_EVT_PRESET_UNAVAILABLE_IND:
        {
            ble_hapc_evt_preset_unavailable_ind_t *ind = (ble_hapc_evt_preset_unavailable_ind_t *)msg;
            snprintf(p, len, "HAPS[%x]:UNAVAILABLE[%d]\r\n", ind->conn_hdl, ind->index);
            break;
        }
        case BLE_HAPC_EVT_PRESET_CHANGE_IND:
        {
            ble_hapc_evt_preset_change_ind_t *ind = (ble_hapc_evt_preset_change_ind_t *)msg;
            snprintf(p, len, "HAPS[%x]:CHANGE[%d][0x%x][%s]\r\n", ind->conn_hdl, ind->index, ind->property, ind->name);
            break;
        }
        case BLE_HAPC_EVT_PRESET_INFO_IND:
        {
            ble_hapc_evt_preset_info_ind_t *ind = (ble_hapc_evt_preset_info_ind_t *)msg;
            snprintf(p, len, "HAPS[%x]:INFO[%d][0x%x][%s]\r\n", ind->conn_hdl, ind->index, ind->property, ind->name);
            break;
        }
    }

    response->response_flag = ATCI_RESPONSE_FLAG_URC_FORMAT;
    response->response_len = strlen((char *)response->response_buf);

    if (response->response_len > 2) atci_send_response(response);

    vPortFree(response);
}
 /**************************************************************************************************
 * Public Functions
 **************************************************************************************************/
void app_le_audio_ucst_handle_iac_evt(ble_iac_event_t event, void *msg)
{
    LE_AUDIO_MSGLOG_I("[APP][HAPC] iac event = %x", 1, event);

    if (event == BLE_IAC_EVT_DISCOVERY_COMPLETE)
    {
        ble_iac_evt_discovery_complete_t *ind = (ble_iac_evt_discovery_complete_t *)msg;
        LE_AUDIO_MSGLOG_I("[APP][HAPC] iac discovery complete, conn_hdl = %x", 1, ind->conn_hdl);
    }
}

void app_le_audio_ucst_handle_hapc_evt(ble_hapc_event_t event, void *msg)
{
    LE_AUDIO_MSGLOG_I("[APP][HAPC] event = %x", 1, event);

    switch (event)
    {
        case BLE_HAPC_EVT_DISCOVERY_COMPLETE:
        {
            ble_hapc_evt_discovery_complete_t *ind = (ble_hapc_evt_discovery_complete_t *)msg;
            LE_AUDIO_MSGLOG_I("[APP][HAPC] handle:%x, status:%x ctrl_supp:5d", 3, ind->conn_hdl, ind->status, ind->ha_ctrl_pt_support);

            if (ind->status == BT_STATUS_SUCCESS)
            {
                if (ble_hapc_read_preset(ind->conn_hdl, 1, 0xFF) == BT_STATUS_SUCCESS) return;

                ble_hapc_discovery_complete_response(ind->conn_hdl);
            }

            break;
        }
        case BLE_HAPC_EVT_REQUEST_COMPLETE:
        {
            ble_hapc_evt_request_complete_t *ind = (ble_hapc_evt_request_complete_t *)msg;
            LE_AUDIO_MSGLOG_I("[APP][HAPC] handle:%x, req:%d, status:%x", 3, ind->conn_hdl, ind->req, ind->status);

            if (ind->req == HAPC_REQ_READ_PRESET) ble_hapc_discovery_complete_response(ind->conn_hdl);

            break;
        }
        case BLE_HAPC_EVT_HA_FEATURE_IND:
        {
            ble_hapc_evt_ha_feature_ind_t *ind = (ble_hapc_evt_ha_feature_ind_t *)msg;
            LE_AUDIO_MSGLOG_I("[APP][HAPC] handle:%x, feature:%x", 2, ind->conn_hdl, ind->feature);
            break;
        }
        case BLE_HAPC_EVT_PRESET_ACTIVE_IND:
        {
            ble_hapc_evt_preset_active_ind_t *ind = (ble_hapc_evt_preset_active_ind_t *)msg;
            LE_AUDIO_MSGLOG_I("[APP][HAPC] handle:%x, active_index:%d", 2, ind->conn_hdl, ind->index);
            break;
        }
        case BLE_HAPC_EVT_PRESET_INFO_IND:
        {
            ble_hapc_evt_preset_info_ind_t *ind = (ble_hapc_evt_preset_info_ind_t *)msg;
            LE_AUDIO_MSGLOG_I("[APP][HAPC] handle:%x, index:%d, property:%x name_len = %d", 4, ind->conn_hdl, ind->index, ind->property, ind->name_len);
            printf("[APP][HAPC] name: %s", ind->name);
            break;
        }
        case BLE_HAPC_EVT_PRESET_CHANGE_IND:
        {
            ble_hapc_evt_preset_change_ind_t *ind = (ble_hapc_evt_preset_change_ind_t *)msg;
            LE_AUDIO_MSGLOG_I("[APP][HAPC] handle:%x, prev_idx:%d, index:%d, property:%x name_len = %d", 5, ind->conn_hdl, ind->prev_idx, ind->index, ind->property, ind->name_len);
            printf("[APP][HAPC] name: %s", ind->name);
            break;
        }
        case BLE_HAPC_EVT_PRESET_DELETED_IND:
        {
            ble_hapc_evt_preset_deleted_ind_t *ind = (ble_hapc_evt_preset_deleted_ind_t *)msg;
            LE_AUDIO_MSGLOG_I("[APP][HAPC] handle:%x, index:%d", 2, ind->conn_hdl, ind->index);
            break;
        }
        case BLE_HAPC_EVT_PRESET_AVAILABLE_IND:
        {
            ble_hapc_evt_preset_available_ind_t *ind = (ble_hapc_evt_preset_available_ind_t *)msg;
            LE_AUDIO_MSGLOG_I("[APP][HAPC] handle:%x, index:%d", 2, ind->conn_hdl, ind->index);
            break;
        }
        case BLE_HAPC_EVT_PRESET_UNAVAILABLE_IND:
        {
            ble_hapc_evt_preset_unavailable_ind_t *ind = (ble_hapc_evt_preset_unavailable_ind_t *)msg;
            LE_AUDIO_MSGLOG_I("[APP][HAPC] handle:%x, index:%d", 2, ind->conn_hdl, ind->index);
            break;
        }

        default:
            break;
    }

    app_le_audio_ucst_show_hapc_info_to_atci(event, msg);
}
#endif
#endif

