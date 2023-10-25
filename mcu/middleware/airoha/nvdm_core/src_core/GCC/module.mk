NVDM_SRC = $(MIDDLEWARE_PROPRIETARY)/nvdm
NVDM_CORE_SRC = $(MIDDLEWARE_PROPRIETARY)/nvdm_core

C_FILES += $(NVDM_CORE_SRC)/src_core/nvdm_data.c \
		   $(NVDM_CORE_SRC)/src_core/nvdm_main.c \
		   $(NVDM_CORE_SRC)/src_core/nvdm_io.c

#################################################################################
#include path
CFLAGS 	+= -I$(SOURCE_DIR)/$(NVDM_SRC)/inc
CFLAGS 	+= -I$(SOURCE_DIR)/$(NVDM_CORE_SRC)/inc_core
CFLAGS  += -I$(SOURCE_DIR)/middleware/util/include
CFLAGS  += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/minicli/include
CFLAGS  += -I$(SOURCE_DIR)/kernel/rtos/$(IC_CONFIG)/FreeRTOS/include
CFLAGS  += -I$(SOURCE_DIR)/kernel/rtos/$(IC_CONFIG)/FreeRTOS/Source/include
CFLAGS  += -I$(SOURCE_DIR)/kernel/rtos/$(IC_CONFIG)/FreeRTOS/Source/include/portable/GCC/ARM_CM4F
CFLAGS  += -I$(SOURCE_DIR)/kernel/rtos/$(IC_CONFIG)/FreeRTOS/Source
CFLAGS  += -I$(SOURCE_DIR)/middleware/mlog/include
CFLAGS  += -I$(SOURCE_DIR)/driver/chip/mt$(PRODUCT_VERSION)/include
CFLAGS  += -I$(SOURCE_DIR)/driver/chip/inc

#################################################################################
#Enable the feature by configuring
CFLAGS += -DMTK_NVDM_ENABLE

# align with the feature setting of kernel/service/system_daemon/module.mk
ifeq ($(PRODUCT_VERSION),3335)
# not support non blocking write on AG3335
else ifeq ($(PRODUCT_VERSION),1565)
CFLAGS += -DSYSTEM_DAEMON_TASK_ENABLE
else ifeq ($(PRODUCT_VERSION),2822)
CFLAGS += -DSYSTEM_DAEMON_TASK_ENABLE
else ifeq ($(AIR_BTA_IC_PREMIUM_G3),y)
CFLAGS += -DSYSTEM_DAEMON_TASK_ENABLE
else ifeq ($(PRODUCT_VERSION),1552)
CFLAGS += -DSYSTEM_DAEMON_TASK_ENABLE
else ifeq ($(PRODUCT_VERSION),2552)
CFLAGS += -DSYSTEM_DAEMON_TASK_ENABLE
else
endif