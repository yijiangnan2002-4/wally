Audio module header usage guide

Brief:          This module is the Audio control header with general codec callback function structure.

Usage:          GCC:  For Audio control header, include the module with
                      1) Add the following module.mk for libs and source file:
                         include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/audio/module.mk
                      2) Add the header file path:
                         CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/audio/inc

Dependency:     None.

Notice:         None.

Relative doc:   None.

Example project:None.
