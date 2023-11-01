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
 * File: app_home_screen_idle_activity.c
 *
 * Description: This file could control system power on/off, trigger BT on/off etc.
 *
 * Note: See doc/Airoha_IoT_SDK_Application_Developers_Guide.pdf for Homescreen APP.
 *
 */

#include "app_home_screen_idle_activity.h"
#ifdef SUPPORT_ROLE_HANDOVER_SERVICE
#include "app_rho_idle_activity.h"
#endif
#include "app_bt_conn_componet_in_homescreen.h"
#include "apps_config_led_manager.h"
#include "apps_config_led_index_list.h"
#include "apps_config_vp_index_list.h"
#include "apps_config_key_remapper.h"
#include "apps_config_features_dynamic_setting.h"
//#include "multi_va_manager.h"
#include "project_config.h"
#include "apps_customer_config.h"

#include "FreeRTOS.h"
#include "timers.h"
#include "ui_shell_manager.h"
#include "apps_events_event_group.h"
#include "apps_events_key_event.h"
#include "apps_events_bt_event.h"
#ifdef AIR_USB_ENABLE
#include "apps_events_usb_event.h"
#include "usb.h"
#endif
#include "apps_config_event_list.h"
#include "app_bt_state_service.h"
#include "nvdm_config_factory_reset.h"
#include "at_command_bt.h"
#include "bt_sink_srv.h"
#include "bt_app_common.h"
#include "bt_power_on_config.h"
#include "apps_events_interaction_event.h"
#include "bt_sink_srv_ami.h"
#include "bt_device_manager.h"
#include "bt_connection_manager.h"
#include "bt_connection_manager_internal.h"
#ifdef AIR_BLE_AUDIO_DONGLE_ENABLE
#include "app_le_audio.h"
#include "app_le_audio_utillity.h"
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
#include "app_le_audio_ucst_utillity.h"
#endif
#endif
#if defined (AIR_BLE_AUDIO_DONGLE_ENABLE) || defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
#include "app_le_audio_component_in_homescreen.h"
#endif
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
#include "app_ull_dongle_le.h"
#endif
#include "nvdm.h"
#include "nvdm_id_list.h"
#include "nvkey.h"
#include "nvkey_id_list.h"
#ifdef MTK_ANC_ENABLE
#include "app_anc_service.h"
#endif
#include "hal_rtc.h"
#include "hal_wdt.h"
#include "hal_clock.h"
#ifdef MTK_AWS_MCE_ENABLE
#include "apps_aws_sync_event.h"
#include "bt_aws_mce_report.h"
#include "bt_aws_mce_srv.h"
#endif
#ifdef MTK_FOTA_ENABLE
#include "fota.h"
#endif
#include "nvdm_config.h"
#include "apps_debug.h"

#include "usb_main.h"
#include "usb.h"
#include "app_dongle_connection_common.h"

#include "bt_hsp.h"

#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
#include "app_dongle_ull_le_hid.h"
#endif

#include "avm_external.h"
#ifdef MTK_RACE_CMD_ENABLE
#include "apps_race_cmd_event.h"
#endif


#define UI_SHELL_IDLE_BT_CONN_ACTIVITY  "[TK_Home]app_home_screen_idle_activity"

#define POWER_OFF_TIMER_NAME       "POWER_OFF"              /* Use a timeout before power off, to show LED and play VP. */
#define WAIT_TIME_BEFORE_POWER_OFF  (3 * 1000)              /* The delay time to do system power off for playing VP and LED. */
#define TIME_TO_START_VISIBLE_AFTER_POWER_OFF   (10 * 1000) /* The delay time to start BT visible after BT power on. */
#define TIME_TO_STOP_RECONNECTION   (2 * 60 * 1000)         /* The delay time to stop reconnection. */
#define VISIBLE_TIMEOUT             (2 * 60 * 1000)         /* The timeout of BT visibility. */

#if defined(AIR_BTA_IC_PREMIUM_G2) || defined(AIR_BTA_IC_STEREO_HIGH_G3)
/* function in hal_dcxo.c */
void dcxo_lp_mode(dcxo_mode_t mode);
#endif

/* Global context for Homescreen APP. */
static home_screen_local_context_type_t s_app_homescreen_context;

static uint16_t s_factory_reset_pending_event = KEY_ACTION_INVALID;
static uint16_t s_factory_reset_key_action = KEY_ACTION_INVALID;
static uint16_t s_factory_reset_doing = false;
static bool s_ready_to_off = false;

#define RETRY_ENTER_RTC_MODE_TIMES      (10)
extern uint8_t g_dongle_mode;

const bt_bd_addr_t s_empty_addr = { 0 };

void app_home_screen_check_power_off_and_reboot(void)
{
    if (s_ready_to_off) {
        if (APP_HOME_SCREEN_STATE_POWERING_OFF == s_app_homescreen_context.state) {
            /* Enter RTC mode as power off action when no waiting and power_key released. */
            uint32_t i;
            /* Because some chip can display LED in RTC mode, must clear it before enter RTC mode. */
            apps_config_led_disable_all();
            for (i = 0; ; i++) {
                if (HAL_RTC_STATUS_ERROR == hal_rtc_enter_rtc_mode()) {
                    APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", Enter RTC mode fail !!!", 0);
                    if (i > RETRY_ENTER_RTC_MODE_TIMES) {
                        configASSERT(0 && "Enter RTC mode fail !!!");
                    }
                } else {
                    break;
                }
            }
        } else if (APP_HOME_SCREEN_STATE_REBOOT == s_app_homescreen_context.state) {
            /* Reset WDT as reboot action. */
            hal_wdt_status_t ret;
            APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", app_home_screen_check_and_do_power_off reboot", 0);
            ret = hal_wdt_software_reset();
            APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", app_home_screen_check_and_do_power_off reboot ret = %d", 1, ret);
        }
    }
}

static void app_home_screen_check_and_do_power_off(home_screen_local_context_type_t *local_context)
{
    /* Must confirm BT off before system power off and reboot. */
    if (local_context->bt_power_state == APP_HOME_SCREEN_BT_POWER_STATE_DISABLED) {
        if (APP_HOME_SCREEN_STATE_POWERING_OFF == local_context->state
            && !local_context->power_off_waiting_time_out
#ifdef AIRO_KEY_EVENT_ENABLE
            && local_context->power_off_waiting_release_key == DEVICE_KEY_NONE
#endif
            ) {
            APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", ready to system off", 0);
            s_ready_to_off = true;
#ifndef NVDM_SUPPORT_MULTIPLE_PARTITION
            nvdm_trigger_garbage_collection(NVDM_GC_IN_CURR, 0);
#else
            nvdm_trigger_garbage_collection(0, NVDM_GC_IN_CURR, 0);
            nvdm_trigger_garbage_collection(1, NVDM_GC_IN_CURR, 0);
#endif
        } else if (APP_HOME_SCREEN_STATE_REBOOT == local_context->state
                   && !local_context->power_off_waiting_time_out) {
            APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", ready to reboot", 0);
            s_ready_to_off = true;
        }
    }
}

static void _trigger_power_off_flow(struct _ui_shell_activity *self, bool need_wait)
{
    home_screen_local_context_type_t *local_context = (home_screen_local_context_type_t *)(self->local_context);

    /* Should return if Homescreen APP is powering off or reboot state. */
    if (local_context->state != APP_HOME_SCREEN_STATE_IDLE) {
        APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", _trigger_power_off_flow, already prepared to power off, just wait", 0);
        return;
    } else {
        local_context->state = APP_HOME_SCREEN_STATE_POWERING_OFF;
    }
    if (need_wait) {
        local_context->power_off_waiting_time_out = true;
        ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                            APPS_EVENTS_INTERACTION_POWER_OFF_WAIT_TIMEOUT, NULL, 0,
                            NULL, WAIT_TIME_BEFORE_POWER_OFF);
    }
    /* ANC suspend before power off. */
#ifdef MTK_ANC_ENABLE
    app_anc_service_save_into_flash();
    bool anc_ret = app_anc_service_suspend();
    APPS_LOG_MSGID_I("audio_anc_suspend after when power off in ret = %d", 1, anc_ret);
#endif
    /* Disable Audio before power off to avoid pop sound. */
    ami_audio_power_off_flow();
    /* Disable BT with for_system_off by using BT state service API. */
    app_bt_state_service_set_bt_on_off(false, false, true, true);

    /* Send delay event to power off directly if need_wait. */
    if (!need_wait) {
        /* Check and do power off. */
        local_context->power_off_waiting_time_out = false;
        app_home_screen_check_and_do_power_off(local_context);
    }
}

static void _trigger_reboot_flow(struct _ui_shell_activity *self, uint32_t wait_time)
{
    home_screen_local_context_type_t *local_context = (home_screen_local_context_type_t *)(self->local_context);

    /* Should return if Homescreen APP is powering off or reboot state. */
    if (local_context->state != APP_HOME_SCREEN_STATE_IDLE) {
        APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", reboot flow, already prepared to power off when request reboot, just wait", 0);
        return;
    } else {
        local_context->state = APP_HOME_SCREEN_STATE_REBOOT;
        /* ANC suspend before power off. */
#ifdef MTK_ANC_ENABLE
        app_anc_service_save_into_flash();
        bool anc_ret = app_anc_service_suspend();
        APPS_LOG_MSGID_I("audio_anc_suspend after when reboot in ret = %d", 1, anc_ret);
#endif
        /* Disable Audio before power off to avoid pop sound. */
        ami_audio_power_off_flow();
        /* Disable BT with for_system_off by using BT state service API. */
        app_bt_state_service_set_bt_on_off(false, false, false, true);
    }
    if (wait_time > 0) {
        local_context->power_off_waiting_time_out = true;
        ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                            APPS_EVENTS_INTERACTION_POWER_OFF_WAIT_TIMEOUT, NULL, 0,
                            NULL, wait_time);
    } else {
        app_home_screen_check_and_do_power_off(local_context);
    }
}

#ifdef MTK_ANC_ENABLE

#ifdef MTK_ANC_HOWLING_TURN_OFF_ANC
static void app_home_screen_disable_anc_when_howling(ui_shell_activity_t *self)
{
    home_screen_local_context_type_t *local_ctx = (home_screen_local_context_type_t *)self->local_context;
    bool control_ret = FALSE;

    if (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_PARTNER
        && local_ctx && local_ctx->aws_connected) {
        if (BT_STATUS_SUCCESS != apps_aws_sync_event_send(EVENT_GROUP_UI_SHELL_AUDIO_ANC, ANC_CONTROL_EVENT_HOWLING)) {
            control_ret = app_anc_service_disable();
            //apps_config_set_vp(VP_INDEX_FAILED, false, 0, VOICE_PROMPT_PRIO_MEDIUM, false, NULL);
            APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", Partner send ANC_CONTROL_EVENT_HOWLING to agent failed, disable self ret : %d",
                             1, control_ret);
        } else {
            APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", Partner send ANC_CONTROL_EVENT_HOWLING to agent success", 0);
        }
    } else {
        control_ret = app_anc_service_disable();
        //apps_config_set_vp(VP_INDEX_FAILED, false, 0, VOICE_PROMPT_PRIO_MEDIUM, false, NULL);
        APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", disable anc filter when ANC_CONTROL_EVENT_HOWLING, ret: %d",
                         1, control_ret);
    }
}
#endif

static bool app_home_screen_process_anc_and_pass_through(ui_shell_activity_t *self, apps_config_key_action_t key_action)
{
    bool ret = false;
    audio_anc_control_filter_id_t target_filter_id;
    audio_anc_control_type_t target_anc_type;
    home_screen_local_context_type_t *local_ctx = (home_screen_local_context_type_t *)self->local_context;

    if (!local_ctx) {
        return ret;
    }

    uint8_t anc_enable;
    audio_anc_control_filter_id_t anc_current_filter_id;
    audio_anc_control_type_t anc_current_type;
    int16_t anc_runtime_gain;
    uint8_t support_hybrid_enable;
    audio_anc_control_get_status(&anc_enable, &anc_current_filter_id, &anc_current_type, &anc_runtime_gain, &support_hybrid_enable, NULL);

    if (KEY_PASS_THROUGH == key_action) {
        target_anc_type = AUDIO_ANC_CONTROL_PASS_THRU_TYPE_DEFAULT;
        target_filter_id = AUDIO_ANC_CONTROL_PASS_THRU_FILTER_DEFAULT;
        ret = true;
    } else if (KEY_ANC == key_action) {
        if (support_hybrid_enable) {
            target_anc_type = AUDIO_ANC_CONTROL_ANC_TYPE_DEFAULT; /* See request: hybrid, ff or fb.*/
        } else {
            target_anc_type = AUDIO_ANC_CONTROL_TYPE_FF;
        }
        target_filter_id = AUDIO_ANC_CONTROL_ANC_FILTER_DEFAULT;
        ret = true;
    } else if (KEY_SWITCH_ANC_AND_PASSTHROUGH == key_action) {
        /* Switch loop is OFF->PassThrough->ANC->OFF. */
        if (!anc_enable) {
            /* When last is OFF, next state is PassThrough. */
            target_anc_type = AUDIO_ANC_CONTROL_PASS_THRU_TYPE_DEFAULT;
            target_filter_id = AUDIO_ANC_CONTROL_PASS_THRU_FILTER_DEFAULT;
        } else {
            /* If current filter is ANC_FILTER, must set to OFF, target is ANC_FILTER
            and if current filter is PassThrough filter, target is ANC filter. */
            if (support_hybrid_enable) {
                target_anc_type = AUDIO_ANC_CONTROL_ANC_TYPE_DEFAULT;
            } else {
                target_anc_type = AUDIO_ANC_CONTROL_TYPE_FF;
            }
            target_filter_id = AUDIO_ANC_CONTROL_ANC_FILTER_DEFAULT;
        }
        ret = true;
    } else if (KEY_BETWEEN_ANC_PASSTHROUGH == key_action) {
        if (anc_enable && (AUDIO_ANC_CONTROL_PASS_THRU_FILTER_DEFAULT == anc_current_filter_id)) {
            /* Set as ANC. */
            if (support_hybrid_enable) {
                /* See request: hybrid, ff or fb.*/
                target_anc_type = AUDIO_ANC_CONTROL_ANC_TYPE_DEFAULT;
            } else {
                target_anc_type = AUDIO_ANC_CONTROL_TYPE_FF;
            }
            target_filter_id = AUDIO_ANC_CONTROL_ANC_FILTER_DEFAULT;
        } else {
            /* Set as passthrough. */
            target_anc_type = AUDIO_ANC_CONTROL_PASS_THRU_TYPE_DEFAULT;
            target_filter_id = AUDIO_ANC_CONTROL_PASS_THRU_FILTER_DEFAULT;
        }
        ret = true;
    } else {
        return false;
    }

#ifdef MTK_AWS_MCE_ENABLE
    if (local_ctx && local_ctx->aws_connected) {
        if (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_PARTNER) {
            if (BT_STATUS_SUCCESS != apps_aws_sync_event_send(EVENT_GROUP_UI_SHELL_KEY, key_action)) {
                APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", Partner send KEY_ANC or KEY_PASS_THROUGH aws to agent failed : %d", 1, key_action);
                //apps_config_set_vp(VP_INDEX_FAILED, false, 0, VOICE_PROMPT_PRIO_MEDIUM, false, NULL);
            } else {
                APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", Partner send KEY_ANC or KEY_PASS_THROUGH aws to agent success : %d", 1, key_action);
            }
            return ret;
        }
    } else {
        APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", app_home_screen_process_anc_and_pass_through, aws_connected is false", 0);
        //apps_config_set_vp(VP_INDEX_FAILED, false, 0, VOICE_PROMPT_PRIO_MEDIUM, false, NULL);
        return ret;
    }
#endif

    bool control_ret = FALSE;
    if (anc_enable && (target_filter_id == anc_current_filter_id)) {
        control_ret = app_anc_service_disable();
        APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", app_homescreen_idle_activity, disable anc filter : %d ret: %d", 2,
                         target_filter_id, control_ret);
    } else {
        control_ret = app_anc_service_enable(target_filter_id, target_anc_type, anc_runtime_gain, NULL);
        APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", app_homescreen_idle_activity, enable anc filter %d ret: %d", 2,
                         target_filter_id, control_ret);
    }
    //apps_config_set_vp(VP_INDEX_SUCCESSED, true, 200, VOICE_PROMPT_PRIO_MEDIUM, false, NULL);
    return ret;
}
#endif

static nvdm_status_t app_home_screen_fact_rst_nvdm_flag(void)
{
    uint8_t factrst_flag;
    nvdm_status_t status;

    APPS_LOG_MSGID_I("Write Factory reset flag to NVDM", 0);

    factrst_flag = FACTORY_RESET_FLAG;
    status = nvkey_write_data(NVID_SYS_FACTORY_RESET_FLAG, &factrst_flag, 1);

    return status;
}

static nvdm_status_t app_home_screen_fact_rst_link_key_nvdm_flag(void)
{
    uint8_t factrst_flag;
    nvdm_status_t status;

    APPS_LOG_MSGID_I("Write Factory reset flag to NVDM", 0);

    factrst_flag = FACTORY_RESET_LINK_KEY;
    status = nvkey_write_data(NVID_SYS_FACTORY_RESET_FLAG, &factrst_flag, 1);

    return status;
}

#ifdef AIR_BT_ULTRA_LOW_LATENCY_ENABLE
static bt_status_t app_home_screen_ull_dongle_connect_headset(bt_bd_addr_t addr)
{
    bt_cm_connect_t param = {
        .profile = BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_CUSTOMIZED_ULL),
    };
    memcpy(param.address, addr, sizeof(param.address));
    bt_status_t bt_status = bt_cm_connect(&param);
    if (BT_STATUS_SUCCESS != bt_status) {
        APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", bt_cm_connect: result;%x", 1, bt_status);
    }
    return bt_status;
}

static bt_status_t app_home_screen_ull_dongle_start_air_pairing(void)
{
    bt_status_t ret = BT_STATUS_FAIL;
    bt_ull_pairing_info_t air_pairing_data = {
        .duration = APPS_AIR_PAIRING_DURATION,
        .role = BT_ULL_ROLE_SERVER,
        .key = APPS_AIR_PAIRING_KEY,
        .info = APPS_AIR_PAIRING_INFO,
        .rssi_threshold = AIR_PAIRING_RSSI_THRESHOLD,
    };
    ret = bt_ull_action(BT_ULL_ACTION_START_PAIRING, &air_pairing_data, sizeof(air_pairing_data));

    return ret;
}

bt_status_t app_home_screen_ull_dongle_start_source(const bt_addr_t addr, app_dongle_cm_start_source_param_t param)
{
    bt_status_t ret = BT_STATUS_FAIL;
    bt_cm_write_scan_mode(BT_CM_COMMON_TYPE_UNKNOW, BT_CM_COMMON_TYPE_ENABLE);
    if (param.is_air_pairing) {
        ret = app_home_screen_ull_dongle_start_air_pairing();
        if (BT_STATUS_SUCCESS == ret) {
            app_bt_state_service_set_ull_air_pairing_doing(true);
        }
    } else {
        if (memcmp(s_app_homescreen_context.ull_peer_addr, s_empty_addr, sizeof(s_app_homescreen_context.ull_peer_addr)) != 0) {
            ret = app_home_screen_ull_dongle_connect_headset(s_app_homescreen_context.ull_peer_addr);
        }
    }
    APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", dongle_start_source(%d), ret:%d,addr: %02X:%02X:%02X:%02X:%02X:%02X", 8,
                     param.is_air_pairing, ret,
                     s_app_homescreen_context.ull_peer_addr[0], s_app_homescreen_context.ull_peer_addr[1], s_app_homescreen_context.ull_peer_addr[2],
                     s_app_homescreen_context.ull_peer_addr[3], s_app_homescreen_context.ull_peer_addr[4], s_app_homescreen_context.ull_peer_addr[5]);
    return ret;
}

bt_status_t app_home_screen_ull_dongle_stop_source(const bt_addr_t addr, app_dongle_cm_stop_source_param_t param)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    bool ull_pairing = app_bt_connection_service_get_current_status()->in_ull_pairing;
    if (ull_pairing) {
        ret = bt_ull_action(BT_ULL_ACTION_STOP_PAIRING, NULL, 0);
    }
    if (memcmp(s_app_homescreen_context.ull_peer_addr, s_empty_addr, sizeof(s_app_homescreen_context.ull_peer_addr)) != 0) {
        bt_bd_addr_t connected_devices[2];
        uint32_t connected_devices_count = 2;
        uint32_t i;
        bool ull_connected = false;
        connected_devices_count = bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK_NONE, connected_devices, connected_devices_count);
        for (i = 0; i < connected_devices_count; i++) {
            if (0 != memcmp(connected_devices[i], s_app_homescreen_context.ull_peer_addr, sizeof(bt_bd_addr_t))) {
                ull_connected = true;
                break;
            }
        }
        bt_cm_connect_t connect_param = {
            .profile = BT_CM_PROFILE_SERVICE_MASK_ALL,
        };
        memcpy(connect_param.address, s_app_homescreen_context.ull_peer_addr, sizeof(bt_bd_addr_t));
        ret = bt_cm_disconnect(&connect_param);
        if (!ull_connected) {
            app_dongle_cm_notify_event(APP_DONGLE_CM_SOURCE_ULL_V1, APP_DONGLE_CM_EVENT_SOURCE_END, BT_STATUS_SUCCESS, NULL);
        }
    }
    bt_cm_write_scan_mode(BT_CM_COMMON_TYPE_UNKNOW, BT_CM_COMMON_TYPE_DISABLE);
    APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", dongle_stop_source(%d), ret:%d,addr: %02X:%02X:%02X:%02X:%02X:%02X", 8,
                     ull_pairing, ret,
                     s_app_homescreen_context.ull_peer_addr[0], s_app_homescreen_context.ull_peer_addr[1], s_app_homescreen_context.ull_peer_addr[2],
                     s_app_homescreen_context.ull_peer_addr[3], s_app_homescreen_context.ull_peer_addr[4], s_app_homescreen_context.ull_peer_addr[5]);
    return ret;
}

const static app_dongle_cm_handle_t s_dongle_cm = {
    .start_source = app_home_screen_ull_dongle_start_source,
    .stop_source = app_home_screen_ull_dongle_stop_source,
    .precheck = NULL,
};
#endif

extern void bt_hfp_enable_ag_service_record(bool enable);

static bool _proc_ui_shell_group(struct _ui_shell_activity *self,
                                 uint32_t event_id,
                                 void *extra_data,
                                 size_t data_len)
{
    bool ret = true;
    home_screen_local_context_type_t *local_ctx = (home_screen_local_context_type_t *)self->local_context;

    switch (event_id) {
        case EVENT_ID_SHELL_SYSTEM_ON_CREATE: {
            APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", home_screen create", 0);
            /* Init context of Homescreen APP. */
            self->local_context = &s_app_homescreen_context;
            memset(self->local_context, 0, sizeof(home_screen_local_context_type_t));
            local_ctx = (home_screen_local_context_type_t *)self->local_context;
            local_ctx->state = APP_HOME_SCREEN_STATE_IDLE;
            local_ctx->bt_power_state = APP_HOME_SCREEN_BT_POWER_STATE_DISABLED;
            local_ctx->target_bt_power_state = APP_HOME_SCREEN_BT_POWER_STATE_DISABLED;
            local_ctx->connection_state = false;
#ifdef AIRO_KEY_EVENT_ENABLE
            local_ctx->power_off_waiting_release_key = DEVICE_KEY_NONE;
#endif
            local_ctx->is_bt_visiable = false;
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE)
            uint32_t address_len = sizeof(local_ctx->ull_peer_addr);
            nvkey_status_t nvkey_status;
            nvkey_status = nvkey_read_data(NVID_APP_ULL_PEER_BT_ADDRESS, local_ctx->ull_peer_addr, &address_len);
            if (NVKEY_STATUS_OK == nvkey_status) {
                APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", start peer addr: %02X:%02X:%02X:%02X:%02X:%02X", 6,
                                 local_ctx->ull_peer_addr[0], local_ctx->ull_peer_addr[1], local_ctx->ull_peer_addr[2],
                                 local_ctx->ull_peer_addr[3], local_ctx->ull_peer_addr[4], local_ctx->ull_peer_addr[5]);
            }
            app_dongle_cm_register_handle(APP_DONGLE_CM_SOURCE_ULL_V1, &s_dongle_cm);
#endif

#ifndef AIR_BT_SOURCE_ENABLE
            /* Disable BT profile SDP. */
            bt_hfp_enable_ag_service_record(false);
            bt_hsp_enable_service_record(false);
            bt_hfp_enable_service_record(false);
            bt_a2dp_enable_service_record(false);
            bt_avrcp_disable_sdp(true);
#endif
            break;
        }
        case EVENT_ID_SHELL_SYSTEM_ON_DESTROY:
            APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", app_homescreen_idle_activity destroy", 0);
            break;
        case EVENT_ID_SHELL_SYSTEM_ON_RESUME:
            APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", app_homescreen_idle_activity resume", 0);
            break;
        case EVENT_ID_SHELL_SYSTEM_ON_PAUSE:
            APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", app_homescreen_idle_activity pause", 0);
            break;
        case EVENT_ID_SHELL_SYSTEM_ON_REFRESH:
            APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", app_homescreen_idle_activity refresh", 0);
            break;
        case EVENT_ID_SHELL_SYSTEM_ON_RESULT:
            APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", app_homescreen_idle_activity result", 0);
            if (extra_data) {
                APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", extra data for app_homescreen_idle_activity result", 0);
            }
            break;
        default:
            break;
    }
    return ret;
}

static bool _proc_key_event_group(ui_shell_activity_t *self,
                                  uint32_t event_id,
                                  void *extra_data,
                                  size_t data_len)
{
    bool ret = false;
    home_screen_local_context_type_t *local_context = (home_screen_local_context_type_t *)(self->local_context);

    uint8_t key_id;
    airo_key_event_t key_event;
    app_event_key_event_decode(&key_id, &key_event, event_id);

#ifdef AIRO_KEY_EVENT_ENABLE
    /* Do not power off before key released. */
    if (local_context->power_off_waiting_release_key == key_id && key_event == AIRO_KEY_RELEASE) {
        local_context->power_off_waiting_release_key = DEVICE_KEY_NONE;
        app_home_screen_check_and_do_power_off(local_context);
    }
#endif

    /* Ignore other key event if Homescreen APP will power off. */
    if (APP_HOME_SCREEN_STATE_POWERING_OFF == local_context->state) {
        return true;
    }

    apps_config_key_action_t action;
    if (extra_data) {
        action = *(uint16_t *)extra_data;
    } else {
        action = apps_config_key_event_remapper_map_action(key_id, key_event);
    }

    switch (action) {
        case KEY_BT_OFF: {
            APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", Send REQUEST_ON_OFF_BT", 0);
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST,
                                EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_REQUEST_ON_OFF_BT,
                                (void *)false, 0,
                                NULL, 0);
            ret = true;
            break;
        }
        case KEY_POWER_OFF:
            ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_POWER_OFF, NULL, 0, NULL, 0);
            /* Apply "power off" VP and foreground LED pattern. */
            //apps_config_set_vp(VP_INDEX_POWER_OFF, false, 0, VOICE_PROMPT_PRIO_EXTREME, true, NULL);
            apps_config_set_foreground_led_pattern(LED_INDEX_POWER_OFF, 30, false);
            _trigger_power_off_flow(self, true);
#ifdef AIRO_KEY_EVENT_ENABLE
            /* When use long press to power off, must wait the key release. */
            if (key_event >= AIRO_KEY_LONG_PRESS_1 && key_event <= AIRO_KEY_SLONG) {
                local_context->power_off_waiting_release_key = key_id;
            }
#endif
            ret = true;
            break;
        case KEY_POWER_ON:
            /* Power on BT if current bt_power_state is disabled. */
            if (local_context->bt_power_state == APP_HOME_SCREEN_BT_POWER_STATE_DISABLED) {
                app_bt_state_service_set_bt_on_off(true, false, false, false);
                //apps_config_set_vp(VP_INDEX_POWER_ON, false, 0, VOICE_PROMPT_PRIO_HIGH, false, NULL);
                apps_config_set_foreground_led_pattern(LED_INDEX_POWER_ON, 30, false);
            }
            break;
        case KEY_SYSTEM_REBOOT:
            _trigger_reboot_flow(self, 0);
            ret = true;
            break;
#if 0 /* Dongle not need */
        case KEY_DISCOVERABLE:
#ifdef MTK_AWS_MCE_ENABLE
            if (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_PARTNER) {
                if (local_context->aws_connected) {
                    if (BT_STATUS_SUCCESS != apps_aws_sync_event_send(EVENT_GROUP_UI_SHELL_KEY, KEY_DISCOVERABLE)) {
                        APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", Partner send KEY_DISCOVERABLE aws to agent failed", 0);
                        //apps_config_set_vp(VP_INDEX_FAILED, false, 100, VOICE_PROMPT_PRIO_MEDIUM, false, NULL);
                    } else {
                        //apps_config_set_vp(VP_INDEX_SUCCESSED, false, 100, VOICE_PROMPT_PRIO_MEDIUM, false, NULL);
                    }
                } else {
                    app_bt_state_service_set_bt_visible(true, false, VISIBLE_TIMEOUT);
                    //apps_config_set_vp(VP_INDEX_PAIRING, false, 100, VOICE_PROMPT_PRIO_MEDIUM, false, NULL);
                    APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", Partner aws disconnected for KEY_DISCOVERABLE", 0);
                }
            } else if (bt_device_manager_aws_local_info_get_role() != BT_AWS_MCE_ROLE_CLINET)
#endif
            {
                app_bt_state_service_set_bt_visible(true, false, VISIBLE_TIMEOUT);
                //apps_config_set_vp(VP_INDEX_PAIRING, false, 100, VOICE_PROMPT_PRIO_MEDIUM, false, NULL);
            }
            ret = true;
            break;
#endif
        case KEY_RECONNECT_LAST_DEVICE:
            /* Send AWS sync event to Agent for Partner role and AWS connected. */
            /* Play "fail" VP if Partner send fail or AWS not connected. */
#ifdef MTK_AWS_MCE_ENABLE
            if (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_PARTNER) {
                if (local_context->aws_connected) {
                    if (BT_STATUS_SUCCESS != apps_aws_sync_event_send(EVENT_GROUP_UI_SHELL_KEY, KEY_RECONNECT_LAST_DEVICE)) {
                        APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", Partner send KEY_RECONNECT_LAST_DEVICE aws to agent failed", 0);
                        //apps_config_set_vp(VP_INDEX_FAILED, false, 0, VOICE_PROMPT_PRIO_MEDIUM, false, NULL);
                    } else {
                        APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", Partner send KEY_RECONNECT_LAST_DEVICE aws to agent success", 0);
                    }
                } else {
                    //apps_config_set_vp(VP_INDEX_FAILED, false, 0, VOICE_PROMPT_PRIO_MEDIUM, false, NULL);
                    APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", Partner aws disconnected for KEY_RECONNECT_LAST_DEVICE", 0);
                }
            } else if (bt_device_manager_aws_local_info_get_role() != BT_AWS_MCE_ROLE_CLINET)
#endif
            {
                bt_bd_addr_t *p_bd_addr = bt_device_manager_remote_get_dev_by_seq_num(1);
                bt_cm_connect_t connect_param = { {0},
                    BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS)
                    | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP)
                    | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK)
                    | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AVRCP)
                };
                if (p_bd_addr) {
                    memcpy(connect_param.address, *p_bd_addr, sizeof(bt_bd_addr_t));
                    APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", app_homescreen_idle_activity receive KEY_RECONNECT_LAST_DEVICE", 0);
                    bt_cm_connect(&connect_param);
                    //apps_config_set_vp(VP_INDEX_SUCCESSED, false, 0, VOICE_PROMPT_PRIO_MEDIUM, false, NULL);
                }
            }
            ret = true;
            break;
        case KEY_RESET_PAIRED_DEVICES:
#ifdef MTK_AWS_MCE_ENABLE
            if (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_PARTNER) {
                if (local_context->aws_connected) {
                    if (BT_STATUS_SUCCESS != apps_aws_sync_event_send(EVENT_GROUP_UI_SHELL_KEY, KEY_RESET_PAIRED_DEVICES)) {
                        APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", Partner send KEY_RESET_PAIRED_DEVICES aws to agent failed", 0);
                        //apps_config_set_vp(VP_INDEX_FAILED, false, 0, VOICE_PROMPT_PRIO_MEDIUM, false, NULL);
                    } else {
                        APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", Partner send KEY_RESET_PAIRED_DEVICES aws to agent success", 0);
                    }
                } else {
                    //apps_config_set_vp(VP_INDEX_FAILED, false, 0, VOICE_PROMPT_PRIO_MEDIUM, false, NULL);
                    APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", Partner aws disconnected for KEY_RESET_PAIRED_DEVICES", 0);
                }
            } else if (bt_device_manager_aws_local_info_get_role() != BT_AWS_MCE_ROLE_CLINET)
#endif
            {
                APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", app_homescreen_idle_activity receive KEY_RESET_PAIRED_DEVICES", 0);
                bt_device_manager_unpair_all();
                //apps_config_set_vp(VP_INDEX_SUCCESSED, false, 0, VOICE_PROMPT_PRIO_MEDIUM, false, NULL);
            }
            ret = true;
            break;
#ifdef MTK_ANC_ENABLE
        case KEY_PASS_THROUGH:
        case KEY_ANC:
        case KEY_SWITCH_ANC_AND_PASSTHROUGH:
        case KEY_BETWEEN_ANC_PASSTHROUGH:
            /* Handle ANC key event. */
            ret = app_home_screen_process_anc_and_pass_through(self, action);
            break;
#endif
        case KEY_FACTORY_RESET:
            APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", Receive key to do factory reset", 0);
#ifdef MTK_AWS_MCE_ENABLE
            if (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_PARTNER) {
                if (BT_STATUS_SUCCESS != apps_aws_sync_event_send(EVENT_GROUP_UI_SHELL_KEY, action)) {
                    APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", sync KEY_FACTORY_RESET_AND_POWEROFF fail.", 0);
                    //apps_config_set_vp(VP_INDEX_FAILED, false, 0, VOICE_PROMPT_PRIO_MEDIUM, false, NULL);
                }
                break;
            }
            //apps_config_set_vp(VP_INDEX_SUCCESSED, true, 100, VOICE_PROMPT_PRIO_EXTREME, false, NULL);
#else
            //apps_config_set_vp(VP_INDEX_SUCCESSED, false, 0, VOICE_PROMPT_PRIO_EXTREME, false, NULL);
#endif
            s_factory_reset_key_action = KEY_FACTORY_RESET;
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_FACTORY_RESET_REQUEST, NULL, 0,
                                NULL, 0);
            ret = true;
            break;

        case KEY_FACTORY_RESET_AND_POWEROFF:
            APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", Receive key to do factory reset and power off", 0);
#ifdef MTK_AWS_MCE_ENABLE
            if (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_PARTNER) {
                if (BT_STATUS_SUCCESS != apps_aws_sync_event_send(EVENT_GROUP_UI_SHELL_KEY, action)) {
                    APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", sync KEY_FACTORY_RESET_AND_POWEROFF fail.", 0);
                    //apps_config_set_vp(VP_INDEX_FAILED, false, 0, VOICE_PROMPT_PRIO_MEDIUM, false, NULL);
                }
                break;
            }
            //apps_config_set_vp(VP_INDEX_SUCCESSED, true, 100, VOICE_PROMPT_PRIO_EXTREME, false, NULL);
#else
            //apps_config_set_vp(VP_INDEX_SUCCESSED, false, 0, VOICE_PROMPT_PRIO_EXTREME, false, NULL);
#endif
            s_factory_reset_key_action = KEY_FACTORY_RESET_AND_POWEROFF;
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_FACTORY_RESET_REQUEST, NULL, 0,
                                NULL, 0);
            ret = true;
            break;

        case KEY_RESET_LINK_KEY:
            /* Clear link key then reboot, only for Agent. */
            APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", Receive key to do reset link key", 0);
            app_home_screen_fact_rst_link_key_nvdm_flag();
#ifdef MTK_AWS_MCE_ENABLE
            if (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT) {
                if (BT_STATUS_SUCCESS != apps_aws_sync_event_send(EVENT_GROUP_UI_SHELL_KEY, action)) {
                    APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", Fail to send KEY_RESET_LINK_KEY to partner", 0);
                }
                ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                    APPS_EVENTS_INTERACTION_REQUEST_REBOOT, NULL, 0,
                                    NULL, 100);
            } else
#else
            {
                _trigger_reboot_flow(self, 0);
            }
#endif
                ret = true;
            break;
        case KEY_ULL_AIR_PAIRING: {
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE)
            if (BT_STATUS_SUCCESS == app_home_screen_ull_dongle_start_air_pairing()) {
                //apps_config_set_vp(VP_INDEX_PAIRING, false, 0, VOICE_PROMPT_PRIO_MEDIUM, false, NULL);
                apps_config_set_foreground_led_pattern(LED_INDEX_AIR_PAIRING, APPS_AIR_PAIRING_DURATION * 10, false);
                /* local_context->in_air_pairing = true; */
            } else {
                //apps_config_set_vp(VP_INDEX_FAILED, false, 0, VOICE_PROMPT_PRIO_MEDIUM, false, NULL);
            }
#endif
            ret = true;
            break;
        }
#ifdef AIR_BLE_AUDIO_DONGLE_ENABLE
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
        case KEY_LE_AUDIO_SCAN: {
            app_le_audio_ucst_connect_coordinated_set(true);
            break;
        }
#endif
#endif
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
        case KEY_LE_ULL_PAIRING: {
            app_ull_dongle_le_scan_new_device();
            break;
        }
#endif
        case KEY_DONGLE_COMMON_PAIRING: {
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_AUDIO_DONGLE_ENABLE) ||defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE)
            app_dongle_cm_pairing_key_event_handler();
            break;
#endif
        }
        case KEY_TEST_MODE_ENTER_DUT_MODE: {
            /* Read BT_DUT_ENABLE flag from NVKEY -> inverse value -> write to NVKEY -> reboot. */
            bool dut_config = false;
            uint32_t dut_size = sizeof(dut_config);
            nvkey_status_t nvkey_ret = nvkey_read_data(NVID_BT_HOST_DUT_ENABLE, (uint8_t *)(&dut_config), &dut_size);
            APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY"ENTER_DUT_MODE nvkey_read:%d, dut_config:%d\r\n", 2, nvkey_ret, dut_config);
            if (NVKEY_STATUS_OK == nvkey_ret) {
                dut_config = !dut_config;
            } else {
                dut_config = true;
            }
            dut_size = sizeof(dut_config);
            nvkey_ret = nvkey_write_data(NVID_BT_HOST_DUT_ENABLE, (uint8_t *)(&dut_config), dut_size);
            APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY"ENTER_DUT_MODE nvkey_write:%d", 1, nvkey_ret);
            _trigger_reboot_flow(self, 0);
        }
        break;
        case KEY_TEST_MODE_ENTER_RELAY_MODE: {
            /* Read BT_RELAY_ENABLE flag from NVKEY -> inverse value -> write to NVKEY -> reboot. */
            bt_power_on_relay_config_t relay_config = {
                .relay_enable = false,
                .port_number = 0,
            };
            uint32_t relay_size = sizeof(relay_config);
            nvkey_status_t nvkey_ret = nvkey_read_data(NVID_BT_HOST_RELAY_ENABLE, (uint8_t *)(&relay_config), &relay_size);
            APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY"ENTER_RELAY_MODE nvkey_read:%d, relay_enable:%d, port_number:%d\r\n", 3, nvkey_ret, relay_config.relay_enable, relay_config.port_number);
            if (NVKEY_STATUS_OK == nvkey_ret) {
                relay_config.relay_enable = !relay_config.relay_enable;
            } else {
                relay_config.relay_enable = true;
                relay_config.port_number = 0;
            }
            relay_size = sizeof(relay_config);
            nvkey_ret = nvkey_write_data(NVID_BT_HOST_RELAY_ENABLE, (uint8_t *)(&relay_config), relay_size);
            APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY"ENTER_RELAY_MODE nvkey_write:%d", 3, nvkey_ret);
            _trigger_reboot_flow(self, 0);
        }
        break;
    }
    return ret;
}

static apps_config_state_t homescreen_app_get_mmi_state(ui_shell_activity_t *self)
{
    home_screen_local_context_type_t *local_context = (home_screen_local_context_type_t *)(self->local_context);
    apps_config_state_t mmi_state = APP_BT_OFF;

    if (!local_context) {
        return mmi_state;
    }
    if (APP_HOME_SCREEN_BT_POWER_STATE_DISABLED == local_context->bt_power_state) {
        mmi_state = APP_BT_OFF;
    } else {
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
        if (app_ull_dongle_le_get_link_num() > 0) {
            mmi_state = APP_CONNECTED;
            return mmi_state;
        }
#endif
#ifdef AIR_BLE_AUDIO_DONGLE_ENABLE
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
        if (app_le_audio_ucst_get_link_num() > 0) {
            mmi_state = APP_CONNECTED;
            return mmi_state;
        }
#endif
#endif
        if (local_context->connection_state) {
            mmi_state = APP_CONNECTED;
        } else {
            mmi_state = local_context->is_bt_visiable ? APP_CONNECTABLE : APP_DISCONNECTED;
        }
    }

    return mmi_state;
}

static bool homescreen_app_bt_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;
#ifdef AIR_BLE_AUDIO_DONGLE_ENABLE
    ret = bt_le_audio_component_bt_event_proc(self, event_id, extra_data, data_len);
#endif
    return ret;
}

static bool homescreen_app_bt_connection_manager_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;
    home_screen_local_context_type_t *local_context = (home_screen_local_context_type_t *)(self->local_context);
    if (!local_context) {
        return ret;
    }
#if defined (AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BT_SOURCE_ENABLE)
    ret = bt_conn_component_bt_cm_event_proc(self, event_id, extra_data, data_len);
#endif
#ifdef AIR_BLE_AUDIO_DONGLE_ENABLE
    ret = bt_le_audio_component_bt_cm_event_proc(self, event_id, extra_data, data_len);
#endif

    switch (event_id) {
        case BT_CM_EVENT_REMOTE_INFO_UPDATE: {
#if 0
            bt_cm_remote_info_update_ind_t *remote_update = (bt_cm_remote_info_update_ind_t *)extra_data;
            if (!remote_update) {
                break;
            }
            if (0 == memcmp(remote_update->address, bt_device_manager_get_local_address(), sizeof(bt_bd_addr_t))) {
                break;
            }
            if (BT_CM_ACL_LINK_DISCONNECTED != remote_update->pre_acl_state
                && BT_CM_ACL_LINK_PENDING_CONNECT != remote_update->pre_acl_state
                && BT_CM_ACL_LINK_CONNECTING != remote_update->pre_acl_state
                && BT_CM_ACL_LINK_DISCONNECTED == remote_update->acl_state) {
                bt_bd_addr_t connected_devices[2];
                uint32_t connected_devices_count = 2;
                uint32_t i;
                bool still_connected = false;
                connected_devices_count = bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK_NONE, connected_devices, connected_devices_count);
                for (i = 0; i < connected_devices_count; i++) {
                    if (0 != memcmp(connected_devices[i], bt_device_manager_get_local_address(), sizeof(bt_bd_addr_t))) {
                        still_connected = true;
                        break;
                    }
                }
                if (!still_connected) {
                    /* Check all Smart phones are disconnected */
                    disconnect = true;
                } else {
                    APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", ACL disconnected but still have SP connected", 0);
                }
            } else if (BT_CM_ACL_LINK_CONNECTED != remote_update->pre_acl_state
                       && BT_CM_ACL_LINK_CONNECTED == remote_update->acl_state) {
                connect = true;
            }
#endif

#ifdef AIR_BT_ULTRA_LOW_LATENCY_ENABLE
            bt_cm_remote_info_update_ind_t *remote_update = (bt_cm_remote_info_update_ind_t *)extra_data;
            if (BT_CM_ACL_LINK_DISCONNECTED == remote_update->acl_state
                && BT_CM_ACL_LINK_DISCONNECTED != remote_update->pre_acl_state) {
                if (memcmp(local_context->ull_peer_addr, remote_update->address, sizeof(local_context->ull_peer_addr)) == 0) {
                    if (remote_update->reason == BT_HCI_STATUS_PIN_OR_KEY_MISSING) {
                        APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", dongle reconnect for key missing: %02X:%02X:%02X:%02X:%02X:%02X", 6,
                                        local_context->ull_peer_addr[0], local_context->ull_peer_addr[1], local_context->ull_peer_addr[2],
                                        local_context->ull_peer_addr[3], local_context->ull_peer_addr[4], local_context->ull_peer_addr[5]);
                        app_home_screen_ull_dongle_connect_headset(local_context->ull_peer_addr);
                    } else if (remote_update->reason != BT_HCI_STATUS_CONNECTION_TIMEOUT
                        && BT_CM_ACL_LINK_CONNECTING != remote_update->pre_acl_state
                        && APP_DONGLE_CM_STATE_STOP_CONNECTION == app_dongle_cm_get_source_state(APP_DONGLE_CM_SOURCE_ULL_V1)) {
                        app_dongle_cm_notify_event(APP_DONGLE_CM_SOURCE_ULL_V1, APP_DONGLE_CM_EVENT_SOURCE_END, BT_STATUS_SUCCESS, NULL);
                    }
                }
            }
            /* Connect AIR when ULL connected. */
            if (BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_CUSTOMIZED_ULL)
                & (remote_update->connected_service & ~remote_update->pre_connected_service)) {
                bt_cm_connect_t connect_param = {
                    .profile = BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AIR),
                };
                memcpy(&connect_param.address, remote_update->address, sizeof(bt_bd_addr_t));
                bt_cm_connect(&connect_param);
                if (APP_DONGLE_CM_STATE_START_CONNECTION == app_dongle_cm_get_source_state(APP_DONGLE_CM_SOURCE_ULL_V1)) {
                app_dongle_cm_notify_event(APP_DONGLE_CM_SOURCE_ULL_V1, APP_DONGLE_CM_EVENT_SOURCE_STARTED, BT_STATUS_SUCCESS, NULL);
            }
            }
#endif /* #ifdef AIR_BT_ULTRA_LOW_LATENCY_ENABLE */
            break;
        }
        case BT_CM_EVENT_POWER_STATE_UPDATE: {
            break;
        }
        default:
            break;
    }

#if 0
    /* Disconnected from Smart phone, set the flag to prepare start BT discoverable. */
    /* If user refused pairing on Smart phone, must restart discoverable. */
    if (bt_on || disconnect) {
        if (bt_on) {
            ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_BT_RECONNECT_TIMEOUT);
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_BT_RECONNECT_TIMEOUT, NULL, 0,
                                NULL, TIME_TO_STOP_RECONNECTION);
        }
    } else if (bt_off || connect) {
        ui_shell_remove_event(EVENT_GROUP_UI_SHELL_APP_INTERACTION, APPS_EVENTS_INTERACTION_BT_RECONNECT_TIMEOUT);
    }
#endif

    return ret;
}


static bool homescreen_app_bt_dm_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;
    home_screen_local_context_type_t *local_context = (home_screen_local_context_type_t *)(self->local_context);
    if (!local_context) {
        return ret;
    }

#ifdef AIR_BT_ULTRA_LOW_LATENCY_ENABLE
    ret = bt_conn_component_bt_dm_event_proc(self, event_id, extra_data, data_len);
#elif defined (AIR_BLE_AUDIO_DONGLE_ENABLE) || defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
    ret = bt_le_audio_component_bt_dm_event_proc(self, event_id, extra_data, data_len);
#endif

    bt_device_manager_power_event_t evt;
    bt_device_manager_power_status_t status;
    bt_event_get_bt_dm_event_and_status(event_id, &evt, &status);

    switch (evt) {
        case BT_DEVICE_MANAGER_POWER_EVT_STANDBY_COMPLETE:
            if (BT_DEVICE_MANAGER_POWER_STATUS_SUCCESS == status) {
#ifdef AIR_USB_ENABLE
#if defined(AIR_BTA_IC_PREMIUM_G2) || defined(AIR_BTA_IC_STEREO_HIGH_G3)
                /**
                 * In USB suspend.
                 * After BT OFF, switch DCXO to low-power mode for power saving.
                 */
                if (USB_SUSPEND_STATE_SUSPEND == USB_Get_Suspend_State()) {
                    APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", dcxo_lp_mode, DCXO_LP_MODE", 0);
                    dcxo_lp_mode(DCXO_LP_MODE);
                }
#endif
#endif
            }
            break;
        default:
            break;
    }

    app_home_screen_check_and_do_power_off(local_context);

    return ret;
}


#ifdef AIR_BLE_AUDIO_DONGLE_ENABLE
static bool homescreen_app_le_audio_manager_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;
    switch (event_id) {
        case APP_LE_AUDIO_EVENT_START_SCAN_NEW_DEVICE:
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN, NULL, 0,
                                NULL, 0);
            break;
        case APP_LE_AUDIO_EVENT_STOP_SCAN_NEW_DEVICE:
            apps_config_set_foreground_led_pattern(LED_INDEX_AIR_PAIRING_SUCCESS, 30, false);
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN, NULL, 0,
                                NULL, 0);
            break;
        default:
            break;
    }
    return ret;
}
#endif

#if defined AIR_BT_ULTRA_LOW_LATENCY_ENABLE || defined AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
static bool homescreen_app_ull_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;
    switch (event_id) {
#if defined AIR_BT_ULTRA_LOW_LATENCY_ENABLE
        case BT_ULL_EVENT_PAIRING_COMPLETE_IND:
            if (extra_data) {
                home_screen_local_context_type_t *local_context = (home_screen_local_context_type_t *)(self->local_context);
                bt_ull_pairing_complete_ind_t *air_pairing_ind = (bt_ull_pairing_complete_ind_t *)extra_data;
                if (!air_pairing_ind || !local_context) {
                    break;
                }
                APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", Received BT_ULL_EVENT_PAIRING_COMPLETE_IND result= %d", 1, air_pairing_ind->result);
                /* local_context->in_air_pairing = false; */

                if (air_pairing_ind->result) {
                    memcpy(local_context->ull_peer_addr, air_pairing_ind->remote_address, sizeof(local_context->ull_peer_addr));
                    nvkey_write_data(NVID_APP_ULL_PEER_BT_ADDRESS, local_context->ull_peer_addr, sizeof(local_context->ull_peer_addr));
                    apps_config_set_foreground_led_pattern(LED_INDEX_AIR_PAIRING_SUCCESS, 30, false);
                    //apps_config_set_vp(VP_INDEX_SUCCESSED, false, 0, VOICE_PROMPT_PRIO_MEDIUM, false, NULL);
                    app_home_screen_ull_dongle_connect_headset(air_pairing_ind->remote_address);
                } else {
                    if (APP_DONGLE_CM_STATE_STOP_CONNECTION == app_dongle_cm_get_source_state(APP_DONGLE_CM_SOURCE_ULL_V1)) {
                        app_dongle_cm_notify_event(APP_DONGLE_CM_SOURCE_ULL_V1, APP_DONGLE_CM_EVENT_SOURCE_END, BT_STATUS_SUCCESS, NULL);
                    } else if (APP_DONGLE_CM_STATE_START_CONNECTION == app_dongle_cm_get_source_state(APP_DONGLE_CM_SOURCE_ULL_V1)) {
                        app_dongle_cm_notify_event(APP_DONGLE_CM_SOURCE_ULL_V1, APP_DONGLE_CM_EVENT_SOURCE_STARTED, BT_STATUS_FAIL, NULL);
                    }
                    apps_config_set_foreground_led_pattern(LED_INDEX_AIR_PAIRING_FAIL, 30, false);
                    //apps_config_set_vp(VP_INDEX_FAILED, false, 0, VOICE_PROMPT_PRIO_MEDIUM, false, NULL);
                }
            }
            break;
#endif
#if defined AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
        case BT_ULL_EVENT_LE_CONNECTED:
        case BT_ULL_EVENT_LE_DISCONNECTED: {
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                    APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN, NULL, 0,
                                    NULL, 0);
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                APPS_EVENTS_INTERACTION_UPDATE_MMI_STATE, NULL, 0,
                                NULL, 0);
            break;
        }
#endif
        default:
            break;
    }
    return ret;
}
#endif

static bool _app_interaction_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    home_screen_local_context_type_t *local_ctx = (home_screen_local_context_type_t *)self->local_context;
    bool ret = false;
#ifdef MTK_AWS_MCE_ENABLE
    bt_aws_mce_role_t role = bt_device_manager_aws_local_info_get_role();
#endif

    switch (event_id) {
#ifdef SUPPORT_ROLE_HANDOVER_SERVICE
        case APPS_EVENTS_INTERACTION_RHO_STARTED: {
            APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", homescreen received RHO started, role=0x%02X", 1, role);
        }
        break;
        case APPS_EVENTS_INTERACTION_RHO_END:
            APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", homescreen received RHO end", 0);
            break;
        case APPS_EVENTS_INTERACTION_PARTNER_SWITCH_TO_AGENT:
#ifdef APPS_TRIGGER_RHO_BY_KEY
            if (!apps_config_features_is_auto_rho_enabled()) {
                /* Partner should play VP when switch to Agent via KEY_RHO_TO_AGENT. */
                if (local_ctx->key_trigger_waiting_rho) {
                    app_rho_result_t result = (app_rho_result_t)extra_data;
                    if (APP_RHO_RESULT_SUCCESS == result) {
                        //apps_config_set_vp(VP_INDEX_SUCCESSED, false, 0, VOICE_PROMPT_PRIO_MEDIUM, false, NULL);
                    } else {
                        //apps_config_set_vp(VP_INDEX_FAILED, false, 0, VOICE_PROMPT_PRIO_MEDIUM, false, NULL);
                    }
                }
            }
#endif
            break;
#endif
        case APPS_EVENTS_INTERACTION_POWER_OFF_WAIT_TIMEOUT: {
            APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", Timeout before power off", 0);
            /* Check and do power off or reboot if WAIT_TIME_BEFORE_POWER_OFF timeout. */
            if (APP_HOME_SCREEN_STATE_IDLE != local_ctx->state) {
                local_ctx->power_off_waiting_time_out = false;
                app_home_screen_check_and_do_power_off(local_ctx);
                ret = true;
            }
        }
        break;
        case APPS_EVENTS_INTERACTION_REQUEST_POWER_OFF: {
            APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", Receive power off request", 0);
            /* Should wait power off timeout for INTERACTION_REQUEST_POWER_OFF event. */
            _trigger_power_off_flow(self, true);
            ret = true;
        }
        break;
        case APPS_EVENTS_INTERACTION_REQUEST_IMMEDIATELY_POWER_OFF: {
            APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", Receive immediately power off request", 0);
            _trigger_power_off_flow(self, false);
            ret = true;
        }
        break;
        case APPS_EVENTS_INTERACTION_REQUEST_REBOOT: {
            uint32_t delay_time = (uint32_t)extra_data;
            APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", Receive sys reboot request after:%d", 1, delay_time);
#if defined(MTK_NVDM_ENABLE) && defined(MTK_FOTA_VIA_RACE_CMD)
            bool fota_flag = false;
            fota_check_upgrade_flag(&fota_flag);
            if (fota_flag) {
                reserved_nvdm_item_list_ask_check();
            }
#endif
            _trigger_reboot_flow(self, delay_time);
            ret = true;
            break;
        }
        case APPS_EVENTS_INTERACTION_REQUEST_ON_OFF_BT: {
            /* Enable or disable BT via bt_state_service API. */
            bool enable_bt = (bool)extra_data;
            APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", Receive BT on/off request : %d", 1, enable_bt);
            app_bt_state_service_set_bt_on_off(enable_bt, false, true, false);
            break;
        }
        case APPS_EVENTS_INTERACTION_REQUEST_CLASSIC_BT_OFF: {
            APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", Receive Classic off", 0);
            app_bt_state_service_set_bt_on_off(false, true, true, false);
            break;
        }
        case APPS_EVENTS_INTERACTION_CLASSIC_OFF_TO_BT_OFF: {
            APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", CLASSIC_OFF_TO_BT_OFF request : %d", 1, local_ctx->target_bt_power_state);
            app_bt_state_service_set_bt_on_off(false, false, true, false);
            break;
        }
        case APPS_EVENTS_INTERACTION_UPDATE_LED_BG_PATTERN:
            ret = true;
            extern bool app_ull_dongle_has_call_sta();
            if (app_ull_dongle_has_call_sta()) {
                ret = false;
            }
            APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", Connection state = %x, is_bt_visiable = %d", 2, local_ctx->connection_state, local_ctx->is_bt_visiable);
            if (local_ctx->is_bt_visiable) {
                apps_config_set_background_led_pattern(LED_INDEX_CONNECTABLE, true, APPS_CONFIG_LED_AWS_SYNC_PRIO_MIDDLE);
            } else {
                apps_config_state_t app_state = homescreen_app_get_mmi_state(self);
                switch (app_state) {
                    case APP_BT_OFF:
                        apps_config_set_background_led_pattern(LED_INDEX_IDLE, false, APPS_CONFIG_LED_AWS_SYNC_PRIO_LOWEST);
                        break;
                    case APP_DISCONNECTED:
#ifdef AIR_BLE_AUDIO_DONGLE_ENABLE
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
                        if (app_le_audio_ucst_get_curr_conn_type() == APP_LE_AUDIO_UCST_CONN_COORDINATED_SET_BY_SIRK) {
                            apps_config_set_background_led_pattern(LED_INDEX_AIR_PAIRING, true, APPS_CONFIG_LED_AWS_SYNC_PRIO_LOW);
                        } else
#endif
#endif
                        {
                            apps_config_set_background_led_pattern(LED_INDEX_DISCONNECTED, true, APPS_CONFIG_LED_AWS_SYNC_PRIO_LOW);
                        }
                        break;
                    case APP_CONNECTED:
#ifdef MTK_AWS_MCE_ENABLE
                        if (role == BT_AWS_MCE_ROLE_PARTNER || role == BT_AWS_MCE_ROLE_CLINET) {
                            if (local_ctx->aws_connected) {
                                apps_config_set_background_led_pattern(LED_INDEX_IDLE, false, APPS_CONFIG_LED_AWS_SYNC_PRIO_LOWEST);
                            } else {
                                apps_config_set_background_led_pattern(LED_INDEX_DISCONNECTED, true, APPS_CONFIG_LED_AWS_SYNC_PRIO_LOW);
                            }
                        } else
#endif
                        {
#ifdef AIR_BLE_AUDIO_DONGLE_ENABLE
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
                            if (app_le_audio_ucst_get_link_num() == 1) {
                                /* This pattern is blick one LED. */
                                apps_config_set_background_led_pattern(LED_INDEX_CHARGING, false, APPS_CONFIG_LED_AWS_SYNC_PRIO_LOWEST);
                            } else
#endif
#endif
                            {
                                apps_config_set_background_led_pattern(LED_INDEX_IDLE, false, APPS_CONFIG_LED_AWS_SYNC_PRIO_LOWEST);
                            }
                        }
                        break;
                    default:
                        ret = false;
                        APPS_LOG_MSGID_W(UI_SHELL_IDLE_BT_CONN_ACTIVITY", Not supported state = %x", 1, local_ctx->connection_state);
                        break;
                }
            }
            break;
        case APPS_EVENTS_INTERACTION_UPDATE_MMI_STATE:
            ret = true;
            apps_config_key_set_mmi_state(homescreen_app_get_mmi_state(self));
            break;

#if APPS_AUTO_SET_BT_DISCOVERABLE
        case APPS_EVENTS_INTERACTION_AUTO_START_BT_VISIBLE:
            /* Update BT visibility via bt_state_service API. */
            app_bt_state_service_set_bt_visible(true, true, VISIBLE_TIMEOUT);
            ret = true;
            break;
#endif
        case APPS_EVENTS_INTERACTION_MULTI_VA_REMOVE_PAIRING_DONE:
            APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", MULTI_VA_REMOVE_PAIRING_DONE, next = %x", 1, s_factory_reset_pending_event);
            if (s_factory_reset_pending_event != KEY_ACTION_INVALID) {
                ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                    APPS_EVENTS_INTERACTION_FACTORY_RESET_REQUEST, NULL, 0,
                                    NULL, 0);
            }
            break;
        case APPS_EVENTS_INTERACTION_BT_RECONNECT_TIMEOUT: {
            APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", stop reconnect when time out.", 0);
            bt_bd_addr_t p_bd_addr[2];
            uint32_t connecting_number = 2;
            uint32_t i;
            connecting_number = bt_cm_get_connecting_devices(~BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS), p_bd_addr, connecting_number);
            bt_cm_connect_t connect_param = { {0},
                ~(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS))
            };
            for (i = 0; i < connecting_number; i++) {
                if (0 == bt_cm_get_gap_handle(p_bd_addr[i])) {
                    memcpy(connect_param.address, p_bd_addr[i], sizeof(bt_bd_addr_t));
                    bt_cm_disconnect(&connect_param);
                }
            }
            break;
        }
        case APPS_EVENTS_INTERACTION_FACTORY_RESET_REQUEST: {
            uint32_t action_after_factory_reset;
            APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", Receive factory reset request, is_doing:%d", 1, s_factory_reset_doing);
            /* if (MULTI_VA_SWITCH_OFF_WAIT_INACTIVE == multi_voice_assistant_manager_va_remove_pairing()) {
                s_factory_reset_pending_event = s_factory_reset_key_action;
                break;
            } else {
                s_factory_reset_pending_event = KEY_ACTION_INVALID;
            }
            */
            s_factory_reset_pending_event = KEY_ACTION_INVALID;

            if (s_factory_reset_doing) {
                APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", factory reset already started.", 0);
                break;
            }
            s_factory_reset_doing = true;

            if (s_factory_reset_key_action == KEY_FACTORY_RESET) {
                action_after_factory_reset = APPS_EVENTS_INTERACTION_REQUEST_REBOOT;
            } else {
                action_after_factory_reset = APPS_EVENTS_INTERACTION_REQUEST_POWER_OFF;
            }

#ifdef MTK_AWS_MCE_ENABLE
            if (!local_ctx->aws_connected) {
                APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", factory reset failed, aws not connected.", 0);
                //apps_config_set_vp(VP_INDEX_FAILED, false, 0, VOICE_PROMPT_PRIO_MEDIUM, false, NULL);
                s_factory_reset_doing = false;
                break;
            }

            if (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT) {
                if (BT_STATUS_SUCCESS != apps_aws_sync_event_send(EVENT_GROUP_UI_SHELL_KEY, s_factory_reset_key_action)) {
                    APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", Fail to send KEY_FACTORY_RESET_AND_POWEROFF to partner", 0);
                    //apps_config_set_vp(VP_INDEX_FAILED, false, 0, VOICE_PROMPT_PRIO_MEDIUM, false, NULL);
                    s_factory_reset_doing = false;
                    break;
                }
            }

            app_home_screen_fact_rst_nvdm_flag();
            if (bt_device_manager_aws_local_info_get_role() == BT_AWS_MCE_ROLE_AGENT) {
                //apps_config_set_vp(VP_INDEX_POWER_OFF, true, 1000, VOICE_PROMPT_PRIO_EXTREME, false, NULL);
            }
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                action_after_factory_reset, NULL, 0,
                                NULL, 4500);
#else
            app_home_screen_fact_rst_nvdm_flag();
            //apps_config_set_vp(VP_INDEX_SUCCESSED, false, 0, VOICE_PROMPT_PRIO_MEDIUM, false, NULL);
            ui_shell_send_event(false, EVENT_PRIORITY_HIGHEST, EVENT_GROUP_UI_SHELL_APP_INTERACTION,
                                action_after_factory_reset, NULL, 0,
                                NULL, 1500);
#endif
        }
        break;
#ifdef MTK_RACE_CMD_ENABLE
        case APPS_EVENTS_INTERACTION_LINK_QUALITY_ESCO_CRC_READ_ONCE: {
            extern uint8_t *bt_pka_get_esco_iso_statistic(void);
            ESCO_ISO_STATISTIC_STRU *esco_stru = (ESCO_ISO_STATISTIC_STRU *)bt_pka_get_esco_iso_statistic();
            uint8_t rate = 0;
            if (esco_stru->esco_bad_packet_cnt + esco_stru->esco_packet_cnt > 0) {
                rate = 100 * esco_stru->esco_bad_packet_cnt / (esco_stru->esco_bad_packet_cnt + esco_stru->esco_packet_cnt);
            } else if (esco_stru->iso_bad_packet_cnt + esco_stru->iso_packet_cnt > 0) {
                rate = 100 * esco_stru->iso_bad_packet_cnt / (esco_stru->iso_bad_packet_cnt + esco_stru->iso_packet_cnt);
            }
            uint16_t data = BT_APP_COMMON_LINK_QUALITY_BT_CRC | rate << 8;
            app_race_send_notify_to_hid(APPS_RACE_CMD_CONFIG_TYPE_LINK_QUALITY, (int8_t *)&data, sizeof(uint16_t));
            break;
        }
#endif
        default:
            APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", Not supported event id = %d", 1, event_id);
            break;
    }

    return ret;
}

static bool homescreen_app_audio_anc_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;

    switch (event_id) {
#ifdef MTK_ANC_HOWLING_TURN_OFF_ANC
        case ANC_CONTROL_EVENT_HOWLING:
            app_home_screen_disable_anc_when_howling(self);
            break;
#endif
        default:
            break;
    }
    return ret;
}

#if defined (AIR_PURE_GAMING_ENABLE)
extern bool g_usb_config_done;

static void app_pure_gaming_usb_resume_post_process(void)
{
    app_dongle_ull_le_hid_stop_create_cis_timer();

    const app_bt_state_service_status_t *bt_service = app_bt_connection_service_get_current_status();
    if(bt_service->connection_state > APP_BT_CONNECTION_SERVICE_BT_STATE_BT_OFF){
        app_dongle_ull_le_hid_start_reconnect();
    } else {
        app_bt_state_service_set_bt_on_off(true, false, true, false);
    }
}

static void app_pure_gaming_set_tx_gc_value(uint32_t value)
{
    extern bt_status_t bt_avm_vendor_set_tx_gc_value(uint8_t value);
    bt_avm_vendor_set_tx_gc_value(value);
}
#endif

static bool homescreen_app_usb_audio_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bool ret = false;

    switch (event_id) {
#if defined(AIR_USB_ENABLE) && !defined(MTK_BATTERY_MANAGEMENT_ENABLE)
        case APPS_EVENTS_USB_AUDIO_SUSPEND:
#if defined (AIR_PURE_GAMING_ENABLE)
            if (g_usb_config_done) {
                // pure gaming for mouse, set tx gc to 41
                #define LOW_TX_GC_VALUE (0x29)
                app_pure_gaming_set_tx_gc_value(LOW_TX_GC_VALUE);
                app_dongle_ull_le_hid_disconnect_all_device(BT_HCI_STATUS_UNSPECIFIED_ERROR);
            }
#else
            app_bt_state_service_set_bt_on_off(false, false, true, false);
#endif
            break;
        case APPS_EVENTS_USB_AUDIO_RESUME:
#if defined(AIR_BTA_IC_PREMIUM_G2) || defined(AIR_BTA_IC_STEREO_HIGH_G3)
            /**
             * Leave USB suspend.
             * Before BT ON, switch DCXO back to normal mode.
             */
            APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", dcxo_lp_mode, DCXO_NORMAL_MODE", 0);
            dcxo_lp_mode(DCXO_NORMAL_MODE);
#endif

#if defined (AIR_PURE_GAMING_ENABLE)
            if (g_usb_config_done) {
                // pure gaming for mouse, restore tx gc to default value
                extern uint8_t bt_ull_le_hid_dm_get_default_tx_gc();
                uint8_t default_tx_gc_value = bt_ull_le_hid_dm_get_default_tx_gc();
                app_pure_gaming_set_tx_gc_value(default_tx_gc_value);
                app_pure_gaming_usb_resume_post_process();
            }
#else
            if (BT_POWER_ON_RELAY != bt_power_on_get_config_type()) {
                app_bt_state_service_set_bt_on_off(true, false, true, false);
            }
#endif
            break;
        case APPS_EVENTS_USB_CONFIG_DONE: {
            APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", USB configure done, Turn BT on", 0);
#ifdef AIR_PURE_GAMING_ENABLE
            //Do nothing;
#else
            if (BT_POWER_ON_RELAY != bt_power_on_get_config_type()) {
                app_bt_state_service_set_bt_on_off(true, false, true, false);
            }
#endif
        }
        break;
#endif
    }
    return ret;
}

bool app_home_screen_idle_activity_proc(ui_shell_activity_t *self,
                                        uint32_t event_group,
                                        uint32_t event_id,
                                        void *extra_data,
                                        size_t data_len)
{
    bool ret = false;

    switch (event_group) {
        /* UI Shell internal events. */
        case EVENT_GROUP_UI_SHELL_SYSTEM: {
            ret = _proc_ui_shell_group(self, event_id, extra_data, data_len);
            break;
        }
        /* UI Shell key events. */
        case EVENT_GROUP_UI_SHELL_KEY: {
            ret = _proc_key_event_group(self, event_id, extra_data, data_len);
            break;
        }
        case EVENT_GROUP_UI_SHELL_BT: {
            homescreen_app_bt_event_proc(self, event_id, extra_data, data_len);
            break;
        }
        /* UI Shell BT Connection Manager events. */
        case EVENT_GROUP_UI_SHELL_BT_CONN_MANAGER: {
            ret = homescreen_app_bt_connection_manager_event_proc(self, event_id, extra_data, data_len);
            break;
        }
        /* UI Shell BT device Manager events. */
        case EVENT_GROUP_UI_SHELL_BT_DEVICE_MANAGER: {
            ret = homescreen_app_bt_dm_event_proc(self, event_id, extra_data, data_len);
            break;
        }
#ifdef AIR_BLE_AUDIO_DONGLE_ENABLE
        case EVENT_GROUP_UI_SHELL_LE_AUDIO: {
            ret = homescreen_app_le_audio_manager_event_proc(self, event_id, extra_data, data_len);
            break;
        }
#endif
#if defined AIR_BT_ULTRA_LOW_LATENCY_ENABLE || defined AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
        /* Ultra low latency events. */
        case EVENT_GROUP_BT_ULTRA_LOW_LATENCY:
            ret = homescreen_app_ull_event_proc(self, event_id, extra_data, data_len);
            break;
#endif
        /* UI Shell APP_INTERACTION events. */
        case EVENT_GROUP_UI_SHELL_APP_INTERACTION:
            ret = _app_interaction_event_proc(self, event_id, extra_data, data_len);
            break;
        /* UI Shell AUDIO ANC events. */
        case EVENT_GROUP_UI_SHELL_AUDIO_ANC:
            ret = homescreen_app_audio_anc_event_proc(self, event_id, extra_data, data_len);
            break;
        case EVENT_GROUP_UI_SHELL_USB_AUDIO:
            ret = homescreen_app_usb_audio_event_proc(self, event_id, extra_data, data_len);
            break;
        default:
            break;
    }
    return ret;
}

bool app_home_screen_idle_activity_is_aws_connected(void)
{
    return s_app_homescreen_context.aws_connected;
}

#ifdef AIR_BT_ULTRA_LOW_LATENCY_ENABLE
bool bt_cm_check_connect_request(bt_bd_addr_ptr_t address, uint32_t cod)
{
    bool allow_connect = true;
    const bt_bd_addr_t zero_addr = {0};
    if (0 != memcmp(s_app_homescreen_context.ull_peer_addr, zero_addr, sizeof(bt_bd_addr_t))
        && 0 != memcmp(s_app_homescreen_context.ull_peer_addr, address, sizeof(bt_bd_addr_t))) {
        allow_connect = false;
        APPS_LOG_MSGID_I(UI_SHELL_IDLE_BT_CONN_ACTIVITY", bt_cm_check_connect_request, reject not peer address", 0);
    }
    return allow_connect;
}

#endif

