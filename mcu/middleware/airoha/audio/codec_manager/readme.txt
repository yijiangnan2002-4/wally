codec manager module usage guide

Brief:          This module manages codecs for audio middleware implementation, to encode or decode different codec data format.

Usage:          GCC:  For codec manager, include the module with
                      1) Add the following module.mk for libs and source file:
                         include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/audio/module.mk
                      2) Module.mk provide different options to enable or disable according profiles.
                         please configure these options on specified GCC/feature.mk:
                         AIR_AUDIO_CODEC_MANAGER_ENABLE
                         AIR_OPUS_ENCODER_ENABLE
                         AIR_SBC_ENCODER_ENABLE
                      3) Add the header file path:
                         CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/audio/codec_manager/inc

Dependency:     none.

Notice:         none.

Relative doc:   none.

Example project:none.

