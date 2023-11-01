include $(SOURCE_DIR)/driver/chip/$(IC_CONFIG)/src/GCC/module.mk



ifneq ($(wildcard $(strip $(SOURCE_DIR))/driver/chip/$(IC_CONFIG)/src_core/),)
include $(SOURCE_DIR)/driver/chip/$(IC_CONFIG)/src_core/GCC/module.mk
else
LIBS += $(SOURCE_DIR)/prebuilt/driver/chip/$(IC_CONFIG)/lib/libhal_core_CM4_GCC.a
endif

LIBS += $(SOURCE_DIR)/prebuilt/driver/chip/$(IC_CONFIG)/lib/libhal_protected_CM4_GCC.a

