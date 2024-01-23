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

#######################################################
# Project vairables assignment

SOURCE_DIR = ../../../../..
SDK_PATH = $(abspath $(PWD)/$(SOURCE_DIR))
PWD= $(shell pwd)
FEATURE ?= feature.mk
DEBUG = 0
FLOAT_TYPE = hard
BUILD_DIR = $(PWD)/build

# Project name
PROJ_NAME = $(shell basename $(dir $(PWD)))
PROJ_PATH = $(abspath $(dir $(PWD)))/GCC
OUTPATH = $(PWD)/build

#######################################################
# Include Module
include $(FEATURE)

#######################################################
# Not changed feature option for this project.

# Temp macro for 3in1 dongle POC
ifeq ($(BOARD_TYPE),ab1571d_evk_3in1_universal)
    CFLAGS += -DAIR_DONGLE_3IN1
endif

#BT external timer
MTK_BT_TIMER_EXTERNAL_ENABLE          = y
AIR_AIRDUMP_ENABLE                              = y
AIR_AUDIO_GAIN_SETTING_ENHANCE_ENABLE           = y
AIR_AUDIO_HW_IO_CONFIG_ENHANCE_ENABLE           = y
AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE_ENABLE    = y
ifeq ($(AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE), y)
AIR_AMP_DC_COMPENSATION_ENABLE              = y
else
AIR_AMP_DC_COMPENSATION_ENABLE              = n
endif
AIR_DC_COMPENSATION_PARA_ENABLE                 = y
AIR_DONGLE_RELAY_RACE_PACKET_ENABLE             = y
# Race relay cmd
MTK_RACE_RELAY_CMD_ENABLE            := n
AIR_SMT_SPK_TO_MIC_TEST_ENABLE                  = n
MTK_AUDIO_CODEC_MANAGER_ENABLE                  = y
MTK_AUDIO_GAIN_TABLE_ENABLE                     = y
ifeq ($(IC_TYPE),ab1627)
AIR_AUDIO_TRANSMITTER_ENABLE                    = n
else
AIR_AUDIO_TRANSMITTER_ENABLE                    = y
endif
MTK_BLE_GAP_SRV_ENABLE                          = y
AIR_BT_AT_COMMAND_ENABLE                        = y
MTK_LINEIN_INS_ENABLE                           = n
MTK_LINEIN_PLAYBACK_ENABLE                      = n
MTK_PURE_LINEIN_PLAYBACK_ENABLE                 = n
AIR_NVDM_ENABLE                                 = y
MTK_OPUS_ENCODER_ENABLE                         = y
MTK_RACE_DUAL_CMD_ENABLE                        = n
MTK_RACE_FIND_ME_ENABLE                         = n
MTK_RECORD_ENABLE                               = y
MTK_RECORD_OPUS_ENABLE                          = n
MTK_USB_AUDIO_PLAYBACK_ENABLE                   = n
MTK_WAV_DECODER_ENABLE                          = n
# heap dump
MTK_SUPPORT_HEAP_DEBUG_ENABLE                   = y
MTK_SUPPORT_HEAP_DEBUG_ADVANCED_ENABLE          = n
# heap peak profiling
MTK_HEAP_SIZE_GUARD_ENABLE                      = n
#SWLA
MTK_SWLA_ENABLE                                 = y
#system service at cmd
MTK_SYSTEM_AT_COMMAND_ENABLE                    = y
MTK_OS_CPU_UTILIZATION_ENABLE                   = y
# port service
MTK_PORT_SERVICE_ENABLE             = y
MTK_PORT_SERVICE_BT_ENABLE          = y
AIR_USB_DONGLE_PROJECT_ENABLE                   = y
MTK_MUX_BT_ENABLE                   := y
MTK_BLE_GAP_SRV_ENABLE              := y

# HW IO
AIR_AUDIO_BIAS_CONTROL_BY_TOOL        := y

MTK_BT_CM_SUPPORT                   = y
MTK_BT_ENABLE                       = y
MTK_BLE_ONLY_ENABLE                 = n
# ULL Sniff Enable
AIR_BT_ULTRA_LOW_LATENCY_IDLE_SNIFF_ENABLE      = n
AIR_GATTC_DISCOVERY_ENHANCE_ENABLE  = y
AIR_LE_AUDIO_UNICAST_ENABLE         := n
ifeq ($(AIR_LE_AUDIO_ENABLE), y)
ifneq ($(AIR_BT_AUDIO_DONGLE_ENABLE), y)
ifneq ($(AIR_DONGLE_AFE_IN_TYPE), LINE_IN_I2S_SLV_IN)
AIR_LE_AUDIO_UNICAST_ENABLE         = y
endif
else
AIR_LE_AUDIO_UNICAST_ENABLE         = y
endif
endif
ifeq ($(AIR_LE_AUDIO_DONGLE_ENABLE), n)
AIR_LE_AUDIO_ENABLE                 = n
AIR_LE_AUDIO_BIS_ENABLE             = n
AIR_LE_AUDIO_UNICAST_ENABLE         = n
endif
ifeq ($(AIR_LE_AUDIO_BIS_ENABLE), n)
ifeq ($(AIR_LE_AUDIO_UNICAST_ENABLE), n)
AIR_LE_AUDIO_ENABLE                 = n
endif
endif
ifeq ($(AIR_LE_AUDIO_ENABLE), n)
AIR_LE_AUDIO_BIS_ENABLE             = n
AIR_LE_AUDIO_UNICAST_ENABLE         = n
endif

# ULL audio 2.0 dongle
ifeq ($(AIR_ULL_AUDIO_V2_DONGLE_ENABLE), y)
CFLAGS += -DAIR_ULL_AUDIO_V2_DONGLE_ENABLE
endif

# Enhanced ATT bearer over LE Enable
AIR_LE_EATT_ENABLE                              = n
# Wireless MIC support battery
ifeq ($(AIR_WIRELESS_MIC_ENABLE), y)
ifeq ($(FPGA_ENV), y)
MTK_BATTERY_MANAGEMENT_ENABLE       = n
else
MTK_BATTERY_MANAGEMENT_ENABLE       = y
endif
endif
ifneq ($(MTK_BATTERY_MANAGEMENT_ENABLE), y)
AIR_PMU_DISABLE_CHARGER = y
endif

# Dongle disable ANC module
AIR_ANC_ENABLE_TYPE        = none
AIR_PASSTHRU_ENABLE_TYPE   = none

AIR_GATT_SRV_CLIENT_ENABLE = y

# Support MIC Loopback AT command
LINE_IN_PURE_FOR_AMIC_CLASS_G_HQA = y

# LE Audio support LC3PLUS codec
ifeq ($(AIR_AUDIO_LC3PLUS_CODEC_ENABLE)_$(AIR_LE_AUDIO_DONGLE_ENABLE), y_y)
ifeq ($(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE), n)
AIR_LE_AUDIO_LC3PLUS_ENABLE     = y
endif
endif
#######################################################

# LD Flags
LDFLAGS = $(ALLFLAGS) $(FPUFLAGS) --specs=nano.specs -lnosys -nostartfiles
LDFLAGS += -Wl,-wrap=malloc -Wl,-wrap=calloc -Wl,-wrap=realloc -Wl,-wrap=free
LDFLAGS += -Wl,--undefined=uxTopUsedPriority

ifeq ($(AIR_DEBUG_LEVEL),warning)
CFLAGS += -Wno-unused-but-set-variable
CFLAGS += -Wno-unused-variable
endif

TARGET ?= FLASH
# Select different linker script files by boot method.
# If it is FLASH boot, the script file used must be configured in feature.mk.
ifeq ($(TARGET), SYSRAM)
    MTK_BOOT_TARGET := SYSRAM
    LSCRIPT      := $(IC_CONFIG)_sysram.ld
    CFLAGS += -DBOOT_METHOD=0
else ifeq ($(TARGET), FLASH)
    MTK_BOOT_TARGET := FLASH
    ifdef AIR_MCU_LINKER_SCRIPT_FILE
        LSCRIPT      := $(AIR_MCU_LINKER_SCRIPT_FILE)
    else
        NoLinkerScript = NotSpecifiedLinkerScript
        LSCRIPT      := $(NoLinkerScript)
    endif
    CFLAGS += -DBOOT_METHOD=1
    LINKER_SCRIPT_PATH = $(SOURCE_DIR)/$(APP_PATH)/GCC/$(LSCRIPT)
endif

LDFLAGS += -Wl,-T$(LSCRIPT)
NVDM_BIN_SIZE=$(shell cat $(AIR_MCU_LINKER_SCRIPT_FILE) | grep nvdm.bin | grep -o "[0-9]*" | tail -n 1 )
NVDM_OU_BIN_SIZE=$(shell cat $(AIR_MCU_LINKER_SCRIPT_FILE) | grep nvdm_ou.bin | grep -o "[0-9]*" | tail -n 1 )

# Auto generate flash_download.cfg file
FLASH_DOWNLOAD_CFG_GENERATOR := $(SOURCE_DIR)/tools/scripts/build/auto_download_cfg.sh

ifeq ($(DEBUG_IO), 1)
LDFLAGS += --specs=rdimon.specs -lrdimon
CFLAGS += -DDEBUG_IO
endif

#######################################################
# Gloabl Config
-include $(SOURCE_DIR)/.config
# IC Config
-include $(SOURCE_DIR)/config/chip/$(IC_CONFIG)/chip.mk
# Board Config
#-include $(SOURCE_DIR)/config/board/$(BOARD_CONFIG)/board.mk
# HAL driver files
include $(SOURCE_DIR)/driver/chip/$(IC_CONFIG)/module.mk
# EPT Config
include $(SOURCE_DIR)/driver/board/$(BOARD_CONFIG)/ept/module.mk


# RTOS source files
include $(SOURCE_DIR)/kernel/rtos/$(IC_CONFIG)/FreeRTOS/module.mk

# bsp flash source files
include $(SOURCE_DIR)/driver/board/component/bsp_flash/module.mk

# rofs source files
ifneq ($(wildcard $(strip $(SOURCE_DIR))/middleware/airoha/rofs/module.mk),)
include $(SOURCE_DIR)/middleware/airoha/rofs/module.mk
endif

# kernel service files
include $(SOURCE_DIR)/kernel/service/module.mk

# Enhanced ATT bearer over LE Enable
ifeq ($(AIR_LE_EATT_ENABLE), y)
CFLAGS += -DBT_EATT_ENABLE
endif

# NVDM files
ifeq ($(AIR_NVDM_ENABLE),y)
include $(SOURCE_DIR)/middleware/airoha/nvdm/module.mk
CFLAGS += -I$(SOURCE_DIR)/prebuilt/middleware/airoha/nvdm/$(IC_CONFIG)/inc
endif
# add USB CLASS files
ifeq ($(AIR_USB_ENABLE), y)
include $(SOURCE_DIR)/middleware/airoha/usb/module.mk
include $(SOURCE_DIR)/middleware/airoha/usb_hid_service/module.mk
endif

ifeq ($(MTK_PORT_SERVICE_ENABLE), y)
# add PORT SERVICE files
include $(SOURCE_DIR)/middleware/airoha/port_service/module.mk
ifeq ($(MTK_PORT_SERVICE_BT_ENABLE),y)
include $(SOURCE_DIR)/middleware/airoha/bt_air/GCC/module.mk
endif
endif
ifeq ($(AIR_REBOOT_AFTER_EXCEPTION_ENABLE), y)
CFLAGS += -DREBOOT_AFTER_EXCEPTION
endif
ifeq ($(LOG_OUTPUT_FROM_USB), y)
CFLAGS += -DLOG_OUTPUT_FROM_USB
endif
ifeq ($(SPECIAL_DEFINE_FOR_MINI_BOARD), y)
CFLAGS += -DSPECIAL_DEFINE_FOR_MINI_BOARD
endif

ifeq ($(AIR_USB_DONGLE_PROJECT_ENABLE), y)
CFLAGS += -DAIR_USB_DONGLE_PROJECT_ENABLE
endif

ifeq ($(MTK_MUX_BT_ENABLE),y)
CFLAGS += -DMTK_MUX_BT_ENABLE
endif

ifeq ($(AIR_RACE_CMD_ENABLE), y)
ifeq ($(AIR_RACE_FIND_ME_ENABLE), y)
CFLAGS += -DRACE_FIND_ME_ENABLE
endif
include $(SOURCE_DIR)/middleware/airoha/race_cmd/module.mk
endif

ifeq ($(AIR_BT_ROLE_HANDOVER_SERVICE_ENABLE), y)
CFLAGS += -DSUPPORT_ROLE_HANDOVER_SERVICE
# bt bt_role_handover_service
include $(SOURCE_DIR)/middleware/airoha/bt_role_handover/module.mk
endif

ifeq ($(MTK_AWS_MCE_ROLE_RECOVERY_ENABLE), y)
CFLAGS += -DMTK_AWS_MCE_ROLE_RECOVERY_ENABLE
endif

#CFU module
ifeq ($(AIR_CFU_ENABLE),y)
CFLAGS += -DAIR_CFU_BUILDER_ENABLE
endif

# Dongle relay RACE packet
ifeq ($(AIR_DONGLE_RELAY_RACE_PACKET_ENABLE), y)
CFLAGS += -DAIR_DONGLE_RELAY_RACE_PACKET_ENABLE
endif

# wireless mic feature
ifeq ($(AIR_WIRELESS_MIC_ENABLE), y)
CFLAGS += -DAIR_WIRELESS_MIC_ENABLE
CFLAGS += -DAIR_WIRELESS_MIC_RX_ENABLE
endif

# ULL V2 lc3plus codec option
ifeq ($(AIR_AUDIO_LC3PLUS_CODEC_ENABLE), y)
CFLAGS += -DAIR_AUDIO_LC3PLUS_CODEC_ENABLE
endif

# ULL V2 ULD codec option
ifeq ($(AIR_AUDIO_ULD_CODEC_ENABLE), y)
CFLAGS += -DAIR_AUDIO_ULD_CODEC_ENABLE
endif

# ULL V2 opus codec option
ifeq ($(AIR_AUDIO_VEND_CODEC_ENABLE), y)
CFLAGS += -DAIR_AUDIO_VEND_CODEC_ENABLE
endif

# ULL V2 opus codec option
ifeq ($(AIR_AUDIO_SILENCE_DETECTION_ENABLE), y)
CFLAGS += -DAIR_AUDIO_SILENCE_DETECTION_ENABLE
endif

# ULL Audio V3 option
ifeq ($(AIR_ULL_AUDIO_V3_ENABLE), y)
CFLAGS += -DAIR_ULL_AUDIO_V3_ENABLE
endif

# Pure gaming(exclude DSP) option.
ifeq ($(AIR_PURE_GAMING_ENABLE), y)
CFLAGS += -DAIR_PURE_GAMING_ENABLE

# HID PLC module
include $(SOURCE_DIR)/middleware/airoha/hid_plc/GCC/module.mk

# Pure gaming for mouse option.
ifeq ($(AIR_PURE_GAMING_MS_ENABLE), y)
CFLAGS += -DAIR_PURE_GAMING_MS_ENABLE
ifeq ($(AIR_NVIDIA_REFLEX_ENABLE), y)
CFLAGS += -DAIR_NVIDIA_REFLEX_ENABLE
endif
endif

# Pure gaming for keyboard option.
ifeq ($(AIR_PURE_GAMING_KB_ENABLE), y)
CFLAGS += -DAIR_PURE_GAMING_KB_ENABLE
endif

# Pure gaming for mouse and keyboard option.
ifeq ($(AIR_PURE_GAMING_MS_KB_ENABLE), y)
CFLAGS += -DAIR_PURE_GAMING_MS_KB_ENABLE
endif

# This option is used to enable QC mouse.
ifeq ($(AIR_QC_DONGLE_MS_ENABLE), y)
CFLAGS += -DAIR_QC_DONGLE_MS_ENABLE
CFLAGS += -DAIR_PURE_GAMING_MS_ENABLE
endif

# This option is used to enable QC keyboard.
ifeq ($(AIR_QC_DONGLE_KB_ENABLE), y)
CFLAGS += -DAIR_QC_DONGLE_KB_ENABLE
CFLAGS += -DAIR_PURE_GAMING_KB_ENABLE
endif

endif

# HID HOGP feature option.
ifeq ($(AIR_HID_BT_HOGP_ENABLE), y)
include $(SOURCE_DIR)/middleware/airoha/bt_hogp/module.mk
CFLAGS += -DAIR_HID_BT_HOGP_ENABLE

# HID HOGP vendor support feature option.
ifeq ($(AIR_HID_HOGP_VENDOR_SUPPORT_ENABLE), y)
CFLAGS += -DAIR_HID_HOGP_VENDOR_SUPPORT_ENABLE
endif

endif

# Bluetooth module
include $(SOURCE_DIR)/middleware/airoha/bluetooth/module.mk

# BT callback manager
include $(SOURCE_DIR)/middleware/airoha/bt_callback_manager/module.mk

# BT connection manager
include $(SOURCE_DIR)/middleware/airoha/bt_connection_manager/module.mk

# bt components le module
include $(SOURCE_DIR)/middleware/airoha/bluetooth_service/module.mk

# ble battery service module
#include $(SOURCE_DIR)/middleware/airoha/ble_bas/module.mk

# IAS service module
ifeq ($(MTK_BLE_IAS), y)
#include $(SOURCE_DIR)/middleware/airoha/ble_ias/module.mk
endif

# DIS service module
include $(SOURCE_DIR)/middleware/airoha/ble_dis/module.mk

#AWS MCE Report
ifeq ($(MTK_AWS_MCE_ENABLE), y)
include $(SOURCE_DIR)/middleware/airoha/bt_aws_mce_report/module.mk
endif
# BT Sink service module
include $(SOURCE_DIR)/middleware/airoha/sink/module.mk

# ATCI module
include $(SOURCE_DIR)/middleware/airoha/atci/module.mk

# Audio module
ifneq ($(wildcard $(strip $(SOURCE_DIR))/middleware/airoha/audio/port/$(IC_CONFIG)/module.mk),)
include $(SOURCE_DIR)/middleware/airoha/audio/port/$(IC_CONFIG)/module.mk
endif

# audio middleware files
ifneq ($(wildcard $(strip $(SOURCE_DIR))/middleware/airoha/audio/module.mk),)
include $(SOURCE_DIR)/middleware/airoha/audio/module.mk
endif

# include the nanopb module.mk
ifeq ($(findstring y,$(AIR_AMA_ENABLE) $(AIR_GSOUND_ENABLE)),y)
include $(SOURCE_DIR)/middleware/third_party/nanopb/module.mk
endif

# ANC module
ifeq ($(AIR_ANC_ENABLE_TYPE),none)
ifeq ($(AIR_PASSTHRU_ENABLE_TYPE),none)
AIR_ANC_ENABLE                     ?= n
else
AIR_ANC_ENABLE                     ?= y
endif
else
AIR_ANC_ENABLE                     ?= y
ifeq ($(AIR_ANC_ENABLE_TYPE),ANC_FULL_ADAPTIVE)
AIR_FULL_ADAPTIVE_ANC_ENABLE       ?= y
else
AIR_FULL_ADAPTIVE_ANC_ENABLE       ?= n
endif
ifeq ($(AIR_PASSTHRU_ENABLE_TYPE),PASSTHRU_HYBRID)
AIR_HYBRID_PT_ENABLE               ?= y
else
AIR_HYBRID_PT_ENABLE               ?= n
endif
endif

ifeq ($(AIR_ANC_ENABLE), y)
ifeq ($(MTK_ANC_LIB_ENABLE), y)
include $(SOURCE_DIR)/prebuilt/middleware/airoha/audio/$(IC_CONFIG)/anc/module.mk
else
include $(SOURCE_DIR)/middleware/airoha/audio/anc_control_protected/GCC/module.mk
endif
endif

AIR_DONGLE_ENABLE = y
ifeq ($(AIR_DONGLE_ENABLE), y)
CFLAGS += -DAIR_DONGLE_ENABLE
endif

AIR_PMU_HQA_SLT_VOLT_ENABLE = n
ifeq ($(AIR_PMU_HQA_SLT_VOLT_ENABLE), y)
CFLAGS += -DAIR_PMU_HQA_SLT_VOLT_ENABLE
endif

ifeq ($(AIR_BTA_IC_STEREO_HIGH_G3),y)
ifeq ($(AIR_PMU_1BUCK_ENABLE), y)
CFLAGS += -DAIR_PMU_1BUCK_ENABLE
endif
endif

ifeq ($(BOARD_TYPE),ab1565_wireless_mic)
CFLAGS += -DAIR_WIRELESS_MIC_DEMO_BOARD_ENABLE
endif



# isink led files
include $(SOURCE_DIR)/driver/board/component/bsp_led/module.mk
CFLAGS += -DLED_ENABLE
CFLAGS += -DAIR_LED_ENABLE
# fota related files
ifeq ($(AIR_FOTA_ENABLE), y)
include $(SOURCE_DIR)/middleware/airoha/fota/module.mk
endif

include $(SOURCE_DIR)/middleware/third_party/mbedtls/module.mk

# Dongle afe in support internal charger
AIR_DONGLE_AFE_IN_TYPE              ?= NONE
ifneq ($(AIR_DONGLE_AFE_IN_TYPE), NONE)
ifeq ($(FPGA_ENV), y)
MTK_BATTERY_MANAGEMENT_ENABLE       = n
else
MTK_BATTERY_MANAGEMENT_ENABLE       = y
ifeq ($(AIR_BTA_IC_STEREO_HIGH_G3),y)
AIR_PMU_MANAGEMENT_ENABLE           = y
else
AIR_PMU_DISABLE_CHARGER             = n
endif
endif
endif

ifeq ($(AIR_PMU_DISABLE_CHARGER), y)
CFLAGS += -DAIR_PMU_DISABLE_CHARGER
endif

ifeq ($(MTK_BATTERY_MANAGEMENT_ENABLE), y)
include $(SOURCE_DIR)/middleware/airoha/battery_management/module.mk
endif

ifeq ($(AIR_PMU_MANAGEMENT_ENABLE), y)
include $(SOURCE_DIR)/middleware/airoha/pmu_management/module.mk
endif

ifeq ($(AIR_AIRO_KEY_ENABLE), y)
ifeq ($(AIR_EINT_KEY_ENABLE), y)
include $(SOURCE_DIR)/driver/board/component/bsp_eint_key/module.mk
endif
include $(SOURCE_DIR)/middleware/airoha/key/module.mk
endif

ifeq ($(MTK_BATTERY_EOC_ENTER_RTC),y)
CFLAGS += -DMTK_BATTERY_EOC_ENTER_RTC
endif

ifeq ($(MTK_HCI_LOG_MIX_WITH_SYSLOG), y)
CFLAGS += -DHCI_LOG_MIX_WITH_SYSLOG
endif

ifeq ($(MTK_BT_ENABLE),y)
include $(SOURCE_DIR)/middleware/third_party/micro_ecc/module.mk
endif

# ui shell
include $(SOURCE_DIR)/middleware/airoha/ui_shell/module.mk

# Add for ms_teams
TEAMS_LIB = $(strip $(SOURCE_DIR))/middleware/airoha/ms_teams/module.mk
ifneq ($(TEAMS_LIB), $(wildcard $(TEAMS_LIB)))
AIR_MS_TEAMS_ENABLE = n
endif
ifeq ($(AIR_MS_TEAMS_ENABLE), y)
include $(SOURCE_DIR)/middleware/airoha/ms_teams/module.mk
endif

# BT Ultra Low Latency
ifeq ($(findstring y,$(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) $(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) $(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)),y)
ifneq ($(wildcard $(strip $(SOURCE_DIR))/middleware/airoha/bt_ultra_low_latency),)
ifeq ($(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE),y)
CFLAGS += -DAIR_BLE_ULTRA_LOW_LATENCY_COMMON_ENABLE
endif
include $(SOURCE_DIR)/middleware/airoha/bt_ultra_low_latency/module.mk
endif
endif

# Audio source control
ifneq ($(wildcard $(strip $(SOURCE_DIR))/middleware/airoha/audio_source_control/module.mk),)
include $(SOURCE_DIR)/middleware/airoha/audio_source_control/module.mk
endif

# MS CFU.
MS_CFU_CODE = $(strip $(SOURCE_DIR))/middleware/airoha/ms_cfu/module.mk
ifneq ($(MS_CFU_CODE), $(wildcard $(MS_CFU_CODE)))
AIR_CFU_ENABLE = n
endif
ifeq ($(AIR_CFU_ENABLE), y)
include $(SOURCE_DIR)/middleware/airoha/ms_cfu/module.mk
endif

# LE Audio
ifeq ($(AIR_LE_AUDIO_ENABLE), y)
LE_AUDIO_LIB = $(strip $(SOURCE_DIR))/middleware/airoha/bt_le_audio/module.mk
ifeq  ($(LE_AUDIO_LIB), $(wildcard $(LE_AUDIO_LIB)))
CFLAGS += -DBT_LE_AUDIO_ENABLE
CFLAGS += -DAIR_LE_AUDIO_ENABLE
CFLAGS += -DAIR_LE_AUDIO_CIS_ENABLE
CFLAGS += -DAIR_BT_CODEC_BLE_ENABLED

# LC3plus
ifeq ($(AIR_LE_AUDIO_LC3PLUS_ENABLE), y)
CFLAGS += -DAIR_LE_AUDIO_LC3PLUS_ENABLE
endif

# BLE audio dongle
ifeq ($(AIR_LE_AUDIO_DONGLE_ENABLE), y)
CFLAGS += -DAIR_BLE_AUDIO_DONGLE_ENABLE
endif

ifeq ($(AIR_LE_AUDIO_BIS_ENABLE), y)
CFLAGS += -DAIR_LE_AUDIO_BIS_ENABLE
endif

ifeq ($(AIR_LE_AUDIO_TMAP_ENABLE), y)
CFLAGS += -DAIR_LE_AUDIO_TMAP_ENABLE
endif

ifeq ($(AIR_LE_AUDIO_HAP_ENABLE), y)
CFLAGS += -DAIR_LE_AUDIO_HAPC_ENABLE
endif

ifeq ($(AIR_LE_AUDIO_GMAP_ENABLE), y)
CFLAGS += -DAIR_LE_AUDIO_GMAP_ENABLE
endif

ifeq ($(AIR_LE_AUDIO_AURACAST_ENABLE), y)
CFLAGS += -DAIR_LE_AUDIO_PBP_ENABLE
endif

ifeq ($(AIR_LE_AUDIO_UNICAST_ENABLE), y)
CFLAGS += -DAIR_LE_AUDIO_UNICAST_ENABLE
# BLE audio dongle multi-device
ifeq ($(AIR_LE_AUDIO_MULTI_DEVICE_ENABLE), y)
CFLAGS += -DAIR_LE_AUDIO_MULTI_DEVICE_ENABLE
endif
endif

ifeq ($(AIR_VOLUME_CONTROL_BY_DONGLE), y)
CFLAGS += -DAIR_VOLUME_CONTROL_BY_DONGLE
endif

ifeq ($(AIR_LE_AUDIO_DONGLE_ENABLE)_$(AIR_LE_AUDIO_UNICAST_ENABLE)_$(AIR_LE_AUDIO_BIS_ENABLE),y_y_y)
CFLAGS += -DAIR_LE_AUDIO_BA_ENABLE
endif

include $(SOURCE_DIR)/middleware/airoha/bt_le_audio/module.mk
endif
endif

# BT Source
ifeq ($(AIR_BT_AUDIO_DONGLE_ENABLE), y)
AIR_SOURCE_SRV_HFP_ENABLE = y
AIR_SOURCE_SRV_MUSIC_ENABLE = y

CFLAGS += -DAIR_BT_SOURCE_ENABLE
ifeq ($(wildcard $(strip $(SOURCE_DIR))/middleware/airoha/bt_source/),)
    $(error "Error: [Addon feature option fail] The source addon package doesn't exist. Please contact Airoha to get this addon package.")
    @echo "[Addon feature option fail] The source addon package doesn't exist. Please contact Airoha to get this addon package.";
    @set -e; exit 2;
else
include $(SOURCE_DIR)/middleware/airoha/bt_source/module.mk
endif
endif

# Game chat volume smart balance feature
ifeq ($(AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE), y)
CFLAGS += -DAIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
endif

# MS GIP.
MS_GIP_CODE = $(strip $(SOURCE_DIR))/middleware/airoha/ms_gip/module.mk
ifneq ($(MS_GIP_CODE), $(wildcard $(MS_GIP_CODE)))
AIR_MS_GIP_ENABLE = n
endif
ifeq ($(AIR_MS_GIP_ENABLE), y)
CFLAGS += -DAIR_MS_GIP_ENABLE
include $(SOURCE_DIR)/middleware/airoha/ms_gip/module.mk
endif

ifeq ($(AIR_WIRED_AUDIO_ENABLE), y)
include $(SOURCE_DIR)/driver/board/component/bsp_audio_ext_codec/module.mk
endif

# bt_audio_common path
ifneq ($(wildcard $(strip $(SOURCE_DIR))/project/ab157x_ab158x/),)
APP_COMMON_PATH := project/ab157x_ab158x/apps/bt_audio_common
endif

ifneq ($(wildcard $(strip $(SOURCE_DIR))/project/ab15xx/),)
APP_COMMON_PATH := project/ab15xx/apps/bt_audio_common
endif


# This option is used to enable microphone bias control by Airoha tool for AB157x SDK3.10.0
#  y: enable config tool bias configuration
#  n: disable tool configuration and configure by SW code
# Please check the bias configuration by Airoha Tool if the option is enabled
ifeq ($(AIR_AUDIO_BIAS_CONTROL_BY_TOOL), y)
CFLAGS += -DAIR_AUDIO_BIAS_CONTROL_BY_TOOL
endif

#######################################################
# Main APP files
APP_PATH        := $(patsubst $(SDK_PATH)/%,%,$(abspath $(dir $(PWD))))
APP_PATH_SRC    := $(APP_PATH)/src
APP_COMMON_PATH := project/$(BOARD)/apps/bt_audio_common
APP_COMMON_SRC  := $(APP_COMMON_PATH)/src

include $(SOURCE_DIR)/$(APP_PATH_SRC)/apps/module.mk

# Config bin path
CONFIG_BIN_PATH := $(SOURCE_DIR)/$(APP_PATH)/config_bin/$(BOARD_TYPE)

# customer file path
ifneq ($(AIR_SDK_CUSTOMER), default)
ifneq ($(wildcard $(strip $(SOURCE_DIR)/customer)),)
include $(SOURCE_DIR)/customer/$(AIR_SDK_CUSTOMER)/project/chips/apps/bt_audio_common/src/apps/module.mk
endif
endif

##############################################################################
#
# SDK source files
#
##############################################################################
APP_FILES      += $(APP_PATH_SRC)/main.c
APP_FILES      += $(APP_PATH)/GCC/syscalls.c
APP_FILES      += $(APP_PATH_SRC)/regions_init.c
APP_FILES      += $(APP_PATH_SRC)/sys_init.c
APP_FILES      += $(APP_PATH_SRC)/boards/$(BOARD_TYPE)/ept_gpio_var.c
APP_FILES      += $(APP_PATH_SRC)/boards/$(BOARD_TYPE)/ept_eint_var.c
APP_FILES      += $(APP_PATH_SRC)/task_def.c
APP_FILES      += $(APP_PATH_SRC)/bt_init.c
APP_FILES      += $(APP_PATH_SRC)/bt_app_utility/hci_log.c
APP_FILES      += $(APP_PATH_SRC)/dcxo_capid.c
APP_FILES      += $(APP_PATH_SRC)/nvdm_config.c
APP_FILES      += $(APP_PATH_SRC)/nvdm_config_factory_reset.c
APP_FILES      += $(APP_PATH_SRC)/bt_app_utility/bt_power_on_config.c

APP_FILES      += $(APP_PATH_SRC)/bt_app_utility/gatt_service.c \
                  $(APP_PATH_SRC)/bt_app_utility/bt_app_common.c \
                  $(APP_PATH_SRC)/bt_app_utility/bt_app_common_at_cmd.c \
                  $(APP_PATH_SRC)/bt_app_utility/bt_app_dis.c \
                  $(APP_PATH_SRC)/bt_app_utility/bt_customer_config.c

ifeq ($(AIR_RACE_CMD_ENABLE), y)
APP_FILES      += $(APP_PATH_SRC)/race/race_app_aws_event_hdl.c \
                  $(APP_PATH_SRC)/race/race_app_race_event_hdl.c \
                  $(APP_PATH_SRC)/race/race_app_bt_event_hdl.c
endif

SYS_FILES      += $(APP_PATH_SRC)/system_cmsis.c

C_FILES        += $(APP_FILES) $(SYS_FILES)
S_FILES        += $(APP_PATH)/GCC/startup_$(IC_CONFIG).s

APP_FILES      += $(APP_PATH_SRC)/bt_app_utility/low_power_demo/bt_at_cmd.c

ifeq ($(BT_ROLE_HANDOVER_WITH_SPP_BLE), y)
CFLAGS += -DBT_ROLE_HANDOVER_WITH_SPP_BLE
endif

ifeq ($(origin AIR_SDK_CUSTOMER), undefined)
    AIR_SDK_CUSTOMER := default
endif

# Add IA2P profile support
ifeq ($(MTK_IAP2_PROFILE_ENABLE),y)
CFLAGS += -DMTK_IAP2_PROFILE_ENABLE
APP_FILES      += $(APP_PATH_SRC)/iap2/iAP2_config.c \
                  $(APP_PATH_SRC)/iap2/iAP2_cust_$(AIR_SDK_CUSTOMER).c
include $(SOURCE_DIR)/middleware/airoha/iap2/GCC/module.mk
include $(SOURCE_DIR)/middleware/airoha/mfi_coprocessor/module.mk

ifeq ($(MTK_IAP2_VIA_MUX_ENABLE),y)
CFLAGS += -DMTK_IAP2_VIA_MUX_ENABLE
endif
endif

ifeq ($(AIR_MULTI_POINT_ENABLE),y)
CFLAGS += -DAIR_MULTI_POINT_ENABLE
endif


#C_FILES += $(APP_PATH_SRC)/ble_bas_app/ble_bas_app.c
#C_FILES += $(APP_PATH_SRC)/ble_find_me_server/ble_find_me_server.c


##############################################################################
#
# SDK object files
#
##############################################################################


C_OBJS          = $(C_FILES:%.c=$(BUILD_DIR)/%.o)
CXX_OBJS        = $(CXX_FILES:%.cpp=$(BUILD_DIR)/%.o)
S_OBJS          = $(S_FILES:%.s=$(BUILD_DIR)/%.o)
CXX_OBJS        = $(CXX_FILES:%.cpp=$(BUILD_DIR)/%.o)

# TODO: need to include secure world function *.o
ifneq ($(AIR_LIMIT_TZ_ENABLE),n)
    ifeq ($(TARGET), SYSRAM)
    C_OBJS     += $(SOURCE_DIR)/prebuilt/middleware/airoha/security_fw/$(IC_CONFIG)/veneer_table.o
    else ifeq ($(TARGET), FLASH)
    C_OBJS     += $(SOURCE_DIR)/prebuilt/middleware/airoha/security_fw_rom/$(IC_CONFIG)/veneer_table.o
    endif
endif

# security compile option
ifeq ($(AIR_BTA_IC_PREMIUM_G3),y)
ifeq ($(AIR_LIMIT_TZ_ENABLE),n)
CFLAGS += -mcmse
endif
endif
#######################################################
# Include path

CFLAGS += -I$(SOURCE_DIR)/$(APP_PATH)/inc
CFLAGS += -I$(SOURCE_DIR)/driver/CMSIS/Source/$(IC_CONFIG)/Include
CFLAGS += -I$(SOURCE_DIR)/driver/CMSIS/Device/airoha/$(IC_CONFIG)/Include
CFLAGS += -I$(SOURCE_DIR)/$(APP_PATH)/inc/bt_app_utility
#CFLAGS += -I$(SOURCE_DIR)/$(APP_PATH)/inc/ble_find_me_server
#CFLAGS += -I$(SOURCE_DIR)/$(APP_PATH)/inc/ble_bas_app
CFLAGS += -I$(SOURCE_DIR)/middleware/airoha/audio_manager/inc
ifeq ($(AIR_RACE_CMD_ENABLE), y)
CFLAGS += -I$(SOURCE_DIR)/$(APP_PATH)/inc/race
endif
ifeq ($(AIR_PROMPT_SOUND_ENABLE), y)
CFLAGS += -I$(SOURCE_DIR)/$(APP_PATH)/inc/voice_tone
endif
CFLAGS += -I$(SOURCE_DIR)/$(APP_PATH)/inc/boards/$(BOARD_TYPE)
CFLAGS += -I$(SOURCE_DIR)/$(APP_PATH)/src/boards/$(BOARD_TYPE)

ifeq ($(MTK_IAP2_PROFILE_ENABLE), y)
CFLAGS += -I$(SOURCE_DIR)/$(APP_PATH)/inc/iap2
endif

######################################################
# Transfer the feature option to the LM generator
# Check AMA and AMA Hotword enabled or not
AMA_HOTWORD_ENABLED = n
GVA_HOTWORD_ENABLED = n
ifeq ($(AIR_AMA_ENABLE)_$(AIR_AMA_HOTWORD_ENABLE),y_y)
AMA_HOTWORD_ENABLED = y
endif
# Check GVA and GVA Hotword enabled or not
ifeq ($(AIR_GSOUND_ENABLE)_$(AIR_GSOUND_HOTWORD_ENABLE),y_y)
GVA_HOTWORD_ENABLED = y
endif

ifeq ($(findstring y,$(GVA_HOTWORD_ENABLED) $(AMA_HOTWORD_ENABLED)),y)
AIR_VA_MODEL_MANAGER_ENABLE = y
include $(SOURCE_DIR)/middleware/airoha/va_model_manager/module.mk
CFLAGS += -DAIR_VA_MODEL_MANAGER_ENABLE
endif

#######################################################
# Rules

.PHONY: proj clean $(MODULE_PATH)

all: cleanlog checkoption proj
	@mkdir -p $(BUILD_DIR)
	@$(SIZE) $(OUTPATH)/$(PROJ_NAME).elf
	@if [ -e $(CONFIG_BIN_PATH)/nvdm.bin ]; then \
		cp -f $(CONFIG_BIN_PATH)/nvdm.bin $(OUTPATH)/; \
	else \
		$(SOURCE_DIR)/$(POSTBUILD) -f nv2bin -size $(NVDM_OU_BIN_SIZE) -o $(OUTPATH)/ -s "$(SOURCE_DIR)/config/chip/$(IC_CONFIG)/nvkey.xml,$(CONFIG_BIN_PATH)/nvkey.xml" -mode "1" >&2; \
		if [ "$$?" != "0" ]; then \
			echo; echo "error: Failed to generate nvdm binary file in mode 1"; echo; \
			set -e; exit 1; \
		else \
			echo; echo "Succeeded to generate nvdm binary file in mode 1"; echo; \
		fi; \
		$(SOURCE_DIR)/$(POSTBUILD) -f nv2bin -size $(NVDM_BIN_SIZE) -o $(OUTPATH)/ -s "$(SOURCE_DIR)/config/chip/$(IC_CONFIG)/nvkey.xml,$(CONFIG_BIN_PATH)/nvkey.xml" -mode "2" >&2; \
		if [ "$$?" != "0" ]; then \
			echo; echo "error: Failed to generate nvdm binary file in mode 2"; echo; \
			set -e; exit 1; \
		else \
			echo "Succeeded to generate nvdm binary file in mode 2"; echo; \
		fi; \
	fi
	@cp -f $(SOURCE_DIR)/config/chip/$(IC_CONFIG)/nvkey.xml $(OUTPATH)/nvkey_chip.xml
	@cp -f $(CONFIG_BIN_PATH)/nvkey.xml $(OUTPATH)/

	# For generate the language model binary file and copy the binary file to the output path.
	@if [ "$(AIR_VA_MODEL_MANAGER_ENABLE)" = "y" ]; then \
		echo; \
		echo "===============================================================" >&2; \
		echo "= Start to generate language model binary" >&2; \
		echo "===============================================================" >&2; \
		cp $(CONFIG_BIN_PATH)/language_model/config.xml $(OUTPATH)/lm_temp_config.xml; \
		sed -i 's#ama#&\/${IC_CONFIG}#' $(OUTPATH)/lm_temp_config.xml; \
		sed -i 's#gsound#&\/${IC_CONFIG}#' $(OUTPATH)/lm_temp_config.xml; \
		$(SOURCE_DIR)/$(LM_BIN_GENERATOR) -i $(OUTPATH)/lm_temp_config.xml -o $(OUTPATH) -l $(AIR_MCU_LINKER_SCRIPT_FILE) -a $(AMA_HOTWORD_ENABLED) -g $(GVA_HOTWORD_ENABLED) >&2; \
		if [ "$$?" != "0" ]; then \
			echo; echo "error: Failed to generate language model binary" >&2; echo; \
			set -e; exit 1; \
		fi; \
		echo "===============================================================" >&2; \
		echo "= Generate language model binary finished" >&2; \
		echo "===============================================================" >&2; \
		echo; \
	fi

	@cp -f $(CONFIG_BIN_PATH)/filesystem.bin $(OUTPATH)/
	@if [ "$(TARGET)" = "FLASH" ]; then $(FLASH_DOWNLOAD_CFG_GENERATOR) $(LINKER_SCRIPT_PATH) $(OUTPATH) $(IC_CONFIG) MCU_FW; fi
	@$(SOURCE_DIR)/tools/scripts/build/copy_firmware.sh $(SOURCE_DIR) $(OUTPATH) $(IC_CONFIG) $(BOARD_CONFIG) $(PROJ_NAME).bin $(PWD)
	@$(SOURCE_DIR)/tools/scripts/build/rofs_size_overflow.sh  ../../$(APP_PATH)/config_bin/$(BOARD_TYPE)/filesystem.bin ../../$(APP_PATH)/GCC/$(LSCRIPT) >&2; \
	if [ "$$?" != "0" ]; then \
		echo; echo "error: rots overflow"; echo; \
		set -e; exit 1; \
	fi

MOD_EXTRA = BUILD_DIR=$(BUILD_DIR) OUTPATH=$(OUTPATH) PROJ_PATH=$(PROJ_PATH) IC_CONFIG=$(IC_CONFIG)

checkoption:
# Check MCU include shared option or not
# Shared optin is a option that MCU and DSP are both used, but should ONLY defined in DSP feature.mk
	$(SOURCE_DIR)/tools/scripts/build/shared_option_checker.sh $(SOURCE_DIR) $(BOARD) $(FEATURE) >&2

# USB : check the dependency of USB Audio/HID option
ifneq ($(AIR_USB_AUDIO_VERSION),none)
ifneq ($(AIR_USB_ENABLE),y)
	$(warning "Error: [Conflict feature option] To enable AIR_USB_AUDIO_VERSION must support AIR_USB_ENABLE")
	@echo "Error: [Conflict feature option] To enable AIR_USB_AUDIO_VERSION must support AIR_USB_ENABLE";
	@set -e; exit 2;
endif
endif

ifeq ($(AIR_USB_AUDIO_MICROPHONE_ENABLE),y)
ifneq ($(AIR_USB_ENABLE),y)
	$(warning "Error: [Conflict feature option] To enable AIR_USB_AUDIO_MICROPHONE_ENABLE must support AIR_USB_ENABLE")
	@echo "Error: [Conflict feature option] To enable AIR_USB_AUDIO_MICROPHONE_ENABLE must support AIR_USB_ENABLE";
	@set -e; exit 2;
endif
ifeq ($(AIR_USB_AUDIO_VERSION),none)
	$(warning "Error: [Conflict feature option] To enable AIR_USB_AUDIO_MICROPHONE_ENABLE must support AIR_USB_AUDIO_VERSION")
	@echo "Error: [Conflict feature option] To enable AIR_USB_AUDIO_MICROPHONE_ENABLE must support AIR_USB_AUDIO_VERSION";
	@set -e; exit 2;
endif
endif

ifeq ($(AIR_USB_AUDIO_2ND_SPEAKER_ENABLE),y)
ifneq ($(AIR_USB_ENABLE),y)
	$(warning "Error: [Conflict feature option] To enable AIR_USB_AUDIO_2ND_SPEAKER_ENABLE must support AIR_USB_ENABLE")
	@echo "Error: [Conflict feature option] To enable AIR_USB_AUDIO_2ND_SPEAKER_ENABLE must support AIR_USB_ENABLE";
	@set -e; exit 2;
endif
ifeq ($(AIR_USB_AUDIO_VERSION),none)
	$(warning "Error: [Conflict feature option] To enable AIR_USB_AUDIO_2ND_SPEAKER_ENABLE must support AIR_USB_AUDIO_VERSION")
	@echo "Error: [Conflict feature option] To enable AIR_USB_AUDIO_2ND_SPEAKER_ENABLE must support AIR_USB_AUDIO_VERSION";
	@set -e; exit 2;
endif
endif

ifeq ($(AIR_USB_AUDIO_HID_ENABLE),y)
ifneq ($(AIR_USB_ENABLE),y)
	$(warning "Error: [Conflict feature option] To enable AIR_USB_AUDIO_HID_ENABLE must support AIR_USB_ENABLE")
	@echo "Error: [Conflict feature option] To enable AIR_USB_AUDIO_HID_ENABLE must support AIR_USB_ENABLE";
	@set -e; exit 2;
endif
endif

ifeq ($(AIR_USB_MFI_ENABLE),y)
ifneq ($(AIR_USB_ENABLE),y)
	$(warning "Error: [Conflict feature option] To enable AIR_USB_MFI_ENABLE must support AIR_USB_ENABLE")
	@echo "Error: [Conflict feature option] To enable AIR_USB_MFI_ENABLE must support AIR_USB_ENABLE";
	@set -e; exit 2;
endif
endif

# CFU : check the dependency of CFU option
ifeq ($(AIR_CFU_ENABLE),y)
ifneq ($(AIR_FOTA_ENABLE),y)
	$(warning "Error: [Conflict feature option] To enable AIR_CFU_ENABLE must support AIR_FOTA_ENABLE")
	@echo "Error: [Conflict feature option] To enable AIR_CFU_ENABLE must support AIR_FOTA_ENABLE";
	@set -e; exit 2;
endif
endif

# fota related files
ifeq ($(AIR_FOTA_VIA_RACE_CMD_ENABLE),y)
ifneq ($(AIR_FOTA_ENABLE)_$(AIR_RACE_CMD_ENABLE),y_y)
	$(warning "Error: [Conflict feature option] To enable AIR_FOTA_VIA_RACE_CMD_ENABLE must support AIR_FOTA_ENABLE/AIR_RACE_CMD_ENABLE")
	@echo "Error: [Conflict feature option] To enable AIR_FOTA_VIA_RACE_CMD_ENABLE must support AIR_FOTA_ENABLE/AIR_RACE_CMD_ENABLE";
	@set -e; exit 2;
endif
endif

ifeq ($(AIR_BT_ULTRA_LOW_LATENCY_IDLE_SNIFF_ENABLE), y)
ifneq ($(AIR_BT_ULTRA_LOW_LATENCY_ENABLE), y)
	$(warning "Error: [Conflict feature option] To enable AIR_BT_ULTRA_LOW_LATENCY_IDLE_SNIFF_ENABLE must support AIR_BT_ULTRA_LOW_LATENCY_ENABLE")
	@echo "Error: [Conflict feature option] To enable AIR_BT_ULTRA_LOW_LATENCY_IDLE_SNIFF_ENABLE must support AIR_BT_ULTRA_LOW_LATENCY_ENABLE";
	@set -e; exit 2;
endif
endif

ifeq ($(AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE), y)
ifneq ($(AIR_BT_AUDIO_DONGLE_ENABLE), y)
	$(warning "Error: [Conflict feature option] To enable AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE must support AIR_BT_AUDIO_DONGLE_ENABLE")
	@echo "Error: [Conflict feature option] To enable AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE must support AIR_BT_AUDIO_DONGLE_ENABLE";
	@set -e; exit 2;
endif
endif

# Add make error for AMA and AMA Hotword feature enabled, but do not exist language model.
ifeq ($(AIR_AMA_ENABLE)_$(AIR_AMA_HOTWORD_ENABLE),y_y)
ifeq ($(wildcard $(SOURCE_DIR)/prebuilt/middleware/third_party/language_model/ama),)
	$(warning "The AMA language model for wake word detection is not exist, please contact Amazon to get the language model. Then put into the folder prebuilt/middleware/third_party/language_model/ama")
	@echo "[Addon feature option fail] The AMA language model for wake word detection is not exist, please contact Amazon to get the language model. Then put into the folder prebuilt/middleware/third_party/language_model/ama";
	@set -e; exit 2;
endif
endif

# Add make error for Secure Boot enabled, but do not exist Secure Boot prebuilt library.
ifeq ($(AIR_SECURITY_SECURE_BOOT_ENABLE),y)
ifeq ($(wildcard $(strip $(SOURCE_DIR))/prebuilt/middleware/airoha/secure_boot/),)
	$(warning "Error: [Addon feature option fail] The secure boot addon package doesn't exist. Please contact Airoha to get this addon package. ")
	@echo "[Addon feature option fail] The secure boot addon package doesn't exist. Please contact Airoha to get this addon package.";
	@set -e; exit 2;
endif
endif

ifeq ($(findstring y,$(AIR_LE_AUDIO_ENABLE) $(AIR_LE_AUDIO_DONGLE_ENABLE)),y)
ifeq ($(wildcard $(strip $(SOURCE_DIR))/middleware/airoha/bt_le_audio/),)
	$(warning "Error: [Addon feature option fail] The bt_le_audio addon package do not exist, please contact Airoha PLM to request the LE Audio addon package.")
	@echo "[Addon feature option fail] The bt_le_audio addon package do not exist, please contact Airoha PLM to request the LE Audio addon package.";
	@set -e; exit 2;
endif
endif

ifneq ($(findstring 3RD_PARTY_AI_NR, $(AIR_ECNR_CONFIG_TYPE)),)
ifeq ($(wildcard $(strip $(SOURCE_DIR))/../dsp/prebuilt/middleware/third_party/dspalg/igo_nr/),)
	$(warning "Error: [Addon feature option fail] The AI NR addon package do not exist, please contact Airoha PLM to request the AI NR addon package.")
	@echo "[Addon feature option fail] The AI NR addon package do not exist, please contact Airoha PLM to request the AI NR addon package.";
	@set -e; exit 2;
endif
endif

# Add make error for BT_Ultra_Low_Latency is not exist.
ifeq ($(findstring y,$(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) $(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) $(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)),y)
ifeq ($(wildcard $(strip $(SOURCE_DIR))/middleware/airoha/bt_ultra_low_latency/),)
    $(warning "Error: [Addon feature option fail] The gaming_mode addon package doesn't exist. Please contact Airoha to get this addon package.")
    @echo "[Addon feature option fail] The gaming_mode addon package doesn't exist. Please contact Airoha to get this addon package.";
    @set -e; exit 2;
endif
endif

# Add make error for wireless mic enabled, but do not exist AIR_BLE_ULTRA_LOW_LATENCY_ENABLE.
ifeq ($(AIR_WIRELESS_MIC_ENABLE), y)
ifneq ($(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE),y)
	$(warning "Error: [Conflict feature option] To enable AIR_WIRELESS_MIC_ENABLE must support AIR_BLE_ULTRA_LOW_LATENCY_ENABLE")
	@echo "Error: [Conflict feature option] To enable AIR_WIRELESS_MIC_ENABLE must support AIR_BLE_ULTRA_LOW_LATENCY_ENABLE";
	@set -e; exit 2;
endif
endif

# Add make error for BT Source enabled, but do not exist AIR_BT_AUDIO_DONGLE_ENABLE & AIR_USB_AUDIO_HID_ENABLE.
ifeq ($(AIR_BT_AUDIO_DONGLE_ENABLE), y)
ifneq ($(AIR_USB_AUDIO_HID_ENABLE),y)
    $(warning "Error: [Conflict feature option] To enable AIR_BT_SOURCE_ENABLE must support AIR_BT_AUDIO_DONGLE_ENABLE and AIR_USB_AUDIO_HID_ENABLE")
    @echo "Error: [Conflict feature option] To enable AIR_BT_SOURCE_ENABLE must support AIR_BT_AUDIO_DONGLE_ENABLE and AIR_USB_AUDIO_HID_ENABLE";
    @set -e; exit 2;
endif
endif

# Add make error for TMAP feature, MUST need LE audio support
ifeq ($(AIR_LE_AUDIO_TMAP_ENABLE), y)
ifneq ($(AIR_LE_AUDIO_ENABLE),y)
	$(warning "Error: [Conflict feature option] To enable AIR_LE_AUDIO_TMAP_ENABLE must support AIR_LE_AUDIO_ENABLE")
	@echo "Error: [Conflict feature option] To enable AIR_LE_AUDIO_TMAP_ENABLE must support AIR_LE_AUDIO_ENABLE";
	@set -e; exit 2;
endif
endif

# Add make error for GMAP feature, MUST need LE audio support
ifeq ($(AIR_LE_AUDIO_GMAP_ENABLE), y)
ifneq ($(AIR_LE_AUDIO_ENABLE),y)
	$(warning "Error: [Conflict feature option] To enable AIR_LE_AUDIO_GMAP_ENABLE must support AIR_LE_AUDIO_ENABLE")
	@echo "Error: [Conflict feature option] To enable AIR_LE_AUDIO_GMAP_ENABLE must support AIR_LE_AUDIO_ENABLE";
	@set -e; exit 2;
endif
endif

# Add make error for HAP feature, MUST need LE audio support
ifeq ($(AIR_LE_AUDIO_HAP_ENABLE), y)
ifneq ($(AIR_LE_AUDIO_ENABLE),y)
	$(warning "Error: [Conflict feature option] To enable AIR_LE_AUDIO_HAP_ENABLE must support AIR_LE_AUDIO_ENABLE")
	@echo "Error: [Conflict feature option] To enable AIR_LE_AUDIO_HAP_ENABLE must support AIR_LE_AUDIO_ENABLE";
	@set -e; exit 2;
endif
endif

# Add make error for TMAP feature, MUST need LE audio support
ifeq ($(AIR_LE_AUDIO_AURACAST_ENABLE), y)
ifneq ($(AIR_LE_AUDIO_ENABLE)_$(AIR_LE_AUDIO_BIS_ENABLE),y_y)
	$(warning "Error: [Conflict feature option] To enable AIR_LE_AUDIO_TMAP_ENABLE must support AIR_LE_AUDIO_ENABLE and AIR_LE_AUDIO_BIS_ENABLE")
	@echo "Error: [Conflict feature option] To enable AIR_LE_AUDIO_AURACAST_ENABLE must support AIR_LE_AUDIO_ENABLE and AIR_LE_AUDIO_BIS_ENABLE";
	@set -e; exit 2;
endif
endif

# Add make error for AIR_MS_GIP_ENABLE enabled
ifeq ($(AIR_MS_GIP_ENABLE),y)
ifneq ($(AIR_USB_XBOX_ENABLE),y)
	$(warning "Error: [Conflict feature option] To enable AIR_MS_GIP_ENABLE must support AIR_USB_XBOX_ENABLE")
	@echo "Error: [Conflict feature option] To enable AIR_MS_GIP_ENABLE must support AIR_USB_XBOX_ENABLE";
	@set -e; exit 2;
endif
ifneq ($(AIR_BT_ULTRA_LOW_LATENCY_ENABLE),y)
ifneq ($(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE),y)
	$(warning "Error: [Conflict feature option] To enable AIR_MS_GIP_ENABLE must support AIR_BT_ULTRA_LOW_LATENCY_ENABLE or AIR_BLE_ULTRA_LOW_LATENCY_ENABLE")
	@echo "Error: [Conflict feature option] To enable AIR_MS_GIP_ENABLE must support AIR_BT_ULTRA_LOW_LATENCY_ENABLE or AIR_BLE_ULTRA_LOW_LATENCY_ENABLE";
	@set -e; exit 2;
endif
endif
endif

# Add make error for AIR_AUDIO_SILENCE_DETECTION_ENABLE enabled
ifeq ($(AIR_LE_AUDIO_ENABLE), y)
ifeq ($(AIR_AUDIO_SILENCE_DETECTION_ENABLE), y)
ifneq ($(AIR_LE_AUDIO_UNICAST_ENABLE),y)
	$(warning "Error: [Conflict feature option] To enable AIR_AUDIO_SILENCE_DETECTION_ENABLE and AIR_LE_AUDIO_ENABLE must support AIR_LE_AUDIO_UNICAST_ENABLE")
	@echo "Error: [Conflict feature option] To enable AIR_AUDIO_SILENCE_DETECTION_ENABLE and AIR_LE_AUDIO_ENABLE must support AIR_LE_AUDIO_UNICAST_ENABLE";
	@set -e; exit 2;
endif
endif
endif

# Add make error for AIR_BT_BLE_UL_SWB_ENABLE or 3RD_PARTY_AI_NR enabled in unsupport project.
ifeq ($(AIR_BT_BLE_UL_SWB_ENABLE),y)
ifeq ($(IC_CONFIG),ab156x)
ifneq ($(AIR_AUDIO_LC3PLUS_CODEC_ENABLE),y)
	$(warning "Error: [Conflict feature option] To enable AIR_BT_BLE_UL_SWB_ENABLE must support AIR_AUDIO_LC3PLUS_CODEC_ENABLE.")
	@echo "Error: [Conflict feature option] To enable AIR_BT_BLE_UL_SWB_ENABLE must support AIR_AUDIO_LC3PLUS_CODEC_ENABLE.";
	@set -e; exit 2;
endif
ifneq ($(findstring $(AIR_ECNR_CONFIG_TYPE), 3RD_PARTY_AI_NR 3RD_PARTY_AI_NR_OFFLOAD 3RD_PARTY_AI_NR_OFFLOAD_POST_EC 3RD_PARTY_AI_NR_PRO_BROADSIDE_SEPARATE_MODE 3RD_PARTY_AI_NR_SEPARATE_MODE_EC 3RD_PARTY_AI_NR_INEAR 3RD_PARTY_AI_NR_SHORT_BOOM_OO 3RD_PARTY_AI_NR_PRO_DISTRACTOR 3RD_PARTY_AI_NR_PRO_TWS_OO 3RD_PARTY_CUSTOMIZED),)
	$(warning "Error: [Conflict feature option] To enable AIR_BT_BLE_UL_SWB_ENABLE must not support 3RD_PARTY_AI_NR.")
	@echo "Error: [Conflict feature option] To enable AIR_BT_BLE_UL_SWB_ENABLE must not support 3RD_PARTY_AI_NR.";
	@set -e; exit 2;
endif
endif
endif

# Add make error for mHDT feature ONLY support in ab1577x serials.
ifeq ($(AIR_BT_MHDT_ENABLE), y)
ifneq ($(IC_CONFIG),ab157x)
	$(warning "Error: [Conflict feature option] To enable AIR_BT_MHDT_ENABLE must support in ab1577 serials")
	@echo "Error: [Conflict feature option] To enable AIR_BT_MHDT_ENABLE must support in ab1577 serials";
	@set -e; exit 2;
endif
endif

ifeq ($(AIR_BTA_IC_PREMIUM_G2),y)
ifeq ($(AIR_ECNR_CONFIG_TYPE),3RD_PARTY_AI_NR_INEAR)
	$(warning "Error: [3rd party AINR lib match fail] dongle not support 1+1mic 3rd party AINR, please config AIR_ECNR_CONFIG_TYPE to 1mic 3rd party AINR.")
	@echo "[3rd party AINR lib match fail] dongle not support 1+1mic 3rd party AINR, please config AIR_ECNR_CONFIG_TYPE to 1mic 3rd party AINR.";
	@set -e; exit 2;
endif
endif

$(LIBS): $(MODULE_PATH)

$(MODULE_PATH):
	@+make -C $@ $(MOD_EXTRA) $($@_EXTRA)

proj: $(OUTPATH)/$(PROJ_NAME).elf

$(OUTPATH)/$(PROJ_NAME).elf: $(C_OBJS) $(CXX_OBJS) $(S_OBJS) $(LIBS)
	@if [ "$(LSCRIPT)" = "$(NoLinkerScript)" ]; then \
		echo >&2 ; \
		echo "=========================================================================" >&2 ; \
		echo "In flash booting mode, the linker script must be specified in feature.mk." >&2 ; \
		echo "=========================================================================" >&2 ; \
		echo >&2 ; \
		exit 68; \
	fi
	@echo Linking...
	@cp $(LSCRIPT) $(OUTPATH)
	@cp $(FEATURE) $(OUTPATH)
	@if [ -e "$@" ]; then rm -f "$@"; fi
	@if [ -e "$(OUTPATH)/$(PROJ_NAME).map" ]; then rm -f "$(OUTPATH)/$(PROJ_NAME).map"; fi
	@if [ -e "$(OUTPATH)/$(PROJ_NAME).bin" ]; then rm -f "$(OUTPATH)/$(PROJ_NAME).bin"; fi
	@echo $(patsubst $(SDK_PATH)/%,$(SOURCE_DIR)/%, $(LDFLAGS) -Wl,--start-group -Wl,--whole-archive $^ -Wl,--no-whole-archive -Wl,--end-group -Wl,-Map=$(patsubst $(SDK_PATH)/%,$(SOURCE_DIR)/%,$(OUTPATH)/$(PROJ_NAME).map) -lm -o $@) > $(OUTPATH)/link_option.tmp
	@$(CC) @$(OUTPATH)/link_option.tmp 2>>$(ERR_LOG)
	@rm -f $(OUTPATH)/link_option.tmp
	@$(OBJCOPY) -O binary --only-section=.log_str $(OUTPATH)/$(PROJ_NAME).elf $(OUTPATH)/mcu_log_str.bin
	@$(OBJCOPY) -O binary --only-section=.partition_table $(OUTPATH)/$(PROJ_NAME).elf $(OUTPATH)/partition_table.bin
	@$(OBJCOPY) -O binary --remove-section=.log_str --remove-section=.partition_table $(OUTPATH)/$(PROJ_NAME).elf $(OUTPATH)/$(PROJ_NAME).bin
	@echo Done


ifneq ($(filter MINGW% MSYS%,$(OS_VERSION)),)
  POSTBUILD := tools/scripts/build/PostBuildTool/windows/nvdm_post_build.exe
  LM_BIN_GENERATOR := tools/scripts/build/lm_bin_generator/windows/gen_lm_tool.exe
else ifeq ($(OS_VERSION), Darwin)
  POSTBUILD := tools/scripts/build/PostBuildTool/mac/nvdm_post_build
  LM_BIN_GENERATOR := tools/scripts/build/lm_bin_generator/mac/gen_lm_tool
else
  POSTBUILD := tools/scripts/build/PostBuildTool/linux/nvdm_post_build
  LM_BIN_GENERATOR := tools/scripts/build/lm_bin_generator/linux/gen_lm_tool
endif

include $(SOURCE_DIR)/.rule.mk

clean:
	rm -rf $(OUTPATH)

.PHONY: options

options:
	@for opt in `sed 's/[()]/\n/g' $(SOURCE_DIR)/config/chip/$(IC_CONFIG)/chip.mk|sed 's/-D/ /'|sort|uniq|grep ^MTK_|grep _ENABLE$$`; do echo $$opt; done
print-%  : ; @echo $* = $($*)
