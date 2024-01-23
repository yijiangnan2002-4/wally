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
#ifdef MTK_ANC_ENABLE

#include "app_anc_service.h"
#include "apps_debug.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"
#include "apps_config_vp_index_list.h"

#if defined(AIR_APP_ANC_SYNC_ENABLE) && defined(AIR_TWS_ENABLE)
#include "apps_aws_sync_event.h"
#endif
#ifdef MTK_RACE_CMD_ENABLE
#include "race_cmd_dsprealtime.h"
#include "race_xport.h"
#endif

#include "bt_connection_manager.h"
#include "voice_prompt_api.h"
#include "ui_shell_manager.h"
#include "FreeRTOS.h"
#include "portable.h"

#ifdef MTK_LEAKAGE_DETECTION_ENABLE
#include "leakage_detection_control.h"
#endif /* MTK_LEAKAGE_DETECTION_ENABLE */

#ifdef AIR_HEARTHROUGH_MAIN_ENABLE
#include "app_hear_through_storage.h"
#include "app_hear_through_activity.h"
#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */

#define LOG_TAG     "[ANC_SRV]"

#ifndef PACKED
#define PACKED  __attribute__((packed))
#endif

typedef enum {
    APP_ANC_STATE_NONE       = 0,
    APP_ANC_STATE_DISABLED,
    APP_ANC_STATE_ENABLED,
} app_anc_state_t;

typedef struct {
    app_anc_state_t                     anc_state;
    uint8_t                             cur_enable;
    audio_anc_control_filter_id_t       cur_filter_id;
    audio_anc_control_type_t            cur_type;
    int16_t                             cur_runtime_gain;
    bool                                user_trigger_testing;
} app_anc_service_context_t;

#if defined(AIR_APP_ANC_SYNC_ENABLE) && defined(AIR_TWS_ENABLE)
typedef struct {
#ifdef AIR_HEARTHROUGH_MAIN_ENABLE
    bool                                ht_enabled;
#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */
    bool                                enable;
    audio_anc_control_filter_id_t       filter_id;
    audio_anc_control_type_t            anc_type;
    int16_t                             runtime_gain;
} PACKED app_anc_service_aws_sync_info_t;

typedef struct {
    audio_anc_control_type_t            anc_type;
    int16_t                             runtime_gain;
} PACKED app_anc_service_aws_sync_gain_t;

/**
 * @brief Fix issue customer issue -156
 * That if agent is switching link in EMP case, the agent is not sync the newest configuration
 * to partner side.
 */
typedef struct {
    bool                                is_not_sent;
    app_anc_service_aws_sync_info_t     sync_info;
} PACKED app_anc_service_aws_un_synced_info_t;

static app_anc_service_aws_un_synced_info_t g_aws_un_synced_info = {0};

#define APP_ANC_SERVICE_REALTIME_SYNC_DELAY_MS        (200)       // 200ms

#endif /* AIR_APP_ANC_SYNC_ENABLE && AIR_TWS_ENABLE */

static app_anc_service_context_t        g_anc_srv_context;

#ifdef AIR_HEARTHROUGH_MAIN_ENABLE
typedef struct {
    bool                                is_suspended;
    bool                                hear_through_enabled;
    uint8_t                             enable;
    audio_anc_control_filter_id_t       cur_filter_id;
    audio_anc_control_type_t            cur_type;
    audio_anc_control_misc_t            cur_control_misc;
    int16_t                             cur_runtime_gain;
} app_anc_service_ht_context_t;

static app_anc_service_ht_context_t   g_anc_srv_ht_context;

#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */



/**================================================================================*/
/**                                 Internal Function                              */
/**================================================================================*/
static audio_anc_control_result_t app_anc_service_local_control(bool enable,
                                                                audio_anc_control_filter_id_t filter_id,
                                                                audio_anc_control_type_t anc_type,
                                                                audio_anc_control_gain_t runtime_gain,
                                                                audio_anc_control_misc_t *control_misc)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    audio_anc_control_result_t anc_result = AUDIO_ANC_CONTROL_EXECUTION_NONE;

    if (enable) {
        anc_result = audio_anc_control_set_runtime_gain(runtime_gain, anc_type);
        anc_result += audio_anc_control_enable(filter_id, anc_type, control_misc);
        APPS_LOG_MSGID_I(LOG_TAG" control, [%02X] enable filter_id=%d anc_type=0x%04X runtime_gain=0x%08X anc_result=%d",
                         5, role, filter_id, anc_type, runtime_gain, anc_result);
    } else {
        anc_result = audio_anc_control_disable(NULL);
        APPS_LOG_MSGID_I(LOG_TAG" control, [%02X] disable anc_result=%d",
                         2, role, anc_result);
    }
    return anc_result;
}

#if defined(AIR_APP_ANC_SYNC_ENABLE) && defined(AIR_TWS_ENABLE)
static void app_anc_srv_reset_un_synced_info()
{
    memset(&g_aws_un_synced_info, 0, sizeof(app_anc_service_aws_un_synced_info_t));
}
#endif /* AIR_APP_ANC_SYNC_ENABLE && AIR_TWS_ENABLE */

static audio_anc_control_result_t app_anc_service_control(bool sync, bool enable,
                                                          audio_anc_control_filter_id_t filter_id,
                                                          audio_anc_control_type_t anc_type,
                                                          audio_anc_control_gain_t runtime_gain,
                                                          audio_anc_control_misc_t *control_misc)
{
    APPS_LOG_MSGID_I(LOG_TAG"[app_anc_service_control] Sync : %d, enable : %d, type : 0x%04x, filter_id : 0x%04x, runtime_gain : 0x%04x",
                        5,
                        sync,
                        enable,
                        anc_type,
                        filter_id,
                        runtime_gain);

#if defined(AIR_APP_ANC_SYNC_ENABLE) && defined(AIR_TWS_ENABLE)
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    if (role == BT_AWS_MCE_ROLE_PARTNER) {
        sync = FALSE;
    }
#else
    sync = FALSE;
#endif

    audio_anc_control_result_t anc_result = AUDIO_ANC_CONTROL_EXECUTION_NONE;
#if defined(AIR_APP_ANC_SYNC_ENABLE) && defined(AIR_TWS_ENABLE)
    bt_aws_mce_srv_link_type_t aws_link_type = bt_aws_mce_srv_get_link_type();

    APPS_LOG_MSGID_I(LOG_TAG"[app_anc_service_control][TWS] sync : %d, role : 0x%02x, aws_link_type : 0x%02x",
                        3,
                        sync,
                        role,
                        aws_link_type);

    app_anc_service_aws_sync_info_t sync_info = {0};
    sync_info.enable = enable;
    if (sync_info.enable) {
        sync_info.filter_id = filter_id;
        sync_info.anc_type = anc_type;
        sync_info.runtime_gain = runtime_gain;
    }

    if (sync && role == BT_AWS_MCE_ROLE_AGENT && aws_link_type != BT_AWS_MCE_SRV_LINK_NONE) {
        // Real time sync and control ANC

        apps_aws_sync_send_future_sync_event(FALSE, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                                APPS_EVENTS_INTERACTION_APP_ANC_SRV_ANC_ACTION,
                                                TRUE, (uint8_t *)&sync_info, sizeof(app_anc_service_aws_sync_info_t),
                                                APP_ANC_SERVICE_REALTIME_SYNC_DELAY_MS);

        /**
         * @brief Fix issue - 156
         * If send data failed, need store the data in the un-synced buffer,  and when AWS connected, need sync
         * the buffer data to partner side.
         */
        app_anc_srv_reset_un_synced_info();

        g_aws_un_synced_info.is_not_sent = true;
        memcpy(&(g_aws_un_synced_info.sync_info), &sync_info, sizeof(app_anc_service_aws_sync_info_t));

        anc_result = AUDIO_ANC_CONTROL_EXECUTION_SUCCESS;
    } else {
        /**
         * @brief Fix issue - 156
         * If AWS is not connected and need sync and current is agent role, need store the data
         * into the un_synced info to wait to send it after AWS connected.
         */
        if (sync == true && role == BT_AWS_MCE_ROLE_AGENT) {
            app_anc_srv_reset_un_synced_info();

            g_aws_un_synced_info.is_not_sent = true;
            memcpy(&(g_aws_un_synced_info.sync_info), &sync_info, sizeof(app_anc_service_aws_sync_info_t));
        }
        anc_result = app_anc_service_local_control(enable, filter_id, anc_type, runtime_gain, control_misc);
    }
#else
    anc_result = app_anc_service_local_control(enable, filter_id, anc_type, runtime_gain, control_misc);
#endif
    return anc_result;
}

static audio_anc_control_result_t app_anc_service_runtime_gain_control(bool sync,
                                                                       audio_anc_control_type_t anc_type,
                                                                       audio_anc_control_gain_t runtime_gain)
{
#if defined(AIR_APP_ANC_SYNC_ENABLE) && defined(AIR_TWS_ENABLE)
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    if (role == BT_AWS_MCE_ROLE_PARTNER) {
        sync = FALSE;
    }
#else
    sync = FALSE;
#endif

    audio_anc_control_result_t anc_result = AUDIO_ANC_CONTROL_EXECUTION_NONE;
#if defined(AIR_APP_ANC_SYNC_ENABLE) && defined(AIR_TWS_ENABLE)
    bt_aws_mce_srv_link_type_t aws_link_type = bt_aws_mce_srv_get_link_type();
    if (sync && role == BT_AWS_MCE_ROLE_AGENT && aws_link_type != BT_AWS_MCE_SRV_LINK_NONE) {
        // Real time sync and control ANC
        app_anc_service_aws_sync_gain_t sync_gain = {0};
        sync_gain.anc_type = anc_type;
        sync_gain.runtime_gain = runtime_gain;
        apps_aws_sync_send_future_sync_event(FALSE, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                             APPS_EVENTS_INTERACTION_APP_ANC_SRV_GAIN_ACTION,
                                             TRUE, (uint8_t *)&sync_gain, sizeof(app_anc_service_aws_sync_gain_t),
                                             APP_ANC_SERVICE_REALTIME_SYNC_DELAY_MS);
        anc_result = AUDIO_ANC_CONTROL_EXECUTION_SUCCESS;
    } else {
        anc_result = audio_anc_control_set_runtime_gain(runtime_gain, anc_type);
    }
#else
    anc_result = audio_anc_control_set_runtime_gain(runtime_gain, anc_type);
#endif
    return anc_result;
}

static void app_anc_srv_update_parameter()
{
    uint8_t                         anc_enable = 0;
    audio_anc_control_filter_id_t   anc_filter_id = AUDIO_ANC_CONTROL_FILTER_ID_NONE;
    audio_anc_control_type_t        anc_type = AUDIO_ANC_CONTROL_TYPE_DUMMY;
    int16_t                         anc_runtime_gain = 0;
    uint8_t                         support_hybrid_enable = 0;
    audio_anc_control_misc_t        anc_control_misc = {0};

    audio_anc_control_get_status(&anc_enable,
                                 &anc_filter_id,
                                 &anc_type,
                                 &anc_runtime_gain,
                                 &support_hybrid_enable,
                                 &anc_control_misc);
    APPS_LOG_MSGID_I(LOG_TAG" update_parameter, enable=%d filter_id=%d type=0x%04X runtime_gain=0x%08X support_hybrid=%d misc=0x%08X",
                     6, anc_enable, anc_filter_id, anc_type, anc_runtime_gain, support_hybrid_enable, anc_control_misc.extend_use_parameters);
    g_anc_srv_context.cur_enable = anc_enable;
    g_anc_srv_context.cur_filter_id = anc_filter_id;
    g_anc_srv_context.cur_type = anc_type;
    g_anc_srv_context.cur_runtime_gain = anc_runtime_gain;
    if (anc_type == AUDIO_ANC_CONTROL_TYPE_HYBRID && support_hybrid_enable == 0) {
        g_anc_srv_context.cur_type = AUDIO_ANC_CONTROL_TYPE_FF;
    }

#ifdef AIR_HEARTHROUGH_MAIN_ENABLE
    APPS_LOG_MSGID_I(LOG_TAG"[HearThrough] update_parameter, hear_through_enabled : %d", 1, g_anc_srv_ht_context.hear_through_enabled);
    if (g_anc_srv_ht_context.hear_through_enabled == false) {
        g_anc_srv_ht_context.cur_type = g_anc_srv_context.cur_type;
        g_anc_srv_ht_context.enable = g_anc_srv_context.cur_enable;
        g_anc_srv_ht_context.cur_filter_id = g_anc_srv_context.cur_filter_id;
        g_anc_srv_ht_context.cur_runtime_gain = g_anc_srv_context.cur_runtime_gain;

        memcpy(&g_anc_srv_ht_context.cur_control_misc, &anc_control_misc, sizeof(audio_anc_control_misc_t));

        APPS_LOG_MSGID_I(LOG_TAG"[HearThrough] update_parameter, enable : %d, type : %d, filter_id : %d, runtime_gain : %d",
                         4,
                         g_anc_srv_ht_context.enable,
                         g_anc_srv_ht_context.cur_type,
                         g_anc_srv_ht_context.cur_filter_id,
                         g_anc_srv_ht_context.cur_runtime_gain);
    }
#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */
}

static void app_anc_srv_control_callback(audio_anc_control_event_t event_id, audio_anc_control_result_t result)
{
    APPS_LOG_MSGID_I(LOG_TAG" control_callback, event_id=%d result=%d", 2, event_id, result);
    if (result == AUDIO_ANC_CONTROL_EXECUTION_SUCCESS) {
        if (event_id == AUDIO_ANC_CONTROL_EVENT_ON) {
            g_anc_srv_context.anc_state = APP_ANC_STATE_ENABLED;
            app_anc_srv_update_parameter();
        } else if (event_id == AUDIO_ANC_CONTROL_EVENT_OFF) {
            g_anc_srv_context.anc_state = APP_ANC_STATE_DISABLED;
            g_anc_srv_context.cur_enable = FALSE;

            /**
             * @brief Fix issue - 46308
             * When ANC has been disabled, if the HT is not enabled, need update the parameter
             */
#ifdef AIR_HEARTHROUGH_MAIN_ENABLE
            APPS_LOG_MSGID_I(LOG_TAG"[HearThrough] control_callback, hear_through_enabled : %d", 1, g_anc_srv_ht_context.hear_through_enabled);
            if (g_anc_srv_ht_context.hear_through_enabled == false) {
                g_anc_srv_ht_context.enable = false;
            }
#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */
        }

        app_anc_srv_result_t *anc_result = (app_anc_srv_result_t *)pvPortMalloc(sizeof(app_anc_srv_result_t));
        if (anc_result != NULL) {
            anc_result->cur_filter_id = g_anc_srv_context.cur_filter_id;
            anc_result->cur_type = g_anc_srv_context.cur_type;
            anc_result->cur_runtime_gain = g_anc_srv_context.cur_runtime_gain;

            ui_shell_send_event(FALSE, EVENT_PRIORITY_HIGHEST,
                                EVENT_GROUP_UI_SHELL_AUDIO_ANC,
                                event_id, anc_result, sizeof(app_anc_srv_result_t),
                                NULL, 0);
        }
    }
}

#if defined(AIR_APP_ANC_SYNC_ENABLE) && defined(AIR_TWS_ENABLE)
static void app_anc_service_send_sync_info(void)
{
// #ifdef AIR_HEARTHROUGH_MAIN_ENABLE
//     if (g_anc_srv_ht_context.hear_through_enabled == true) {
//         APPS_LOG_MSGID_I(LOG_TAG" [HearThrough] send_sync_info, hear through enabled, ignore the sync information", 0);
//         return;
//     }
// #endif /* AIR_HEARTHROUGH_MAIN_ENABLE */

    app_anc_srv_update_parameter();
    app_anc_service_aws_sync_info_t sync_info = {0};

#ifdef AIR_HEARTHROUGH_MAIN_ENABLE
    sync_info.ht_enabled = g_anc_srv_ht_context.hear_through_enabled;
    if (sync_info.ht_enabled == false)
#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */
    {
        /**
         * @brief Fix issue - 156
         * If the un-synced info is not sent to partner side, need send the newest synced info to partner side.
         */
        APPS_LOG_MSGID_I(LOG_TAG"[app_anc_service_send_sync_info] un_synced_info, not_sent : %d, enable : %d, anc_type : 0x%04x, filter_id : 0x%04x, runtime_gain : 0x%08x",
                            5,
                            g_aws_un_synced_info.is_not_sent,
                            g_aws_un_synced_info.sync_info.enable,
                            g_aws_un_synced_info.sync_info.anc_type,
                            g_aws_un_synced_info.sync_info.filter_id,
                            g_aws_un_synced_info.sync_info.runtime_gain);

        if ((g_aws_un_synced_info.is_not_sent == true)
            && ((g_aws_un_synced_info.sync_info.anc_type != g_anc_srv_context.cur_type)
                || (g_aws_un_synced_info.sync_info.enable != g_anc_srv_context.cur_enable)
                || (g_aws_un_synced_info.sync_info.filter_id != g_anc_srv_context.cur_filter_id)
                || (g_aws_un_synced_info.sync_info.runtime_gain != g_anc_srv_context.cur_runtime_gain))) {
            memcpy(&sync_info, &(g_aws_un_synced_info.sync_info), sizeof(app_anc_service_aws_sync_info_t));
        } else {
            sync_info.enable = g_anc_srv_context.cur_enable;
            if (sync_info.enable) {
                sync_info.filter_id = g_anc_srv_context.cur_filter_id;
                sync_info.anc_type = g_anc_srv_context.cur_type;
                sync_info.runtime_gain = g_anc_srv_context.cur_runtime_gain;
            }
        }

        app_anc_srv_reset_un_synced_info();
    }
#ifdef AIR_HEARTHROUGH_MAIN_ENABLE
    else {
        sync_info.enable = g_anc_srv_ht_context.enable;
        sync_info.filter_id = g_anc_srv_ht_context.cur_filter_id;
        sync_info.anc_type = g_anc_srv_ht_context.cur_type;
        sync_info.runtime_gain = g_anc_srv_ht_context.cur_runtime_gain;
    }
#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */

    bt_status_t bt_status = apps_aws_sync_event_send_extra(EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                                           APPS_EVENTS_INTERACTION_APP_ANC_SRV_SYNC_INFO,
                                                           &sync_info, sizeof(app_anc_service_aws_sync_info_t));

#ifdef AIR_HEARTHROUGH_MAIN_ENABLE
    APPS_LOG_MSGID_I(LOG_TAG" send_sync_info, ht_enabled=%d, enable=%d filter_id=%d anc_type=%d runtime_gain=0x%08X bt_status=0x%08X",
                        6,
                        sync_info.ht_enabled,
                        sync_info.enable,
                        sync_info.filter_id,
                        sync_info.anc_type,
                        sync_info.runtime_gain,
                        bt_status);
#else
    APPS_LOG_MSGID_I(LOG_TAG" send_sync_info, enable=%d filter_id=%d anc_type=%d runtime_gain=0x%08X bt_status=0x%08X",
                     5, sync_info.enable, sync_info.filter_id, sync_info.anc_type, sync_info.runtime_gain, bt_status);
#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */
}

static void app_anc_service_handle_sync_info(uint8_t *extra_data)
{
    app_anc_service_aws_sync_info_t *sync_info = (app_anc_service_aws_sync_info_t *)extra_data;
    bool enable = sync_info->enable;
    audio_anc_control_filter_id_t filter_id = sync_info->filter_id;
    audio_anc_control_type_t anc_type = sync_info->anc_type;
    int16_t runtime_gain = sync_info->runtime_gain;

#ifdef AIR_HEARTHROUGH_MAIN_ENABLE
    if (sync_info->ht_enabled == false) {
        /**
         * @brief Fix issue : 47152
         * When ANC sync info happen, need notify ANC changed.
         */
        app_hear_through_activity_handle_anc_switched(false, enable);
    }
#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */

#ifdef AIR_HEARTHROUGH_MAIN_ENABLE
    if (sync_info->ht_enabled == false)
#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */
    {
        if (enable != g_anc_srv_context.cur_enable) {
            APPS_LOG_MSGID_I(LOG_TAG" handle_sync_info, enable state diff own=%d peer=%d",
                            2, g_anc_srv_context.cur_enable, enable);
        } else if (enable && (filter_id != g_anc_srv_context.cur_filter_id || anc_type != g_anc_srv_context.cur_type || runtime_gain != g_anc_srv_context.cur_runtime_gain)) {
            APPS_LOG_MSGID_I(LOG_TAG" handle_sync_info, enable filter_id=%d-%d anc_type=0x%04X-0x%04X runtime_gain=0x%08X-0x%08X",
                            6, g_anc_srv_context.cur_filter_id, filter_id, g_anc_srv_context.cur_type,
                            anc_type, g_anc_srv_context.cur_runtime_gain, runtime_gain);
        } else {
            APPS_LOG_MSGID_W(LOG_TAG" handle_sync_info, no need", 0);
            return;
        }

        audio_anc_control_result_t anc_result = app_anc_service_control(FALSE, enable, filter_id, anc_type, runtime_gain, NULL);
        APPS_LOG_MSGID_I(LOG_TAG" handle_sync_info, enable=%d filter_id=%d anc_type=0x%04X runtime_gain=0x%08X anc_result=%d",
                        5, enable, filter_id, anc_type, runtime_gain, anc_result);
    }
#ifdef AIR_HEARTHROUGH_MAIN_ENABLE
    else {
        g_anc_srv_ht_context.enable = sync_info->enable;
        g_anc_srv_ht_context.cur_filter_id = sync_info->filter_id;
        g_anc_srv_ht_context.cur_runtime_gain = sync_info->runtime_gain;
        g_anc_srv_ht_context.cur_type = sync_info->anc_type;

        APPS_LOG_MSGID_I(LOG_TAG" handle_sync_info, sync HT configuration, enable : %d, type : %d, filter_id : %d, gain : %d",
                            4,
                            g_anc_srv_ht_context.enable,
                            g_anc_srv_ht_context.cur_type,
                            g_anc_srv_ht_context.cur_filter_id,
                            g_anc_srv_ht_context.cur_runtime_gain);
    }
#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */
}
#endif



/**================================================================================*/
/**                                     Public API                                 */
/**================================================================================*/
void app_anc_service_init(void)
{
#ifdef AIR_APP_ANC_SYNC_ENABLE
    audio_anc_control_set_attach_enable(AUDIO_ANC_CONTROL_ATTACH_MODE_2);
#endif

    audio_anc_control_result_t anc_result = audio_anc_control_register_callback(app_anc_srv_control_callback,
                                                                                AUDIO_ANC_CONTROL_EVENT_ON
                                                                                | AUDIO_ANC_CONTROL_EVENT_OFF
                                                                                | AUDIO_ANC_CONTROL_EVENT_COPY_FILTER
                                                                                | AUDIO_ANC_CONTROL_EVENT_SET_REG
                                                                                | AUDIO_ANC_CONTROL_EVENT_FORCE_OFF
                                                                                | AUDIO_ANC_CONTROL_EVENT_HOWLING
                                                                                | AUDIO_ANC_CONTROL_EVENT_SUSPEND_NOTIFY,
                                                                                AUDIO_ANC_CONTROL_CB_LEVEL_ALL);
    //APPS_LOG_MSGID_I(LOG_TAG" init, register_callback anc_result=%d", 1, anc_result);

    anc_result = audio_anc_control_get_status_from_flash(&g_anc_srv_context.cur_enable,
                                                         &g_anc_srv_context.cur_filter_id,
                                                         &g_anc_srv_context.cur_type,
                                                         &g_anc_srv_context.cur_runtime_gain,
#ifdef AIR_HEARTHROUGH_MAIN_ENABLE
                                                         & (g_anc_srv_ht_context.cur_control_misc)
#else
                                                         NULL
#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */
                                                        );
    if (anc_result != AUDIO_ANC_CONTROL_EXECUTION_SUCCESS) {
        APPS_LOG_MSGID_E(LOG_TAG" init, get_status_from_flash fail anc_result=%d", 1, anc_result);
    }
    APPS_LOG_MSGID_I(LOG_TAG" init, enable=%d filter_id=%d type=%d runtime_gain=0x%08X",
                     4, g_anc_srv_context.cur_enable, g_anc_srv_context.cur_filter_id,
                     g_anc_srv_context.cur_type, g_anc_srv_context.cur_runtime_gain);

#ifdef AIR_HEARTHROUGH_MAIN_ENABLE
    memset(&g_anc_srv_ht_context, 0, sizeof(app_anc_service_ht_context_t));

    g_anc_srv_ht_context.hear_through_enabled = false;
    g_anc_srv_ht_context.enable = g_anc_srv_context.cur_enable;
    g_anc_srv_ht_context.cur_filter_id = g_anc_srv_context.cur_filter_id;
    g_anc_srv_ht_context.cur_runtime_gain = g_anc_srv_context.cur_runtime_gain;
    g_anc_srv_ht_context.cur_type = g_anc_srv_context.cur_type;

    APPS_LOG_MSGID_I(LOG_TAG"[HearThrough] init, enable : %d, filter_id : %d, type : %d, runtime_gain : %d",
                     4,
                     g_anc_srv_ht_context.enable,
                     g_anc_srv_ht_context.cur_filter_id,
                     g_anc_srv_ht_context.cur_type,
                     g_anc_srv_ht_context.cur_runtime_gain);
#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */

    g_anc_srv_context.anc_state = APP_ANC_STATE_NONE;
    g_anc_srv_context.user_trigger_testing = FALSE;

    if (g_anc_srv_context.cur_enable == 0) {
        APPS_LOG_MSGID_I(LOG_TAG" init, no need to disable", 0);
        return;
    }

    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    anc_result = app_anc_service_control(FALSE, TRUE,
                                         g_anc_srv_context.cur_filter_id,
                                         g_anc_srv_context.cur_type,
                                         g_anc_srv_context.cur_runtime_gain,
                                         NULL);
    APPS_LOG_MSGID_I(LOG_TAG" init, [%02X] anc_result=%d", 2, role, anc_result);
}

void app_anc_service_save_into_flash(void)
{
    app_anc_srv_update_parameter();

#ifdef AIR_HEARTHROUGH_MAIN_ENABLE
    audio_anc_control_result_t anc_result = AUDIO_ANC_CONTROL_EXECUTION_SUCCESS;
    bool enable = false;
    /**
     * @brief Fix issue, if hear through is enabled, need save ANC as disabled state.
     * Then power on device, will resume it as disable state, and hear through will enable
     * ANC with target configuration.
     */
    if (app_hear_through_storage_get_hear_through_switch() == false) {
        enable = g_anc_srv_ht_context.enable;
    }
    anc_result = audio_anc_control_set_status_into_flash(enable,
                                                         g_anc_srv_ht_context.cur_filter_id,
                                                         g_anc_srv_ht_context.cur_type,
                                                         g_anc_srv_ht_context.cur_runtime_gain,
                                                         &(g_anc_srv_ht_context.cur_control_misc));

    APPS_LOG_MSGID_I(LOG_TAG" [HearThrough] save_into_flash, enable : %d, filter_id : %d, type : %d, gain : %d, anc_result : %d",
                     5,
                     enable,
                     g_anc_srv_ht_context.cur_filter_id,
                     g_anc_srv_ht_context.cur_type,
                     g_anc_srv_ht_context.cur_runtime_gain,
                     anc_result);
#else
    audio_anc_control_result_t anc_result = audio_anc_control_set_status_into_flash(g_anc_srv_context.cur_enable,
                                                                                    g_anc_srv_context.cur_filter_id,
                                                                                    g_anc_srv_context.cur_type,
                                                                                    g_anc_srv_context.cur_runtime_gain,
                                                                                    NULL);
    APPS_LOG_MSGID_I(LOG_TAG" save_into_flash, anc_result=%d", 1, anc_result);
#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */
}

bool app_anc_service_is_enable(void)
{
    return (g_anc_srv_context.cur_enable > 0);
}

bool app_anc_service_enable(audio_anc_control_filter_id_t filter_id,
                            audio_anc_control_type_t anc_type,
                            audio_anc_control_gain_t runtime_gain,
                            audio_anc_control_misc_t *control_misc)
{
    if (g_anc_srv_context.user_trigger_testing) {
        APPS_LOG_MSGID_W(LOG_TAG" user trigger test ongoing, cannot enable ANC", 0);
        return FALSE;
    }

    APPS_LOG_MSGID_I(LOG_TAG"[app_anc_service_enable] Enable, type : 0x%04x, filter_id : 0x0%04x, gain : 0x%04x",
                        3,
                        anc_type,
                        filter_id,
                        runtime_gain);

#ifdef MTK_LEAKAGE_DETECTION_ENABLE
    if (audio_anc_leakage_compensation_get_status() == true) {
        APPS_LOG_MSGID_W(LOG_TAG" leakage detection is ongoing, cannot enable ANC", 0);
        return false;
    }
#endif /* MTK_LEAKAGE_DETECTION_ENABLE */

#ifdef AIR_HEARTHROUGH_MAIN_ENABLE
    if (anc_type == AUDIO_ANC_CONTROL_TYPE_PASSTHRU_FF) {
        app_hear_through_activity_switch_to_hear_through();
        return true;
    }

    app_hear_through_activity_handle_anc_switched(true, true);
#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */

    audio_anc_control_result_t anc_result = app_anc_service_control(TRUE, TRUE, filter_id, anc_type, runtime_gain, control_misc);
#ifndef AIR_HEARTHROUGH_MAIN_ENABLE
    voice_prompt_play_sync_vp_succeed();
#endif /* !AIR_HEARTHROUGH_MAIN_ENABLE */
    return (anc_result == AUDIO_ANC_CONTROL_EXECUTION_SUCCESS);
}

bool app_anc_service_disable()
{
    if (g_anc_srv_context.user_trigger_testing) {
        APPS_LOG_MSGID_W(LOG_TAG" user trigger test ongoing, cannot disable ANC", 0);
        return FALSE;
    }

    APPS_LOG_MSGID_I(LOG_TAG"[app_anc_service_disable] Disable", 0);

#ifdef MTK_LEAKAGE_DETECTION_ENABLE
    if (audio_anc_leakage_compensation_get_status() == true) {
        APPS_LOG_MSGID_W(LOG_TAG" leakage detection is ongoing, cannot disable ANC", 0);
        return false;
    }
#endif /* MTK_LEAKAGE_DETECTION_ENABLE */

#ifdef AIR_HEARTHROUGH_MAIN_ENABLE
    app_hear_through_activity_handle_anc_switched(true, false);
#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */

    audio_anc_control_result_t anc_result = app_anc_service_control(TRUE, FALSE, 0, 0, 0, NULL);

#ifndef AIR_HEARTHROUGH_MAIN_ENABLE
    voice_prompt_play_sync_vp_succeed();
#endif /* !AIR_HEARTHROUGH_MAIN_ENABLE */
    return (anc_result == AUDIO_ANC_CONTROL_EXECUTION_SUCCESS);
}

bool app_anc_service_set_runtime_gain(audio_anc_control_type_t anc_type,
                                      audio_anc_control_gain_t runtime_gain)
{
    if (g_anc_srv_context.user_trigger_testing) {
        APPS_LOG_MSGID_W(LOG_TAG" user trigger test ongoing, cannot set runtime gain", 0);
        return FALSE;
    }

#ifdef MTK_LEAKAGE_DETECTION_ENABLE
    if (audio_anc_leakage_compensation_get_status() == true) {
        APPS_LOG_MSGID_W(LOG_TAG" leakage detection is ongoing, cannot set runtime gain", 0);
        return false;
    }
#endif /* MTK_LEAKAGE_DETECTION_ENABLE */

    audio_anc_control_result_t anc_result = app_anc_service_runtime_gain_control(TRUE, anc_type, runtime_gain);
    if (anc_result == AUDIO_ANC_CONTROL_EXECUTION_SUCCESS) {
        g_anc_srv_context.cur_runtime_gain = runtime_gain;
    }
//    APPS_LOG_MSGID_I(LOG_TAG" set_runtime_gain, anc_type=%d gain=0x%08X anc_result=%d",
//                     3, anc_type, runtime_gain, anc_result);
    return (anc_result == AUDIO_ANC_CONTROL_EXECUTION_SUCCESS);
}

bool app_anc_service_suspend(void)
{
    if (g_anc_srv_context.user_trigger_testing) {
        APPS_LOG_MSGID_W(LOG_TAG" user trigger test ongoing, cannot suspend ANC", 0);
        return FALSE;
    }

#ifdef MTK_LEAKAGE_DETECTION_ENABLE
    if (audio_anc_leakage_compensation_get_status() == true) {
        APPS_LOG_MSGID_W(LOG_TAG" leakage detection is ongoing, cannot suspend ANC", 0);
        return false;
    }
#endif /* MTK_LEAKAGE_DETECTION_ENABLE */

    app_anc_srv_update_parameter();

    audio_anc_control_misc_t anc_enable_param = {0};
    anc_enable_param.disable_with_suspend = TRUE;
    audio_anc_control_result_t anc_result = audio_anc_control_disable(&anc_enable_param);
    APPS_LOG_MSGID_I(LOG_TAG" suspend, anc_result=%d", 1, anc_result);

#ifdef AIR_HEARTHROUGH_MAIN_ENABLE
    if (anc_result == AUDIO_ANC_CONTROL_EXECUTION_SUCCESS) {
        g_anc_srv_ht_context.is_suspended = true;
    }
#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */

    return (anc_result == AUDIO_ANC_CONTROL_EXECUTION_SUCCESS);
}

bool app_anc_service_resume(void)
{
    if (g_anc_srv_context.user_trigger_testing) {
        APPS_LOG_MSGID_W(LOG_TAG" user trigger test ongoing, cannot resume ANC", 0);
        return FALSE;
    }

#ifdef MTK_LEAKAGE_DETECTION_ENABLE
    if (audio_anc_leakage_compensation_get_status() == true) {
        APPS_LOG_MSGID_W(LOG_TAG" leakage detection is ongoing, cannot resume ANC", 0);
        return false;
    }
#endif /* MTK_LEAKAGE_DETECTION_ENABLE */

    app_anc_srv_update_parameter();

    audio_anc_control_misc_t anc_enable_param = {0};
    anc_enable_param.enable_with_resume = TRUE;
    audio_anc_control_result_t anc_result = audio_anc_control_enable(g_anc_srv_context.cur_filter_id, g_anc_srv_context.cur_type, &anc_enable_param);
    APPS_LOG_MSGID_I(LOG_TAG" resume, anc_result=%d", 1, anc_result);

#ifdef AIR_HEARTHROUGH_MAIN_ENABLE
    if (anc_result == AUDIO_ANC_CONTROL_EXECUTION_SUCCESS) {
        g_anc_srv_ht_context.is_suspended = false;
    }
#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */

    return (anc_result == AUDIO_ANC_CONTROL_EXECUTION_SUCCESS);
}

bool app_anc_service_reinit_nvdm()
{
    audio_anc_control_result_t anc_result = audio_anc_control_reinit_nvdm();
    APPS_LOG_MSGID_I(LOG_TAG" reinit_nvdm result=%d", 1, anc_result);
    return (anc_result == AUDIO_ANC_CONTROL_EXECUTION_SUCCESS);
}

void app_anc_service_set_user_trigger_state(bool ongoing)
{
    APPS_LOG_MSGID_I(LOG_TAG" set user trigger state=%d", 1, ongoing);
    g_anc_srv_context.user_trigger_testing = ongoing;
}

app_anc_service_state_t app_anc_service_get_state(void)
{
    app_anc_service_state_t state = APP_ANC_STATE_DISABLE;
    uint8_t                         anc_enable = 0;
    audio_anc_control_type_t        anc_type = AUDIO_ANC_CONTROL_TYPE_DUMMY;
    audio_anc_control_get_status(&anc_enable, NULL, &anc_type,
                                 NULL, NULL, NULL);

    if (anc_enable && (anc_type < AUDIO_ANC_CONTROL_TYPE_PASSTHRU_FF || anc_type == AUDIO_ANC_CONTROL_TYPE_FULL_ADAPT)) {
        state = APP_ANC_STATE_ANC_ENABLE;
    } else if (anc_enable && anc_type != AUDIO_ANC_CONTROL_TYPE_DUMMY) {
        state = APP_ANC_STATE_PT_ENABLE;
    }
    APPS_LOG_MSGID_I(LOG_TAG" [ADVANCE_PT] get_state, anc_enable=%d anc_type=0x%04X state=%d",
                     3, anc_enable, anc_type, state);
    return state;
}

void app_anc_service_handle_event(uint32_t event_group, uint32_t event_id, void *extra_data, size_t data_len)
{
#if defined(AIR_APP_ANC_SYNC_ENABLE) && defined(AIR_TWS_ENABLE)
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    if (event_group == EVENT_GROUP_UI_SHELL_BT_CONN_MANAGER && event_id == BT_CM_EVENT_REMOTE_INFO_UPDATE) {
        bt_cm_remote_info_update_ind_t *remote_update = (bt_cm_remote_info_update_ind_t *)extra_data;
        if (remote_update != NULL) {
            bool aws_conntected = (!(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service)
                                   && (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service));
            if (role == BT_AWS_MCE_ROLE_AGENT && aws_conntected) {
                APPS_LOG_MSGID_I(LOG_TAG"[app_anc_service_handle_event] AWS connected, sync info", 0);
                app_anc_service_send_sync_info();
            }
        }
    }

    if (event_group == EVENT_GROUP_UI_SHELL_APP_INTERACTION && event_id == APPS_EVENTS_INTERACTION_APP_ANC_SRV_ANC_ACTION) {
        apps_aws_sync_future_event_local_event_t *local_event = (apps_aws_sync_future_event_local_event_t *)extra_data;
        app_anc_service_aws_sync_info_t *anc_sync_info = (app_anc_service_aws_sync_info_t *)local_event->extra_data;
        APPS_LOG_MSGID_I(LOG_TAG" handle_event, [%02X] ANC ACTION, enable=%d filter_id=0x%04X anc_type=0x%04X gain=0x%08X",
                         5, role, anc_sync_info->enable, anc_sync_info->filter_id,
                         anc_sync_info->anc_type, anc_sync_info->runtime_gain);

        app_anc_service_local_control(anc_sync_info->enable,
                                      anc_sync_info->filter_id,
                                      anc_sync_info->anc_type,
                                      anc_sync_info->runtime_gain,
                                      NULL);
    } else if (event_group == EVENT_GROUP_UI_SHELL_APP_INTERACTION && event_id == APPS_EVENTS_INTERACTION_APP_ANC_SRV_GAIN_ACTION) {
        apps_aws_sync_future_event_local_event_t *local_event = (apps_aws_sync_future_event_local_event_t *)extra_data;
        app_anc_service_aws_sync_gain_t *anc_sync_info = (app_anc_service_aws_sync_gain_t *)local_event->extra_data;
        APPS_LOG_MSGID_I(LOG_TAG" handle_event, [%02X] GAIN ACTION, anc_type=0x%04X gain=0x%08X",
                         3, role, anc_sync_info->anc_type, anc_sync_info->runtime_gain);
        audio_anc_control_set_runtime_gain(anc_sync_info->runtime_gain, anc_sync_info->anc_type);
    }

    if (event_group == EVENT_GROUP_UI_SHELL_AWS_DATA) {
        bt_aws_mce_report_info_t *aws_data_ind = (bt_aws_mce_report_info_t *)extra_data;
        if (aws_data_ind->module_id == BT_AWS_MCE_REPORT_MODULE_APP_ACTION) {
            uint32_t aws_event_group = 0;
            uint32_t aws_event_id = 0;
            void *p_extra_data = NULL;
            uint32_t extra_data_len = 0;
            apps_aws_sync_event_decode_extra(aws_data_ind, &aws_event_group, &aws_event_id, &p_extra_data, &extra_data_len);
            if (aws_event_group == EVENT_GROUP_UI_SHELL_APP_INTERACTION
                && aws_event_id == APPS_EVENTS_INTERACTION_APP_ANC_SRV_SYNC_INFO
                && role == BT_AWS_MCE_ROLE_PARTNER) {
                app_anc_service_handle_sync_info((uint8_t *)p_extra_data);
            }
        }
    }
#endif
}

void app_anc_service_handle_race_cmd(void *param)
{
#if defined(AIR_APP_ANC_SYNC_ENABLE) && defined(MTK_RACE_CMD_ENABLE)
    race_dsprealtime_notify_struct *audio_param = (race_dsprealtime_notify_struct *)param;
    uint16_t race_event = audio_param->dsp_realtime_race_evet;
    uint8_t channel_id = audio_param->race_channel;

    race_dsprealtime_anc_struct *anc_param = (race_dsprealtime_anc_struct *)audio_param->payload;
    uint8_t anc_id = (uint8_t)anc_param->param.header.ancId;
    uint8_t sync_mode = 0;

    if (race_event != anc_id) {
        APPS_LOG_MSGID_E(LOG_TAG" handle_race_cmd, incorrect ANC ID 0x%08X %d %d", 3, audio_param, race_event, anc_id);
        return;
    }

    bool success = FALSE;
    race_dsprealtime_anc_struct *rsp = NULL;
    switch (race_event) {
        case RACE_ANC_ON: {
#ifdef AIR_ANC_V3
            audio_anc_control_filter_id_t filter_id = anc_param->param.onV3Cmd.flash_no;
            audio_anc_control_type_t anc_type = anc_param->param.onV3Cmd.ancType;
            sync_mode = anc_param->param.onV3Cmd.syncMode;
            uint8_t sub_id = anc_param->param.onV3Cmd.sub_ID;
            APPS_LOG_MSGID_I(LOG_TAG" handle_race_cmd, RACE_ANC_ON filter_id=%d anc_type=0x%04X sub_id=%d",
                             3, filter_id, anc_type, sub_id);
#else
            audio_anc_control_filter_id_t filter_id = anc_param->param.onCmd.flash_no;
            audio_anc_control_type_t anc_type = anc_param->param.onCmd.ancType;
            sync_mode = anc_param->param.onCmd.syncMode;
            APPS_LOG_MSGID_I(LOG_TAG" handle_race_cmd, RACE_ANC_ON filter_id=%d anc_type=0x%04X",
                             2, filter_id, anc_type);
#endif

            success = app_anc_service_enable(filter_id, anc_type, g_anc_srv_context.cur_runtime_gain, NULL);

#ifdef AIR_ANC_V3
            rsp = RACE_ClaimPacket(RACE_TYPE_RESPONSE, RACE_DSPREALTIME_ANC,
                                   sizeof(RACE_RSP_ANC_PASSTHRU_ON_V3_PARAM), channel_id);
            if (rsp != NULL) {
                rsp->param.onV3Rsp.header.ancId  = anc_id;
                rsp->param.onV3Rsp.header.status = (success ? 0 : 1);
                rsp->param.onV3Rsp.flash_no      = filter_id;
                if (filter_id < AUDIO_ANC_CONTROL_FILTER_ID_PASS_THRU_1) {
                    rsp->param.onV3Rsp.ancType = anc_type;
                } else {
                    rsp->param.onV3Rsp.ancType = AUDIO_ANC_CONTROL_TYPE_PASSTHRU_FF;
                }
                rsp->param.onV3Rsp.syncMode = sync_mode;
                rsp->param.onV3Rsp.sub_ID = sub_id;
                rsp->param.onV3Rsp.Reserve_1 = 0;
            }
#else
            rsp = RACE_ClaimPacket(RACE_TYPE_RESPONSE, RACE_DSPREALTIME_ANC,
                                   sizeof(RACE_RSP_ANC_PASSTHRU_ON_PARAM), channel_id);
            if (rsp != NULL) {
                rsp->param.onRsp.header.ancId  = anc_id;
                rsp->param.onRsp.header.status = (success ? 0 : 1);
                rsp->param.onRsp.flash_no      = filter_id;
                if (filter_id < AUDIO_ANC_CONTROL_FILTER_ID_PASS_THRU_1) {
                    rsp->param.onRsp.ancType = anc_type;
                } else {
                    rsp->param.onRsp.ancType = AUDIO_ANC_CONTROL_TYPE_PASSTHRU_FF;
                }
                rsp->param.onRsp.syncMode = sync_mode;
            }
#endif
            break;
        }

        case RACE_ANC_OFF: {
            sync_mode = anc_param->param.offCmd.syncMode;
            APPS_LOG_MSGID_I(LOG_TAG" handle_race_cmd, RACE_ANC_OFF", 0);
            success = app_anc_service_disable();

            rsp = RACE_ClaimPacket(RACE_TYPE_RESPONSE, RACE_DSPREALTIME_ANC,
                                   sizeof(RACE_RSP_ANC_PASSTHRU_OFF_PARAM), channel_id);
            if (rsp != NULL) {
                rsp->param.offRsp.header.ancId  = anc_id;
                rsp->param.offRsp.header.status = (success ? 0 : 1);
                rsp->param.offRsp.syncMode      = sync_mode;
            }
            break;
        }

        case RACE_ANC_SET_RUNTIME_VOL: {
            sync_mode = anc_param->param.runtimeGainCmd.syncMode;
            int16_t runtime_gain = anc_param->param.runtimeGainCmd.gain;
            APPS_LOG_MSGID_I(LOG_TAG" handle_race_cmd, SET_RUNTIME_VOL runtime_gain=0x%08X", 1, runtime_gain);
            success = app_anc_service_set_runtime_gain(g_anc_srv_context.cur_type, runtime_gain);

            rsp = RACE_ClaimPacket(RACE_TYPE_RESPONSE, RACE_DSPREALTIME_ANC,
                                   sizeof(RACE_RSP_ANC_PASSTHRU_SET_RUNTIME_VOL), channel_id);
            if (rsp != NULL) {
                rsp->param.runtimeGainRsp.header.ancId  = anc_id;
                rsp->param.runtimeGainRsp.header.status = (success ? 0 : 1);
                rsp->param.runtimeGainRsp.gain          = runtime_gain;
                rsp->param.runtimeGainRsp.syncMode      = sync_mode;
            }
            break;
        }

        default:
            break;
    }

    if (rsp != NULL) {
        race_flush_packet((void *)rsp, channel_id);
    }
#endif
}



/**================================================================================*/
/**                                  HEARTHROUGH API                               */
/**================================================================================*/
#ifdef AIR_HEARTHROUGH_MAIN_ENABLE
bool app_anc_service_is_suspended()
{
    return g_anc_srv_ht_context.is_suspended;
}

void app_anc_service_set_hear_through_enabled(bool enable)
{
    APPS_LOG_MSGID_I(LOG_TAG"[HearThrough] app_anc_service_set_hear_through_enabled, %d -> %d, enable : %d, cur_filer_id : %d, cur_type : %d, cur_runtime_gain : %d",
                     6,
                     g_anc_srv_ht_context.hear_through_enabled,
                     enable,
                     g_anc_srv_ht_context.enable,
                     g_anc_srv_ht_context.cur_filter_id,
                     g_anc_srv_ht_context.cur_type,
                     g_anc_srv_ht_context.cur_runtime_gain);

    g_anc_srv_ht_context.hear_through_enabled = enable;
}

void app_anc_service_reset_hear_through_anc(bool enable)
{
    APPS_LOG_MSGID_I(LOG_TAG" [HearThrough] app_anc_service_reset_hear_through_anc, enable : %d, anc_enabled : %d, filter_id : %d, type : %d, gain : %d",
                     5,
                     enable,
                     g_anc_srv_ht_context.enable,
                     g_anc_srv_ht_context.cur_filter_id,
                     g_anc_srv_ht_context.cur_type,
                     g_anc_srv_ht_context.cur_runtime_gain);

    if (g_anc_srv_ht_context.is_suspended == true) {
        APPS_LOG_MSGID_I(LOG_TAG" [HearThrough] app_anc_service_reset_hear_through_anc, ANC suspended", 0);
        return;
    }

    if (enable == false) {
        if (g_anc_srv_ht_context.enable == true) {
            app_anc_service_control(false,
                                    true,
                                    g_anc_srv_ht_context.cur_filter_id,
                                    g_anc_srv_ht_context.cur_type,
                                    g_anc_srv_ht_context.cur_runtime_gain,
                                    &(g_anc_srv_ht_context.cur_control_misc));
        } else {
            app_anc_service_control(false, false, 0, 0, 0, NULL);
        }
    } else {
        app_anc_service_control(false,
                                true,
                                g_anc_srv_ht_context.cur_filter_id,
                                g_anc_srv_ht_context.cur_type,
                                g_anc_srv_ht_context.cur_runtime_gain,
                                &(g_anc_srv_ht_context.cur_control_misc));
    }
}

bool app_anc_service_get_hear_through_anc_parameter(audio_anc_control_filter_id_t *filter_id, int16_t *runtime_gain)
{
    if ((filter_id == NULL) || (runtime_gain == NULL)) {
        return false;
    }

    *filter_id = g_anc_srv_ht_context.cur_filter_id;
    *runtime_gain = g_anc_srv_ht_context.cur_runtime_gain;
    return true;
}

bool app_anc_service_is_anc_enabled()
{
    return (g_anc_srv_ht_context.enable == 0) ? false : true;
}

void app_anc_service_disable_without_notify_hear_through()
{
    app_anc_service_control(false, false, 0, 0, 0, NULL);
}

#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */

#endif /* MTK_ANC_ENABLE */
