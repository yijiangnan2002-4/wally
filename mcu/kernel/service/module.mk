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
####                  3335                    #####
###################################################
ifeq ($(PRODUCT_VERSION),3335)
###################################################
# Sources
KERNEL_SERVICE_SRC    = kernel/service/src

KERNEL_SERVICE_FILES += $(KERNEL_SERVICE_SRC)/airoha_HeapSizeGuard.c \
                        $(KERNEL_SERVICE_SRC)/os_cli.c \
                        $(KERNEL_SERVICE_SRC)/os_port_callback.c \
                        $(KERNEL_SERVICE_SRC)/os_trace_callback.c \
                        $(KERNEL_SERVICE_SRC)/toi.c \
                        $(KERNEL_SERVICE_SRC)/utils.c \

ifeq ($(MTK_MUX_ENABLE), y)
include  $(SOURCE_DIR)/kernel/service/mux/module.mk
endif
include  $(SOURCE_DIR)/kernel/service/syslog/module.mk
include  $(SOURCE_DIR)/kernel/service/exception_handler/module.mk
include  $(SOURCE_DIR)/kernel/service/offline_dump/module.mk
include  $(SOURCE_DIR)/kernel/service/swla/module.mk
include  $(SOURCE_DIR)/kernel/service/systemhang_tracer/module.mk
ifeq ($(MTK_BOOTREASON_CHECK_ENABLE), y)
include  $(SOURCE_DIR)/kernel/service/bootreason_check/module.mk
endif
include  $(SOURCE_DIR)/kernel/service/memory_monitor/module.mk

C_FILES += $(KERNEL_SERVICE_FILES)

###################################################
# include path
CFLAGS += -I$(SOURCE_DIR)/kernel/service/inc
CFLAGS += -I$(SOURCE_DIR)/kernel/rtos/FreeRTOS/Source/include
CFLAGS += -I$(SOURCE_DIR)/middleware/airoha/minicli/inc

###################################################
####       AIR_BTA_IC_PREMIUM_G2              #####
###################################################
else ifeq ($(AIR_BTA_IC_PREMIUM_G2),y)
###################################################
# Sources
KERNEL_SERVICE_SRC    = kernel/service/src

###################################################
# include path
CFLAGS += -I$(SOURCE_DIR)/kernel/service/inc
CFLAGS += -I$(SOURCE_DIR)/kernel/service/exception_handler/inc/bta_ic_premium_g2
CFLAGS += -I$(SOURCE_DIR)/kernel/rtos/$(IC_CONFIG)/FreeRTOS/Source/include
CFLAGS += -I$(SOURCE_DIR)/middleware/airoha/minicli/inc

KERNEL_SERVICE_FILES += $(KERNEL_SERVICE_SRC)/airoha_HeapSizeGuard.c \
                        $(KERNEL_SERVICE_SRC)/os_cli.c \
                        $(KERNEL_SERVICE_SRC)/os_port_callback.c \
                        $(KERNEL_SERVICE_SRC)/os_trace_callback.c \
                        $(KERNEL_SERVICE_SRC)/toi.c \
                        $(KERNEL_SERVICE_SRC)/utils.c \

ifeq ($(MTK_MUX_ENABLE), y)
include  $(SOURCE_DIR)/kernel/service/mux/module.mk
endif
include  $(SOURCE_DIR)/kernel/service/syslog/module.mk
include  $(SOURCE_DIR)/kernel/service/exception_handler/module.mk
include  $(SOURCE_DIR)/kernel/service/offline_dump/module.mk
include  $(SOURCE_DIR)/kernel/service/swla/module.mk
include  $(SOURCE_DIR)/kernel/service/systemhang_tracer/module.mk
AIR_BOOTREASON_CHECK_ENABLE ?= $(MTK_BOOTREASON_CHECK_ENABLE)
ifeq ($(AIR_BOOTREASON_CHECK_ENABLE), y)
include  $(SOURCE_DIR)/kernel/service/bootreason_check/module.mk
endif
include  $(SOURCE_DIR)/kernel/service/system_daemon/module.mk
include  $(SOURCE_DIR)/kernel/service/memory_monitor/module.mk
include  $(SOURCE_DIR)/kernel/service/clib_optimization/module.mk

C_FILES += $(KERNEL_SERVICE_FILES)

ifeq ($(CCCI_ENABLE), y)
include  $(SOURCE_DIR)/kernel/service/ccci/module.mk
endif


###################################################
####                  1552                    #####
###################################################
else ifeq ($(PRODUCT_VERSION),1552)
###################################################
# Sources
KERNEL_SERVICE_SRC    = kernel/service/src

KERNEL_SERVICE_FILES += $(KERNEL_SERVICE_SRC)/airoha_HeapSizeGuard.c \
                        $(KERNEL_SERVICE_SRC)/os_cli.c \
                        $(KERNEL_SERVICE_SRC)/os_port_callback.c \
                        $(KERNEL_SERVICE_SRC)/os_trace_callback.c \
                        $(KERNEL_SERVICE_SRC)/toi.c \
                        $(KERNEL_SERVICE_SRC)/utils.c \


include  $(SOURCE_DIR)/kernel/service/syslog/module.mk
include  $(SOURCE_DIR)/kernel/service/exception_handler/module.mk
include  $(SOURCE_DIR)/kernel/service/offline_dump/module.mk
include  $(SOURCE_DIR)/kernel/service/swla/module.mk
include  $(SOURCE_DIR)/kernel/service/systemhang_tracer/module.mk
ifeq ($(MTK_BOOTREASON_CHECK_ENABLE), y)
include  $(SOURCE_DIR)/kernel/service/bootreason_check/module.mk
endif
include  $(SOURCE_DIR)/kernel/service/system_daemon/module.mk
include  $(SOURCE_DIR)/kernel/service/memory_monitor/module.mk

C_FILES += $(KERNEL_SERVICE_FILES)

###################################################
# include path
CFLAGS += -I$(SOURCE_DIR)/kernel/service/inc
CFLAGS += -I$(SOURCE_DIR)/kernel/rtos/FreeRTOS/Source/include
CFLAGS += -I$(SOURCE_DIR)/middleware/airoha/minicli/inc


###################################################
####                  2552                    #####
###################################################
else ifeq ($(PRODUCT_VERSION),2552)
###################################################
# Sources
KERNEL_SERVICE_SRC    = kernel/service/src

KERNEL_SERVICE_FILES += $(KERNEL_SERVICE_SRC)/airoha_HeapSizeGuard.c \
                        $(KERNEL_SERVICE_SRC)/os_cli.c \
                        $(KERNEL_SERVICE_SRC)/os_port_callback.c \
                        $(KERNEL_SERVICE_SRC)/os_trace_callback.c \
                        $(KERNEL_SERVICE_SRC)/toi.c \
                        $(KERNEL_SERVICE_SRC)/utils.c \


include  $(SOURCE_DIR)/kernel/service/syslog/module.mk
include  $(SOURCE_DIR)/kernel/service/exception_handler/module.mk
include  $(SOURCE_DIR)/kernel/service/offline_dump/module.mk
include  $(SOURCE_DIR)/kernel/service/swla/module.mk
include  $(SOURCE_DIR)/kernel/service/systemhang_tracer/module.mk
ifeq ($(MTK_BOOTREASON_CHECK_ENABLE), y)
include  $(SOURCE_DIR)/kernel/service/bootreason_check/module.mk
endif
include  $(SOURCE_DIR)/kernel/service/system_daemon/module.mk
include  $(SOURCE_DIR)/kernel/service/memory_monitor/module.mk

C_FILES += $(KERNEL_SERVICE_FILES)

###################################################
# include path
CFLAGS += -I$(SOURCE_DIR)/kernel/service/inc
CFLAGS += -I$(SOURCE_DIR)/kernel/rtos/FreeRTOS/Source/include
CFLAGS += -I$(SOURCE_DIR)/middleware/airoha/minicli/inc


###################################################
####          AIR_BTA_IC_PREMIUM_G3           #####
###################################################
else ifeq ($(AIR_BTA_IC_PREMIUM_G3),y)
###################################################
# Sources
KERNEL_SERVICE_SRC    = kernel/service/src

KERNEL_SERVICE_FILES += $(KERNEL_SERVICE_SRC)/airoha_HeapSizeGuard.c \
                        $(KERNEL_SERVICE_SRC)/os_cli.c \
                        $(KERNEL_SERVICE_SRC)/os_port_callback.c \
                        $(KERNEL_SERVICE_SRC)/os_trace_callback.c \
                        $(KERNEL_SERVICE_SRC)/toi.c \
                        $(KERNEL_SERVICE_SRC)/utils.c \

ifeq ($(MTK_MUX_ENABLE), y)
include  $(SOURCE_DIR)/kernel/service/mux/module.mk
endif

ifeq ($(CCCI_ENABLE), y)
include  $(SOURCE_DIR)/kernel/service/ccci/module.mk
endif

include  $(SOURCE_DIR)/kernel/service/syslog/module.mk
include  $(SOURCE_DIR)/kernel/service/exception_handler/module.mk
include  $(SOURCE_DIR)/kernel/service/offline_dump/module.mk
include  $(SOURCE_DIR)/kernel/service/swla/module.mk
include  $(SOURCE_DIR)/kernel/service/systemhang_tracer/module.mk
ifeq ($(AIR_BOOTREASON_CHECK_ENABLE), y)
include  $(SOURCE_DIR)/kernel/service/bootreason_check/module.mk
endif
include  $(SOURCE_DIR)/kernel/service/system_daemon/module.mk
include  $(SOURCE_DIR)/kernel/service/memory_monitor/module.mk
include  $(SOURCE_DIR)/kernel/service/clib_optimization/module.mk

# layout partition
include $(SOURCE_DIR)/kernel/service/layout_partition/module.mk

C_FILES += $(KERNEL_SERVICE_FILES)

###################################################
# include path
CFLAGS += -I$(SOURCE_DIR)/kernel/service/inc
CFLAGS += -I$(SOURCE_DIR)/kernel/service/exception_handler/inc/bta_ic_premium_g3
CFLAGS += -I$(SOURCE_DIR)/kernel/rtos/$(IC_CONFIG)/FreeRTOS/Source/include
CFLAGS += -I$(SOURCE_DIR)/middleware/airoha/minicli/inc
###################################################
####        AIR_BTA_IC_STEREO_HIGH_G3         #####
###################################################
else ifeq ($(AIR_BTA_IC_STEREO_HIGH_G3),y)
###################################################
# Sources
KERNEL_SERVICE_SRC    = kernel/service/src

KERNEL_SERVICE_FILES += $(KERNEL_SERVICE_SRC)/context_info_save.c \
                        $(KERNEL_SERVICE_SRC)/airoha_HeapSizeGuard.c \
                        $(KERNEL_SERVICE_SRC)/os_cli.c \
                        $(KERNEL_SERVICE_SRC)/os_port_callback.c \
                        $(KERNEL_SERVICE_SRC)/os_trace_callback.c \
                        $(KERNEL_SERVICE_SRC)/toi.c \
                        $(KERNEL_SERVICE_SRC)/utils.c \

ifeq ($(MTK_MUX_ENABLE), y)
include  $(SOURCE_DIR)/kernel/service/mux/module.mk
endif
include  $(SOURCE_DIR)/kernel/service/syslog/module.mk
include  $(SOURCE_DIR)/kernel/service/exception_handler/module.mk
include  $(SOURCE_DIR)/kernel/service/offline_dump/module.mk
include  $(SOURCE_DIR)/kernel/service/swla/module.mk
include  $(SOURCE_DIR)/kernel/service/systemhang_tracer/module.mk
AIR_BOOTREASON_CHECK_ENABLE ?= $(MTK_BOOTREASON_CHECK_ENABLE)
ifeq ($(AIR_BOOTREASON_CHECK_ENABLE), y)
include  $(SOURCE_DIR)/kernel/service/bootreason_check/module.mk
endif
include  $(SOURCE_DIR)/kernel/service/system_daemon/module.mk
include  $(SOURCE_DIR)/kernel/service/memory_monitor/module.mk
include  $(SOURCE_DIR)/kernel/service/clib_optimization/module.mk

# layout partition
include $(SOURCE_DIR)/kernel/service/layout_partition/module.mk

C_FILES += $(KERNEL_SERVICE_FILES)

ifeq ($(CCCI_ENABLE), y)
include  $(SOURCE_DIR)/kernel/service/ccci/module.mk
endif

###################################################
# include path
CFLAGS += -I$(SOURCE_DIR)/kernel/service/inc
CFLAGS += -I$(SOURCE_DIR)/kernel/rtos/$(IC_CONFIG)/FreeRTOS/Source/include
CFLAGS += -I$(SOURCE_DIR)/middleware/airoha/minicli/inc

###################################################
####                  others                  #####
###################################################
else
###################################################
# Sources
KERNEL_SERVICE_SRC    = kernel/service/src

KERNEL_SERVICE_FILES += $(KERNEL_SERVICE_SRC)/exception_handler.c \
                        $(KERNEL_SERVICE_SRC)/airoha_HeapSizeGuard.c \
                        $(KERNEL_SERVICE_SRC)/os_cli.c \
                        $(KERNEL_SERVICE_SRC)/os_port_callback.c \
                        $(KERNEL_SERVICE_SRC)/os_trace_callback.c \
                        $(KERNEL_SERVICE_SRC)/toi.c \
                        $(KERNEL_SERVICE_SRC)/utils.c \

include  $(SOURCE_DIR)/kernel/service/syslog/module.mk
include  $(SOURCE_DIR)/kernel/service/offline_dump/module.mk
include  $(SOURCE_DIR)/kernel/service/swla/module.mk
include  $(SOURCE_DIR)/kernel/service/systemhang_tracer/module.mk

C_FILES += $(KERNEL_SERVICE_FILES)

###################################################
# include path
CFLAGS += -I$(SOURCE_DIR)/kernel/service/inc
CFLAGS += -I$(SOURCE_DIR)/kernel/rtos/$(IC_CONFIG)/FreeRTOS/Source/include
CFLAGS += -I$(SOURCE_DIR)/middleware/airoha/minicli/inc
endif

