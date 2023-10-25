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

SOURCE_DIR = ../../../../..
BINPATH = $(SOURCE_DIR)/tools/gcc/gcc-arm-none-eabi/bin

PWD= $(shell pwd)
SDK_PATH    = $(abspath $(PWD)/$(SOURCE_DIR))

ifeq ($(FEATURE),)
$(info ==============================================================)
$(info )
$(info CAN NOT BUILD BOOTLOADER!!!!)
$(info bootloader FEATURE is empty!!!!)
$(info )
$(info If you want to co-build with main bin, please assign BL_FEATURE in main bin's feature.mk)
$(info ----- for example, add BL_FEATURE = feature_85_evb.mk  into main bin's feature.mk)
$(info )
$(info If you want to build bootloader only, please assign feature_xx_evb.mk for bootloader)
$(info ----- for example, ./build.sh ab158x bootloader -f=feature_85_evb.mk)
$(info )
$(info ==============================================================)
endif

# include feature_xx_evb.mk
include $(FEATURE)

# Gloabl Config
-include $(SOURCE_DIR)/.config
# IC Config
-include $(SOURCE_DIR)/config/chip/$(IC_CONFIG)/chip.mk
# Board Config
-include $(SOURCE_DIR)/config/board/$(BOARD_CONFIG)/board.mk
# HW Resource Assignment Config
ifeq ($(AIR_BTA_IC_PREMIUM_G2),y)
include $(SOURCE_DIR)/driver/board/$(BOARD_CONFIG)/hw_resource_assignment/module.mk
endif

export WITH_BOOTLOADER=n


BUILD_DIR = $(PWD)/Build

# Project name
#PROJ_NAME=$(IC_CONFIG)_bootloader
PROJ_NAME=bootloader
OUTPATH = $(PWD)/Build
PROJ_PATH = $(PWD)

APP_PATH        = $(patsubst $(SDK_PATH)/%,%,$(abspath $(dir $(PWD))))

# HAL driver files
include $(SOURCE_DIR)/driver/chip/$(IC_CONFIG)/module.mk

# lzma files
include $(SOURCE_DIR)/middleware/third_party/lzma_decoder/module.mk

# mbedtls files
include $(SOURCE_DIR)/middleware/third_party/mbedtls/module.mk

S_FILES += $(APP_PATH)/GCC/startup_bootloader_$(IC_CONFIG).s
C_FILES += $(APP_PATH)/GCC/syscalls.c
C_FILES += $(APP_PATH)/src/bl_dbgprint.c
C_FILES += $(APP_PATH)/src/custom_blconfig.c
C_FILES += $(APP_PATH)/src/bl_rom_parameter.c
C_FILES += $(APP_PATH)/src/main.c
C_FILES += $(APP_PATH)/src/sys_init.c

include $(SOURCE_DIR)/driver/board/component/bsp_flash/module.mk

# In order to provide the location of each Flash partition.
include $(SOURCE_DIR)/kernel/service/layout_partition/module.mk

# NVDM files
ifeq ($(MTK_NVDM_ENABLE),y)
include $(SOURCE_DIR)/middleware/airoha/nvdm/module.mk
CFLAGS += -I$(SOURCE_DIR)/prebuilt/middleware/airoha/nvdm/$(IC_CONFIG)/inc
endif

# boot time analyze
C_FILES += $(APP_PATH)/src/bl_boot_time_analyze.c


ifeq ($(AIR_FOTA_ENABLE),y)
ifeq ($(MTK_BL_FOTA_CUST_ENABLE),y)
C_FILES += $(APP_PATH)/src/custom_fota.c
else
ifeq ($(AIR_FOTA_VIA_RACE_CMD_ENABLE),y)
# NEW_FOTA
include $(SOURCE_DIR)/driver/board/component/bsp_flash/module.mk

include $(SOURCE_DIR)/middleware/airoha/fota/module.mk
C_FILES += $(APP_PATH)/src/bl_fota_util.c
C_FILES += $(APP_PATH)/src/bl_fota_flash_ctrl.c
C_FILES += $(APP_PATH)/src/bl_fota_upgrade.c
CFLAGS  += -DMOD_CFG_FOTA_DISABLE_OS
CFLAGS  += -DBL_FOTA_ENABLE
CFLAGS  += -DBL_FOTA_DEBUG
CFLAGS += -I$(SOURCE_DIR)/middleware/airoha/fota/inc/race
CFLAGS += -I$(SOURCE_DIR)/middleware/airoha/fota/inc/internal
else
C_FILES += $(APP_PATH)/src/bl_fota.c
C_FILES += middleware/third_party/mbedtls/library/sha1.c
CFLAGS += -DMBEDTLS_CONFIG_FILE='<config-mtk-sha1.h>'
endif
endif
endif

# FOTA partion table
AIR_FOTA_MEM_LAYOUT_CHANGE_ENABLE = n

# fatfs files
ifeq ($(MTK_FOTA_FS_ENABLE),y)
include $(SOURCE_DIR)/middleware/third_party/fatfs/module.mk
endif

ifeq ($(MTK_BL_DCXO_KVALUE_SW),y)
CFLAGS += -DBL_DCXO_KVALUE_SW
endif

ifeq ($(MTK_CAL_DCXO_CAPID),0)
CFLAGS += -DBL_RUN_DCXO_CAL
  ifneq ($(MTK_BL_DCXO_KVALUE_SW),y)
  CFLAGS += -DBL_CAL_DCXO_CAPID0
  endif
endif

ifeq ($(MTK_CAL_DCXO_CAPID),1)
CFLAGS += -DBL_RUN_DCXO_CAL
  ifneq ($(MTK_BL_DCXO_KVALUE_SW),y)
  CFLAGS += -DBL_CAL_DCXO_CAPID1
  endif
endif

ifeq ($(MTK_CAL_DCXO_CAPID),2)
CFLAGS += -DBL_RUN_DCXO_CAL
  ifneq ($(MTK_BL_DCXO_KVALUE_SW),y)
  CFLAGS += -DBL_CAL_DCXO_CAPID2
  endif
endif

ifeq ($(AIR_BL_CACHE_ENABLE),y)
CFLAGS += -DAIR_BL_CACHE_ENABLE
endif

# Secure Boot
ifeq ($(AIR_SECURITY_SECURE_BOOT_ENABLE),y)
	ifneq ($(wildcard $(strip $(SOURCE_DIR))/prebuilt/middleware/airoha/secure_boot/$(IC_CONFIG)/),)
		MTK_BOOTLOADER_USE_MBEDTLS  = y
		AIR_MBEDTLS_CONFIG_FILE     = config-vendor-secure_boot.h
        ifeq ($(AIR_BTA_IC_PREMIUM_G3),y)
            LIBS += $(SOURCE_DIR)/prebuilt/middleware/airoha/secure_boot/$(IC_CONFIG)/libsboot_gen3_CM33_GCC.a
        else ifeq ($(AIR_BTA_IC_PREMIUM_G2), y)
            LIBS += $(SOURCE_DIR)/prebuilt/middleware/airoha/secure_boot/$(IC_CONFIG)/libsboot_gen2_CM4_GCC.a
        else ifeq ($(AIR_BTA_IC_STEREO_HIGH_G3), y)
            LIBS += $(SOURCE_DIR)/prebuilt/middleware/airoha/secure_boot/$(IC_CONFIG)/libsboot_gen3_stereohigh_CM4_GCC.a
        else

        endif
		CFLAGS += -DMTK_SECURE_BOOT_ENABLE
		CFLAGS += -I$(SOURCE_DIR)/prebuilt/middleware/airoha/secure_boot/$(IC_CONFIG)/inc
	else
	    # Secure Boot addon repo doesn't exist. MakeFile will build fail during checkoption
	endif
endif

# anti rollback
ifeq ($(AIR_ANTI_ROLLBACK_ENABLE),y)
	CFLAGS += -DAIR_ANTI_ROLLBACK_ENABLE
    CFLAGS += -I$(SOURCE_DIR)/prebuilt/middleware/airoha/secure_boot/$(IC_CONFIG)/inc
endif

# mbedtls
ifeq ($(MTK_BOOTLOADER_USE_MBEDTLS),y)
	include $(SOURCE_DIR)/middleware/third_party/mbedtls/module.mk
	CFLAGS += -DMTK_BOOTLOADER_USE_MBEDTLS
endif

# BL DFU
ifeq ($(AIR_BL_DFU_ENABLE),y)
    C_FILES += $(APP_PATH)/src/bl_dfu.c
    include $(SOURCE_DIR)/middleware/airoha/fota/module.mk
    ifeq ($(AIR_BL_DFU_GPIO_CTRL_ENABLE),y)
        CFLAGS += -DAIR_BL_DFU_GPIO_CTRL_ENABLE
    endif
endif

# USB HID for BL DFU
ifeq ($(AIR_BL_USB_HID_DFU_ENABLE),y)
    AIR_USB_ENABLE              = y
    AIR_USB_HID_ENABLE          = y
    include $(SOURCE_DIR)/middleware/airoha/usb/module.mk

    CFLAGS += -DAIR_BL_USB_HID_DFU_ENABLE
    C_FILES += $(APP_PATH)/src/bl_usb_hid.c
endif

#race cmd for bootloader DFU
ifeq ($(AIR_BL_USB_HID_DFU_ENABLE),y)
include $(SOURCE_DIR)/middleware/airoha/race_cmd/module.mk
endif

COM_CFLAGS += -DMTK_HAL_PLAIN_LOG_ENABLE


C_OBJS = $(C_FILES:%.c=$(BUILD_DIR)/%.o)
S_OBJS = $(S_FILES:%.s=$(BUILD_DIR)/%.o)

###################################################

# Check for valid float argument
# NOTE that you have to run make clan after
# changing these as hardfloat and softfloat are not
# binary compatible
ifneq ($(FLOAT_TYPE), hard)
ifneq ($(FLOAT_TYPE), soft)
override FLOAT_TYPE = hard
#override FLOAT_TYPE = soft
endif
endif

###################################################
# CC Flags
CFLAGS += $(ALLFLAGS) -flto -ffunction-sections -fdata-sections -fno-builtin

ifeq ($(FLOAT_TYPE), hard)
FPUFLAGS = -fsingle-precision-constant -Wdouble-promotion
FPUFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=hard
#CFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=softfp
else
FPUFLAGS = -msoft-float
endif

# Definitions
CFLAGS += $(FPUFLAGS)

ifeq ($(MTK_BL_FPGA_LOAD_ENABLE),y)
CFLAGS += -D__FPGA_TARGET__
endif

ifeq ($(MTK_BL_DEBUG_ENABLE),y)
CFLAGS += -DBL_DEBUG
endif

ifeq ($(AIR_FOTA_ENABLE),y)
CFLAGS += -DBL_FOTA_ENABLE
CFLAGS += -DMOD_CFG_FOTA_BL_RESERVED
endif

ifeq ($(MTK_NO_PSRAM_ENABLE),y)
CFLAGS += -DMTK_NO_PSRAM_ENABLE
endif

ifeq ($(MTK_FOTA_EXTERNEL_FLASH),y)
CFLAGS += -DFOTA_EXTERNAL_FLASH_SUPPORT
endif

ifeq ($(MTK_BL_LOAD_ENABLE),y)
CFLAGS += -DBL_LOAD_ENABLE
endif

ifeq ($(AIR_BOOT_TIME_MEASURE_ENABLE),y)
CFLAGS += -DAIR_BOOT_TIME_MEASURE_ENABLE
endif

ifdef AIR_BL_USB_TIMEOUT_IN_MS
CFLAGS += -DAIR_BL_USB_TIMEOUT_IN_MS=$(AIR_BL_USB_TIMEOUT_IN_MS)
endif


CFLAGS += -D__UBL__ -D__EXT_BOOTLOADER__
CFLAGS += -DAIR_CPU_IN_SECURITY_MODE

CFLAGS+=-DOTA_ENC_KEY="{0xf,0xe,0xd,0xc,0xb,0xa,9,8,7,6,5,4,3,2,1,0}"
CFLAGS += -DOTA_ENC_IV="{\\'c\\',\\'7\\',\\'8\\',\\'2\\',\\'d\\',\\'c\\',\\'4\\',\\'c\\',\\'0\\',\\'9\\',\\'8\\',\\'c\\',\\'6\\',\\'6\\',\\'c\\',\\'b\\'}"


# LD Flags
LSCRIPT      := tcm_$(IC_CONFIG).ld
LDFLAGS = $(ALLFLAGS) $(FPUFLAGS) --specs=nano.specs -lnosys -nostartfiles
LDFLAGS += -Wl,-T$(LSCRIPT) -Wl,--gc-sections
ifeq ($(DEBUG), 1)
LDFLAGS += --specs=rdimon.specs -lrdimon
endif
#LDFLAGS = $(CFLAGS)

LINKER_SCRIPT_PATH = $(SOURCE_DIR)/$(APP_PATH)/GCC/$(LSCRIPT)
# Auto generate flash_download.cfg file
FLASH_DOWNLOAD_CFG_GENERATOR := $(SOURCE_DIR)/tools/scripts/build/auto_download_cfg.sh

###################################################
#CFLAGS += -I$(SOURCE_DIR)/$(RTOS_SRC)/portable/GCC/ARM_CM4F
CFLAGS += -I$(SOURCE_DIR)/driver/CMSIS/Device/airoha/$(IC_CONFIG)/Include
CFLAGS += -I$(SOURCE_DIR)/driver/CMSIS/Source/$(IC_CONFIG)/Include
CFLAGS += -I$(SOURCE_DIR)/$(APP_PATH)/inc
CFLAGS += -I$(SOURCE_DIR)/middleware/third_party/mbedtls/include
CFLAGS += -I$(SOURCE_DIR)/middleware/third_party/lzma_decoder/inc
ifeq ($(AIR_BTA_IC_PREMIUM_G3),y)
CFLAGS += -mcmse
endif
###################################################
# Rules
.PHONY: proj clean $(MODULE_PATH)

all: cleanlog checkoption proj
	@mkdir -p $(BUILD_DIR)
	@$(SIZE) $(OUTPATH)/$(PROJ_NAME).elf
	@$(FLASH_DOWNLOAD_CFG_GENERATOR) $(LINKER_SCRIPT_PATH) $(OUTPATH) $(IC_CONFIG) BL
	@$(SOURCE_DIR)/tools/scripts/build/copy_firmware.sh $(SOURCE_DIR) $(OUTPATH) $(IC_CONFIG) $(BOARD_CONFIG) $(PROJ_NAME).bin $(PWD) $(AIR_SECURITY_SECURE_BOOT_ENABLE)
MOD_EXTRA = BUILD_DIR=$(BUILD_DIR) OUTPATH=$(OUTPATH) PROJ_PATH=$(PROJ_PATH)

checkoption:
# Add make error for Secure Boot enabled, but do not exist Secure Boot prebuilt library.
ifeq ($(AIR_SECURITY_SECURE_BOOT_ENABLE),y)
ifeq ($(wildcard $(strip $(SOURCE_DIR))/prebuilt/middleware/airoha/secure_boot/$(IC_CONFIG)/),)
	$(warning "Error: [Addon error] The secure boot addon package doesn't exist. Please contact Airoha to get this addon package. ")
	@echo "[Addon error] The secure boot addon package doesn't exist. Please contact Airoha to get this addon package.";
	@set -e; exit 2;
endif
endif

ifeq ($(AIR_ANTI_ROLLBACK_ENABLE),y)
ifeq ($(AIR_SECURITY_SECURE_BOOT_ENABLE),n)
	$(warning "Error: [Conflict feature option] To enable AIR_ANTI_ROLLBACK_ENABLE must support AIR_SECURITY_SECURE_BOOT_ENABLE ")
	@echo "Error: [Conflict feature option] To enable AIR_ANTI_ROLLBACK_ENABLE must support AIR_SECURITY_SECURE_BOOT_ENABLE";
	@set -e; exit 2;
endif
endif

$(LIBS): $(MODULE_PATH)

$(MODULE_PATH):
	@+make -C $@ $(MOD_EXTRA) $($@_EXTRA)

proj: $(OUTPATH)/$(PROJ_NAME).elf

$(OUTPATH)/$(PROJ_NAME).elf: $(C_OBJS) $(S_OBJS) $(LIBS)
	@echo Linking...
	@if [ -e "$@" ]; then rm -f "$@"; fi
	@if [ -e "$(OUTPATH)/$(PROJ_NAME).map" ]; then rm -f "$(OUTPATH)/$(PROJ_NAME).map"; fi
	@if [ -e "$(OUTPATH)/$(PROJ_NAME).dis" ]; then rm -f "$(OUTPATH)/$(PROJ_NAME).dis"; fi
	@if [ -e "$(OUTPATH)/$(PROJ_NAME).hex" ]; then rm -f "$(OUTPATH)/$(PROJ_NAME).hex"; fi
	@if [ -e "$(OUTPATH)/$(PROJ_NAME).bin" ]; then rm -f "$(OUTPATH)/$(PROJ_NAME).bin"; fi
	@$(CC) $(LDFLAGS) -Wl,--start-group $^ -Wl,--end-group -Wl,-Map=$(OUTPATH)/$(PROJ_NAME).map -lm -o $@ 2>>$(ERR_LOG)
	@$(OBJDUMP) -D $(OUTPATH)/$(PROJ_NAME).elf > $(OUTPATH)/$(PROJ_NAME).dis
	@$(OBJCOPY) -O ihex $(OUTPATH)/$(PROJ_NAME).elf $(OUTPATH)/$(PROJ_NAME).hex
	@$(OBJCOPY) -O binary $(OUTPATH)/$(PROJ_NAME).elf $(OUTPATH)/$(PROJ_NAME).bin
	@echo Done

ifeq ($(AIR_SECURITY_SECURE_BOOT_ENABLE), y)
	@$(SOURCE_DIR)/tools/security/secure_boot/scripts/build/build_sboot_header.sh $(SOURCE_DIR) $(OUTPATH) $(BOARD_CONFIG) $(PROJ_NAME) $(IC_CONFIG)
endif

include $(SOURCE_DIR)/.rule.mk

clean:
	rm -rf $(OUTPATH)


