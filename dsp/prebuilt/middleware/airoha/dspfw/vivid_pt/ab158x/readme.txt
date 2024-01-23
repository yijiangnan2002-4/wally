Vivid PT module usage guide
Brief:          This module is the Vivid Passthrough (Vivid PT) implementation. It provides support for Vivid PT.
Usage:          XT-XCC:  For Vivid PT make sure to include the following:
                      1) Add the following module.mk for lib and its API header file:
                         include $(SOURCE_DIR)/prebuilt/$(MIDDLEWARE_PROPRIETARY)/dspfw/vivid_pt/$(IC_CONFIG)/module.mk
                      2) module.mk provides different options to enable or disable according to the profiles.
                         Please configure the related options on the specified XT-XCC/feature.mk.
                         AIR_PASSTHRU_ENABLE_TYPE = PASSTHRU_VIVID
                      3) Add the header file path:
                         CCFLAG += -I$(SOURCE_DIR)/prebuilt/$(MIDDLEWARE_PROPRIETARY)/dspfw/vivid_pt/$(IC_CONFIG)/inc
Dependency:     None
Notice:         None
Relative doc:   None
Example project: None