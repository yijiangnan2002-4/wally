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

#ifdef AIR_SPOTIFY_TAP_ENABLE

#include "app_spotify_tap_activity.h"
#include "spotify_tap.h"
#include "apps_debug.h"
#include "bt_aws_mce.h"
#include "bt_type.h"
#include "bt_device_manager.h"
#include "apps_events_event_group.h"
#include "apps_aws_sync_event.h"
#include "apps_config_event_list.h"
#include "nvkey_id_list.h"
#include "nvkey.h"
#include "assert.h"
#include "bt_connection_manager.h"

#define APP_SPOTIFY_TAP_ACTIVITY_TAG    "Spotify_Tap_Activity"

typedef struct {
    bool                inited;
    bool                connected;
} app_spotify_tap_context_t;

static app_spotify_tap_context_t s_app_tap_context;

static void app_spotify_tap_connection_callback_handler(bool connected)
{
    if (s_app_tap_context.connected != connected) {
        s_app_tap_context.connected = connected;
    }
}

static bool app_spotify_tap_activity_handle_key_event(uint16_t key_id)
{
    if (key_id != KEY_SPOTIFY_TAP_TRIGGER) {
        return false;
    }

#ifdef MTK_AWS_MCE_ENABLE
    bt_aws_mce_role_t role = bt_device_manager_aws_local_info_get_role();
#endif /* MTK_AWS_MCE_ENABLE */

    APPS_LOG_MSGID_I(APP_SPOTIFY_TAP_ACTIVITY_TAG", app_spotify_tap_activity_handle_key_event, role : 0x%02x, inited : %d, connected : %d",
                     3,
#ifdef MTK_AWS_MCE_ENABLE
                     role,
#else
                     BT_AWS_MCE_ROLE_NONE,
#endif /* MTK_AWS_MCE_ENABLE */
                     s_app_tap_context.inited,
                     s_app_tap_context.connected);

    if (s_app_tap_context.inited == false) {
        return false;
    }

#ifdef MTK_AWS_MCE_ENABLE
    if ((s_app_tap_context.connected == false) && (role == BT_AWS_MCE_ROLE_AGENT)) {
        return false;
    }

    if (role == BT_AWS_MCE_ROLE_PARTNER) {
        if (BT_STATUS_SUCCESS != apps_aws_sync_event_send(EVENT_GROUP_UI_SHELL_KEY, key_id)) {
            APPS_LOG_MSGID_E(APP_SPOTIFY_TAP_ACTIVITY_TAG", app_spotify_tap_activity_handle_key_event, Partner key event (0x%04x) to agent failed", 1, key_id);
        }
    } else
#endif /* MTK_AWS_MCE_ENABLE */
    {
        APPS_LOG_MSGID_I(APP_SPOTIFY_TAP_ACTIVITY_TAG", app_spotify_tap_activity_handle_key_event, Spotify Tap Triggered", 0);
        spotify_tap_trigger();
    }
    return true;
}

static bool app_spotify_tap_activity_read_settings(char *client_id, char *brand, char *model)
{
    /**
     * @brief Store the SpotifyTap ID to the nvdm.
     * The store order is : client ID, brand, model. And each item
     */

    uint8_t *read_buf = NULL;
    uint32_t read_buf_len = 255;

    read_buf = (uint8_t *)pvPortMalloc(sizeof(uint8_t) * 255);
    if (read_buf == NULL) {
        assert(0 && "Allocate read buffer failed");
    }

    memset(read_buf, 0, sizeof(uint8_t) * 255);

    nvkey_status_t status = nvkey_read_data(NVID_APP_SPOTIFY_TAP_SETTINGS, read_buf, &read_buf_len);

    if ((status != NVKEY_STATUS_OK) || (read_buf_len == 0)) {
        assert(0 && "Spotify Tap Setting Read Failed");
    }
    APPS_LOG_MSGID_I(APP_SPOTIFY_TAP_ACTIVITY_TAG", app_spotify_tap_activity_read_settings, read length : %d",
                     1, read_buf_len);

    if (read_buf_len <= SPOTIFY_TAP_CLIENT_ID_LEN) {
        assert(0 && "Spotify Tap Setting Error - Length < 33");
    }

    if (read_buf[SPOTIFY_TAP_CLIENT_ID_LEN - 1] != 0x00) {
        assert(0 && "Spotify Tap Setting Error - Client ID configure error");
    }

    uint8_t read_index = 0;
    uint8_t index = 0;
    uint8_t read_confirm = 0;
    uint8_t read_length = 0;

    memcpy(client_id, read_buf, SPOTIFY_TAP_CLIENT_ID_LEN);
    read_index += SPOTIFY_TAP_CLIENT_ID_LEN;
    read_confirm = 1;

    for (index = read_index; index < 255; index ++) {
        if (read_buf[index] == 0x00) {
            read_length = index - read_index;
            if (read_confirm == 1) {
                memcpy(brand, read_buf + read_index, read_length);
                read_index = index + 1;
            }
            if (read_confirm == 2) {
                memcpy(model, read_buf + read_index, read_length);
                break;
            }
            read_confirm ++;
        }
    }

    vPortFree(read_buf);
    read_buf = NULL;

    if ((strlen(client_id) == 0) || (strlen(brand) == 0) || (strlen(model) == 0)) {
        APPS_LOG_MSGID_E(APP_SPOTIFY_TAP_ACTIVITY_TAG", app_spotify_tap_activity_read_settings, Please configure spotify tap settings", 0);
        return false;
    }

    return true;
}

bool app_spotify_tap_activity_proc(ui_shell_activity_t *self,
                                   uint32_t event_group,
                                   uint32_t event_id,
                                   void *extra_data,
                                   size_t data_len)
{
    if ((event_group == EVENT_GROUP_UI_SHELL_SYSTEM)
        && (event_id == EVENT_ID_SHELL_SYSTEM_ON_CREATE)) {

        char client_id[SPOTIFY_TAP_CLIENT_ID_LEN] = {0};
        char *model = NULL;
        char *brand = NULL;

        model = (char *)pvPortMalloc(sizeof(char) * 100);
        brand = (char *)pvPortMalloc(sizeof(char) * 100);
        if ((model == NULL) || (brand == NULL)) {
            assert(0 && "Allocate model/brand buffer failed");
        }

        memset(model, 0, sizeof(char) * 100);
        memset(brand, 0, sizeof(char) * 100);

        app_spotify_tap_activity_read_settings(client_id, brand, model);

        /**printf("SpotifyTapActivity, read settings result, client id : %s, brand : %s, model : %s",
                client_id, brand, model); */

        spotify_tap_init_param_t param = {0};

        memcpy(param.client_id, client_id, SPOTIFY_TAP_CLIENT_ID_LEN - 1);
        param.brand = brand;
        param.model = model;

        bool ret = spotify_tap_init(&param, app_spotify_tap_connection_callback_handler);

        vPortFree(brand);
        brand = NULL;
        vPortFree(model);
        model = NULL;

        if (ret == false) {
            APPS_LOG_MSGID_E(APP_SPOTIFY_TAP_ACTIVITY_TAG", app_spotify_tap_activity_proc init spotify tap failed", 0);
            s_app_tap_context.inited = false;
        } else {
            s_app_tap_context.inited = true;
        }

        return true;
    }

    if (event_group == EVENT_GROUP_UI_SHELL_KEY) {
        uint16_t key_id = *(uint16_t *)extra_data;
        return app_spotify_tap_activity_handle_key_event(key_id);
    }

#ifdef MTK_AWS_MCE_ENABLE
    if (event_group == EVENT_GROUP_UI_SHELL_AWS_DATA) {
        bt_aws_mce_report_info_t *aws_data_ind = (bt_aws_mce_report_info_t *)extra_data;
        bt_aws_mce_role_t role = bt_device_manager_aws_local_info_get_role();

        if (aws_data_ind->module_id == BT_AWS_MCE_REPORT_MODULE_APP_ACTION) {
            uint32_t aws_event_group;
            uint32_t action;

            apps_aws_sync_event_decode(aws_data_ind, &aws_event_group, &action);

            /* Handle the key event come from partner. */
            if ((aws_event_group == EVENT_GROUP_UI_SHELL_KEY) && (role == BT_AWS_MCE_ROLE_AGENT)) {
                return app_spotify_tap_activity_handle_key_event(action);
            }
        }
    }
#endif /* MTK_AWS_MCE_ENABLE */

    if ((event_group == EVENT_GROUP_UI_SHELL_BT_CONN_MANAGER)
        && (event_id == BT_CM_EVENT_REMOTE_INFO_UPDATE)) {
        bt_cm_remote_info_update_ind_t *remote_update = (bt_cm_remote_info_update_ind_t *)extra_data;
        if (remote_update == NULL) {
            return false;
        }

        bool pre_hfp_conn = ((BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP) & remote_update->pre_connected_service) > 0);
        if (!pre_hfp_conn) {
            pre_hfp_conn = ((BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HSP) & remote_update->pre_connected_service) > 0);
        }
        bool pre_a2dp_conn = ((BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK) & remote_update->pre_connected_service) > 0);
        bool cur_hfp_conn = ((BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP)) & remote_update->connected_service) > 0;
        if (!cur_hfp_conn) {
            cur_hfp_conn = ((BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HSP) & remote_update->connected_service) > 0);
        }
        bool cur_a2dp_conn = ((BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK) & remote_update->connected_service) > 0);

        APPS_LOG_MSGID_I(APP_SPOTIFY_TAP_ACTIVITY_TAG", bt_info_update srv=0x%08X->0x%08X hfp=%d->%d a2dp=%d->%d",
                         6,
                         remote_update->pre_connected_service,
                         remote_update->connected_service,
                         pre_hfp_conn,
                         cur_hfp_conn,
                         pre_a2dp_conn,
                         cur_a2dp_conn);

        if (!cur_hfp_conn && !cur_a2dp_conn && (pre_hfp_conn || pre_a2dp_conn)) {
            /**
             * @brief If A2DP/HFP disconnected, disconnect spotify tap link
             */
            APPS_LOG_MSGID_I(APP_SPOTIFY_TAP_ACTIVITY_TAG", bt_info_update A2DP and HFP disconnected, Disconnect Spotify Tap link", 0);
            spotify_tap_disconnect();
            return false;
        }
    }

    return false;
}

#endif /* AIR_SPOTIFY_TAP_ENABLE */


