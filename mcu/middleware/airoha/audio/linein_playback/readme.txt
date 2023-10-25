linein playback module usage guide

Brief:          This module is line-in playback implementation.

Usage:          GCC:  For line-in playback, include the module with
                      1) Add the following module.mk for libs and source file:
                         include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/audio/module.mk
                      2) module.mk provides different options to enable or disable according profiles, please configure these options on specified GCC/feature.mk:
                         AIR_LINE_IN_ENABLE
                      3) Add the header file path:
                         CFLAGS  += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/audio/linein_playback/inc
                         CFLAGS  += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/audio/linein_playback/inc/linein_control

Dependency:     none.

Notice:         none.

Relative doc:   none.

Example project:none.

