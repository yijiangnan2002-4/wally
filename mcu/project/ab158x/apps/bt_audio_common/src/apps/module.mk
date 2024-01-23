# Copyright Statement:
#
# (C) 2017  Airoha Technology Corp. All rights reserved.
#
# This software/firmware and related documentation ("Airoha Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or its licensors.
# Without the prior written permission of Airoha and/or its licensors,
# any reproduction, modification, use or disclosure of Airoha Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
# You may only use, reproduce, modify, or distribute (as applicable) Airoha Software
# if you have agreed to and been bound by the applicable license agreement with
# Airoha ("License Agreement") and been granted explicit permission to do so within
# the License Agreement ("Permitted User").  If you are not a Permitted User,
# please cease any access or use of Airoha Software immediately.
# BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# THAT AIROHA SOFTWARE RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES
# ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL
# WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
# NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
# SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
# SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
# THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
# THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
# CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL ALSO NOT BE RESPONSIBLE FOR ANY AIROHA
# SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
# STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND
# CUMULATIVE LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,
# AT AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE,
# OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
# AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.
#

APPS_SRC = $(APP_COMMON_PATH)/src/apps
APPS_INC = $(APP_COMMON_PATH)/inc/apps

C_FILES += $(APPS_SRC)/apps_init.c
# Apps C files
ifeq ($(MTK_FOTA_ENABLE),y)
C_FILES += $(APPS_SRC)/app_fota/app_fota_idle_activity.c
endif
ifeq ($(AIR_SPEAKER_ENABLE),y)
C_FILES += $(APPS_SRC)/app_speaker/app_speaker_idle_activity.c
C_FILES += $(APPS_SRC)/app_speaker/app_speaker_le_association.c
endif

# Pre-proc activity
C_FILES += $(APPS_SRC)/app_preproc/app_preproc_activity.c
C_FILES += $(APPS_SRC)/app_preproc/app_preproc_sys_pwr.c

ifneq ($(AIR_DUAL_CHIP_MIXING_MODE)_$(AIR_DUAL_CHIP_AUDIO_INTERFACE),slave_i2s)
ifeq ($(MTK_BATTERY_MANAGEMENT_ENABLE),y)
# Battery app
C_FILES += $(APPS_SRC)/app_battery/app_battery_idle_activity.c
C_FILES += $(APPS_SRC)/app_battery/app_battery_transient_activity.c
endif
endif

# BT state service
C_FILES += $(APPS_SRC)/app_bt_state/app_bt_state_service.c

# app service
ifeq ($(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE), y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_dongle_service
C_FILES += $(APPS_SRC)/app_dongle_service/app_dongle_service.c
endif
ifeq ($(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE), y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_dongle_service
C_FILES += $(APPS_SRC)/app_dongle_service/app_dongle_service.c
endif

# BT takeover service
C_FILES += $(APPS_SRC)/app_bt_state/app_bt_takeover_service.c

ifeq ($(AIR_APPS_POWER_SAVE_ENABLE),y)
# Power saving
C_FILES += $(APPS_SRC)/app_power_save/app_power_save_idle_activity.c
C_FILES += $(APPS_SRC)/app_power_save/app_power_save_timeout_activity.c
C_FILES += $(APPS_SRC)/app_power_save/app_power_save_waiting_activity.c
C_FILES += $(APPS_SRC)/app_power_save/app_power_save_utils.c
endif

ifeq ($(MTK_BLE_GAP_SRV_ENABLE),y)
# multi va
C_FILES += $(APPS_SRC)/app_multi_va/app_multi_va_idle_activity.c
C_FILES += $(APPS_SRC)/app_multi_va/multi_va_manager.c
C_FILES += $(APPS_SRC)/app_multi_va/multi_ble_adv_manager.c
CFLAGS  += -DAIR_APP_MULTI_VA
endif

ifeq ($(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE),y)
CFLAGS += -DAIR_BLE_ULL_TAKEOVER_ENABLE
CFLAGS += -DAIR_BLE_ULL_PARING_MODE_ENABLE
ifeq ($(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE),y)
CFLAGS += -DAIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
endif
C_FILES += $(APPS_SRC)/app_ull/app_ull_takeover.c
C_FILES += $(APPS_SRC)/app_ull/app_ull_idle_activity.c
endif

ifeq ($(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE),y)
CFLAGS += -DAIR_BLE_ULL_TAKEOVER_ENABLE
CFLAGS += -DAIR_BLE_ULL_PARING_MODE_ENABLE
#CFLAGS += -DAIR_BLE_ULL_KEEP_APP_BOND_INFO_ENABLE
CFLAGS += -DAIR_BLE_ULL_REMOVE_OLD_RECORD_ENABLE
ifeq ($(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE),y)
CFLAGS += -DAIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
endif
C_FILES += $(APPS_SRC)/app_ull/app_ull_takeover.c
C_FILES += $(APPS_SRC)/app_ull/app_ull_idle_activity.c
endif

ifeq ($(AIR_TILE_ENABLE),y)
#tile
C_FILES += $(APPS_SRC)/app_tile/app_tile.c
endif

# Some utils app and home screen app
ifeq ($(AIR_BT_ROLE_HANDOVER_SERVICE_ENABLE),y)
C_FILES += $(APPS_SRC)/app_idle/app_rho_idle_activity.c
endif
C_FILES += $(APPS_SRC)/app_idle/app_home_screen_idle_activity.c
C_FILES += $(APPS_SRC)/app_idle/app_bt_conn_componet_in_homescreen.c
ifeq ($(AIR_ANC_ENABLE), y)
C_FILES += $(APPS_SRC)/app_idle/app_anc_service.c
CFLAGS += -DAIR_APP_ANC_SYNC_ENABLE
endif
ifeq ($(AIR_MULTI_POINT_ENABLE), y)
C_FILES += $(APPS_SRC)/app_idle/app_bt_emp_service.c
endif
C_FILES += $(APPS_SRC)/app_idle/app_bt_conn_manager.c

# BT Takeover & Power on Reconnect
ifeq ($(AIR_BT_TAKEOVER_ENABLE),y)
CFLAGS += -DAIR_BT_TAKEOVER_ENABLE

CFLAGS += -DAPP_CONN_MGR_RECONNECT_CONTROL
#CFLAGS += -DAPP_CONN_MGR_RECONNECT_ONLY_ONE
endif

ifeq ($(AIR_SWIFT_PAIR_ENABLE),y)
CFLAGS += -DAIR_SWIFT_PAIR_ENABLE
ifeq ($(AIR_BT_INTEL_EVO_ENABLE),y)
CFLAGS += -DAPP_SWIFT_PAIR_SHOW_TWS_ICON
CFLAGS += -DAPP_SWIFT_PAIR_LE_EDR_SECURE_MODE
else
ifeq ($(AIR_MS_TEAMS_ENABLE),y)
CFLAGS += -DAPP_SWIFT_PAIR_LE_EDR_SECURE_MODE
endif
endif
C_FILES += $(APPS_SRC)/app_swift_pair/app_swift_pair_idle_activity.c
ifeq ($(AIR_CUST_PAIR_ENABLE),y)
C_FILES += $(APPS_SRC)/app_swift_pair/app_cust_pair_idle_activity.c
endif
endif

#in-ear part
ifeq ($(AIR_WEARING_DETECTION_ENABLE), y)
C_FILES += $(APPS_SRC)/app_in_ear/app_in_ear_idle_activity.c
C_FILES += $(APPS_SRC)/app_in_ear/app_in_ear_utils.c
endif

#music part
ifeq ($(MTK_BT_A2DP_ENABLE), y)
C_FILES += $(APPS_SRC)/app_music/app_music_idle_activity.c
C_FILES += $(APPS_SRC)/app_music/app_music_activity.c
C_FILES += $(APPS_SRC)/app_music/app_music_utils.c
endif

#hfp part
ifeq ($(AIR_BT_HFP_ENABLE), y)
CFLAGS += -DAIR_BT_HFP_ENABLE
C_FILES += $(APPS_SRC)/app_hfp/app_hfp_idle_activity.c
C_FILES += $(APPS_SRC)/app_hfp/app_hfp_utils.c
C_FILES += $(APPS_SRC)/app_hfp/app_hfp_activity.c
C_FILES += $(APPS_SRC)/app_hfp/app_hfp_va_activity.c
endif

# FIND ME
ifeq ($(AIR_RACE_FIND_ME_ENABLE), y)
C_FILES += $(APPS_SRC)/app_fm/app_fm_idle_activity.c
C_FILES += $(APPS_SRC)/app_fm/app_fm_activity.c
endif

# App state report
C_FILES += $(APPS_SRC)/app_state_report/apps_state_report.c

ifeq ($(AIR_DUAL_CHIP_MIXING_MODE),master)
C_FILES += $(APPS_SRC)/app_master/app_master_idle_activity.c
C_FILES += $(APPS_SRC)/app_master/app_master_transient_activity.c
C_FILES += $(APPS_SRC)/app_master/app_master_utils.c
endif

ifneq ($(findstring $(AIR_DUAL_CHIP_MIXING_MODE), master_slave),)
C_FILES += $(APPS_SRC)/events/apps_race_cmd_co_sys_event.c
endif

# Events folder
C_FILES += $(APPS_SRC)/events/apps_events_audio_event.c
C_FILES += $(APPS_SRC)/events/apps_events_key_event.c
ifeq ($(MTK_BATTERY_MANAGEMENT_ENABLE),y)
C_FILES += $(APPS_SRC)/events/apps_events_battery_event.c
endif
C_FILES += $(APPS_SRC)/events/apps_events_bt_event.c
C_FILES += $(APPS_SRC)/events/apps_events_test_event.c
C_FILES += $(APPS_SRC)/events/apps_race_cmd_event.c
ifeq ($(AIR_USB_ENABLE),y)
C_FILES += $(APPS_SRC)/events/apps_events_usb_event.c
endif

# Config folder
C_FILES += $(APPS_SRC)/config/apps_config_audio_helper.c
ifeq ($(AIR_ADVANCED_PASSTHROUGH_ENABLE),y)
C_FILES += $(APPS_SRC)/config/app_advance_passthrough.c
endif
C_FILES += $(APPS_SRC)/config/apps_config_key_remapper.c
C_FILES += $(APPS_SRC)/config/apps_config_led_manager.c
C_FILES += $(APPS_SRC)/config/apps_config_features_dynamic_setting.c

# LED
C_FILES += $(APPS_SRC)/led/app_led_control.c
C_FILES += $(APPS_SRC)/led/led_control_internal.c
C_FILES += $(APPS_SRC)/led/led_control_style_cfg.c

# FAST_PAIR
ifeq ($(AIR_BT_FAST_PAIR_ENABLE),y)
ifeq ($(AIR_SPOT_ENABLE),y)
CFLAGS += -DAIR_SPOT_ENABLE
endif
C_FILES += $(APPS_SRC)/app_fast_pair/app_fast_pair_idle_activity.c
endif

# SMART_CHARGER
ifeq ($(AIR_SMART_CHARGER_ENABLE),y)
C_FILES += $(APPS_SRC)/app_smart_charger/app_smcharger_idle_activity.c
C_FILES += $(APPS_SRC)/app_smart_charger/app_smcharger_lid_close_activity.c
C_FILES += $(APPS_SRC)/app_smart_charger/app_smcharger_lid_open_activity.c
C_FILES += $(APPS_SRC)/app_smart_charger/app_smcharger_off_activity.c
C_FILES += $(APPS_SRC)/app_smart_charger/app_smcharger_out_of_case_activity.c
C_FILES += $(APPS_SRC)/app_smart_charger/app_smcharger_startup_activity.c
C_FILES += $(APPS_SRC)/app_smart_charger/app_smcharger_utils.c
endif

# Utils
C_FILES += $(APPS_SRC)/utils/apps_debug.c
C_FILES += $(APPS_SRC)/utils/apps_atcmd.c
ifeq ($(MTK_AWS_MCE_ENABLE),y)
C_FILES += $(APPS_SRC)/utils/apps_aws_sync_event.c
ifeq ($(MTK_CONN_VP_SYNC_ENABLE),y)
CFLAGS += -D__CONN_VP_SYNC_STYLE_TWO__
endif
endif
C_FILES += $(APPS_SRC)/utils/apps_control_touch_key_status.c
C_FILES += $(APPS_SRC)/utils/apps_dongle_sync_event.c

ifeq ($(AIR_AUDIO_DETACHABLE_MIC_ENABLE), y)
C_FILES += $(APPS_SRC)/utils/apps_detachable_mic.c
CFLAGS  += -DAIR_AUDIO_DETACHABLE_MIC_ENABLE
endif

#voice_prompt
C_FILES +=   $(APPS_SRC)/vp/ui_realtime_task.c
C_FILES +=   $(APPS_SRC)/vp/voice_prompt_main.c
C_FILES +=   $(APPS_SRC)/vp/voice_prompt_api.c
C_FILES +=   $(APPS_SRC)/vp/voice_prompt_local.c
C_FILES +=   $(APPS_SRC)/vp/voice_prompt_queue.c
C_FILES +=   $(APPS_SRC)/vp/voice_prompt_nvdm.c
AIR_VOICE_PROMPT_COSYS_ENABLE = n
ifneq ($(findstring $(AIR_DUAL_CHIP_MIXING_MODE), master_slave),)
AIR_VOICE_PROMPT_COSYS_ENABLE = y
endif
ifeq ($(AIR_VOICE_PROMPT_COSYS_ENABLE), y)
CFLAGS  += -DAIR_VOICE_PROMPT_COSYS_ENABLE
C_FILES +=   $(APPS_SRC)/vp/voice_prompt_cosys.c
endif
ifeq ($(MTK_AWS_MCE_ENABLE),y)
C_FILES +=   $(APPS_SRC)/vp/voice_prompt_aws.c
endif

# AMA
ifeq ($(AIR_AMA_ENABLE),y)
C_FILES +=   $(APPS_SRC)/app_ama/app_ama_idle_activity.c
C_FILES +=   $(APPS_SRC)/app_ama/app_ama_activity.c
C_FILES +=   $(APPS_SRC)/app_ama/app_ama_audio.c
C_FILES +=   $(APPS_SRC)/app_ama/app_ama_multi_va.c
CFLAGS  += -DAIR_AMA_ENABLE

# Add AMA iAP2 support feature option
# When AMA enabled, must enable iAP2 and iAP2 via MUX feature.
ifeq ($(AIR_IAP2_VIA_MUX_ENABLE),y)
CFLAGS  += -DAMA_IAP2_SUPPORT_ENABLE
CFLAGS  += -DAMA_IAP2_VIA_MUX_ENABLE
# Add app relay enable (for Spotify)
ifeq ($(AIR_AMA_IAP2_APP_RELAY_ENABLE),y)
CFLAGS  += -DAMA_IAP2_APP_RELAY_ENABLE
endif
endif

# Enable AMA WWE during calling feature
ifeq ($(AIR_AMA_HOTWORD_DURING_CALL_ENABLE),y)
CFLAGS  += -DAIR_AMA_HOTWORD_DURING_CALL_ENABLE
endif

# Enable AMA ADV before EDR connected feature
ifeq ($(AIR_AMA_ADV_ENABLE_BEFORE_EDR_CONNECT_ENABLE),y)
CFLAGS  += -DAIR_AMA_ADV_ENABLE_BEFORE_EDR_CONNECT_ENABLE
endif

# Configure AMA trigger mode
ifeq ($(findstring TTT, $(AIR_AMA_BUTTON_TRIGGER_MODE)), TTT)
CFLAGS  += -DAMA_TRIGGER_MODE_TTT_ENABLE
endif

ifeq ($(findstring PTT, $(AIR_AMA_BUTTON_TRIGGER_MODE)), PTT)
CFLAGS  += -DAMA_TRIGGER_MODE_PTT_ENABLE
endif

ifeq ($(AIR_AMA_HOTWORD_ENABLE),y)
CFLAGS  += -DAMA_TRIGGER_MODE_WWD_ENABLE
endif

# Enable AMA side tone feature
ifeq ($(AIR_AMA_SIDETONE_ENABLE),y)
CFLAGS  += -DAIR_AMA_SIDETONE_ENABLE
endif
endif
# AMA end

# GSound VA
ifeq ($(AIR_GSOUND_ENABLE), y)
C_FILES += $(APPS_SRC)/app_gsound/app_gsound_battery_ohd.c
C_FILES += $(APPS_SRC)/app_gsound/app_gsound_idle_activity.c
C_FILES += $(APPS_SRC)/app_gsound/app_gsound_service.c
C_FILES += $(APPS_SRC)/app_gsound/app_gsound_multi_va.c
C_FILES += $(APPS_SRC)/app_gsound/app_gsound_device_action.c
endif

# Add xiaowei VA activity support
ifeq ($(AIR_XIAOWEI_ENABLE), y)
C_FILES += $(APPS_SRC)/app_va_xiaowei/app_va_xiaowei_activity.c
C_FILES += $(APPS_SRC)/app_va_xiaowei/app_va_xiaowei_multi_support.c
C_FILES += $(APPS_SRC)/app_va_xiaowei/app_va_xiaowei_transient_activity.c
C_FILES += $(APPS_SRC)/app_va_xiaowei/app_va_xiaowei_device_control.c
endif

# Add xiaoai VA activity support
ifeq ($(AIR_XIAOAI_ENABLE), y)
C_FILES += $(APPS_SRC)/app_va_xiaoai/app_va_xiaoai_activity.c
C_FILES += $(APPS_SRC)/app_va_xiaoai/app_va_xiaoai_ble_adv.c
C_FILES += $(APPS_SRC)/app_va_xiaoai/app_va_xiaoai_bt_helper.c
C_FILES += $(APPS_SRC)/app_va_xiaoai/app_va_xiaoai_device_config.c
C_FILES += $(APPS_SRC)/app_va_xiaoai/app_va_xiaoai_hfp_at_cmd.c
C_FILES += $(APPS_SRC)/app_va_xiaoai/app_va_xiaoai_ptt_speech_activity.c
C_FILES += $(APPS_SRC)/app_va_xiaoai/app_va_xiaoai_miui_fast_connect.c
C_FILES += $(APPS_SRC)/app_va_xiaoai/app_va_xiaoai_ota.c
endif

ifeq ($(AIR_MS_TEAMS_ENABLE), y)
CFLAGS  += -DAIR_MS_TEAMS_ENABLE
#CFLAGS  += -DAIR_AUDIO_VOLUME_MONITOR_ENABLE
C_FILES += $(APPS_SRC)/app_ms_teams/app_ms_teams_idle_activity.c
#C_FILES += $(APPS_SRC)/app_ms_teams/app_ms_teams_activity.c
C_FILES += $(APPS_SRC)/app_ms_teams/app_ms_teams_utils.c
C_FILES += $(APPS_SRC)/app_ms_teams/app_ms_teams_led.c
C_FILES += $(APPS_SRC)/app_ms_teams/app_ms_teams_telemetry.c
endif

ifeq ($(AIR_USB_HID_CALL_CTRL_ENABLE), y)
CFLAGS  += -DAIR_USB_HID_CALL_CTRL_ENABLE
C_FILES += $(APPS_SRC)/app_hid_call/app_hid_call_idle_activity.c
endif

ifeq ($(AIR_GOOGLE_SPATIAL_AUDIO_ENABLE), y)
C_FILES += $(APPS_SRC)/app_head_tracker/app_head_tracker_idle_activity.c
endif

# Add for line_in feature
AIR_APP_LINE_IN_ENABLED = n
ifeq ($(AIR_LINE_IN_ENABLE), y)
AIR_APP_LINE_IN_ENABLED = y
CFLAGS += -DAIR_LINE_IN_ENABLE
ifeq ($(AIR_LINE_IN_AND_BT_MIX_ENABLE), y)
AIR_LINE_IN_MIX_ENABLE = y
endif
ifeq ($(AIR_LINE_IN_MIX_ENABLE), y)
CFLAGS += -DAIR_LINE_IN_MIX_ENABLE
endif
endif
ifeq ($(AIR_LINE_OUT_ENABLE), y)
AIR_APP_LINE_IN_ENABLED = y
CFLAGS += -DAIR_LINE_OUT_ENABLE
endif
ifeq ($(AIR_APP_LINE_IN_ENABLED), y)
CFLAGS += -DAPPS_LINE_IN_SUPPORT
ifeq ($(AIR_APP_SYSTEM_ON_BY_LINE_IN_ENABLE),y)
CFLAGS += -DAIR_APP_SYSTEM_ON_BY_LINE_IN_ENABLE
endif
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_line_in
C_FILES += $(APPS_SRC)/app_line_in/app_line_in_idle_activity.c
endif

#usb audio
ifeq ($(AIR_USB_ENABLE),y)
AIR_APP_USB_AUDIO_ENABLED = n
ifeq ($(AIR_USB_AUDIO_IN_ENABLE),y)
AIR_APP_USB_AUDIO_ENABLED = y
CFLAGS += -DAIR_USB_AUDIO_IN_ENABLE
ifeq ($(AIR_USB_AUDIO_IN_AND_BT_MIX_ENABLE), y)
AIR_USB_AUDIO_IN_MIX_ENABLE = y
endif
ifeq ($(AIR_USB_AUDIO_IN_MIX_ENABLE),y)
CFLAGS += -DAIR_USB_AUDIO_IN_MIX_ENABLE
endif
endif
ifeq ($(AIR_USB_AUDIO_OUT_ENABLE),y)
AIR_APP_USB_AUDIO_ENABLED = y
CFLAGS += -DAIR_USB_AUDIO_OUT_ENABLE
endif
ifeq ($(AIR_APP_USB_AUDIO_ENABLED),y)
CFLAGS += -DAPPS_USB_AUDIO_SUPPORT
ifeq ($(AIR_USB_HID_ENABLE),y)
CFLAGS += -DAIR_USB_HID_MEDIA_CTRL_ENABLE
endif
ifeq ($(AIR_USB_AUDIO_IN_AND_OUT_MIX_ENABLE),y)
CFLAGS += -DAIR_USB_AUDIO_IN_AND_OUT_MIX_ENABLE
endif
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_usb_audio
C_FILES +=   $(APPS_SRC)/app_usb_audio/app_usb_audio_idle_activity.c
endif
endif

#app audio manager
AIR_APP_AUDIO_MANAGER_ENABLE = n
ifeq ($(AIR_APP_USB_AUDIO_ENABLED),y)
AIR_APP_AUDIO_TRANS_MGR_ENABLE = y
endif
ifeq ($(AIR_APP_LINE_IN_ENABLED), y)
AIR_APP_AUDIO_TRANS_MGR_ENABLE = y
endif
ifeq ($(AIR_DUAL_CHIP_MIXING_MODE), slave)
AIR_APP_AUDIO_TRANS_MGR_ENABLE = y
endif
ifeq ($(AIR_APP_AUDIO_TRANS_MGR_ENABLE), y)
CFLAGS += -DAPP_AUDIO_MANAGER_AUDIO_SOURCE_CTRL_ENABLE
CFLAGS += -DAIR_APP_AUDIO_TRANS_MGR_ENABLE
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/audio_trans_mgr
C_FILES += $(APPS_SRC)/audio_trans_mgr/app_audio_trans_mgr.c
endif

# leakage detection
ifeq ($(AIR_ANC_FIT_DETECTION_ENABLE), y)
C_FILES += $(APPS_SRC)/app_seal_check/app_seal_check_idle_activity.c
endif

ifeq ($(AIR_ANC_ENABLE), y)
ifeq ($(AIR_ANC_WIND_NOISE_DETECTION_ENABLE), y)
C_FILES += $(APPS_SRC)/app_adaptive_anc/app_anc_extend_gain_activity.c
else
ifeq ($(AIR_ANC_ENVIRONMENT_DETECTION_ENABLE), y)
C_FILES += $(APPS_SRC)/app_adaptive_anc/app_anc_extend_gain_activity.c
else
ifeq ($(AIR_ANC_USER_UNAWARE_ENABLE), y)
C_FILES += $(APPS_SRC)/app_adaptive_anc/app_anc_extend_gain_activity.c
endif
endif
endif
C_FILES += $(APPS_SRC)/app_adaptive_anc/app_adaptive_anc_idle_activity.c
endif

# share mode
ifeq ($(AIR_MCSYNC_SHARE_ENABLE), y)
C_FILES += $(APPS_SRC)/app_share/app_share_idle_activity.c
C_FILES += $(APPS_SRC)/app_share/app_share_activity.c
C_FILES += $(APPS_SRC)/app_share/app_share_utils.c
endif

# CFU
ifeq ($(AIR_CFU_ENABLE), y)
C_FILES += $(APPS_SRC)/app_ms_cfu/app_ms_cfu_idle_activity.c
endif

# Adaptive eq activity
ifeq ($(AIR_ADAPTIVE_EQ_ENABLE)_$(AIR_PEQ_ENABLE), y_y)
C_FILES += $(APPS_SRC)/app_adaptive_eq/app_adaptive_eq_idle_activity.c
CFLAGS  += -DAIR_ADAPTIVE_EQ_ENABLE
endif

# Self fitting activity
ifeq ($(AIR_ADVANCED_PASSTHROUGH_ENABLE), y)
C_FILES += $(APPS_SRC)/app_self_fitting/app_self_fitting_idle_activity.c
endif

# Wireless MIC
ifeq ($(AIR_WIRELESS_MIC_ENABLE), y)
C_FILES += $(APPS_SRC)/app_wireless_mic/app_wireless_mic_idle_activity.c
C_FILES += $(APPS_SRC)/app_wireless_mic/app_wireless_mic_fatfs.c
C_FILES += $(APPS_SRC)/app_wireless_mic/app_wireless_mic_realtime_task.c
ifeq ($(AIR_AUDIO_VOLUME_MONITOR_ENABLE), y)
C_FILES += $(APPS_SRC)/app_wireless_mic/app_wireless_mic_volume_det.c
endif
endif

# Spotify tap activity
ifeq ($(AIR_SPOTIFY_TAP_ENABLE), y)
C_FILES += $(APPS_SRC)/app_spotify_tap/app_spotify_tap_activity.c
endif

# Hearing Aid
ifeq ($(findstring y,$(AIR_HEARING_AID_ENABLE) $(AIR_HEARTHROUGH_PSAP_ENABLE)),y)
C_FILES += $(APPS_SRC)/app_hearing_aid/app_hearing_aid_activity.c
C_FILES += $(APPS_SRC)/app_hearing_aid/app_hearing_aid_utils.c
C_FILES += $(APPS_SRC)/app_hearing_aid/app_hearing_aid_config.c
C_FILES += $(APPS_SRC)/app_hearing_aid/app_hearing_aid_key_handler.c
C_FILES += $(APPS_SRC)/app_hearing_aid/app_hearing_aid_aws.c
C_FILES += $(APPS_SRC)/app_hearing_aid/app_hearing_aid_storage.c
endif

# Hear through
ifeq ($(AIR_HEARTHROUGH_MAIN_ENABLE), y)
C_FILES += $(APPS_SRC)/app_hear_through/app_hear_through_activity.c
C_FILES += $(APPS_SRC)/app_hear_through/app_hear_through_race_cmd_handler.c
C_FILES += $(APPS_SRC)/app_hear_through/app_hear_through_adv.c
C_FILES += $(APPS_SRC)/app_hear_through/app_hear_through_storage.c
endif

# APP RX EQ
C_FILES += $(APPS_SRC)/app_rx_eq/app_rx_eq.c

# Include bt sink path
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)
ifeq ($(MTK_BATTERY_MANAGEMENT_ENABLE),y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_battery
endif
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_bt_state
ifeq ($(MTK_FOTA_ENABLE),y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_fota
endif
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_gsound
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_hfp
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_idle
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_ull
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_multi_va
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/config
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/events
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/utils
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_music
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/led
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_power_save
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_preproc
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_fm
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_fast_pair
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/vp
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_smart_charger
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_ama
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_in_ear
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_seal_check
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_hid_call
ifeq ($(AIR_ANC_ENABLE), y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_adaptive_anc
endif
ifeq ($(AIR_TILE_ENABLE),y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_tile
endif
ifeq ($(AIR_DUAL_CHIP_MIXING_MODE),master)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_master
endif
ifeq ($(AIR_SWIFT_PAIR_ENABLE),y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_swift_pair
endif
# Adaptive eq activity
ifeq ($(AIR_ADAPTIVE_EQ_ENABLE)_$(AIR_PEQ_ENABLE), y_y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_adaptive_eq
endif
ifeq ($(AIR_ADVANCED_PASSTHROUGH_ENABLE), y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_self_fitting
endif

ifeq ($(AIR_GOOGLE_SPATIAL_AUDIO_ENABLE), y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_head_tracker
endif

# Add MS CFU support
ifeq ($(AIR_CFU_ENABLE), y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_ms_cfu
CFLAGS += -I$(SOURCE_DIR)/middleware/airoha/ms_cfu/inc
endif

ifeq ($(AIR_SPEAKER_ENABLE),y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_speaker
endif

# GSound VA
ifeq ($(AIR_GSOUND_ENABLE), y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_gsound
endif

# Add xiaowei VA activity support
ifeq ($(AIR_XIAOWEI_ENABLE), y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_va_xiaowei
CFLAGS += -I$(SOURCE_DIR)/middleware/airoha/xiaowei/inc
endif

# Add xiaoai VA activity support
ifeq ($(AIR_XIAOAI_ENABLE), y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_va_xiaoai
CFLAGS += -I$(SOURCE_DIR)/middleware/airoha/xiaoai/inc
endif
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_state_report

# Add MS Teams support
ifeq ($(AIR_MS_TEAMS_ENABLE), y)
# CFLAGS += -DMS_TEAMS_TEST
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_ms_teams
CFLAGS += -I$(SOURCE_DIR)/middleware/airoha/ms_teams/inc
endif

#APP LE AUDIO
ifeq ($(AIR_LE_AUDIO_ENABLE), y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_le_audio
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_lea_service
endif

# app ULL2
ifeq ($(AIR_LE_AUDIO_ENABLE)_$(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE), n_y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_lea_service
endif

ifeq ($(AIR_LE_AUDIO_ENABLE)_$(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE), n_y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_lea_service
endif


ifeq ($(AIR_WIRELESS_MIC_ENABLE), y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_wireless_mic
endif

ifeq ($(AIR_SPOTIFY_TAP_ENABLE), y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_spotify_tap
CFLAGS += -I$(SOURCE_DIR)/middleware/airoha/spotify_tap/inc
endif

# Add for hearing aid
ifeq ($(findstring y,$(AIR_HEARING_AID_ENABLE) $(AIR_HEARTHROUGH_PSAP_ENABLE)),y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_hearing_aid
endif

# Add for hear through
ifeq ($(AIR_HEARTHROUGH_MAIN_ENABLE), y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_hear_through
endif

# APP RX EQ
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_rx_eq

ifeq ($(AIR_BLE_HRS_ENABLE),y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_hrs
endif
#APPs features

##
## AIR_APPS_DISABLE_BT_WHEN_CHARGING_ENABLE
## Brief:       This option is used to enable the feature that disabling BT if device is charging.
## Usage:       Enable the feature by configuring it as y.
## Path:        project
## Dependency:  None
## Notice:      Do not enable the feature if CCASE_ENABLE is y
## Relative doc:None
##
ifeq ($(AIR_APPS_DISABLE_BT_WHEN_CHARGING_ENABLE), y)
CFLAGS += -DAPPS_DISABLE_BT_WHEN_CHARGING
endif

##
## APPS_AUTO_TRIGGER_RHO
## Brief:       This option is used to enable the feature that triggers RHO before power off, disable BT and battery level of agent is lower than partner.
## Usage:       Enable the feature by configuring it as y.
## Path:        project
## Dependency:  SUPPORT_ROLE_HANDOVER_SERVICE
## Notice:      Do not enable APPS_TRIGGER_RHO_BY_KEY and APPS_AUTO_TRIGGER_RHO in one project
## Relative doc:None
##
ifeq ($(APPS_AUTO_TRIGGER_RHO), y)
# Do RHO when agent low battery, power off or disable BT
CFLAGS += -DAPPS_AUTO_TRIGGER_RHO
endif

##
## APPS_TRIGGER_RHO_BY_KEY
## Brief:       This option is used to enable the feature trigger RHO user presses key.
## Usage:       Enable the feature by configuring it as y.
## Path:        project
## Dependency:  SUPPORT_ROLE_HANDOVER_SERVICE
## Notice:      Do not enable APPS_TRIGGER_RHO_BY_KEY and APPS_AUTO_TRIGGER_RHO in one project
## Relative doc:None
##
ifeq ($(APPS_TRIGGER_RHO_BY_KEY), y)
# Do RHO press key on partner
CFLAGS += -DAPPS_TRIGGER_RHO_BY_KEY
endif

##
## AIR_APPS_POWER_SAVE_ENABLE
## Brief:       This option is used to enable the feature start a timer to sleep when device doesn't connect to smart phone.
## Usage:       Enable the feature by configuring it as y.
## Path:        project
## Dependency:  None
## Notice:      None
## Relative doc:None
##
ifeq ($(AIR_APPS_POWER_SAVE_ENABLE), y)
# Press key to do air pairing
CFLAGS += -DAPPS_SLEEP_AFTER_NO_CONNECTION
endif

##
## AIR_TILE_ENABLE
## Brief:       This option is used to enable the feature to start or change the advertising of Tile.
## Usage:       Enable the feature by configuring it as y.
## Path:        project
## Dependency:  None
## Notice:      None
## Relative doc:None
##
ifeq ($(AIR_TILE_ENABLE), y)
CFLAGS += -DAIR_TILE_ENABLE
endif

# Multi Voice assistant
ifeq ($(AIR_MULTI_VA_SUPPORT_COMPETITION_ENABLE), y)
CFLAGS += -DMULTI_VA_SUPPORT_COMPETITION
endif

ifeq ($(AIR_WEARING_DETECTION_ENABLE), y)
CFLAGS += -DMTK_IN_EAR_FEATURE_ENABLE
endif

ifeq ($(AIR_MCSYNC_SHARE_ENABLE), y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_share
CFLAGS += -DAIR_MCSYNC_SHARE_ENABLE
endif

CFLAGS += -DTEMP_CLASSIC_BT_OFF

# app leaudio
ifeq ($(AIR_LE_AUDIO_ENABLE), y)

# Use direct adv to avoiding Targeted announcement flag on whitelist to impact other device.
# CFLAGS += -DAIR_LE_AUDIO_USE_DIRECT_ADV_TO_ACTIVE_RECONNECT
# CFLAGS += -DAIR_LE_AUDIO_DIRECT_ADV

C_FILES += $(APPS_SRC)/app_le_audio/app_le_audio.c
C_FILES += $(APPS_SRC)/app_le_audio/app_mcp.c
C_FILES += $(APPS_SRC)/app_le_audio/app_ccp.c
ifeq ($(AIR_TWS_ENABLE), y)
C_FILES += $(APPS_SRC)/app_le_audio/app_le_audio_hfp_at_cmd.c
C_FILES += $(APPS_SRC)/app_le_audio/app_dual_headset_sink_ble_service.c
C_FILES += $(APPS_SRC)/app_le_audio/app_dual_headset_sink_sdp.c
C_FILES += $(APPS_SRC)/app_le_audio/app_mps.c
endif
C_FILES += $(APPS_SRC)/app_le_audio/app_le_audio_aird_client.c
ifeq ($(AIR_LE_AUDIO_BIS_ENABLE), y)
C_FILES += $(APPS_SRC)/app_le_audio/app_le_audio_bis.c
C_FILES += $(APPS_SRC)/app_le_audio/app_le_audio_bis_activity.c
endif
C_FILES += $(APPS_SRC)/app_lea_service/app_lea_service.c
C_FILES += $(APPS_SRC)/app_lea_service/app_lea_service_adv_mgr.c
C_FILES += $(APPS_SRC)/app_lea_service/app_lea_service_conn_mgr.c
C_FILES += $(APPS_SRC)/app_lea_service/app_lea_service_sync_info.c
C_FILES += $(APPS_SRC)/app_lea_service/app_lea_service_target_addr.c
endif

# app ULL2 - need app_lea_service
ifeq ($(AIR_LE_AUDIO_ENABLE)_$(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE), n_y)
C_FILES += $(APPS_SRC)/app_lea_service/app_lea_service.c
C_FILES += $(APPS_SRC)/app_lea_service/app_lea_service_adv_mgr.c
C_FILES += $(APPS_SRC)/app_lea_service/app_lea_service_conn_mgr.c
C_FILES += $(APPS_SRC)/app_lea_service/app_lea_service_sync_info.c
C_FILES += $(APPS_SRC)/app_lea_service/app_lea_service_target_addr.c
endif

ifeq ($(AIR_LE_AUDIO_ENABLE)_$(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE), n_y)
C_FILES += $(APPS_SRC)/app_lea_service/app_lea_service.c
C_FILES += $(APPS_SRC)/app_lea_service/app_lea_service_adv_mgr.c
C_FILES += $(APPS_SRC)/app_lea_service/app_lea_service_conn_mgr.c
C_FILES += $(APPS_SRC)/app_lea_service/app_lea_service_sync_info.c
C_FILES += $(APPS_SRC)/app_lea_service/app_lea_service_target_addr.c
endif

ifeq ($(AIR_PMU_CHARGER_PFM_ENABLE), y)
CFLAGS += -DAIR_PMU_CHARGER_PFM_ENABLE
endif

ifeq ($(AIR_MS_GIP_ENABLE), y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_ms_xbox
C_FILES += $(APPS_SRC)/app_ms_xbox/app_ms_xbox_idle_activity.c
endif # AIR_MS_GIP_ENABLE

ifeq ($(AIR_BLE_HRS_ENABLE),y)
#hrs
C_FILES += $(APPS_SRC)/app_hrs/ble_app_hrs.c
C_FILES += $(APPS_SRC)/app_hrs/ble_app_hrs_data.c
endif

CFLAGS += -DAIR_BT_LINKRECORD_ENCRYPTED
