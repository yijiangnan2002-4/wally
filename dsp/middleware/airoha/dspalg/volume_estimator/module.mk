
###################################################
VOLUME_PATH = middleware/airoha/dspalg/volume_estimator
#ifeq ($(AIR_VOLUME_ESTIMATOR_ENABLE), y)
C_SRC += $(VOLUME_PATH)/src/volume_estimator_interface.c
LIBS += $(strip $(LIBDIR2))/volume_estimator/$(IC_CONFIG)/libchat_vol.a
#endif

###################################################
# include path

INC += $(VOLUME_PATH)/inc
INC += $(VOLUME_PATH)/portable