PSAP module usage guide

Brief:          This module is PSAP (Personal Sound Amplification Products) implementation.

Usage:          GCC:  For PSAP, include the module with
                      1) Add the following module.mk for libs and source file:
                         include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/audio/module.mk
                      2) Module.mk provides different options to enable or disable according to the profiles.
                         Please configure the related options on the specified GCC/feature.mk.
                         AIR_ADVANCED_PASSTHROUGH_ENABLE
                      3) Add the header file path:
                         CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/audio/psap_common/inc

Dependency:     None.

Notice:         None.

Relative doc:   None.

Example project:None.
