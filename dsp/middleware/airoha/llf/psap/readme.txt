PSAP module usage guide

Brief:          This module is the Personal sound amplification product (PSAP) implementation. It provides support for PSAP.

Usage:          XT-XCC:  For PSAP, make sure to include the following:
                      1) Add the following module.mk for libs and source file:
                         $(ROOTDIR)/middleware/airoha/llf/stream/module.mk
                      2) Module.mk provides different options to enable or disable according to the profiles.
                         Please configure the related options according to your requirement on the specified XT-XCC/feature.mk.
                         Set AIR_PASSTHRU_ENABLE_TYPE as "PASSTHRU_PSAP" for PSAP.
                         Set AIR_PASSTHRU_ENABLE_TYPE as "PASSTHRU_HEARING_AID" for Hearing aid.
                      3) Add the header file path:
                         INC += $(ROOTDIR)/middleware/airoha/llf/psap/inc

Dependency:     PSAP module is only effective when the PSAP prebuilt module and LLF module also exist.

Notice:         None

Relative doc:   None

Example project: None
