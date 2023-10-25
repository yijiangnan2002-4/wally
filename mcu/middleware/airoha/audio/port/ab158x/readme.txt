Audio mcu porting layer module usage guide

Brief:          This module is the audio mcu porting layer implementation.

Usage:          GCC:  For audio mcu porting layer, include the module with
                      1) Add the following module.mk for libs and source file:
                         include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/audio/port/$(IC_CONFIG)/module.mk
                      2) Add the header file path:
                         CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/audio/port/$(IC_CONFIG)/inc

Dependency:     None.

Notice:         None.

Relative doc:   None.

Example project:None.
