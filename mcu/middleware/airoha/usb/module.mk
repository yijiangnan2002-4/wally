# Copyright Statement:
#
# (C) 2005-2016  MediaTek Inc. All rights reserved.
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
# Without the prior written permission of MediaTek and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
# You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
# if you have agreed to and been bound by the applicable license agreement with
# MediaTek ("License Agreement") and been granted explicit permission to do so within
# the License Agreement ("Permitted User").  If you are not a Permitted User,
# please cease any access or use of MediaTek Software immediately.
# BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
# ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
# WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
# NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
# SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
# SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
# THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
# THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
# CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
# SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
# STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
# CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
# AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
# OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
# MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
#


###############################################################################
# USB option default value
###############################################################################
AIR_USB_ENABLE             ?= n
AIR_USB_DEVICE_ENABLE      ?= n
AIR_USB_HOST_ENABLE        ?= n
AIR_USB_AUDIO_ENABLE       ?= n
AIR_USB_CDC_ENABLE         ?= n
AIR_USB_MSC_ENABLE         ?= n
AIR_USB_HID_ENABLE         ?= n
AIR_USB_XBOX_ENABLE        ?= n
AIR_USB_AUDIO_VERSION      ?= none
AIR_USB_AUDIO_1_SPK_ENABLE ?= n
AIR_USB_AUDIO_1_MIC_ENABLE ?= n
AIR_USB_AUDIO_2_SPK_ENABLE ?= n
AIR_USB_AUDIO_2_MIC_ENABLE ?= n
AIR_USB_CDC_PORT_NUM       ?= 2

###############################################################################
# Convert old option to new option
###############################################################################
# This is temp option to convert old option to new.
# For those project without new option.
AIR_USB_CONVERT_OLD_OPTION = y

ifeq ($(AIR_USB_CONVERT_OLD_OPTION), y)

AIR_USB_ENABLE        = y
AIR_USB_DEVICE_ENABLE = y
AIR_USB_HOST_ENABLE   = n

ifeq ($(AIR_USB_AUDIO_VERSION), v1)
AIR_USB_AUDIO_ENABLE = y
AIR_USB_AUDIO_1_SPK_ENABLE = y
AIR_USB_HID_ENABLE = y
endif

ifeq ($(AIR_USB_AUDIO_SPEAKER_ENABLE), y)
AIR_USB_HID_ENABLE = y
AIR_USB_AUDIO_1_SPK_ENABLE = y
endif

ifeq ($(AIR_USB_AUDIO_SPEAKER_ENABLE), n)
AIR_USB_HID_ENABLE = y
AIR_USB_AUDIO_1_SPK_ENABLE = n
endif

ifeq ($(AIR_USB_AUDIO_MICROPHONE_ENABLE), y)
AIR_USB_HID_ENABLE = y
AIR_USB_AUDIO_1_MIC_ENABLE = y
endif

ifeq ($(AIR_USB_AUDIO_2ND_SPEAKER_ENABLE), y)
AIR_USB_AUDIO_2_SPK_ENABLE = y
endif

AIR_USB_AUDIO_2_MIC_ENABLE = n

ifeq ($(AIR_USB_AUDIO_HID_ENABLE), y)
AIR_USB_HID_ENABLE = y
endif

AIR_USB_CDC_ENABLE = y

ifeq ($(AIR_USB_MSC_DEV_ENABLE), y)
AIR_USB_MSC_ENABLE = y
endif
endif # AIR_USB_CONVERT_OLD_OPTION

# Temp Solution
ifeq ($(AIR_DUAL_CHIP_MIXING_MODE), dongle)
USB_MACROS += AIR_USB_AUDIO_MULTI_CH_MODE
endif

###############################################################################
# USB Basic option
###############################################################################
ifeq ($(AIR_USB_ENABLE), y)
USB_MACROS += AIR_USB_ENABLE
USB_SRCS += _common/usb_main.c \
            _common/usb_resource.c \
            _common/usb_custom.c \
            _common/usb.c \
            _common/usb_case.c \
            _common/usb_host_detect.c \
            _common/usb_acti.c \
            _common/usb_dbg.c
endif

ifeq ($(AIR_USB_DEVICE_ENABLE), y)
USB_MACROS += AIR_USB_DEVICE_ENABLE
# USB_SRCS +=
endif

ifeq ($(AIR_USB_HOST_ENABLE), y)
USB_MACROS += AIR_USB_HOST_ENABLE
# USB_SRCS +=
endif

###############################################################################
# USB Feature option
###############################################################################
ifeq ($(AIR_USB_AUDIO_ENABLE), y)
USB_MACROS += AIR_USB_AUDIO_ENABLE
USB_SRCS += audio/usbaudio_drv.c
endif

ifeq ($(AIR_USB_CDC_ENABLE), y)
USB_MACROS += AIR_USB_CDC_ENABLE
USB_SRCS += cdc/usbacm_drv.c \
            cdc/usbacm_adap.c
endif

ifeq ($(AIR_USB_MSC_ENABLE), y)
USB_MACROS += AIR_USB_MSC_ENABLE
USB_SRCS += msc/usbms_adap.c \
            msc/usbms_drv.c \
            msc/usbms_state.c \
            msc/usbms_msdc.c \
            msc/USBMS_snand.c
endif

ifeq ($(AIR_USB_HID_ENABLE), y)
USB_MACROS += AIR_USB_HID_ENABLE
USB_SRCS += hid/usbhid_drv.c
endif # AIR_USB_HID_ENABLE

ifeq ($(AIR_USB_XBOX_ENABLE), y)
USB_MACROS += AIR_USB_XBOX_ENABLE
USB_SRCS += xbox/usb_xbox.c
endif

###############################################################################
# USB Audio option
###############################################################################
USB_MACROS += AIR_USB_AUDIO_VERSION=$(AIR_USB_AUDIO_VERSION)

ifeq ($(AIR_USB_AUDIO_1_SPK_ENABLE), y)
USB_MACROS += AIR_USB_AUDIO_1_SPK_ENABLE
endif

ifeq ($(AIR_USB_AUDIO_1_MIC_ENABLE), y)
USB_MACROS += AIR_USB_AUDIO_1_MIC_ENABLE
endif

ifeq ($(AIR_USB_AUDIO_2_SPK_ENABLE), y)
USB_MACROS += AIR_USB_AUDIO_2_SPK_ENABLE
endif

ifeq ($(AIR_USB_AUDIO_2_MIC_ENABLE), y)
USB_MACROS += AIR_USB_AUDIO_2_MIC_ENABLE
endif

###############################################################################
# USB CDC option
###############################################################################
AIR_USB_CDC_PORT_NUM ?= 2
USB_MACROS += AIR_USB_CDC_PORT_NUM=$(AIR_USB_CDC_PORT_NUM)

###############################################################################
# else
###############################################################################
USB_MACROS += USB_LOW_MEM_REQ

###############################################################################
# Use new option generate old macros
###############################################################################
# This is temp option use new option to generate old macros.
# Some module still use old macros currently.
USB_NEW_OPTION_GEN_OLD_MACROS = y

ifeq ($(USB_NEW_OPTION_GEN_OLD_MACROS), y)
ifeq ($(AIR_USB_ENABLE), y)
USB_MACROS += MTK_USB_ENABLED
endif

ifeq ($(AIR_USB_AUDIO_ENABLE), y)
USB_MACROS += MTK_USB_AUDIO_V1_ENABLE
endif

ifeq ($(AIR_USB_AUDIO_1_MIC_ENABLE), y)
USB_MACROS += MTK_USB_AUDIO_MICROPHONE
endif

ifeq ($(AIR_USB_AUDIO_2_SPK_ENABLE), y)
USB_MACROS += MTK_USB_AUDIO_2ND_SPEAKER
endif

ifeq ($(AIR_USB_HID_ENABLE), y)
USB_MACROS += MTK_USB_AUDIO_HID_ENABLE
endif

ifeq ($(AIR_USB_CDC_ENABLE), y)
USB_MACROS += USB_CDC_ACM_ENABLE
endif

ifeq ($(AIR_USB_XBOX_ENABLE), y)
USB_MACROS += MTK_USB_XBOX_ENABLE
endif

ifeq ($(AIR_USB_MSC_ENABLE), y)
USB_MACROS += __USB_MASS_STORAGE_ENABLE__
endif
endif # USB_NEW_OPTION_GEN_OLD_MACROS

###############################################################################
# USB module.mk debug
###############################################################################
USB_MODULE_MK_DEBUG = n
ifeq ($(USB_MODULE_MK_DEBUG), y)
USB_OPTIONS += AIR_USB_ENABLE
USB_OPTIONS += AIR_USB_DEVICE_ENABLE
USB_OPTIONS += AIR_USB_HOST_ENABLE
USB_OPTIONS += AIR_USB_AUDIO_ENABLE
USB_OPTIONS += AIR_USB_CDC_ENABLE
USB_OPTIONS += AIR_USB_MSC_ENABLE
USB_OPTIONS += AIR_USB_HID_ENABLE
USB_OPTIONS += AIR_USB_XBOX_ENABLE
USB_OPTIONS += AIR_USB_AUDIO_VERSION
USB_OPTIONS += AIR_USB_AUDIO_1_SPK_ENABLE
USB_OPTIONS += AIR_USB_AUDIO_1_MIC_ENABLE
USB_OPTIONS += AIR_USB_AUDIO_2_SPK_ENABLE
USB_OPTIONS += AIR_USB_AUDIO_2_MIC_ENABLE
USB_OPTIONS += AIR_USB_CDC_PORT_NUM

$(info ==== USB module.mk debug start =========================================)
$(info USB Sources ------------------------------------------------------------)
$(foreach src,$(USB_SRCS), \
    $(info $(shell printf '%s' $(src))) \
)
$(info USB Options -----------------------------------------------------------)
$(foreach option,$(USB_OPTIONS), \
    $(info $(shell printf '%-26s = %s' $(option) $($(option)))) \
)
$(info USB Macros ------------------------------------------------------------)
$(foreach macro,$(USB_MACROS), \
    $(info $(shell printf '%s' $(macro))) \
)
$(info ==== USB module.mk debug end ==========================================)
endif

###############################################################################
# Add C files, Include, Macros
###############################################################################
USB_PATH = $(MIDDLEWARE_PROPRIETARY)/usb/src/
C_FILES += $(addprefix $(USB_PATH),$(USB_SRCS))
CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/usb/inc
CFLAGS += $(addprefix -D,$(USB_MACROS))
#CFLAGS += -DUSB_TEST_MIC