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
 * File: apps_init.c
 *
 * Description: This file is used to init APP modules.
 *
 */

#ifdef MTK_ANC_ENABLE
#include "app_anc_service.h"
#endif
#include "apps_events_key_event.h"
#include "apps_events_battery_event.h"
#include "apps_events_bt_event.h"
#ifdef AIR_USB_ENABLE
#include "apps_events_usb_event.h"
#endif
#include "apps_events_i2s_in_event.h"
#include "apps_race_cmd_event.h"

#if defined(MTK_BATTERY_MANAGEMENT_ENABLE)
#include "app_battery_idle_activity.h"
#endif
#include "app_home_screen_idle_activity.h"
#include "app_ull_dongle_idle_activity.h"
#include "app_dongle_common_idle_activity.h"
#include "ui_shell_manager.h"
#include "apps_config_features_dynamic_setting.h"
#include "apps_config_led_manager.h"
#ifdef MTK_FOTA_ENABLE
#include "app_fota_idle_activity.h"
#endif
#ifdef APPS_SLEEP_AFTER_NO_CONNECTION
#include "app_power_save_idle_activity.h"
#endif
#include "app_preproc_activity.h"

#ifdef AIR_MS_GIP_ENABLE
#include "app_ms_xbox_idle_activity.h"
#endif

#include "bt_app_common.h"
#include "app_bt_conn_componet_in_homescreen.h"
#include "apps_debug.h"
#include "nvkey.h"
#include "nvkey_id_list.h"
#include "app_led_control.h"
#include "apps_config_key_remapper.h"
#include "apps_state_report.h"

#include "bt_customer_config.h"

#ifdef AIR_MS_TEAMS_ENABLE
#include "app_ms_teams_idle_activity.h"
#endif

#ifdef APPS_LINE_IN_SUPPORT
#include "app_line_in_idle_activity.h"
#endif

#ifdef APPS_USB_AUDIO_SUPPORT
#include "app_usb_audio_idle_activity.h"
#endif

#ifdef AIR_LE_AUDIO_ENABLE
#include "app_le_audio.h"
#endif
#ifdef AIR_BT_SOURCE_ENABLE
#include "app_bt_source.h"
#endif

#ifdef AIR_CFU_ENABLE
#include "app_ms_cfu_idle_activity.h"
#endif

#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
#include "app_dongle_service.h"
#endif /* AIR_BT_ULTRA_LOW_LATENCY_ENABLE */

#include "apps_dongle_sync_event.h"
#include "usb_main.h"

#ifdef AIR_APP_AUDIO_TRANS_MGR_ENABLE
#include "app_audio_trans_mgr_idle_activity.h"
#endif

static void apps_init_events_senders(void)
{
    apps_event_key_event_init();
#ifdef MTK_BATTERY_MANAGEMENT_ENABLE
    apps_events_battery_event_init();
#endif
    apps_events_bt_event_init();
#ifdef MTK_RACE_CMD_ENABLE
    bt_race_app_event_init();
    apps_race_cmd_event_init();
    apps_dongle_sync_init();
#endif
#ifdef MTK_AWS_MCE_ENABLE
    app_aws_report_event_init();
#endif
#ifdef MTK_ANC_ENABLE
    app_anc_service_init();
#endif

#ifdef AIR_USB_ENABLE
    apps_event_usb_event_init();
#endif
    app_events_i2s_in_init();
}

static void apps_configs_module_init(void)
{
    apps_config_led_manager_init();
}

#ifdef AIR_MS_GIP_ENABLE
#if defined(AIR_USB_AUDIO_ENABLE)

#include "usbaudio_drv.h"
static void apps_usb_audio_set_interface_empty_callback_handler(uint8_t interface_number, uint8_t alternate_set)
{

}
#endif
#endif /* AIR_MS_GIP_ENABLE */

static void apps_init_applications(void)
{
    ui_shell_set_pre_proc_func(app_preproc_activity_proc);
    ui_shell_start_activity(NULL, app_event_state_report_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
    /* Register all idle activities here. The last registered activity is the foreground activity. */
#if defined(MTK_BATTERY_MANAGEMENT_ENABLE)
    ui_shell_start_activity(NULL, app_battery_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
#ifdef MTK_FOTA_ENABLE
    ui_shell_start_activity(NULL, app_fota_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
    ui_shell_start_activity(NULL, app_ull_dongle_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
#ifdef APPS_SLEEP_AFTER_NO_CONNECTION
    ui_shell_start_activity(NULL, app_power_save_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
#ifdef APPS_LINE_IN_SUPPORT
    ui_shell_start_activity(NULL, app_line_in_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
#ifdef AIR_LE_AUDIO_ENABLE
    ui_shell_start_activity(NULL, app_le_audio_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
#ifdef AIR_BT_SOURCE_ENABLE
    ui_shell_start_activity(NULL, app_bt_source_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
    ui_shell_start_activity(NULL, app_home_screen_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_TOP, NULL, 0);
#ifdef AIR_MS_TEAMS_ENABLE
    ui_shell_start_activity(NULL, app_ms_teams_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_TOP, NULL, 0);
#endif
#ifdef AIR_CFU_ENABLE
    ui_shell_start_activity(NULL, app_ms_cfu_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif

#ifdef AIR_MS_GIP_ENABLE
    if (Get_USB_Host_Type() == USB_HOST_TYPE_XBOX) {
        ui_shell_start_activity(NULL, app_ms_xbox_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);

        /**
         * If current is XBOX mode, then plug into the PC to check functionality,
         * Register empty callback to avoid crash.
         */
#if defined(AIR_USB_AUDIO_ENABLE)
        USB_Audio_Register_SetInterface_Callback(0, apps_usb_audio_set_interface_empty_callback_handler);
#ifdef AIR_USB_AUDIO_2_SPK_ENABLE
        USB_Audio_Register_SetInterface_Callback(1, apps_usb_audio_set_interface_empty_callback_handler);
#endif /* AIR_USB_AUDIO_2_SPK_ENABLE */
#endif
    }
#endif/* AIR_MS_GIP_ENABLE */
    ui_shell_start_activity(NULL, app_dongle_common_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
    ui_shell_start_activity(NULL, app_dongle_service_activity_proc, ACTIVITY_PRIORITY_IDLE_TOP, NULL, 0);
#endif /* AIR_BT_ULTRA_LOW_LATENCY_ENABLE */
#ifdef APPS_USB_AUDIO_SUPPORT
    ui_shell_start_activity(NULL, app_usb_audio_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
#ifdef AIR_APP_AUDIO_TRANS_MGR_ENABLE
    ui_shell_start_activity(NULL, app_audio_trans_mgr_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
}

#if 0
static void apps_init_multi_va(void)
{
#ifdef MTK_AMA_ENABLE
    apps_register_ama_in_multi_voice();
#endif
#ifdef GSOUND_LIBRARY_ENABLE
    app_gsound_multi_va_register();
#endif
#ifdef MTK_VA_XIAOWEI_ENABLE
    app_va_xiaowei_multi_support_register();
#endif
#ifdef MTK_VA_XIAOAI_MULTI_VA_ENABLE
    app_va_xiaoai_multi_support_register();
#endif

    multi_va_manager_start();
}
#endif

void apps_init(void)
{
    /* init LED module. */
#if LED_ENABLE
    app_led_control_init();
#endif
    /* update feature setting. */
    apps_config_features_dynamic_setting_init();
    /* init APP event sender, such as key event, battery event, bt event etc. */
    apps_init_events_senders();
    /* init APP config module, only led manager. */
    apps_configs_module_init();
    /* All VA register to multi VA manager and start. */
    //apps_init_multi_va();
    /* Init all APP activities. */
    apps_init_applications();
    /* Init Key remaper table. */
    apps_config_key_remaper_init_configurable_table();
    /* Start UI shell. */
    ui_shell_start();
}

void apps_pre_init(void)
{
#ifdef AIR_MS_GIP_ENABLE
    dongle_switch_det_init();
#endif /* AIR_MS_GIP_ENABLE */
    app_dongle_common_idle_activity_init_mode();
}
