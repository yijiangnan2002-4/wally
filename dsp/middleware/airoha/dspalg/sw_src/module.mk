
###################################################
SW_SRC_PATH = $(MIDDLEWARE_PROPRIETARY)/dspalg/sw_src
#ifeq ($(AIR_SOFTWARE_SRC_ENABLE), y)
C_SRC += $(SW_SRC_PATH)/src/sw_src_interface.c
ifeq ($(AIR_BTA_IC_PREMIUM_G3),y)
LIBS += $(strip $(LIBDIR2))/sw_src/ab158x/libswsrc_158x.a
else ifeq ($(AIR_BTA_IC_STEREO_HIGH_G3),y)
LIBS += $(strip $(LIBDIR2))/sw_src/ab157x/libswsrc_157x.a
else
LIBS += $(strip $(LIBDIR2))/sw_src/ab156x/libswsrc_156x.a
endif
#endif

###################################################
# include path

INC += $(SW_SRC_PATH)/inc