mixer stream module usage guide

Brief:          This module is sw mixer stream implementation.

Usage:          GCC:  For sw mixer stream, include the module with
                      1) Add the following module.mk for libs and source file:
                         include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/audio/module.mk
                      2) module.mk provides different options to enable or disable according profiles, please configure these options on specified GCC/feature.mk:
                         AIR_SW_MIXER_STREAM_ENABLE
                      3) Add the header file path:
                         CFLAGS  += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/audio/mixer_stream/inc

Dependency:     none.

Notice:         none.

Relative doc:   none.

Example project:none.

