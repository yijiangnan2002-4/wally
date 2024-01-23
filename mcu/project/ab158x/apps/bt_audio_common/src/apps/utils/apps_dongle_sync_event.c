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

#include "apps_dongle_sync_event.h"
#include "FreeRTOS.h"
#include "apps_debug.h"
#include "ui_shell_manager.h"
#include "apps_events_event_group.h"
#include "bt_connection_manager.h"
#ifdef MTK_RACE_CMD_ENABLE
#include "race_cmd.h"
#include "race_cmd_relay_cmd.h"
#include "race_noti.h"
#include "race_bt.h"
#endif
#if defined(AIR_LE_AUDIO_UNICAST_ENABLE) && defined(AIR_DONGLE_ENABLE)
#include "app_le_audio_ucst_utillity.h"
#endif

#define TAG "[DONGLE SYNC] "
#define APPS_DONGLE_SYNC_SET_DATA 0x2E10
#define APPS_DONGLE_SYNC_ID_START APPS_DONGLE_SYNC_SET_DATA
#define APPS_DONGLE_SYNC_ID_END 0x2E11

apps_dongle_role_t apps_get_dongle_role(void)
{
    //return APPS_ULL_DONGLE;
    return APPS_DONGLE_DONGLE;
    //return APPS_ULL_HEADSET;
}

#if defined(MTK_RACE_CMD_ENABLE)
bool apps_is_dongle_connected(void)
{
    bt_bd_addr_t addr_list[1];
    uint32_t list_num = 1;
    list_num = bt_sink_srv_cm_get_connected_device_list((bt_sink_srv_profile_type_t)BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_CUSTOMIZED_ULL),
                                                        addr_list, list_num);
    if (list_num > 0 &&
        BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AIR) & bt_cm_get_connected_profile_services(addr_list[0])) {
        APPS_LOG_MSGID_I(TAG"ull connected", 0);
        return true;
    }

    return false;
}

bt_status_t apps_dongle_sync_event_send_extra(uint32_t event_group,
                                              uint32_t event_id, void *extra_data, uint32_t extra_data_len)
{
    return apps_dongle_sync_event_send_extra_by_address(event_group, event_id, extra_data, extra_data_len, NULL);
}

/**
* @brief      This function is used to send UI Shell event with extra_data.
* @param[in]  event_group, the current event group to be sent.
* @param[in]  event_id, the current event ID to be sent.
* @param[in]  extra_data, extra data pointer of the current event, NULL means there is no extra data.
* @param[in]  extra_data_len, the length of the extra data. 0 means extra_data is NULL.
* @return
* #BT_STATUS_SUCCESS, if the operation is successful.
* #BT_STATUS_FAIL, if the event is not sent successfully.
*/
bt_status_t apps_dongle_sync_event_send_extra_by_channel(uint32_t event_group,
                                              uint32_t event_id, void *extra_data,
                                              uint32_t extra_data_len, uint8_t channel_id)
{
    apps_dongle_event_sync_info_t *pkg = RACE_ClaimPacket(
                                             (uint8_t)RACE_TYPE_COMMAND_WITHOUT_RSP,
                                             (uint16_t)APPS_DONGLE_SYNC_SET_DATA,
                                             (uint16_t)(sizeof(apps_dongle_event_sync_info_t) + extra_data_len),
                                             channel_id);
    if (pkg == NULL) {
        return BT_STATUS_FAIL;
    }
    pkg->event_group = event_group;
    pkg->event_id = event_id;
    pkg->extra_data_len = extra_data_len;
    if (extra_data_len > 0) {
        memcpy(&pkg->data, extra_data, extra_data_len);
    }
    RACE_ERRCODE race_notify_ret = race_noti_send(pkg, channel_id, false);
    APPS_LOG_MSGID_I(TAG"set race data ret=%d, input_channel=%d", 2, race_notify_ret, channel_id);
    if (race_notify_ret != RACE_ERRCODE_SUCCESS) {
        RACE_FreePacket((void *)pkg);
    }
    return race_notify_ret == RACE_ERRCODE_SUCCESS ? BT_STATUS_SUCCESS : BT_STATUS_FAIL;
}

bt_status_t apps_dongle_sync_event_send_extra_by_address(uint32_t event_group,
                                                         uint32_t event_id,
                                                         void *extra_data,
                                                         uint32_t extra_data_len,
                                                         bt_bd_addr_t *addr)
{
    uint8_t channel_id;
#if defined(AIR_DONGLE_ENABLE)
#if (defined(AIR_LE_AUDIO_ENABLE) && defined(AIR_LE_AUDIO_UNICAST_ENABLE)) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
    channel_id = race_get_channel_id_by_port_type(RACE_SERIAL_PORT_TYPE_BLE);
#if (defined(AIR_LE_AUDIO_ENABLE) && defined(AIR_LE_AUDIO_UNICAST_ENABLE))
    app_le_audio_ucst_active_group_addr_list_t active_addr_list;
    if (addr == NULL) {
        memset(&active_addr_list, 0, sizeof(app_le_audio_ucst_active_group_addr_list_t));
        app_le_audio_ucst_get_active_group_address(&active_addr_list);
        if (active_addr_list.num != 0) {
            addr = &active_addr_list.addr[0].addr;
        } else {
            APPS_LOG_MSGID_E(TAG"no activate channel found", 0);
        }
    }
#endif
    if (addr) {
        uint8_t activate_channel = race_get_channel_id_by_conn_address(addr);
        if (activate_channel != RACE_SERIAL_PORT_TYPE_NONE) {
            channel_id = activate_channel;
        } else {
            APPS_LOG_MSGID_E(TAG"get invalid race channel", 0);
            return BT_STATUS_FAIL;
        }
    }
#else
    channel_id = race_get_channel_id_by_port_type(RACE_SERIAL_PORT_TYPE_SPP);
#endif
#else /* #if defined(AIR_DONGLE_ENABLE) */
    if (addr != NULL) {
        channel_id = race_get_channel_id_by_conn_address(addr);
    }
    if (addr == NULL || channel_id == RACE_INVALID_CHANNEL_ID) {
#if defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
        channel_id = race_get_channel_id_by_port_type(RACE_SERIAL_PORT_TYPE_BLE);
#else
        channel_id = race_get_channel_id_by_port_type(RACE_SERIAL_PORT_TYPE_SPP);
#endif
    }
#endif /* #if defined(AIR_DONGLE_ENABLE) */
    return apps_dongle_sync_event_send_extra_by_channel(event_group, event_id, extra_data, extra_data_len, channel_id);
}

bt_status_t apps_dongle_sync_event_send(uint32_t event_group, uint32_t event_id)
{
    return apps_dongle_sync_event_send_extra(event_group, event_id, NULL, 0);
}

static void *apps_dongle_sync_race_handler(ptr_race_pkt_t p_race_package, uint16_t length, uint8_t channel_id)
{
    typedef struct {
        RACE_COMMON_HDR_STRU cmdhdr;
        uint8_t data[0];
    } PACKED CMD1;
    CMD1 *cmd = (CMD1 *)p_race_package;
    APPS_LOG_MSGID_I(TAG"receive ull event sync data, len=%d, channel=%d", 2, length, channel_id);

#if 1
    uint8_t *shell_data = (uint8_t *)pvPortMalloc(length + sizeof(channel_id));
#else
    uint8_t *shell_data = (uint8_t *)pvPortMalloc(length);
#endif
    if (shell_data != NULL) {
        memcpy(shell_data, cmd->data, length);
#if 1
        *(shell_data + length) = channel_id;
#endif
        ui_shell_status_t status = ui_shell_send_event(true, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_DONGLE_DATA,
                                                       0, (void *)shell_data, length,
                                                       NULL, 0);
        if (status != UI_SHELL_STATUS_OK) {
            APPS_LOG_MSGID_E(TAG"send ui shell event failed", 0);
            vPortFree(shell_data);
        }
    } else {
        APPS_LOG_MSGID_E(TAG"malloc mem failed", 0);
    }
    return NULL;
}

void apps_dongle_sync_init(void)
{
    RACE_HANDLER handler = {
        .id_start = APPS_DONGLE_SYNC_ID_START,
        .id_end = APPS_DONGLE_SYNC_ID_END,
        .handler = apps_dongle_sync_race_handler
    };
    race_status_t ret = RACE_Register_Handler(&handler);
    APPS_LOG_MSGID_I(TAG"init race ret=%d", 1, ret);
}

#if 1
uint8_t apps_dongle_sync_event_get_channel_id(void* extra_data, uint32_t len)
{
    if (extra_data == NULL || len == 0) {
        return RACE_INVALID_CHANNEL_ID;
    }
    uint8_t *sync_data = (uint8_t*)extra_data;
    return sync_data[len];
}
#endif

#else
bool apps_is_dongle_connected(void)
{
    return false;
}

bt_status_t apps_dongle_sync_event_send_extra(uint32_t event_group,
                                              uint32_t event_id, void *extra_data, uint32_t extra_data_len)
{
    return BT_STATUS_SUCCESS;
}

bt_status_t apps_dongle_sync_event_send(uint32_t event_group, uint32_t event_id)
{
    return BT_STATUS_SUCCESS;
}

bt_status_t apps_dongle_sync_event_send_extra_by_address(uint32_t event_group, uint32_t event_id,
                                            void *extra_data, uint32_t extra_data_len, bt_bd_addr_t *addr)
{
    return BT_STATUS_SUCCESS;
}

void apps_dongle_sync_init(void) {;}

uint8_t apps_dongle_sync_event_get_channel_id(void* extra_data, uint32_t len){return 0;}

#endif

