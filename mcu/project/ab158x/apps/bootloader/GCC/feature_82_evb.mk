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
IC_TYPE                               = ab1582p
BOARD_TYPE                            = ab1582p_evb
# debug level: none, error, warning, info and debug

MTK_HAL_PLAIN_LOG_ENABLE			  = y

# all in security & s/ns world option
AIR_LIMIT_TZ_ENABLE                   = n
# code run in S or NS world #s or ns
AIR_CPU_SECURITY_MODE                 = s

MTK_HAL_EXT_32K_ENABLE = n
MTK_NO_PSRAM_ENABLE = y


#*****
MTK_BL_LOAD_ENABLE          = y

#can modify
MTK_BL_FOTA_CUST_ENABLE     = n
MTK_BL_DEBUG_ENABLE         = y

# This option is used to enable FOTA basic function.
AIR_FOTA_ENABLE             = y

# This option is to enable FOTA via race cmd.
# Dependency: AIR_FOTA_ENABLE must be enabled, when this option is set to y.
AIR_FOTA_VIA_RACE_CMD_ENABLE       = y

MTK_FOTA_FS_ENABLE          = n
AIR_MBEDTLS_CONFIG_FILE     = config-vendor-bootloader.h
MTK_FOTA_STORE_IN_EXTERNAL_FLASH = n


# This option is used to enable/disable secure boot module.
# NOTICE:
#   - This option can only be enabled if the secure-boot-addon-repo exists.
#   - secure-boot-addon-repo path: mcu\prebuilt\middleware\airoha\secure_boot
#   - If this option enabled without secure boot addon repo, the code will build fail.
AIR_SECURITY_SECURE_BOOT_ENABLE = n


#internal use
MTK_BL_FPGA_LOAD_ENABLE     = y


#factory
MTK_CAL_DCXO_CAPID          = n
# DCXO calibration value is in SW
MTK_BL_DCXO_KVALUE_SW       = n

MTK_HAL_EXT_32K_ENABLE      = n

MTK_NO_PSRAM_ENABLE         = y

MTK_NVDM_ENABLE             = n
