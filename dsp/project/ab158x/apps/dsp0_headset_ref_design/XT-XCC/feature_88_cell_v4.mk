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

##############################################

    # NOTE - IMPORTANT
    # All shared feature option/configuration shall be put in DSP side.

##############################################

IC_CONFIG                             ?= ab158x
BOARD_CONFIG                          ?= ab158x_evb

# Using specific linker script during linking process.
AIR_DSP_LINKER_SCRIPT_FILE = ab158x_dsp0_flash.lcf

##############################################
#           Custom feature option            #
##############################################

## part1: configure y/n

###### Audio/Voice effects ######

# This option is to enable/disable Audio PLC.
# This feature option/configuration is shared between the MCU and DSP side.
AIR_AUDIO_PLC_ENABLE = n

# This option is used to enable/disable User triggered adaptive ANC.
# This feature option/configuration is shared between the MCU and DSP side.
# Dependency: AIR_ANC_ENABLE and AIR_AUDIO_TRANSMITTER_ENABLE must be enabled when this option is set to y.
AIR_ANC_USER_TRIGGER_ENABLE = n

# This option is to enable adaptive eq feature.
# This feature option/configuration is shared between the MCU and DSP side.
# Dependency: AIR_PEQ_ENABLE must be enabled when this option is set to y.
AIR_ADAPTIVE_EQ_ENABLE = n

# This option is to enable PEQ feature.
# This feature option/configuration is shared between the MCU and DSP side.
# Dependency: AIR_BT_PEQ_USE_PIC_ENABLE must be enabled when this option is set to y.
AIR_PEQ_ENABLE = y

# This option is to enable Dynamic range compression feature.
AIR_DRC_ENABLE = y

# This option is to enable VP PEQ feature.
# This feature option/configuration is shared between the MCU and DSP side.
# Dependency: AIR_PEQ_ENABLE must be enabled when this option is set to y.
AIR_VP_PEQ_ENABLE = n

# This option is to enable/disable fit detection.
# This feature option/configuration is shared between the MCU and DSP side.
# Dependency: AIR_ANC_ENABLE must be enabled when this option is set to y.
AIR_ANC_FIT_DETECTION_ENABLE = y

# This option is used to enable/disable wind noise detection adaptive ANC.
# This feature option/configuration is shared between the MCU and DSP side.
# Dependency: AIR_ANC_ENABLE must be enabled when this option is set to y.
AIR_ANC_WIND_NOISE_DETECTION_ENABLE = y

# This option is used to enable/disable Environment Detection adaptive ANC.
# This feature option/configuration is shared between the MCU and DSP side.
# Dependency: AIR_ANC_ENABLE and AIR_AUDIO_TRANSMITTER_ENABLE must be enabled when this option is set to y.
AIR_ANC_ENVIRONMENT_DETECTION_ENABLE = y

# This is the option of audio transmitter. Users who use audio transmitter need to open this option.
# This feature option/configuration is shared between the MCU and DSP side.
AIR_AUDIO_TRANSMITTER_ENABLE = y

# This option is used to support HWSRC for DL1.
AIR_HWSRC_ON_MAIN_STREAM_ENABLE = y

# This option is for speaker project hwsrc clk skew. It should not be enabled by default.
AIR_HWSRC_IN_STREAM_ENABLE = n

# This option is used to enable/disable HWSRC CLK SKEW.
# This feature option/configuration is shared between the MCU and DSP side.
# Dependency: AIR_HWSRC_ON_MAIN_STREAM_ENABLE must be enabled when this option is set to y.
AIR_HWSRC_CLKSKEW_ENABLE = y

# This option is to enable/disable the detachable mic.
# This feature option/configuration is shared between the MCU and DSP side.
AIR_AUDIO_DETACHABLE_MIC_ENABLE = n

# This option is used to enable/disable HWSRC RX TRACKING.
# This feature option/configuration is shared between the MCU and DSP side.
AIR_HWSRC_RX_TRACKING_ENABLE = n

# This option is used to enable/disable HWSRC TX TRACKING.
# This feature option/configuration is shared between the MCU and DSP side.
AIR_HWSRC_TX_TRACKING_ENABLE = n

# This option is used to enable/disable User triggered adaptive ANC.
# This feature option/configuration is shared between the MCU and DSP side.
# Dependency: AIR_ANC_ENABLE and AIR_AUDIO_TRANSMITTER_ENABLE must be enabled when this option is set to y.
AIR_USER_TRIGGER_FF_ENABLE = n

# This option is used to enable/disable User Unaware adaptive ANC.
# This feature option/configuration is shared between the MCU and DSP side.
# Dependency: AIR_ANC_ENABLE and AIR_AUDIO_TRANSMITTER_ENABLE must be enabled when this option is set to y.
AIR_ANC_USER_UNAWARE_ENABLE = n

###### Audio peripheral ######

# This option is used to enable line in mix with A2DP/HFP.
# This feature option/configuration is shared between the MCU and DSP side. No support for earbuds project..
AIR_LINE_IN_AND_BT_MIX_ENABLE = n

# This option is used to enable line in.
# This feature option/configuration is shared between the MCU and DSP side. No support for earbuds project.
AIR_LINE_IN_ENABLE = n

# This option is to enable/disable LINE IN PEQ feature.
# This feature option/configuration is shared between the MCU and DSP side. No support for earbuds project.
# Dependency: AIR_DRC_ENABLE and AIR_LINE_IN_ENABLE must be enabled when this option is set to y.
AIR_LINE_IN_PEQ_ENABLE = n

# This option is to enable/disable LINE IN INS feature.
# This feature option/configuration is shared between the MCU and DSP side. No support for earbuds project.
AIR_LINE_IN_INS_ENABLE = n

# This option is used to enable line out.
# This feature option/configuration is shared between the MCU and DSP side. No support for earbuds project.
AIR_LINE_OUT_ENABLE = n

# This option is used to enable wired USB audio playback.
# This feature option/configuration is shared between the MCU and DSP side. No support for earbuds project.
AIR_USB_AUDIO_IN_ENABLE = n

# This option is used to enable wired USB audio out.
# This feature option/configuration is shared between the MCU and DSP side. No support for earbuds project.
AIR_USB_AUDIO_IN_AND_OUT_MIX_ENABLE = n

# This option is used to enable wired USB audio out.
# This feature option/configuration is shared between the MCU and DSP side. No support for earbuds project.
AIR_USB_AUDIO_OUT_ENABLE = n

# This option is used to enable wired USB audio in mix with A2DP/HFP.
# This feature option/configuration is shared between the MCU and DSP side. No support for earbuds project.
AIR_USB_AUDIO_IN_AND_BT_MIX_ENABLE = n

# This option is used to enable/disable volume estimator.
# This feature option/configuration is shared between the MCU and DSP side.
AIR_VOLUME_ESTIMATOR_ENABLE = n

# This option is used to enable/disable volume smart balance.
# This feature option/configuration is shared between the MCU and DSP side.
AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE = n

###### Scenario ######

# This option is used to enable/disable HFP scenario.
# This feature option/configuration is shared between the MCU and DSP side.
AIR_BT_HFP_ENABLE = y

# This option is used to enable/disable CPD PIC.
AIR_BT_A2DP_CPD_USE_PIC_ENABLE = n

# This option is used to enable/disable record mic input scenario.
AIR_MIC_RECORD_ENABLE = y

######  Codec ######

# This option is used to enable vendor codec.
# This feature option/configuration is shared between the MCU and DSP side.
AIR_BT_A2DP_VENDOR_ENABLE = n

# This option is used to enable AAC codec.
# This feature option/configuration is shared between the MCU and DSP side.
AIR_BT_A2DP_AAC_ENABLE = y

# This option is used to enable SBC codec.
# This feature option/configuration is shared between the MCU and DSP side.
AIR_BT_A2DP_SBC_ENABLE = y

######  Voice assistant/VAD ######

# This option is used to enable AMA HOT word feature.
# This feature option/configuration is shared between the MCU and DSP side.
AIR_AMA_HOTWORD_ENABLE = n

# This option is used to enable AMA hotword PIC version.
# Dependency: AIR_AMA_HOTWORD_ENABLE must be enabled when this option is set to y.
AIR_AMA_HOTWORD_USE_PIC_ENABLE = n

# This option is used to enable GSOUND HOT word feature.
# This feature option/configuration is shared between the MCU and DSP side.
AIR_GSOUND_HOTWORD_ENABLE = n

######  BT/MCSync ######

# This option is to enable/disable LE Audio.
# This feature option/configuration is shared between the MCU and DSP side.
AIR_LE_AUDIO_ENABLE = y

# This option is to enable LE Audio dongle scenario.
# This feature option/configuration is shared between the MCU and DSP side.
AIR_LE_AUDIO_DONGLE_ENABLE = n

# This option is to enable ULL dongle scenario.
# It must be turned on/off for both DSP and MCU side AIR_BT_ULTRA_LOW_LATENCY_ENABLE option, otherwise, it will not work.
AIR_ULL_GAMING_DONGLE_ENABLE = n

# This option is to enable ULL headset scenario.
# It must be turned on/off for both DSP and MCU side AIR_BT_ULTRA_LOW_LATENCY_ENABLE option, otherwise, it will not work.
AIR_ULL_GAMING_HEADSET_ENABLE = n

# This option is to enable ULL BLE headset scenario.
# It must be turned on/off for both DSP and MCU side AIR_BT_ULTRA_LOW_LATENCY_ENABLE option, otherwise, it will not work.
AIR_ULL_BLE_HEADSET_ENABLE = n

# This option is used to enable/disable LC3PLUS codec support. It should be disable by default.
# This feature option/configuration is shared between the MCU and DSP side.
AIR_AUDIO_LC3PLUS_CODEC_ENABLE = n

# This option is used to enable/disable ULD codec support. It should be disable by default.
# This feature option/configuration is shared between the MCU and DSP side.
AIR_AUDIO_ULD_CODEC_ENABLE = n

# This option is used to enable/disable vendor codec support. It should be disable by default.
# This feature option/configuration is shared between the MCU and DSP side.
AIR_AUDIO_VEND_CODEC_ENABLE = n

# This option is used to enable/disable silence detection feature. It should be disable by default.
# This feature option/configuration is shared between the MCU and DSP side.
AIR_AUDIO_SILENCE_DETECTION_ENABLE = n

# This option is to enable wireless microphone scenario.
# This feature option/configuration is shared between the MCU and DSP side.
AIR_WIRELESS_MIC_ENABLE = n

# This option is used to enable/disable volume monitor.
AIR_AUDIO_VOLUME_MONITOR_ENABLE = n

# This option is used to enable/disable voice activity while muted feature.
AIR_AUDIO_VAD_ON_MUTE_ENABLE = y

######  System ######

# This option is to enable audio dump for CM4 side debug use.
# This feature option/configuration is shared between the MCU and DSP side.
AIR_AUDIO_DUMP_ENABLE = y

# This option is to support air dump for ecnr.
# This feature option/configuration is shared between the MCU and DSP side.
AIR_AIRDUMP_ENABLE = n

## part2: need to set specified value

######  System ######

# This option is to configure system log debug level.
# Usage: none, error, warning, info, debug
#              empty   : All debug logs are compiled.
#              error   : Only error logs are compiled.
#              warning : Only warning and error logs are compiled.
#              info    : Only info, warning, and error logs are compiled.
#              debug   : All debug logs are compiled.
#              none    : All debugs are disabled.
AIR_DEBUG_LEVEL = info

##
## AIR_DSP_PRODUCT_CATEGORY
## Brief:       This option is to configure DSP different project.
## Usage:       Project category includes none/Headset/Earbuds
## Notice:      None.
##
AIR_DSP_PRODUCT_CATEGORY = Earbuds

######  Audio ######

# This option is to choose the default type of Active Noise Cancellation (ANC). Default setting is ANC_STATIC.
# It must be set to the same value for both DSP and MCU, otherwise, it will not work.
# Usage: none, ANC_STATIC, ANC_FULL_ADAPTIVE
#                   none              : Non-support Active Noise Cancellation.
#                   ANC_STATIC        : Default support type of Active Noise Cancellation will be static ANC.
#                   ANC_FULL_ADAPTIVE : Default support type of Active Noise Cancellation will be full adaptive ANC.
AIR_ANC_ENABLE_TYPE   = ANC_FULL_ADAPTIVE

# This option is to choose the default type of Passthru. Default setting is PASSTHRU_HYBRID.
# It must be set to the same value for both DSP and MCU, otherwise, it will not work.
# Usage: none, PASSTHRU_FF, PASSTHRU_HYBRID, PASSTHRU_ADAPTIVE
#                        none  : Non-support Passthru.
#                        PASSTHRU_FF       : Default support type of Passthru will be feedforward Passthru.
#                        PASSTHRU_HYBRID   : Default support type of Passthru will be hybrid Passthru.
#                        PASSTHRU_ADAPTIVE : Default support type of Passthru will be adaptive Passthru.
AIR_PASSTHRU_ENABLE_TYPE = PASSTHRU_HYBRID

# This option is to choose the uplink rate. Default setting is none.
# It must be set to the same value for both DSP and MCU, otherwise, it will not work.
# Usage: none, 32k, 48k
#                none : uplink rate will be handled by scenario itself.
#                32k  : uplink rate will be fixed in 32k Hz.
#                48k  : uplink rate will be fixed in 48k Hz.
AIR_UPLINK_RATE   = none

# This option is to fix the uplink DMA resolution. Default setting is none.
# It must be set to the same value for both DSP and MCU, otherwise, it will not work.
# Usage: none, 32bit
#                      none : uplink resolution will be handled by scenario itself.
#                      32bit: uplink resolution will be fixed at 32-bit.
AIR_UPLINK_RESOLUTION   = none

# This option is to choose the downlink rate. Default setting is none.
# It must be set to the same value for both DSP and MCU, otherwise, it will not work.
# Usage: none, 48k, 96k
#                  none  : downlink rate will be handled by scenario itself.
#                  48k   : downlink rate will be fixed in 48k Hz.
#                  96k   : downlink rate will be fixed in 96k Hz.
AIR_DOWNLINK_RATE = 48k

# This option is to choose the type of BAND. Default setting is SWB.
# It must be set to the same value for both DSP and MCU, otherwise, it will not work.
# Usage: WB(Wideband), SWB(Super wideband), FB(Full band)
#                  WB   : The maximum bandwidth achievable is Wideband
#                  SWB  : The maximum bandwidth achievable is Super wideband
#                  FB   : The maximum bandwidth achievable is Full band
AIR_VOICE_BAND_CONFIG_TYPE = SWB

# This option is to choose the type of ECNR. Default setting is none.
# It must be set to the same value for both DSP and MCU, otherwise, it will not work.
# Usage: none, ECNR_1_OR_2_MIC, ECNR_1MIC_INEAR, ECNR_2MIC_INEAR, 3RD_PARTY_AI_NR, 3RD_PARTY_AI_NR_OFFLOAD, 3RD_PARTY_AI_NR_OFFLOAD_POST_EC, 3RD_PARTY_AI_NR_PRO_BROADSIDE_SEPARATE_MODE, 3RD_PARTY_AI_NR_SEPARATE_MODE_EC, 3RD_PARTY_AI_NR_INEAR, 3RD_PARTY_AI_NR_SHORT_BOOM_OO, 3RD_PARTY_AI_NR_PRO_DISTRACTOR, 3RD_PARTY_AI_NR_PRO_TWS_OO, 3RD_PARTY_CUSTOMIZED, LD_NR
#                  none             : disable ECNR
#                  ECNR_1_OR_2_MIC  : Inhouse ECNR to support 1 or 2 MIC.
#                  ECNR_1MIC_INEAR  : Inhouse ECNR to support 1 + 1 MIC.
#                  ECNR_2MIC_INEAR  : Inhouse ECNR to support 1 + 2 MIC.
#                  3RD_PARTY_AI_NR  : 3rd party AINR to support 1/2 MIC.
#                  3RD_PARTY_AI_NR_OFFLOAD  : 3rd party AINR to support 1 MIC with offload.
#                  3RD_PARTY_AI_NR_OFFLOAD_POST_EC  : 3rd party AINR to support 1 MIC with offload(dongle).
#                  3RD_PARTY_AI_NR_PRO_BROADSIDE_SEPARATE_MODE  : 3rd party AINR to support 1 MIC with separate EC/NR_PostEC.
#                  3RD_PARTY_AI_NR_SEPARATE_MODE_EC  : 3rd party AINR to support 1 MIC with separate EC only.
#                  3RD_PARTY_AI_NR_INEAR : 3rd party AINR to support 1/2 + 1 MIC.
#                  3RD_PARTY_AI_NR_SHORT_BOOM_OO : 3rd party AINR to support short boom mic for open office.
#                  3RD_PARTY_AI_NR_PRO_DISTRACTOR : 3rd party AINR to support pro distractor.
#                  3RD_PARTY_AI_NR_PRO_TWS_OO : 3rd party AINR to support pro tws for open office.
#                  3RD_PARTY_CUSTOMIZED : Customized NR.
#                  LD_NR : Inhouse LDNR(Low Delay NR)to support 1/2 MIC.
AIR_ECNR_CONFIG_TYPE = 3RD_PARTY_AI_NR_INEAR

######  Audio peripheral ######

# This option is to choose the type of dual chip mixing mode.
# Usage: master, slave, none
#                         master : The side which have HW key and control the partner. It supports ULL connection.
#                         slave  : The side which is controlled. It doesn't support ULL connection.
#                         none   : Not a master or salve.
# It must be set to the same value for both DSP and MCU, otherwise, it will not work.
AIR_DUAL_CHIP_MIXING_MODE = none

# This option is to choose the audio interface of dual chip mixing mode.
# This option is only valid when AIR_DUAL_CHIP_MIXING_MODE is not none.
# Usage: uart, i2s
#                         uart : The audio interface is UART.
#                         i2s  : The audio interface is I2S.
# It must be set to the same value for both DSP and MCU, otherwise, it will not work.
AIR_DUAL_CHIP_AUDIO_INTERFACE = i2s

# This option is used to  concurrently use AFE source.
AIR_MULTI_MIC_STREAM_ENABLE = y

# This option is for customized project audio path.
AIR_AUDIO_PATH_CUSTOMIZE_ENABLE = n

############################################
#               Internal use               #
############################################
