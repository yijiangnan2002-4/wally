Leakage Compensation prebuilt module usage guide

Brief:          This module is the Leakage Compensation prebuilt library.

Usage:          GCC:  For Leakage Compensation, include the module with
                      1) Add the following module.mk for libs and source file:
                         include $(SOURCE_DIR)/middleware/airoha/dspalg/leakage_compensation/module.mk
                      2) module.mk provides different options to enable or disable according profiles, please configure these options on specified XT-XCC/feature.mk:
                         AIR_ANC_FIT_DETECTION_ENABLE
                      3) Add the header file path:
                         CFLAGS += -I$(SOURCE_DIR)/middleware/airoha/dspalg/leakage_compensation/inc

Dependency:     None

Notice:         None

Relative doc:   None

Example project:None
