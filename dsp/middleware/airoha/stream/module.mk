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


###################################################
STREAM_PATH = $(MIDDLEWARE_PROPRIETARY)/stream

C_SRC += $(STREAM_PATH)/src/source.c
C_SRC += $(STREAM_PATH)/src/sink.c
C_SRC += $(STREAM_PATH)/src/stream.c
C_SRC += $(STREAM_PATH)/src/transform.c
C_SRC += $(STREAM_PATH)/src/stream_interface/audio_transmitter_scenario_port/scenario_audio_common.c
C_SRC += $(STREAM_PATH)/src/stream_interface/stream_virtual.c

ifeq ($(AIR_AUDIO_HARDWARE_ENABLE),y)
C_SRC += $(STREAM_PATH)/src/stream_interface/stream_audio.c
C_SRC += $(STREAM_PATH)/src/stream_interface/stream_audio_afe.c
endif
C_SRC += $(STREAM_PATH)/src/stream_interface/stream_memory.c

ifeq ($(AIR_BT_A2DP_ENABLE), y)
    C_SRC += $(STREAM_PATH)/src/stream_interface/stream_avm_a2dp.c
endif

ifneq ($(AIR_BT_HFP_ENABLE)_$(AIR_BT_AUDIO_DONGLE_ENABLE), n_n)
    C_SRC += $(STREAM_PATH)/src/stream_interface/stream_forwarder_sco.c
endif

ifeq ($(AIR_BT_CODEC_BLE_ENABLED), y)
    C_SRC += $(STREAM_PATH)/src/stream_interface/stream_n9ble.c
endif

ifneq ($(MTK_CM4_PLAYBACK_ENABLE)_$(MTK_PROMPT_SOUND_ENABLE)_$(AIR_PROMPT_SOUND_DUMMY_SOURCE_ENABLE), n_n_n)
    C_SRC += $(STREAM_PATH)/src/stream_interface/stream_cm4_playback.c
endif

ifeq ($(AIR_MIC_RECORD_ENABLE), y)
    C_SRC += $(STREAM_PATH)/src/stream_interface/stream_cm4_record.c
endif

ifeq ($(MTK_PROMPT_SOUND_ENABLE), y)
    C_SRC += $(STREAM_PATH)/src/stream_interface/stream_cm4_vp_playback.c
endif

ifeq ($(AIR_PROMPT_SOUND_DUMMY_SOURCE_ENABLE), y)
    CCFLAG += -DAIR_PROMPT_SOUND_DUMMY_SOURCE_ENABLE
    C_SRC += $(STREAM_PATH)/src/stream_interface/stream_cm4_vp_dummy_source_playback.c
endif

ifeq ($(MTK_SENSOR_SOURCE_ENABLE), y)
    C_SRC += $(STREAM_PATH)/src/stream_interface/stream_gsensor_detect.c
endif

#ifeq ($(AIR_AUDIO_TRANSMITTER_ENABLE), y)
    C_SRC += $(STREAM_PATH)/src/stream_interface/stream_audio_transmitter.c
#endif

ifeq ($(AIR_AUDIO_BT_COMMON_ENABLE), y)
    C_SRC += $(STREAM_PATH)/src/stream_interface/stream_bt_common.c
endif

ifeq ($(AIR_GAMING_MODE_DONGLE_ENABLE), y)
    C_SRC += $(STREAM_PATH)/src/stream_interface/audio_transmitter_scenario_port/scenario_ull_audio.c
endif

ifeq ($(AIR_WIRED_AUDIO_ENABLE), y)
    C_SRC += $(STREAM_PATH)/src/stream_interface/audio_transmitter_scenario_port/scenario_wired_audio.c
endif

ifeq ($(AIR_BLE_AUDIO_DONGLE_ENABLE), y)
    C_SRC += $(STREAM_PATH)/src/stream_interface/audio_transmitter_scenario_port/scenario_ble_audio.c
endif

ifeq ($(AIR_ULL_AUDIO_V2_DONGLE_ENABLE), y)
    C_SRC += $(STREAM_PATH)/src/stream_interface/audio_transmitter_scenario_port/scenario_ull_audio_v2.c
endif

ifeq ($(AIR_WIRELESS_MIC_RX_ENABLE), y)
    C_SRC += $(STREAM_PATH)/src/stream_interface/audio_transmitter_scenario_port/scenario_wireless_mic_rx.c
endif

ifeq ($(AIR_BT_AUDIO_DONGLE_ENABLE), y)
    C_SRC += $(STREAM_PATH)/src/stream_interface/audio_transmitter_scenario_port/scenario_bt_audio.c
endif

ifeq ($(AIR_RECORD_ADVANCED_ENABLE), y)
    C_SRC += $(STREAM_PATH)/src/stream_interface/audio_transmitter_scenario_port/scenario_advanced_record.c
endif

ifeq ($(AIR_VP_NOT_USE_HWSRC_DEFAULT_ENABLE), y)
    CCFLAG += -DAIR_VP_NOT_USE_HWSRC_DEFAULT_ENABLE
endif

ifeq ($(AIR_DUAL_CHIP_AUDIO_INTERFACE),uart)
    C_SRC += $(STREAM_PATH)/src/stream_interface/stream_dchs.c
endif

ifeq ($(AIR_FULL_ADAPTIVE_ANC_ENABLE), y)
    C_SRC += $(STREAM_PATH)/src/stream_interface/stream_full_adapt_anc.c
endif

ifeq ($(AIR_HW_VIVID_PT_ENABLE), y)
    C_SRC += $(STREAM_PATH)/src/stream_interface/stream_hw_vivid_passthru.c
endif

###################################################
# include path

INC += $(STREAM_PATH)/inc
INC += $(STREAM_PATH)/inc/audio_transmitter_scenario_port

INC  +=  ../common/middleware/airoha/audio/audio_transmitter/inc/

