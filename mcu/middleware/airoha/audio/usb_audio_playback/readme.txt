usb audio playback module usage guide

Brief:          This module is usb audio playback implementation.

Usage:          GCC:  For usb audio playback, include the module with
                      1) Add the following module.mk for libs and source file:
                         include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/audio/module.mk
                      2) module.mk provides different options to enable or disable according profiles, please configure these options on specified GCC/feature.mk:
                         AIR_USB_AUDIO_IN_ENABLE
                      3) Add the header file path:
                         CFLAGS  += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/audio/usb_audio_playback/inc

Dependency:     none.

Notice:         none.

Relative doc:   none.

Example project:none.

