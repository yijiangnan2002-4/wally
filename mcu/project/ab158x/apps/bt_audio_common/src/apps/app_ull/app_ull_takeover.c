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
 * File: app_ull_idle_activity.c
 *
 * Description: This file is the activity to handle ultra low latency state.
 *
 * Note: See doc/AB1565_AB1568_Earbuds_Reference_Design_User_Guide.pdf for more detail.
 *
 */

#if defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE) && defined(AIR_BLE_ULL_TAKEOVER_ENABLE)

#include "app_ull_idle_activity.h"
#include "app_ull_takeover.h"
#include "app_ull_nvkey_struct.h"
#include "apps_events_event_group.h"
#include "apps_events_interaction_event.h"
#include "apps_events_key_event.h"
#include "apps_events_bt_event.h"
#include "apps_config_event_list.h"
#include "apps_config_vp_index_list.h"
#include "voice_prompt_api.h"
#include "apps_config_key_remapper.h"
#include "apps_config_led_manager.h"
#include "apps_config_led_index_list.h"
#include "apps_customer_config.h"
#include "app_bt_state_service.h"
#include "multi_ble_adv_manager.h"
#include "bt_app_common.h"
#include "bt_connection_manager.h"
#include "bt_connection_manager_internal.h"
#include "bt_device_manager.h"
#include "bt_device_manager_power.h"
#include "bt_ull_service.h"
#include "bt_sink_srv_ami.h"
#include "nvkey_id_list.h"
#include "atci.h"
#ifdef AIR_BT_FAST_PAIR_ENABLE
#include "app_fast_pair.h"
#endif
#if defined(MTK_FOTA_ENABLE) && defined (MTK_FOTA_VIA_RACE_CMD)
#include "race_event.h"
#endif
#include "nvkey.h"
#ifdef MTK_AWS_MCE_ENABLE
#include "apps_aws_sync_event.h"
#include "bt_aws_mce_srv.h"
#endif
#include "apps_debug.h"

#ifdef AIR_LE_AUDIO_ENABLE
#include "bt_sink_srv_le.h"
#include "app_lea_service_conn_mgr.h"
#endif

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
#include "bt_ull_le_service.h"
#endif
#include "app_lea_service.h"
#ifdef AIR_LE_AUDIO_ENABLE
#include "app_lea_service_event.h"
#endif
#include "app_lea_service_adv_mgr.h"

#define LOG_TAG "[ULL_activity][takeover]"

typedef enum {
    ULL2_TAKE_OVER_EV_SP_EDR_CONNECTED,
    ULL2_TAKE_OVER_EV_SP_EDR_DISCONNECTED,
    ULL2_TAKE_OVER_EV_LE_AUDIO_CONNECTED,
    ULL2_TAKE_OVER_EV_LE_AUDIO_DISCONNECTED,
    ULL2_TAKE_OVER_EV_ULL2_CONNECTED,
    ULL2_TAKE_OVER_EV_ULL2_DISCONNECTED,
    ULL2_TAKE_OVER_EV_SINK_STREAMING,
    ULL2_TAKE_OVER_EV_SINK_IDLE,
    ULL2_TAKE_OVER_EV_ULL2_STREAMING,
    ULL2_TAKE_OVER_EV_ULL2_IDLE,
    ULL2_TAKE_OVER_EV_EDR_PROFILE_CONNECT_DONE,
} ull2_take_over_event_type_t;

typedef struct {
    uint8_t sp_edr_connected;
    uint8_t le_audio_connected;
    uint8_t ull2_connected;
    uint8_t sink_streaming;
    uint8_t ull2_streaming;
    uint8_t wait_le_audio_disconnect_ev;
    uint8_t wait_sp_disconnect_ev;
    uint8_t wait_ull_disconnect_ev;
} app_ull_take_over_context_t;

static app_ull_take_over_context_t s_ctx = {0};

#ifdef AIR_LE_AUDIO_ENABLE
extern void app_lea_service_stop_advertising(bool sync);
extern void app_lea_service_start_advertising(uint8_t mode, bool sync, uint32_t timeout);
extern uint8_t app_lea_conn_mgr_get_conn_num(void);
extern void app_lea_conn_mgr_disconnect(const uint8_t *addr, uint8_t reason);
#endif

extern bool app_ull_is_multi_link_mode();
static bool s_visible_lock = false;

#ifdef AIR_LE_AUDIO_ENABLE
bool app_ull_is_lea_connected()
{
    return (s_ctx.le_audio_connected > 0);
}
#endif

bool app_ull_is_sp_connected()
{
    return (s_ctx.sp_edr_connected > 0);
}

static void app_ull_take_over_set_le_audio_con(bool connect)
{
#ifdef AIR_LE_AUDIO_ENABLE
    APPS_LOG_MSGID_I(LOG_TAG"app_ull_take_over_set_le_audio_con cur=%d, target=%d", 2, s_ctx.le_audio_connected, connect);
    if (connect == false && s_ctx.le_audio_connected > 0) {
        s_ctx.wait_le_audio_disconnect_ev = app_lea_conn_mgr_get_conn_num();
        APPS_LOG_MSGID_I(LOG_TAG"app_ull_take_over_set_le_audio_con connected dev nums=%d", 1, s_ctx.wait_le_audio_disconnect_ev);
        //app_lea_conn_mgr_disconnect(NULL, BT_HCI_STATUS_REMOTE_TERMINATED_CONNECTION_DUE_TO_LOW_RESOURCES);
        uint8_t *addr = app_lea_conn_mgr_get_addr_by_handle(app_lea_conn_mgr_get_dongle_handle(APP_LEA_CONN_TYPE_LE_ULL));
        app_lea_service_disconnect(false,
                                   APP_LE_AUDIO_DISCONNECT_MODE_KEEP,
                                   addr,
                                   BT_HCI_STATUS_REMOTE_TERMINATED_CONNECTION_DUE_TO_LOW_RESOURCES);
    }
#else
    APPS_LOG_MSGID_I(LOG_TAG"app_ull_take_over_set_le_audio_con not defined", 0);
#endif
}

static void app_ull_take_over_set_le_audio_adv(bool en)
{
#ifdef AIR_LE_AUDIO_ENABLE
    APPS_LOG_MSGID_I(LOG_TAG"app_ull_take_over_set_le_audio_adv set le audio adv=%d", 1, en);
    if (en) {
#if 0
        app_lea_service_control_adv_data(APP_LEA_SERVICE_ADV_DATA_LEA, true);
        app_lea_service_start_advertising(APP_LEA_ADV_MODE_TARGET_ALL, false, 0);
#else
        app_lea_service_start_advertising(APP_LEA_ADV_MODE_TARGET_ALL, false, 0);
#endif
    } else {
        if (!s_visible_lock) {
#if 0
            app_lea_service_control_adv_data(APP_LEA_SERVICE_ADV_DATA_LEA, false);
#else
            app_lea_adv_mgr_stop_advertising(false);
#endif
        } else {
            APPS_LOG_MSGID_I(LOG_TAG"app_ull_take_over_set_le_audio_adv locked by visible", 0);
        }
    }
#else
    APPS_LOG_MSGID_I(LOG_TAG"app_ull_take_over_set_le_audio_adv not defined", 0);
#endif
}

extern void app_le_ull_set_advertising_enable(bool enable);
extern void app_le_ull_disconnect_dongle();

static void app_ull_take_over_set_ull_con(bool connect)
{
    APPS_LOG_MSGID_I(LOG_TAG"app_ull_take_over_set_ull_con cur=%d, target=%d", 2, s_ctx.ull2_connected, connect);
    if (connect == false && s_ctx.ull2_connected > 0) {
        s_ctx.wait_ull_disconnect_ev = 1;//ull2.0 support only one link now.
        app_le_ull_disconnect_dongle();
    }
}

static inline void app_ull_take_over_set_ull_adv(bool en)
{
    app_le_ull_set_advertising_enable(en);
}

static void app_ull_take_over_set_sp_con(bool connect)
{
    APPS_LOG_MSGID_I(LOG_TAG"app_ull_take_over_set_sp_con cur=%d, target=%d", 2, s_ctx.sp_edr_connected, connect);
    if (connect == false) {
        bt_bd_addr_t connected_address[3];
        uint32_t connected_num = 3;
        uint32_t i;
        bt_cm_connect_t cm_param;

        cm_param.profile = BT_CM_PROFILE_SERVICE_MASK_ALL;
        s_ctx.wait_sp_disconnect_ev = 0;
        connected_num = bt_cm_get_connecting_devices(BT_CM_PROFILE_SERVICE_MASK_NONE, connected_address, connected_num);
        for (i = 0; i < connected_num; i++) {
            memcpy(cm_param.address, connected_address[i], sizeof(bt_bd_addr_t));
            bt_cm_disconnect(&cm_param);
            s_ctx.wait_sp_disconnect_ev++;
        }

        connected_num = 3;
#ifdef AIR_HEADSET_ENABLE
        connected_num = bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK_NONE,
                                                    connected_address, connected_num);
#else
        connected_num = bt_cm_get_connected_devices(~BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS),
                                                    connected_address, connected_num);
#endif
        for (i = 0; i < connected_num; i++) {
            memcpy(cm_param.address, connected_address[i], sizeof(bt_bd_addr_t));
            bt_cm_disconnect(&cm_param);
            s_ctx.wait_sp_disconnect_ev++;
        }
    }
}

static void app_ull_take_over_set_page_scan(bool en)
{
    APPS_LOG_MSGID_I(LOG_TAG"app_ull_take_over_set_page_scan en=%d", 1, en);
    if (en) {
        bt_cm_write_scan_mode(BT_CM_COMMON_TYPE_UNKNOW, BT_CM_COMMON_TYPE_ENABLE);
    } else {
        if (!s_visible_lock) {
            bt_cm_write_scan_mode(BT_CM_COMMON_TYPE_UNKNOW, BT_CM_COMMON_TYPE_DISABLE);
        } else {
            APPS_LOG_MSGID_I(LOG_TAG"page scan lock by visible", 0);
        }
    }
}

void app_ull_take_over_enable_visible()
{
    if (!app_ull_is_multi_link_mode()) {
        APPS_LOG_MSGID_I(LOG_TAG"single link will not enable pagescan", 0);
        return;
    }
    s_visible_lock = true;
    app_ull_take_over_set_page_scan(true);
    if (s_ctx.le_audio_connected == 0) {
        app_ull_take_over_set_le_audio_adv(true);
    }
}

void app_ull_take_over_disable_visible()
{
#if 0
    if (!app_ull_is_multi_link_mode()) {
        return;
    }
#endif
    s_visible_lock = false;
    if (s_ctx.ull2_streaming > 0) {
        app_ull_take_over_set_page_scan(false);
        app_ull_take_over_set_le_audio_adv(false);
    } else {
        APPS_LOG_MSGID_I(LOG_TAG"ull not streaming, do not disable page scan for visable change", 0);
    }
}

static void app_ull_take_over_event_process_with_multi_mode(ull2_take_over_event_type_t event)
{
    APPS_LOG_MSGID_I(LOG_TAG"app_ull_take_over_event_process_with_multi_mode event=%d", 1, event);
    switch (event) {
        case ULL2_TAKE_OVER_EV_SP_EDR_CONNECTED: {
            s_ctx.sp_edr_connected++;
            break;
        }

        case ULL2_TAKE_OVER_EV_SP_EDR_DISCONNECTED: {
            if (s_ctx.sp_edr_connected > 0) {
                s_ctx.sp_edr_connected--;
            }
            if (s_ctx.sp_edr_connected == 0) {
                app_ull_take_over_set_page_scan(true);
            }
            if (s_ctx.wait_sp_disconnect_ev != 0) {
                s_ctx.wait_sp_disconnect_ev--;
                break;
            }
            break;
        }

        case ULL2_TAKE_OVER_EV_LE_AUDIO_CONNECTED: {
            s_ctx.le_audio_connected++;
            if (s_ctx.ull2_connected > 0) {
                bt_app_common_pre_set_ultra_low_latency_retry_count(BT_APP_COMMON_ULL_LATENCY_MODULE_RECONNECT, BT_ULL_LE_SRV_LATENCY_MULTI_LINK_LEA_7_5MS_30MS_STANDBY_MODE);
                bt_app_common_apply_ultra_low_latency_retry_count();
            }
            break;
        }

        case ULL2_TAKE_OVER_EV_LE_AUDIO_DISCONNECTED: {
            if (s_ctx.le_audio_connected > 0) {
                s_ctx.le_audio_connected--;
            }
            if (s_ctx.wait_le_audio_disconnect_ev != 0) {
                s_ctx.wait_le_audio_disconnect_ev--;
                break;
            }
            if (s_ctx.ull2_connected > 0 && s_ctx.sp_edr_connected == 0) {
                bt_app_common_pre_set_ultra_low_latency_retry_count(BT_APP_COMMON_ULL_LATENCY_MODULE_RECONNECT, BT_APP_COMMON_ULL_STREAM_RETRY_COUNT_FOR_CONNECTING);
                bt_app_common_apply_ultra_low_latency_retry_count();
            }
            break;
        }

        case ULL2_TAKE_OVER_EV_ULL2_CONNECTED: {
            s_ctx.ull2_connected++;
            break;
        }

        case ULL2_TAKE_OVER_EV_ULL2_DISCONNECTED: {
            if (s_ctx.ull2_connected > 0) {
                s_ctx.ull2_connected--;
            }
            if (s_ctx.wait_ull_disconnect_ev != 0) {
                s_ctx.wait_ull_disconnect_ev--;
            }
            break;
        }

        case ULL2_TAKE_OVER_EV_ULL2_STREAMING: {
            bt_bd_addr_t connecting_address;
            uint32_t connecting_num = bt_cm_get_connecting_devices(BT_CM_PROFILE_SERVICE_MASK_NONE, &connecting_address, 1);
            if (connecting_num > 0) {
                bt_cm_connect_t cm_param;
                cm_param.profile = BT_CM_PROFILE_SERVICE_MASK_ALL;
                memcpy(cm_param.address, &connecting_address, sizeof(bt_bd_addr_t));
                bt_cm_disconnect(&cm_param);
                APPS_LOG_MSGID_I(LOG_TAG"[APP_CONN], cancel reconnecting due to ull streaming, addr [%02X:%02X:%02X:%02X:%02X:%02X]", 6,
                                 connecting_address[5], connecting_address[4], connecting_address[3],
                                 connecting_address[2], connecting_address[1], connecting_address[0]);
            }
            if (s_ctx.sp_edr_connected > 0 || s_ctx.le_audio_connected > 0) {
                app_ull_take_over_set_page_scan(false);
                app_ull_take_over_set_le_audio_adv(false);
            }
        }
        case ULL2_TAKE_OVER_EV_SINK_STREAMING: {
            APPS_LOG_MSGID_I(LOG_TAG"app_ull_take_over_event_process STREAMING, sta=%d,%d,%d", 3,
                             s_ctx.le_audio_connected, s_ctx.sp_edr_connected, s_ctx.ull2_connected);
            s_ctx.sink_streaming = true;
            break;
        }

        case ULL2_TAKE_OVER_EV_ULL2_IDLE:
            app_ull_take_over_set_page_scan(true);
            app_ull_take_over_set_le_audio_adv(true);
        case ULL2_TAKE_OVER_EV_SINK_IDLE: {
            s_ctx.sink_streaming = false;
            APPS_LOG_MSGID_I(LOG_TAG"app_ull_take_over_event_process IDLE, sta=%d,%d,%d", 3,
                             s_ctx.le_audio_connected, s_ctx.sp_edr_connected, s_ctx.ull2_connected);
            break;
        }
        case ULL2_TAKE_OVER_EV_EDR_PROFILE_CONNECT_DONE: {
            app_ull_take_over_disable_visible();
            bt_app_common_apply_ultra_low_latency_retry_count();
            break;
        }
    }
}

static void app_ull_take_over_event_process(ull2_take_over_event_type_t event)
{
    if (app_ull_is_multi_link_mode()) {
        app_ull_take_over_event_process_with_multi_mode(event);
        return;
    }
    APPS_LOG_MSGID_I(LOG_TAG"app_ull_take_over_event_process event=%d", 1, event);
    switch (event) {
        case ULL2_TAKE_OVER_EV_SP_EDR_CONNECTED: {
            s_ctx.sp_edr_connected++;
            if (s_ctx.sp_edr_connected == 1) {
                if (s_ctx.ull2_connected > 0 && s_ctx.sink_streaming) {
                    app_ull_take_over_set_sp_con(false);
                    s_ctx.sp_edr_connected = 0;/* work around, to avoid connection case. */
                    break;
                }
                //app_ull_take_over_set_le_audio_con(false);
                app_ull_take_over_set_ull_con(false);
            }
            break;
        }

        case ULL2_TAKE_OVER_EV_SP_EDR_DISCONNECTED: {
            if (s_ctx.sp_edr_connected > 0) {
                s_ctx.sp_edr_connected--;
            }
            if (s_ctx.wait_sp_disconnect_ev != 0) {
                s_ctx.wait_sp_disconnect_ev--;
                break;
            }
            if (s_ctx.sp_edr_connected == 0 && s_ctx.ull2_streaming > 0) {
                app_ull_take_over_set_page_scan(false);
            }
            break;
        }

        case ULL2_TAKE_OVER_EV_LE_AUDIO_CONNECTED: {
            s_ctx.le_audio_connected++;
            if (s_ctx.le_audio_connected == 1) {
                app_ull_take_over_set_ull_con(false);
                //app_ull_take_over_set_sp_con(false);
            }
            break;
        }

        case ULL2_TAKE_OVER_EV_LE_AUDIO_DISCONNECTED: {
            if (s_ctx.le_audio_connected > 0) {
                s_ctx.le_audio_connected--;
            }
            if (s_ctx.wait_le_audio_disconnect_ev != 0) {
                s_ctx.wait_le_audio_disconnect_ev--;
                break;
            }
            break;
        }

        case ULL2_TAKE_OVER_EV_ULL2_CONNECTED: {
            s_ctx.ull2_connected++;
            if (s_ctx.ull2_connected == 1) {
                if (s_ctx.sp_edr_connected > 0 && s_ctx.sink_streaming) {
                    app_ull_take_over_set_ull_con(false);
                    break;
                }
                app_ull_take_over_set_le_audio_con(false);
                app_ull_take_over_set_sp_con(false);
            }
            break;
        }

        case ULL2_TAKE_OVER_EV_ULL2_DISCONNECTED: {
            if (s_ctx.ull2_connected > 0) {
                s_ctx.ull2_connected--;
            }
            if (s_ctx.wait_ull_disconnect_ev != 0) {
                s_ctx.wait_ull_disconnect_ev--;
                if (s_ctx.le_audio_connected > 0 || s_ctx.sp_edr_connected > 0) {
#ifndef AIR_WIRELESS_MIC_ENABLE
                    app_ull_take_over_set_ull_adv(true);
#endif
                }
                break;
            }
            break;
        }

        case ULL2_TAKE_OVER_EV_ULL2_STREAMING:
        case ULL2_TAKE_OVER_EV_SINK_STREAMING: {
            APPS_LOG_MSGID_I(LOG_TAG"app_ull_take_over_event_process STREAMING, sta=%d,%d,%d", 3,
                             s_ctx.le_audio_connected, s_ctx.sp_edr_connected, s_ctx.ull2_connected);
            s_ctx.sink_streaming = true;
            if (s_ctx.le_audio_connected == 0 && s_ctx.sp_edr_connected == 0) {
                app_ull_take_over_set_le_audio_adv(false);
                app_ull_take_over_set_page_scan(false);
            }
            if (s_ctx.ull2_connected == 0) {
                app_ull_take_over_set_ull_adv(false);
            }
            break;
        }

        case ULL2_TAKE_OVER_EV_ULL2_IDLE:
        case ULL2_TAKE_OVER_EV_SINK_IDLE: {
            s_ctx.sink_streaming = false;
            APPS_LOG_MSGID_I(LOG_TAG"app_ull_take_over_event_process IDLE, sta=%d,%d,%d", 3,
                             s_ctx.le_audio_connected, s_ctx.sp_edr_connected, s_ctx.ull2_connected);
            if (s_ctx.le_audio_connected == 0) {
                app_ull_take_over_set_le_audio_adv(true);
            }
            if (s_ctx.sp_edr_connected == 0) {
                app_ull_take_over_set_page_scan(true);
            }
            if (s_ctx.ull2_connected == 0) {
#ifndef AIR_WIRELESS_MIC_ENABLE
                app_ull_take_over_set_ull_adv(true);
#endif
            }
            break;
        }
    }
}

static bool app_ull_proc_ui_shell_group(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    /* UI shell internal event must process by this activity, so default is true. */
    bool ret = true;
    switch (event_id) {
        case EVENT_ID_SHELL_SYSTEM_ON_CREATE: {
            break;
        }
        default:
            ret = false;
            break;
    }
    return ret;
}

static bool app_ull_proc_bt_cm_group(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
#if 0
#else
    extern bool s_a2dp_standby_enable;
    static bool s_profile_connected = false;
    if (event_id == BT_CM_EVENT_REMOTE_INFO_UPDATE) {
        bt_cm_remote_info_update_ind_t *remote_update = (bt_cm_remote_info_update_ind_t *)extra_data;
        if (NULL == remote_update) {
            return false;
        }
#ifdef MTK_AWS_MCE_ENABLE
        if (BT_AWS_MCE_ROLE_AGENT == bt_device_manager_aws_local_info_get_role() ||
            BT_AWS_MCE_ROLE_NONE == bt_device_manager_aws_local_info_get_role())
#endif
        {
            if ((~BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service) == 0 &&
                (~BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service) != 0) {
                app_ull_take_over_event_process(ULL2_TAKE_OVER_EV_SP_EDR_CONNECTED);
            } else if ((~BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service) != 0 &&
                       (~BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service) == 0) {
                app_ull_take_over_event_process(ULL2_TAKE_OVER_EV_SP_EDR_DISCONNECTED);
                s_profile_connected = false;
            }
            /* check connected profile. */
            if (!s_profile_connected) {
                extern bool s_a2dp_standby_enable;
                bt_bd_addr_t connected_address[3];
                uint32_t connected_num = 3;
                bt_cm_profile_service_mask_t profile_mask = BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP);
                if (s_a2dp_standby_enable) {
                    profile_mask |= BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK);
                }
                connected_num = bt_cm_get_connected_devices(profile_mask,
                                                            connected_address, connected_num);
                if (connected_num > 0) {
                    s_profile_connected = true;
                    app_ull_take_over_event_process(ULL2_TAKE_OVER_EV_EDR_PROFILE_CONNECT_DONE);
                }
            }
        }
#ifdef MTK_AWS_MCE_ENABLE
        else if (BT_AWS_MCE_ROLE_PARTNER == bt_device_manager_aws_local_info_get_role()) {
            if (!(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service)
                && (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service)) {
                if (BT_AWS_MCE_SRV_LINK_NORMAL == bt_aws_mce_srv_get_link_type()) {
                    /* Partner Connect SP = AWS connected + LINK_NORMAL */
                    if (s_a2dp_standby_enable) {
                        bt_app_common_pre_set_ultra_low_latency_retry_count(BT_APP_COMMON_ULL_LATENCY_MODULE_RECONNECT, BT_APP_COMMON_ULL_STREAM_RETRY_COUNT_FOR_MULTI_LINK_A2DP);
                    } else {
                        bt_app_common_pre_set_ultra_low_latency_retry_count(BT_APP_COMMON_ULL_LATENCY_MODULE_RECONNECT, BT_APP_COMMON_ULL_STREAM_RETRY_COUNT_FOR_MULTI_LINK);
                    }
                    app_ull_take_over_event_process(ULL2_TAKE_OVER_EV_SP_EDR_CONNECTED);
                    app_ull_take_over_event_process(ULL2_TAKE_OVER_EV_EDR_PROFILE_CONNECT_DONE);
                } else if (BT_AWS_MCE_SRV_LINK_SPECIAL == bt_aws_mce_srv_get_link_type()) {
                    if (s_ctx.sp_edr_connected > 0) {
                        app_ull_take_over_event_process(ULL2_TAKE_OVER_EV_SP_EDR_DISCONNECTED);
                        s_profile_connected = false;
                    }
                }
            } else if ((BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->pre_connected_service)
                       && !(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS) & remote_update->connected_service)) {
                if (s_ctx.sp_edr_connected > 0) {
                    app_ull_take_over_event_process(ULL2_TAKE_OVER_EV_SP_EDR_DISCONNECTED);
                }
            }
        }
#endif
        /* check a2dp profile connection */
        if (!s_a2dp_standby_enable && s_ctx.ull2_connected > 0) {
            if ((!(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK) & remote_update->pre_connected_service)
                 && (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK) & remote_update->connected_service)) ||
                (!(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AVRCP) & remote_update->pre_connected_service)
                 && (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AVRCP) & remote_update->connected_service))) {
                bt_cm_connect_t cm_param;
                cm_param.profile = (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK) |
                                    BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AVRCP));
                memcpy(cm_param.address, remote_update->address, sizeof(bt_bd_addr_t));
                bt_cm_disconnect(&cm_param);
            }
        }
    }
#endif
    return false;
}

static bool app_ull_proc_bt_sink_events(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    switch (event_id) {
        /* check le audio connection state */
        case LE_SINK_SRV_EVENT_REMOTE_INFO_UPDATE: {
#ifdef AIR_LE_AUDIO_CIS_ENABLE
            bt_le_sink_srv_event_remote_info_update_t *update_ind = (bt_le_sink_srv_event_remote_info_update_t *)extra_data;
            if (update_ind == NULL) {
                break;
            }
            if (update_ind->pre_state == BT_BLE_LINK_DISCONNECTED
                && update_ind->state == BT_BLE_LINK_CONNECTED) {
                app_ull_take_over_event_process(ULL2_TAKE_OVER_EV_LE_AUDIO_CONNECTED);
            } else if (update_ind->pre_state == BT_BLE_LINK_CONNECTED
                       && update_ind->state == BT_BLE_LINK_DISCONNECTED) {
#ifdef AIR_LE_AUDIO_MULTIPOINT_ENABLE
                bt_handle_t le_handle = bt_sink_srv_cap_check_links_state(BT_SINK_SRV_CAP_STATE_CONNECTED);
                APPS_LOG_MSGID_I(LOG_TAG"[Power_Saving] LEA_BT_SINK, Disconnect remain le_handle=0x%04X", 1, le_handle);
                if (BT_HANDLE_INVALID == le_handle) {
                    app_ull_take_over_event_process(ULL2_TAKE_OVER_EV_LE_AUDIO_DISCONNECTED);
                }
#else
                app_ull_take_over_event_process(ULL2_TAKE_OVER_EV_LE_AUDIO_DISCONNECTED);
#endif
            }
#endif
            break;
        }

        /* check streaming state of le_audio/EDR */
        case BT_SINK_SRV_EVENT_STATE_CHANGE: {
            bt_sink_srv_state_change_t *param = (bt_sink_srv_state_change_t *) extra_data;
            if ((param->previous < BT_SINK_SRV_STATE_STREAMING) && (param->current >= BT_SINK_SRV_STATE_STREAMING)) {
                app_ull_take_over_event_process(ULL2_TAKE_OVER_EV_SINK_STREAMING);
            } else if ((param->previous >= BT_SINK_SRV_STATE_STREAMING) && (param->current < BT_SINK_SRV_STATE_STREAMING)) {
                app_ull_take_over_event_process(ULL2_TAKE_OVER_EV_SINK_IDLE);
            }
            break;
        }
    }

    return false;
}

static bool app_ull_proc_ull_event(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;

    switch (event_id) {
        case BT_ULL_EVENT_USB_PLAYING_IND: {
            app_ull_take_over_event_process(ULL2_TAKE_OVER_EV_ULL2_STREAMING);
            break;
        }
        case BT_ULL_EVENT_USB_STOP_IND: {
            app_ull_take_over_event_process(ULL2_TAKE_OVER_EV_ULL2_IDLE);
            break;
        }
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
        case BT_ULL_EVENT_LE_CONNECTED: {
            bt_ull_le_connected_info_t *con_info = (bt_ull_le_connected_info_t*)extra_data;
            if (con_info->status != BT_STATUS_SUCCESS) {
                APPS_LOG_MSGID_E(LOG_TAG", connected with error=0x%x", 1, con_info->status);
                break;
            }
            app_ull_take_over_event_process(ULL2_TAKE_OVER_EV_ULL2_CONNECTED);
            break;
        }
        case BT_ULL_EVENT_LE_DISCONNECTED:
            app_ull_take_over_event_process(ULL2_TAKE_OVER_EV_ULL2_DISCONNECTED);
            break;
        case BT_ULL_EVENT_LE_STREAMING_START_IND:
            s_ctx.ull2_streaming++;
            if (s_ctx.ull2_streaming == 1) {
                app_ull_take_over_event_process(ULL2_TAKE_OVER_EV_ULL2_STREAMING);
            }
            break;
        case BT_ULL_EVENT_LE_STREAMING_STOP_IND:
            if (s_ctx.ull2_streaming > 0) {
                s_ctx.ull2_streaming--;
            }
            if (s_ctx.ull2_streaming == 0) {
                app_ull_take_over_event_process(ULL2_TAKE_OVER_EV_ULL2_IDLE);
            }
            break;
#endif
        default:
            break;
    }

    return ret;
}

static bool app_ull_proc_key_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    if (extra_data) {
        apps_config_key_action_t action;
        action = *(uint16_t *)extra_data;

        if (action == KEY_ULL_SWITCH_LINK_MODE) {
            /* will switch to single link mode. */
            if (app_ull_is_multi_link_mode()) {
                if (s_ctx.ull2_streaming > 0) {
                    app_ull_take_over_set_page_scan(false);
                    bt_app_common_pre_set_ultra_low_latency_retry_count(BT_APP_COMMON_ULL_LATENCY_MODULE_DISCOVERABLE, APPS_ULL_STREAMING_RETRY_COUNT_FOR_SINGLE_LINK);
                }
                if (s_ctx.le_audio_connected > 0) {
                    app_ull_take_over_set_le_audio_con(false);
                }
            } else {/* will switch to multi link mode. */
                if (s_ctx.le_audio_connected == 0) {
                    app_ull_take_over_set_le_audio_adv(true);
                }
                app_ull_take_over_set_page_scan(true);
            }
        }
    }
    return false;
}


#ifdef MTK_AWS_MCE_ENABLE
static bool app_ull_proc_aws_data_event_proc(ui_shell_activity_t *self, uint32_t aws_event_id, void *aws_extra, size_t data_len)
{
    bt_aws_mce_report_info_t *aws_data_ind = (bt_aws_mce_report_info_t *)aws_extra;
    if (NULL == aws_data_ind || aws_data_ind->module_id != BT_AWS_MCE_REPORT_MODULE_APP_ACTION) {
        return false;
    }
    uint32_t event_group;
    uint32_t event_id;
    uint8_t *extra_data;
    uint32_t extra_data_len;


    apps_aws_sync_event_decode_extra(aws_data_ind, &event_group, &event_id, (void *)&extra_data, &extra_data_len);
    if (event_group == EVENT_GROUP_UI_SHELL_KEY && event_id == KEY_ULL_SWITCH_LINK_MODE) {
        app_ull_proc_key_event_proc(self, event_id, &event_id, 0);
    }
    return false;
}
#endif

bool app_ull_takeover_activity_proc(
    struct _ui_shell_activity *self,
    uint32_t event_group,
    uint32_t event_id,
    void *extra_data,
    size_t data_len)
{
    bool ret = false;

    switch (event_group) {
        case EVENT_GROUP_UI_SHELL_SYSTEM: {
            /* UI Shell internal events, please refer to doc/Airoha_IoT_SDK_UI_Framework_Developers_Guide.pdf. */
            ret = app_ull_proc_ui_shell_group(self, event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_BT_CONN_MANAGER: {
            ret = app_ull_proc_bt_cm_group(self, event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_BT_SINK: {
            /* BT_SINK events, indicates the state of music. */
            ret = app_ull_proc_bt_sink_events(self, event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_BT_ULTRA_LOW_LATENCY: {
            ret = app_ull_proc_ull_event(self, event_id, extra_data, data_len);
            break;
        }
#ifdef MTK_AWS_MCE_ENABLE
        case EVENT_GROUP_UI_SHELL_AWS_DATA: {
            ret = app_ull_proc_aws_data_event_proc(self, event_id, extra_data, data_len);
            break;
        }
#endif
        case EVENT_GROUP_UI_SHELL_KEY: {
            ret = app_ull_proc_key_event_proc(self, event_id, extra_data, data_len);
            break;
        }
#ifdef AIR_LE_AUDIO_ENABLE
        case EVENT_GROUP_UI_SHELL_LE_AUDIO: {
            if (event_id == EVENT_ID_LE_AUDIO_BIS_START_STREAMING) {
                app_ull_take_over_event_process(ULL2_TAKE_OVER_EV_SINK_STREAMING);
            } else if (event_id == EVENT_ID_LE_AUDIO_BIS_STOP_STREAMING) {
                app_ull_take_over_event_process(ULL2_TAKE_OVER_EV_SINK_IDLE);
            }
            break;
        }
#endif
        default: {
            break;
        }
    }

    return ret;
}

#endif

