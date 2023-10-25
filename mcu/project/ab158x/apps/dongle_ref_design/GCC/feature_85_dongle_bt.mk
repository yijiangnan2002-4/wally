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
BOARD_TYPE                            := ab1585_dongle_bt
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

######  BT/MCSync ######

# This option is used to support LE Audio broadcast.
# Dependency: AIR_LE_AUDIO_ENABLE must be enabled when this option is set to y.
AIR_LE_AUDIO_BIS_ENABLE = y

# This option is to enable LE Audio dongle multi-device scenario.
# Dependency: AIR_LE_AUDIO_DONGLE_ENABLE must be enabled when this option is set to y.
AIR_LE_AUDIO_MULTI_DEVICE_ENABLE = n

# This option is used to enable/disable volume control on the dongle side.
# It currently only works in unicast mode of LE Audio.
AIR_VOLUME_CONTROL_BY_DONGLE = n

# This option is used to enable teams.
# Dependency: AIR_USB_AUDIO_HID_ENABLE must be enabled when this option is set to y.
AIR_MS_TEAMS_ENABLE = y

# This option is to enable MS XBOX GIP feature.
# Dependency: AIR_BLE_ULTRA_LOW_LATENCY_ENABLE must be enabled when this option is set to y.
AIR_MS_GIP_ENABLE = n

######  System ######

# This option is used to enable FOTA basic function.
AIR_FOTA_ENABLE                     = y

# This option is to enable FOTA via race cmd.
# Dependency: AIR_FOTA_ENABLE, AIR_RACE_CMD_ENABLE and AIR_RACE_DUAL_CMD_ENABLE (if it is an earbuds project) must be enabled when this option is set to y.
AIR_FOTA_VIA_RACE_CMD_ENABLE = y

# This option is used to enable component firmware update module.
# Dependency: AIR_USB_AUDIO_HID_ENABLE, AIR_LE_AUDIO_ENABLE, AIR_RACE_CMD_ENABLE, AIR_FOTA_ENABLE must enable, and AIR_USB_AUDIO_VERSION cannot be none.
AIR_CFU_ENABLE = n

# This option is to enable USB module.
AIR_USB_ENABLE                        = y

# This option is to use USB RX as usage of speaker.
# Dependency: AIR_USB_ENABLE must be enabled, and AIR_USB_AUDIO_VERSION cannot be none when this option is set to y.
AIR_USB_AUDIO_SPEAKER_ENABLE          = y

# This option is to use USB TX as usage of microphone.
# Dependency: AIR_USB_ENABLE  must enable, and AIR_USB_AUDIO_VERSION cannot be none.
AIR_USB_AUDIO_MICROPHONE_ENABLE       = y

# This option is to add more one USB RX to enable second speaker.
# Dependency: AIR_USB_ENABLE  must enable, and AIR_USB_AUDIO_VERSION cannot be none.
AIR_USB_AUDIO_2ND_SPEAKER_ENABLE      = n

# This option is used to support HID (Human Interface Device).
# Dependency: AIR_USB_ENABLE  must enable.
AIR_USB_AUDIO_HID_ENABLE              = y

# This option is used to support XBOX.
# Dependency: AIR_USB_ENABLE must be enabled when this option is set to y.
AIR_USB_XBOX_ENABLE                   = n

# This option is used to enable race cmd.
AIR_RACE_CMD_ENABLE                 = y

# This option is to enable minidump feature.
AIR_MINIDUMP_ENABLE                 = n

# This option is used to enable/disable the power saving in APP.
AIR_APPS_POWER_SAVE_ENABLE = n

# This option is used to enable/disable key driver.
AIR_AIRO_KEY_ENABLE = y

# This option is used to enable/disable eint key module.
AIR_EINT_KEY_ENABLE                    = y

# This option is used to enable/disable secure boot module.
# NOTICE:
#   - This option can only be enabled if the secure-boot-addon-repo exists.
#   - secure-boot-addon-repo path: mcu\prebuilt\middleware\airoha\secure_boot
#   - If this option enabled without secure boot addon repo, the code will build fail.
AIR_SECURITY_SECURE_BOOT_ENABLE = n

# This option is used to enable/disable ultra low latency version1.0 module.
AIR_BT_ULTRA_LOW_LATENCY_ENABLE = n

# This option is used to enable/disable ultra low latency version2.0 module.
AIR_BLE_ULTRA_LOW_LATENCY_ENABLE = n

# This option is used to enable/disable BT source module.
AIR_BT_SOURCE_ENABLE = y

# This option is used to enable ULL voice uplink low latency (AFE buffering 7.5ms)
# Dependency: AIR_BT_ULTRA_LOW_LATENCY_ENABLE be enabled.
AIR_ULL_VOICE_LOW_LATENCY_ENABLE = n

# This option is used to enable/disable volume monitor.
AIR_AUDIO_VOLUME_MONITOR_ENABLE = n

## part2: need to set specified value

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
AIR_DEBUG_LEVEL                     = info

# Choose the version of UAC (USB Audio Class). Default setting is version 1.
# Dependency: AIR_USB_ENABLE must be enabled when this option is set to v1.
# Usage: v1, none
#                     none  : means disable USB Audio.
AIR_USB_AUDIO_VERSION                 = v1

# This option is to define product category.
AIR_PRODUCT_CATEGORY = AB1585_BT_DONGLE

# This option is to configure DVFS minimum level, default is LV.
# Usage: DVFS_LV, DVFS_NV, DVFS_HV
AIR_DVFS_MIN_LEVEL = DVFS_LV

# This option is to configure DVFS maximum  level, default is HV.
# Usage: DVFS_LV, DVFS_NV, DVFS_HV
AIR_DVFS_MAX_LEVEL = DVFS_HV

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
# Include the MCU DSP shared feature option checking flow
##############################################
include $(PWD)/mcu_dsp_share_option_checker.mk

##############################################
# Include the DSP feature makefile to inlcude the feature options are both supported in MCU and DSP
##############################################
include $(SOURCE_DIR)/../dsp/project/$(BOARD)/apps/dsp0_headset_ref_design/XT-XCC/feature_85_dongle_bt_source.mk
