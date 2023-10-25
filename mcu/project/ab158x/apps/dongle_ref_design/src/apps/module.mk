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

APPS_SRC = $(APP_PATH)/src/apps
APPS_INC = $(APP_PATH)/inc/apps
APPS_COMMON_SRC = $(APP_COMMON_PATH)/src/apps
APPS_COMMON_INC = $(APP_COMMON_PATH)/inc/apps

C_FILES += $(APPS_SRC)/apps_init.c
# Apps C files
ifeq ($(AIR_FOTA_ENABLE),y)
C_FILES += $(APPS_COMMON_SRC)/app_fota/app_fota_idle_activity.c
#C_FILES += $(APPS_SRC)/app_fota/app_fota_activity.c
endif

# Pre-proc activity
C_FILES += $(APPS_SRC)/app_preproc/app_preproc_activity.c
#C_FILES += $(APPS_SRC)/app_preproc/app_preproc_sys_pwr.c

ifeq ($(MTK_BATTERY_MANAGEMENT_ENABLE),y)
# Battery app
C_FILES += $(APPS_COMMON_SRC)/app_battery/app_battery_idle_activity.c
C_FILES += $(APPS_COMMON_SRC)/app_battery/app_battery_transient_activity.c
endif
# BT state service
C_FILES += $(APPS_SRC)/app_bt_state/app_bt_state_service.c

ifeq ($(AIR_APPS_POWER_SAVE_ENABLE),y)
# Power saving
C_FILES += $(APPS_COMMON_SRC)/app_power_save/app_power_save_idle_activity.c
C_FILES += $(APPS_COMMON_SRC)/app_power_save/app_power_save_timeout_activity.c
C_FILES += $(APPS_COMMON_SRC)/app_power_save/app_power_save_waiting_activity.c
C_FILES += $(APPS_COMMON_SRC)/app_power_save/app_power_save_utils.c
endif

# app service
ifeq ($(AIR_BT_ULTRA_LOW_LATENCY_ENABLE),y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_dongle_service
C_FILES += $(APPS_SRC)/app_dongle_service/app_dongle_service.c
endif

# app service
ifeq ($(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE),y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_dongle_service
C_FILES += $(APPS_SRC)/app_dongle_service/app_dongle_service.c
endif

# app service
ifeq ($(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE),y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_dongle_service
C_FILES += $(APPS_SRC)/app_dongle_service/app_dongle_service.c
endif

# Some utils app and home screen app
ifeq ($(AIR_BT_ROLE_HANDOVER_ENABLE),y)
C_FILES += $(APPS_SRC)/app_idle/app_rho_idle_activity.c
endif
C_FILES += $(APPS_SRC)/app_idle/app_home_screen_idle_activity.c
ifeq ($(findstring y,$(AIR_LE_AUDIO_DONGLE_ENABLE) $(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)),y)
C_FILES += $(APPS_SRC)/app_idle/app_le_audio_component_in_homescreen.c
endif
ifeq ($(findstring y,$(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) $(AIR_BT_AUDIO_DONGLE_ENABLE)),y)
C_FILES += $(APPS_SRC)/app_idle/app_bt_conn_componet_in_homescreen.c
endif

ifeq ($(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE),y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_dongle_service
C_FILES += $(APPS_SRC)/app_idle/app_le_audio_component_in_homescreen.c
endif

# App state report
C_FILES += $(APPS_COMMON_SRC)/app_state_report/apps_state_report.c

# ull dongle acitivity
C_FILES += $(APPS_SRC)/app_dongle/app_ull_dongle_idle_activity.c
C_FILES += $(APPS_SRC)/app_dongle/app_dongle_race.c
C_FILES += $(APPS_SRC)/app_dongle/app_dongle_common_idle_activity.c
C_FILES += $(APPS_SRC)/app_dongle/app_dongle_connection_common.c
C_FILES += $(APPS_SRC)/app_dongle/app_dongle_le_race.c
ifeq ($(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE),y)
C_FILES += $(APPS_SRC)/app_dongle/app_ull_dongle_le.c
endif

ifeq ($(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE),y)
C_FILES += $(APPS_SRC)/app_dongle/app_dongle_ull_le_hid.c
endif

C_FILES += $(APPS_SRC)/app_i2s/app_i2s_idle_activity.c

# Events folder
C_FILES += $(APPS_SRC)/events/apps_events_key_event.c
C_FILES += $(APPS_SRC)/events/apps_events_battery_event.c
C_FILES += $(APPS_SRC)/events/apps_events_bt_event.c
C_FILES += $(APPS_SRC)/events/apps_race_cmd_event.c
ifeq ($(AIR_USB_ENABLE), y)
C_FILES += $(APPS_SRC)/events/apps_events_usb_event.c
endif

#ifeq ($(findstring y,$(AIR_GAMING_MODE_DONGLE_LINE_IN_ENABLE) $(AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE) $(AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE) $(AIR_GAMING_MODE_DONGLE_V2_LINE_OUT_ENABLE)),y)
C_FILES += $(APPS_SRC)/events/apps_events_line_in_event.c
C_FILES += $(APPS_SRC)/events/apps_events_i2s_in_event.c
#endif

ifeq ($(findstring y,$(AIR_GAMING_MODE_DONGLE_LINE_IN_ENABLE) $(AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE)),y)
CFLAGS += -DAIR_ULL_DONGLE_LINE_IN_ENABLE
endif

ifeq ($(findstring y,$(AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE) $(AIR_GAMING_MODE_DONGLE_V2_LINE_OUT_ENABLE)),y)
CFLAGS += -DAIR_ULL_DONGLE_LINE_OUT_ENABLE
endif

# Config folder
C_FILES += $(APPS_COMMON_SRC)/config/apps_config_key_remapper.c
C_FILES += $(APPS_COMMON_SRC)/config/apps_config_led_manager.c
C_FILES += $(APPS_COMMON_SRC)/config/apps_config_features_dynamic_setting.c

# LED
C_FILES += $(APPS_COMMON_SRC)/led/app_led_control.c
C_FILES += $(APPS_COMMON_SRC)/led/led_control_internal.c
C_FILES += $(APPS_COMMON_SRC)/led/led_control_style_cfg.c

# Utils
C_FILES += $(APPS_COMMON_SRC)/utils/apps_debug.c
ifeq ($(MTK_AWS_MCE_ENABLE),y)
C_FILES += $(APPS_COMMON_SRC)/utils/apps_aws_sync_event.c
endif
C_FILES += $(APPS_COMMON_SRC)/utils/apps_dongle_sync_event.c

ifeq ($(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE), y)
C_FILES += $(APPS_SRC)/utils/apps_usb_utils.c
C_FILES += $(APPS_SRC)/utils/app_key_remap.c
endif

ifeq ($(AIR_AUDIO_DETACHABLE_MIC_ENABLE), y)
C_FILES += $(APPS_COMMON_SRC)/utils/apps_detachable_mic.c
CFLAGS  += -DAIR_AUDIO_DETACHABLE_MIC_ENABLE
endif

ifeq ($(AIR_MS_TEAMS_ENABLE), y)
#ifeq ($(AIR_BT_AUDIO_DONGLE_ENABLE), y)
#CFLAGS += -DAIR_MS_TEAMS_SPECIAL_CLIENT_ENABLE
#C_FILES += $(APPS_SRC)/app_ms_teams/app_ms_teams_special_client1.c
#endif
CFLAGS  += -DAIR_MS_TEAMS_ENABLE
C_FILES += $(APPS_SRC)/app_ms_teams/app_ms_teams_idle_activity.c
#C_FILES += $(APPS_SRC)/app_ms_teams/app_ms_teams_activity.c
C_FILES += $(APPS_SRC)/app_ms_teams/app_ms_teams_utils.c
C_FILES += $(APPS_SRC)/app_ms_teams/app_ms_teams_led.c
C_FILES += $(APPS_SRC)/app_ms_teams/app_ms_teams_telemetry.c
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
C_FILES += $(APPS_COMMON_SRC)/app_line_in/app_line_in_idle_activity.c
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
ifeq ($(AIR_APP_AUDIO_TRANS_MGR_ENABLE), y)
CFLAGS += -DAPP_AUDIO_MANAGER_AUDIO_SOURCE_CTRL_ENABLE
CFLAGS += -DAIR_APP_AUDIO_TRANS_MGR_ENABLE
CFLAGS += -I$(SOURCE_DIR)/$(APPS_COMMON_INC)/audio_trans_mgr
C_FILES += $(APPS_COMMON_SRC)/audio_trans_mgr/app_audio_trans_mgr.c
endif

# CFU
ifeq ($(AIR_CFU_ENABLE), y)
C_FILES += $(APPS_SRC)/app_ms_cfu/app_ms_cfu_idle_activity.c
endif

# Include bt sink path
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)
ifeq ($(MTK_BATTERY_MANAGEMENT_ENABLE),y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_COMMON_INC)/app_battery
endif
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_bt_state
ifeq ($(AIR_FOTA_ENABLE),y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_COMMON_INC)/app_fota
endif
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_idle
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_in_ear
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_dongle
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_i2s
CFLAGS += -I$(SOURCE_DIR)/$(APPS_COMMON_INC)/config
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/events
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/utils
CFLAGS += -I$(SOURCE_DIR)/$(APPS_COMMON_INC)/utils
CFLAGS += -I$(SOURCE_DIR)/$(APPS_COMMON_INC)/led
ifeq ($(AIR_APPS_POWER_SAVE_ENABLE),y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_COMMON_INC)/app_power_save
endif
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_preproc
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/vp
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_leakage_detection
CFLAGS += -I$(SOURCE_DIR)/$(APPS_COMMON_INC)/app_state_report

# Add MS Teams support
ifeq ($(AIR_MS_TEAMS_ENABLE), y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_ms_teams
CFLAGS += -I$(SOURCE_DIR)/middleware/airoha/ms_teams/inc
endif

# Add MS CFU support
ifeq ($(AIR_CFU_ENABLE), y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_ms_cfu
CFLAGS += -I$(SOURCE_DIR)/middleware/airoha/ms_cfu/inc
endif

# app_le_audio
ifeq ($(AIR_LE_AUDIO_ENABLE), y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_le_audio
endif

# BT Source
ifeq ($(AIR_BT_AUDIO_DONGLE_ENABLE), y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_bt_source
endif

# app_le_air
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_dongle_air

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
## AIR_APPS_AUTO_TRIGGER_RHO_ENABLE
## Brief:       This option is used to enable the feature that triggers RHO before power off, disable BT and battery level of agent is lower than partner.
## Usage:       Enable the feature by configuring it as y.
## Path:        project
## Dependency:  AIR_BT_ROLE_HANDOVER_ENABLE
## Notice:      Do not enable AIR_APPS_TRIGGER_RHO_BY_KEY_ENABLE and AIR_APPS_AUTO_TRIGGER_RHO_ENABLE in one project
## Relative doc:None
##
ifeq ($(AIR_APPS_AUTO_TRIGGER_RHO_ENABLE), y)
# Do RHO when agent low battery, power off or disable BT
CFLAGS += -DAPPS_AUTO_TRIGGER_RHO
endif

##
## AIR_APPS_TRIGGER_RHO_BY_KEY_ENABLE
## Brief:       This option is used to enable the feature trigger RHO user presses key.
## Usage:       Enable the feature by configuring it as y.
## Path:        project
## Dependency:  AIR_BT_ROLE_HANDOVER_ENABLE
## Notice:      Do not enable AIR_APPS_TRIGGER_RHO_BY_KEY_ENABLE and AIR_APPS_AUTO_TRIGGER_RHO_ENABLE in one project
## Relative doc:None
##
ifeq ($(AIR_APPS_TRIGGER_RHO_BY_KEY_ENABLE), y)
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

# app gsound
ifeq ($(GSOUND_LIBRARY_ENABLE), y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_gsound
C_FILES += $(APPS_SRC)/app_gsound/app_gsound_service.c
C_FILES += $(APPS_SRC)/app_gsound/app_gsound_multi_va.c
endif

# Multi Voice assistant
ifeq ($(AIR_MULTI_VA_SUPPORT_COMPETITION_ENABLE), y)
CFLAGS += -DMULTI_VA_SUPPORT_COMPETITION
endif

ifeq ($(MTK_IN_EAR_FEATURE_ENABLE), y)
CFLAGS += -DMTK_IN_EAR_FEATURE_ENABLE
endif

ifeq ($(MTK_LEAKAGE_DETECTION_ENABLE), y)
CFLAGS += -DMTK_LEAKAGE_DETECTION_ENABLE
endif

ifeq ($(AIR_LINE_IN_ENABLE), y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_COMMON_INC)/app_line_in
ifeq ($(LINE_IN_ENABLE_WITH_LOW_LEVEL), y)
CFLAGS += -DLINE_IN_ENABLE_WITH_LOW_LEVEL
endif
# Pressing key to do power off only disable BT when Line in. If system on is triggered by line in, do not enable BT.
ifeq ($(AIR_APP_SYSTEM_ON_BY_LINE_IN_ENABLE),y)
CFLAGS += -DAIR_APP_SYSTEM_ON_BY_LINE_IN_ENABLE
endif
endif


# app_le_audio
ifeq ($(AIR_LE_AUDIO_ENABLE), y)
C_FILES += $(APPS_SRC)/app_le_audio/app_le_audio.c
C_FILES += $(APPS_SRC)/app_le_audio/app_le_audio_usb.c
C_FILES += $(APPS_SRC)/app_le_audio/app_le_audio_utillity.c
C_FILES += $(APPS_SRC)/app_le_audio/app_le_audio_i2s_in.c
C_FILES += $(APPS_SRC)/app_le_audio/app_le_audio_line_in.c
C_FILES += $(APPS_SRC)/app_le_audio/app_le_audio_transmitter.c

ifeq ($(AIR_LE_AUDIO_UNICAST_ENABLE), y)
C_FILES += $(APPS_SRC)/app_le_audio/app_le_audio_ucst.c
C_FILES += $(APPS_SRC)/app_le_audio/app_le_audio_ucst_utillity.c
C_FILES += $(APPS_SRC)/app_le_audio/app_le_audio_aird.c
C_FILES += $(APPS_SRC)/app_le_audio/app_le_audio_race.c
C_FILES += $(APPS_SRC)/app_le_audio/app_le_audio_ccp_call_control_server.c
C_FILES += $(APPS_SRC)/app_le_audio/app_le_audio_csip_set_coordinator.c
C_FILES += $(APPS_SRC)/app_le_audio/app_le_audio_micp_micophone_controller.c
C_FILES += $(APPS_SRC)/app_le_audio/app_le_audio_silence_detection.c
C_FILES += $(APPS_SRC)/app_le_audio/app_le_audio_vcp_volume_controller.c
C_FILES += $(APPS_SRC)/app_le_audio/app_le_audio_tmap.c
C_FILES += $(APPS_SRC)/app_le_audio/app_le_audio_hap.c
endif

ifeq ($(AIR_LE_AUDIO_BIS_ENABLE), y)
C_FILES += $(APPS_SRC)/app_le_audio/app_le_audio_bcst.c
C_FILES += $(APPS_SRC)/app_le_audio/app_le_audio_bcst_utillity.c
endif

ifeq ($(AIR_LE_AUDIO_DONGLE_ENABLE)_$(AIR_LE_AUDIO_UNICAST_ENABLE)_$(AIR_LE_AUDIO_BIS_ENABLE),y_y_y)
C_FILES += $(APPS_SRC)/app_le_audio/app_le_audio_ba.c
C_FILES += $(APPS_SRC)/app_le_audio/app_le_audio_ba_utillity.c
endif
endif

# BT Source
ifeq ($(AIR_BT_AUDIO_DONGLE_ENABLE), y)
C_FILES += $(APPS_SRC)/app_bt_source/app_bt_source.c
C_FILES += $(APPS_SRC)/app_bt_source/app_bt_source_conn_mgr.c
C_FILES += $(APPS_SRC)/app_bt_source/app_bt_source_call.c
C_FILES += $(APPS_SRC)/app_bt_source/app_bt_source_music.c
endif

# app_dongle_air
C_FILES += $(APPS_SRC)/app_dongle_air/app_le_audio_air.c

CFLAGS += -DTEMP_CLASSIC_BT_OFF

# MS GIP
ifeq ($(AIR_MS_GIP_ENABLE), y)
CFLAGS += -I$(SOURCE_DIR)/$(APPS_INC)/app_ms_xbox
C_FILES += $(APPS_SRC)/app_ms_xbox/app_ms_xbox_idle_activity.c
endif # AIR_MS_GIP_ENABLE

