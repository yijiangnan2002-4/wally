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

#include "apps_init.h"

#ifdef MTK_ANC_ENABLE
#include "app_anc_service.h"
#endif
#include "apps_events_audio_event.h"
#include "apps_events_key_event.h"
#include "apps_events_battery_event.h"
#include "apps_events_bt_event.h"
#ifdef AIR_USB_ENABLE
#include "apps_events_usb_event.h"
#endif
#include "apps_race_cmd_event.h"
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DCHS_MODE_ENABLE)
#include "apps_race_cmd_co_sys_event.h"
#endif
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DCHS_MODE_MASTER_ENABLE)
#include "app_master_idle_activity.h"
#endif

#if defined(MTK_BATTERY_MANAGEMENT_ENABLE) && !(defined (AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE) || defined(AIR_DCHS_MODE_SLAVE_ENABLE))
#include "app_battery_idle_activity.h"
#endif
#include "app_home_screen_idle_activity.h"
#ifdef SUPPORT_ROLE_HANDOVER_SERVICE
#include "app_rho_idle_activity.h"
#endif
#include "ui_shell_manager.h"
#include "app_hfp_idle_activity.h"
#include "app_music_idle_activity.h"
#include "apps_config_features_dynamic_setting.h"
#include "apps_config_led_manager.h"
#ifdef MTK_FOTA_ENABLE
#include "app_fota_idle_activity.h"
#endif
#ifdef APPS_SLEEP_AFTER_NO_CONNECTION
#include "app_power_save_idle_activity.h"
#endif
#include "app_preproc_activity.h"
#ifdef AIR_APP_MULTI_VA
#include "app_multi_va_idle_activity.h"
#include "multi_va_manager.h"
#endif
#ifdef RACE_FIND_ME_ENABLE
#include "app_fm_idle_activity.h"
#endif
#include "bt_app_common.h"
#ifdef AIR_BT_FAST_PAIR_ENABLE
#include "app_fast_pair.h"
#endif
#include "app_bt_conn_componet_in_homescreen.h"
#include "apps_debug.h"
#include "nvkey.h"
#include "nvkey_id_list.h"
#ifdef AIR_LED_ENABLE
#include "app_led_control.h"
#endif
#ifdef AIR_PROMPT_SOUND_ENABLE
#include "voice_prompt_internal.h"
#endif
#ifdef AIR_AMA_ENABLE
#include "app_ama_audio.h"
#include "app_ama_idle_activity.h"
#include "app_ama_multi_va.h"

#endif
#include "apps_config_key_remapper.h"
#include "apps_state_report.h"

#ifdef AIR_GSOUND_ENABLE
#include "app_gsound_idle_activity.h"
#include "app_gsound_multi_va.h"
#endif

#ifdef AIR_SMART_CHARGER_ENABLE
#include "app_smcharger_idle_activity.h"
#endif
#include "bt_customer_config.h"
#ifdef MTK_IN_EAR_FEATURE_ENABLE
#include "app_in_ear_idle_activity.h"
#endif
#ifdef AIR_MULTI_POINT_ENABLE
#include "app_bt_emp_service.h"
#endif

#if defined(AIR_SMART_CHARGER_ENABLE)
extern void DrvCharger_SmartCase_Init(void);
#endif

#ifdef AIR_XIAOWEI_ENABLE
#include "app_va_xiaowei_multi_support.h"
#include "app_va_xiaowei_activity.h"
#endif
#ifdef AIR_XIAOAI_ENABLE
#include "app_va_xiaoai_activity.h"
#include "app_va_xiaoai_ble_adv.h"
#endif

#ifdef MTK_LEAKAGE_DETECTION_ENABLE
#include "app_seal_check_idle_activity.h"
#endif

#ifdef APPS_LINE_IN_SUPPORT
#include "app_line_in_idle_activity.h"
#endif

#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
#include "app_ull_idle_activity.h"
#endif

#ifdef AIR_MCSYNC_SHARE_ENABLE
#include "app_share_idle_activity.h"
#endif
#ifdef APPS_USB_AUDIO_SUPPORT
#include "app_usb_audio_idle_activity.h"
#endif

#if defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
#include "app_lea_service.h"
#endif
#include "app_bt_conn_manager.h"

#ifdef AIR_TILE_ENABLE
#include "app_tile.h"
#include "apps_events_test_event.h"
#endif

#include "apps_control_touch_key_status.h"
#include "app_bt_takeover_service.h"

#ifdef AIR_MS_TEAMS_ENABLE
#include "app_ms_teams_idle_activity.h"
#endif


#include "apps_dongle_sync_event.h"

#ifdef AIR_CFU_ENABLE
#include "app_ms_cfu_idle_activity.h"
#endif

#ifdef AIR_MS_GIP_ENABLE
#include "app_ms_xbox_idle_activity.h"
#endif

#ifdef AIR_SWIFT_PAIR_ENABLE
#ifdef AIR_CUST_PAIR_ENABLE
#include "app_cust_pair_idle_activity.h"
#else
#include "app_swift_pair_idle_activity.h"
#endif
#endif

#if defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
#include "app_dongle_service.h"
#endif /* AIR_BT_ULTRA_LOW_LATENCY_ENABLE */

#ifdef MTK_ANC_ENABLE
#include "app_adaptive_anc_idle_activity.h"
#endif

#ifdef AIR_USB_HID_CALL_CTRL_ENABLE
#include "app_hid_call_idle_activity.h"
#endif

#ifdef AIR_ADAPTIVE_EQ_ENABLE
#include "app_adaptive_eq_idle_activity.h"
#endif

#ifdef AIR_ADVANCED_PASSTHROUGH_ENABLE_V2
#include "app_self_fitting_idle_activity.h"
#endif

#ifdef AIR_WIRELESS_MIC_ENABLE
#include "app_wireless_mic_idle_activity.h"
#endif

#ifdef AIR_SPOTIFY_TAP_ENABLE
#include "app_spotify_tap_activity.h"
#endif /* AIR_SPOTIFY_TAP_ENABLE */

#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
#include "app_hearing_aid_activity.h"
#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */

#ifdef AIR_HEARTHROUGH_MAIN_ENABLE
#include "app_hear_through_activity.h"
#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */

#ifdef AIR_APP_AUDIO_TRANS_MGR_ENABLE
#include "app_audio_trans_mgr_idle_activity.h"
#endif

#ifdef AIR_BT_HFP_ENABLE
#include "app_rx_eq.h"
#endif

#ifdef AIR_HEAD_TRACKER_ENABLE
#include "app_head_tracker_idle_activity.h"
#endif

#ifdef AIR_SPEAKER_ENABLE
#include "app_speaker_idle_activity.h"
#include "app_speaker_le_association.h"
#endif

// richard for customer UI spec.
#include "app_psensor_px31bf_activity.h"
#include "app_hall_sensor_activity.h"
#include "app_customer_common_activity.h"

static void apps_init_events_senders(void)
{
    apps_event_key_event_init();
#ifdef MTK_BATTERY_MANAGEMENT_ENABLE
    apps_events_battery_event_init();
#endif
    apps_events_bt_event_init();
#ifdef CCASE_ENABLE
    apps_events_charger_case_event_init();
#endif
#ifdef MTK_RACE_CMD_ENABLE
    bt_race_app_event_init();
    apps_race_cmd_event_init();
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DCHS_MODE_ENABLE)
    apps_race_cmd_co_sys_event_init();
#endif
    apps_dongle_sync_init();
#endif
#ifdef MTK_AWS_MCE_ENABLE
    app_aws_report_event_init();
#endif
#ifdef AIR_TILE_ENABLE
    apps_events_test_event_init();
#endif
    apps_events_audio_event_init();
#ifdef AIR_USB_ENABLE
    apps_event_usb_event_init();
#endif
}

static void apps_configs_module_init(void)
{
    apps_config_led_manager_init();
}

static void apps_init_applications(void)
{
    ui_shell_set_pre_proc_func(app_preproc_activity_proc);
    ui_shell_start_activity(NULL, app_event_state_report_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#ifdef AIR_APP_MULTI_VA
    ui_shell_start_activity(NULL, app_multi_va_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
    ui_shell_start_activity(NULL, app_ull_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
    /* Register all idle activities here. The last registered activity is the foreground activity. */
#if defined(AIR_SMART_CHARGER_ENABLE)
    DrvCharger_SmartCase_Init();
    ui_shell_start_activity(NULL, app_smcharger_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#elif defined(MTK_BATTERY_MANAGEMENT_ENABLE) && !(defined (AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE) || defined(AIR_DCHS_MODE_SLAVE_ENABLE))
    ui_shell_start_activity(NULL, app_battery_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
#ifdef SUPPORT_ROLE_HANDOVER_SERVICE
    ui_shell_start_activity(NULL, app_rho_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
#ifdef MTK_FOTA_ENABLE
    ui_shell_start_activity(NULL, app_fota_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
#ifdef CCASE_ENABLE
    ui_shell_start_activity(NULL, app_charger_case_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
#ifdef RACE_FIND_ME_ENABLE
    ui_shell_start_activity(NULL, app_find_me_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
#ifdef AIR_GSOUND_ENABLE
    ui_shell_start_activity(NULL, app_gsound_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
#ifdef AIR_BT_FAST_PAIR_ENABLE
    ui_shell_start_activity(NULL, app_fast_pair_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
    ui_shell_start_activity(NULL, app_music_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#ifdef APPS_SLEEP_AFTER_NO_CONNECTION
    ui_shell_start_activity(NULL, app_power_save_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
#ifdef MTK_IN_EAR_FEATURE_ENABLE
    ui_shell_start_activity(NULL, app_in_ear_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
	// richard for customer UI spec.
    ui_shell_start_activity(NULL, apps_psensor_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
    ui_shell_start_activity(NULL, apps_hall_sensor_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
    ui_shell_start_activity(NULL, apps_customer_common_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#ifdef AIR_BT_HFP_ENABLE
    ui_shell_start_activity(NULL, app_hfp_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
    ui_shell_start_activity(NULL, app_home_screen_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_TOP, NULL, 0);
    /* APPS_USB_AUDIO_SUPPORT for headset project */
#ifdef APPS_USB_AUDIO_SUPPORT
    ui_shell_start_activity(NULL, app_usb_audio_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_TOP, NULL, 0);
#endif
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DCHS_MODE_MASTER_ENABLE)
    ui_shell_start_activity(NULL, app_master_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_TOP, NULL, 0);
#endif
#ifdef AIR_AMA_ENABLE
    ui_shell_start_activity(NULL, app_ama_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
    /* APPS_LINE_IN_SUPPORT for headset project */
#ifdef APPS_LINE_IN_SUPPORT
    ui_shell_start_activity(NULL, app_line_in_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
#ifdef AIR_XIAOWEI_ENABLE
    ui_shell_start_activity(NULL, app_va_xiaowei_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
#ifdef AIR_XIAOAI_ENABLE
    ui_shell_start_activity(NULL, app_va_xiaoai_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
#ifdef MTK_LEAKAGE_DETECTION_ENABLE
    ui_shell_start_activity(NULL, app_leakage_detection_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
#ifdef AIR_MULTI_POINT_ENABLE
    ui_shell_start_activity(NULL, app_bt_emp_service_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif

#ifdef MTK_ANC_ENABLE
    ui_shell_start_activity(NULL, app_adaptive_anc_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif

#ifdef AIR_MCSYNC_SHARE_ENABLE
    ui_shell_start_activity(NULL, app_share_idle_activity, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
#if defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
    ui_shell_start_activity(NULL, app_lea_service_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
    ui_shell_start_activity(NULL, app_bt_conn_manager_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);

#ifdef AIR_TILE_ENABLE
    ui_shell_start_activity(NULL, app_tile_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
#ifdef AIR_MS_TEAMS_ENABLE
    ui_shell_start_activity(NULL, app_ms_teams_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_TOP, NULL, 0);
#endif
#ifdef AIR_CFU_ENABLE
    ui_shell_start_activity(NULL, app_ms_cfu_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
#if defined(AIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE)
    ui_shell_start_activity(NULL, app_dongle_service_activity_proc, ACTIVITY_PRIORITY_IDLE_TOP, NULL, 0);
#endif
#ifdef AIR_MS_GIP_ENABLE
    ui_shell_start_activity(NULL, app_ms_xbox_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
#ifdef AIR_SWIFT_PAIR_ENABLE
#ifdef AIR_CUST_PAIR_ENABLE
    ui_shell_start_activity(NULL, app_cust_pair_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#else
    ui_shell_start_activity(NULL, app_swift_pair_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
#endif
#ifdef AIR_USB_HID_CALL_CTRL_ENABLE
    ui_shell_start_activity(NULL, app_hid_call_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_TOP, NULL, 0);
#endif
#ifdef AIR_ADAPTIVE_EQ_ENABLE
    ui_shell_start_activity(NULL, app_adaptive_eq_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
#ifdef AIR_ADVANCED_PASSTHROUGH_ENABLE_V2
    ui_shell_start_activity(NULL, app_self_fitting_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
#ifdef AIR_WIRELESS_MIC_ENABLE
    ui_shell_start_activity(NULL, app_wireless_mic_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
#ifdef AIR_SPOTIFY_TAP_ENABLE
    ui_shell_start_activity(NULL, app_spotify_tap_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif /* AIR_SPOTIFY_TAP_ENABLE */
#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
    ui_shell_start_activity(NULL, app_hearing_aid_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */
#ifdef AIR_HEARTHROUGH_MAIN_ENABLE
    ui_shell_start_activity(NULL, app_hear_through_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */
#ifdef AIR_APP_AUDIO_TRANS_MGR_ENABLE
    ui_shell_start_activity(NULL, app_audio_trans_mgr_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_TOP, NULL, 0);
#endif
#ifdef AIR_BT_HFP_ENABLE
    ui_shell_start_activity(NULL, app_rx_eq_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
#ifdef AIR_HEAD_TRACKER_ENABLE
    ui_shell_start_activity(NULL, app_head_tracker_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_TOP, NULL, 0);
#endif
#ifdef AIR_SPEAKER_ENABLE
    ui_shell_start_activity(NULL, app_speaker_idle_activity_proc, ACTIVITY_PRIORITY_IDLE_BACKGROUND, NULL, 0);
#endif
}

static void apps_init_multi_va(void)
{
#ifdef AIR_APP_MULTI_VA

#ifdef AIR_AMA_ENABLE
    apps_register_ama_in_multi_voice();
#endif
#ifdef AIR_GSOUND_ENABLE
    app_gsound_multi_va_register();
#endif
#ifdef AIR_XIAOWEI_ENABLE
    app_va_xiaowei_multi_support_register();
#endif
#ifdef MTK_VA_XIAOAI_MULTI_VA_ENABLE
    app_va_xiaoai_multi_support_register();
#endif

    multi_va_manager_start();
#endif
}

void apps_default_init_customer_applications() {}
#pragma weak apps_init_customer_applications = apps_default_init_customer_applications

void apps_init(void)
{
    /* init LED module. */
#ifdef LED_ENABLE
    app_led_control_init();
#endif
    /* update feature setting. */
    apps_config_features_dynamic_setting_init();
    /* init APP event sender, such as key event, battery event, bt event etc. */
    apps_init_events_senders();
    /* init APP config module, only led manager. */
    apps_configs_module_init();
    /* init speaker association. */
#ifdef AIR_SPEAKER_ENABLE
    app_speaker_le_ass_init();
#endif
    /* All VA register to multi VA manager and start. */
    apps_init_multi_va();
    /* Init BT takeover service. */
    app_bt_takeover_service_init();
    /* Init all APP activities. */
    apps_init_applications();
    /* Init customer activities. */
    apps_init_customer_applications();
    /* Init Key remaper table. */
    apps_config_key_remaper_init_configurable_table();
    /* Init touch key status. */
    apps_init_touch_key_status();
#ifdef AIR_PROMPT_SOUND_ENABLE
    voice_prompt_init(NULL);
#endif
    /* Start UI shell. */
    ui_shell_start();
}
