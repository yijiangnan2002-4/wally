
###################################################
SW_GAIN_PATH = $(MIDDLEWARE_PROPRIETARY)/dspalg/sw_gain
#ifeq ($(AIR_SOFTWARE_GAIN_ENABLE), y)
C_SRC += $(SW_GAIN_PATH)/src/sw_gain_interface.c
#endif

###################################################
# include path

INC += $(SW_GAIN_PATH)/inc