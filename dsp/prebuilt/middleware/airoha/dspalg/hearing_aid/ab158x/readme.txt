HA module usage guide

Brief:          This module is the Hearing Aid (HA) implementation. It provides support for hearing aid.

Usage:          XT-XCC:  For HA, make sure to include the following:
                      1) Add the following module.mk for lib and its API header file:
                         include $(SOURCE_DIR)/prebuilt/$(MIDDLEWARE_PROPRIETARY)/dspalg/hearing_aid/$(IC_CONFIG)/module.mk
                      2) module.mk provides different options to enable or disable according to the profiles.
                         Please configure the related options on the specified XT-XCC/feature.mk.
                         AIR_HEARING_AID_ENABLE
                      3) Add the header file path:
                         CFLAGS += -I$(SOURCE_DIR)/prebuilt/$(MIDDLEWARE_PROPRIETARY)/dspalg/hearing_aid/$(IC_CONFIG)/inc

Dependency:     None

Notice:         pisplit_awha_nr.o cites some open source information, please refer to oss_info.txt for more details and refer to noisePowProposed_license.txt for the license.

Relative doc:   None

Example project: None
