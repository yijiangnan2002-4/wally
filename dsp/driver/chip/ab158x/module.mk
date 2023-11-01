include $(ROOTDIR)/driver/chip/$(IC_CONFIG)/src/XT-XCC/module.mk

ifneq ($(wildcard $(strip $(ROOTDIR))/driver/chip/$(IC_CONFIG)/src_core/),)
include $(ROOTDIR)/driver/chip/$(IC_CONFIG)/src_core/XT-XCC/module.mk
else
LIBS += $(ROOTDIR)/prebuilt/driver/chip/$(IC_CONFIG)/lib/libhal_core_CM4_GCC.a
endif

