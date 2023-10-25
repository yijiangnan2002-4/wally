Audio Manager module usage guide

Brief:          This module is the Audio Manager control implementation including all main audio behavior management and most of the control for DSP.

Usage:          GCC:  For Audio general control implementation, include the module with
                      1) Add the following module.mk for libs and source file:
                         include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/audio_manager/module.mk
                      2) Add the header file path:
                         CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/audio_manager/inc

Dependency:     None.

Notice:         None.

Relative doc:   None.

Example project:None.
