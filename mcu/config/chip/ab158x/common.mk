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

IC_CONFIG                             = ab158x
IC_TYPE                              ?= ab1585
MIDDLEWARE_PROPRIETARY                = middleware/airoha
# debug level: none, error, warning, info and debug
MTK_DEBUG_LEVEL                       = info
MTK_NVDM_ENABLE                       = n

MTK_USB_DEMO_ENABLED                  = n
MTK_USB_AUDIO_V1_ENABLE               = n
MTK_USB_AUDIO_V2_ENABLE               = n
MTK_USB_AUDIO_MICROPHONE              = n

MTK_HAL_EXT_32K_ENABLE                = n
MTK_NO_PSRAM_ENABLE					  = y
# heap dump
MTK_SUPPORT_HEAP_DEBUG                = y
MTK_SUPPORT_HEAP_DEBUG_ADVANCED       = n
# heap peak profiling
MTK_HEAP_SIZE_GUARD_ENABLE            = n
# system hang debug: none, y, o1, o2 and mp
MTK_SYSTEM_HANG_TRACER_ENABLE         = y
###################################################

# port service
MTK_PORT_SERVICE_ENABLE = y

# bt codec enable
MTK_BT_CODEC_ENABLED = y

# BT A2DP codec AAC support
MTK_BT_A2DP_AAC_ENABLE = y

# BT A2DP codec vendor support
MTK_BT_A2DP_VENDOR_ENABLE = n

# avm direct feature
MTK_AVM_DIRECT                       = y

# prompt sound
MTK_PROMPT_SOUND_ENABLE	            = y
MTK_PROMPT_SOUND_SYNC_ENABLE        = n
MTK_AUDIO_AT_CMD_PROMPT_SOUND_ENABLE          = n

# mp3
MTK_AUDIO_MP3_ENABLED               = n
MTK_MP3_DECODER_ENABLED             = y
MTK_MP3_CODEC_TASK_DEDICATE         = y
MTK_MP3_STEREO_SUPPORT              = y
# wave decoder by charlie
MTK_WAV_DECODER_ENABLE              = n
# record middleware
MTK_RECORD_ENABLE                   = y
# audio dump
MTK_AUDIO_DUMP_ENABLE               = y
# PEQ
MTK_PEQ_ENABLE                      = y

# LINEIN PEQ
MTK_LINEIN_PEQ_ENABLE               = n

# AirDump module
MTK_AIRDUMP_EN                      = y

# NVDM gain setting table
MTK_AUDIO_GAIN_TABLE_ENABLE         = y
# boot reason check
MTK_BOOTREASON_CHECK_ENABLE         = n
MTK_BT_FAST_PAIR_ENABLE             = n
MTK_MINIDUMP_ENABLE                 = n
MTK_FULLDUMP_ENABLE                 = y

# ANC module
MTK_ANC_ENABLE = n
ifeq ($(MTK_ANC_ENABLE), y)
ANC_SOURCE_CODE = $(strip $(SOURCE_DIR))/../mcu/middleware/airoha/audio/anc_control_protected/GCC/module.mk
ifeq ($(ANC_SOURCE_CODE), $(wildcard $(ANC_SOURCE_CODE)))
MTK_ANC_LIB_ENABLE =  n
else
MTK_ANC_LIB_ENABLE =  y
endif
endif
ifeq ($(MTK_ANC_ENABLE),y)
MTK_ANC_BACKUP_STATUS_ENABLE        = n
MTK_HYBRID_ANC_ENABLE               = y
ifeq ($(MTK_HYBRID_ANC_ENABLE),y)
MTK_POST_PEQ_DEFAULT_ON             = n
MTK_VOICE_ANC_EQ                    = n
MTK_DEQ_ENABLE                      = n
endif
endif

AIR_AUDIO_CODEC_MANAGER_ENABLE = y

# Add opus encode support
AIR_OPUS_ENCODER_ENABLE				= y

# Record opus encoder
MTK_RECORD_OPUS_ENABLE = n

# 2a2d
MTK_AUDIO_SUPPORT_MULTIPLE_MICROPHONE = n

# Use HWSRC to do clkskew
ENABLE_HWSRC_CLKSKEW = y

