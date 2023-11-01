Vivid PT module usage guide

Brief:          This module is the Vivid passthrough (Vivid PT) implementation. It provides support for Vivid PT.

Usage:          XT-XCC:  For Vivid PT, make sure to include the following:
                      1) Add the following module.mk for libs and source file:
                         $(ROOTDIR)/middleware/airoha/llf/stream/module.mk
                      2) Module.mk provides different options to enable or disable according to the profiles.
                         Please configure the related options according to your requirement on the specified XT-XCC/feature.mk.
                         Set AIR_PASSTHRU_ENABLE_TYPE as "PASSTHRU_VIVID" for vivid Passs through.
                      3) Add the header file path:
                         INC += $(ROOTDIR)/middleware/airoha/llf/vivid_pt/inc

Dependency:     The Vivid PT module is only effective when the Vivid PT prebuilt module and LLF module also exists.

Notice:         None

Relative doc:   None

Example project: None
