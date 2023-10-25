Vivid PT module usage guide

Brief:          This module is the Vivid passthrough (Vivid PT) implementation.

Usage:          GCC:  For Vivid PT, include the module with
                      1) Add the following module.mk for libs and source file:
                         include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/audio/anc/vivid_pt/module.mk
                      2) Module.mk provides different options to enable or disable according to the profiles.
                         Please configure the related options according to your requirement on the specified XT-XCC/feature.mk.
                         Set AIR_PASSTHRU_ENABLE_TYPE as "PASSTHRU_VIVID" for vivid Passs through.
                      3) Add the header file path:
                         CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/audio/anc/vivid_pt/inc

Dependency:     The Vivid PT module is only effective when the LLF module also exists.

Notice:         None.

Relative doc:   None.

Example project:None.
