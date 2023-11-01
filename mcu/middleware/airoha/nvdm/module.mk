NVDM_SRC = $(MIDDLEWARE_PROPRIETARY)/nvdm

C_FILES += $(NVDM_SRC)/src/nvdm_port.c

ifeq ($(PRODUCT_VERSION),7687)
C_FILES += $(NVDM_SRC)/src/nvdm_cli.c
endif
ifeq ($(PRODUCT_VERSION),7697)
C_FILES += $(NVDM_SRC)/src/nvdm_cli.c
endif
ifeq ($(PRODUCT_VERSION),7686)
C_FILES += $(NVDM_SRC)/src/nvdm_cli.c
endif
ifeq ($(PRODUCT_VERSION),7682)
C_FILES += $(NVDM_SRC)/src/nvdm_cli.c
endif
ifeq ($(PRODUCT_VERSION),5932)
ifneq ($(MTK_WIFI_STUB_CONF_ENABLE), y)
C_FILES += $(NVDM_SRC)/src/nvdm_cli.c
endif
endif
ifeq ($(IC_CONFIG),ab155x)
C_FILES += $(NVDM_SRC)/src/nvkey.c
endif
ifeq ($(IC_CONFIG),ab156x)
C_FILES += $(NVDM_SRC)/src/nvkey.c
endif

ifeq ($(PRODUCT_VERSION),2822)
C_FILES += $(NVDM_SRC)/src/nvkey.c
endif

ifeq ($(AIR_BTA_IC_PREMIUM_G3),y)
C_FILES += $(NVDM_SRC)/src/nvkey.c
endif

ifeq ($(AIR_BTA_IC_STEREO_HIGH_G3),y)
C_FILES += $(NVDM_SRC)/src/nvkey.c
endif

ifeq ($(IC_CONFIG),am255x)
C_FILES += $(NVDM_SRC)/src/nvkey.c
endif

ifeq ($(PRODUCT_VERSION),7698)
C_FILES += $(NVDM_SRC)/src/nvdm_cli.c
endif

#################################################################################
#include path
CFLAGS  += -I$(SOURCE_DIR)/$(NVDM_SRC)/inc
CFLAGS  += -I$(SOURCE_DIR)/$(NVDM_SRC)/inc_core
CFLAGS  += -I$(SOURCE_DIR)/middleware/util/include
CFLAGS  += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/minicli/include
CFLAGS  += -I$(SOURCE_DIR)/kernel/rtos/$(IC_CONFIG)/FreeRTOS/include
CFLAGS  += -I$(SOURCE_DIR)/kernel/rtos/$(IC_CONFIG)/FreeRTOS/Source/include
CFLAGS  += -I$(SOURCE_DIR)/kernel/rtos/$(IC_CONFIG)/FreeRTOS/Source/include/portable/GCC/ARM_CM4F
CFLAGS  += -I$(SOURCE_DIR)/kernel/rtos/$(IC_CONFIG)/FreeRTOS/Source
CFLAGS  += -I$(SOURCE_DIR)/middleware/mlog/include
CFLAGS  += -I$(SOURCE_DIR)/driver/chip/mt$(PRODUCT_VERSION)/include
CFLAGS  += -I$(SOURCE_DIR)/driver/chip/inc

ifneq ($(wildcard $(strip $(SOURCE_DIR))/$(MIDDLEWARE_PROPRIETARY)/nvdm_core/),)
include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/nvdm_core/src_core/GCC/module.mk
else
include $(SOURCE_DIR)/prebuilt/$(MIDDLEWARE_PROPRIETARY)/nvdm/$(IC_CONFIG)/module.mk
endif

#################################################################################
#Enable the feature by configuring
CFLAGS += -DMTK_NVDM_ENABLE
CFLAGS += -DAIR_NVDM_ENABLE
CFLAGS += -DNVDM_SUPPORT_MULTIPLE_PARTITION
