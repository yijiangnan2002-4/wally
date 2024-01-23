/* Copyright Statement:
 *
 * (C) 2020  Airoha Technology Corp. All rights reserved.
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


#include "apps_race_cmd_event.h"

#include "apps_config_key_remapper.h"
#include "ui_shell_manager.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"
#include "apps_debug.h"
#include "bt_device_manager.h"
#include "nvkey.h"
#include "nvkey_id_list.h"
#include "apps_aws_sync_event.h"
#include "app_home_screen_idle_activity.h"
#ifdef MTK_IN_EAR_FEATURE_ENABLE
#include "app_music_utils.h"
#endif
#ifdef MTK_RACE_CMD_ENABLE
#include "race_cmd.h"
#include "race_cmd_relay_cmd.h"
#include "race_noti.h"
#include "race_bt.h"
#endif
#include "apps_control_touch_key_status.h"

#define LOG_TAG     "[apps_touch_key]"

uint8_t s_cmd_touch_state = 1;

uint8_t apps_set_touch_control_enable(uint8_t isEnable, bool isApp)
{
    uint8_t ret = 0;
    bool temp_states = false;
    bt_status_t send_state = BT_STATUS_FAIL;
    nvkey_status_t status = 0;

    status = nvkey_write_data(NVID_APP_RACE_TOUCH_KEY_ENABLE, (uint8_t *)&isEnable, sizeof(uint8_t));
    if (NVKEY_STATUS_OK == status) {
        s_cmd_touch_state = isEnable;
        temp_states = apps_touch_key_status_update_notify(isEnable, isApp);
#ifdef MTK_AWS_MCE_ENABLE
        if ((true == app_home_screen_idle_activity_is_aws_connected())
            && (BT_AWS_MCE_ROLE_AGENT == bt_device_manager_aws_local_info_get_role())) {
            send_state = apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                                        APPS_EVENTS_INTERACTION_SYNC_TOUCH_KEY_STATUS,
                                                        &s_cmd_touch_state, sizeof(uint8_t));
        }
#endif
        ret = 1;
    }
    APPS_LOG_MSGID_I(LOG_TAG" set_touch_control_enable: status=%d, temp_states=%d, send_state=0x%x, touch_state=%d.",
                     4, status, temp_states, send_state, s_cmd_touch_state);
    return ret;
}

bool apps_touch_key_status_update_notify(uint8_t isEnable, bool isAppNotify)
{
    bool ret = false;

    APPS_LOG_MSGID_I(LOG_TAG" send UI shell event to notify app touch key status update: isEnable=%d, isApp=%d. ", 2, isEnable, isAppNotify);
    ui_shell_remove_event(EVENT_GROUP_UI_SHELL_TOUCH_KEY, APPS_TOUCH_KEY_STATUS_UPDATE);
    ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_TOUCH_KEY,
                        APPS_TOUCH_KEY_STATUS_UPDATE, NULL, 0, NULL, 0);
#if defined(MTK_RACE_CMD_ENABLE)
    if (isAppNotify) {
        uint8_t channel_id;
        RACE_ERRCODE race_notify_ret;
        uint32_t serial_port;
        serial_port_dev_t dev;
        bt_handle_t handle;

        typedef struct {
            uint16_t config_type;
            uint8_t isEnable;
        } PACKED NOTI;

        handle = race_bt_get_ble_conn_hdl();
        APPS_LOG_MSGID_I(LOG_TAG" notify touch key status update: handle=%x.", 1, handle);

        if (handle != BT_HANDLE_INVALID) {
            dev = SERIAL_PORT_DEV_BT_LE;
        } else {
            dev = SERIAL_PORT_DEV_BT_SPP;
        }

        serial_port = race_get_serial_port_handle(dev);

        channel_id = (uint8_t)race_get_channel_id_by_port_handle(serial_port);
        NOTI *Noti = RACE_ClaimPacket((uint8_t)RACE_TYPE_COMMAND_WITHOUT_RSP,
                                      (uint16_t)RACE_SET_APP_COMMON_CONFIG,
                                      (uint16_t)sizeof(NOTI),
                                      channel_id);
        if (Noti != NULL) {
            Noti->config_type = APPS_RACE_CMD_CONFIG_TYPE_TOUCH_KEY_ENABLE;
            if (isEnable) {
                Noti->isEnable = 1;/*means agent is in ref*/
            } else {
                Noti->isEnable = 0;
            }
            race_notify_ret = race_noti_send((NOTI *)Noti, channel_id, false);
            APPS_LOG_MSGID_I(LOG_TAG" notify touch key status ret=%x.", 1, race_notify_ret);
            if (race_notify_ret != RACE_ERRCODE_SUCCESS) {
                RACE_FreePacket((void *)Noti);
                Noti = NULL;
                ret = false;
            }
        } else {
            APPS_LOG_MSGID_I(LOG_TAG" RACE_ClaimPacketAppID return NULL. ", 0);
        }
    }
#endif
    return ret;
}

uint8_t apps_get_touch_control_status(void)
{
    nvkey_status_t status = 0;
    uint8_t temp_touch_status = 0;
    uint32_t temp_touch_lenth = sizeof(temp_touch_status);
    status = nvkey_read_data(NVID_APP_RACE_TOUCH_KEY_ENABLE, (uint8_t *)&temp_touch_status, &temp_touch_lenth);
    if (NVKEY_STATUS_OK != status) {
        temp_touch_status = 0xFF;
    }
    APPS_LOG_MSGID_I(LOG_TAG" get touch key status: temp_touch_status=0x%02X.", 1, temp_touch_status);
    return temp_touch_status;
}

void apps_init_touch_key_status(void)
{
    uint8_t isEnable = 1;
    /* Init touch key into enable. */
    apps_set_touch_control_enable(isEnable, false);
    APPS_LOG_MSGID_I(LOG_TAG" init touch key enable. ", 0);
}


