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

SOURCE_DIR = ../../../../..
LIB_COMPARE_SCRIPT=$(SOURCE_DIR)/tools/airoha/genlib/iot_lib_compare/compare_lib_iot.pl

CONFIG_PATH ?= .

CFLAGS += -I$(SOURCE_DIR)/$(CONFIG_PATH)

# Gloabl Config
-include $(SOURCE_DIR)/.config
# IC Config
-include $(SOURCE_DIR)/config/chip/mt2523/chip.mk
-include $(SOURCE_DIR)/config/chip/ab155x/chip.mk
-include $(SOURCE_DIR)/config/chip/am255x/chip.mk
-include $(SOURCE_DIR)/config/chip/ag3335/chip.mk
-include $(SOURCE_DIR)/config/chip/ab156x/chip.mk
-include $(SOURCE_DIR)/config/chip/mt2822/chip.mk
-include $(SOURCE_DIR)/config/chip/$(BOARD)/chip.mk

# Board Config
-include $(SOURCE_DIR)/config/board/mt2523_hdk/board.mk

# Project name
TARGET_LIB=libnvdm_CM4_GCC

BUILD_DIR = Build
OUTPATH = Build


# Sources
include module.mk
C_OBJS = $(C_FILES:%.c=$(BUILD_DIR)/%.o)


.PHONY: all $(TARGET_LIB).a

all: $(TARGET_LIB).a
	@echo Build $< Done


include $(SOURCE_DIR)/.rule.mk

clean:
	rm -rf $(OUTPATH)/$(TARGET_LIB).a $(BUILD_DIR)

prebuilt_install:
	@mkdir -p $(SOURCE_DIR)/prebuilt/$(MIDDLEWARE_PROPRIETARY)/nvdm/$(IC_CONFIG)/lib
	@if [ -f "${LIB_COMPARE_SCRIPT}" ]; then \
		echo "File ${LIB_COMPARE_SCRIPT} exist"; \
		perl ${LIB_COMPARE_SCRIPT} $(OUTPATH)/$(TARGET_LIB).a $(SOURCE_DIR)/prebuilt/$(MIDDLEWARE_PROPRIETARY)/nvdm/$(IC_CONFIG)/lib/$(TARGET_LIB).a $(BUILD_DIR) > $(BUILD_DIR)/lib.log;\
		if [ "$$?" == "0" ]; then \
			echo "Lib $(OUTPATH)/$(TARGET_LIB).a $(SOURCE_DIR)/prebuilt/$(MIDDLEWARE_PROPRIETARY)/nvdm/$(IC_CONFIG)/lib/$(TARGET_LIB).a is the same!"; \
		else \
			echo "Lib $(OUTPATH)/$(TARGET_LIB).a $(SOURCE_DIR)/prebuilt/$(MIDDLEWARE_PROPRIETARY)/nvdm/$(IC_CONFIG)/lib/$(TARGET_LIB).a is different!"; \
	                cp $(OUTPATH)/$(TARGET_LIB).a $(SOURCE_DIR)/prebuilt/$(MIDDLEWARE_PROPRIETARY)/nvdm/$(IC_CONFIG)/lib/$(TARGET_LIB).a;\
		fi;\
	else \
		echo "File ${LIB_COMPARE_SCRIPT} not exist"; \
	        cp $(OUTPATH)/$(TARGET_LIB).a $(SOURCE_DIR)/prebuilt/$(MIDDLEWARE_PROPRIETARY)/nvdm/$(IC_CONFIG)/lib/$(TARGET_LIB).a;\
	fi;
