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
# Airoha restricted information


FATFS_ROOT = middleware/third_party/fatfs
FATFS_PORTABLE = middleware/third_party/fatfs/portable
FATFS_SRC = middleware/third_party/fatfs/ff14b/source

CFLAGS += -I$(SOURCE_DIR)/$(APP_PATH)/inc/

ifndef FATFS_VERSION
    include $(SOURCE_DIR)/$(FATFS_ROOT)/R0.12b/module.mk
    CFLAGS += -D__FATFS_VERSION__=201609L
    CFLAGS += -I$(SOURCE_DIR)/$(APP_PATH)/inc/fatfs_config/R0.12b/
else
    CFLAGS += -D__FATFS_VERSION__=202104L
    CFLAGS += -I$(SOURCE_DIR)/$(APP_PATH)/inc/fatfs_config/$(FATFS_VERSION)/
    #include $(SOURCE_DIR)/$(FATFS_ROOT)/$(FATFS_VERSION)/module.mk
endif


C_FILES  += $(FATFS_SRC)/ff.c
C_FILES  += $(FATFS_SRC)/ffunicode.c
C_FILES  += $(FATFS_SRC)/ffsystem.c

CFLAGS   += -fwide-exec-charset=UTF-16LE
CFLAGS   += -I$(SOURCE_DIR)/$(FATFS_SRC)


ifeq ($(PRODUCT_VERSION),2523)
ifeq ($(AIR_FOTA_ENABLE),y)
C_FILES +=     $(FATFS_PORTABLE)/mt2523/bootloader/src/diskio.c
else
C_FILES +=     $(FATFS_PORTABLE)/mt2523/src/diskio.c
C_FILES +=     $(FATFS_PORTABLE)/mt2523/src/diskio_snand.c
C_FILES +=     $(FATFS_PORTABLE)/mt2523/src/diskio_sd.c
endif
endif


ifeq ($(PRODUCT_VERSION),2533)
ifeq ($(AIR_FOTA_ENABLE),y)
C_FILES +=     $(FATFS_PORTABLE)/mt2533/bootloader/src/diskio.c
else
C_FILES +=     $(FATFS_PORTABLE)/mt2533/src/diskio.c
C_FILES +=     $(FATFS_PORTABLE)/mt2533/src/diskio_sd.c
C_FILES +=     $(FATFS_PORTABLE)/mt2533/src/diskio_snand.c
endif
endif


ifeq ($(PRODUCT_VERSION),7687)
C_FILES +=     $(FATFS_PORTABLE)/mt7687/src/diskio.c
C_FILES +=     $(FATFS_PORTABLE)/mt7687/src/diskio_sd.c
endif


ifeq ($(PRODUCT_VERSION),7697)
C_FILES +=     $(FATFS_PORTABLE)/mt7697/src/diskio.c
C_FILES +=     $(FATFS_PORTABLE)/mt7697/src/diskio_sd.c
endif

ifeq ($(PRODUCT_VERSION),7686)
C_FILES +=     $(FATFS_PORTABLE)/mt7686/src/diskio.c
C_FILES +=     $(FATFS_PORTABLE)/mt7686/src/diskio_sd.c
C_FILES +=     $(FATFS_PORTABLE)/mt7686/src/diskio_snor.c
endif

ifeq ($(PRODUCT_VERSION),7682)
C_FILES +=     $(FATFS_PORTABLE)/mt7682/src/diskio.c
C_FILES +=     $(FATFS_PORTABLE)/mt7682/src/diskio_sd.c
endif

ifeq ($(PRODUCT_VERSION),5932)
C_FILES +=     $(FATFS_PORTABLE)/mt5932/src/diskio.c
C_FILES +=     $(FATFS_PORTABLE)/mt5932/src/diskio_sd.c
endif

ifeq ($(PRODUCT_VERSION),1552)
C_FILES +=     $(FATFS_PORTABLE)/ab155x/src/diskio.c
C_FILES +=     $(FATFS_PORTABLE)/ab155x/src/diskio_sd.c
endif

ifeq ($(IC_CONFIG),am255x)
C_FILES +=      $(FATFS_PORTABLE)/am255x/src/diskio.c
C_FILES +=      $(FATFS_PORTABLE)/am255x/src/diskio_sd.c
endif

ifeq ($(IC_CONFIG),ab156x)
C_FILES +=      $(FATFS_PORTABLE)/ab156x/src/diskio.c
C_FILES +=      $(FATFS_PORTABLE)/ab156x/src/diskio_sd.c
endif

ifeq ($(PRODUCT_VERSION),7698)
C_FILES +=     $(FATFS_PORTABLE)/aw7698/src/diskio.c
C_FILES +=     $(FATFS_PORTABLE)/aw7698/src/diskio_sd.c
C_FILES +=     $(FATFS_PORTABLE)/aw7698/src/diskio_snor.c
endif

ifeq ($(PRODUCT_VERSION),2822)
C_FILES +=     $(FATFS_PORTABLE)/mt2822/src/diskio.c
C_FILES +=     $(FATFS_PORTABLE)/mt2822/src/diskio_sd.c
endif

ifeq ($(PRODUCT_VERSION),3335)
C_FILES +=     $(FATFS_PORTABLE)/ag3335/src/diskio.c
C_FILES +=     $(FATFS_PORTABLE)/ag3335/src/diskio_sd.c
C_FILES +=     $(FATFS_PORTABLE)/ag3335/src/diskio_snor.c
endif


ifdef AIR_BTA_IC_PREMIUM_G3
C_FILES +=     $(FATFS_PORTABLE)/bta_ic_g3/src/diskio.c
C_FILES +=     $(FATFS_PORTABLE)/bta_ic_g3/src/diskio_sd.c
endif



#################################################################################
# include path

CFLAGS     += -I$(SOURCE_DIR)/middleware/util/include
CFLAGS     += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/minicli/inc
CFLAGS  += -I$(SOURCE_DIR)/kernel/rtos/$(IC_CONFIG)/FreeRTOS/Source/include
#CFLAGS  += -I$(SOURCE_DIR)/kernel/rtos/FreeRTOS/Source/portable/GCC/ARM_CM4F
CFLAGS  += -I$(SOURCE_DIR)/kernel/service/inc
CFLAGS  += -I$(SOURCE_DIR)/driver/chip/inc
CFLAGS  += -I$(SOURCE_DIR)/middleware/third_party/fatfs/ff14b/source


ifeq ($(PRODUCT_VERSION),2523)
CFLAGS  += -I$(SOURCE_DIR)/$(FATFS_PORTABLE)/mt2523/inc
ifeq ($(AIR_FOTA_ENABLE),y)
CFLAGS  += -I$(SOURCE_DIR)/driver/board/mt25x3_hdk/bootloader/core/inc
endif
endif


ifeq ($(PRODUCT_VERSION),2533)
CFLAGS  += -I$(SOURCE_DIR)/$(FATFS_PORTABLE)/mt2533/inc
ifeq ($(AIR_FOTA_ENABLE),y)
CFLAGS  += -I$(SOURCE_DIR)/driver/board/mt25x3_hdk/bootloader/core/inc
endif
endif


ifeq ($(PRODUCT_VERSION),7687)
CFLAGS  += -I$(SOURCE_DIR)/$(FATFS_PORTABLE)/mt7687/inc
endif


ifeq ($(PRODUCT_VERSION),7697)
CFLAGS  += -I$(SOURCE_DIR)/$(FATFS_PORTABLE)/mt7697/inc
endif

ifeq ($(PRODUCT_VERSION),7686)
CFLAGS  += -I$(SOURCE_DIR)/$(FATFS_PORTABLE)/mt7686/inc
endif


ifeq ($(PRODUCT_VERSION),7682)
CFLAGS  += -I$(SOURCE_DIR)/$(FATFS_PORTABLE)/mt7682/inc
endif


ifeq ($(PRODUCT_VERSION),5932)
CFLAGS  += -I$(SOURCE_DIR)/$(FATFS_PORTABLE)/mt5932/inc
endif


ifeq ($(PRODUCT_VERSION),1552)
CFLAGS  += -I$(SOURCE_DIR)/$(FATFS_PORTABLE)/ab155x/inc
endif

ifeq ($(IC_CONFIG),am255x)
CFLAGS  += -I$(SOURCE_DIR)/$(FATFS_PORTABLE)/am255x/inc
endif

ifeq ($(IC_CONFIG),ab156x)
CFLAGS  += -I$(SOURCE_DIR)/$(FATFS_PORTABLE)/ab156x/inc
endif

ifeq ($(PRODUCT_VERSION),7698)
CFLAGS  += -I$(SOURCE_DIR)/$(FATFS_PORTABLE)/aw7698/inc
endif

ifeq ($(PRODUCT_VERSION),2822)
CFLAGS  += -I$(SOURCE_DIR)/$(FATFS_PORTABLE)/mt2822/inc
endif

ifeq ($(PRODUCT_VERSION),3335)
CFLAGS  += -I$(SOURCE_DIR)/$(FATFS_PORTABLE)/ag3335/inc
endif


ifdef AIR_BTA_IC_PREMIUM_G3
CFLAGS   += -I$(SOURCE_DIR)/$(FATFS_PORTABLE)/bta_ic_g3/inc
endif
