Leakage Detection module usage guide

Brief:          This module is the Leakage Detection control implementation.

Usage:          GCC:  For Leakage Detection, include the module with
                      1) Add the following module.mk for libs and source file:
                         include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/audio/module.mk
                      2) module.mk provides different options to enable or disable according to the profiles.
                         Please configure the related options on the specified GCC/feature.mk.
                         AIR_ANC_FIT_DETECTION_ENABLE
                      3) Add the header file path:
                         CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/audio/leakage_detection/inc

Dependency:     None.

Notice:         None.

Relative doc:   None.

Example project:None.
