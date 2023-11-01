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


/**
 * File: app_gsound_service.c
 *
 * Description: This file is to handle the GSound implementation functions.
 */

#ifdef AIR_GSOUND_ENABLE

#include "app_gsound_service.h"

#include "gsound_api.h"
#include "app_gsound_battery_ohd.h"
#include "app_gsound_device_action.h"
#include "app_gsound_event.h"

#include "apps_config_vp_index_list.h"
#ifdef AIR_MULTI_POINT_ENABLE
#include "app_bt_emp_service.h"
#endif
#include "voice_prompt_api.h"
#ifdef MTK_AWS_MCE_ENABLE
#include "bt_aws_mce_srv.h"
#endif
#include "bt_device_manager.h"
#include "multi_va_manager.h"
#include "nvkey.h"
#include "nvdm.h"
#include "nvkey_id_list.h"
#include "nvdm_id_list.h"
#include "app_gsound_nvkey_struct.h"

#ifdef AIR_BT_FAST_PAIR_ENABLE
#include "app_fast_pair.h"
#endif
#define LOG_TAG "[GS][APP][SRV]"

/* Customer configure option: use own model_id. */
static const char gsound_model_id[] = "google-bisto-dev-v1";

#define APP_GSOUND_VP_SYNC_TIME         100

#define GSOUND_INITIAL_STATE_DISABLED   0
#define GSOUND_INITIAL_STATE_ENABLED    1

#define MODULE_SN_LENGTH_SIZE           5
#define VER_LENGTH_SIZE                 1

static void app_gsound_srv_enabled_callback(void);
static void app_gsound_srv_disabled_callback(void);
static GSoundServiceObserver gsound_srv_observer = {app_gsound_srv_enabled_callback,
                                                    app_gsound_srv_disabled_callback
                                                   };

static GSoundServiceConfig                  gsound_srv_config = {0};
static bool                                 gsound_srv_ignore_first_callback = TRUE;
static uint8_t                              gsound_enable_state = APP_GSOUND_SRV_STATE_DISABLED;
static bool                                 gsound_va_connected = FALSE;

static void app_gsound_srv_write_init_state(uint8_t init_state)
{
#ifdef MTK_NVDM_ENABLE
    uint32_t size = 1;
    nvkey_status_t sta = NVKEY_STATUS_OK;
    sta = nvkey_write_data(NVID_APP_GSOUND_INIT_STATE, (const uint8_t *)&init_state, size);
    GSOUND_LOG_I(LOG_TAG" write_init_state, init_state=%d status=%d", 2, init_state, sta);
#endif
}

static void app_gsound_srv_update_init_state(bool selected)
{
#ifdef MTK_NVDM_ENABLE
    uint32_t size = 1;
    uint8_t init_state = GSOUND_INITIAL_STATE_DISABLED;
    bool update_nvdm = FALSE;
    nvkey_status_t status = nvkey_read_data(NVID_APP_GSOUND_INIT_STATE, (uint8_t *)&init_state, &size);
    GSOUND_LOG_I(LOG_TAG" update_init_state, selected=%d read_status=%d init_state=%d",
                 3, selected, status, init_state);

    if (selected) {
        if ((status == NVKEY_STATUS_ITEM_NOT_FOUND) ||
            ((status == NVKEY_STATUS_OK) && (init_state == GSOUND_INITIAL_STATE_DISABLED))) {
            init_state = GSOUND_INITIAL_STATE_ENABLED;
            update_nvdm = TRUE;
        }
    } else {
        if ((status == NVKEY_STATUS_OK) && (init_state == GSOUND_INITIAL_STATE_ENABLED)) {
            init_state = GSOUND_INITIAL_STATE_DISABLED;
            update_nvdm = TRUE;
        }
    }
    if (update_nvdm) {
        app_gsound_srv_write_init_state(init_state);
    }
#endif
}

static void app_gsound_srv_enabled_callback(void)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    GSOUND_LOG_I(LOG_TAG" enabled_callback, [%02X] ignore=%d", 2, role, gsound_srv_ignore_first_callback);

    gsound_enable_state = APP_GSOUND_SRV_STATE_ENABLED;

    if (gsound_srv_ignore_first_callback) {
        // While app_gsound_srv_init(TRUE), need to ignore the 1st callback.
        gsound_srv_ignore_first_callback = FALSE;
        return;
    }
    app_gsound_srv_write_init_state(GSOUND_INITIAL_STATE_ENABLED);

#ifdef MTK_AWS_MCE_ENABLE
    if (BT_AWS_MCE_ROLE_AGENT != role) {
        return;
    }
#endif
    ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_GSOUND,
                        APPS_EVENT_GSOUND_ENABLED,
                        NULL, 0, NULL, 0);
}

static void app_gsound_srv_disabled_callback(void)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    GSOUND_LOG_I(LOG_TAG" disabled_callback, [%02X] ignore=%d", 2, role, gsound_srv_ignore_first_callback);

    gsound_enable_state = APP_GSOUND_SRV_STATE_DISABLED;

    if (gsound_srv_ignore_first_callback) {
        // While app_gsound_srv_init(FALSE), need to ignore the 1st callback.
        gsound_srv_ignore_first_callback = FALSE;
        return;
    }
    app_gsound_srv_write_init_state(GSOUND_INITIAL_STATE_DISABLED);

#ifdef MTK_AWS_MCE_ENABLE
    if (BT_AWS_MCE_ROLE_AGENT != role) {
        return;
    }
#endif
    ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_GSOUND,
                        APPS_EVENT_GSOUND_DISABLED,
                        NULL, 0, NULL, 0);
}



/**************************************************************************************************
 * GSound Service APP Callback Function
**************************************************************************************************/

static void app_gsound_srv_connected_callback(void)
{
    multi_va_type_t va_type = multi_va_manager_get_current_va_type();
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
#ifdef MTK_AWS_MCE_ENABLE
    if (MULTI_VA_TYPE_GSOUND != va_type || BT_AWS_MCE_ROLE_AGENT != role)
#else
    if (MULTI_VA_TYPE_GSOUND != va_type)
#endif
    {
        GSOUND_LOG_E(LOG_TAG" connected_callback, ERROR [%02X] va_type=%d", 2, role, va_type);
        return;
    }

    GSOUND_LOG_I(LOG_TAG" connected_callback", 0);
    ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_GSOUND,
                        APPS_EVENT_GSOUND_CONNECTED,
                        NULL, 0, NULL, 0);
}

static void app_gsound_srv_disconnected_callback(void)
{
    multi_va_type_t va_type = multi_va_manager_get_current_va_type();
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
#ifdef MTK_AWS_MCE_ENABLE
    if (MULTI_VA_TYPE_GSOUND != va_type || BT_AWS_MCE_ROLE_AGENT != role)
#else
    if (MULTI_VA_TYPE_GSOUND != va_type)
#endif
    {
        GSOUND_LOG_E(LOG_TAG" disconnected_callback, ERROR [%02X] va_type=%d", 2, role, va_type);
        return;
    }

    GSOUND_LOG_I(LOG_TAG" disconnected_callback", 0);
    ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_GSOUND,
                        APPS_EVENT_GSOUND_DISCONNECTED,
                        NULL, 0, NULL, 0);
}

static bool app_gsound_srv_is_charging()
{
    bool is_charging = app_gsound_battery_get_info(APP_GSOUND_BATTERY_INFO_LOCAL_IS_CHARGING);
    GSOUND_LOG_I(LOG_TAG" is_charging, %d", 1, is_charging);
    return is_charging;
}

static uint8_t app_gsound_srv_covert_vp_index(uint8_t clip)
{
    uint8_t vp_index = 0xFF;
    switch (clip) {
        case GSOUND_AUDIO_OUT_CLIP_FETCH:
        case GSOUND_AUDIO_OUT_CLIP_PTT_QUERY:
        case GSOUND_AUDIO_OUT_CLIP_REMOTE_MIC_OPEN:
            vp_index = VP_INDEX_BISTO_MIC_OPEN;
            break;
        case GSOUND_AUDIO_OUT_CLIP_QUERY_INTERRUPTED:
        case GSOUND_AUDIO_OUT_CLIP_PTT_REJECTED:
        case GSOUND_AUDIO_OUT_CLIP_REMOTE_MIC_CLOSE:
        case GSOUND_AUDIO_OUT_CLIP_PTT_MIC_CLOSE:
            vp_index = VP_INDEX_BISTO_MIC_CLOSE;
            break;
        case GSOUND_AUDIO_OUT_CLIP_GSOUND_NC:
            vp_index = VP_INDEX_BISTO_MIC_NOT_CONNECTED;
            break;
        default:
            break;
    }
    return vp_index;
}

static void app_gsound_srv_play_vp(uint8_t clip)
{
    uint32_t vp_index = app_gsound_srv_covert_vp_index(clip);
    if (vp_index != 0xFF) {
        voice_prompt_param_t vp = {0};
        vp.vp_index = vp_index;
        vp.control = VOICE_PROMPT_CONTROL_MASK_SYNC;
        vp.delay_time = APP_GSOUND_VP_SYNC_TIME;
        voice_prompt_play(&vp, NULL);
        //apps_config_set_vp(vp_index, TRUE, APP_GSOUND_VP_SYNC_TIME, VOICE_PROMPT_PRIO_MEDIUM, FALSE, NULL);
    }
}

static void app_gsound_srv_reboot(void)
{
    ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                        APPS_EVENTS_INTERACTION_REQUEST_REBOOT, NULL, 0,
                        NULL, 3000);
}

static bool app_gsound_srv_get_serial_num(uint8_t *serial_num, uint32_t max_len)
{
    uint32_t nvkey_size = 0;
    uint8_t *sn = NULL;
    uint8_t sn_length = 0;

    nvkey_status_t status = nvkey_data_item_length(NVID_APP_GSOUND_INFO, &nvkey_size);
    if (status != NVKEY_STATUS_OK) {
        return FALSE;
    }

    app_gsound_info_nvkey_t *data = (app_gsound_info_nvkey_t *)pvPortMalloc(nvkey_size);
    status = nvkey_read_data(NVID_APP_GSOUND_INFO, (uint8_t *)data, &nvkey_size);
    if (data != NULL && status == NVKEY_STATUS_OK) {
        sn_length = data->serial_num_length;
        sn = (uint8_t *)data;
        memcpy(serial_num, &sn[MODULE_SN_LENGTH_SIZE], (sn_length > max_len) ? max_len : sn_length);
        GSOUND_LOG_I(LOG_TAG" get_serial_num, module_id=0x%08X sn_length=%d",
                     2, data->module_id, sn_length);
    }
    if (data != NULL) {
        vPortFree(data);
    }
    return (sn_length > 0);
}

static bool app_gsound_srv_get_version(char *version, uint32_t max_len)
{
    /* Notice that the customer must write this function again!!!  */
    const char *const_version = "V3.2.0";
    uint32_t version_len = strlen(const_version) + 1;
    memcpy(version, const_version, version_len > max_len ? max_len : version_len);

    return (version_len <= max_len);

}

static bool app_gsound_srv_get_device_id(uint32_t *device_id)
{
//#if defined(AIR_BT_FAST_PAIR_ENABLE) && defined(MTK_AWS_MCE_ENABLE)
#if 0
    // Only earbuds model_id same as fast_pair
    *device_id = app_fast_pair_get_model_id();
    return TRUE;
#else
    uint32_t nvkey_size = 0;
    nvkey_status_t status = nvkey_data_item_length(NVID_APP_GSOUND_INFO, &nvkey_size);
    if (status != NVKEY_STATUS_OK) {
        return FALSE;
    }

    app_gsound_info_nvkey_t *data = (app_gsound_info_nvkey_t *)pvPortMalloc(nvkey_size);
    status = nvkey_read_data(NVID_APP_GSOUND_INFO, (uint8_t *)data, &nvkey_size);
    if (data != NULL && status == NVKEY_STATUS_OK) {
        *device_id = data->module_id;
        GSOUND_LOG_I(LOG_TAG" get_device_id, module_id=0x%08X", 1, data->module_id);
    }
    if (data != NULL) {
        vPortFree(data);
    }
    return (data != NULL && status == NVKEY_STATUS_OK);
#endif
}

static char *app_gsound_srv_get_request_config_id()
{
    return NULL;
}

static char *app_gsound_srv_get_model_id()
{
    return (char *)gsound_model_id;
}

bool app_gsound_srv_check_bt_sink_action(uint32_t sink_action)
{
    uint32_t target_action = GSOUND_ACTION_EVENT_NONE;
    switch (sink_action) {
        case BT_SINK_SRV_ACTION_NEXT_TRACK:
            target_action = GSOUND_TARGET_ACTION_NEXT_TRACK;
            break;
        case BT_SINK_SRV_ACTION_PREV_TRACK:
            target_action = GSOUND_TARGET_ACTION_PREV_TRACK;
            break;
        case BT_SINK_SRV_ACTION_PLAY:
        case BT_SINK_SRV_ACTION_PAUSE:
        case BT_SINK_SRV_ACTION_PLAY_PAUSE:
            target_action = GSOUND_TARGET_ACTION_TOGGLE_PLAY_PAUSE;
            break;
        case GSOUND_TARGET_ACTION_LEGACY_VOICE_ACTIVATION:
            target_action = GSOUND_TARGET_ACTION_LEGACY_VOICE_ACTIVATION;
            break;
        default:
            break;
    }
    GSOUND_LOG_I(LOG_TAG" app_gsound_srv_check_bt_sink_action, 0x%08X->0x%08X",
                 2, sink_action, target_action);
    return (target_action != GSOUND_ACTION_EVENT_NONE);
}

static uint32_t app_gsound_srv_covert_to_bt_sink_action(gsound_app_action_t action)
{
    bt_sink_srv_action_t bt_action = BT_SINK_SRV_ACTION_NONE;
    switch (action) {
        case GSOUND_TARGET_ACTION_NEXT_TRACK:
            bt_action = BT_SINK_SRV_ACTION_NEXT_TRACK;
            break;
        case GSOUND_TARGET_ACTION_PREV_TRACK:
            bt_action = BT_SINK_SRV_ACTION_PREV_TRACK;
            break;
        case GSOUND_TARGET_ACTION_TOGGLE_PLAY_PAUSE:
            bt_action = BT_SINK_SRV_ACTION_PLAY_PAUSE;
            break;
        case GSOUND_TARGET_ACTION_LEGACY_VOICE_ACTIVATION:
            bt_action = GSOUND_TARGET_ACTION_LEGACY_VOICE_ACTIVATION;
            break;
        default:
            GSOUND_LOG_E(LOG_TAG" convert_gsound_action_to_bt_action, error action=%d", 1, action);
            break;
    }
    GSOUND_LOG_I(LOG_TAG" convert_gsound_action_to_bt_action, 0x%08X->0x%08X",
                 2, action, bt_action);
    return bt_action;
}

bool app_gsound_srv_reject_action(uint32_t action)
{
    bool ret = FALSE;
    bt_sink_srv_action_t sink_action = app_gsound_srv_covert_to_bt_sink_action(action);
    if (sink_action != -1) {
        ui_shell_status_t status = ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGHEST,
                                                       EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                                       APPS_EVENTS_INTERACTION_GSOUND_ACTION_REJECTED,
                                                       (void *)sink_action, 0,
                                                       NULL, 0);
        ret = (status == UI_SHELL_STATUS_OK);
    }
    GSOUND_LOG_I(LOG_TAG" app_gsound_srv_reject_action, sink_action=0x%08X ret=%d", 2, sink_action, ret);
    return ret;
}

bool app_gsound_srv_handle_bt_sink_action(bt_sink_srv_action_t sink_action)
{
    bool ret = FALSE;
    if ((sink_action == BT_SINK_SRV_ACTION_PLAY_PAUSE
         || sink_action == BT_SINK_SRV_ACTION_NEXT_TRACK
         || sink_action == BT_SINK_SRV_ACTION_PREV_TRACK)
        && app_gsound_srv_check_bt_sink_action(sink_action)) {
        if (gsound_va_connected) {
            ret = TRUE;
        }
    }
    GSOUND_LOG_I(LOG_TAG" Intercept music sink_action, ret=%d", 1, ret);
    return ret;
}



/**================================================================================*/
/**                                       Service API                              */
/**================================================================================*/
bool gsound_bt_is_emp_enable(void)
{
#ifdef AIR_MULTI_POINT_ENABLE
    return app_bt_emp_is_enable();
#else
    return FALSE;
#endif
}

static gsound_app_callback_param_t  gsound_srv_callback_param = {0};

void app_gsound_srv_enable(void)
{
    GSoundStatus status = GSoundServiceEnable();
    if (status == GSOUND_STATUS_OK) {
        gsound_enable_state = APP_GSOUND_SRV_STATE_ENABLING;
    }
    GSOUND_LOG_I(LOG_TAG" enable, GSoundServiceEnable status=%d", 1, status);
}

void app_gsound_srv_disable(void)
{
    GSoundStatus status = GSoundServiceDisable();
    if (status == GSOUND_STATUS_OK) {
        gsound_enable_state = APP_GSOUND_SRV_STATE_DISABLING;
    }
    GSOUND_LOG_I(LOG_TAG" disable, GSoundServiceDisable status=%d", 1, status);
}

uint8_t app_gsound_srv_get_enable_state(void)
{
    return gsound_enable_state;
}

void app_gsound_srv_set_connected(bool connected)
{
    gsound_va_connected = connected;
}

bool app_gsound_srv_is_connected()
{
    return gsound_va_connected;
}

void app_gsound_srv_va_init(bool va_selected)
{
    gsound_srv_callback_param.notify_app_connected = app_gsound_srv_connected_callback;
    gsound_srv_callback_param.notify_app_disconnected = app_gsound_srv_disconnected_callback;
    gsound_srv_callback_param.notify_app_play_vp = app_gsound_srv_play_vp;
    gsound_srv_callback_param.notify_app_reboot = app_gsound_srv_reboot;
    gsound_srv_callback_param.get_app_is_charging = app_gsound_srv_is_charging;
    gsound_srv_callback_param.get_app_serial_num = app_gsound_srv_get_serial_num;
    gsound_srv_callback_param.get_app_version = app_gsound_srv_get_version;
    gsound_srv_callback_param.get_app_device_id = app_gsound_srv_get_device_id;
    gsound_srv_callback_param.get_app_ohd_state = app_gsound_ohd_get_state;
    gsound_srv_callback_param.get_app_request_config_id = app_gsound_srv_get_request_config_id;
    gsound_srv_callback_param.get_app_model_id = app_gsound_srv_get_model_id;
    gsound_srv_callback_param.reject_action = app_gsound_srv_reject_action;
    gsound_srv_callback_param.device_action_request_state = app_gsound_device_action_request_state;
    gsound_srv_callback_param.device_action_consume = app_gsound_device_action_consume;
    gsound_srv_callback_param.device_action_do_action = app_gsound_device_action_do_action;
    gsound_register_app_callback(&gsound_srv_callback_param);

    app_gsound_srv_update_init_state(va_selected);

    gsound_srv_config.gsound_enabled = va_selected;
    GSoundServiceInit(GSOUND_BUILD_ID, &gsound_srv_config, &gsound_srv_observer);
    GSOUND_LOG_I(LOG_TAG" va_init, GSoundServiceInit va_selected=%d", 1, va_selected);
}

void app_gsound_srv_init()
{
#ifdef MTK_AWS_MCE_ENABLE
    GSoundServiceInitAsTws();
#else
    GSoundServiceInitAsStereo();
#endif
#ifdef AIR_GSOUND_HOTWORD_ENABLE
    GSoundServiceInitHotwordExternal();
#endif
}

#endif /* AIR_GSOUND_ENABLE */
