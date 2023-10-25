
###################################################
SRC_FIXED_RATIO_PATH = middleware/airoha/dspalg/src_fixed_ratio

C_SRC += $(SRC_FIXED_RATIO_PATH)/src/src_fixed_ratio_interface.c
ifeq ($(IC_CONFIG),ab158x)
ifneq ($(AIR_FIXED_RATIO_SRC_USE_PIC), y)
    LIBS += $(strip $(LIBDIR2))/src_fixed_ratio/$(IC_CONFIG)/libsampler_by_n.a
else
    PIC   += $(strip $(LIBDIR2))/src_fixed_ratio/$(IC_CONFIG)/pisplit/pisplit_sampler_by_n.o
    C_SRC += $(SRC_FIXED_RATIO_PATH)/portable/src_fixed_ratio_portable.c
endif
else
ifneq ($(AIR_FIXED_RATIO_SRC_USE_PIC), y)
    LIBS += $(strip $(LIBDIR2))/src_fixed_ratio/$(IC_CONFIG)/libsampler_by_n.a
else
    PIC   += $(strip $(LIBDIR2))/src_fixed_ratio/$(IC_CONFIG)/pisplit/pisplit_sampler_by_n.o
    C_SRC += $(SRC_FIXED_RATIO_PATH)/portable/src_fixed_ratio_portable.c
endif
endif

###################################################
# include path

INC += $(SRC_FIXED_RATIO_PATH)/inc
INC += $(SRC_FIXED_RATIO_PATH)/portable