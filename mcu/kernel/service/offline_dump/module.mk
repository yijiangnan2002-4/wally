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

###################################################
# Sources
KERNEL_OFFLINE_DUMP_SRC    = kernel/service/offline_dump

C_FILES += $(KERNEL_OFFLINE_DUMP_SRC)/portable/src/offline_dump_port.c


# include $(SOURCE_DIR)/kernel/service_protected/offline_dump/GCC/module.mk
ifeq ($(AIR_BTA_IC_PREMIUM_G2),y)
LIBS += $(SOURCE_DIR)/prebuilt/system/chip/bta_ic_premium_g2/offline_dump/lib_offline_dump.a
else ifeq ($(AIR_BTA_IC_PREMIUM_G3),y)
LIBS += $(SOURCE_DIR)/prebuilt/system/chip/bta_ic_premium_g3/offline_dump/lib_offline_dump.a
else ifeq ($(AIR_BTA_IC_STEREO_HIGH_G3),y)
LIBS += $(SOURCE_DIR)/prebuilt/system/chip/bta_ic_stereo_high_g3/offline_dump/lib_offline_dump.a
endif

###################################################
# include path
CFLAGS += -I$(SOURCE_DIR)/kernel/service/offline_dump/inc
CFLAGS += -I$(SOURCE_DIR)/kernel/service/offline_dump/portable/inc
ifeq ($(AIR_BTA_IC_PREMIUM_G3),y)
CFLAGS += -I$(SOURCE_DIR)/kernel/service/exception_handler/portable/bta_ic_premium_g3
else
CFLAGS += -I$(SOURCE_DIR)/kernel/service/exception_handler/portable/bta_ic_premium_g2
endif