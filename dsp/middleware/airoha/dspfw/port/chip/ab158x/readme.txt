middleware common module usage guide

Brief:          This module is used to create a middleware of the DSP audio streaming usage.

Usage:          XT-XCC:  For streaming, make sure to include the following:
                      1) Add the following module.mk for libs and source file:
                         include $(ROOTDIR)/$(MIDDLEWARE_PROPRIETARY)/dspfw/port/chip/bta_ic_premium_g3/module.mk
                      2) Add the header file path:
                         CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/dspfw/port/chip/bta_ic_premium_g3/inc

Dependency:     None

Notice:         None

Relative doc:   None

Example project: None