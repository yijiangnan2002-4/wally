# Copyright Statement:
#
# (C) 2019 Airoha Technology Corp. All rights reserved.
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
# the License Agreement ("Permitted User"). If you are not a Permitted User,
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

ifeq ($(AIR_LE_AUDIO_ENABLE), y)
###################################################
# Sources
###################################################
LE_AUDIO_SRC = $(MIDDLEWARE_PROPRIETARY)/bt_le_audio/src

LE_AUDIO_FILE = $(LE_AUDIO_SRC)/ascs/ble_ascs_service.c \
                $(LE_AUDIO_SRC)/csip/ble_csis_service.c \
                $(LE_AUDIO_SRC)/micp/ble_mics_service.c \
                $(LE_AUDIO_SRC)/pacs/ble_pacs_service.c \
                $(LE_AUDIO_SRC)/vcp/ble_aics_service.c \
                $(LE_AUDIO_SRC)/vcp/ble_vcs_service.c \
                $(LE_AUDIO_SRC)/vcp/ble_vocs_service.c \
                $(LE_AUDIO_SRC)/util/bt_le_audio_util.c \
                $(LE_AUDIO_SRC)/util/bt_le_audio_log.c \
                $(LE_AUDIO_SRC)/cap/ble_cas_service.c \
                $(LE_AUDIO_SRC)/haps/ble_has_service.c \
                $(LE_AUDIO_SRC)/gmas/ble_gmas_service.c \


C_FILES    +=    $(LE_AUDIO_FILE)
C_FILES    +=    $(LE_AUDIO_SRC)/bt_le_audio_sink.c
C_FILES    +=    $(LE_AUDIO_SRC)/bt_le_audio_source.c
C_FILES    +=    $(LE_AUDIO_SRC)/bt_le_audio_source_service_discovery.c
C_FILES    +=    $(LE_AUDIO_SRC)/bt_le_audio_atci_cmd.c
###################################################
# include path (bt_le_audio)
###################################################
CFLAGS    +=    -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_le_audio/inc
CFLAGS    +=    -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_le_audio/inc/ascs
CFLAGS    +=    -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_le_audio/inc/bap
CFLAGS    +=    -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_le_audio/inc/bass
CFLAGS    +=    -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_le_audio/inc/ccp
CFLAGS    +=    -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_le_audio/inc/csip
CFLAGS    +=    -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_le_audio/inc/mcp
CFLAGS    +=    -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_le_audio/inc/micp
CFLAGS    +=    -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_le_audio/inc/pacs
CFLAGS    +=    -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_le_audio/inc/tmap
CFLAGS    +=    -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_le_audio/inc/vcp
CFLAGS    +=    -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_le_audio/inc/util
CFLAGS    +=    -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_le_audio/inc/cap
CFLAGS    +=    -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_le_audio/inc/pbp
CFLAGS    +=    -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_le_audio/inc/haps
CFLAGS    +=    -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_le_audio/inc/hapc
CFLAGS    +=    -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_le_audio/inc/gmap
CFLAGS    +=    -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_le_audio/inc/iac

###################################################
# include path
###################################################
CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bluetooth/inc
CFLAGS += -I$(SOURCE_DIR)/prebuilt/$(MIDDLEWARE_PROPRIETARY)/le_audio/inc

###################################################
# Libs
###################################################
# The library name
LE_AUDIO_LIB = libbt_leaudio.a

LE_AUDIO_LIB_PATH = le_audio/$(IC_CONFIG)

# check the bt_cap_protected fodler exist or not.
# If the folder eixst, make the lib with source code
# otherwise, use the library directly.
ifneq ($(wildcard $(strip $(SOURCE_DIR))/$(MIDDLEWARE_PROPRIETARY)/bt_le_audio_protected/),)
LIBS += $(OUTPATH)/$(LE_AUDIO_LIB)
MODULE_PATH += $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_le_audio_protected/GCC
else
LIBS += $(SOURCE_DIR)/prebuilt/$(MIDDLEWARE_PROPRIETARY)/$(LE_AUDIO_LIB_PATH)/lib/$(LE_AUDIO_LIB)
endif


endif
