Audio Mixer control module usage guide

Brief:          This module is the Software Audio Mixer control implementation.

Usage:          GCC:  For Software Audio Mixer control, include the module with
                      1) Add the following module.mk for libs and source file:
                         include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/audio/module.mk
                      2) Add the header file path:
                         CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/audio/audio_mixer/inc

Dependency:     None.

Notice:         None.

Relative doc:   None.

Example project:None.
