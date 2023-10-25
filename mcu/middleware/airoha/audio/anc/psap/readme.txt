PSAP module usage guide

Brief:          This module is the Personal sound amplification product (PSAP) implementation.

Usage:          GCC:  For PSAP, include the module with
                      1) Add the following module.mk for libs and source file:
                         include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/audio/anc/psap/module.mk
                      2) Module.mk provides different options to enable or disable according to the profiles.
                         Please configure the related options according to your requirement on the specified XT-XCC/feature.mk.
                         Set AIR_PASSTHRU_ENABLE_TYPE as "PASSTHRU_PSAP" for PSAP.
                         Set AIR_PASSTHRU_ENABLE_TYPE as "PASSTHRU_HEARING_AID" for Hearing aid.
                      3) Add the header file path:
                         CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/audio/anc/psap/inc

Dependency:     The PSAP module is only effective when the LLF module also exists.

Notice:         None.

Relative doc:   None.

Example project:None.
