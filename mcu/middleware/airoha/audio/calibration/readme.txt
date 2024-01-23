Audio Calibration module usage guide

Brief:          This module is the Audio Calibration control implementation.

Usage:          GCC:  For Audio Calibration, include the module with
                      1) Add the following module.mk for libs and source file:
                         include $(SOURCE_DIR)/middleware/airoha/audio/module.mk
                      2) Module.mk provides different options to enable or disable according to the profiles.
                         Please configure the related options on the specified GCC/feature.mk.
                         AIR_COMPONENT_CALIBRATION_ENABLE
                      3) Add the header file path:
                         CFLAGS += -I$(SOURCE_DIR)/middleware/airoha/audio/calibration/inc

Dependency:     None.

Notice:         None.

Relative doc:   None.

Example project:None.
