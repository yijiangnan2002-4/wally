LLF module usage guide

Brief:          This module is the Low Latency Framework (LLF) implementation. It provides support for LLF.

Usage:          XT-XCC:  For LLF, make sure to include the following:
                      1) Add the following module.mk for libs and source file:
                         $(ROOTDIR)/middleware/airoha/llf/stream/module.mk
                      2) Module.mk provides different options to enable or disable according to the profiles.
                         Please configure the related options according to your requirement on the specified XT-XCC/feature.mk.
                         Set AIR_LLF_ENABLE as "y" for customization.
                         Set AIR_PASSTHRU_ENABLE_TYPE as "PASSTHRU_PSAP" for PSAP.
                         Set AIR_PASSTHRU_ENABLE_TYPE as "PASSTHRU_HEARING_AID" for Hearing aid.
                         Set AIR_PASSTHRU_ENABLE_TYPE as "PASSTHRU_VIVID" for vivid Passs through.
                      3) Add the header file path:
                         INC += $(ROOTDIR)/middleware/airoha/llf/stream/inc

Dependency:     None

Notice:         None

Relative doc:   None

Example project: None
