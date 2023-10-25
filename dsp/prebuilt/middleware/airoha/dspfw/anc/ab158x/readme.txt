ANC module usage guide

Brief:          This module is the Active Noise Cancellation (ANC) implementation. It provides support for noise cancellation.

Usage:          XT-XCC:  For ANC, make sure to include the following:
                      1) Add the following module.mk for lib and its API header file:
                         include $(SOURCE_DIR)/prebuilt/$(MIDDLEWARE_PROPRIETARY)/dspfw/anc/$(IC_CONFIG)/module.mk
                      2) module.mk provides different options to enable or disable according to the profiles.
                         Please configure the related options on the specified XT-XCC/feature.mk.
                         AIR_ANC_ENABLE
                      3) Add the header file path:
                         CFLAGS += -I$(SOURCE_DIR)/prebuilt/$(MIDDLEWARE_PROPRIETARY)/dspfw/anc/$(IC_CONFIG)/inc

Dependency:     None

Notice:         None

Relative doc:   None

Example project: None
