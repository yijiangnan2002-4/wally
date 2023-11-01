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
#if (defined AIR_LE_AUDIO_UNICAST_ENABLE) && (defined MTK_RACE_CMD_ENABLE)

#include "race_cmd.h"
#include "race_xport.h"
#include "race_noti.h"
#include "bt_le_audio_msglog.h"
#include "app_le_audio_ucst.h"
#include "app_le_audio_air.h"
#include "app_le_audio_race.h"
#include "apps_race_cmd_event.h"

#if defined(AIR_BLE_AUDIO_DONGLE_ENABLE)
#include "app_le_audio.h"
#include "app_le_audio_utillity.h"
#include "app_dongle_connection_common.h"
#include "race_cmd_le_audio.h"

extern bt_status_t app_le_audio_start_source(const bt_addr_t addr, app_dongle_cm_start_source_param_t param);
extern bt_status_t app_le_audio_stop_source(const bt_addr_t addr, app_dongle_cm_stop_source_param_t param);
#endif

#define LOG_TAG        "[app_le_audio_race]"

void app_le_audio_race_get_device_status_handler(uint8_t race_channel, app_dongle_le_race_get_device_status_cmd_t *cmd)
{
    RACE_ERRCODE race_ret;
    app_le_audio_ucst_link_info_t *link_info = app_le_audio_ucst_get_link_info_by_addr(&cmd->addr);
    LE_AUDIO_MSGLOG_I(LOG_TAG" get link_info is 0x%x", 1, link_info);
    app_dongle_le_race_get_device_status_rsp_t *response = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                           (uint16_t)RACE_DONGLE_LE_GET_DEVICE_STATUS,
                                                                           (uint16_t)(sizeof(app_dongle_le_race_get_device_status_rsp_t)),
                                                                           race_channel);
    if (response != NULL) {
        if (link_info) {
            response->status = 0;
            memcpy(&response->addr, &link_info->addr, sizeof(bt_addr_t));
            response->device_id = race_get_device_id_by_conn_address(&response->addr.addr);
            response->group_id = link_info->group_id;
            response->role = app_le_audio_air_get_role(&response->addr);
        } else {
            response->status = APP_DONGLE_LE_RACE_GET_STATUS_NOT_CONNECTED_STATUS;
            memcpy(&response->addr, &cmd->addr, sizeof(bt_addr_t));
            response->device_id = 0xFF;
            response->group_id = 0xFF;
            response->role = 0xFF;
        }
        race_ret = race_noti_send(response, race_channel, false);
        if (race_ret != RACE_ERRCODE_SUCCESS) {
            RACE_FreePacket((void *)response);
        }
    }

}

void app_le_audio_race_get_device_list_handler(uint8_t race_channel)
{
    app_le_audio_ucst_link_info_t *link_info = NULL;
    uint32_t count = app_le_audio_ucst_get_link_num();
    uint32_t position = 0;
    RACE_ERRCODE race_ret;
    LE_AUDIO_MSGLOG_I(LOG_TAG" get link_info list count %d", 1, count);
    app_dongle_le_race_get_device_list_rsp_t *response = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                         (uint16_t)RACE_DONGLE_LE_GET_DEVICE_LIST,
                                                                         (uint16_t)(sizeof(app_dongle_le_race_get_device_list_rsp_t) + count * sizeof(app_dongle_le_race_device_status_item_t)),
                                                                         race_channel);
    if (response != NULL) {
        response->status = 0;
        for (uint32_t i = 0; i < APP_LE_AUDIO_UCST_LINK_MAX_NUM && position < count; i++) {
            link_info = app_le_audio_ucst_get_link_info_by_idx(i);
            if (link_info && BT_HANDLE_INVALID != link_info->handle) {
                memcpy(&response->devices_list[position].addr, &link_info->addr, sizeof(bt_addr_t));
                response->devices_list[position].device_id = race_get_device_id_by_conn_address(&link_info->addr.addr);
                response->devices_list[position].group_id = link_info->group_id;
                response->devices_list[position].role = app_le_audio_air_get_role(&link_info->addr);
                position++;
            }
        }
        race_ret = race_noti_send(response, race_channel, false);
        if (race_ret != RACE_ERRCODE_SUCCESS) {
            RACE_FreePacket((void *)response);
        }
    }

}

void app_le_audio_race_get_paired_device_list_handler(uint8_t race_channel)
{
    RACE_ERRCODE race_ret;
    app_le_audio_ucst_bonded_list_t bonded_list;
    app_le_audio_ucst_get_bonded_list(&bonded_list);
    uint32_t count = bonded_list.num;
    uint32_t i;
    LE_AUDIO_MSGLOG_I(LOG_TAG" get paried list count %d", 1, count);
    app_dongle_le_race_get_device_list_rsp_t *response = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                            (uint16_t)RACE_DONGLE_LE_GET_PAIRED_LIST,
                                                                            (uint16_t)(sizeof(app_dongle_le_race_get_device_list_rsp_t) + count * sizeof(app_dongle_le_race_device_status_item_t)),
                                                                            race_channel);
    if (response != NULL) {
        response->status = 0;
        for (i = 0; i < count; i++) {
            memcpy(&response->devices_list[i].addr, &bonded_list.device[i].addr, sizeof(bt_addr_t));
            response->devices_list[i].device_id = race_get_device_id_by_conn_address(&(bonded_list.device[i].addr.addr));
            response->devices_list[i].group_id = bonded_list.device[i].group_id;
            response->devices_list[i].role = app_le_audio_air_get_role(&bonded_list.device[i].addr);
        }
        race_ret = race_noti_send(response, race_channel, false);
        if (race_ret != RACE_ERRCODE_SUCCESS) {
            RACE_FreePacket((void *)response);
        }
    }
}

void app_le_audio_race_switch_active_device_handler(uint8_t race_channel, app_dongle_le_race_switch_active_audio_cmd_t *cmd)
{
    RACE_ERRCODE race_ret;
    app_dongle_le_race_switch_active_audio_rsp_t *response = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                         (uint16_t)RACE_DONGLE_LE_SWITCH_ACTIVE_DEVICE,
                                                                         (uint16_t)(sizeof(app_dongle_le_race_switch_active_audio_rsp_t)),
                                                                          race_channel);

    if (response) {
        response->set_or_get = cmd->set_or_get;
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
        response->status = 0;
        if (cmd->set_or_get) {
            app_le_audio_ucst_set_active_group(cmd->group_id);
            response->status = 0;
        } else {
            response->group_id = app_le_audio_ucst_get_active_group();
        }
#else
        response->status = 0xFF;
#endif
        race_ret = race_noti_send(response, race_channel, false);
        if (race_ret != RACE_ERRCODE_SUCCESS) {
            RACE_FreePacket((void *)response);
        }
    }

}

#if defined(AIR_BLE_AUDIO_DONGLE_ENABLE)
static void *app_le_audio_dongle_race_set_stream_mode_handler(ptr_race_pkt_t p_race_package, uint8_t channel_id)
{
    typedef struct {
        RACE_COMMON_HDR_STRU cmdhdr;
        uint8_t mode;
    } PACKED CMD;

    typedef struct {
        uint8_t status;
    } PACKED RSP;

    CMD *pCmd = (CMD *)p_race_package;
    RSP *pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                 (uint16_t)RACE_LEAUDIO_SET_DONGLE_STREAM_MODE,
                                 (uint16_t)sizeof(race_le_audio_status_t),
                                 channel_id);
    int32_t ret = RACE_ERRCODE_FAIL;
    bt_status_t status = BT_STATUS_FAIL;
    bt_addr_t addr = {0};

    LE_AUDIO_MSGLOG_I(LOG_TAG" app_le_audio_dongle_race_set_stream_mode_handler, set mode:%d", 1, pCmd->mode);

    if (pEvt) {
        if (pCmd->mode == 0) {
            app_dongle_cm_stop_source_param_t param;
            param.param = 0x0;
            status = app_le_audio_stop_source(addr, param);
        } else if (pCmd->mode == 1 || pCmd->mode == 2) {
            app_dongle_cm_start_source_param_t param;
            param.lea_mode = pCmd->mode;
            status = app_le_audio_start_source(addr, param);
        }

        if (BT_STATUS_SUCCESS == status) {
            ret = RACE_ERRCODE_SUCCESS;
        }
        pEvt->status = ret;
    }

    return pEvt;
}

static void *app_le_audio_dongle_race_get_stream_mode_handler(ptr_race_pkt_t p_race_package, uint8_t channel_id)
{
    typedef struct {
        uint8_t status;
        uint8_t mode;
    } PACKED RSP;

    RSP *pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                 (uint16_t)RACE_LEAUDIO_GET_DONGLE_STREAM_MODE,
                                 (uint16_t)sizeof(race_le_audio_dongle_mode_t),
                                 channel_id);
    int32_t ret = RACE_ERRCODE_FAIL;

    if (pEvt) {
        pEvt->mode = app_le_audio_get_current_mode();
        LE_AUDIO_MSGLOG_I(LOG_TAG" app_le_audio_dongle_race_get_stream_mode_handler, get mode:%d", 1, pEvt->mode);
        ret = RACE_ERRCODE_SUCCESS;
        pEvt->status = ret;
    }

    return pEvt;
}

static void *app_le_audio_dongle_race_broadcast_device_handler(ptr_race_pkt_t p_race_package, uint8_t channel_id)
{
    uint8_t *p_Data = &p_race_package->payload[0];
    uint8_t opt = 0;
    int32_t ret = RACE_ERRCODE_FAIL;

    LE_AUDIO_MSGLOG_I(LOG_TAG" app_le_audio_dongle_race_broadcast_device_handler", 0);

    opt = p_Data[0];

    //TO DO
    if (0 == opt) {
        /* Get Broadcast Device Group ID */
        uint8_t group_num = 2;
        uint16_t total_size = 0;

        //group_num = get roup num

        typedef struct {
            uint8_t status;
            uint8_t opt;
            uint8_t group_number;
            uint8_t group_id[1];
        } PACKED RSP;

        total_size = sizeof(race_le_audio_dongle_broadcast_option_t) + 1 + group_num;

        RSP *pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                     (uint16_t)RACE_LEAUDIO_DONGLE_BROADCAST_DEVICE,
                                     total_size,
                                     channel_id);
        if (pEvt) {
            pEvt->opt = opt;
            pEvt->group_number = group_num;
            pEvt->group_id[0] = 0;
            pEvt->group_id[1] = 1;

            ret = RACE_ERRCODE_SUCCESS;
            pEvt->status = ret;
        }

        return pEvt;

    } else if (1 == opt) {
        /* Set Broadcast Device Group ID */
        uint8_t i = 0, group_num = 0;

        group_num = p_Data[1];

        for (i = 0; i < group_num; i++) {
            //group_id = p_Data[i + 1];
            //Set Group id
        }

        typedef struct {
            uint8_t status;
            uint8_t opt;
        } PACKED RSP;

        RSP *pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                     (uint16_t)RACE_LEAUDIO_DONGLE_BROADCAST_DEVICE,
                                     (uint16_t)sizeof(race_le_audio_dongle_broadcast_option_t),
                                     channel_id);
        if (pEvt) {
            pEvt->opt = opt;
            ret = RACE_ERRCODE_SUCCESS;
            pEvt->status = ret;
        }

        return pEvt;
    }

    return NULL;
}

void *app_le_audio_dongle_race_handler(ptr_race_pkt_t p_race_package, uint16_t length, uint8_t channel_id)
{
    void *ptr = NULL;
    LE_AUDIO_MSGLOG_I(LOG_TAG" app_le_audio_dongle_race_handler, type[0x%X], id[0x%X]", 2, p_race_package->hdr.type, p_race_package->hdr.id);

    if (p_race_package->hdr.type == RACE_TYPE_COMMAND ||
        p_race_package->hdr.type == RACE_TYPE_COMMAND_WITHOUT_RSP) {
        switch (p_race_package->hdr.id) {
            case RACE_LEAUDIO_SET_DONGLE_STREAM_MODE: {
                ptr = app_le_audio_dongle_race_set_stream_mode_handler(p_race_package, channel_id);
            }
            break;
            case RACE_LEAUDIO_GET_DONGLE_STREAM_MODE: {
                ptr = app_le_audio_dongle_race_get_stream_mode_handler(p_race_package, channel_id);
            }
            break;
            case RACE_LEAUDIO_DONGLE_BROADCAST_DEVICE: {
                ptr = app_le_audio_dongle_race_broadcast_device_handler(p_race_package, channel_id);
            }
            break;
            default:
                break;
        }
    }

    return ptr;
}
#endif

void app_le_audio_race_init(void)
{
#if defined(AIR_BLE_AUDIO_DONGLE_ENABLE)
    race_le_audio_dongle_register_callback(app_le_audio_dongle_race_handler);
#endif
}

#endif
