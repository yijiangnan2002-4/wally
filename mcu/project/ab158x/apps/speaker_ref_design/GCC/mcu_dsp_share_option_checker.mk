# Copyright Statement:
#
# (C) 2023  Airoha Technology Corp. All rights reserved.
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

# MCD DSP shared feature option list
SHARED_FEATURE_OPTIONS :=   AIR_AUDIO_PLC_ENABLE \
                            AIR_ANC_USER_TRIGGER_ENABLE \
                            AIR_PEQ_ENABLE \
                            AIR_ANC_FIT_DETECTION_ENABLE \
                            AIR_ANC_WIND_NOISE_DETECTION_ENABLE \
                            AIR_USER_TRIGGER_FF_ENABLE \
                            AIR_ANC_USER_UNAWARE_ENABLE \
                            AIR_ANC_ENVIRONMENT_DETECTION_ENABLE \
                            AIR_AUDIO_TRANSMITTER_ENABLE \
                            AIR_HWSRC_CLKSKEW_ENABLE \
                            AIR_AUDIO_DETACHABLE_MIC_ENABLE \
                            AIR_HWSRC_RX_TRACKING_ENABLE \
                            AIR_HWSRC_TX_TRACKING_ENABLE \
                            AIR_VP_PEQ_ENABLE \
                            AIR_LINE_IN_AND_BT_MIX_ENABLE \
                            AIR_LINE_IN_ENABLE \
                            AIR_LINE_IN_PEQ_ENABLE \
                            AIR_LINE_IN_INS_ENABLE \
                            AIR_LINE_OUT_ENABLE \
                            AIR_USB_AUDIO_IN_ENABLE \
                            AIR_USB_AUDIO_IN_AND_OUT_MIX_ENABLE \
                            AIR_USB_AUDIO_OUT_ENABLE \
                            AIR_USB_AUDIO_IN_AND_BT_MIX_ENABLE \
                            AIR_VOLUME_ESTIMATOR_ENABLE \
                            AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE \
                            AIR_BT_HFP_ENABLE \
                            AIR_BT_A2DP_VENDOR_ENABLE \
                            AIR_BT_A2DP_LHDC_ENABLE \
                            AIR_BT_A2DP_AAC_ENABLE \
                            AIR_BT_A2DP_SBC_ENABLE \
                            AIR_AMA_HOTWORD_ENABLE \
                            AIR_GSOUND_HOTWORD_ENABLE \
                            AIR_LE_AUDIO_ENABLE \
                            AIR_LE_AUDIO_DONGLE_ENABLE \
                            AIR_ULL_GAMING_DONGLE_ENABLE \
                            AIR_ULL_GAMING_HEADSET_ENABLE \
                            AIR_ULL_BLE_HEADSET_ENABLE \
                            AIR_AUDIO_LC3PLUS_CODEC_ENABLE \
                            AIR_AUDIO_ULD_CODEC_ENABLE \
                            AIR_AUDIO_VEND_CODEC_ENABLE \
                            AIR_AUDIO_SILENCE_DETECTION_ENABLE \
                            AIR_WIRELESS_MIC_ENABLE \
                            AIR_BT_BLE_UL_SWB_ENABLE \
                            AIR_AUDIO_DUMP_ENABLE \
                            AIR_AIRDUMP_ENABLE \
                            AIR_ANC_ENABLE_TYPE \
                            AIR_PASSTHRU_ENABLE_TYPE \
                            AIR_UPLINK_RATE \
                            AIR_UPLINK_RESOLUTION \
                            AIR_DOWNLINK_RATE \
                            AIR_ECNR_CONFIG_TYPE \
                            AIR_DUAL_CHIP_MIXING_MODE \
                            AIR_DUAL_CHIP_AUDIO_INTERFACE \
                            AIR_ANC_ENABLE \
                            AIR_FULL_ADAPTIVE_ANC_ENABLE \
                            AIR_ADAPTIVE_EQ_ENABLE \
                            AIR_HYBRID_PT_ENABLE \
                            AIR_LINE_IN_MIX_ENABLE \
                            AIR_USB_AUDIO_IN_MIX_ENABLE \
                            AIR_ULL_AUDIO_V2_DONGLE_ENABLE \
                            AIR_BT_AUDIO_DONGLE_ENABLE \
                            AIR_WIRED_AUDIO_ENABLE \
                            AIR_DONGLE_AFE_IN_TYPE \
                            AIR_DONGLE_AFE_OUT_TYPE \
                            AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE \
                            AIR_RECORD_ADVANCED_ENABLE \

# Check the feature option is defined or not.
$(foreach VAR,$(SHARED_FEATURE_OPTIONS),$(if $(filter undefined,$(origin $(VAR))),\
    ,\
    $(info "$(VAR) is a MCU-DSP-Shared feature option but defined in MCU. Remove it from the MCU feature.mk and define it in the corresponding DSP feature.mk.")\
    $(error $(VAR) is a MCU-DSP-Shared feature option but defined in MCU. Remove it from the MCU feature.mk and define it in the corresponding DSP feature.mk.)\
))

