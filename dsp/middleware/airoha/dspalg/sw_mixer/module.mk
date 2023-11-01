
###################################################
SW_MIXER_PATH = $(MIDDLEWARE_PROPRIETARY)/dspalg/sw_mixer
#ifeq ($(AIR_SOFTWARE_MIXER_ENABLE), y)
C_SRC += $(SW_MIXER_PATH)/src/sw_mixer_interface.c
#endif

###################################################
# include path

INC += $(SW_MIXER_PATH)/inc