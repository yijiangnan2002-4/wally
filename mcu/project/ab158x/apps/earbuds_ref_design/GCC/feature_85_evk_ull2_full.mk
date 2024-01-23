# Copyright Statement:
#
# (C) 2021  Airoha Technology Corp. All rights reserved.
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

IC_CONFIG                             := ab158x
BOARD_CONFIG                          := ab158x_evb
IC_TYPE                               := ab1585
BOARD_TYPE                            := ab1585_evk
BL_FEATURE                            := feature_85_evb.mk

# Using specific linker script during linking process.
AIR_MCU_LINKER_SCRIPT_FILE = ab158x_flash_8m.ld

##############################################
# NOTE - IMPORTANT
# MCU AND DSP SHARED FEATURE OPTION HAS BEEN REMOVED FROM MCU SIDE FROM SDK 3.8.0 BY INCLUDING THE DSP FEATURE MAKEFILE IN THE MCU SIDE.
# PLEASE DO NOT ADD FEATURE OPTION THAT ALREADY EXIST IN THE DSP FEATURE MAKEFILE,
# PLEASE MODIFY THE FEATURE OPTION IN THE CORRESPONDING DSP FEATURE MAKEFILE, THAT INCLUDED IN THE MCU SIDE IN THE LAST OF THIS FILE.
##############################################

##############################################
#           Custom feature option            #
##############################################

## part1: configure y/n

###### Audio/Voice effects ######

# This option is uesd for enhanced gain setting.
AIR_AUDIO_GAIN_SETTING_ENHANCE_ENABLE = y

# This option is used to enable/disable the AT CMD trigging voice prompt.
AIR_AUDIO_AT_CMD_PROMPT_SOUND_ENABLE = n

# This option is used to enable/disable amp dc compensation.
AIR_AMP_DC_COMPENSATION_ENABLE = y

# This option is used to enable/disable SBC encoder.
AIR_SBC_ENCODER_ENABLE = y

# This option is used to support stereo MP3 voice prompt.
AIR_MP3_STEREO_SUPPORT_ENABLE = y

###### Scenario ######

# This option is used to enable record. It must be enabled when using voice assistant.
AIR_RECORD_ENABLE = y

# This option is used to enable/disable BT sink state manager.
AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE = y

######  Codec ######

# This option is to enable Codec Manager.
AIR_AUDIO_CODEC_MANAGER_ENABLE = y

# This option is used to add opus encode support.
AIR_OPUS_ENCODER_ENABLE = y

######  Voice assistant/VAD ######

# This option is used to enable AMA feature.
AIR_AMA_ENABLE = y

# This option is used to enable AMA IAP2 .
# Dependency: AIR_AMA_ENABLE must be enabled when this option is set to y.
AIR_AMA_IAP2_APP_RELAY_ENABLE = y

# This option is used to enable AMA control sidetone.
# Dependency: AIR_AMA_ENABLE must be enabled when this option is set to y.
AIR_AMA_SIDETONE_ENABLE = y

# This option is used to enable AMA WWE during calling feature
# Dependency: AIR_AMA_ENABLE and AIR_AMA_HOTWORD_ENABLE must be enabled when this option is set to y.
AIR_AMA_HOTWORD_DURING_CALL_ENABLE = y

# This option is used to enable AMA ADV before EDR connected feature
# Dependency: AIR_AMA_ENABLE must be enabled when this option is set to y.
AIR_AMA_ADV_ENABLE_BEFORE_EDR_CONNECT_ENABLE = n

# This option is used to enable GSound.
# Dependency: AIR_RECORD_ENABLE and AIR_SBC_ENCODER_ENABLE must be enabled when this option is set to y.
AIR_GSOUND_ENABLE = y

# This option is used to enable multi va support competition.
AIR_MULTI_VA_SUPPORT_COMPETITION_ENABLE = y

# This option is used to enable Xiaowei.
# Dependency: AIR_RECORD_ENABLE and AIR_OPUS_ENCODER_ENABLE must be enabled when this option is set to y.
AIR_XIAOWEI_ENABLE = n

######  BT/MCSync ######

# This option is used to enable/disable BT related AT command.
AIR_BT_AT_COMMAND_ENABLE = y

# This option is used to enable some speaker-specific functions
AIR_BT_SPEAKER_ENABLE = n

# This option is used to enable multi link at the same time.
AIR_MULTI_POINT_ENABLE = y

# This option is used to specify the maximum number of connections that can exist simultaneously.
# Dependency: If the AIR_MULTI_POINT_ENABLE option is enabled, this option must be set to 2 or a larger value.
AIR_MULTI_POINT_MAX_CONNECTION = 2

# This option is to enable/disable MUX BT HID feature.
# Dependency:AIR_BT_HID_ENABLE must be enable when this option is set to y.
AIR_MUX_BT_HID_ENABLE = n

# This option is to enable/disable BT HID feature.
AIR_BT_HID_ENABLE = n

# This option is to set whether earbuds can join another set of earbuds to listen common music.
AIR_MCSYNC_SHARE_ENABLE = y

# This option is to enable/disable BT Intel EVO certification items.
AIR_BT_INTEL_EVO_ENABLE = n

# This option is used to support LE Audio broadcast.
# Dependency: AIR_LE_AUDIO_ENABLE must be enabled when this option is set to y.
AIR_LE_AUDIO_BIS_ENABLE = y

# This option is used to support dual mode of classic/LE Audio.
# Dependency: AIR_LE_AUDIO_ENABLE must be enabled when this option is set to y.
AIR_LE_AUDIO_DUAL_MODE_ENABLE = n

# This option is used to support LE Audio Telephony and Media Audio Profile.
# Dependency: AIR_LE_AUDIO_ENABLE must be enabled when this option is set to y.
AIR_LE_AUDIO_TMAP_ENABLE = y

# This option is used to support LE Audio Hearing Access Profile.
# Dependency: AIR_LE_AUDIO_ENABLE must be enabled when this option is set to y.
AIR_LE_AUDIO_HAP_ENABLE = n

# This option is used to support LE Audio Gaming Audio Profile.
# Dependency: AIR_LE_AUDIO_ENABLE must be enabled when this option is set to y.
AIR_LE_AUDIO_GMAP_ENABLE = n

# This option is used to support Auracast
# Dependency: AIR_LE_AUDIO_ENABLE and AIR_LE_AUDIO_BIS_ENABLE must be enabled when this option is set to y.
AIR_LE_AUDIO_AURACAST_ENABLE = y

# This option is to enable/disable latency test mode
AIR_BT_LATENCY_TEST_MODE_ENABLE = n

# This option is to enable/disable BLE OTP(Object Transfer Profile).
AIR_LE_OTP_ENABLE = n

# This option is used to enable the feature that iap2 data is transmitted through MUX
AIR_IAP2_VIA_MUX_ENABLE = y

# This option is used to enable teams.
# Dependency: AIR_USB_AUDIO_HID_ENABLE must be enabled when this option is set to y.
AIR_MS_TEAMS_ENABLE = y

# This option is used to enable ull2.0.
AIR_BLE_ULTRA_LOW_LATENCY_ENABLE = y

######  System ######

# This option is used to enable fast pair.
AIR_BT_FAST_PAIR_ENABLE = y

# This option is used to enable fast pair SASS.
# Dependency: AIR_BT_FAST_PAIR_ENABLE must be enabled when this option is set to y. AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE and AIR_MULTI_POINT_ENABLE must be the same.
AIR_BT_FAST_PAIR_SASS_ENABLE = y

# This option is used to enable fast pair spot.
# Dependency: AIR_BT_FAST_PAIR_ENABLE must be enabled when this option is set to y.
AIR_SPOT_ENABLE = y

# This option is used to enable FOTA basic function.
AIR_FOTA_ENABLE = y

# This option is to enable FOTA via race cmd.
# Dependency: AIR_FOTA_ENABLE, AIR_RACE_CMD_ENABLE and AIR_RACE_DUAL_CMD_ENABLE (if it is an earbuds project) must be enabled when this option is set to y.
AIR_FOTA_VIA_RACE_CMD_ENABLE = y

# This option is to enable smart charger case.
AIR_SMART_CHARGER_ENABLE = y

# This option is to enable smart charger 1wire function.
AIR_1WIRE_ENABLE = y

# This option is to switch early log print from uart1.
AIR_UART1_EARLY_LOG_ENABLE = n

# This option is used to enable wearing detection.
AIR_WEARING_DETECTION_ENABLE = y

# This option is used to enable component firmware update module.
# Dependency: AIR_USB_AUDIO_HID_ENABLE, AIR_LE_AUDIO_ENABLE, AIR_RACE_CMD_ENABLE, AIR_FOTA_ENABLE must enable, and AIR_USB_AUDIO_VERSION cannot be none.
AIR_CFU_ENABLE = n

# This option is to enable MS XBOX GIP feature.
# Dependency: AIR_BT_ULTRA_LOW_LATENCY_ENABLE must be enabled when this option is set to y.
AIR_MS_GIP_ENABLE = n

# This option is used to support RACE CMD trigger Find me.
# Dependency: AIR_RACE_CMD_ENABLE must be enabled when this option is set to y.
AIR_RACE_FIND_ME_ENABLE = y

# This option is used to enable race cmd.
AIR_RACE_CMD_ENABLE = y

# This option is used to enable 3rd party BLE advertising
AIR_TILE_ENABLE = n

# This option is to enable minidump feature.
AIR_MINIDUMP_ENABLE = n

# This option is to enable and disable CPU utilization function.
AIR_OS_CPU_UTILIZATION_ENABLE = y

# This option is used to enable the feature that disabling BT if device is charging.
AIR_APPS_DISABLE_BT_WHEN_CHARGING_ENABLE = y

# This option is used to enable/disable the power saving in APP.
AIR_APPS_POWER_SAVE_ENABLE = y

# This option is used to enable/disable key driver.
AIR_AIRO_KEY_ENABLE = y

# This option is used to enable/disable eint key module.
AIR_EINT_KEY_ENABLE = y

# This option is used to enable/disable secure boot module.
# NOTICE:
#   - This option can only be enabled if the secure-boot-addon-repo exists.
#   - secure-boot-addon-repo path: mcu\prebuilt\middleware\airoha\secure_boot
#   - If this option enabled without secure boot addon repo, the code will build fail.
AIR_SECURITY_SECURE_BOOT_ENABLE = n

# This option is to enable/disable Microsoft Swift Pair.
AIR_SWIFT_PAIR_ENABLE = y

# This option is to enable/disable Spotify Tap feature
AIR_SPOTIFY_TAP_ENABLE = y

# This option is used to enable/disable google spatial audio head tracker. It should be disable by default.
# Dependency: AIR_BT_HID_ENABLE or AIR_USB_HID_ENABLE must be enable when this option is set to y.
AIR_GOOGLE_SPATIAL_AUDIO_ENABLE = n

## part2: need to set specified value

###### Voice assistant/VAD ######

# This option is to configure key trigger mode of AMA.
# Usage: NONE, PTT, TTT
#     NONE : The button trigger mode is disabled
#     TTT : Tap-To-Talk trigger mode.
#     PTT : Push-To-Talk trigger mode.
# Dependency: AIR_AMA_ENABLE must be enabled when this option is not set to NONE.
AIR_AMA_BUTTON_TRIGGER_MODE = TTT

######  System ######

# This option is to configure mbedTLS features.
AIR_MBEDTLS_CONFIG_FILE = config-vendor-fota-race-cmd.h

# This option is to configure system log debug level.
# Usage: none, error, warning, info, debug
#              empty   : All debug logs are compiled.
#              error   : Only error logs are compiled.
#              warning : Only warning and error logs are compiled.
#              info    : Only info, warning, and error logs are compiled.
#              debug   : All debug logs are compiled.
#              none    : All debugs are disabled.
AIR_DEBUG_LEVEL = info

######  Audio peripheral ######

# This option is to configure voice prompt decoder codec type.
# Usage: none, mp3, wav, opus, all
#             all  : Support all decoder codec types, mp3/pcm/wav/opus.
#             mp3  : Support MP3 voice prompt files, default selection.
#             wav  : Support WAV voice prompt files, a-law/u-law/ms-adpcm/ima-adpcm/pure pcm.
#             opus : Support OPUS voice prompt files, 1 channel, 48 KHz, 20 ms frame size and CELT elementary stream.
#             none : Unused codec. It can still be used to play pure pcm.
AIR_PROMPT_CODEC_TYPE = mp3

# This option is to define product category.
AIR_PRODUCT_CATEGORY = AB1585_Earbuds

# This option is to configure DVFS minimum level, default is LV.
# Usage: DVFS_LV, DVFS_NV, DVFS_HV
AIR_DVFS_MIN_LEVEL = DVFS_LV

# This option is to configure DVFS maximum  level, default is HV.
# Usage: DVFS_LV, DVFS_NV, DVFS_HV
AIR_DVFS_MAX_LEVEL = DVFS_HV

###### Sensor Algo For Spatial ######

# This option is to configure spatial algo type.
# Usage: none, in_house, vendor1
#     none : Not support spatial algo.
#     in_house : Use the airoha in_house algo.
#     vendor1 : Use the third vendor algo.
AIR_SENSOR_ALGO_FOR_SPATIAL = none

#This option is used to config supported device type.
# Usage: spi_flash, esc_flash, none
#                    spi_flash  : connect Flash with SPI interface.
#                    esc_flash  : connect Flash with ESC interface.
#                         none  : not support.
AIR_HAL_EXTERNAL_FLASH_TYPE = none

#This option is used to config supported device type.
# Usage: spi_flash, esc_psram, none
#                    esc_psram  : connect PSRAM with ESC interface.
#                         none  : not support.
AIR_HAL_EXTERNAL_RAM_TYPE = none

# This option is used for customization files
# Usage: default, or other customers
AIR_SDK_CUSTOMER = default

##############################################
# Include the DSP feature makefile to inlcude the feature options are both supported in MCU and DSP
##############################################
include $(SOURCE_DIR)/../dsp/project/$(BOARD)/apps/dsp0_headset_ref_design/XT-XCC/feature_85_ull2_earbuds_full.mk
