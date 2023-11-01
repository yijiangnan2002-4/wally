SMART_SRC = $(MIDDLEWARE_PROPRIETARY)/smart_charger
ifeq ($(AIR_1WIRE_ENABLE),y)
C_FILES += $(SMART_SRC)/src/chargersmartcase.c
C_FILES += $(SMART_SRC)/src/smartcase.c
C_FILES += $(SMART_SRC)/src/smchg_1wire.c
endif
C_FILES += $(SMART_SRC)/src/smchg_1wire_config.c
#################################################################################
#include path
CFLAGS  += -I$(SOURCE_DIR)/middleware/util/include
CFLAGS  += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/minicli/include
CFLAGS  += -I$(SOURCE_DIR)/kernel/service/mux/inc
CFLAGS  += -I$(SOURCE_DIR)/kernel/rtos/$(IC_CONFIG)/FreeRTOS/include
CFLAGS  += -I$(SOURCE_DIR)/kernel/rtos/$(IC_CONFIG)/FreeRTOS/Source/include
ifeq ($(AIR_BTA_IC_PREMIUM_G3),y)
CFLAGS  += -I$(SOURCE_DIR)/kernel/rtos/$(IC_CONFIG)/FreeRTOS/Source/include/portable/GCC/ARM_CM33
else
CFLAGS  += -I$(SOURCE_DIR)/kernel/rtos/$(IC_CONFIG)/FreeRTOS/Source/include/portable/GCC/ARM_CM4F
endif
CFLAGS  += -I$(SOURCE_DIR)/kernel/rtos/$(IC_CONFIG)/FreeRTOS/Source
CFLAGS  += -I$(SOURCE_DIR)/middleware/mlog/include
CFLAGS  += -I$(SOURCE_DIR)/driver/chip/mt$(PRODUCT_VERSION)/include
CFLAGS  += -I$(SOURCE_DIR)/driver/chip/inc
CFLAGS  += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/smart_charger/inc

#################################################################################
#Enable the feature by configuring
CFLAGS += -DMTK_CHARGER_CASE
CFLAGS += -DMTK_SMART_ENABLED
