Audio PLC prebuilt module usage guide

Brief:          This module is the library for Audio PLC implementation.

Usage:          For Audio PLC, make sure to include the following:
                      1) Add the following module.mk for libs and source file:
                         include $(SOURCE_DIR)/middleware/third_party/dspalg/audio_plc/module.mk
                      2) Set AIR_AUDIO_PLC_ENABLE as "y" in specified XT-XCC/feature.mk.
                      3) Add the header file path:
                         CFLAGS += -I$(SOURCE_DIR)/middleware/third_party/dspalg/audio_plc/inc
                      4) Add FUNC_AUDIO_PLC into the feature table in dsp_sdk.c to apply Audio PLC in the specified scenario.

Dependency:     None
Notice:         None
Relative doc:   None
Example project:None