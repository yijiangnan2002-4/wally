PRE_LOAD_LIB = kernel/pre_libloader

ifeq ($(PRELOADER_ENABLE),y)
ifeq ($(PRELOADER_ENABLE_DSP0_LOAD_FOR_DSP1),y)

CCFLAG += -DPRELOADER_ENABLE_DSP0_LOAD_FOR_DSP1

C_SRC  += $(PRE_LOAD_LIB)/dsp1/src/preloader_pisplit.c
C_SRC  += $(PRE_LOAD_LIB)/dsp1/src/preloader_pisplit_internal.c
endif

ifeq ($(DSP1_PISPLIT_DEMO_LIBRARY),y)
include $(ROOTDIR)/kernel/pre_libloader/dsp1/dsp1_pic_demo_portable/module.mk
endif

endif

#################################################################################
#include path
INC  += kernel/pre_libloader/inc
INC  += kernel/pre_libloader/dsp1/inc
