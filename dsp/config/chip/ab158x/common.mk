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
BOARD_CONFIG                          = ab158x_evb
IC_TYPE                               = ab1585
BOARD_TYPE                            = ab1585_evb

# debug level: none, error, warning, info and debug
MTK_DEBUG_LEVEL                       = info

# let syslog dump to flash
MTK_SAVE_LOG_TO_FLASH_ENABLE          = n

# heap peak profiling
MTK_HEAP_SIZE_GUARD_ENABLE            = n

###################################################
# bt at command
MTK_BT_AT_COMMAND_ENABLE = y

# port service
MTK_PORT_SERVICE_ENABLE = y

#SWLA
MTK_SWLA_ENABLE                       = n
# heap dump
MTK_SUPPORT_HEAP_DEBUG                = y

MTK_USB_DEMO_ENABLED                  = n

#NVDM feature
MTK_NVDM_ENABLE = y

# bt codec enable
MTK_BT_CODEC_ENABLED = y

# BT A2DP codec AAC support
MTK_BT_A2DP_AAC_ENABLE = y


# BT A2DP codec vendor support
VENDOR_LIB = $(strip $(ROOTDIR))/prebuilt/middleware/third_party/dspalg/vendor_decoder/vend.flag
ifeq ($(VENDOR_LIB), $(wildcard $(VENDOR_LIB)))
MTK_BT_A2DP_VENDOR_ENABLE =  y
MTK_BT_A2DP_VENDOR_BC_ENABLE = y
endif

# Load Clk Skew Lib From Source Code or Object File
CLKSKEW_LIB = $(strip $(ROOTDIR))/middleware/airoha/dspalg/clk_skew_protected/module.mk
ifeq ($(CLKSKEW_LIB), $(wildcard $(CLKSKEW_LIB)))
MTK_BT_CLK_SKEW_LOAD_ENABLE =  y
else
MTK_BT_CLK_SKEW_LOAD_ENABLE =  n
endif

# BT A2DP codec SRC support
MTK_SRC_ENABLE = n

# avm direct feature
MTK_AVM_DIRECT                       = y

# BT Dual mode
MTK_BT_DUO_ENABLE = y

# bt module enable
MTK_BT_ENABLE                       = y
MTK_BLE_ONLY_ENABLE                 = n
MTK_BT_HFP_ENABLE                   = y
MTK_BT_HFP_FORWARDER_ENABLE         = y
MTK_BT_AVRCP_ENABLE                 = y
MTK_BT_AVRCP_ENH_ENABLE             = y
MTK_BT_A2DP_ENABLE                  = y
MTK_BT_PBAP_ENABLE                  = n
MTK_BT_SPP_ENABLE                   = y
MTK_BT_AVM_SHARE_BUF                = y
# aws earbuds feature
MTK_AWS                             = n

#BT external timer
MTK_BT_TIMER_EXTERNAL_ENABLE = y
MTK_PORT_SERVICE_ENABLE               = y

#DSP Audio Message
MTK_DSP_AUDIO_MESSAGE_ENABLE        = y

# CM4 playback module
MTK_CM4_PLAYBACK_ENABLE = y

# CM4 record module
MTK_CM4_RECORD_ENABLE = y

# WWE module
MTK_WWE_ENABLE = y

# Sensor Source module
MTK_SENSOR_SOURCE_ENABLE = y

# Voice Prompt module
MTK_PROMPT_SOUND_ENABLE = y

# I2S Slave module
MTK_I2S_SLAVE_ENABLE = n

# ANC module
MTK_ANC_ENABLE = y
ifeq ($(MTK_ANC_ENABLE), y)
ANC_SOURCE_CODE = $(strip $(ROOTDIR))/middleware/airoha/dspfw/anc_protected/port/chip/mt2822/module.mk
ifeq ($(ANC_SOURCE_CODE), $(wildcard $(ANC_SOURCE_CODE)))
MTK_ANC_LIB_ENABLE =  n
else
MTK_ANC_LIB_ENABLE =  y
endif
endif

# PEQ module
MTK_PEQ_ENABLE = y

# Audio Dump
MTK_AUDIO_DUMP_BY_CONFIGTOOL = n

# AirDump module
AIR_AIRDUMP_ENABLE = y

# For 1568 ALG interface (compander/clkskew)
MTK_BT_HFP_SPE_ALG_V2 = y

# Use HWSRC to do clkskew
ENABLE_HWSRC_CLKSKEW = y

# AGC module
MTK_VOICE_AGC_ENABLE = y

# For 3rd party NR
AIR_3RD_PARTY_NR_ENABLE = n
ifeq ($(AIR_3RD_PARTY_NR_ENABLE),y)
        AIR_AIRDUMP_ENABLE = n
endif

PRELOADER_ENABLE = y
        DSP0_PISPLIT_DEMO_LIBRARY = n
        MTK_BT_A2DP_AAC_USE_PIC = y
        MTK_BT_A2DP_SBC_USE_PIC = y
        MTK_BT_A2DP_MSBC_USE_PIC = y
        MTK_BT_A2DP_CVSD_USE_PIC = n
        MTK_BT_A2DP_CPD_USE_PIC = y
        AIR_VOICE_NR_USE_PIC_ENABLE = y
        MTK_BT_CLK_SKEW_USE_PIC = y
ifeq ($(MTK_VOICE_AGC_ENABLE),y)
		MTK_BT_AGC_USE_PIC = y
endif
ifeq ($(MTK_BT_A2DP_VENDOR_ENABLE),y)
        MTK_BT_A2DP_VENDOR_USE_PIC = n
endif
ifeq ($(MTK_PEQ_ENABLE),y)
        MTK_BT_PEQ_USE_PIC = y
endif
        MTK_PLC_USE_PIC = y
    PRELOADER_ENABLE_DSP0_LOAD_FOR_DSP1 = n
        DSP1_PISPLIT_DEMO_LIBRARY = n
ifeq ($(AIR_ADAPTIVE_EQ_ENABLE),y)
        AIR_ADAPITVE_EQ_DETECTION_USE_PIC_ENABLE = y
endif
# Low power level
MTK_LOWPOWER_LEVEL = 2

# HWSRC for DL1
ENABLE_HWSRC_ON_MAIN_STREAM = y

# AMP delay timer
ENABLE_AMP_TIMER = y

# SideTone Gain Ramp
ENABLE_SIDETONE_RAMP_TIMER = y
# DEFAULT ENABLE DSP HW LOOPBACK
AB1568_BRING_UP_DSP_DEFAULT_HW_LOOPBACK = n

# 2a2d
MTK_AUDIO_SUPPORT_MULTIPLE_MICROPHONE = n

# 1+1 inear
AIR_ECNR_1MIC_INEAR_ENABLE = n
ifeq ($(AIR_ECNR_1MIC_INEAR_ENABLE), y)
MTK_AUDIO_SUPPORT_MULTIPLE_MICROPHONE = y
endif

# 2+1 Dual mic inear
AIR_ECNR_2MIC_INEAR_ENABLE = n
ifeq ($(AIR_ECNR_2MIC_INEAR_ENABLE), y)
MTK_AUDIO_SUPPORT_MULTIPLE_MICROPHONE = y
endif

# Audio PLC
MTK_AUDIO_PLC_ENABLE = n
# afe verification
LINE_IN_PURE_FOR_AMIC_CLASS_G_HQA = y

# Multiple Mic Source
AIR_MULTI_MIC_STREAM_ENABLE = n

# Audio transmitter
AIR_AUDIO_TRANSMITTER_ENABLE = y
