Wind Detection prebuilt module usage guide

Brief:          This module is the Wind Detection prebuilt library.

Usage:          GCC:  For Wind Detection, include the module with
                      1) Add the following module.mk for libs and source file:
                         include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/dspalg/wind_detection/module.mk
                      2) module.mk provides different options to enable or disable according profiles, please configure these options on specified XT-XCC/feature.mk:
                         AIR_ANC_WIND_NOISE_DETECTION_ENABLE
                      3) Add the header file path:
                         CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/dspalg/wind_detection/inc

Dependency:     None

Notice:         None

Relative doc:   None

Example project:None